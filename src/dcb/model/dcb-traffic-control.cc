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
#include "ns3/boolean.h"
#include "ns3/ethernet-header.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/net-device.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/type-id.h"
#include "ns3/simulator.h"
#include "../utils/pfc-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcbTrafficControl");

NS_OBJECT_ENSURE_REGISTERED (DcbTrafficControl);

TypeId
DcbTrafficControl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DcbTrafficControl")
                          .SetParent<TrafficControlLayer> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<DcbTrafficControl> ()
                          .AddAttribute ("PfcEnabled", "Whether enable PFC.", BooleanValue (false),
                                         MakeBooleanAccessor (&DcbTrafficControl::m_pfcEnabled),
                                         MakeBooleanChecker ());
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
DcbTrafficControl::RegisterDeviceNumber (uint32_t num)
{
  m_pfcCounter.resize (num);
}

void
DcbTrafficControl::Receive (Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol,
                            const Address &from, const Address &to,
                            NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << device << p << protocol << from << to << packetType);
  if (m_pfcEnabled) // PFC logic
    {
      uint32_t index = device->GetIfIndex ();
	  // NS_LOG_DEBUG (Simulator::GetContext() << " receive packet from " << index);
      DeviceIndexTag tag (index);
      p->AddPacketTag (tag);
      m_pfcCounter[index]++; // NOTICE: no checking for better performance, be careful
    }

  TrafficControlLayer::Receive (device, p, protocol, from, to, packetType);
}

void
DcbTrafficControl::ReceivePfc (Ptr<NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from,
            const Address &to, NetDevice::PacketType packetType)
{
  // TODO: process PFC pause frame
}

void
DcbTrafficControl::Send (Ptr<NetDevice> device, Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << device << item);
  if (m_pfcEnabled) // PFC logic
    {
      DeviceIndexTag tag;
      if (item->GetPacket ()->RemovePacketTag (tag))
        {
          m_pfcCounter[tag.GetIndex ()]--; // NOTICE: no checking for better performance, be careful
          NS_LOG_DEBUG ("Tag at egress: " << Simulator::GetContext () << " " << tag.GetIndex ());
        }
    }

  TrafficControlLayer::Send (device, item);
}

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

} // namespace ns3
