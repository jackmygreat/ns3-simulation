/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Pavinberg <pavin0702@gmail.com>
 */

#include "rocev2-socket.h"
#include "dcb-net-device.h"
#include "dcqcn.h"
#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/packet.h"
#include "rocev2-l4-protocol.h"
#include "udp-based-l4-protocol.h"
#include "udp-based-socket.h"
#include "ns3/data-rate.h"
#include "ns3/ipv4-route.h"
#include "ns3/simulator.h"
#include "ns3/csv-writer.h"
#include <tuple>
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RoCEv2Socket");

NS_OBJECT_ENSURE_REGISTERED (RoCEv2Socket);

TypeId
RoCEv2Socket::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::RoCEv2Socket")
                          .SetParent<UdpBasedSocket> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<RoCEv2Socket> ();
  return tid;
}

RoCEv2Socket::RoCEv2Socket ()
    : UdpBasedSocket (), m_isSending (false), m_senderNextPSN (0), m_psnEnd (0)
{
  NS_LOG_FUNCTION (this);
  m_sockState = CreateObject<RoCEv2SocketState> ();
  m_ccOps = CreateObject<DcqcnCongestionOps> (m_sockState);
  m_flowStartTime = Simulator::Now ();
}

RoCEv2Socket::~RoCEv2Socket ()
{
  NS_LOG_FUNCTION (this);
}

void
RoCEv2Socket::DoSendTo (Ptr<Packet> payload, Ipv4Address daddr, Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION (this << payload << daddr << route);

  RoCEv2Header rocev2Header = CreateNextProtocolHeader ();
  m_buffer.Push (rocev2Header.GetPSN (), std::move (rocev2Header), payload, daddr, route);
  SendPendingPacket ();
}

void
RoCEv2Socket::SendPendingPacket ()
{
  NS_LOG_FUNCTION (this);

  if (m_isSending || m_buffer.GetSizeToBeSent () == 0)
    {
      return;
    }
  // rateRatio is controled by congestion control
  // rateRatio = sending rate calculated by CC / line rate, which is between [0.0., 1.0]
  const double rateRatio = m_sockState->GetRateRatio (); // in percentage, i.e., maximum is 100.0
  if (rateRatio > 1e-6)
    {
      m_isSending = true;
      [[maybe_unused]] const auto &[_, rocev2Header, payload, daddr, route] =
          m_buffer.GetNextShouldSent ();
      const uint32_t sz = payload->GetSize () + 8 + 20 + 14;
      // DCQCN is a rate based CC.
      // Send packet and delay a bit time to control the sending rate.
      Time delay = m_deviceRate.CalculateBytesTxTime (sz * 100 / rateRatio);
      m_ccOps->UpdateStateSend (payload);
      Ptr<Packet> packet = payload->Copy (); // do not modify the payload in the buffer
      packet->AddHeader (rocev2Header);
      m_innerProto->Send (packet, route->GetSource (), daddr, m_endPoint->GetLocalPort (),
                          m_endPoint->GetPeerPort (), route);
      Simulator::Schedule (delay, &RoCEv2Socket::FinishSendPendingPacket, this);
    }
}

void
RoCEv2Socket::FinishSendPendingPacket ()
{
  m_isSending = false;
  SendPendingPacket ();
}

void
RoCEv2Socket::ForwardUp (Ptr<Packet> packet, Ipv4Header header, uint32_t port,
                         Ptr<Ipv4Interface> incomingInterface)
{
  RoCEv2Header rocev2Header;
  packet->RemoveHeader (rocev2Header);

  switch (rocev2Header.GetOpcode ())
    {
    case RoCEv2Header::Opcode::RC_ACK:
      HandleACK (packet, rocev2Header);
      break;
    case RoCEv2Header::Opcode::CNP:
      m_ccOps->UpdateStateWithCNP ();
      break;
    default:
      HandleDataPacket (packet, header, port, incomingInterface, rocev2Header);
    }
}

void
RoCEv2Socket::HandleACK (Ptr<Packet> packet, const RoCEv2Header &roce)
{
  NS_LOG_FUNCTION (this << packet);

  AETHeader aeth;
  packet->RemoveHeader (aeth);

  switch (aeth.GetSyndromeType ())
    {
      case AETHeader::SyndromeType::FC_DISABLED: { // normal ACK
        uint32_t psn = m_buffer.Pop ().m_psn; // packet acked, pop out from buffer
        if (psn != roce.GetPSN ())
          {
            NS_FATAL_ERROR ("[WARN] RoCEv2 socket receive an ACK with PSN not expected");
          }
        if (psn + 1 == m_psnEnd)
          { // last ACk received, flow finshed
            // TODO: do some trae here, record FCT
            NotifyFlowCompletes ();
            // NS_LOG_INFO ("At time " << Simulator::Now () << " node " << Simulator::GetContext ()
            //                         << " received last ack packet");

            Close ();
          }
        break;
      }
      case AETHeader::SyndromeType::NACK: {
        GoBackN (roce.GetPSN ());
        break;
      }
      default: {
        NS_FATAL_ERROR ("Unexpected AET header Syndrome. Packet format wrong.");
      }
    }
}

void
RoCEv2Socket::HandleDataPacket (Ptr<Packet> packet, Ipv4Header header, uint32_t port,
                                Ptr<Ipv4Interface> incomingInterface, const RoCEv2Header &roce)
{
  NS_LOG_FUNCTION (this << packet);

  const uint32_t srcQP = roce.GetSrcQP (), dstQP = roce.GetDestQP ();
  Time lastCNPTime (0);
  uint32_t expectedPSN = 0;
  std::map<uint32_t, FlowInfo>::iterator flowInfoIter = m_receiverFlowInfo.find (srcQP);
  if (flowInfoIter != m_receiverFlowInfo.end ())
    {
      lastCNPTime = flowInfoIter->second.lastCNPTime;
      expectedPSN = flowInfoIter->second.nextPSN;
    }
  else
    {
      auto pp = m_receiverFlowInfo.emplace (srcQP, FlowInfo {0, Time (0)});
      flowInfoIter = std::move (pp.first);
    }

  // Check ECN
  if (header.GetEcn () == Ipv4Header::EcnType::ECN_CE) // ECN congestion encountered
    {
      if (Simulator::Now () - lastCNPTime >= MicroSeconds (50))
        {
          // send Congestion Notification Packet (CNP) to sender
          Ptr<Packet> cnp = RoCEv2L4Protocol::GenerateCNP (dstQP, srcQP);
          m_innerProto->Send (cnp, header.GetDestination (), header.GetSource (), dstQP, srcQP, 0);
          flowInfoIter->second.lastCNPTime = Simulator::Now ();
        }
    }

  // Check PSN
  const uint32_t psn = roce.GetPSN ();
  if (psn > expectedPSN)
    { // packet out-of-order, send NACK
      Ptr<Packet> nack = RoCEv2L4Protocol::GenerateNACK (dstQP, srcQP, expectedPSN);
      m_innerProto->Send (nack, header.GetDestination (), header.GetSource (), dstQP, srcQP, 0);
    }
  else if (psn == expectedPSN)
    {
      flowInfoIter->second.nextPSN = (expectedPSN + 1) & 0xffffff;
      if (roce.GetAckQ ())
        { // send ACK
          Ptr<Packet> ack = RoCEv2L4Protocol::GenerateACK (dstQP, srcQP, psn);
          m_innerProto->Send (ack, header.GetDestination (), header.GetSource (), dstQP, srcQP, 0);
        }
    }
  else
    {
      NS_LOG_WARN ("[WARN] RoCEv2 socket receives smaller PSN than expected, something wrong");
    }

  UdpBasedSocket::ForwardUp (packet, header, port, incomingInterface);
}

void
RoCEv2Socket::GoBackN (uint32_t lostPSN) const
{
  // DcbTxBuffer::DcbTxBufferItemI item = m_buffer.FindPSN(lostPSN);
  NS_LOG_WARN (
      "[WARN] Go-back-N not implemented. Packet lost or out-of-order happens. Sender is node "
      << Simulator::GetContext () << " at time " << Simulator::Now ());
}

int
RoCEv2Socket::Bind ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_boundnetdevice,
                 "RoCEv2Socket should be bound to a net device before calling Bind");
  m_endPoint = m_innerProto->Allocate ();
  m_endPoint->SetRxCallback (MakeCallback (&RoCEv2Socket::ForwardUp, this));
  return 0;
}

void
RoCEv2Socket::BindToNetDevice (Ptr<NetDevice> netdevice)
{
  NS_LOG_FUNCTION (this << netdevice);
  // a little check
  if (netdevice != 0)
    {
      bool found = false;
      Ptr<Node> node = GetNode ();
      for (uint32_t i = 0; i < node->GetNDevices (); i++)
        {
          if (node->GetDevice (i) == netdevice)
            {
              found = true;
              break;
            }
        }
      NS_ASSERT_MSG (found, "Socket cannot be bound to a NetDevice not existing on the Node");
    }
  m_boundnetdevice = netdevice;
  // store device data rate
  Ptr<DcbNetDevice> dcbDev = DynamicCast<DcbNetDevice> (netdevice);
  if (dcbDev)
    {
      m_deviceRate = dcbDev->GetDataRate ();
      double rai =
          static_cast<double> (DataRate ("40Mbps").GetBitRate ()) / m_deviceRate.GetBitRate ();
      m_ccOps->SetRateAIRatio (rai);
      m_ccOps->SetRateHyperAIRatio (5 * rai);
      m_ccOps->SetReady ();
    }
}

int
RoCEv2Socket::BindToLocalPort (uint32_t port)
{
  NS_LOG_FUNCTION (this << port);

  NS_ASSERT_MSG (m_boundnetdevice,
                 "RoCEv2Socket should be bound to a net device before calling Bind");
  m_endPoint = DynamicCast<RoCEv2L4Protocol> (m_innerProto)->Allocate (port, 0);
  m_endPoint->SetRxCallback (MakeCallback (&RoCEv2Socket::ForwardUp, this));
  return 0;
}

void
RoCEv2Socket::FinishSending ()
{
  NS_LOG_FUNCTION (this);

  m_psnEnd = m_senderNextPSN;
}

void
RoCEv2Socket::SetStopTime (Time stopTime)
{
  m_ccOps->SetStopTime (stopTime);
}

Time
RoCEv2Socket::GetFlowStartTime () const
{
  return m_flowStartTime;
}

RoCEv2Header
RoCEv2Socket::CreateNextProtocolHeader ()
{
  NS_LOG_FUNCTION (this);

  RoCEv2Header rocev2Header;
  rocev2Header.SetOpcode (RoCEv2Header::Opcode::RC_SEND_ONLY); // TODO: custom opcode
  rocev2Header.SetDestQP (m_endPoint->GetPeerPort ());
  rocev2Header.SetSrcQP (m_endPoint->GetLocalPort ());
  rocev2Header.SetPSN (m_senderNextPSN);
  rocev2Header.SetAckQ (true);
  m_senderNextPSN = (m_senderNextPSN + 1) % 0xffffff;
  return rocev2Header;
}

NS_OBJECT_ENSURE_REGISTERED (DcbTxBuffer);

TypeId
DcbTxBuffer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcbTxBuffer")
                          .SetGroupName ("Dcb")
                          .SetParent<Object> ()
                          .AddConstructor<DcbTxBuffer> ();
  return tid;
}

DcbTxBuffer::DcbTxBuffer () : m_sentIdx (0)
{
}

void
DcbTxBuffer::Push (uint32_t psn, RoCEv2Header header, Ptr<Packet> packet, Ipv4Address daddr,
                   Ptr<Ipv4Route> route)
{
  m_buffer.emplace_back (psn, header, packet, daddr, route);
}

const DcbTxBuffer::DcbTxBufferItem &
DcbTxBuffer::Front () const
{
  return m_buffer.front ();
}

DcbTxBuffer::DcbTxBufferItem
DcbTxBuffer::Pop ()
{
  DcbTxBuffer::DcbTxBufferItem item = std::move (m_buffer.front ());
  m_buffer.pop_front ();
  if (m_sentIdx)
    {
      m_sentIdx--;
    }
  return item;
}

const DcbTxBuffer::DcbTxBufferItem &
DcbTxBuffer::GetNextShouldSent ()
{
  if (m_buffer.size () - m_sentIdx)
    {
      return m_buffer.at (m_sentIdx++);
    }
  NS_FATAL_ERROR ("DcbTxBuffer has no packet to be sent.");
}

uint32_t
DcbTxBuffer::Size () const
{
  return m_buffer.size ();
}

uint32_t
DcbTxBuffer::GetSizeToBeSent () const
{
  return m_buffer.size () - m_sentIdx;
}

DcbTxBuffer::DcbTxBufferItemI
DcbTxBuffer::FindPSN (uint32_t psn) const
{
  for (auto it = m_buffer.cbegin (); it != m_buffer.cend (); it++)
    {
      if (psn == (*it).m_psn)
        {
          return it;
        }
    }
  return End ();
}

DcbTxBuffer::DcbTxBufferItemI
DcbTxBuffer::End () const
{
  return m_buffer.cend ();
}

DcbTxBuffer::DcbTxBufferItem::DcbTxBufferItem (uint32_t psn, RoCEv2Header header,
                                               Ptr<Packet> payload, Ipv4Address daddr,
                                               Ptr<Ipv4Route> route)
    : m_psn (psn), m_header (header), m_payload (payload), m_daddr (daddr), m_route (route)
{
}

NS_OBJECT_ENSURE_REGISTERED (RoCEv2SocketState);

TypeId
RoCEv2SocketState::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::RoCEv2SocketState")
                          .SetParent<Object> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<RoCEv2SocketState> ();
  return tid;
}

RoCEv2SocketState::RoCEv2SocketState () : m_rateRatio (100.)
{
}

} // namespace ns3
