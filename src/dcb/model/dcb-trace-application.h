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

#ifndef WEBSEARCH_APPLICATION_H
#define WEBSEARCH_APPLICATION_H

#include "ns3/application.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"
#include "ns3/seq-ts-size-header.h"
#include "ns3/random-variable-stream.h"
#include "dcb-socket.h"
#include "ns3/dc-topology.h"
#include <set>

namespace ns3 {

class Socket;

class TraceApplication : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TraceApplication (Ptr<DcTopology> topology, uint32_t nodeIndex);
  virtual ~TraceApplication ();

  /**
  * \brief Assign a fixed random variable stream number to the random variables
  * used by this model.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  // int64_t AssignStreams (int64_t stream);

  struct Flow
  {
    const Time startTime;
    Time finishTime;
    const uint64_t totalBytes;
    uint64_t remainBytes;
    const Ptr<Socket> socket;
    Flow (uint64_t s, Time t, Ptr<Socket> sock)
      : startTime (t), totalBytes (s), remainBytes (s), socket (sock)
    {
    }
    void Dispose () // to provide a similar API as ns-3
    {
      socket->Close ();
      socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
      delete this; // TODO: is it ok to suicide here?
    }
  };

  void SetFlowMeanArriveInterval (double interval);

  typedef const std::vector<std::pair<double, double>> TraceCdf;

  void SetFlowCdf (const TraceCdf &cdf);

  void SetFlowDestination (const Address dest, const uint16_t port);

  constexpr static inline const uint64_t MSS = 1000; // bytes

  static inline TraceCdf TRACE_WEBSEARCH_CDF = {
      {0.,        0.00},
      {10000.,    0.15},
      {20000.,    0.20},
      {30000.,    0.30},
      {50000.,    0.40},
      {80000.,    0.53},
      {200000.,   0.60},
      {1000000.,  0.70},
      {2000000.,  0.80},
      {5000000.,  0.90},
      {10000000., 0.97},
      {30000000., 1.00}
  };

  static inline TraceCdf TRACE_FDHADOOP_CDF = {
      {0.,        0.00},
      {100.,      0.10},
      {200.,      0.20},
      {300.,      0.50},
      {350.,      0.15},
      {400.,      0.20},
      {500.,      0.30},
      {600.,      0.40},
      {700.,      0.50},
      {1000.,     0.60},
      {2000.,     0.67},
      {7000.,     0.70},
      {30000.,    0.72},
      {50000.,    0.82},
      {80000.,    0.87},
      {120000.,   0.90},
      {300000.,   0.95},
      {1000000.,  0.975},
      {2000000.,  0.99},
      {10000000., 1.00}
  };

private:
  // inherited from Application base class.
  virtual void StartApplication (void); // Called at time specified by Start
  virtual void StopApplication (void); // Called at time specified by Stop

  /**
   * \brief Schedule the next On period start
   */
  void ScheduleNextFlow (const Time &startTime);

  /**
   * \breif Create new socket destined to a random host.
   */
  Ptr<Socket> CreateNewRandomSocket ();

  /**
   * \brief Send a dummy packet according to m_remainBytes.
   * Raise error if packet does not sent successfully.
   * \param socket the socket to send packet.
   */
  void SendNextPacket (Flow *flow);

  /**
   * \brief Get next random flow start time.
   */
  Time GetNextFlowArriveInterval () const;

  /**
   * \brief Get next random flow size in bytes.
   */
  uint32_t GetNextFlowSize () const;

  //helpers
  /**
   * \brief Cancel all pending events.
   */
  // void CancelEvents ();

  /**
   * \brief Handle a Connection Succeed event
   * \param socket the connected socket
   */
  void ConnectionSucceeded (Ptr<Socket> socket);
  /**
   * \brief Handle a Connection Failed event
   * \param socket the not connected socket
   */
  void ConnectionFailed (Ptr<Socket> socket);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);
  
  // std::set<Flow *> m_flows;

  const Ptr<DcTopology>  m_topology;        //!< The topology
  const uint32_t         m_nodeIndex;       //!< Node index this application belongs to
  bool                   m_connected;       //!< True if connected
  DataRate               m_socketLinkRate;  //!< Link rate of the deice
  uint64_t               m_totBytes;        //!< Total bytes sent so far
  TypeId                 m_tid;             //!< Type of the socket used
  Ptr<EmpiricalRandomVariable>   m_flowSizeRng;       //!< Flow size random generator
  Ptr<ExponentialRandomVariable> m_flowArriveTimeRng; //!< Flow arrive time random generator
  Ptr<UniformRandomVariable>     m_hostIndexRng;      //!< Host index random generator

  /// traced Callback: transmitted packets.
  TracedCallback<Ptr<const Packet>> m_txTrace;

  /// Callbacks for tracing the packet Tx events, includes source and destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_txTraceWithAddresses;

  /// Callback for tracing the packet Tx events, includes source, destination, the packet sent, and header
  TracedCallback<Ptr<const Packet>, const Address &, const Address &, const SeqTsSizeHeader &>
      m_txTraceWithSeqTsSize;
};

} // namespace ns3

#endif // WEBSEARCH_APPLICATION_H