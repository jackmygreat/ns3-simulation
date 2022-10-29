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
#include "dcb-net-device.h"
#include "ns3/address.h"
#include "ns3/boolean.h"
#include "ns3/ethernet-header.h"
#include "ns3/fatal-error.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/pfc-frame.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/type-id.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-header.h"
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
DcbTrafficControl::SetRootQueueDiscOnDevice (Ptr<DcbNetDevice> device, Ptr<PausableQueueDisc> qDisc)
{
  NS_LOG_FUNCTION (this << device << qDisc);

  device->SetQueueDisc (qDisc);
  TrafficControlLayer::SetRootQueueDiscOnDevice (device, qDisc);
}

void
DcbTrafficControl::RegisterDeviceNumber (uint32_t num)
{
  m_pfc.ports.resize (num);
}

void
DcbTrafficControl::Receive (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                            const Address &from, const Address &to,
                            NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << device << packet << protocol << from << to << packetType);
  if (m_pfcEnabled) // PFC logic
    {
      uint32_t index = device->GetIfIndex ();
      uint8_t priority = PeekPriorityOfPacket (packet);
      
      // Add priority to packet tag
      CoSTag cosTag;
      cosTag.SetCoS (priority);
      packet->AddPacketTag (cosTag);
      
      if (m_pfc.CheckEnableVec (index, priority)) // if PFC is enabled on this priority
        {
          // Add ingress port index to packet tag
          DeviceIndexTag tag (index);
          packet->AddPacketTag (tag); // egress will read the index from tag to decrement counter

          if (m_pfc.CheckShouldSendPause (index, priority, packet->GetSize ()))
            {
              NS_LOG_DEBUG ("Send pause frame from " << Simulator::GetContext () << " port " << index);
              Ptr<Packet> pfcFrame = GeneratePauseFrame (priority);
              // pause frames are sent directly to device without queueing in egress QueueDisc
              device->Send (pfcFrame, from, PfcFrame::PROT_NUMBER);
              m_pfc.SetPaused(index, priority, true); // mark this ingress queue as paused
            }
          m_pfc.IncrementPfcQueueCounter (index, priority, packet->GetSize ());
        }
    }

  TrafficControlLayer::Receive (device, packet, protocol, from, to, packetType);
}

void
DcbTrafficControl::ReceivePfc (Ptr<NetDevice> dev, Ptr<const Packet> packet,
                               uint16_t protocol, const Address &from, const Address &to,
                               NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION(this << dev << protocol << from << to);

  Ptr<DcbNetDevice> device = DynamicCast<DcbNetDevice>(dev);
  const uint32_t index = device->GetIfIndex ();
  PfcFrame pfcFrame;
  packet->PeekHeader (pfcFrame);
  uint8_t enableVec = pfcFrame.GetEnableClassField ();
  for (uint8_t priority = 0; enableVec > 0; enableVec >>= 1, priority++)
    {
      if (enableVec & 1)
        {
          uint16_t quanta = pfcFrame.GetQuanta (priority);
          if (quanta > 0)
            {
              uint64_t bitRate = device->GetDataRate ().GetBitRate ();
              Time pauseTime = NanoSeconds (1e9 * quanta * PfcFrame::QUANTUM_BIT / bitRate);
              Ptr<PausableQueueDisc> qDisc = device->GetQueueDisc ();
              qDisc->SetPaused (priority, true);
              EventId event =
                  Simulator::Schedule (pauseTime, &PausableQueueDisc::SetPaused, qDisc, priority,
                                       false); // resume the queue after the pause time.
              m_pfc.getQueue(index, priority).ReplacePauseEvent (event);
              NS_LOG_LOGIC("PFC: port " << index << " priority " << priority << " is paused");
            }
          else
            {
              m_pfc.getQueue(index, priority).CancelPauseEvent();
              NS_LOG_LOGIC("PFC: port " << index << " priority " << priority << " is resumed");
            }
        }
    }
}

void
DcbTrafficControl::Send (Ptr<NetDevice> device, Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << device << item);
  const Ptr<Packet> packet = item->GetPacket ();
  // Get priority from packet tag.
  // We use tag rather than DSCP field to get the priority because in this way strategy
  // can use different ways to set priority.
  CoSTag cosTag;
  packet->PeekPacketTag (cosTag);
  const uint8_t priority = cosTag.GetCoS ();
  
  if (m_pfcEnabled) // PFC logic: update counter and check if should send Resume Frame
    {
      DeviceIndexTag devTag;
      if (packet->RemovePacketTag (devTag))
        {
          const uint32_t fromIdx = devTag.GetIndex ();
          m_pfc.DecrementPfcQueueCounter (fromIdx, priority, packet->GetSize ());
          if (m_pfc.CheckShouldSendResume (fromIdx, priority))
            {
              NS_LOG_DEBUG ("Send Resume frame from " << Simulator::GetContext () << " port "
                                                      << fromIdx);
              Ptr<Packet> pfcFrame = GeneratePauseFrame (priority, (uint16_t) 0);
              device->GetNode ()->GetDevice (fromIdx)->Send (pfcFrame, Address (),
                                                             PfcFrame::PROT_NUMBER);
              m_pfc.SetPaused(fromIdx, priority, false);
            }
        }
    }

  TrafficControlLayer::Send (device, item);
}

void
DcbTrafficControl::SetEnableVec (uint32_t deviceIndex, uint8_t enableVec)
{
  m_pfc.ports[deviceIndex].m_enableVec = enableVec;
}

void
DcbTrafficControl::SetPfcOfPortPriority (uint32_t index, uint32_t priority, uint32_t reserve,
                                         uint32_t xon)
{
  if (index >= m_pfc.ports.size ())
    {
      NS_FATAL_ERROR ("Index of port " << index << " is out of range of " << m_pfc.ports.size ());
    }
  if (priority >= 8)
    {
      NS_FATAL_ERROR ("PFC priority should be 0~7, your input is " << priority);
    }
  if (xon > reserve)
    {
      NS_FATAL_ERROR ("XON should be less or equal to reserve");
    }
  Pfc::PortInfo::IngressQueueInfo &q = m_pfc.ports[index].getQueue (priority);
  q.xon = xon;
  q.reserve = reserve;
}

bool
DcbTrafficControl::Pfc::CheckShouldSendPause (uint32_t port, uint8_t priority,
                                              uint32_t packetSize) const
{
  // TODO: add support for dynamic threshold
  const PortInfo::IngressQueueInfo &q = ports[port].getQueue (priority);
  return !q.isPaused && q.queueLength + packetSize > q.reserve;
}
bool
DcbTrafficControl::Pfc::CheckShouldSendResume (uint32_t port, uint8_t priority) const
{
  // TODO: add support for dynamic threshold
  const PortInfo::IngressQueueInfo &q = ports[port].getQueue (priority);
  return q.isPaused && q.queueLength <= q.xon;
}

Ptr<Packet>
DcbTrafficControl::GeneratePauseFrame (uint8_t priority, uint16_t quanta)
{
  PfcFrame pfcFrame;
  pfcFrame.EnableClass (priority); // only enable this priority
  pfcFrame.SetQuanta (priority, quanta);

  Ptr<Packet> packet = Create<Packet> (0);
  packet->AddHeader (pfcFrame);
  return packet;
}

Ptr<Packet>
DcbTrafficControl::GeneratePauseFrame (uint8_t enableVec, uint16_t quantaList[8])
{
  PfcFrame pfcFrame;
  pfcFrame.SetEnableClassField (enableVec);
  for (int cls = 0; cls < 8; cls++)
    {
      pfcFrame.SetQuanta (cls, quantaList[cls]);
    }
  Ptr<Packet> packet = Create<Packet> (0);
  packet->AddHeader (pfcFrame);
  return packet;
}

inline uint8_t
DcbTrafficControl::PeekPriorityOfPacket (Ptr<const Packet> packet)
{
  Ipv4Header ipv4Header;
  packet->PeekHeader (ipv4Header);
  return ipv4Header.GetDscp () >> 3;
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
