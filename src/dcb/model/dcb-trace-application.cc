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

#include "dcb-net-device.h"
#include "ns3/integer.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "dcb-trace-application.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet-socket-address.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TraceApplication");
NS_OBJECT_ENSURE_REGISTERED (TraceApplication);

TypeId
TraceApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TraceApplication")
                          .SetParent<Application> ()
                          .SetGroupName ("DcApp")
                          .AddAttribute ("Protocol",
                                         "The type of protocol to use. This should be "
                                         "a subclass of ns3::SocketFactory",
                                         TypeIdValue (UdpSocketFactory::GetTypeId ()),
                                         MakeTypeIdAccessor (&TraceApplication::m_tid),
                                         // This should check for SocketFactory as a parent
                                         MakeTypeIdChecker ());
  return tid;
}

TraceApplication::TraceApplication (Ptr<DcTopology> topology, uint32_t nodeIndex)
    : m_topology (topology), m_nodeIndex (nodeIndex), m_connected (false), m_totBytes (0)

{
  NS_LOG_FUNCTION (this);

  m_hostIndexRng = topology->CreateRamdomHostChooser ();

  m_flowArriveTimeRng = CreateObject<ExponentialRandomVariable> ();
  m_flowArriveTimeRng->SetAntithetic (true);

  m_flowSizeRng = CreateObject<EmpiricalRandomVariable> ();
  m_flowSizeRng->SetAttribute ("Interpolate", BooleanValue (true));

  Ptr<DcbNetDevice> dev = DynamicCast<DcbNetDevice> (topology->GetNetDeviceOfNode (nodeIndex, 0));
  m_socketLinkRate = dev->GetDataRate ();
}

TraceApplication::~TraceApplication ()
{
  NS_LOG_FUNCTION (this);
}

// int64_t
// TraceApplication::AssignStreams (int64_t stream)
// {
//   NS_LOG_FUNCTION (this << stream);
// }

void
TraceApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;

  Time t = Simulator::Now ();
  while (t < m_stopTime)
    {
      t += GetNextFlowArriveInterval ();
      ScheduleNextFlow (t);
    }
}

void
TraceApplication::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<Socket>
TraceApplication::CreateNewRandomSocket ()
{
  Ptr<Socket> socket = Socket::CreateSocket (GetNode (), m_tid);
  socket->BindToNetDevice (GetNode ()->GetDevice (0));

  int ret = socket->Bind ();
  if (ret == -1)
    {
      NS_FATAL_ERROR ("Failed to bind socket");
    }

  uint32_t destNode;
  do
    {
      destNode = m_hostIndexRng->GetInteger (); // randomly send to a host
  } while (destNode == m_nodeIndex);
  Address destAddr = m_topology->GetNetDeviceOfNode (destNode, 0)->GetAddress ();

  socket->Connect (destAddr);
  socket->SetAllowBroadcast (false);

  // m_socket->SetConnectCallback (MakeCallback (&TraceApplication::ConnectionSucceeded, this),
  //                               MakeCallback (&TraceApplication::ConnectionFailed, this));
  socket->SetRecvCallback (MakeCallback (&TraceApplication::HandleRead, this));
  return socket;
}

void
TraceApplication::ScheduleNextFlow (const Time &startTime)
{
  NS_LOG_FUNCTION (this);

  Ptr<Socket> socket = CreateNewRandomSocket ();

  uint64_t size = GetNextFlowSize ();

  Flow *flow = new Flow (size, startTime, socket);
  // m_flows.emplace (flow);
  Simulator::Schedule (startTime - Simulator::Now (), &TraceApplication::SendNextPacket, this,
                       flow);
}

void
TraceApplication::SendNextPacket (Flow *flow)
{
  const uint32_t packetSize = std::min (flow->remainBytes, MSS);
  Ptr<Packet> packet = Create<Packet> (packetSize);
  int actual = flow->socket->Send (packet);
  if (actual == packetSize)
    {
      m_totBytes += packetSize;
      // const uint32_t headerSize = m_socket->GetSerializedHeaderSize ();
      constexpr const uint32_t headerSize = 8 + 20 + 14;
      Time txTime = m_socketLinkRate.CalculateBytesTxTime (packetSize + headerSize);
      if (Simulator::Now () + txTime < m_stopTime)
        {
          if (flow->remainBytes > MSS)
            { // Schedule next packet
              flow->remainBytes -= MSS;
              Simulator::Schedule (txTime, &TraceApplication::SendNextPacket, this, flow);
            }
          else
            { // flow completes
              // TODO: do some trace here
              // ...

              // m_flows.erase (flow);
              flow->Dispose ();
              flow = nullptr;
            }
        }
    }
  else
    {
      NS_FATAL_ERROR ("Unable to send packet; actual " << actual << " size " << packetSize << ";");
    }
}

void
TraceApplication::SetFlowMeanArriveInterval (double interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_flowArriveTimeRng->SetAttribute ("Mean", DoubleValue (interval));
}

void
TraceApplication::SetFlowCdf (const TraceCdf &cdf)
{
  NS_LOG_FUNCTION (this);
  for (auto [sz, prob] : cdf)
    {
      m_flowSizeRng->CDF (sz, prob);
    }
}

inline Time
TraceApplication::GetNextFlowArriveInterval () const
{
  return Time (MicroSeconds (m_flowArriveTimeRng->GetInteger ()));
}

inline uint32_t
TraceApplication::GetNextFlowSize () const
{
  return m_flowSizeRng->GetInteger ();
}

void
TraceApplication::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void
TraceApplication::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void
TraceApplication::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client received "
                                  << packet->GetSize () << " bytes from "
                                  << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port "
                                  << InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().As (Time::S) << " client received "
                                  << packet->GetSize () << " bytes from "
                                  << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port "
                                  << Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
      socket->GetSockName (localAddress);
      // m_rxTrace (packet);
      // m_rxTraceWithAddresses (packet, from, localAddress);
    }
}

} // namespace ns3
