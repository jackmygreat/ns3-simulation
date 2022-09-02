/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Universita' di Firenze, Italy
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
 * Author: Pavinberg (pavin0702@gmail.com)
 */

#ifndef DC_TOPOLOGY_H
#define DC_TOPOLOGY_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/dpsk-module.h"
#include "ns3/pfc-module.h"
#include "ns3/topology.pb.h"

#include <vector>

/**
 * \file
 * \ingroup protobuf-topology
 * ns3::DcTopology declaration
 */
namespace ns3 {

class DcNode : public Object
{
private:
  const Ptr<Node> m_node;
}; // class DcNode

  class HostNode : DcNode
{
public:
  class HostPort
  {
    
  }; // class HostPort
  
private:
  const Ptr<DpskNetDevice> m_dev;

}; // class Host

class DcTopology : public Object
{
public:
  /**
	 * \brief Get the type ID.
	 * \return the object TypeId.
	 */
  static TypeId GetTypeId (void);

  DcTopology () = default;
  DcTopology (uint32_t nodeNum);
  virtual ~DcTopology ();

  struct TopoNode
  {
    enum NodeType { HOST, SWITCH };
    NodeType type;
    Ptr<DpskMachine> nodePtr;
  };

  void InstallNodes (ns3_proto::AllNodes nodes);

  void InstallNode (uint32_t index, TopoNode node);

private:

  std::vector<TopoNode> m_nodes;
  Ipv4AddressHelper m_ipAddrHelper;

}; // class DcTopology

} // namespace ns3

#endif
