/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Author: Andrew Stanton <acstanton515@gmail.com>
 */
 
#include "datp-scheduler-simple.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpSchedulerSimple");

NS_OBJECT_ENSURE_REGISTERED (DatpSchedulerSimple);

TypeId DatpSchedulerSimple::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpSchedulerSimple")
    .SetParent<DatpScheduler> ()
    .AddConstructor<DatpSchedulerSimple> ()
    .AddAttribute ("MaximumHold",
                   "The maximum time to hold a message", 
                   TimeValue (MilliSeconds (1)),
                   MakeTimeAccessor (&DatpSchedulerSimple::m_maximumHold),
                   MakeTimeChecker ())
    .AddAttribute ("MinimumHold",
                   "The minimum time to hold a message", 
                   TimeValue (MicroSeconds (500)),
                   MakeTimeAccessor (&DatpSchedulerSimple::m_minimumHold),
                   MakeTimeChecker ())
  ;
  return tid;
}

DatpSchedulerSimple::DatpSchedulerSimple ()
{
  NS_LOG_FUNCTION (this);
  m_messagesConcatenated=0;
  m_messagesTotal=0;
  m_schedulerDelay = Seconds (0.0);
}

DatpSchedulerSimple::~DatpSchedulerSimple()
{
  NS_LOG_FUNCTION (this);
}

uint32_t 
DatpSchedulerSimple::GetMessagesConcatenated ()
{
  return m_messagesConcatenated;
}

uint32_t
DatpSchedulerSimple::GetMessagesTotal ()
{
  return m_messagesTotal;
}

Time 
DatpSchedulerSimple::GetSchedulerDelay ()
{
  return m_schedulerDelay;
}

void 
DatpSchedulerSimple::ReceiveQuery (DatpHeader datpHeader)
{
  NS_LOG_FUNCTION (this);
  DatpHeader existingDatpHeader;
  Ptr<Packet> existingPacket = NULL;
  
  for (std::map<uint32_t,DatpHeader> ::iterator it = m_headerBuffer.begin (); it != m_headerBuffer.end (); ++it)
    {
      if (it->second.GetApplication () == datpHeader.GetApplication ())
        {
          existingDatpHeader = it->second;
          existingPacket = m_messageBuffer[it->first];
          break;
        }
    }

  NotifyQueryResponse( existingDatpHeader, existingPacket);
}

void 
DatpSchedulerSimple::ReceiveNewMessage (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  
  uint32_t mId = datpHeader.GetInternalMessageIdentifier ();
  NS_ASSERT (mId != 0);
  NS_ASSERT (m_messageBuffer.count (mId) == 0);
  m_headerBuffer[mId] = datpHeader;
  
  uint32_t sizeBefore = m_messageBuffer.size ();
  m_messageBuffer[mId] = packet;
  uint32_t sizeAfter = m_messageBuffer.size ();
  NS_LOG_INFO ("Buffer Add: Size from " << sizeBefore << " to " << sizeAfter << " mId=" << mId);
  NS_ASSERT (sizeBefore != sizeAfter);

  Timer messageEjectTimer (Timer::CANCEL_ON_DESTROY);
  m_timerBuffer[mId] = messageEjectTimer;
  m_timerBuffer[mId].SetFunction (&DatpSchedulerSimple::MessageTimerExpired, this);
  m_timerBuffer[mId].SetDelay (m_maximumHold);
  m_timerBuffer[mId].Schedule ();
}

void 
DatpSchedulerSimple::ReceiveExistingMessage (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  //should already exist - but lets check anyway
  uint32_t mId = datpHeader.GetInternalMessageIdentifier ();
  NS_ASSERT (m_messageBuffer.count (mId));
  m_messageBuffer[mId] = packet;
  m_headerBuffer[mId] = datpHeader;
}

void 
DatpSchedulerSimple::MessageTimerExpired (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> ejectPacket = Create<Packet> (0);
  bool didTimerExpire = false;
  std::vector<uint32_t> timersToErase;
  for (std::map<uint32_t,Timer >::iterator it = m_timerBuffer.begin (); it != m_timerBuffer.end (); ++it)
    {
      if (it->second.GetDelayLeft () < m_minimumHold)
        {
          NS_LOG_INFO ("Eject Message: mId=" << it->first);
          NS_ASSERT (m_messageBuffer.count (it->first));
          
          DatpGenericApplicationDataHeader dataHeader;
          m_messageBuffer[it->first]->PeekHeader (dataHeader);
          
          m_messageBuffer[it->first]->AddHeader(m_headerBuffer[it->first]);
          ejectPacket->AddAtEnd (m_messageBuffer[it->first]);
          
          if (dataHeader.GetValue () > 0)
            {
              uint64_t timeDifference = Simulator::Now ().GetNanoSeconds () - m_headerBuffer[it->first].GetInternalReceiveTime ().GetNanoSeconds ();
              m_schedulerDelay += NanoSeconds (timeDifference * dataHeader.GetValue ());
              m_messagesTotal += dataHeader.GetValue ();
            }
          else
            {
              m_schedulerDelay += Simulator::Now () - m_headerBuffer[it->first].GetInternalReceiveTime ();
              m_messagesTotal += 1;
            }
          if (didTimerExpire == true)
            m_messagesConcatenated++;
          didTimerExpire = true;
          
          uint32_t sizeBefore = m_messageBuffer.size ();
          uint32_t sizeBefore2 = m_headerBuffer.size ();
          it->second.Cancel ();
          m_messageBuffer.erase (it->first);
          m_headerBuffer.erase (it->first);
          // m_timerBuffer.erase (it);    //cannot erase now, since we have an active iterator
          timersToErase.push_back(it->first);
          uint32_t sizeAfter = m_messageBuffer.size ();
          uint32_t sizeAfter2 = m_headerBuffer.size ();

          NS_LOG_INFO ("Buffer Remove: Size from " << sizeBefore << " " << sizeBefore2 << " (packet, timer) to " << sizeAfter << " " << sizeAfter2);
          NS_ASSERT (sizeBefore != sizeAfter);
          NS_ASSERT (sizeBefore2 != sizeAfter2);
        }
        
    } 
  for (std::vector<uint32_t>::iterator it = timersToErase.begin (); it != timersToErase.end (); ++it)
    {
      m_timerBuffer.erase (*it);
    }
  NS_ASSERT (didTimerExpire);
  NS_LOG_INFO ("Packet Eject!");
  NotifyPacketEject (ejectPacket);
}

} // namespace ns3

