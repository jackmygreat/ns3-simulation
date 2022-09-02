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

#ifndef DC_MACHINE_LIST_H
#define DC_MACHINE_LIST_H

#include "dpsk-machine.h"

namespace ns3 {

/**
 * \ingroup dc-machine
 * \brief the list of DpskMachines as a replacement of NodeList
 */
class DpskMachineList
{
public:
  /// Node container iterator
  typedef std::vector<Ptr<DpskMachine>>::const_iterator Iterator;

  /**
   * \param machine DpskMachine to add
   * \returns index of machine in list.
   *
   * This method is called automatically from Node::Node so
   * the user has little reason to call it himself.
   */
  static uint32_t Add (Ptr<DpskMachine> machine);
  /**
   * \returns a C++ iterator located at the beginning of this
   *          list.
   */
  static Iterator Begin (void);
  /**
   * \returns a C++ iterator located at the end of this
   *          list.
   */
  static Iterator End (void);
  /**
   * \param n index of requested machine.
   * \returns the DpskMachine associated to index n.
   */
  static Ptr<DpskMachine> GetMachine (uint32_t n);
  /**
   * \returns the number of machines currently in the list.
   */
  static uint32_t GetNMachines (void);

private:

  static std::vector<Ptr<DpskMachine>> m_machines;

};

} // namespace ns3

#endif /* DC_MACHINE_LIST_H */
