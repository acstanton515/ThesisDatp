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
 
#ifndef __DATP_SCHEDULER_SIMPLE_H__
#define __DATP_SCHEDULER_SIMPLE_H__

#include "datp-headers.h"
#include "datp-scheduler.h"

namespace ns3 {

class DatpSchedulerSimple : public DatpScheduler
{
public:
  static TypeId GetTypeId (void);

  DatpSchedulerSimple ();
  virtual ~DatpSchedulerSimple ();
  
  uint32_t GetMessagesConcatenated ();
  uint32_t GetMessagesTotal ();
  Time GetSchedulerDelay ();

  virtual void ReceiveQuery (DatpHeader datpHeader);
  virtual void ReceiveNewMessage (DatpHeader datpHeader, Ptr<Packet> packet);
  virtual void ReceiveExistingMessage (DatpHeader datpHeader, Ptr<Packet> packet);

private:
  Time m_maximumHold;
  Time m_minimumHold;
  void MessageTimerExpired (void);
  
  uint32_t m_messagesConcatenated;
  uint32_t m_messagesTotal;
  Time m_schedulerDelay;

};

} // namespace ns3

#endif /* __DATP_SCHEDULER_SIMPLE_H__ */

