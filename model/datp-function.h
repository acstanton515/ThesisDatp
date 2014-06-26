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
 
#ifndef __DATP_FUNCTION_H__
#define __DATP_FUNCTION_H__

#include "datp-headers.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/object.h"

namespace ns3 {

class DatpFunction : public Object
{
public:
  static TypeId GetTypeId (void);

  DatpFunction ();
  virtual ~DatpFunction ();
  
  virtual void ReceiveQueryResponse (DatpHeader datpHeader, Ptr<Packet> packet) = 0;
  virtual void ReceiveNewMessage (DatpHeader datpHeader, Ptr<Packet> packet) = 0;
  
  void SetQueryCallback (Callback<void, DatpHeader > query);
  void SetNewMessageCallback (Callback<void, DatpHeader, Ptr<Packet> > newMessage);
  void SetExistingMessageCallback (Callback<void, DatpHeader, Ptr<Packet> > existingMessage);
  
protected:

  void NotifyQuery (DatpHeader datpHeader);
  void NotifyNewMessage (DatpHeader datpHeader, Ptr<Packet> packet);
  void NotifyExistingMessage (DatpHeader datpHeader, Ptr<Packet> packet);

  DatpHeader m_existingMessageDatpHeader;
  Ptr<Packet> m_existingMessagePacket;

private:
  
  Callback<void, DatpHeader > m_query;
  Callback<void, DatpHeader, Ptr<Packet> > m_newMessage;
  Callback<void, DatpHeader, Ptr<Packet> > m_existingMessage;
};

} // namespace ns3

#endif /* __DATP_FUNCTION_H__ */

