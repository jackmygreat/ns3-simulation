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

#include "ns3/core-module.h"
#include "ns3/nstime.h"
#include "ns3/protobuf-topology-loader.h"
#include "ns3/log.h"
#include "ns3/dcb-module.h"
#include "ns3/internet-module.h" 

using namespace ns3;

int 
main ()
{
  ProtobufTopologyLoader topoLoader;
  // topoLoader.RunConfigScript("config/dumbell_topo.py");
  Ptr<DcTopology> topology = topoLoader.LoadTopology();
  // topology->Print(std::cout);
  
  LogComponentEnable ("TraceApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("ProtobufTopologyLoader", LOG_LEVEL_DEBUG);
  LogComponentEnable ("DcbTrafficControl", LOG_LEVEL_INFO);
  LogComponentEnable ("DcbSwitchStackHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("DcbHostStackHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("DcbPfcPort", LOG_LEVEL_DEBUG);
  LogComponentEnable ("RoCEv2L4Protocol", LOG_LEVEL_INFO);
  LogComponentEnable ("RoCEv2Socket", LOG_LEVEL_INFO);
  LogComponentEnable ("FifoQueueDiscEcn", LOG_LEVEL_WARN);
  LogComponentEnable ("DcqcnCongestionOps", LOG_LEVEL_DEBUG);
  LogComponentEnableAll (LOG_PREFIX_LEVEL);
  LogComponentEnableAll(LOG_PREFIX_NODE);

  TracerExtension::ConfigOutputDirectory("data");
  TracerExtension::ConfigStopTime (MilliSeconds (7));

  // TracerExtension::ConfigTraceFCT (TracerExtension::Protocol::RoCEv2, "fct.csv");

  // // capture packet at host-0
  // Ptr<NetDevice> dev = topology->GetNetDeviceOfNode(0, 0);
  // TracerExtension::EnableDevicePcap(dev, "host");

  // // capture packet at host-2
  // dev = topology->GetNetDeviceOfNode(1, 0);
  // TracerExtension::EnableDevicePcap(dev, "host");

  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(16).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(17).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(18).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(19).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(24).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(25).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(26).nodePtr, "switch");
  // TracerExtension::EnableSwitchIpv4Pcap(topology->GetNode(27).nodePtr, "switch");
  // capture packet at all switches
  // for (auto sw = topology->switches_begin(); sw != topology->switches_end(); sw++)
  //   {
  //     TracerExtension::EnableSwitchIpv4Pcap(sw->nodePtr, "switch");
  //   }

  TracerExtension::EnableDeviceRateTrace(topology->GetNetDeviceOfNode(6, 2), "6-2", MicroSeconds (50));
  TracerExtension::EnablePortQueueLength(topology->GetNetDeviceOfNode(6, 2), "6-2", MicroSeconds(50));
  TracerExtension::EnablePortQueueLength(topology->GetNetDeviceOfNode(3, 1), "3-1", MicroSeconds(50));
  TracerExtension::EnablePortQueueLength(topology->GetNetDeviceOfNode(4, 1), "4-1", MicroSeconds(50));

  Simulator::Stop (MilliSeconds (7));
  Simulator::Run ();
  Simulator::Destroy ();
}
