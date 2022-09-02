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

#include "dpsk-machine.h"
#include "ns3/object-base.h"
#include "ns3/log.h"
#include "dpsk-net-device.h"
#include "dpsk-machine-list.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DpskMachine");

NS_OBJECT_ENSURE_REGISTERED (DpskMachine);

TypeId
DpskMachine::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DpskMachine")
                          .SetParent<Object> ()
                          .SetGroupName ("Dpsk")
                          .AddConstructor<DpskMachine> ()
                          .AddAttribute ("id", "The id (unique integer) of this machine.",
                                         TypeId::ATTR_GET, // allow only getting it.
                                         UintegerValue (0), MakeUintegerAccessor (&DpskMachine::m_id),
                                         MakeUintegerChecker<uint32_t> ());
  return tid;
}

DpskMachine::DpskMachine ()
{
  NS_LOG_FUNCTION (this);
  m_id = DpskMachineList::Add (this);
}

void
DpskMachine::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  for (std::vector<Ptr<DpskNetDevice>>::iterator i = m_devices.begin (); i != m_devices.end (); i++)
    {
      (*i)->Initialize ();
    }

  Object::DoInitialize ();
}

uint32_t
DpskMachine::AddDevice (Ptr<DpskNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  uint32_t index = m_devices.size ();
  m_devices.push_back (device);
  device->SetMachine (this);
  device->SetIfIndex (index);
  // device->SetReceiveCallback (MakeCallback (&Node::NonPromiscReceiveFromDevice, this));
  Simulator::ScheduleWithContext (this->GetId (), Seconds (0.0), &DpskNetDevice::Initialize,
                                  device);
  return index;
}

uint32_t
DpskMachine::GetId () const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

} // namespace ns3
