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

#include "dc-topology.h"
#include "ns3/dpsk-net-device.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/object-base.h"
#include "ns3/object.h"
#include "ns3/topology.pb.h"
#include "ns3/type-id.h"
#include <_types/_uint32_t.h>

/**
 * \file
 * \ingroup protobuf-topology
 * ns3::DcTopology implementation
 */
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcTopology");

NS_OBJECT_ENSURE_REGISTERED (DcTopology);

TypeId
DcTopology::GetTypeId ()
{
  static TypeId tid =
      TypeId ("ns3::DcTopology").SetGroupName ("ProtobufTopology").AddConstructor<DcTopology> ();
  return tid;
}

DcTopology::DcTopology (uint32_t nodeNum)
{
  NS_LOG_FUNCTION (this);
  m_nodes.resize (nodeNum);
  m_ipAddrHelper.SetBase ("10.0.0.0", "255.0.0.0");
}

DcTopology::~DcTopology ()
{
  NS_LOG_FUNCTION (this);
}

void
DcTopology::InstallNodes (ns3_proto::AllNodes nodes)
{
  // InstallHosts (nodes.hostgroups ());
  // InstallSwitches (nodes.switchgroups ());
}

void
DcTopology::InstallNode (uint32_t index, TopoNode node)
{
  m_nodes[index] = node;
}
/**
 * How to configure a host with the configuration of the HostGroup
 */
// DcTopology::TopoNode
// DcTopology::ConfigHost (ns3_proto::HostGroup hostGroup)
// {
//   DcTopology::TopoNode topoNode = {
//     .type = DcTopology::TopoNode::NodeType::HOST,
//     .nodePtr = CreateObject<Node> ()
//   };
//   for (auto port: hostGroup.ports())
//     {
//       const Ptr<DpskNetDevice> dev = CreateObject<DpskNetDevice> ();
//       dev->SetAddress (Mac48Address::Allocate ());
//       dev->SetTxMode (DpskNetDevice::TxMode::ACTIVE);
//       topoNode.nodePtr->AddDevice (dev);
//       const Ptr<PfcHostPort> impl = CreateObject<PfcHostPort> ();
//       dev->SetImplementation (impl);
//       impl->SetupQueues(1); // TODO: support more queues
//       impl->EnablePfc(port.pfcenabled());

//     }

// }

} // namespace ns3
