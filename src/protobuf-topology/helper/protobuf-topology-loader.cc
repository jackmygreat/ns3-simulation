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

#include <fstream>
#include "ns3/dc-topology.h"
#include "ns3/pfc-module.h"
#include "ns3/dpsk-module.h"
#include "protobuf-topology-loader.h"

/**
 * \file
 * \ingroup protobuf-topology
 * ns3::ProtobufTopologyReader implementation.
 */

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ProtobufTopologyReader");

ProtobufTopologyLoader::ProtobufTopologyLoader ()
{
}

void
ProtobufTopologyLoader::RunConfigScript (std::string configFile)
{
  if (configFile == "")
    {
      NS_FATAL_ERROR ("config file name passed to ProtobufTopologyLoader is an empty string.");
    }
  // Simply execute it with the default Python3 interpreter.
  std::system (("python3 " + configFile).c_str ());
}

DcTopology *
ProtobufTopologyLoader::LoadTopology ()
{
  ns3_proto::Topology topoConfig = ReadProtoTopology ();
  DcTopology topology (topoConfig.nodes ().num ());
  LoadHosts (topoConfig.nodes ().hostgroups (), topology);
  LoadSwitches (topoConfig.nodes ().switchgroups(), topology);
  return nullptr;
}

ns3_proto::Topology
ProtobufTopologyLoader::ReadProtoTopology ()
{
  std::fstream input (m_protoBinaryName, std::ios::in | std::ios::binary);
  if (!input)
    {
      NS_FATAL_ERROR ("cannot find file "
                      << m_protoBinaryName
                      << " which should be created by the Python script with Protobuf");
    }

  ns3_proto::Topology topology;
  if (!topology.ParseFromIstream (&input))
    {
      NS_FATAL_ERROR ("cannot parse binary file " << m_protoBinaryName
                                                  << " which should be a Protobuf serialized file");
    }
  // NS_LOG_INFO ("Topo links num: " << topology.links_size ()
  //                                 << "\nHost group num: " << topology.nodes ().hostgroups_size ());
  return topology;
}

void
ProtobufTopologyLoader::LoadHosts (
    google::protobuf::RepeatedPtrField<ns3_proto::HostGroup> hostGroups, DcTopology &topology)
{
  for (ns3_proto::HostGroup hostGroup : hostGroups)
    {
      uint32_t num = hostGroup.nodesnum ();
      uint32_t baseIndex = hostGroup.baseindex ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneHost (hostGroup.ports ());
          topology.InstallNode (i, host);
        }
    }
}

void
ProtobufTopologyLoader::LoadSwitches (
    google::protobuf::RepeatedPtrField<ns3_proto::SwitchGroup> switchGroups, DcTopology &topology)
{
  for (ns3_proto::SwitchGroup switchGroup : switchGroups)
    {
      uint32_t num = switchGroup.nodesnum ();
      uint32_t baseIndex = switchGroup.baseindex ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneSwitch (switchGroup.ports ());
          topology.InstallNode (i, host);
        }
    }
}

DcTopology::TopoNode
ProtobufTopologyLoader::CreateOneHost (
    google::protobuf::RepeatedPtrField<ns3_proto::HostPortConfig> portsConfig)
{
  // TODO: optimize code here
  DcTopology::TopoNode node = {.type = DcTopology::TopoNode::NodeType::HOST,
                               .nodePtr = CreateObject<Node> ()};
  for (auto port : portsConfig)
    {
      // create a net device for the port
      const Ptr<DpskNetDevice> dev = CreateObject<DpskNetDevice> ();
      node.nodePtr->AddDevice (dev);
      dev->SetAddress (Mac48Address::Allocate ());
      dev->SetTxMode (DpskNetDevice::TxMode::ACTIVE);

      // Add an implementation for the net device
      const Ptr<PfcHostPort> impl = CreateObject<PfcHostPort> ();
      dev->SetImplementation (impl);
      impl->SetupQueues (1); // TODO: support more queues
      impl->EnablePfc (port.pfcenabled ());
      // TODO: other configurations
      // ...
    }
  DpskHelper dpskHelper;
  const auto dpsk = dpskHelper.Install (node.nodePtr);
  // TODO: Why so complicated?
  auto pfcHost = CreateObject<PfcHost> ();
  pfcHost->InstallDpsk (dpsk);
  
  return node;
}

DcTopology::TopoNode
ProtobufTopologyLoader::CreateOneSwitch (
    google::protobuf::RepeatedPtrField<ns3_proto::SwitchPortConfig> portsConfig)
{
  DcTopology::TopoNode node = {.type = DcTopology::TopoNode::NodeType::HOST,
                               .nodePtr = CreateObject<Node> ()};
  for (auto portConfig : portsConfig)
    {
      AddOnePortToSwitch(portConfig, node);
    }
  DpskHelper dpskHelper;
  const auto dpsk = dpskHelper.Install (node.nodePtr);
  const auto pfcSwitch = CreateObject<PfcSwitch> ();
  
  return node;
}

void
ProtobufTopologyLoader::AddOnePortToSwitch (ns3_proto::SwitchPortConfig portConfig, DcTopology::TopoNode &sw)
{
  // Create a net device for this port
  const Ptr<DpskNetDevice> dev = CreateObject<DpskNetDevice> ();
  sw.nodePtr->AddDevice (dev);
  dev->SetAddress (Mac48Address::Allocate ());
  dev->SetTxMode (DpskNetDevice::TxMode::ACTIVE); // TODO: configurable mode
  
  // Add an implementation for the net device
  // TODO: improve the logic, maybe with more dynamic features
  if (portConfig.pfcenabled())
    {
      const Ptr<PfcSwitchPort> impl = CreateObject<PfcSwitchPort> ();
      dev->SetImplementation (impl);
      impl->SetupQueues (portConfig.queuenum ());
      
      if (portConfig.has_pfcpassthrough())
        {
          impl->SetPassThrough(portConfig.pfcpassthrough ());
        }
    }
  
}

} // namespace ns3
