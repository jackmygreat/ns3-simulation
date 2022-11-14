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

#include "dcb-socket.h"
#include "ns3/log.h"
#include "ns3/object-base.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DcbSocket");

NS_OBJECT_ENSURE_REGISTERED (DcbSocket);

TypeId
DcbSocket::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::DcbSocket")
                          .SetParent<Socket> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<DcbSocket> ();
  return tid;
}

DcbSocket::DcbSocket ()
{
  NS_LOG_FUNCTION (this);
}

DcbSocket::~DcbSocket ()
{
  NS_LOG_FUNCTION (this);
}

enum Socket::SocketErrno
DcbSocket::GetErrno (void) const
{
  NS_LOG_FUNCTION (this);
  return Socket::SocketErrno::ERROR_ADDRINUSE;
}

enum Socket::SocketType
DcbSocket::GetSocketType (void) const
{
  NS_LOG_FUNCTION (this);
  return Socket::SocketType::NS3_SOCK_RAW;
}

Ptr<Node>
DcbSocket::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return 0;
}

const uint32_t
DcbSocket::GetSerializedHeaderSize () const
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::Bind (const Address &address)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::Bind ()
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
int
DcbSocket::Bind6 ()
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
int
DcbSocket::Close (void)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::ShutdownSend (void)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::ShutdownRecv (void)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::Connect (const Address &address)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::Listen (void)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

uint32_t
DcbSocket::GetTxAvailable (void) const
{
  NS_LOG_FUNCTION (this);
  return 0;
}

int
DcbSocket::Send (Ptr<Packet> p, uint32_t flags)
{
  NS_LOG_FUNCTION (this << p << flags);
  return 0;
}

int
DcbSocket::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  return Send (p, 0);
}

int
DcbSocket::SendTo (Ptr<Packet> p, uint32_t flags, const Address &toAddress)
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
uint32_t
DcbSocket::GetRxAvailable (void) const
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
Ptr<Packet>
DcbSocket::Recv (uint32_t maxSize, uint32_t flags)
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
Ptr<Packet>
DcbSocket::RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress)
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
int
DcbSocket::GetSockName (Address &address) const
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
int
DcbSocket::GetPeerName (Address &address) const
{
  NS_LOG_FUNCTION (this);
  return 0;
}
  
bool
DcbSocket::SetAllowBroadcast (bool allowBroadcast)
{
  NS_LOG_FUNCTION (this);
  return 0;
}

bool
DcbSocket::GetAllowBroadcast () const
{
  NS_LOG_FUNCTION (this);
  return false;
}

} // namespace ns3
