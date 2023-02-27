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

#ifndef PROTOBUF_TOPOLOGY_LOADER_H
#define PROTOBUF_TOPOLOGY_LOADER_H

#include "google/protobuf/repeated_field.h"
#include "ns3/dc-topology.h"
#include "ns3/dcb-net-device.h"
#include "ns3/dcb-channel.h"
#include "ns3/dcb-trace-application-helper.h"
#include "ns3/dcb-host-stack-helper.h"
#include "ns3/dcb-switch-stack-helper.h"
#include "ns3/configurations.pb.h"
#include <functional>

/**
 * \file
 * \ingroup protobuf-loader
 * ns3::ProtobufTopologyLoader declaration.
 */
namespace ns3 {

/**
 * \ingroup topology
 *
 * \brief Topology file loader for Protobuf.
 *
 */
namespace topology {

  Ptr<DcTopology> LoadTopology (const ns3_proto::Configurations &configurations);
  
} // namespace topology_loader

} // namespace ns3

#endif // PROTOBUF_TOPOLOGY_LOADER_H
