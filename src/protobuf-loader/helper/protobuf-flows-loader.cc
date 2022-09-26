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

/**
 * \file
 * \ingroup protobuf-loader
 * ns3::ProtobufFlowsLoader implementation.
 */

#include <_types/_uint32_t.h>
#include <fstream>
#include <string>
#include "ns3/nstime.h"
#include "ns3/udp-echo-helper.h"
#include "protobuf-flows-loader.h"
#include "ns3/flows.pb.h"
#include "ns3/log.h"
#include "ns3/fatal-error.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ProtobufFlowsLoader");

ProtobufFlowsLoader::ProtobufFlowsLoader ()
{
}

void
ProtobufFlowsLoader::LoadFlowsTo (DcTopology &topology)
{
  NS_LOG_FUNCTION (this);
  ns3_proto::Flows flowsConfig = ReadProtoFlows ();
  for (const ns3_proto::Flow &flow : flowsConfig.flows()) {
    const uint32_t dstnode = flow.dstnode();
    const uint32_t dstport = flow.dstport();
    const uint32_t intfi = 1; // index 0 is loopback interface
    UdpEchoClientHelper echoClient (topology.GetInterfaceOfNode(dstnode, intfi).GetAddress(), dstport);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (flow.size() / 1024));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
    ApplicationContainer clientApps = echoClient.Install (topology.GetNode(flow.srcnode()).nodePtr);
    clientApps.Start (MicroSeconds (flow.arrivetime()));
    clientApps.Stop (Seconds (10.0));
  }
}

ns3_proto::Flows
ProtobufFlowsLoader::ReadProtoFlows ()
{
  NS_LOG_FUNCTION (this);
  std::fstream input (m_protoBinaryName, std::ios::in | std::ios::binary);
  if (!input)
    {
      NS_FATAL_ERROR ("cannot find file "
                      << m_protoBinaryName
                      << " which should be created by the Python script with Protobuf");
    }

  ns3_proto::Flows flows;
  if (!flows.ParseFromIstream (&input))
    {
      NS_FATAL_ERROR ("cannot parse binary file " << m_protoBinaryName
                                                  << " which should be a Protobuf serialized file");
    }
  return flows;
}

} // namespace ns3
