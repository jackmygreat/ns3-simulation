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

#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/type-id.h"
#include "ns3/ipv4-end-point.h"
#include "ns3/rocev2-header.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "udp-based-l4-protocol.h"
#include "udp-based-socket.h"
#include "rocev2-l4-protocol.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RoCEv2L4Protocol");

NS_OBJECT_ENSURE_REGISTERED (RoCEv2L4Protocol);

TypeId
RoCEv2L4Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RoCEv2L4Protocol")
                          .SetParent<UdpBasedL4Protocol> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<RoCEv2L4Protocol> ();
  return tid;
}

RoCEv2L4Protocol::RoCEv2L4Protocol ()
{
  NS_LOG_FUNCTION (this);
  m_innerEndPoints = new InnerEndPointDemux (0x100, 0xfffff);
  InnerEndPoint *endPoint =  m_innerEndPoints->Allocate (GetDefaultServicePort(), 0); // as a receive service
  endPoint->SetRxCallback (MakeCallback (&RoCEv2L4Protocol::ServerReceive, this));
}

RoCEv2L4Protocol::~RoCEv2L4Protocol ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Socket>
RoCEv2L4Protocol::CreateSocket ()
{
  NS_LOG_FUNCTION (this);
  Ptr<UdpBasedSocket> socket = CreateObject<UdpBasedSocket> ();
  socket->SetNode (m_node);
  socket->SetUdp (this);
  m_sockets.emplace_back (socket);
  return socket;
}

void
RoCEv2L4Protocol::FinishSetup (Ipv4EndPoint *const udpEndPoint)
{
  NS_LOG_FUNCTION (this);

  if (udpEndPoint != 0)
    {
      udpEndPoint->SetRxCallback (MakeCallback (&RoCEv2L4Protocol::ForwardUp, this));
      udpEndPoint->SetDestroyCallback (MakeCallback (&RoCEv2L4Protocol::Destroy, this));
    }
  else
    {
      NS_FATAL_ERROR ("No UDP endpoint allocated to me.");
    }
}

void
RoCEv2L4Protocol::PreSend (Ptr<Packet> packet, Ipv4Address saddr, Ipv4Address daddr, uint32_t srcQP,
                           uint32_t destQP, Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION (this << packet << saddr << daddr << srcQP << destQP);

  RoCEv2Header rocev2Header;
  rocev2Header.SetOpCode (RoCEv2Header::Opcode::UD_SEND_ONLY);
  rocev2Header.SetDestQP (destQP);
  packet->AddHeader (rocev2Header);
}

uint16_t
RoCEv2L4Protocol::GetProtocolNumber (void) const
{
  return PROT_NUMBER;
}

uint32_t
RoCEv2L4Protocol::GetInnerProtocolHeaderSize () const
{
  static uint32_t sz = 0;
  if (sz == 0)
    {
      RoCEv2Header rocev2Header;
      sz = rocev2Header.GetSerializedSize ();
    }
  return sz;
}

uint32_t
RoCEv2L4Protocol::GetHeaderSize () const
{
  static uint32_t sz = 8 + 20 + 14 + GetInnerProtocolHeaderSize ();
  return sz;
}

uint32_t
RoCEv2L4Protocol::GetDefaultServicePort () const
{
  return DEFAULT_DST_QP;
}

InnerEndPoint *
RoCEv2L4Protocol::Allocate (uint32_t dstPort)
{
  return m_innerEndPoints->Allocate (dstPort);
}

uint32_t
RoCEv2L4Protocol::ParseInnerPort (Ptr<Packet> packet, Ipv4Header header, uint16_t port,
                                  Ptr<Ipv4Interface> incomingIntf)
{
  NS_LOG_FUNCTION (this << packet);
  RoCEv2Header rocev2Header;
  packet->PeekHeader (rocev2Header);
  return rocev2Header.GetDestQP ();
}

void
RoCEv2L4Protocol::ServerReceive (Ptr<Packet> packet, Ipv4Header header, uint32_t port,
                                 Ptr<Ipv4Interface> incommingInterface)
{
  NS_LOG_FUNCTION (this << packet);

  RoCEv2Header rocev2Header;
  packet->RemoveHeader (rocev2Header);
  RoCEv2Header::Opcode opcode = rocev2Header.GetOpCode ();
  switch (opcode)
    {
    case RoCEv2Header::Opcode::RC_SEND_ONLY:
      [[fallthrough]];
    case RoCEv2Header::Opcode::UD_SEND_ONLY:
      NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " RoCEv2 server received " << packet->GetSize () << " bytes from " << header.GetSource());
      break;
    case RoCEv2Header::Opcode::CNP:
      break;
    }
}

} // namespace ns3
