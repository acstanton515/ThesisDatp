/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 *                      
 */
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "datp-application.h"
#include "datp-headers.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpApplication");

NS_OBJECT_ENSURE_REGISTERED (DatpApplication);

TypeId
DatpApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpApplication")
    .SetParent<Application> ()
  ;
  return tid;
}

DatpApplication::DatpApplication ()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_sendEvent = EventId ();
  m_peerAddress.Set ("127.0.0.1");
  m_peerPort = 9999;
  m_messagesSent = 0;
  m_bytesSent = 0;
  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
}

DatpApplication::~DatpApplication ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t 
DatpApplication::GetMessagesSent (void)
{
  return m_messagesSent;
}

uint32_t 
DatpApplication::GetBytesSent (void)
{
  return m_bytesSent;
}

void
DatpApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
DatpApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  m_origin = GetNode ()->GetId ();
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      // InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   // m_port);
      // m_socket->Bind (local);
      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  uint32_t randomDelay = m_uniformRandomVariable->GetInteger (0, 200) * 20 + 1000;  //1ms-5ms
  m_sendEvent = Simulator::Schedule (MicroSeconds (randomDelay), &DatpApplication::Send, this);
}

void
DatpApplication::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}




NS_OBJECT_ENSURE_REGISTERED (DatpApplicationOne);

TypeId
DatpApplicationOne::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpApplicationOne")
    .SetParent<DatpApplication> ()
    .AddConstructor<DatpApplicationOne> ()
    .AddAttribute ("Interval",
                   "The time to wait between sends", 
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&DatpApplicationOne::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("DataLength",
                   "Length of data payload generated apart from headers",
                   UintegerValue (20),
                   MakeUintegerAccessor (&DatpApplicationOne::m_dataLength),
                   MakeUintegerChecker<uint32_t> (0,1476))
  ;
  return tid;
}

DatpApplicationOne::DatpApplicationOne ()
{
  NS_LOG_FUNCTION (this);
  m_application = 1;
  m_priority = 0;
}

DatpApplicationOne::~DatpApplicationOne ()
{
  NS_LOG_FUNCTION (this);
}

void
DatpApplicationOne::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  DatpHeader datpHeader;
  // datpHeader.SetOrigin (m_origin);
  datpHeader.SetApplication (m_application);
  datpHeader.SetTimestamp (Simulator::Now ().GetNanoSeconds ());
  datpHeader.SetPriority (m_priority);
  datpHeader.SetDataLength (m_dataLength);
  // datpHeader.SetSequence (m_messagesSent);  
  Ptr<Packet> p = Create<Packet> (m_dataLength);
  p->AddHeader (datpHeader);
  if ((m_socket->Send (p)) >= 0)
    {
      ++m_messagesSent;
      m_bytesSent += p->GetSize ();
    }
  m_sendEvent = Simulator::Schedule (m_interval, &DatpApplicationOne::Send, this);
}




NS_OBJECT_ENSURE_REGISTERED (DatpApplicationTwo);

TypeId
DatpApplicationTwo::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpApplicationTwo")
    .SetParent<DatpApplication> ()
    .AddConstructor<DatpApplicationTwo> ()
    .AddAttribute ("Interval",
                   "The time to wait between sends",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&DatpApplicationTwo::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("DataLength",
                   "Length of data payload generated apart from headers",
                   UintegerValue (60),
                   MakeUintegerAccessor (&DatpApplicationTwo::m_dataLength),
                   MakeUintegerChecker<uint32_t> (0,1476))
  ;
  return tid;
}

DatpApplicationTwo::DatpApplicationTwo ()
{
  NS_LOG_FUNCTION (this);
  m_application = 2;
  m_priority = 0;
}

DatpApplicationTwo::~DatpApplicationTwo ()
{
  NS_LOG_FUNCTION (this);
}

void
DatpApplicationTwo::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  DatpHeader datpHeader;
  // datpHeader.SetOrigin (m_origin);
  datpHeader.SetApplication (m_application);
  datpHeader.SetTimestamp (Simulator::Now ().GetNanoSeconds ());
  datpHeader.SetPriority (m_priority);
  datpHeader.SetDataLength (m_dataLength);
  // datpHeader.SetSequence (m_messagesSent);  
  Ptr<Packet> p = Create<Packet> (m_dataLength);
  p->AddHeader (datpHeader);
  if ((m_socket->Send (p)) >= 0)
    {
      ++m_messagesSent;
      m_bytesSent += p->GetSize ();
    }
  m_sendEvent = Simulator::Schedule (m_interval, &DatpApplicationTwo::Send, this);
}




NS_OBJECT_ENSURE_REGISTERED (DatpApplicationThree);

TypeId
DatpApplicationThree::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpApplicationThree")
    .SetParent<DatpApplication> ()
    .AddConstructor<DatpApplicationThree> ()
    .AddAttribute ("Interval",
                   "The time to wait between sends",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&DatpApplicationThree::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("DataLength",
                   "Length of data payload generated apart from headers",
                   UintegerValue (40),
                   MakeUintegerAccessor (&DatpApplicationThree::m_dataLength),
                   MakeUintegerChecker<uint32_t> (0,1476))
  ;
  return tid;
}

DatpApplicationThree::DatpApplicationThree ()
{
  NS_LOG_FUNCTION (this);
  m_application = 3;
  m_priority = 0;
}

DatpApplicationThree::~DatpApplicationThree ()
{
  NS_LOG_FUNCTION (this);
}

void
DatpApplicationThree::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  DatpHeader datpHeader;
  // datpHeader.SetOrigin (m_origin);
  datpHeader.SetApplication (m_application);
  datpHeader.SetTimestamp (Simulator::Now ().GetNanoSeconds ());
  datpHeader.SetPriority (m_priority);
  datpHeader.SetDataLength (m_dataLength);
  // datpHeader.SetSequence (m_messagesSent);  
  Ptr<Packet> p = Create<Packet> (m_dataLength);
  p->AddHeader (datpHeader);
  if ((m_socket->Send (p)) >= 0)
    {
      ++m_messagesSent;
      m_bytesSent += p->GetSize ();
    }
  m_sendEvent = Simulator::Schedule (m_interval, &DatpApplicationThree::Send, this);
}

} // Namespace ns3