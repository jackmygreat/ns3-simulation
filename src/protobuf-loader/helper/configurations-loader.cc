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

#include "configurations-loader.h"
#include "protobuf-topology-loader.h"
#include "ns3/configurations.pb.h"
#include "ns3/log.h"
#include "ns3/fatal-error.h"
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ConfigurationsLoader");

namespace configurations {

void
RunConfigScript (std::string configFile)
{
  std::ifstream s;
  s.open (configFile);
  if (!s)
    {
      NS_FATAL_ERROR ("Python config file '" << configFile << "' does not exist");
    }
  // Simply execute it with the default Python3 interpreter.
  std::system (("python3 " + configFile).c_str ());
}

ns3_proto::Configurations
LoadConfigurations (const std::string &binName /* = protoBinaryName*/)
{
  std::fstream input (binName, std::ios::in | std::ios::binary);
  if (!input)
    {
      NS_FATAL_ERROR ("cannot find file "
                      << binName << " which should be created by the Python script with Protobuf");
    }

  ns3_proto::Configurations configurations;
  if (!configurations.ParseFromIstream (&input))
    {
      NS_FATAL_ERROR ("cannot parse binary file " << binName
                                                  << " which should be a Protobuf serialized file");
    }
  return configurations;
}

Ptr<DcTopology>
LoadTopology (const ns3_proto::Configurations &configurations)
{
  return topology::LoadTopology (configurations);
}

typedef std::function<void (const ns3_proto::Application &, Ptr<DcTopology>)> AppInstallFunc;
static void InstallTraceApplication (const ns3_proto::Application &appConfig,
                                     Ptr<DcTopology> topology);
  
static std::map<std::string, AppInstallFunc> appInstallMapper = {
    {"TraceApplication", InstallTraceApplication},
    // {"PacketSink", ProtobufTopologyLoader::InstallPacketSink},
    // {"PreGeneratedApplication", ProtobufTopologyLoader::InstallPreGeneratedApplication}
};

static std::map<std::string, TraceApplication::ProtocolGroup> protocolGroupMapper = {
    {"RAW_UDP", TraceApplication::ProtocolGroup::RAW_UDP},
    {"TCP", TraceApplication::ProtocolGroup::TCP},
    {"RoCEv2", TraceApplication::ProtocolGroup::RoCEv2},
};

static std::map<std::string, TraceApplication::TraceCdf *> appCdfMapper = {
    {"WebSearch", &TraceApplication::TRACE_WEBSEARCH_CDF},
    {"FdHadoop", &TraceApplication::TRACE_FDHADOOP_CDF}};

void
InstallApplications (const ns3_proto::Configurations &conf, Ptr<DcTopology> topology)
{
  const google::protobuf::RepeatedPtrField<ns3_proto::Application> &appsConfig =
      conf.applications ();
  for (const auto &appConfig : appsConfig)
    {
      auto it = appInstallMapper.find (appConfig.appname ());
      if (it == appInstallMapper.end ())
        {
          NS_FATAL_ERROR ("App \"" << appConfig.appname ()
                                   << "\" installation logic has not been implemented");
        }
      AppInstallFunc appInstallLogic = it->second;
      appInstallLogic (appConfig, topology);
    }
}

static void
InstallTraceApplication (const ns3_proto::Application &appConfig, Ptr<DcTopology> topology)
{
  TraceApplicationHelper appHelper (topology);

  {
    if (!appConfig.has_protocolgroup ())
      {
        NS_FATAL_ERROR ("Using TraceApplication needs to specify \"protocolGroup\"");
      }
    // set protocol group
    auto p = protocolGroupMapper.find (appConfig.protocolgroup ());
    if (p == protocolGroupMapper.end ())
      {
        NS_FATAL_ERROR ("Cannot recognize protocol group \"" << appConfig.protocolgroup () << "\"");
      }
    appHelper.SetProtocolGroup (p->second);
  }

  {
    if (!appConfig.has_arg ())
      {
        NS_FATAL_ERROR ("Using TraceApplication needs to specify \"arg\" which is the CDF name");
      }
    // set CDF
    auto p = appCdfMapper.find (appConfig.arg ());
    if (p == appCdfMapper.end ())
      {
        NS_FATAL_ERROR ("Cannot recognize CDF \"" << appConfig.arg () << "\".");
      }
    appHelper.SetCdf (*(p->second));
  }

  if (appConfig.has_dest ())
    {
      appHelper.SetDestination (appConfig.dest ());
    }

  for (const auto &nodeI : appConfig.nodeindices ())
    {
      if (!topology->IsHost (nodeI))
        {
          NS_FATAL_ERROR ("Node " << nodeI
                                  << " is not a host and thus could not install an application.");
        }
      Ptr<Node> node = topology->GetNode (nodeI).nodePtr;
      if (!appConfig.has_load ())
        {
          NS_FATAL_ERROR ("Using TraceApplication needs to specify \"load\"");
        }
      appHelper.SetLoad (DynamicCast<DcbNetDevice> (node->GetDevice (0)), appConfig.load ());
      ApplicationContainer app = appHelper.Install (node);
      app.Start (Time (appConfig.starttime ()));
      app.Stop (Time (appConfig.stoptime ()));
    }
}

} // namespace configurations

} // namespace ns3
