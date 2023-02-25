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
#include "ns3/dcb-net-device.h"
#include "ns3/dcb-traffic-control.h"
#include "ns3/rocev2-header.h"
#include "ns3/rocev2-socket.h"
#include "ns3/dcb-trace-application.h"

namespace ns3 {

class Ipv4;

class TracerExtension
{
public:
  TracerExtension ();

  typedef void (*FlowTracedCallback) (uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, Time, Time);

  enum Protocol {
    None,
    RoCEv2,
  };

  static void ConfigOutputDirectory (std::string dirName);
  static void ConfigStopTime (Time stopTime);

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

  static void EnableDeviceRateTrace (Ptr<NetDevice> device, std::string context, Time interval);

  static void EnablePortQueueLength (Ptr<NetDevice> device, std::string context, Time interval);

  static void EnableBufferoverflowTrace (Ptr<Node> sw, std::string context);

  static void CleanTracers ();

private:
  static std::string outputDirectory;
  static Time stopTime;

  static std::string GetRealFileName (std::string fileName);
  template <class T>
  static void ClearTracersList (std::list<T> &tracers);

  struct TraceFCTUnit
  {
    static Protocol protocol; // TODO: not supporting multiple protocols
    static std::ofstream fctFileStream;

    static void FlowCompletionTracer (uint32_t srcNode, uint32_t dstNode, uint32_t srcPort,
                                      uint32_t dstPort, uint32_t flowSize, Time startTime,
                                      Time finishTime);
  };

  class RateTracer
  {
  public:
    RateTracer (Time interval, std::string context);
    ~RateTracer ();
    void Trace (Ptr<const Packet> packet);
    static std::list<RateTracer *> tracers;

  private:
    void LogRate ();
    uint64_t m_bytes;
    Timer m_timer;
    std::string m_context;
    std::ofstream m_ofstream;
  }; // class RateTracer

  class QueueLengthTracer
  {
  public:
    QueueLengthTracer (std::string context, Ptr<PausableQueueDisc> queueDisc, Time interval);
    ~QueueLengthTracer ();
    void Trace ();
    static std::list<QueueLengthTracer*> tracers;

  private:
    Ptr<PausableQueueDisc> m_queueDisc;
    Timer m_timer;
    std::ofstream m_ofstream;
  }; // class QueueLengthTracer

  class BufferOverflowTracer {
  public:
    BufferOverflowTracer (std::string context, Ptr<DcbTrafficControl> tc);
    ~BufferOverflowTracer ();
    void Trace (Ptr<const Packet> packet);
    static std::list<BufferOverflowTracer *> tracers;
    
  private:
    std::ofstream m_ofstream;
  }; // class BufferoverflowTracer
  

}; // class TracerExtension

template <class T>
void TracerExtension::ClearTracersList (std::list<T> &tracers)
{
  for (auto tracer: tracers)
    {
      delete tracer;
    }
  tracers.clear ();
}


} // namespace ns3

#endif // TRACER_EXTENSION_H
