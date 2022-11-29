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
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "dcb-trace-application.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/type-id.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet-socket-address.h"
#include "rocev2-l4-protocol.h"
#include "udp-based-socket.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TraceApplication");
NS_OBJECT_ENSURE_REGISTERED (TraceApplication);

TypeId
TraceApplication::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TraceApplication")
                          .SetParent<Application> ()
                          .SetGroupName ("Dcb")
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
    : m_enableSend (true),
      m_topology (topology),
      m_nodeIndex (nodeIndex),
      m_randomDestination (true),
      m_totBytes (0),
      m_headerSize (8 + 20 + 14)
{
  NS_LOG_FUNCTION (this);

  m_hostIndexRng = topology->CreateRamdomHostChooser ();

  InitForRngs ();
}

TraceApplication::TraceApplication (Ptr<DcTopology> topology, uint32_t nodeIndex,
                                    uint32_t destIndex)
    : m_enableSend (true),
      m_topology (topology),
      m_nodeIndex (nodeIndex),
      m_randomDestination (false),
      m_totBytes (0),
      m_headerSize (8 + 20 + 14)
{
  NS_LOG_FUNCTION (this);

  // 0 interface is LoopbackNetDevice
  m_destAddr = topology->GetInterfaceOfNode (destIndex, 1).GetAddress ();

  InitForRngs ();
}

void
TraceApplication::InitForRngs ()
{
  NS_LOG_FUNCTION (this);

  m_flowArriveTimeRng = CreateObject<ExponentialRandomVariable> ();
  m_flowArriveTimeRng->SetAntithetic (true);

  m_flowSizeRng = CreateObject<EmpiricalRandomVariable> ();
  m_flowSizeRng->SetAttribute ("Interpolate", BooleanValue (true));

  Ptr<DcbNetDevice> dev =
      DynamicCast<DcbNetDevice> (m_topology->GetNetDeviceOfNode (m_nodeIndex, 0));
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
TraceApplication::SetInnerUdpProtocol (std::string innerTid)
{
  SetInnerUdpProtocol (TypeId (innerTid));
}

void
TraceApplication::SetInnerUdpProtocol (TypeId innerTid)
{
  NS_LOG_FUNCTION (this << innerTid);
  m_tid = UdpBasedSocketFactory::GetTypeId ();
  Ptr<Node> node = GetNode ();
  Ptr<UdpBasedSocketFactory> socketFactory = node->GetObject<UdpBasedSocketFactory> ();
  if (socketFactory)
    {
      m_headerSize = socketFactory->AddUdpBasedProtocol (node, node->GetDevice (0), innerTid);
    }
  else
    {
      NS_FATAL_ERROR ("Application cannot use inner-UDP protocol because UdpBasedL4Protocol and "
                      "UdpBasedSocketFactory is not bound to node correctly.");
    }
}

void
TraceApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  if (m_enableSend)
    {
      for (Time t = Simulator::Now () + GetNextFlowArriveInterval (); t < m_stopTime;
           t += GetNextFlowArriveInterval ())
        {
          ScheduleNextFlow (t);
        }
    }
}

void
TraceApplication::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
}

InetSocketAddress
TraceApplication::GetDestinationAddr () const
{
  NS_LOG_FUNCTION (this);

  uint32_t portNum = 1234; // TODO: dynamic port
  if (m_randomDestination)
    { // randomly send to a host
      uint32_t destNode;
      do
        {
          destNode = m_hostIndexRng->GetInteger ();
      } while (destNode == m_nodeIndex);
      Ipv4Address ipv4Addr = m_topology->GetInterfaceOfNode (destNode, 1)
                                 .GetAddress (); // 0 interface is LoopbackNetDevice
      return InetSocketAddress (ipv4Addr, portNum);
    }
  else
    {
      return InetSocketAddress (m_destAddr, portNum);
    }
}

Ptr<Socket>
TraceApplication::CreateNewSocket ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Socket> socket = Socket::CreateSocket (GetNode (), m_tid);
  socket->BindToNetDevice (GetNode ()->GetDevice (0));
  int ret = socket->Bind ();
  if (ret == -1)
    {
      NS_FATAL_ERROR ("Failed to bind socket");
    }

  InetSocketAddress destAddr = GetDestinationAddr ();
  ret = socket->Connect (destAddr);
  if (ret == -1)
    {
      NS_FATAL_ERROR ("Socket connection failed");
    }
  socket->SetAllowBroadcast (false);
  // m_socket->SetConnectCallback (MakeCallback (&TraceApplication::ConnectionSucceeded, this),
  //                               MakeCallback (&TraceApplication::ConnectionFailed, this));
  socket->SetRecvCallback (MakeCallback (&TraceApplication::HandleRead, this));
  return socket;
}

void
TraceApplication::ScheduleNextFlow (const Time &startTime)
{
  Ptr<Socket> socket = CreateNewSocket ();

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
  if (actual == static_cast<int> (packetSize))
    {
      m_totBytes += packetSize;
      Time txTime = m_socketLinkRate.CalculateBytesTxTime (packetSize + m_headerSize);
      if (Simulator::Now () + txTime < m_stopTime)
        {
          if (flow->remainBytes > MSS)
            { // Schedule next packet
              flow->remainBytes -= MSS;
              Simulator::Schedule (txTime, &TraceApplication::SendNextPacket, this, flow);
              return;
            }
          else
            {
              // flow completes
              // TODO: do some trace here
              // ...
            }
        }
      // m_flows.erase (flow);
      flow->Dispose ();
      // Dispose will delete the struct leading to flow a dangling pointer.
      flow = nullptr;
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
  m_flowArriveTimeRng->SetAttribute ("Mean", DoubleValue (interval)); // in microseconds
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
  // Address localAddress;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("TraceApplication: At time "
                       << Simulator::Now ().As (Time::S) << " client received "
                       << packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port "
                       << InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("TraceApplication: At time "
                       << Simulator::Now ().As (Time::S) << " client received "
                       << packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port "
                       << Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
      // socket->GetSockName (localAddress);
      // m_rxTrace (packet);
      // m_rxTraceWithAddresses (packet, from, localAddress);
    }
}

void
TraceApplication::SetSendEnabled (bool enabled)
{
  m_enableSend = enabled;
}

} // namespace ns3
