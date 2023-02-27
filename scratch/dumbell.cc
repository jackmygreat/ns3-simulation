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

#include "ns3/configurations-loader.h"
#include "ns3/configurations.pb.h"
#include "ns3/core-module.h"
#include "ns3/nstime.h"
#include "ns3/protobuf-loader-module.h"
#include "ns3/log.h"
#include "ns3/dcb-module.h"
#include "ns3/internet-module.h"

using namespace ns3;

int
main ()
{
  // configurations::RunConfigScript("config/dumbell_topo.py");
  const ns3_proto::Configurations config = configurations::LoadConfigurations ();
  
  Ptr<DcTopology> topology = configurations::LoadTopology (config);
  // topology->Print(std::cout);
  configurations::InstallApplications (config, topology);

  LogComponentEnableAll (LOG_LEVEL_WARN);
  LogComponentEnable ("TraceApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("ProtobufTopologyLoader", LOG_LEVEL_DEBUG);
  LogComponentEnable ("DcbTrafficControl", LOG_LEVEL_INFO);
  //LogComponentEnable ("DcbPfcPort", LOG_LEVEL_DEBUG);
  LogComponentEnable ("RoCEv2Socket", LOG_LEVEL_DEBUG);
  // LogComponentEnable ("DcqcnCongestionOps", LOG_DEBUG);
  // LogComponentEnable ("FifoQueueDiscEcn", LOG_LEVEL_INFO);
  LogComponentEnableAll (LOG_PREFIX_LEVEL);
  LogComponentEnableAll (LOG_PREFIX_NODE);

  TracerExtension::ConfigOutputDirectory ("data");
  TracerExtension::ConfigTraceFCT (TracerExtension::Protocol::RoCEv2, "fct.csv");
  TracerExtension::ConfigStopTime (MilliSeconds(6));

  // capture packet at host-0
  Ptr<NetDevice> dev = topology->GetNetDeviceOfNode (0, 0);
  // TracerExtension::EnableDevicePcap (dev, "host");
  // TracerExtension::EnableDeviceRateTrace (dev, "host0", MicroSeconds (10));

  // capture packet at host-2
  // dev = topology->GetNetDeviceOfNode (2, 0);
  // TracerExtension::EnableDevicePcap (dev, "host");

  TracerExtension::EnableBufferoverflowTrace (topology->GetNode (4).nodePtr, "sw4");
  TracerExtension::EnableBufferoverflowTrace (topology->GetNode (5).nodePtr, "sw5");

  Ptr<NetDevice> swDev = topology->GetNetDeviceOfNode(4, 2);
  TracerExtension::EnableDeviceRateTrace(swDev, "sw4", MicroSeconds (100));
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(16).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(17).nodePtr, "switch");
  // capture packet at all switches
  // for (auto sw = topology->switches_begin(); sw != topology->switches_end(); sw++)
  //   {
  //     TracerExtension::EnableSwitchIpv4Pcap(sw->nodePtr, "switch");
  //   }
  TracerExtension::EnablePortQueueLengthTrace(topology->GetNetDeviceOfNode(4, 0), "sw4-0", MicroSeconds (10));
  TracerExtension::EnablePortQueueLengthTrace(topology->GetNetDeviceOfNode(5, 2), "sw5-2", MicroSeconds (10));
  Simulator::Run ();
  Simulator::Destroy ();

  TracerExtension::CleanTracers ();
}
