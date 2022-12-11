/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Pavinberg <pavin0702@gmail.com>
 */

#ifndef TRACER_EXTENSION_H
#define TRACER_EXTENSION_H

#include <fstream>
#include "ns3/rocev2-header.h"
#include "ns3/rocev2-socket.h"
#include "ns3/dcb-trace-application.h"

namespace ns3 {

class Ipv4;

class TracerExtension
{
public:
  TracerExtension ();

  typedef void (*FlowTracedCallback) (uint32_t, uint32_t, uint32_t, Time, Time);

  enum Protocol {
    None,
    RoCEv2,
  };

  static void ConfigOutputDirectory (std::string dirName);

  static void ConfigTraceFCT (Protocol protocol, std::string fileName);

  static void RegisterTraceFCT (Ptr<TraceApplication> app);

  /**
   * Capture packet at device and output the pcap file with prefix fileNamePrefix.
   */
  static void EnableDevicePcap (Ptr<NetDevice> device, std::string fileNamePrefix);
  /**
   * Capture packets at all IPv4 interfaces of a switch and output the pcap file
   * with prefix fileNamePrefix.
   */
  static void EnableSwitchIpv4Pcap (Ptr<Node> sw, std::string fileNamePrefix);

private:
  static std::string outputDirectory;

  static std::string GetRealFileName (std::string fileName);

  struct TraceFCTConfig
  {
    static Protocol protocol; // TODO: not supporting multiple protocols
    static std::ofstream fctFileStream;

    static void FlowCompletionTracer (uint32_t srcPort, uint32_t dstPort, uint32_t flowSize, Time startTime, Time finishTime);
  };

}; // class TracerExtension

} // namespace ns3

#endif // TRACER_EXTENSION_H
