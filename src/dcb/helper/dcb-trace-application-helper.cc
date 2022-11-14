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

#include "dcb-trace-application-helper.h"
#include "ns3/node.h"

namespace ns3 {

TraceApplicationHelper::TraceApplicationHelper ()
{
  m_factory.SetTypeId (TraceApplication::GetTypeId ());
}

void
TraceApplicationHelper::SetProtocolGroup (ProtocolGroup protoGroup)
{
  m_protoGroup = protoGroup;
}

void
TraceApplicationHelper::SetCdf (const TraceApplication::TraceCdf& cdf)
{
  
}

void
TraceApplicationHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
TraceApplicationHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
TraceApplicationHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<TraceApplication> ();
  node->AddApplication (app);
  return app;
}

} //namespace ns3
