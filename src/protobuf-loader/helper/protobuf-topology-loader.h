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

#ifndef PROTOBUF_TOPOLOGY_LOADER_H
#define PROTOBUF_TOPOLOGY_LOADER_H

#include "google/protobuf/repeated_field.h"
#include "ns3/dc-topology.h"
#include "ns3/dc-switch.h"
#include "ns3/dc-host.h"
#include "ns3/topology.pb.h"

/**
 * \file
 * \ingroup protobuf-loader
 * ns3::ProtobufTopologyLoader declaration.
 */
namespace ns3 {

/**
 * \ingroup topology
 *
 * \brief Topology file loader for Protobuf.
 *
 */
class ProtobufTopologyLoader
{
public:
  ProtobufTopologyLoader ();

  /**
   * \brief Run the configuration Python script to generate Protobuf binary.
   */
  void RunConfigScript (const std::string configFile);

  DcTopology LoadTopology ();

protected:
  /**
   * \brief Read topology configuration from m_protoBinaryName file.  
   */
  ns3_proto::Topology ReadProtoTopology ();

  void LoadHosts (const google::protobuf::RepeatedPtrField<ns3_proto::HostGroup> &hostGroups,
                  DcTopology &topology);

  void LoadSwitches (const google::protobuf::RepeatedPtrField<ns3_proto::SwitchGroup> &switchGroups,
                     DcTopology &topology);
  void LoadLinks (const google::protobuf::RepeatedPtrField<ns3_proto::Link> &linksConfig,
                  DcTopology &topology);

  DcTopology::TopoNode CreateOneHost (const ns3_proto::HostGroup &hostGroup);

  DcTopology::TopoNode CreateOneSwitch (const uint32_t queueNum,
                                        const ns3_proto::SwitchGroup &switchGroup);

  /**
   * Add one port to the switch `sw` according to portConfig
   */
  void AddPortToSwitch (const ns3_proto::SwitchPortConfig portConfig, const Ptr<DcSwitch> sw);

  /**
   * Config SwitchMmu
   */
  // void ConfigMmu (const ns3_proto::SwitchMmuConfig mmuConfig, const Ptr<SwitchMmu> mmu);

  void InstallLink (const ns3_proto::Link &linkConfig, DcTopology &topology);

  void AssignAddresses (DcTopology &topology);

  void InitGlobalRouting ();

private:
  // Notice: this variable should be consistent with `TopologyGenerator.outputFile`
  // in config/topology_helper.py
  std::string m_protoBinaryName = "config/topology.bin";

  uint32_t m_ecmpSeed = 0;

  void LogIpAddress (const DcTopology& topology) const; // for debug
  void LogAllRoutes (const DcTopology& topology) const; // for debug
  void LogGlobalRouting (DcTopology& topology) const; // for debug
};

} // namespace ns3

#endif // PROTOBUF_TOPOLOGY_LOADER_H