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
#include "ns3/pfc-host-port.h"
#include "ns3/pfc-host.h"
#include "ns3/dpsk.h"
#include "ns3/dpsk-net-device.h"
#include "ns3/dpsk-helper.h"
#include "ns3/topology.pb.h"
#include "ns3/traced-value.h"
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
  m_ecmpSeed = topoConfig.globalconfig ().randomseed ();
  LoadHosts (topoConfig.nodes ().hostgroups (), topology);
  LoadSwitches (topoConfig.nodes ().switchgroups (), topology);
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
    const google::protobuf::RepeatedPtrField<ns3_proto::HostGroup> hostGroups, DcTopology &topology)
{
  for (ns3_proto::HostGroup hostGroup : hostGroups)
    {
      uint32_t num = hostGroup.nodesnum ();
      uint32_t baseIndex = hostGroup.baseindex ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneHost (hostGroup);
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
      const uint32_t num = switchGroup.nodesnum ();
      const uint32_t baseIndex = switchGroup.baseindex ();
      const uint32_t queueNum = switchGroup.queuenum ();
      for (size_t i = baseIndex; i < baseIndex + num; i++)
        {
          DcTopology::TopoNode host = CreateOneSwitch (queueNum, switchGroup);
          topology.InstallNode (i, host);
        }
    }
}

DcTopology::TopoNode
ProtobufTopologyLoader::CreateOneHost (const ns3_proto::HostGroup hostGroup)
{
  const Ptr<DcHost> host = CreateObject<DcHost> ();
  // TODO: optimize code here
  for (auto port : hostGroup.ports ())
    {
      // create a net device for the port
      const Ptr<DpskNetDevice> dev = CreateObject<DpskNetDevice> ();
      host->AddDevice (dev);
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
  // DpskHelper dpskHelper;
  // const auto dpsk = dpskHelper.Install (host);
  // TODO: Why so complicated?
  // auto pfcHost = CreateObject<PfcHost> ();
  // pfcHost->InstallDpsk (dpsk);

  return {.type = DcTopology::TopoNode::NodeType::HOST, .nodePtr = host};
}

DcTopology::TopoNode
ProtobufTopologyLoader::CreateOneSwitch (const uint32_t queueNum,
                                         const ns3_proto::SwitchGroup switchGroup)
{
  const Ptr<DcSwitch> sw = CreateObject<DcSwitch> ();
  // Basic configurations
  sw->SetEcmpSeed (m_ecmpSeed);
  sw->SetNQueues (queueNum);

  // Configure SwitchMmu
  const Ptr<SwitchMmu> mmu = CreateObject<SwitchMmu> ();
  sw->InstallMmu (mmu);
  ConfigMmu (switchGroup.mmu (), mmu);

  // Configure ports
  for (auto portConfig : switchGroup.ports ())
    {
      AddOnePortToSwitch (portConfig, sw, mmu);
    }

  // DpskHelper dpskHelper;
  // const Ptr<Dpsk> dpsk = dpskHelper.Install (sw);

  // const Ptr<PfcSwitch> pfcSwitch = CreateObject<PfcSwitch> ();
  // pfcSwitch->InstallDpsk (dpsk);

  return {.type = DcTopology::TopoNode::NodeType::SWITCH, .nodePtr = sw};
}

void
ProtobufTopologyLoader::AddOnePortToSwitch (const ns3_proto::SwitchPortConfig portConfig,
                                            const Ptr<DcSwitch> sw, const Ptr<SwitchMmu> mmu)
{
  // Create a net device for this port
  Ptr<DpskNetDevice> dev = CreateObject<DpskNetDevice> ();
  sw->AddDevice (dev);
  dev->SetAddress (Mac48Address::Allocate ());
  dev->SetTxMode (DpskNetDevice::TxMode::ACTIVE); // TODO: configurable mode

  // Add an implementation for the net device
  // TODO: improve the logic, maybe with more dynamic features
  if (portConfig.pfcenabled ())
    {
      const Ptr<PfcSwitchPort> impl = CreateObject<PfcSwitchPort> ();
      dev->SetImplementation (impl);
      impl->SetupQueues (portConfig.queues_size ());

      if (portConfig.has_pfcpassthrough ())
        {
          impl->SetPassThrough (portConfig.pfcpassthrough ());
        }

      for (auto queueConf = portConfig.queues ().cbegin ();
           queueConf != portConfig.queues ().cend ();
           queueConf++)
        {
          int index = queueConf - portConfig.queues ().begin ();

          const uint32_t headroom = QueueSize (queueConf->pfcheadroom ()).GetValue ();
          mmu->ConfigHeadroom (dev, index, headroom); // TODO: use index

          const uint32_t reserved = QueueSize (queueConf->pfcreserved ()).GetValue ();
          mmu->ConfigReserve (dev, index, reserved);

          const uint32_t ecnKMin = QueueSize (queueConf->ecnkmin ()).GetValue ();
          const uint32_t ecnKMax = QueueSize (queueConf->ecnkmax ()).GetValue ();
          const double ecnPMax = queueConf->ecnpmax ();
          mmu->ConfigEcn (dev, index, ecnKMin, ecnKMax, ecnPMax);
        }
    }
}

void
ProtobufTopologyLoader::ConfigMmu (const ns3_proto::SwitchMmuConfig mmuConfig,
                                   const Ptr<SwitchMmu> mmu)
{
  uint32_t bufferSize = QueueSize (mmuConfig.buffersize ()).GetValue ();
  mmu->ConfigBufferSize (bufferSize);
  if (mmuConfig.has_pfcdynamicshift ())
    {
      mmu->ConfigDynamicThreshold (true, mmuConfig.pfcdynamicshift ());
    }
}

} // namespace ns3
