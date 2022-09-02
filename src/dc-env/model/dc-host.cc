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

#include "dc-host.h"
#include "ns3/dpsk-net-device.h"
#include "ns3/log.h"
#include "ns3/object-base.h"
#include "ns3/object.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcHost");

NS_OBJECT_ENSURE_REGISTERED (DcHost);

TypeId
DcHost::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DcHost")
                          .SetParent<DpskMachine> ()
                          .SetGroupName ("DcMachine")
                          .AddConstructor<DcHost> ();
  return tid;
}

DcHost::DcHost ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

DcHost::~DcHost ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

// bool
// DcHost::NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet,
//                                     uint16_t protocol, const Address &from)
// {
//   NS_LOG_FUNCTION (this << device << packet << protocol << &from);
//   Ptr<DpskNetDevice> dpskDev = DynamicCast<DpskNetDevice> (device);
//   return ReceiveFromDevice (dpskDev, packet, protocol, from, device->GetAddress (), NetDevice::PacketType (0));
// }


bool
DcHost::SendFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                               const Address &source, const Address &destination)
{
  return false;
}

bool
DcHost::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet,
                   uint16_t protocol, const Address &source,
                   const Address &destination, NetDevice::PacketType packetType)
{
  return false;
}

} // namespace ns3
