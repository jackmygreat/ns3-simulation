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

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("TraceApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  LogComponentEnable ("ProtobufTopologyLoader", LOG_LEVEL_DEBUG);
  LogComponentEnable ("DcbTrafficControl", LOG_LEVEL_INFO);
  LogComponentEnable ("DcbSwitchStackHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("DcbStackHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("DcbPfcPort", LOG_LEVEL_DEBUG);
  LogComponentEnable ("RoCEv2L4Protocol", LOG_LEVEL_INFO);
  LogComponentEnable ("RoCEv2Socket", LOG_LEVEL_INFO);

  TracerExtension::ConfigOutputDirectory("data");
  TracerExtension::ConfigTraceFCT (TracerExtension::Protocol::RoCEv2, "fct.csv");

  // capture packet at host-0
  // Ptr<NetDevice> dev = topology->GetNode (0)->GetDevice (0);
  // TracerExtension::EnableDevicePcap(dev, "host");

  // capture packet at all switches
  // for (auto sw = topology->switches_begin(); sw != topology->switches_end(); sw++)
  //   {
  //     TracerExtension::EnableSwitchIpv4Pcap(sw->nodePtr, "switch");
  //   }

  Simulator::Run ();
  Simulator::Destroy ();
}
