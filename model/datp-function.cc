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
 
#include "datp-function.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpFunction");

NS_OBJECT_ENSURE_REGISTERED (DatpFunction);

TypeId DatpFunction::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpFunction")
    .SetParent<Object> ()
  ;
  return tid;
}

DatpFunction::DatpFunction ()
{
  NS_LOG_FUNCTION (this);
}

DatpFunction::~DatpFunction()
{
  NS_LOG_FUNCTION (this);
}

void 
DatpFunction::SetQueryCallback (Callback<void, DatpHeader > query)
{
  NS_LOG_FUNCTION (this << &query);
  m_query = query;
}

void 
DatpFunction::SetNewMessageCallback (Callback<void, DatpHeader, Ptr<Packet> > newMessage)
{
  NS_LOG_FUNCTION (this << &newMessage);
  m_newMessage = newMessage;
}

void 
DatpFunction::SetExistingMessageCallback (Callback<void, DatpHeader, Ptr<Packet> > existingMessage)
{
  NS_LOG_FUNCTION (this << &existingMessage);
  m_existingMessage = existingMessage;
}

void 
DatpFunction::NotifyQuery (DatpHeader datpHeader)
{
  NS_LOG_FUNCTION (this);
  if (!m_query.IsNull ())
    m_query (datpHeader);
}

void 
DatpFunction::NotifyNewMessage (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  if (!m_newMessage.IsNull ())
    m_newMessage (datpHeader, packet);
}

void 
DatpFunction::NotifyExistingMessage (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  if (!m_existingMessage.IsNull ())
    m_existingMessage (datpHeader, packet);
}

} // namespace ns3

