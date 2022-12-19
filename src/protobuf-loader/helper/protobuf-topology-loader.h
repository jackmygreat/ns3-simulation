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
#include "ns3/dcb-net-device.h"
#include "ns3/dcb-channel.h"
#include "ns3/dcb-trace-application-helper.h"
#include "ns3/dcb-host-stack-helper.h"
#include "ns3/dcb-switch-stack-helper.h"
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

  Ptr<DcTopology> LoadTopology ();

protected:
  /**
   * \brief Read topology configuration from m_protoBinaryName file.  
   */
  ns3_proto::Topology ReadProtoTopology ();

  void LoadHosts (const google::protobuf::RepeatedPtrField<ns3_proto::HostGroup> &hostGroups,
                  Ptr<DcTopology> topology);

  void LoadSwitches (const google::protobuf::RepeatedPtrField<ns3_proto::SwitchGroup> &switchGroups,
                     Ptr<DcTopology> topology);
  void LoadLinks (const google::protobuf::RepeatedPtrField<ns3_proto::Link> &linksConfig,
                  Ptr<DcTopology> topology);

  DcTopology::TopoNode CreateOneHost (const ns3_proto::HostGroup &hostGroup);

  DcTopology::TopoNode CreateOneSwitch (const uint32_t queueNum,
                                        const ns3_proto::SwitchGroup &switchGroup);

  /**
   * Add one port to the switch `sw` according to portConfig
   */
  Ptr<DcbNetDevice> AddPortToSwitch (const ns3_proto::SwitchPortConfig portConfig,
                                     const Ptr<Node> sw, DcbSwitchStackHelper& switchStack);

  void AssignAddress (const Ptr<Node> node, const Ptr<NetDevice> device);

  void InstallLink (const ns3_proto::Link &linkConfig, Ptr<DcTopology> topology);

  void InitGlobalRouting ();

  void
  InstallApplications (const google::protobuf::RepeatedPtrField<ns3_proto::Application> &appsConfig,
                       Ptr<DcTopology> topology);

private:
  // Notice: this variable should be consistent with `TopologyGenerator.outputFile`
  // in config/topology_helper.py
  std::string m_protoBinaryName = "config/topology.bin";

  uint32_t m_ecmpSeed = 0;

  static std::map<std::string, TraceApplicationHelper::ProtocolGroup> protocolGroupMapper;
  static std::map<std::string, TraceApplication::TraceCdf *> appCdfMapper;

  void LogIpAddress (const Ptr<const DcTopology> topology) const; // for debug
  void LogAllRoutes (const Ptr<const DcTopology> topology) const; // for debug
  void LogGlobalRouting (const Ptr<DcTopology> topology) const; // for debug
};

} // namespace ns3

#endif // PROTOBUF_TOPOLOGY_LOADER_H
