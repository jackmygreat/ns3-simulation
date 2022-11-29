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

#ifndef ROCEV2_HEADR_H
#define ROCEV2_HEADR_H

#include "ns3/header.h"

namespace ns3 {

class RoCEv2Header : public Header
{
public:

  RoCEv2Header ();
  virtual ~RoCEv2Header ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const override;

  virtual uint32_t GetSerializedSize (void) const override;
  virtual void Serialize (Buffer::Iterator start) const override;
  virtual uint32_t Deserialize (Buffer::Iterator start) override;
  virtual void Print (std::ostream &os) const override;

  enum Opcode { // part of the opcodes for now
    RC_SEND_ONLY = 0b000'00100,
    UD_SEND_ONLY = 0b011'00100,
    CNP          = 0b100'00000
  };

  Opcode GetOpCode () const;
  void SetOpCode (Opcode opcode);

  uint32_t GetDestQP () const;
  void SetDestQP (uint32_t destQP);
  
  
private:

  Opcode m_opcode;
  union {
    struct {
      uint8_t se: 1; // solicited event
      uint8_t m: 1;  // MigReq
      uint8_t padCnt: 2; // Pad count
      uint8_t tVer: 4; // Transport header version 
    } __attribute__((__packed__));
    uint8_t u;
  }  m_ua; 
  uint16_t m_pKey; // Partition key
  union {
    struct {
      uint8_t fr: 1; // F/Res1
      uint8_t br: 1;  // B/Res1
      uint8_t reserved: 6; // reserved
      uint32_t destQP: 24; // Destination QP
    } __attribute__((__packed__));
    uint32_t u;
  } m_ub;
  union {
    struct {
      uint8_t ackQ: 1; // Ackknowledge request
      uint8_t reserved: 7; // reserved
      uint32_t m_psn: 24; // packet sequence number
    } __attribute__((__packed__));
    uint32_t u;
  } m_uc;
  
}; // class RoCEv2Headerq

} // namespace ns3

#endif // ROCEV2_HEADR_H
