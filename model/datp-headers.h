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

#ifndef __DATP_HEADER_H__ 
#define __DATP_HEADER_H__

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 { 

/**
* \ingroup datp header
* \brief   Datp flexible application header (* marks optional field)
      The size modifier field is not implemented
      Nor are the system specific fields implemented
      Note that as per DATP, all fields except HFF are optional
      Note that all field defaults are the same except the HFF and timestamp
      The HFF is implemented in the one byte form
      The timestamp is implemented with the same class used in ns-3
      The HFF can indicate the presence of another HFF with its final flag value
  \verbatim
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |       HFF     |          Size Modifiers (not included)        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            Origin*                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  Application* |    Priority*  |       Timestamp 00-15*        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                         Timestamp 16-47*                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |      Timestamp 48-63*         |        Message Length*        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            Sequence*                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |           System Specific Fields (not implemented)            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/
 
class DatpHeader : public Header
{
public:
  static TypeId GetTypeId (void);
  
  DatpHeader ();
  virtual ~DatpHeader ();
  
  uint8_t GetHeaderFieldFlags (void) const;
  
  //Size Modifier is left out... Consider replacing it with Another HFF field
  
  void SetOrigin (uint32_t origin);
  uint32_t GetOrigin (void) const;
  
  void SetApplication (uint8_t application);
  uint8_t GetApplication (void) const;
  
  void SetPriority (uint8_t size);
  uint8_t GetPriority (void) const;
  
  void SetTimestamp (uint64_t timestamp);
  uint64_t GetTimestamp (void) const;
  
  void SetDataLength (uint8_t dataLength);
  uint8_t GetDataLength (void) const;
  
  void SetSequence (uint32_t sequence);
  uint32_t GetSequence (void) const;
  
  uint8_t GetInternalHeaderSize (void) const;
  
  void SetInternalReceiveTime (Time receiveTime);
  Time GetInternalReceiveTime (void) const;
  
  void SetInternalMessageIdentifier (uint32_t messageIdentifier);
  uint32_t GetInternalMessageIdentifier (void) const;
  
  virtual bool operator< (DatpHeader const & datpHeader) const;
  
private:
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint8_t m_hff;
  // uint8_t m_sizeModifier;
  uint32_t m_origin;
  uint8_t m_application;
  uint8_t m_priority;
  uint64_t m_timestamp;
  uint8_t m_dataLength;
  uint32_t m_sequence;
  
  uint8_t m_internalHeaderSize;
  Time m_internalReceiveTime;
  uint32_t m_internalMessageIdentifier;

};


class DatpGenericApplicationDataHeader : public Header
{
public:
  static TypeId GetTypeId (void);
  
  DatpGenericApplicationDataHeader ();
  virtual ~DatpGenericApplicationDataHeader ();
  
  void SetValue (uint32_t value);
  uint32_t GetValue (void) const;
  
private:
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint32_t m_value;
};

} // namespace ns3

#endif /* __DATP_HEADER_H__ */