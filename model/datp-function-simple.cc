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
 
#include "datp-function-simple.h"
#include "ns3/log.h"
#include "ns3/assert.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpFunctionSimple");

NS_OBJECT_ENSURE_REGISTERED (DatpFunctionSimple);

TypeId DatpFunctionSimple::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpFunctionSimple")
    .SetParent<DatpFunction> ()
    .AddConstructor<DatpFunctionSimple> ()
  ;
  return tid;
}

DatpFunctionSimple::DatpFunctionSimple ()
{
  NS_LOG_FUNCTION (this);
  m_messagesMerged=0;
  m_bytesMerged=0;
}

DatpFunctionSimple::~DatpFunctionSimple()
{
  NS_LOG_FUNCTION (this);
}

uint32_t 
DatpFunctionSimple::GetMessagesMerged ()
{
  return m_messagesMerged;
}

uint32_t 
DatpFunctionSimple::GetBytesMerged ()
{
  return m_bytesMerged;
}


void 
DatpFunctionSimple::ReceiveQueryResponse (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Query response: app=" << (uint32_t) datpHeader.GetApplication ());
  m_existingMessageDatpHeader = datpHeader;
  m_existingMessagePacket = packet;
}

void 
DatpFunctionSimple::ReceiveNewMessage (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  //should be a valid new header and data from aggregator receiver
  
  //query scheduler to see if same application has an existing message
  NotifyQuery (datpHeader);
  
  if (m_existingMessageDatpHeader.GetInternalMessageIdentifier () != 0 && m_existingMessagePacket != NULL)
    {
      NS_LOG_INFO ("We got an existing message on app " << (uint32_t) datpHeader.GetApplication () 
                                                        << " Ids: " 
                                                        << m_existingMessageDatpHeader.GetInternalMessageIdentifier ()
                                                        << " "
                                                        << datpHeader.GetInternalMessageIdentifier ());
      Ptr<Packet> newPacket = Create<Packet> (0);
      //add up the data in 4 byte segments with the two data segments aligned... 
      //if any extra 4 byte segments, keep them, if any remaining bytes less than four, discard
      uint32_t c1 = 0;
      uint32_t c2 = 0;
      while (packet->GetSize () > 0 || m_existingMessagePacket->GetSize () > 0)
        {
          DatpGenericApplicationDataHeader value;
          DatpGenericApplicationDataHeader value2;
          uint16_t headerBytesRemoved = packet->RemoveHeader (value);
          uint16_t headerBytesRemoved2 = m_existingMessagePacket->RemoveHeader (value2);
          NS_LOG_INFO ("Existing Message: " << value2.GetValue () << " New Message : " << value.GetValue ());
          if (headerBytesRemoved == 4 && headerBytesRemoved2 == 4)
            {
              if (value.GetValue () == 0 && value2.GetValue () == 0)
                {
                  value.SetValue (2);
                  c1 = c2 = 1;
                }
              else if (value.GetValue () == 0 || value2.GetValue () == 0)
                if (value.GetValue () == 0)
                  {
                    value.SetValue (value2.GetValue () + 1);
                    c1 = 1;
                    c2 = value2.GetValue ();
                  }
                else
                  {
                    value.SetValue (value.GetValue () + 1);
                    c2 = 1;
                    c1 = value.GetValue ();
                  }
              else 
                {
                  value.SetValue (value.GetValue () + value2.GetValue ());
                  c1 = value.GetValue ();
                  c2 = value2.GetValue ();
                }
                
              newPacket->AddHeader (value);
              NS_LOG_INFO ("Merged Message: " << value.GetValue ());
            }
          else 
            {
              NS_ASSERT_MSG (false, "Uneven data in messages for function to parse");
            }
          m_bytesMerged += 4;
        }
      
      uint64_t timestamp = (datpHeader.GetTimestamp () * c1 + m_existingMessageDatpHeader.GetTimestamp () * c2) / (c1 + c2);
      NS_LOG_INFO ("Timestamp Result: " << NanoSeconds (timestamp).GetSeconds () 
                << " = New: "           << NanoSeconds (datpHeader.GetTimestamp ()).GetSeconds ()
                << " + Existing: "      << NanoSeconds (m_existingMessageDatpHeader.GetTimestamp ()).GetSeconds ());
      m_existingMessageDatpHeader.SetTimestamp (timestamp);
      timestamp = (datpHeader.GetInternalReceiveTime ().GetNanoSeconds () * c1 + m_existingMessageDatpHeader.GetInternalReceiveTime ().GetNanoSeconds () * c2) / (c1 + c2);
      m_existingMessageDatpHeader.SetInternalReceiveTime (NanoSeconds (timestamp));

      m_messagesMerged++;
      m_bytesMerged += datpHeader.GetInternalHeaderSize ();
      NotifyExistingMessage (m_existingMessageDatpHeader, newPacket);
    }
  else
    {
      NS_LOG_INFO ("We got a new message with Id: " << datpHeader.GetInternalMessageIdentifier () << " and app=" << (uint32_t) datpHeader.GetApplication ());
      NotifyNewMessage (datpHeader, packet);
    }
}

} // namespace ns3

