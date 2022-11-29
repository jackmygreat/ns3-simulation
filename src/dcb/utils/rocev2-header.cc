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

#include "rocev2-header.h"
#include "ns3/type-id.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RoCEv2Header);

TypeId
RoCEv2Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RoCEv2Header")
                          .SetParent<Header> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<RoCEv2Header> ();
  return tid;
}

TypeId
RoCEv2Header::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

RoCEv2Header::RoCEv2Header ()
{
}

RoCEv2Header::~RoCEv2Header ()
{
}

uint32_t
RoCEv2Header::GetSerializedSize (void) const
{
  return 12;
}

void
RoCEv2Header::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_opcode);
  i.WriteU8 (m_ua.u);
  i.WriteU16 (m_pKey);
  i.WriteU32 (m_ub.u);
  i.WriteU32 (m_uc.u);
}

uint32_t
RoCEv2Header::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_opcode = static_cast<RoCEv2Header::Opcode > (i.ReadU8 ()); // be careful here
  m_ua.u = i.ReadU8 ();
  m_pKey = i.ReadU16 ();
  m_ub.u = i.ReadU32 ();
  m_uc.u = i.ReadU32 ();
  return GetSerializedSize ();
}

void
RoCEv2Header::Print (std::ostream &os) const
{
  os << "RoCEv2: opcode=" << m_opcode << " destQP=" << GetDestQP ();
}

RoCEv2Header::Opcode
RoCEv2Header::GetOpCode () const
{
  return m_opcode;
}

void
RoCEv2Header::SetOpCode (RoCEv2Header::Opcode opcode)
{
  m_opcode = opcode;
}

uint32_t
RoCEv2Header::GetDestQP () const
{
  return m_ub.destQP;
}

void
RoCEv2Header::SetDestQP (uint32_t destQP)
{
  m_ub.destQP = destQP;
}

} // namespace ns3
