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
 
#include "datp-scheduler.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpScheduler");

NS_OBJECT_ENSURE_REGISTERED (DatpScheduler);

TypeId DatpScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpScheduler")
    .SetParent<Object> ()
  ;
  return tid;
}

DatpScheduler::DatpScheduler ()
{
  NS_LOG_FUNCTION (this);
}

DatpScheduler::~DatpScheduler()
{
  NS_LOG_FUNCTION (this);
}

void 
DatpScheduler::SetQueryResponseCallback (Callback<void, DatpHeader, Ptr<Packet> > queryResponse)
{
  NS_LOG_FUNCTION (this << &queryResponse);
  m_queryResponse = queryResponse;
}

void 
DatpScheduler::SetPacketEjectCallback (Callback<void, Ptr<Packet> > ejectPacket)
{
  NS_LOG_FUNCTION (this << &ejectPacket);
  m_ejectPacket = ejectPacket;
}

void 
DatpScheduler::NotifyQueryResponse (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  if (!m_queryResponse.IsNull ())
    m_queryResponse (datpHeader, packet);
}

void
DatpScheduler::NotifyPacketEject (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  if (!m_ejectPacket.IsNull ())
    m_ejectPacket (packet);
}

} // namespace ns3

