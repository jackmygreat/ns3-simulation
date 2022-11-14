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

#ifndef DCB_SOCKET_H
#define DCB_SOCKET_H

#include "ns3/socket.h"

namespace ns3 {

class DcbSocket : public Socket
{
public:
  /**
   * Get the type ID.
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  
  DcbSocket ();
  virtual ~DcbSocket ();

  /**
   * \brief Get last error number.
   *
   * \return the errno associated to the last call which failed in this
   *         socket. Each socket's errno is initialized to zero
   *         when the socket is created.
   */
  virtual enum Socket::SocketErrno GetErrno (void) const;

  /**
    * \return the socket type, analogous to getsockopt (SO_TYPE)
    */
  virtual enum Socket::SocketType GetSocketType (void) const;
  /**
   * \brief Return the node this socket is associated with.
   * \returns the node
   */
  virtual Ptr<Node> GetNode (void) const;

  virtual const uint32_t GetSerializedHeaderSize () const;

  /** 
   * \brief Allocate a local endpoint for this socket.
   * \param address the address to try to allocate
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind (const Address &address);

  /** 
   * \brief Allocate a local IPv4 endpoint for this socket.
   *
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind ();

  /** 
   * \brief Allocate a local IPv6 endpoint for this socket.
   *
   * \returns 0 on success, -1 on failure.
   */
  virtual int Bind6 ();

  /**
   * \brief Close a socket.
   * \returns zero on success, -1 on failure.
   *
   * After the Close call, the socket is no longer valid, and cannot
   * safely be used for subsequent operations.
   */
  virtual int Close (void);

  /**
   * \returns zero on success, -1 on failure.
   *
   * Do not allow any further Send calls. This method is typically
   * implemented for Tcp sockets by a half close.
   */
  virtual int ShutdownSend (void);

  /**
   * \returns zero on success, -1 on failure.
   *
   * Do not allow any further Recv calls. This method is typically
   * implemented for Tcp sockets by a half close.
   */
  virtual int ShutdownRecv (void);

  /**
   * \brief Initiate a connection to a remote host
   * \param address Address of remote.
   * \returns 0 on success, -1 on error (in which case errno is set).
   */
  virtual int Connect (const Address &address);

  /**
   * \brief Listen for incoming connections.
   * \returns 0 on success, -1 on error (in which case errno is set).
   */
  virtual int Listen (void);

  /**
   * \brief Returns the number of bytes which can be sent in a single call
   * to Send. 
   * 
   * For datagram sockets, this returns the number of bytes that
   * can be passed atomically through the underlying protocol.
   *
   * For stream sockets, this returns the available space in bytes
   * left in the transmit buffer.
   *
   * \returns The number of bytes which can be sent in a single Send call.
   */
  virtual uint32_t GetTxAvailable (void) const;

    /**
   * \brief Send data (or dummy data) to the remote host
   *
   * This function matches closely in semantics to the send() function
   * call in the standard C library (libc):
   *   ssize_t send (int s, const void *msg, size_t len, int flags);
   * except that the send I/O is asynchronous.  This is the
   * primary Send method at this low-level API and must be implemented 
   * by subclasses.
   * 
   * In a typical blocking sockets model, this call would block upon
   * lack of space to hold the message to be sent.  In ns-3 at this
   * API, the call returns immediately in such a case, but the callback
   * registered with SetSendCallback() is invoked when the socket
   * has space (when it conceptually unblocks); this is an asynchronous
   * I/O model for send().
   * 
   * This variant of Send() uses class ns3::Packet to encapsulate
   * data, rather than providing a raw pointer and length field.
   * This allows an ns-3 application to attach tags if desired (such
   * as a flow ID) and may allow the simulator to avoid some data
   * copies.  Despite the appearance of sending Packets on a stream
   * socket, just think of it as a fancy byte buffer with streaming
   * semantics.
   *
   * If either the message buffer within the Packet is too long to pass 
   * atomically through the underlying protocol (for datagram sockets), 
   * or the message buffer cannot entirely fit in the transmit buffer
   * (for stream sockets), -1 is returned and SocketErrno is set 
   * to ERROR_MSGSIZE.  If the packet does not fit, the caller can
   * split the Packet (based on information obtained from 
   * GetTxAvailable) and reattempt to send the data.
   *
   * The flags argument is formed by or'ing one or more of the values:
   *        MSG_OOB        process out-of-band data 
   *        MSG_DONTROUTE  bypass routing, use direct interface 
   * These flags are _unsupported_ as of ns-3.1.
   *
   * \param p ns3::Packet to send
   * \param flags Socket control flags
   * \returns the number of bytes accepted for transmission if no error
   *          occurs, and -1 otherwise.
   *
   * \see SetSendCallback
   */
  virtual int Send (Ptr<Packet> p, uint32_t flags);

  int Send (Ptr<Packet> p);

  /**
   * \brief Send data to a specified peer.
   *
   * This method has similar semantics to Send () but subclasses may
   * want to provide checks on socket state, so the implementation is
   * pushed to subclasses.
   *
   * \param p packet to send
   * \param flags Socket control flags
   * \param toAddress IP Address of remote host
   * \returns -1 in case of error or the number of bytes copied in the 
   *          internal buffer and accepted for transmission.
   */
  virtual int SendTo (Ptr<Packet> p, uint32_t flags, 
                      const Address &toAddress);

  /**
   * Return number of bytes which can be returned from one or 
   * multiple calls to Recv.
   * Must be possible to call this method from the Recv callback.
   *
   * \returns the number of bytes which can be returned from one or
   *          multiple Recv calls.
   */
  virtual uint32_t GetRxAvailable (void) const;

  /**
   * \brief Read data from the socket
   *
   * This function matches closely in semantics to the recv() function
   * call in the standard C library (libc):
   *   ssize_t recv (int s, void *buf, size_t len, int flags);
   * except that the receive I/O is asynchronous.  This is the
   * primary Recv method at this low-level API and must be implemented 
   * by subclasses.
   * 
   * This method is normally used only on a connected socket.
   * In a typical blocking sockets model, this call would block until
   * at least one byte is returned or the connection closes.
   * In ns-3 at this API, the call returns immediately in such a case
   * and returns 0 if nothing is available to be read.
   * However, an application can set a callback, ns3::SetRecvCallback,
   * to be notified of data being available to be read
   * (when it conceptually unblocks); this is an asynchronous
   * I/O model for recv().
   * 
   * This variant of Recv() uses class ns3::Packet to encapsulate
   * data, rather than providing a raw pointer and length field.
   * This allows an ns-3 application to attach tags if desired (such
   * as a flow ID) and may allow the simulator to avoid some data
   * copies.  Despite the appearance of receiving Packets on a stream
   * socket, just think of it as a fancy byte buffer with streaming
   * semantics.
   *
   * The semantics depend on the type of socket.  For a datagram socket,
   * each Recv() returns the data from at most one Send(), and order
   * is not necessarily preserved.  For a stream socket, the bytes
   * are delivered in order, and on-the-wire packet boundaries are
   * not preserved.
   * 
   * The flags argument is formed by or'ing one or more of the values:
   *        MSG_OOB             process out-of-band data
   *        MSG_PEEK            peek at incoming message
   * None of these flags are supported for now.
   *
   * Some variants of Recv() are supported as additional API,
   * including RecvFrom(), overloaded Recv() without arguments,
   * and variants that use raw character buffers.
   *
   * \param maxSize reader will accept packet up to maxSize
   * \param flags Socket control flags
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet conforming
   * to the maxSize and flags.
   *
   * \see SetRecvCallback
   */
  virtual Ptr<Packet> Recv (uint32_t maxSize, uint32_t flags);

  /**
   * \brief Read a single packet from the socket and retrieve the sender 
   * address.
   *
   * Calls Recv(maxSize, flags) with maxSize
   * implicitly set to maximum sized integer, and flags set to zero.
   *
   * This method has similar semantics to Recv () but subclasses may
   * want to provide checks on socket state, so the implementation is
   * pushed to subclasses.
   *
   * \param maxSize reader will accept packet up to maxSize
   * \param flags Socket control flags
   * \param fromAddress output parameter that will return the
   * address of the sender of the received packet, if any.  Remains
   * untouched if no packet is received.
   * \returns Ptr<Packet> of the next in-sequence packet.  Returns
   * 0 if the socket cannot return a next in-sequence packet.
   */
  virtual Ptr<Packet> RecvFrom (uint32_t maxSize, uint32_t flags,
                                Address &fromAddress);

  /**
   * \brief Get socket address.
   * \param address the address name this socket is associated with.
   * \returns 0 if success, -1 otherwise
   */
  virtual int GetSockName (Address &address) const; 

  /**
   * \brief Get the peer address of a connected socket.
   * \param address the address this socket is connected to.
   * \returns 0 if success, -1 otherwise
   */
  virtual int GetPeerName (Address &address) const;

  /**
   * \brief Configure whether broadcast datagram transmissions are allowed
   *
   * This method corresponds to using setsockopt() SO_BROADCAST of
   * real network or BSD sockets.  If set on a socket, this option
   * will enable or disable packets to be transmitted to broadcast
   * destination addresses.
   *
   * \param allowBroadcast Whether broadcast is allowed
   * \return true if operation succeeds
   */
  virtual bool SetAllowBroadcast (bool allowBroadcast);

  /**
   * \brief Query whether broadcast datagram transmissions are allowed
   *
   * This method corresponds to using getsockopt() SO_BROADCAST of
   * real network or BSD sockets.
   *
   * \returns true if broadcast is allowed, false otherwise
   */
  virtual bool GetAllowBroadcast () const;


  
};

} // namspace  ns3

#endif // DCB_SOCKET_H
