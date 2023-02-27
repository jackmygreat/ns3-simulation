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

#ifndef CONFIGURATIONS_LOADER_H
#define CONFIGURATIONS_LOADER_H

#include <string>
#include "ns3/configurations.pb.h"
#include "ns3/dc-topology.h"
#include "ns3/dcb-trace-application.h"

namespace ns3 {

namespace configurations {
  /**
   * \brief Run the configuration Python script to generate Protobuf binary.
   */
  void RunConfigScript (const std::string configFile);

  // Notice: this variable should be consistent with `TopologyGenerator.outputFile`
  // in config/config_helper.py
  static std::string const protoBinaryName = "config/configurations.bin";

  ns3_proto::Configurations LoadConfigurations (const std::string &binName = protoBinaryName);

  Ptr<DcTopology> LoadTopology (const ns3_proto::Configurations &configurations);

  void InstallApplications (const ns3_proto::Configurations &conf, Ptr<DcTopology> topology);

} // namespace configurations

} // namespace ns3

#endif // CONFIGURATIONS_LOADER_H
