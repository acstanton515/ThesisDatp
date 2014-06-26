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

#include "ns3/buffer.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/nstime.h"
#include "datp-headers.h"

NS_LOG_COMPONENT_DEFINE ("DatpHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DatpHeader);

TypeId
DatpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpHeader")
    .SetParent<Header> ()
    .AddConstructor<DatpHeader> ()
  ;
  return tid;
}

DatpHeader::DatpHeader ()
  : m_hff (128),
    //m_sizeModifier (0),
    m_origin (0),
    m_application (0),
    m_priority (0),
    m_timestamp (Seconds (0.0).GetNanoSeconds ()),   // or auto set time ---> Simulator::Now ().GetTimeStep ()
    m_dataLength (0),
    m_sequence (0),
    m_internalHeaderSize (1),
    m_internalReceiveTime (Seconds (0.0)),
    m_internalMessageIdentifier (0)
{
  NS_LOG_FUNCTION (this);
}

DatpHeader::~DatpHeader()
{
  NS_LOG_FUNCTION (this);
}

uint8_t 
DatpHeader::GetHeaderFieldFlags (void) const
{
  return m_hff;
}

void 
DatpHeader::SetOrigin (uint32_t origin)
{
  m_origin = origin;
  if (!(m_hff&64))
    { 
      m_hff += 64;
      m_internalHeaderSize += 4;
    }
}

uint32_t 
DatpHeader::GetOrigin (void) const
{
  return m_origin;
}

void 
DatpHeader::SetApplication (uint8_t application)
{
  m_application = application;
  if (!(m_hff&32))
    { 
      m_hff += 32;
      m_internalHeaderSize += 1;
    }
}

uint8_t 
DatpHeader::GetApplication (void) const
{
  return m_application;
}

void 
DatpHeader::SetPriority (uint8_t priority)
{
  m_priority = priority;
  if (!(m_hff&16))
    { 
      m_hff += 16;
      m_internalHeaderSize += 1;
    }
}

uint8_t 
DatpHeader::GetPriority (void) const
{
  return m_priority;
}

void 
DatpHeader::SetTimestamp (uint64_t timestamp)
{
  m_timestamp = timestamp;
  if (!(m_hff&8))
    { 
      m_hff += 8;
      m_internalHeaderSize += 8;
    }
}

uint64_t 
DatpHeader::GetTimestamp (void) const
{
  return m_timestamp;
}

void 
DatpHeader::SetDataLength (uint8_t dataLength)
{
  m_dataLength = dataLength;
  if (!(m_hff&4))
    { 
      m_hff += 4;
      m_internalHeaderSize += 1;
    }
}

uint8_t 
DatpHeader::GetDataLength (void) const
{
  return m_dataLength;
}

void 
DatpHeader::SetSequence (uint32_t sequence)
{
  m_sequence = sequence;
  if (!(m_hff&2))
    { 
      m_hff += 2;
      m_internalHeaderSize += 4;
    }
}

uint32_t 
DatpHeader::GetSequence (void) const
{
  return m_sequence;
}

uint8_t 
DatpHeader::GetInternalHeaderSize (void) const
{
  return m_internalHeaderSize;
}

void 
DatpHeader::SetInternalReceiveTime (Time receiveTime)
{
  m_internalReceiveTime = receiveTime;
}

Time 
DatpHeader::GetInternalReceiveTime (void) const
{
  return m_internalReceiveTime;
}

void 
DatpHeader::SetInternalMessageIdentifier (uint32_t messageIdentifier)
{
  m_internalMessageIdentifier = messageIdentifier;
}

uint32_t 
DatpHeader::GetInternalMessageIdentifier (void) const
{
  return m_internalMessageIdentifier;
}

bool
DatpHeader::operator< (DatpHeader const & datpHeader) const
{
  return (GetInternalMessageIdentifier () < datpHeader.GetInternalMessageIdentifier ());
}

TypeId
DatpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
DatpHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "Datp Header: " << m_origin << m_application << m_priority << m_timestamp << m_dataLength << m_sequence;
}

uint32_t
DatpHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_internalHeaderSize;  //max size 20, min 1
}

void
DatpHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteU8 (m_hff);
  if (m_hff&64)
     i.WriteHtonU32 (m_origin);
  if (m_hff&32)
    i.WriteU8 (m_application);
  if (m_hff&16)
    i.WriteU8 (m_priority);
  if (m_hff&8)
    i.WriteHtonU64 (m_timestamp);
  if (m_hff&4)
    i.WriteU8 (m_dataLength);
  if (m_hff&2)
    i.WriteHtonU32 (m_sequence);
}

uint32_t
DatpHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_hff = i.ReadU8 ();  //might need error checking to ensure we get every piece
  NS_LOG_INFO ("Deserialized HFF = "<<(bool)(m_hff&128)<<(bool)(m_hff&64)
                                    <<(bool)(m_hff&32)<<(bool)(m_hff&16)<<(bool)(m_hff&8)
                                    <<(bool)(m_hff&4)<<(bool)(m_hff&2)<<(bool)(m_hff&1));
  m_internalHeaderSize = 1;
  if (m_hff&64)
    {
      m_origin = i.ReadNtohU32 ();
      m_internalHeaderSize += 4;
    }
  if (m_hff&32)
    {
      m_application = i.ReadU8 ();
      m_internalHeaderSize += 1;
    }
  if (m_hff&16)
    {
      m_priority = i.ReadU8 ();
      m_internalHeaderSize += 1;
    }
  if (m_hff&8)
    {
      m_timestamp = i.ReadNtohU64 ();
      m_internalHeaderSize += 8;
    }
  if (m_hff&4)
    {
      m_dataLength = i.ReadU8 ();
      m_internalHeaderSize += 1;
    }
  if (m_hff&2)
    {
      m_sequence = i.ReadNtohU32 ();
      m_internalHeaderSize += 4;
    }
  NS_LOG_INFO ("Deserialized Datp Header: " << (uint32_t) m_application 
                                            << " " << (uint32_t) m_priority 
                                            << " " << m_timestamp 
                                            << " " << (uint32_t) m_dataLength 
                                            << " " << m_sequence);
  
  return GetSerializedSize ();
}




NS_OBJECT_ENSURE_REGISTERED (DatpGenericApplicationDataHeader);

TypeId
DatpGenericApplicationDataHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpGenericApplicationDataHeader")
    .SetParent<Header> ()
    .AddConstructor<DatpGenericApplicationDataHeader> ()
  ;
  return tid;
}

DatpGenericApplicationDataHeader::DatpGenericApplicationDataHeader ()
  : m_value (0)
{
  NS_LOG_FUNCTION (this);
}

DatpGenericApplicationDataHeader::~DatpGenericApplicationDataHeader()
{
  NS_LOG_FUNCTION (this);
}

void 
DatpGenericApplicationDataHeader::SetValue (uint32_t value)
{
  m_value = value;
}

uint32_t 
DatpGenericApplicationDataHeader::GetValue (void) const
{
  return m_value;
}

TypeId
DatpGenericApplicationDataHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
DatpGenericApplicationDataHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "Datp Generic Application Data Header: " << m_value;
}

uint32_t
DatpGenericApplicationDataHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 4;  //uint32_t
}

void
DatpGenericApplicationDataHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  
  i.WriteHtonU32 (m_value);
}

uint32_t
DatpGenericApplicationDataHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;

  m_value = i.ReadNtohU32 ();
  
  return GetSerializedSize ();
}

} // namespace ns3
