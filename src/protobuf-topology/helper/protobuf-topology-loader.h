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

#ifndef PROTOBUF_TOPOLOGY_READER_H
#define PROTOBUF_TOPOLOGY_READER_H

#include "ns3/dc-topology.h"
#include "ns3/core-module.h"
#include "ns3/dc-topology.h"
#include "ns3/topology.pb.h"


/**
 * \file
 * \ingroup protobuf-topology
 * ns3::ProtobufTopologyReader declaration.
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
  void RunConfigScript (std::string configFile);

  DcTopology* LoadTopology ();

private:

  /**
   * \brief Read topology configuration from m_protoBinaryName file.  
   */
  ns3_proto::Topology ReadProtoTopology ();

  void LoadHosts (google::protobuf::RepeatedPtrField<ns3_proto::HostGroup> hostGroups,
                  DcTopology &topology);
  
  void LoadSwitches (google::protobuf::RepeatedPtrField<ns3_proto::SwitchGroup> switchGroups,
                  DcTopology &topology);

  DcTopology::TopoNode CreateOneHost (google::protobuf::RepeatedPtrField<ns3_proto::HostPortConfig> portsConfig);

  DcTopology::TopoNode CreateOneSwitch (google::protobuf::RepeatedPtrField<ns3_proto::SwitchPortConfig> portsConfig);

  /**
   * Add one port to the switch `sw` according to portConfig
   */
  void AddOnePortToSwitch (ns3_proto::SwitchPortConfig portConfig, DcTopology::TopoNode &sw);

  // Notice: this variable should be consistent with `TopologyGenerator.outputFile`
  // in config/topology_helper.py
  std::string m_protoBinaryName = "config/topology.bin"; 
  
};

} // namespace ns3

#endif // PROTOBUF_TOPOLOGY_READER_H
