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

#include "pfc-frame.h"
#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>

namespace ns3 {

PfcFrame::PfcFrame () : m_opcode (DEFAULT_OPCODE), m_enableVec (0)
{
}

void
PfcFrame::EnableClass (uint16_t cls)
{
  m_enableVec |= (1 << cls);
}

void
PfcFrame::DisableClass (uint16_t cls)
{
  m_enableVec &= ~static_cast<uint16_t> (1 << cls);
}

void
PfcFrame::SetEnableClassField (uint8_t vec)
{
  m_enableVec = vec; // only keep the lower 8 bits
}

uint8_t
PfcFrame::GetEnableClassField () const
{
  return static_cast<uint8_t>(m_enableVec);
}

void
PfcFrame::SetQuanta (uint8_t cls, uint16_t quanta)
{
  m_quantaVec[cls] = quanta;
}

uint16_t
PfcFrame::GetQuanta (uint8_t cls) const
{
  return m_quantaVec[cls];
}

TypeId
PfcFrame::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::PfcFrame").SetParent<Header> ().AddConstructor<PfcFrame> ();
  return tid;
}

TypeId
PfcFrame::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
PfcFrame::GetSerializedSize () const
{
  return sizeof (m_opcode) + sizeof (m_enableVec) + sizeof (m_quantaVec) + sizeof (m_reserved) +
         sizeof (m_frameCheck);
}

void
PfcFrame::Serialize (Buffer::Iterator start) const
{
  start.WriteU16 (m_opcode);
  start.WriteU16 (m_enableVec);
  start.Write (reinterpret_cast<const uint8_t *> (m_quantaVec), sizeof (m_quantaVec));
  start.Write (reinterpret_cast<const uint8_t *> (m_reserved), sizeof (m_reserved));
  start.WriteU16 (m_frameCheck);
}

uint32_t
PfcFrame::Deserialize (Buffer::Iterator start)
{
  m_opcode = start.ReadU16 ();
  m_enableVec = start.ReadU16 ();
  start.Read (reinterpret_cast<uint8_t *> (m_quantaVec), sizeof (m_quantaVec));
  start.Read (reinterpret_cast<uint8_t *> (m_reserved), sizeof (m_reserved));
  m_frameCheck = start.ReadU16 ();
  return GetSerializedSize ();
}

void
PfcFrame::Print (std::ostream &os) const
{
  os << "PFC frame: ";
  for (uint32_t i = 0; i < 8; i++)
    {
      os << "CoS " << i << " pauses quanta: " << GetQuanta (i) << std::endl;
    }
}

} // namespace ns3
