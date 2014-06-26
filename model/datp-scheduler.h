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
 
#ifndef __DATP_SCHEDULER_H__
#define __DATP_SCHEDULER_H__

#include "datp-headers.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/timer.h"
#include <map>

namespace ns3 {

class DatpScheduler : public Object
{
public:
  static TypeId GetTypeId (void);

  DatpScheduler ();
  virtual ~DatpScheduler ();

  virtual void ReceiveQuery (DatpHeader datpHeader) = 0;
  virtual void ReceiveNewMessage (DatpHeader datpHeader, Ptr<Packet> packet) = 0;
  virtual void ReceiveExistingMessage (DatpHeader datpHeader, Ptr<Packet> packet) = 0;
  
  void SetQueryResponseCallback (Callback<void, DatpHeader, Ptr<Packet> > queryResponse);
  void SetPacketEjectCallback (Callback<void, Ptr<Packet> > ejectPacket);

protected:

  void NotifyQueryResponse (DatpHeader datpHeader, Ptr<Packet> packet);
  void NotifyPacketEject (Ptr<Packet> packet);

  std::map<uint32_t,Ptr<Packet> > m_messageBuffer;
  std::map<uint32_t,Timer> m_timerBuffer;
  std::map<uint32_t,DatpHeader> m_headerBuffer;
  
private:

  Callback<void, DatpHeader, Ptr<Packet> > m_queryResponse;
  Callback<void, Ptr<Packet> > m_ejectPacket;

};

} // namespace ns3

#endif /* __DATP_SCHEDULER_H__ */

