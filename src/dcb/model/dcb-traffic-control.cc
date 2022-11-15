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

#include "dcb-traffic-control.h"
#include "dcb-flow-control-port.h"
#include "dcb-pfc-port.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/ethernet-header.h"
#include "ns3/fatal-error.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/nstime.h"
#include "ns3/pfc-frame.h"
#include "ns3/type-id.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-header.h"
#include "ns3/socket.h"
#include "pausable-queue-disc.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcbTrafficControl");

NS_OBJECT_ENSURE_REGISTERED (DcbTrafficControl);

TypeId
DcbTrafficControl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcbTrafficControl")
                          .SetParent<TrafficControlLayer> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<DcbTrafficControl> ();
  return tid;
}

TypeId
DcbTrafficControl::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

DcbTrafficControl::DcbTrafficControl () : TrafficControlLayer ()
{
  NS_LOG_FUNCTION (this);
}

DcbTrafficControl::~DcbTrafficControl ()
{
  NS_LOG_FUNCTION (this);
}

void
DcbTrafficControl::SetRootQueueDiscOnDevice (Ptr<DcbNetDevice> device, Ptr<PausableQueueDisc> qDisc)
{
  NS_LOG_FUNCTION (this << device << qDisc);

  device->SetQueueDisc (qDisc);
  TrafficControlLayer::SetRootQueueDiscOnDevice (device, qDisc);
}

void
DcbTrafficControl::RegisterDeviceNumber (const uint32_t num)
{
  NS_LOG_FUNCTION (this << num);
  m_ports.resize (num);
}

void
DcbTrafficControl::Receive (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                            const Address &from, const Address &to,
                            NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << device << packet << protocol << from << to << packetType);

  // Add priority to packet tag
  uint8_t priority = PeekPriorityOfPacket (packet);
  CoSTag cosTag;
  cosTag.SetCoS (priority);
  packet->AddPacketTag (cosTag); // CoSTag is removed in PausableQueueDisc::DoEnqueue

  // Add from-index to packet tag
  uint32_t index = device->GetIfIndex ();
  DeviceIndexTag tag (index);
  packet->AddPacketTag (tag); // egress will read the index from tag to decrement counter
  // update ingress queue length
  Ipv4Header ipv4Header;
  packet->PeekHeader (ipv4Header);
  IncrementIngressQueueCounter (index, priority,
                                packet->GetSize () - ipv4Header.GetSerializedSize ());

  if (m_ports[index].FcEnabled ())
    {
      // run flow control ingress process
      m_ports[index].GetFC ()->IngressProcess (packet, protocol, from, to, packetType);
    }
  TrafficControlLayer::Receive (device, packet, protocol, from, to, packetType);
}

void
DcbTrafficControl::EgressProcess (uint32_t outPort, uint8_t priority, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << outPort << priority << packet);
  DeviceIndexTag tag;
  packet->RemovePacketTag (tag);
  uint32_t fromIdx = tag.GetIndex ();
  DecrementIngressQueueCounter (fromIdx, priority, packet->GetSize ());

  m_ports[outPort].CallFCPacketOutPipeline (fromIdx, priority, packet);
  if (m_ports[outPort].FcEnabled ())
    {
      m_ports[outPort].GetFC ()->EgressProcess (packet);
    }
}

void
DcbTrafficControl::InstallFCToPort (uint32_t portIdx, Ptr<DcbFlowControlPort> fc)
{
  NS_LOG_FUNCTION (this << portIdx);
  m_ports[portIdx].SetFC (fc);

  // Set egress callback to other ports.
  // When we enable FC on one port, it means that other ports may do something when
  // sending out the packet.
  // For example, if we config PFC on port 0, than ports other than 0 should check
  // whether port 0 has to send RESUME frame when sending out a packet.
  PortInfo::FCPacketOutCb cb = MakeCallback (&DcbFlowControlPort::PacketOutCallbackProcess, fc);
  for (auto &port : m_ports)
    {
      port.AddPacketOutCallback (portIdx, cb);
    }
}

// static
uint8_t
DcbTrafficControl::PeekPriorityOfPacket (const Ptr<const Packet> packet)
{
  Ipv4Header ipv4Header;
  packet->PeekHeader (ipv4Header);
  return Socket::IpTos2Priority (ipv4Header.GetTos ());
  // return ipv4Header.GetDscp () >> 3;
}

inline void
DcbTrafficControl::IncrementIngressQueueCounter (uint32_t index, uint8_t priority,
                                                 uint32_t packetSize)
{
  // NOTICE: no index checking nor value checking for better performance, be careful
  m_ports[index].IncreQueueLength (priority, static_cast<uint32_t> (ceil (packetSize / CELL_SIZE)));
}

void
DcbTrafficControl::DecrementIngressQueueCounter (uint32_t index, uint8_t priority,
                                                 uint32_t packetSize)
{
  // NOTICE: no index checking nor value checking for better performance, be careful
  m_ports[index].IncreQueueLength (priority,
                                   -static_cast<uint32_t> (ceil (packetSize / CELL_SIZE)));
}

void
DcbTrafficControl::PortInfo::AddPacketOutCallback (uint32_t fromIdx, FCPacketOutCb cb)
{
  NS_LOG_FUNCTION (this);
  m_fcPacketOutPipeline.emplace_back (fromIdx, cb);
}

void
DcbTrafficControl::PortInfo::CallFCPacketOutPipeline (uint32_t fromIdx, uint8_t priority,
                                                      Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  for (auto &handler : m_fcPacketOutPipeline)
    {
      if (handler.first == fromIdx)
        {
          const FCPacketOutCb &cb = handler.second;
          cb (priority, packet);
        }
    }
}

/** Tags implementation **/

DeviceIndexTag::DeviceIndexTag (uint32_t index) : m_index (index)
{
}

void
DeviceIndexTag::SetIndex (uint32_t index)
{
  m_index = index;
}

uint32_t
DeviceIndexTag::GetIndex () const
{
  return m_index;
}

TypeId
DeviceIndexTag::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DevIndexTag")
                          .SetParent<Tag> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<DeviceIndexTag> ();
  return tid;
}

TypeId
DeviceIndexTag::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
DeviceIndexTag::GetSerializedSize () const
{
  return sizeof (uint32_t);
}

void
DeviceIndexTag::Serialize (TagBuffer i) const
{
  i.WriteU32 (m_index);
}

void
DeviceIndexTag::Deserialize (TagBuffer i)
{
  m_index = i.ReadU32 ();
}

void
DeviceIndexTag::Print (std::ostream &os) const
{
  os << "Device = " << m_index;
}

CoSTag::CoSTag (uint8_t cos) : m_cos (cos)
{
}

void
CoSTag::SetCoS (uint8_t cos)
{
  m_cos = cos;
}

uint8_t
CoSTag::GetCoS () const
{
  return m_cos;
}

TypeId
CoSTag::GetTypeId ()
{
  static TypeId tid =
      TypeId ("ns3::CoSTag").SetParent<Tag> ().SetGroupName ("Dcb").AddConstructor<CoSTag> ();
  return tid;
}

TypeId
CoSTag::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
CoSTag::GetSerializedSize () const
{
  return sizeof (uint8_t);
}

void
CoSTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_cos);
}

void
CoSTag::Deserialize (TagBuffer i)
{
  m_cos = i.ReadU8 ();
}

void
CoSTag::Print (std::ostream &os) const
{
  os << "Device = " << m_cos;
}

} // namespace ns3
