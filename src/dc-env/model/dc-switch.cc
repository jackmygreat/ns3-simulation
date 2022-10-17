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
 * Author: Pavinberg <pavin0702@gmail.com>
 */

#include "dc-switch.h"
#include "ns3/dpsk-net-device.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/type-id.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcSwitch");

NS_OBJECT_ENSURE_REGISTERED (DcSwitch);

TypeId
DcSwitch::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DcSwitch")
                          .SetParent<Node> ()
                          .SetGroupName ("DpskMachine")
                          .AddConstructor<DcSwitch> ();
  return tid;
}

DcSwitch::DcSwitch ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

DcSwitch::~DcSwitch ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool
DcSwitch::SendFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                          const Address &source, const Address &destination)
{
  return false;
}

bool
DcSwitch::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                             const Address &source, const Address &destination,
                             NetDevice::PacketType packetType)
{
  return false;
}

void
DcSwitch::SetEcmpSeed (uint32_t seed)
{
  NS_LOG_FUNCTION (seed);
  m_seed = seed;
}

void
DcSwitch::SetNQueues (uint32_t nQueues)
{
  NS_LOG_FUNCTION (nQueues);
  m_nQueues = nQueues;
}

// void
// DcSwitch::InstallMmu (Ptr<SwitchMmu> mmu)
// {
//   NS_LOG_FUNCTION (mmu);
//   m_mmu = mmu;
//   m_mmu->ConfigNQueue (m_nQueues);
//   for (int i = 0; i < this->GetNDevices (); i++)
//     {
//       // m_mmu->AggregateDevice (this->GetDevice(i));
//     }
// }

} // namespace ns3
