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

#ifndef DC_MACHINE_H
#define DC_MACHINE_H

#include "ns3/object.h"
#include "ns3/type-id.h"
#include "dpsk-net-device.h"

namespace ns3 {

/**
 * \ingroup dc-machine
 * \brief a replacement of Node to handler DC machines with DPSK features
 */
class DpskMachine : public Object
{
public:

  static TypeId GetTypeId (void);

  DpskMachine ();

  /**
   * \returns the unique id of this machine.
   * 
   * This unique id happens to be also the index of the DcMacine into
   * the DpskMachineList. 
   */
  uint32_t GetId (void) const;

  uint32_t AddDevice (Ptr<DpskNetDevice> device);

  const Ptr<DpskNetDevice>& GetDevice(uint32_t index);

protected:
  
  void DoInitialize (void);

  std::vector<Ptr<DpskNetDevice>> m_devices;

private:

  uint32_t m_id;
  
};

} // namespace ns3

#endif
