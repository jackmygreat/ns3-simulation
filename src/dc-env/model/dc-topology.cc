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

#include "dc-topology.h"
#include "ns3/fatal-error.h"
#include "ns3/integer.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/object-base.h"
#include "ns3/object.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/topology.pb.h"
#include "ns3/type-id.h"
#include <_types/_uint32_t.h>

/**
 * \file
 * \ingroup protobuf-topology
 * ns3::DcTopology implementation
 */
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcTopology");

NS_OBJECT_ENSURE_REGISTERED (DcTopology);

TypeId
DcTopology::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DcTopology").SetParent<Object> (). SetGroupName ("DcEnv");
  return tid;
}

DcTopology::DcTopology (uint32_t nodeNum)
{
  NS_LOG_FUNCTION (this);
  m_nodes.resize (nodeNum);
  // m_ipAddrHelper.SetBase ("10.0.0.0", "255.0.0.0");
}

DcTopology::~DcTopology ()
{
  NS_LOG_FUNCTION (this);
}

void
DcTopology::InstallNode (const uint32_t index, const TopoNode node)
{
  if (index >= m_nodes.size ())
    {
      NS_FATAL_ERROR ("node index " << index << " is out of bound, since there are "
                                    << m_nodes.size () << " nodes initialized.");
    }
  m_nodes[index] = node;
  m_nHosts += (node.type == TopoNode::NodeType::HOST);
}

const DcTopology::TopoNode &
DcTopology::GetNode (const uint32_t index) const
{
  if (index >= m_nodes.size ())
    {
      NS_FATAL_ERROR ("node index " << index << " is out of bound, since there are "
                                    << m_nodes.size () << " nodes initialized.");
    }
  return m_nodes[index];
}

const Ptr<NetDevice>
DcTopology::GetNetDeviceOfNode (const uint32_t nodei, const uint32_t devi) const
{
  const int ndev = GetNode (nodei)->GetNDevices ();
  if (devi >= ndev)
    {
      NS_FATAL_ERROR ("port index " << devi << " is out of bound, since there are " << ndev
                                    << " devices installed");
    }
  return StaticCast<NetDevice> (GetNode (nodei)->GetDevice (devi));
}

const Ipv4InterfaceAddress
DcTopology::GetInterfaceOfNode (const uint32_t nodei, uint32_t intfi) const
{
  Ptr<Ipv4> ipv4 = GetNode (nodei)->GetObject<Ipv4> ();
  const int nintf = ipv4->GetNInterfaces ();
  if (intfi > nintf)
    {
      NS_FATAL_ERROR ("interface index " << intfi << " is out of bound, since there are " << nintf
                                         << " devices installed");
    }
  return std::move (ipv4->GetAddress (intfi, 0)); // TODO: just return the first address for now
}

const Ptr<UniformRandomVariable>
DcTopology::CreateRamdomHostChooser () const
{
  Ptr<UniformRandomVariable> rng = Create<UniformRandomVariable> ();
  rng->SetAttribute ("Min", IntegerValue (0));
  rng->SetAttribute ("Max", IntegerValue (m_nHosts));
  return rng;
}

} // namespace ns3
