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

// #include <_types/_uint32_t.h>
#include <_types/_uint32_t.h>
#include <fstream>
#include <vector>
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/dc-topology.h"
#include "ns3/global-router-interface.h"
#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/object-factory.h"
#include "ns3/queue-size.h"
#include "ns3/topology.pb.h"
#include "ns3/traced-value.h"
#include "protobuf-topology-loader.h"
#include "ns3/dcb-net-device.h"
#include "ns3/dcb-channel.h"
#include "ns3/dcb-stack-helper.h"
#include "ns3/dcb-switch-stack-helper.h"

/**
 * \file
 * \ingroup protobuf-loader
 * ns3::ProtobufTopologyLoader implementation.
 */

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ProtobufTopologyLoader");

ProtobufTopologyLoader::ProtobufTopologyLoader ()
{
}

void
ProtobufTopologyLoader::RunConfigScript (std::string configFile)
{
  if (!std::filesystem::exists (configFile))
    {
      NS_FATAL_ERROR ("Python config file '" << configFile << "' does not exist");
    }
  // Simply execute it with the default Python3 interpreter.
  std::system (("python3 " + configFile).c_str ());
}

/**
   * \ingroup protobuf-loader
   * This is the function for user to call to load a topology from the Protobuf
   */
DcTopology
ProtobufTopologyLoader::LoadTopology ()
{
  NS_LOG_FUNCTION (this);
  ns3_proto::Topology topoConfig = ReadProtoTopology ();
  DcTopology topology (topoConfig.nodes ().num ());
  m_ecmpSeed = topoConfig.globalconfig ().randomseed ();

  Ipv4AddressGenerator::Init ("10.0.0.0", "255.0.0.0", "0.0.0.1");

  LoadHosts (topoConfig.nodes ().hostgroups (), topology);
  LoadSwitches (topoConfig.nodes ().switchgroups (), topology);
  LoadLinks (topoConfig.links (), topology);
  // AssignAddresses (topology);
  InitGlobalRouting ();

  // LogIpAddress (topology);

  return topology;
}

ns3_proto::Topology
ProtobufTopologyLoader::ReadProtoTopology ()
{
  NS_LOG_FUNCTION (this);
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
  return topology;
}

void
ProtobufTopologyLoader::LoadHosts (
    const google::protobuf::RepeatedPtrField<ns3_proto::HostGroup> &hostGroups,
    DcTopology &topology)
{
  NS_LOG_FUNCTION (this);
  for (const ns3_proto::HostGroup &hostGroup : hostGroups)
    {
      uint32_t num = hostGroup.nodesnum ();
      uint32_t baseIndex = hostGroup.baseindex ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneHost (hostGroup);
          topology.InstallNode (i, std::move (host));
        }
    }
}

void
ProtobufTopologyLoader::LoadSwitches (
    const google::protobuf::RepeatedPtrField<ns3_proto::SwitchGroup> &switchGroups,
    DcTopology &topology)
{
  NS_LOG_FUNCTION (this);
  for (const ns3_proto::SwitchGroup &switchGroup : switchGroups)
    {
      const uint32_t num = switchGroup.nodesnum ();
      const uint32_t baseIndex = switchGroup.baseindex ();
      const uint32_t queueNum = switchGroup.queuenum ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneSwitch (queueNum, switchGroup);
          topology.InstallNode (i, std::move (host));
        }
    }
}

void
ProtobufTopologyLoader::LoadLinks (
    const google::protobuf::RepeatedPtrField<ns3_proto::Link> &linksConfig, DcTopology &topology)
{
  NS_LOG_FUNCTION (this);
  for (const ns3_proto::Link &linkConfig : linksConfig)
    {
      InstallLink (linkConfig, topology);
    }
}

DcTopology::TopoNode
ProtobufTopologyLoader::CreateOneHost (const ns3_proto::HostGroup &hostGroup)
{
  NS_LOG_FUNCTION (this);
  const Ptr<DcHost> host = CreateObject<DcHost> ();
  
  for (auto port : hostGroup.ports ())
    {
      // create a net device for the port
      const Ptr<DcbNetDevice> dev = CreateObject<DcbNetDevice> ();
      host->AddDevice (dev);
      dev->SetAddress (Mac48Address::Allocate ());

      ObjectFactory queueFactory;
      queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
      Ptr<Queue<Packet>> queue = queueFactory.Create<Queue<Packet>> ();
      dev->SetQueue (queue);
    }

  DcbStackHelper hostStack;
  hostStack.Install (host);

  for (uint32_t i = 0; i < hostGroup.ports_size(); i++)
    {
      AssignAddress (host, host->GetDevice (i));
    }

  return {.type = DcTopology::TopoNode::NodeType::HOST, .nodePtr = host};
}

DcTopology::TopoNode
ProtobufTopologyLoader::CreateOneSwitch (const uint32_t queueNum,
                                         const ns3_proto::SwitchGroup &switchGroup)
{
  NS_LOG_FUNCTION (this);
  const Ptr<DcSwitch> sw = CreateObject<DcSwitch> ();
  // Basic configurations
  sw->SetEcmpSeed (m_ecmpSeed);
  sw->SetNQueues (queueNum);

  // Configure SwitchMmu
  // const Ptr<SwitchMmu> mmu = CreateObject<SwitchMmu> ();
  // sw->InstallMmu (mmu);
  // ConfigMmu (switchGroup.mmu (), mmu);

  // Configure ports
  DcbSwitchStackHelper switchStack;
  uint32_t portId = 0;
  for (auto portConfig : switchGroup.ports ())
    {
      AddPortToSwitch (portConfig, sw);
      if (portConfig.pfcenabled ()) // Configure PFC
        {
          for (int qi = 0; qi < portConfig.queues_size (); qi++)
            {
              const ns3_proto::PortQueueConfig &queueConfig = portConfig.queues (qi);
              const uint32_t reserve = QueueSize (queueConfig.pfcreserve ()).GetValue ();
              const uint32_t xon = QueueSize (queueConfig.pfcxon ()).GetValue ();
              switchStack.ConfigPfc (portId, qi, reserve, xon);
            }
        }
      portId++;
    }
  switchStack.Install (sw);

  for (uint32_t i = 0; i < switchGroup.ports_size(); i++)
    {
      AssignAddress (sw, sw->GetDevice (i));
    }

  return {.type = DcTopology::TopoNode::NodeType::SWITCH, .nodePtr = sw};
}

Ptr<DcbNetDevice>
ProtobufTopologyLoader::AddPortToSwitch (const ns3_proto::SwitchPortConfig portConfig,
                                         const Ptr<DcSwitch> sw)
{
  NS_LOG_FUNCTION (this);
  // Create a net device for this port
  Ptr<DcbNetDevice> dev = CreateObject<DcbNetDevice> ();
  sw->AddDevice (dev);
  dev->SetAddress (Mac48Address::Allocate ());

  ObjectFactory queueFactory;
  queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
  Ptr<Queue<Packet>> queue = queueFactory.Create<Queue<Packet>> ();
  dev->SetQueue (queue);

  return dev;
}

void
ProtobufTopologyLoader::AssignAddress (const Ptr<Node> node, const Ptr<NetDevice> device)
{
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ipv4, "Ipv4AddressHelper::Assign(): NetDevice is associated"
                       " with a node without IPv4 stack installed -> fail "
                       "(maybe need to use DcbStackHelper?)");

  int32_t interface = ipv4->GetInterfaceForDevice (device);
  if (interface == -1)
    {
      interface = ipv4->AddInterface (device);
    }
  NS_ASSERT_MSG (interface >= 0, "Ipv4AddressHelper::Assign(): "
                                 "Interface index not found");

  Ipv4Address addr = Ipv4AddressGenerator::NextAddress ("255.0.0.0");
  Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (addr, "255.0.0.0");
  ipv4->AddAddress (interface, ipv4Addr);
  ipv4->SetMetric (interface, 1);
  ipv4->SetUp (interface);
}

void
ProtobufTopologyLoader::InstallLink (const ns3_proto::Link &linkConfig, DcTopology &topology)
{
  NS_LOG_FUNCTION (this);
  uint32_t node1 = linkConfig.node1 ();
  uint32_t node2 = linkConfig.node2 ();
  uint32_t port1 = linkConfig.port1 ();
  uint32_t port2 = linkConfig.port2 ();
  Ptr<DcbNetDevice> dev1 = StaticCast<DcbNetDevice> (topology.GetNode (node1)->GetDevice (port1));
  Ptr<DcbNetDevice> dev2 = StaticCast<DcbNetDevice> (topology.GetNode (node2)->GetDevice (port2));

  std::string rate = linkConfig.rate ();
  std::string delay = linkConfig.delay ();
  dev1->SetAttribute ("DataRate", DataRateValue (DataRate (rate)));
  dev2->SetAttribute ("DataRate", DataRateValue (DataRate (rate)));

  Ptr<DcbChannel> channel = CreateObject<DcbChannel> ();
  channel->SetAttribute ("Delay", TimeValue (Time (delay)));

  dev1->Attach (channel);
  dev2->Attach (channel);
}

void
ProtobufTopologyLoader::AssignAddresses (DcTopology &topology)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer container;
  DcbStackHelper hostStack;
  for (DcTopology::HostIterator host = topology.hosts_begin (); host != topology.hosts_end ();
       host++)
    {
      for (int i = 0; i < (*host)->GetNDevices (); i++)
        {
          container.Add ((*host)->GetDevice (i));
        }
      // InternetStackHelper will install a LoopbackNetDevice to the node.
      // We do not add them to the `container` so that the Ipv4AddressHelper
      // won't assign a redundant address to the LoopbackNetDevice.
      hostStack.Install (host->nodePtr);
    }
  DcbSwitchStackHelper switchStack;
  for (DcTopology::SwitchIterator sw = topology.switches_begin (); sw != topology.switches_end ();
       sw++)
    {
      for (int i = 0; i < (*sw)->GetNDevices (); i++)
        {
          container.Add ((*sw)->GetDevice (i));
        }
      switchStack.Install (sw->nodePtr);
    }

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  address.Assign (container);
}

void
ProtobufTopologyLoader::InitGlobalRouting ()
{
  NS_LOG_FUNCTION (this);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

void
ProtobufTopologyLoader::LogIpAddress (const DcTopology &topology) const
{
  NS_LOG_FUNCTION (this);
  int ni = 0;
  for (const auto &node : topology)
    {
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      const int nintf = ipv4->GetNInterfaces ();
      for (int i = 0; i < nintf; i++)
        {
          const int naddr = ipv4->GetNAddresses (i);
          for (int j = 0; j < naddr; j++)
            {
              std::string name =
                  (node.type == DcTopology::TopoNode::NodeType::HOST) ? "host " : "switch ";
              NS_LOG_DEBUG (name << ni << " intf " << i << " addr " << j << " "
                                 << ipv4->GetAddress (i, j));
            }
        }
      ni++;
    }
}

void
ProtobufTopologyLoader::LogAllRoutes (const DcTopology &topology) const
{
  NS_LOG_FUNCTION (this);
  int ni = 0;
  for (const auto &node : topology)
    {
      Ptr<GlobalRouter> router = node->GetObject<GlobalRouter> ();
      Ptr<Ipv4GlobalRouting> route = router->GetRoutingProtocol ();
      std::string name = (node.type == DcTopology::TopoNode::NodeType::HOST) ? "host " : "switch ";
      if (!route)
        {
          NS_LOG_DEBUG (name << ni << " does not have global routing");
          ni++;
          continue;
        }
      const int n = route->GetNRoutes ();
      for (int i = 0; i < n; i++)
        {
          Ipv4RoutingTableEntry entry = route->GetRoute (i);
          NS_LOG_DEBUG (name << ni << " " << entry);
        }
      ni++;
    }
}

void
ProtobufTopologyLoader::LogGlobalRouting (DcTopology &topology) const
{
  for (DcTopology::SwitchIterator sw = topology.switches_begin (); sw != topology.switches_end ();
       sw++)
    {
      Ptr<Ipv4ListRouting> lrouting =
          DynamicCast<Ipv4ListRouting> ((*sw)->GetObject<Ipv4> ()->GetRoutingProtocol ());
      int16_t prio;
      Ptr<Ipv4GlobalRouting> glb =
          DynamicCast<Ipv4GlobalRouting> (lrouting->GetRoutingProtocol (0, prio));
      uint32_t n = glb->GetNRoutes ();
      for (int i = 0; i < n; i++)
        {
          NS_LOG_DEBUG ("global: " << *glb->GetRoute (i));
        }
      BooleanValue b;
      glb->GetAttribute ("RandomEcmpRouting", b);
      NS_LOG_DEBUG ("ecmp: " << b);
    }
}

} // namespace ns3
