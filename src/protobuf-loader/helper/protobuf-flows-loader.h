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

#ifndef PROTOBUF_FLOWS_LOADER_H
#define PROTOBUF_FLOWS_LOADER_H

#include "ns3/dc-topology.h"
#include "ns3/flows.pb.h"

/**
 * \file
 * \ingroup protobuf-loader
 * ns3::ProtobufFlowsLoader declaration
 */
namespace ns3 {

class ProtobufFlowsLoader
{
public:
  ProtobufFlowsLoader ();

  void LoadFlowsTo (DcTopology &topology);

protected:

  /**
   * \brief Read flows configuration from m_protoBinaryName file.
   */
  ns3_proto::Flows ReadProtoFlows ();

private:
  // Notice: this variable should be consistent with `FlowsGenerator.outputFile`
  // in config/flows_helper.py
  std::string m_protoBinaryName = "config/flows.bin";

}; // class ProtobufFlowsLoader

} // namespace ns3

#endif
