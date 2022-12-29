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
#include <limits>
#include <vector>
#include "ns3/application-container.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/dc-topology.h"
#include "ns3/dcb-net-device.h"
#include "ns3/global-router-interface.h"
#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/object-factory.h"
#include "ns3/queue-disc.h"
#include "ns3/queue-size.h"
#include "ns3/topology.pb.h"
#include "ns3/traced-value.h"
#include "protobuf-topology-loader.h"
#include "ns3/dcb-fc-helper.h"
#include "ns3/dcb-pfc-port.h"

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
  std::ifstream s;
  s.open (configFile);
  if (!s)
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
Ptr<DcTopology>
ProtobufTopologyLoader::LoadTopology ()
{
  NS_LOG_FUNCTION (this);
  ns3_proto::Topology topoConfig = ReadProtoTopology ();
  Ptr<DcTopology> topology = CreateObject<DcTopology> (topoConfig.nodes ().num ());
  // DcTopology topology (topoConfig.nodes ().num ());
  m_ecmpSeed = topoConfig.globalconfig ().randomseed ();

  Ipv4AddressGenerator::Init ("10.0.0.0", "255.0.0.0", "0.0.0.1");

  LoadHosts (topoConfig.nodes ().hostgroups (), topology);
  LoadSwitches (topoConfig.nodes ().switchgroups (), topology);
  LoadLinks (topoConfig.links (), topology);
  InitGlobalRouting ();
  
  LogAllRoutes (topology); // TODO: remove me

  InstallApplications (topoConfig.applications (), topology);

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
    Ptr<DcTopology> topology)
{
  NS_LOG_FUNCTION (this);
  for (const ns3_proto::HostGroup &hostGroup : hostGroups)
    {
      uint32_t num = hostGroup.nodesnum ();
      uint32_t baseIndex = hostGroup.baseindex ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneHost (hostGroup);
          topology->InstallNode (i, std::move (host));
        }
    }
}

void
ProtobufTopologyLoader::LoadSwitches (
    const google::protobuf::RepeatedPtrField<ns3_proto::SwitchGroup> &switchGroups,
    Ptr<DcTopology> topology)
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
          topology->InstallNode (i, std::move (host));
        }
    }
}

void
ProtobufTopologyLoader::LoadLinks (
    const google::protobuf::RepeatedPtrField<ns3_proto::Link> &linksConfig,
    Ptr<DcTopology> topology)
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
  const Ptr<Node> host = CreateObject<Node> ();

  for (auto port : hostGroup.ports ())
    {
      // create a net device for the port
      const Ptr<DcbNetDevice> dev = CreateObject<DcbNetDevice> ();
      host->AddDevice (dev);
      dev->SetAddress (Mac48Address::Allocate ());

      ObjectFactory queueFactory;
      queueFactory.SetTypeId (DropTailQueue<Packet>::GetTypeId ());
      Ptr<Queue<Packet>> queue = queueFactory.Create<Queue<Packet>> ();
      queue->SetMaxSize ({QueueSizeUnit::PACKETS, std::numeric_limits<uint32_t>::max ()});
      dev->SetQueue (queue);
    }

  DcbHostStackHelper hostStack;
  hostStack.Install (host);

  for (int i = 0; i < hostGroup.ports_size (); i++)
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
  const Ptr<Node> sw = CreateObject<Node> ();
  // Basic configurations
  // sw->SetEcmpSeed (m_ecmpSeed);

  // Configure ports
  DcbSwitchStackHelper switchStack;
  switchStack.SetBufferSize (QueueSize (switchGroup.buffersize()).GetValue ());
  for (const auto &portConfig : switchGroup.ports ())
    {
      AddPortToSwitch (portConfig, sw, switchStack);
    }
  switchStack.Install (sw);

  for (int i = 0; i < switchGroup.ports_size (); i++)
    {
      const ns3_proto::SwitchPortConfig &portConfig = switchGroup.ports (i);
      
      if (portConfig.queues_size () != 0 && portConfig.queues_size () != 8)
        {
          NS_FATAL_ERROR ("The port configuration should have 8 queues or 0 queue, not " << portConfig.queues_size());
        }

      if (portConfig.pfcenabled ()) // Configure PFC
        {
          DcbPfcPortConfig pfcConfig;
          for (int qi = 0; qi < portConfig.queues_size (); qi++)
            {
              const ns3_proto::PortQueueConfig &queueConfig = portConfig.queues (qi);
              const uint32_t reserve = QueueSize (queueConfig.pfcreserve ()).GetValue ();
              const uint32_t xon = QueueSize (queueConfig.pfcxon ()).GetValue ();
              pfcConfig.AddQueueConfig (qi, reserve, xon);
            }
          DcbFcHelper::InstallPFCtoNodePort (sw, i, pfcConfig);
        }
      AssignAddress (sw, sw->GetDevice (i));

      if (portConfig.ecnenabled()) // Configure ECN
        {
          ObjectFactory factory;
          factory.SetTypeId ("ns3::FifoQueueDiscEcn");
          Ptr<QueueDisc> dev = DynamicCast<DcbNetDevice> (sw->GetDevice (i))->GetQueueDisc ();
      
          for (int qi = 0; qi < portConfig.queues_size (); qi++)
            {
              const ns3_proto::PortQueueConfig &queueConfig = portConfig.queues (qi);
              uint32_t ecnKMin = QueueSize (queueConfig.ecnkmin ()).GetValue ();
              uint32_t ecnKMax = QueueSize (queueConfig.ecnkmax ()).GetValue ();
              double ecnPMax = queueConfig.ecnpmax ();
              // ecnConfig.AddQueueConfig (qi, ecnKMin, ecnKMax, ecnPMax);

              Ptr<FifoQueueDiscEcn> qd = factory.Create<FifoQueueDiscEcn> ();
              qd->Initialize ();
              qd->ConfigECN (ecnKMin, ecnKMax, ecnPMax);
              Ptr<PausableQueueDiscClass> c = CreateObject<PausableQueueDiscClass> ();
              c->SetQueueDisc (qd);
              dev->AddQueueDiscClass (c);
            }
        }
    }

  return {.type = DcTopology::TopoNode::NodeType::SWITCH, .nodePtr = sw};
}

Ptr<DcbNetDevice>
ProtobufTopologyLoader::AddPortToSwitch (const ns3_proto::SwitchPortConfig portConfig,
                                         const Ptr<Node> sw, DcbSwitchStackHelper &switchStack)
{
  NS_LOG_FUNCTION (this);
  // Create a net device for this port
  Ptr<DcbNetDevice> dev = CreateObject<DcbNetDevice> ();
  dev->SetAddress (Mac48Address::Allocate ());

  ObjectFactory queueFactory;
  queueFactory.SetTypeId (DropTailQueue<Packet>::GetTypeId ());
  Ptr<Queue<Packet>> queue = queueFactory.Create<Queue<Packet>> ();
  queue->SetMaxSize (QueueSize ("10p"));
  dev->SetQueue (queue);

  sw->AddDevice (dev);

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
ProtobufTopologyLoader::InstallLink (const ns3_proto::Link &linkConfig, Ptr<DcTopology> topology)
{
  NS_LOG_FUNCTION (this);
  uint32_t node1 = linkConfig.node1 ();
  uint32_t node2 = linkConfig.node2 ();
  uint32_t port1 = linkConfig.port1 ();
  uint32_t port2 = linkConfig.port2 ();

  Ptr<DcbNetDevice> dev1 = StaticCast<DcbNetDevice> (topology->GetNetDeviceOfNode (node1, port1));
  Ptr<DcbNetDevice> dev2 = StaticCast<DcbNetDevice> (topology->GetNetDeviceOfNode (node2, port2));

  std::string rate = linkConfig.rate ();
  std::string delay = linkConfig.delay ();
  dev1->SetAttribute ("DataRate", DataRateValue (DataRate (rate)));
  dev2->SetAttribute ("DataRate", DataRateValue (DataRate (rate)));

  Ptr<DcbChannel> channel = CreateObject<DcbChannel> ();
  channel->SetAttribute ("Delay", TimeValue (Time (delay)));

  dev1->Attach (channel);
  dev2->Attach (channel);

  topology->InstallLink (node1, node2); // as metadata
}

void
ProtobufTopologyLoader::InitGlobalRouting ()
{
  NS_LOG_FUNCTION (this);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

std::map<std::string, TraceApplicationHelper::ProtocolGroup>
    ProtobufTopologyLoader::protocolGroupMapper = {
        {"RAW_UDP", TraceApplicationHelper::ProtocolGroup::RAW_UDP},
        {"TCP", TraceApplicationHelper::ProtocolGroup::TCP},
        {"RoCEv2", TraceApplicationHelper::ProtocolGroup::RoCEv2},
};

std::map<std::string, TraceApplication::TraceCdf *> ProtobufTopologyLoader::appCdfMapper = {
    {"WebSearch", &TraceApplication::TRACE_WEBSEARCH_CDF},
    {"FdHadoop", &TraceApplication::TRACE_FDHADOOP_CDF}};

void
ProtobufTopologyLoader::InstallApplications (
    const google::protobuf::RepeatedPtrField<ns3_proto::Application> &appsConfig,
    Ptr<DcTopology> topology)
{
  NS_LOG_FUNCTION (this);

  TraceApplicationHelper appHelper (topology);
  for (const auto &appConfig : appsConfig)
    {
      { // set protocol group
        auto p = protocolGroupMapper.find (appConfig.protocolgroup ());
        if (p == protocolGroupMapper.end ())
          {
            NS_FATAL_ERROR ("Cannot recognize protocol group \"" << appConfig.protocolgroup ()
                                                                 << "\"");
          }
        appHelper.SetProtocolGroup (p->second);
      }

      { // set CDF
        auto p = appCdfMapper.find (appConfig.cdf ());
        if (p == appCdfMapper.end ())
          {
            NS_FATAL_ERROR ("Cannot recognize CDF \"" << appConfig.cdf () << "\".");
          }
        appHelper.SetCdf (*(p->second));
      }

      if (appConfig.has_dest ())
        {
          appHelper.SetDestination(appConfig.dest ());
        }

      for (const auto &nodeI : appConfig.nodeindices ())
        {
          if (!topology->IsHost (nodeI))
            {
              NS_FATAL_ERROR (
                  "Node " << nodeI << " is not a host and thus could not install an application.");
            }
          Ptr<Node> node = topology->GetNode (nodeI).nodePtr;
          appHelper.SetLoad (DynamicCast<DcbNetDevice> (node->GetDevice (0)), appConfig.load ());
          ApplicationContainer app = appHelper.Install (node);
          app.Start (MicroSeconds (appConfig.starttime ()));
          app.Stop (MicroSeconds (appConfig.stoptime ()));
        }
    }
}

void
ProtobufTopologyLoader::LogIpAddress (const Ptr<const DcTopology> topology) const
{
  NS_LOG_FUNCTION (this);
  int ni = 0;
  for (const auto &node : *topology)
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
ProtobufTopologyLoader::LogAllRoutes (const Ptr<const DcTopology> topology) const
{
  NS_LOG_FUNCTION (this);
  int ni = 0;
  for (const auto &node : *topology)
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
ProtobufTopologyLoader::LogGlobalRouting (const Ptr<DcTopology> topology) const
{
  for (DcTopology::SwitchIterator sw = topology->switches_begin (); sw != topology->switches_end ();
       sw++)
    {
      Ptr<Ipv4ListRouting> lrouting =
          DynamicCast<Ipv4ListRouting> ((*sw)->GetObject<Ipv4> ()->GetRoutingProtocol ());
      int16_t prio;
      Ptr<Ipv4GlobalRouting> glb =
          DynamicCast<Ipv4GlobalRouting> (lrouting->GetRoutingProtocol (0, prio));
      uint32_t n = glb->GetNRoutes ();
      for (uint32_t i = 0; i < n; i++)
        {
          NS_LOG_DEBUG ("global: " << *glb->GetRoute (i));
        }
      BooleanValue b;
      glb->GetAttribute ("RandomEcmpRouting", b);
      NS_LOG_DEBUG ("ecmp: " << b);
    }
}

} // namespace ns3
