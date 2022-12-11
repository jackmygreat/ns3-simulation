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
#include <fstream>

namespace ns3 {

using TE = TracerExtension;

std::string TE::outputDirectory = "";
TE::Protocol TE::TraceFCTConfig::protocol = TE::Protocol::None;
std::ofstream TE::TraceFCTConfig::fctFileStream;

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
TE::ConfigTraceFCT (Protocol protocol, std::string fileName)
{
  TraceFCTConfig::protocol = protocol;
  std::string fctFile = GetRealFileName (fileName);
  TraceFCTConfig::fctFileStream.open (fctFile);
  if (!TraceFCTConfig::fctFileStream.good ())
    {
      std::cerr << "Error: Cannot open file \"" << fctFile << "\"" << std::endl;
    }
}

// static
void
TE::RegisterTraceFCT (Ptr<Socket> socket)
{
  switch (TraceFCTConfig::protocol)
    {
    case Protocol::RoCEv2:
      socket->TraceConnectWithoutContext (
          "FlowComplete", MakeCallback (&TraceFCTConfig::RoCEv2FlowCompletionTracer));
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
	  switchStack.EnablePcapIpv4(GetRealFileName(fileNamePrefix), ipv4, i, false);
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
TE::TraceFCTConfig::RoCEv2FlowCompletionTracer (Ptr<RoCEv2Socket> socket, const RoCEv2Header &roce)
{
  // TODO: add mutex lock for concurrency
  Time startTime = socket->GetFlowStartTime ();
  Time finishTime = Simulator::Now ();
  Time fct = finishTime - startTime;
  CsvWriter writer (&TE::TraceFCTConfig::fctFileStream, 6);
  writer.WriteNextValue (Simulator::GetContext ());
  writer.WriteNextValue (roce.GetDestQP ());
  writer.WriteNextValue (roce.GetSrcQP ());
  writer.WriteNextValue (startTime.GetNanoSeconds ());
  writer.WriteNextValue (finishTime.GetNanoSeconds ());
  writer.WriteNextValue (fct.GetNanoSeconds ());

  // NS_LOG_UNCOND ("Node " << Simulator::GetContext () << " flow of RoCEv2 " << roce.GetDestQP ()
  //                        << "->" << roce.GetSrcQP () << " start at " << startTime << " finish at "
  //                        << finishTime << " with FCT=" << fct);
}

} // namespace ns3
