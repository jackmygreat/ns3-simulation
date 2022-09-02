/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "pfc-switch.h"
#include "ns3/internet-module.h"
#include "qbb-header.h"
#include "pfc-header.h"
#include "pfc-switch-tag.h"
#include "pfc-switch-port.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PfcSwitch");

NS_OBJECT_ENSURE_REGISTERED (PfcSwitch);

TypeId
PfcSwitch::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PfcSwitch")
                          .SetParent<DpskLayer> ()
                          .SetGroupName ("Pfc")
                          .AddConstructor<PfcSwitch> ();
  return tid;
}

PfcSwitch::PfcSwitch ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_dpsk = NULL;
  m_name = "PfcSwitch";
  m_nDevices = 0;
  m_mmu = 0;
  m_passThroughFrom = 0;
  m_passThroughTo = 0;
}

PfcSwitch::~PfcSwitch ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool
PfcSwitch::SendFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                           const Address &source, const Address &destination)
{
  NS_LOG_FUNCTION (device << packet << protocol << &source << &destination);

  if (m_dpsk != NULL)
    {
      m_dpsk->SendFromDevice (device, packet->Copy (), protocol, source, destination);
    }

  return true;
}

void
PfcSwitch::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                              const Address &source, const Address &destination,
                              NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (device << packet << protocol << &source << &destination << packetType);
  // Remove Ethernet header
  Ptr<Packet> p = packet->Copy ();
  EthernetHeader ethHeader;
  p->RemoveHeader (ethHeader);

  if (m_passThroughFrom != 0 && m_passThroughTo != 0 &&
      ethHeader.GetLengthType () == PfcHeader::PROT_NUM) // PFC protocol number
    {
      SendFromDevice (m_passThroughTo, p, PfcHeader::PROT_NUM, m_passThroughTo->GetAddress (),
                      m_passThroughTo->GetRemote ());
      return;
    }

  Ptr<DpskNetDevice> inDev = DynamicCast<DpskNetDevice> (device);
  Ptr<DpskNetDevice> outDev = DynamicCast<DpskNetDevice> (GetOutDev (p));
  if (outDev == 0)
    return; // Drop packet

  NS_ASSERT_MSG (outDev->IsLinkUp (), "The routing table look up should return link that is up");

  Ipv4Header ipHeader;
  p->PeekHeader (ipHeader);

  uint32_t pSize = packet->GetSize ();
  auto dscp = ipHeader.GetDscp ();
  uint32_t qIndex = (dscp >= m_nQueues) ? m_nQueues : dscp;

  if (qIndex != m_nQueues) // not control queue
    {
      // Check enqueue admission
      if (m_mmu->CheckIngressAdmission (inDev, dscp, pSize) &&
          m_mmu->CheckEgressAdmission (outDev, dscp, pSize))
        {
          // Enqueue
          m_mmu->UpdateIngressAdmission (inDev, dscp, pSize);
          m_mmu->UpdateEgressAdmission (outDev, dscp, pSize);
        }
      else
        {
          m_nIngressDropPacket[inDev]++;
          return; // Drop packet
        }

      const auto inDevType = DeviceToL2Type (inDev);
      if (inDevType == PFC)
        {
          // Check and send PFC
          if (m_mmu->CheckShouldSendPfcPause (inDev, qIndex))
            {
              m_mmu->SetPause (inDev, qIndex);

              PfcHeader pfcHeader (PfcHeader::PfcType::Pause, qIndex);
              Ptr<Packet> pfc = Create<Packet> (0);
              pfc->AddHeader (pfcHeader);

              SendFromDevice (inDev, pfc, PfcHeader::PROT_NUM, inDev->GetAddress (),
                              inDev->GetRemote ());
            }
        }
    }

  SendFromDevice (outDev, p, protocol, outDev->GetAddress (), outDev->GetRemote ());
}

void
PfcSwitch::InstallDpsk (Ptr<Dpsk> dpsk)
{
  NS_LOG_FUNCTION (dpsk);

  m_dpsk = dpsk;
  m_dpsk->RegisterReceiveFromDeviceHandler (MakeCallback (&PfcSwitch::ReceiveFromDevice, this));

  const auto &devices = m_dpsk->GetDevices ();
  m_nDevices = devices.size ();

  for (const auto &dev : devices)
    {
      const auto dpskDev = DynamicCast<DpskNetDevice> (dev);
      m_devices.insert ({dev->GetIfIndex (), dpskDev});
      const auto type = DeviceToL2Type (dpskDev);
      if (type == PFC)
        {
          const auto pfcPortImpl = dpskDev->GetObject<PfcSwitchPort> ();
          pfcPortImpl->SetDeviceDequeueHandler (
              MakeCallback (&PfcSwitch::DeviceDequeueHandler, this));
          // PFC pass through source device register (only one)
          if (pfcPortImpl->IsPassThrough ())
            {
              m_passThroughFrom = DynamicCast<DpskNetDevice> (dev);
            }
        }
      // else if (type == CBFC)
      //   {
      //     const auto cbfcPortImpl = dpskDev->GetObject<CbfcSwitchPort> ();
      //     cbfcPortImpl->SetDeviceDequeueHandler (
      //         MakeCallback (&PfcSwitch::DeviceDequeueHandler, this));
      //   }
      // else if (type == CBPFC)
      //   {
      //     const auto cbpfcPortImpl = dpskDev->GetObject<CbpfcSwitchPort> ();
      //     cbpfcPortImpl->SetDeviceDequeueHandler (
      //         MakeCallback (&PfcSwitch::DeviceDequeueHandler, this));
      //   }
      // else if (type == PTPFC)
      //   {
      //     const auto ptpfcPortImpl = dpskDev->GetObject<PtpfcSwitchPort> ();
      //     ptpfcPortImpl->SetDeviceDequeueHandler (
      //         MakeCallback (&PfcSwitch::DeviceDequeueHandler, this));
      //     // PFC pass through destination device register (only one)
      //     m_passThroughTo = DynamicCast<DpskNetDevice> (dev);
      //   }
      // else if (type == NOPFC)
      //   {
      //     const auto noPfcPortImpl = dpskDev->GetObject<NoPfcSwitchPort> ();
      //     noPfcPortImpl->SetDeviceDequeueHandler (
      //         MakeCallback (&PfcSwitch::DeviceDequeueHandler, this));
      //   }
    }

  AggregateObject (m_dpsk->GetNode ());
}

void
PfcSwitch::InstallMmu (Ptr<SwitchMmu> mmu)
{
  NS_LOG_FUNCTION (mmu);

  m_mmu = mmu;
  m_mmu->ConfigNQueue (m_nQueues);
  for (const auto &entry : m_devices)
    {
      const auto &dev = entry.second;
      m_mmu->AggregateDevice (dev);
    }

  AggregateObject (mmu);
}

Ptr<SwitchMmu>
PfcSwitch::GetMmu (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mmu;
}

void
PfcSwitch::SetEcmpSeed (uint32_t s)
{
  NS_LOG_FUNCTION (s);
  m_ecmpSeed = s;
}

void
PfcSwitch::SetNQueues (uint32_t n)
{
  NS_LOG_FUNCTION (n);
  m_nQueues = n;
}

void
PfcSwitch::ConfigPortHeadroom (const Ptr<DpskNetDevice> port, uint32_t qIndex, uint64_t headroom)
{
  m_mmu->ConfigHeadroom(port, qIndex, headroom);
}

void
PfcSwitch::AddRouteTableEntry (const Ipv4Address &dest, Ptr<DpskNetDevice> dev)
{
  NS_LOG_FUNCTION (dest << dev);

  bool find = false;
  for (const auto &entry : m_devices)
    {
      if (entry.second == dev)
        {
          find = true;
          break;
        }
    }
  NS_ASSERT_MSG (find, "PfcSwitch::AddRouteTableEntry: No such device");

  uint32_t destVal = dest.Get ();
  m_routeTable[destVal].push_back (dev);
}

std::unordered_map<uint32_t, std::vector<Ptr<DpskNetDevice>>>
PfcSwitch::GetRouteTable ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_routeTable;
}

void
PfcSwitch::ClearRouteTable ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_routeTable.clear ();
}

Ptr<DpskNetDevice>
PfcSwitch::GetOutDev (Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (p);

  Ptr<Packet> packet = p->Copy ();
  // Reveal IP header
  Ipv4Header ipHeader;
  packet->RemoveHeader (ipHeader);
  uint32_t srcAddr = ipHeader.GetSource ().Get ();
  uint32_t destAddr = ipHeader.GetDestination ().Get ();
  uint8_t protocol = ipHeader.GetProtocol ();

  auto entry = m_routeTable.find (destAddr);
  if (entry == m_routeTable.end ())
    return 0;
  auto &nextHops = entry->second;

  union {
    uint8_t u8[12] = {0};
    uint32_t u32[3];
  } ecmpKey;
  size_t ecmpKeyLen = 0;

  // Assemble IP to ECMP key
  ecmpKey.u32[0] = srcAddr;
  ecmpKey.u32[1] = destAddr;
  ecmpKeyLen += 2;

  if (protocol == 0x11) // UDP
    {
      // Reveal UDP header
      QbbHeader qbbHeader;
      packet->PeekHeader (qbbHeader);
      uint32_t srcPort = qbbHeader.GetSourcePort ();
      uint32_t destPort = qbbHeader.GetDestinationPort ();
      // Assemble UDP to ECMP key
      ecmpKey.u32[2] = srcPort | (destPort << 16);
      ecmpKeyLen++;
    }
  else
    {
      // Unexpected
      NS_ASSERT_MSG (false, "PfcSwitch::GetOutDev: Unexpected IP protocol number");
      return 0;
    }

  uint32_t hashVal = CalcTime31Hash (ecmpKey.u8, ecmpKeyLen * 4) % nextHops.size ();
  return nextHops[hashVal];
}

uint32_t
PfcSwitch::CalcTime31Hash (const uint8_t *key, size_t len)
{
  NS_LOG_FUNCTION (key << len);

  uint32_t h = m_ecmpSeed;
  for (size_t i = 0; i < len; i++)
    {
      h = 31 * h + key[i];
    }
  return h;
}

uint32_t
PfcSwitch::CalcEcmpHash (const uint8_t *key, size_t len)
{
  NS_LOG_FUNCTION (key << len);

  uint32_t h = m_ecmpSeed;
  if (len > 3)
    {
      const uint32_t *key_x4 = (const uint32_t *) key;
      size_t i = len >> 2;
      do
        {
          uint32_t k = *key_x4++;
          k *= 0xcc9e2d51;
          k = (k << 15) | (k >> 17);
          k *= 0x1b873593;
          h ^= k;
          h = (h << 13) | (h >> 19);
          h += (h << 2) + 0xe6546b64;
      } while (--i);
      key = (const uint8_t *) key_x4;
    }
  if (len & 3)
    {
      size_t i = len & 3;
      uint32_t k = 0;
      key = &key[i - 1];
      do
        {
          k <<= 8;
          k |= *key--;
      } while (--i);
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
    }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

void
PfcSwitch::DeviceDequeueHandler (Ptr<NetDevice> outDev, Ptr<Packet> packet, uint32_t qIndex)
{
  NS_LOG_FUNCTION (outDev << packet << qIndex);

  PfcSwitchTag tag;
  packet->PeekPacketTag (tag);

  Ptr<DpskNetDevice> inDev = DynamicCast<DpskNetDevice> (m_devices[tag.GetInDevIdx ()]);
  uint32_t pSize = packet->GetSize ();

  if (qIndex == m_nQueues) // control queue
    return;
  // Update ingress and egress
  m_mmu->RemoveFromIngressAdmission (inDev, qIndex, pSize);
  m_mmu->RemoveFromEgressAdmission (outDev, qIndex, pSize);
  // ECN
  bool congested = m_mmu->CheckShouldSetEcn (outDev, qIndex);
  if (congested)
    {
      EthernetHeader ethHeader;
      Ipv4Header ipHeader;
      packet->RemoveHeader (ethHeader);
      packet->RemoveHeader (ipHeader);
      ipHeader.SetEcn (Ipv4Header::EcnType::ECN_CE);
      packet->AddHeader (ipHeader);
      packet->AddHeader (ethHeader);
    }

  const auto inDevType = DeviceToL2Type (inDev);
  if (inDevType == PFC)
    {
      // Check and send resume
      if (m_mmu->CheckShouldSendPfcResume (inDev, qIndex))
        {
          m_mmu->SetResume (inDev, qIndex);

          PfcHeader PfcHeader (PfcHeader::PfcType::Resume, qIndex);
          Ptr<Packet> pfc = Create<Packet> (0);
          pfc->AddHeader (PfcHeader);

          SendFromDevice (inDev, pfc, PfcHeader::PROT_NUM, inDev->GetAddress (),
                          inDev->GetRemote ());
        }
    }
}

// void
// PfcSwitch::InitSendCbfcFeedback ()
// {
//   NS_LOG_FUNCTION_NOARGS ();
//   for (const auto &entry : m_devices)
//     {
//       const auto dev = entry.second;
//       const auto type = DeviceToL2Type (dev);
//       if (type == CBFC)
//         {
//           for (uint32_t i = 0; i < m_nQueues; ++i)
//             {
//               const auto period = m_mmu->GetCbfcFeedbackPeroid (dev, i);
//               Simulator::ScheduleNow (&PfcSwitch::SendCbfcFeedback, this, period, dev, i);
//             }
//         }
//     }
// }

// void
// PfcSwitch::SendCbfcFeedback (Time period, Ptr<DpskNetDevice> dev, uint32_t qIndex)
// {
//   NS_LOG_FUNCTION_NOARGS ();

//   CbfcHeader cbfcHeader (m_mmu->GetCbfcFccl (dev, qIndex), qIndex);
//   Ptr<Packet> cbfc = Create<Packet> (0);
//   cbfc->AddHeader (cbfcHeader);

//   SendFromDevice (dev, cbfc, CbfcHeader::PROT_NUM, dev->GetAddress (), dev->GetRemote ());

//   Simulator::Schedule (Time (period), &PfcSwitch::SendCbfcFeedback, this, period, dev, qIndex);
// }

// void
// PfcSwitch::InitSendCbpfcFeedback ()
// {
//   NS_LOG_FUNCTION_NOARGS ();
//   for (const auto &entry : m_devices)
//     {
//       const auto dev = entry.second;
//       const auto type = DeviceToL2Type (dev);
//       if (type == CBPFC)
//         {
//           for (uint32_t i = 0; i < m_nQueues; ++i)
//             {
//               const auto period = m_mmu->GetCbpfcFeedbackPeroid (dev, i);
//               Simulator::ScheduleNow (&PfcSwitch::SendCbpfcFeedback, this, period, dev, i);
//             }
//         }
//     }
// }

// void
// PfcSwitch::SendCbpfcFeedback (Time period, Ptr<DpskNetDevice> dev, uint32_t qIndex)
// {
//   NS_LOG_FUNCTION_NOARGS ();

//   const uint64_t total_time = m_mmu->GetCbpfcFree (dev, qIndex) / 64;
//   const uint16_t time = total_time > UINT16_MAX ? UINT16_MAX : total_time;

//   if (time > 0)
//     {
//       m_mmu->AddCbpfcReserve (dev, qIndex, time);

//       PfcHeader pfcHeader (PfcHeader::Resume, qIndex, time);
//       Ptr<Packet> pfc = Create<Packet> (0);
//       pfc->AddHeader (pfcHeader);

//       SendFromDevice (dev, pfc, PfcHeader::PROT_NUM, dev->GetAddress (), dev->GetRemote ());
//     }

//   Simulator::Schedule (Time (period), &PfcSwitch::SendCbpfcFeedback, this, period, dev, qIndex);
// }

PfcSwitch::L2FlowControlType
PfcSwitch::DeviceToL2Type (Ptr<NetDevice> dev)
{
  const auto name = DynamicCast<DpskNetDevice> (dev)->GetImplementation ()->GetName ();
  if (name == "PfcSwitchPort")
    return PFC;
  else if (name == "CbfcSwitchPort")
    return CBFC;
  else if (name == "CbpfcSwitchPort")
    return CBPFC;
  else if (name == "PtpfcSwitchPort")
    return PTPFC;
  else if (name == "NoPfcSwitchPort")
    return NOPFC;
  else
    return UNKNOWN;
}

void
PfcSwitch::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_devices.clear ();
  m_routeTable.clear ();
  m_mmu = 0;
  DpskLayer::DoDispose ();
}

} // namespace ns3
