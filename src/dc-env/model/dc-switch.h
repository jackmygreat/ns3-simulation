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

#ifndef DC_SWITCH_H
#define DC_SWITCH_H

#include <stdint.h>
#include "ns3/node.h" 
#include "ns3/net-device.h"
#include <map>


namespace ns3 {

class DcSwitch : public Node
{
public:
  static TypeId GetTypeId (void);

  DcSwitch ();
  ~DcSwitch ();

  /**
   * \brief Sends a packet from one device.
   * \param device the originating port (if NULL then broadcasts to all ports)
   * \param packet the sended packet
   * \param protocol the packet protocol (e.g., Ethertype)
   * \param source the packet source
   * \param destination the packet destination
   */
  virtual bool SendFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                               const Address &source, const Address &destination);
  /**
   * \brief Receives a packet from one device.
   * \param device the originating port
   * \param packet the received packet
   * \param protocol the packet protocol (e.g., Ethertype)
   * \param source the packet source
   * \param destination the packet destination
   * \param packetType the packet type (e.g., host, broadcast, etc.)
   */
  virtual bool ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet,
                                  uint16_t protocol, const Address &source,
                                  const Address &destination, NetDevice::PacketType packetType);

  void SetEcmpSeed (uint32_t seed);

  void SetNQueues (uint32_t nQueues);

protected:

  uint32_t m_seed;

  uint32_t m_nQueues; // number of queues
  
}; // class DcSwitch

} // namespace ns3

#endif
