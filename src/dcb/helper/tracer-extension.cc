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

#include "tracer-extension.h"
#include "csv-writer.h"
#include "dcb-net-device-helper.h"
#include "dcb-switch-stack-helper.h"
#include "ns3/dcb-net-device.h"
#include <fstream>

namespace ns3 {

using TE = TracerExtension;

std::string TE::outputDirectory = "";
Time TE::stopTime;
TE::Protocol TE::TraceFCTUnit::protocol = TE::Protocol::None;
std::ofstream TE::TraceFCTUnit::fctFileStream;

TE::TracerExtension ()
{
}

// static
void
TE::ConfigOutputDirectory (std::string dirName)
{
  // TODO: create the directory if not exists
  TE::outputDirectory = dirName;
}

// static  
void
TE::ConfigStopTime(Time stopTime)
{
  TE::stopTime = stopTime;
}

// static
void
TE::ConfigTraceFCT (Protocol protocol, std::string fileName)
{
  TraceFCTUnit::protocol = protocol;
  std::string fctFile = GetRealFileName (fileName);
  TraceFCTUnit::fctFileStream.open (fctFile);
  if (!TraceFCTUnit::fctFileStream.good ())
    {
      std::cerr << "Error: Cannot open file \"" << fctFile << "\"" << std::endl;
    }
}

// static
void
TE::RegisterTraceFCT (Ptr<TraceApplication> app)
{
  switch (TraceFCTUnit::protocol)
    {
    case Protocol::RoCEv2:
      app->TraceConnectWithoutContext ("FlowComplete",
                                       MakeCallback (&TraceFCTUnit::FlowCompletionTracer));
      break;
    default: // do nothing
        ;
    }
}

// static
void
TE::EnableDevicePcap (Ptr<NetDevice> device, std::string fileNamePrefix)
{
  DcbNetDeviceHelper devHelper;
  devHelper.EnablePcap (GetRealFileName (fileNamePrefix), device);
}

// static
void
TE::EnableSwitchIpv4Pcap (Ptr<Node> sw, std::string fileNamePrefix)
{
  Ptr<Ipv4> ipv4 = sw->GetObject<Ipv4> ();
  if (!ipv4)
    {
      NS_FATAL_ERROR ("Ipv4 is not bound to the switch " << sw);
    }
  const int nintf = ipv4->GetNInterfaces ();
  DcbSwitchStackHelper switchStack;
  for (uint32_t i = 1; i < nintf; i++) // interface 0 is loopback so we skip it.
    {
      switchStack.EnablePcapIpv4 (GetRealFileName (fileNamePrefix), ipv4, i, false);
    }
}

// static
void
TE::EnableDeviceRateTrace (Ptr<NetDevice> device, std::string context, Time interval)
{
  RateTracer *tracer = new RateTracer (interval, context);
  device->TraceConnectWithoutContext (
      "MacTx", MakeCallback (&TE::RateTracer::Trace, tracer));
}

// static
void
TE::EnablePortQueueLength (Ptr<NetDevice> device, std::string context, Time interval)
{
  Ptr<DcbNetDevice> dcbDev = DynamicCast<DcbNetDevice> (device);
  if (dcbDev)
    {
      new QueueLengthTracer (context, dcbDev->GetQueueDisc(), interval);
    }
  else
    {
      NS_FATAL_ERROR ("Cannot cast NetDevice to DcbNetDevice");
    }
}

// static
std::string
TE::GetRealFileName (std::string fileName)
{
  return TE::outputDirectory + "/" + fileName;
}

// static
void
TE::TraceFCTUnit::FlowCompletionTracer (uint32_t destNode, uint32_t srcPort, uint32_t dstPort,
                                          uint32_t flowSize, Time startTime, Time finishTime)
{
  // TODO: add mutex lock for concurrency
  Time fct = finishTime - startTime;
  CsvWriter writer (&TE::TraceFCTUnit::fctFileStream, 8);
  writer.WriteNextValue (Simulator::GetContext ()); // src node
  writer.WriteNextValue (destNode); // dest node
  writer.WriteNextValue (srcPort); // src port/qp
  writer.WriteNextValue (dstPort); // dest port/qp
  writer.WriteNextValue (flowSize);
  writer.WriteNextValue (startTime.GetNanoSeconds ());
  writer.WriteNextValue (finishTime.GetNanoSeconds ());
  writer.WriteNextValue (fct.GetNanoSeconds ());

  // NS_LOG_UNCOND ("Node " << Simulator::GetContext () << " flow of RoCEv2 " << roce.GetDestQP ()
  //                        << "->" << roce.GetSrcQP () << " start at " << startTime << " finish at "
  //                        << finishTime << " with FCT=" << fct);
}

TE::RateTracer::RateTracer (Time interval, std::string context)
  : m_bytes (0), m_context (context)
{
  m_timer.SetFunction (&TE::RateTracer::LogRate, this);
  m_timer.SetDelay (interval);
  m_timer.Schedule ();

  std::string filename = outputDirectory + "/rate-tx-" + context + ".csv";
  m_ofstream.open (filename);
  if (!m_ofstream.good ())
    {
      std::cerr << "Error: Cannot open file \"" << filename << "\"" << std::endl;
    }
}

void
TE::RateTracer::Trace (Ptr<const Packet> packet)
{
  m_bytes += packet->GetSize ();
}

void
TE::RateTracer::LogRate ()
{
  double rate = static_cast<double> (m_bytes) * 8 / m_timer.GetDelay ().GetMicroSeconds ();
  m_ofstream << Simulator::Now ().GetMicroSeconds () << "," << rate << std::endl;
  // NS_LOG_UNCOND ("rate of device " <<  m_context << " is " << rate << "Mbps");
  m_bytes = 0;
  if (Simulator::Now () < TE::stopTime)
    {
      m_timer.Schedule ();
    }
  else
    {
      delete this; // kill myself
    }
}

TE::QueueLengthTracer::QueueLengthTracer (std::string context, Ptr<PausableQueueDisc> queueDisc, Time interval) : m_queueDisc (queueDisc)
{
  m_timer.SetFunction (&TE::QueueLengthTracer::Trace, this);
  m_timer.SetDelay (interval);
  m_timer.Schedule ();

  std::string filename = outputDirectory + "/queue-" + context + ".csv";
  m_ofstream.open (filename);
  if (!m_ofstream.good ())
    {
      std::cerr << "Error: Cannot open file \"" << filename << "\"" << std::endl;      
    }
}

void
TE::QueueLengthTracer::Trace ()
{
  size_t nQueue = m_queueDisc->GetNQueueDiscClasses ();
  m_ofstream << Simulator::Now ().GetMicroSeconds ();
  for (size_t i = 0; i < nQueue; i++) // Get queue length of each priority
    {
      uint32_t bytes = m_queueDisc->GetQueueDiscClass(i)->GetQueueDisc()->GetNBytes();
      m_ofstream << "," << bytes; 
    }
  m_ofstream << std::endl;
  if (Simulator::Now () < TE::stopTime)
    {
      m_timer.Schedule ();
    }
  else
    {
      delete this; // kill myself
    }
}

} // namespace ns3
