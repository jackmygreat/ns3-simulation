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

#include "dpsk-machine-list.h"
#include "ns3/simulator.h"


namespace ns3 {

std::vector<Ptr<DpskMachine>> DpskMachineList::m_machines;

NS_LOG_COMPONENT_DEFINE ("DpskMachineList");

uint32_t
DpskMachineList::Add (Ptr<DpskMachine> machine)
{
  NS_LOG_FUNCTION (machine);
  uint32_t index = m_machines.size ();
  m_machines.push_back (machine);
  Simulator::ScheduleWithContext (index, TimeStep (0), &DpskMachine::Initialize, machine);
  return index;
}

DpskMachineList::Iterator
DpskMachineList::Begin ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_machines.begin ();
}

DpskMachineList::Iterator
DpskMachineList::End ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_machines.end ();
}

Ptr<DpskMachine>
DpskMachineList::GetMachine (uint32_t n)
{
  NS_LOG_FUNCTION (n);
  NS_ASSERT_MSG (n < m_machines.size (), "Machine index " << n << " is out of range (only have "
                                                          << m_machines.size () << " machines).");
  return m_machines[n];
}

uint32_t
DpskMachineList::GetNMachines ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_machines.size ();
}

} // namespace ns3
