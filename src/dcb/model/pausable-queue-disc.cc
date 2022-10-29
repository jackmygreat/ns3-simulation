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

#include "pausable-queue-disc.h"
#include "dcb-traffic-control.h"
#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/fatal-error.h"
#include "ns3/log-macros-enabled.h"
#include "ns3/log.h"
#include "ns3/object-base.h"
#include "ns3/object-factory.h"
#include "ns3/queue-disc.h"
#include "ns3/queue-item.h"
#include "ns3/type-id.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PausableQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (PausableQueueDisc);

TypeId
PausableQueueDisc::GetTypeId ()
{
  static TypeId tid =
      TypeId ("ns3::PausableQueueDisc")
          .SetParent<QueueDisc> ()
          .SetGroupName ("Dcb")
          .AddConstructor<PausableQueueDisc> ()
          .AddAttribute ("PfcEnabled", "Wether PFC is enabled", BooleanValue (true),
                         MakeBooleanAccessor (&PausableQueueDisc::m_pfcEnabled),
                         MakeBooleanChecker ());
  return tid;
}

PausableQueueDisc::PausableQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

PausableQueueDisc::~PausableQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<PausableQueueDiscClass>
PausableQueueDisc::GetQueueDiscClass (std::size_t i) const
{
  NS_LOG_FUNCTION (this);
  return DynamicCast<PausableQueueDiscClass> (QueueDisc::GetQueueDiscClass (i));
}

void
PausableQueueDisc::Run ()
{
  NS_LOG_FUNCTION (this);
  // TODO: Not supporting Requeue () at this moment
  Ptr<QueueDiscItem> item = DoDequeue();
  if (item)
    {
      NS_ASSERT_MSG (m_send, "Send callback not set");
      m_send (item); // m_send is usually set to NetDevice::Send ()
    }
}

void
PausableQueueDisc::SetPaused (uint8_t priority, bool paused)
{
  NS_LOG_FUNCTION (this);
  GetQueueDiscClass (priority)->SetPaused (paused);
}

bool
PausableQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  // TODO: Use Classify to call PacketFilter

  CoSTag cosTag;
  if (item->GetPacket ()->RemovePacketTag (cosTag))
    {
      uint8_t priority = cosTag.GetCoS () & 0x0f;
      NS_ASSERT_MSG (priority < 8, "Priority should be 0~7 but here we have " << priority);
      bool retval = GetQueueDiscClass (priority)->GetQueueDisc ()->Enqueue (item);
      return retval;
    }
  NS_FATAL_ERROR ("No priority tag set one the item" << item);
  return false;
}

Ptr<QueueDiscItem>
PausableQueueDisc::DoDequeue ()
{
  NS_LOG_FUNCTION (this);
  Ptr<QueueDiscItem> item = 0;

  for (uint32_t i = 0; i < GetNQueueDiscClasses (); i++)
    {
      Ptr<PausableQueueDiscClass> qdclass = GetQueueDiscClass (i);
      if ((!m_pfcEnabled || !qdclass->IsPaused ()) &&
          (item = qdclass->GetQueueDisc ()->Dequeue ()) != 0)
        {
          NS_LOG_LOGIC ("Popoed from priority " << i << ": " << item);
          return item;
        }
    }
  NS_LOG_LOGIC ("Queue empty");
  return item;
}

Ptr<const QueueDiscItem>
PausableQueueDisc::DoPeek ()
{
  NS_LOG_FUNCTION (this);
  Ptr<const QueueDiscItem> item;

  for (uint32_t i = 0; i < GetNQueueDiscClasses (); i++)
    {
      Ptr<PausableQueueDiscClass> qdclass = GetQueueDiscClass (i);
      if ((!m_pfcEnabled || !qdclass->IsPaused ()) &&
          (item = qdclass->GetQueueDisc ()->Dequeue ()) != 0)
        {
          NS_LOG_LOGIC ("Peeked from priority " << i << ": " << item);
          return item;
        }
    }

  NS_LOG_LOGIC ("Queue empty");
  return item;
}

bool
PausableQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNInternalQueues () > 0)
    {
      NS_LOG_ERROR ("PausableQueueDisc cannot have internal queues");
      return false;
    }
  if (GetNQueueDiscClasses () == 0)
    {
      // create 8 fifo queue discs
      ObjectFactory factory;
      factory.SetTypeId ("ns3::FifoQueueDisc");
      for (uint8_t i = 0; i < 8; i++)
        {
          Ptr<QueueDisc> qd = factory.Create<QueueDisc> ();
          qd->Initialize ();
          Ptr<PausableQueueDiscClass> c = CreateObject<PausableQueueDiscClass> ();
          c->SetQueueDisc (qd);
          AddQueueDiscClass (c);
        }
    }
  return true;
}

void
PausableQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
PausableQueueDiscClass::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::PausableQueueDiscClass")
                          .SetParent<QueueDiscClass> ()
                          .SetGroupName ("Dcb")
                          .AddConstructor<PausableQueueDiscClass> ();
  return tid;
}

PausableQueueDiscClass::PausableQueueDiscClass () : m_isPaused (false)
{
  NS_LOG_FUNCTION (this);
}

PausableQueueDiscClass::~PausableQueueDiscClass ()
{
  NS_LOG_FUNCTION (this);
}

bool
PausableQueueDiscClass::IsPaused () const
{
  return m_isPaused;
}

void
PausableQueueDiscClass::SetPaused (bool paused)
{
  m_isPaused = paused;
}

} //namespace ns3
