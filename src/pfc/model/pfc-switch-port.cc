/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 Nanjing University
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
 * Author: Yanqing Chen  <shellqiqi@outlook.com>
 */

#include "pfc-switch-port.h"
#include "pfc-switch-tag.h"
#include "ns3/internet-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PfcSwitchPort");

NS_OBJECT_ENSURE_REGISTERED (PfcSwitchPort);

TypeId
PfcSwitchPort::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PfcSwitchPort")
                          .SetParent<Object> ()
                          .SetGroupName ("PfcSwitchPort")
                          .AddConstructor<PfcSwitchPort> ()
                          .AddTraceSource ("PfcRx", "Receive a PFC packet",
                                           MakeTraceSourceAccessor (&PfcSwitchPort::m_pfcRxTrace),
                                           "Ptr<DpskNetDevice>, uint32_t, "
                                           "PfcHeader::PfcType, uint16_t");
  return tid;
}

PfcSwitchPort::PfcSwitchPort ()
    : m_nQueues (0),
      m_lastQueueIdx (0),
      m_isPassThrough (false),
      m_nInQueueBytes (0),
      m_nInQueuePackets (0),
      m_nTxBytes (0),
      m_nRxBytes (0)
{
  NS_LOG_FUNCTION (this);
  m_name = "PfcSwitchPort";
}

PfcSwitchPort::~PfcSwitchPort ()
{
  NS_LOG_FUNCTION (this);
}

void
PfcSwitchPort::SetupQueues (uint32_t n)
{
  NS_LOG_FUNCTION (n);
  CleanQueues ();
  m_nQueues = n;
  for (uint32_t i = 0; i <= m_nQueues; i++) // with another control queue
    {
      m_queues.push_back (std::queue<Ptr<Packet>> ());
      m_pausedStates.push_back (false);
      m_inQueueBytesList.push_back (0);
      m_inQueuePacketsList.push_back (0);
    }
}

void
PfcSwitchPort::CleanQueues ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_queues.clear ();
  m_pausedStates.clear ();
  m_inQueueBytesList.clear ();
  m_inQueuePacketsList.clear ();
}

void
PfcSwitchPort::SetDeviceDequeueHandler (DeviceDequeueNotifier h)
{
  m_mmuCallback = h;
}

void
PfcSwitchPort::SetPassThrough (bool flag)
{
  m_isPassThrough = flag;
}

bool
PfcSwitchPort::IsPassThrough ()
{
  return m_isPassThrough;
}

Ptr<Packet>
PfcSwitchPort::Transmit ()
{
  NS_LOG_FUNCTION_NOARGS ();

  uint32_t qIndex;
  Ptr<Packet> p = Dequeue (qIndex);

  if (p == 0)
    return 0;

  // Notify switch dequeue event
  m_mmuCallback (m_dev, p, qIndex);

  PfcSwitchTag tag;
  p->RemovePacketTag (tag);

  m_nTxBytes += p->GetSize ();

  return p;
}

bool
PfcSwitchPort::Send (Ptr<Packet> packet, const Address &source, const Address &dest,
                     uint16_t protocolNumber)
{
  // Assembly Ethernet header
  EthernetHeader ethHeader;
  ethHeader.SetSource (Mac48Address::ConvertFrom (source));
  ethHeader.SetDestination (Mac48Address::ConvertFrom (dest));
  ethHeader.SetLengthType (protocolNumber);

  if (protocolNumber == PfcHeader::PROT_NUM) // PFC protocol number
    {
      // Add Ethernet header
      packet->AddHeader (ethHeader);
      // Enqueue control queue
      m_queues[m_nQueues].push (packet);
      m_inQueueBytesList[m_nQueues] += packet->GetSize ();
      m_inQueuePacketsList[m_nQueues]++;
    }
  else // Not PFC
    {
      // Get queue index
      Ipv4Header ipHeader;
      packet->PeekHeader (ipHeader);
      auto dscp = ipHeader.GetDscp ();
      // Add Ethernet header
      packet->AddHeader (ethHeader);
      // Enqueue
      m_queues[dscp].push (packet);
      m_inQueueBytesList[dscp] += packet->GetSize ();
      m_inQueuePacketsList[dscp]++;
    }

  m_nInQueueBytes += packet->GetSize ();
  m_nInQueuePackets++;

  return true; // Enqueue packet successfully
}

bool
PfcSwitchPort::Receive (Ptr<Packet> p)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_nRxBytes += p->GetSize ();

  PfcSwitchTag tag (m_dev->GetIfIndex ());
  p->AddPacketTag (tag); // Add tag for tracing input device when dequeue in the net device

  // Get Ethernet header
  EthernetHeader ethHeader;
  p->PeekHeader (ethHeader);

  if (ethHeader.GetLengthType () == PfcHeader::PROT_NUM) // PFC protocol number
    {
      // Pop Ethernet header
      p->RemoveHeader (ethHeader);
      // Pop PFC header
      PfcHeader pfcHeader;
      p->RemoveHeader (pfcHeader);

      if (pfcHeader.GetType () == PfcHeader::Pause) // PFC Pause
        {
          // Update paused state
          uint32_t pfcQIndex = pfcHeader.GetQIndex ();
          uint32_t qIndex = (pfcQIndex >= m_nQueues) ? m_nQueues : pfcQIndex;
          m_pausedStates[qIndex] = true;
          m_pfcRxTrace (m_dev, qIndex, PfcHeader::Pause, pfcHeader.GetTime ());
        }
      else if (pfcHeader.GetType () == PfcHeader::Resume) // PFC Resume
        {
          // Update paused state
          uint32_t pfcQIndex = pfcHeader.GetQIndex ();
          uint32_t qIndex = (pfcQIndex >= m_nQueues) ? m_nQueues : pfcQIndex;
          m_pausedStates[qIndex] = false;
          m_pfcRxTrace (m_dev, qIndex, PfcHeader::Resume, pfcHeader.GetTime ());
          m_dev->TriggerTransmit (); // Trigger device transmitting
        }
      else
        {
          NS_ASSERT_MSG (false, "PfcSwitchPort::Rx: Invalid PFC type");
          return false; // Drop this packet
        }
      // Check whether pass through PFC frame
      if (m_isPassThrough)
        {
          p->AddHeader (pfcHeader);
          p->AddHeader (ethHeader);
          return true;
        }
      else
        {
          return false;
        }
    }
  else // Not PFC
    {
      return true; // Forward up to node without any change
    }
}

Ptr<Packet>
PfcSwitchPort::DequeueRoundRobin (uint32_t &qIndex)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_nInQueuePackets == 0 || m_nQueues == 0)
    {
      NS_LOG_LOGIC ("Queues empty");
      return 0;
    }
  else
    {
      for (uint32_t i = 0; i < m_nQueues; i++)
        {
          uint32_t qIdx = (m_lastQueueIdx + i) % m_nQueues;
          if (m_pausedStates[qIdx] == false && m_inQueuePacketsList[qIdx] > 0)
            {
              Ptr<Packet> p = m_queues[qIdx].front ();
              m_queues[qIdx].pop ();

              m_nInQueueBytes -= p->GetSize ();
              m_nInQueuePackets--;
              m_inQueueBytesList[qIdx] -= p->GetSize ();
              m_inQueuePacketsList[qIdx]--;

              m_lastQueueIdx = qIdx;
              qIndex = qIdx;

              return p;
            }
        }
      return 0;
    }
}

Ptr<Packet>
PfcSwitchPort::Dequeue (uint32_t &qIndex)
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_inQueuePacketsList[m_nQueues] == 0 || m_pausedStates[m_nQueues]) // No control packets
    {
      return DequeueRoundRobin (qIndex);
    }
  else // Dequeue control packets
    {
      Ptr<Packet> p = m_queues[m_nQueues].front ();
      m_queues[m_nQueues].pop ();

      m_nInQueueBytes -= p->GetSize ();
      m_nInQueuePackets--;
      m_inQueueBytesList[m_nQueues] -= p->GetSize ();
      m_inQueuePacketsList[m_nQueues]--;

      qIndex = m_nQueues;
      return p;
    }
}

void
PfcSwitchPort::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_queues.clear ();
  DpskNetDeviceImpl::DoDispose ();
}

} // namespace ns3
