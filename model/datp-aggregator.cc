/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "datp-aggregator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpAggregator");
NS_OBJECT_ENSURE_REGISTERED (DatpAggregator);


TypeId
DatpAggregator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpAggregator")
    .SetParent<Application> ()
    .AddConstructor<DatpAggregator> ()
    .AddAttribute ("SchedulerOn",
                   "Turn scheduler on, otherwise just pass through",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DatpAggregator::m_schedulerOn),
                   MakeBooleanChecker ())
    .AddAttribute ("FunctionOn",
                   "Turn function on, otherwise just pass through",
                   BooleanValue (true),
                   MakeBooleanAccessor (&DatpAggregator::m_functionOn),
                   MakeBooleanChecker ())
    .AddAttribute ("CollectorAddress",
                   "The address of the collector in the aggregation system",
                   AddressValue (),
                   MakeAddressAccessor (&DatpAggregator::m_collectorAddress),
                   MakeAddressChecker ()) 
    .AddAttribute ("AggregatorPort",
                   "Set UDP port number for aggregation system",
                   UintegerValue (9999),
                   MakeUintegerAccessor (&DatpAggregator::m_aggregatorPort),
                   MakeUintegerChecker<uint16_t> (1024)) 
    .AddAttribute ("TreeControllerType",
                   "Set type of tree controller to use",
                   TypeIdValue (DatpTreeControllerAodv::GetTypeId ()),   //needs to be set to a child of class tree controller
                   MakeTypeIdAccessor (&DatpAggregator::m_treeControllerTypeId),
                   MakeTypeIdChecker ())  
    .AddAttribute ("FunctionType",
                   "Set type of function to use",
                   TypeIdValue (DatpFunctionSimple::GetTypeId ()),   //needs to be set to a child of class function
                   MakeTypeIdAccessor (&DatpAggregator::m_functionTypeId),
                   MakeTypeIdChecker ())    
    .AddAttribute ("SchedulerType",
                   "Set type of scheduler to use",
                   TypeIdValue (DatpSchedulerSimple::GetTypeId ()),   //needs to be set to a child of class scheduler
                   MakeTypeIdAccessor (&DatpAggregator::m_schedulerTypeId),
                   MakeTypeIdChecker ())                   
  ;
  return tid;
}

DatpAggregator::DatpAggregator ()
{
  NS_LOG_FUNCTION (this);
  m_isInstalled = false;
  m_packetsSent = 0;
  m_bytesSent = 0;
  m_packetsSentFailure = 0;
  m_packetsReceived = 0;
  m_messagesReceived = 0;
  m_bytesReceived = 0;
}

DatpAggregator::~DatpAggregator ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t 
DatpAggregator::GetPacketsSent (void)
{
  return m_packetsSent;
}

uint32_t 
DatpAggregator::GetBytesSent (void)
{
  return m_bytesSent;
}

uint32_t 
DatpAggregator::GetPacketsSentFailure (void)
{
  return m_packetsSentFailure;
}

uint32_t 
DatpAggregator::GetPacketsReceived (void)
{
 return m_packetsReceived;
}

uint32_t 
DatpAggregator::GetMessagesReceived (void)
{
  return m_messagesReceived;
}

uint32_t 
DatpAggregator::GetBytesReceived (void)
{
  return m_bytesReceived;
}


void 
DatpAggregator::SetParentAggregatorAddress (Address parentAggregatorAddress)
{
  NS_LOG_FUNCTION (this << parentAggregatorAddress);
  m_parentAggregatorAddress = parentAggregatorAddress;
}

Address 
DatpAggregator::GetParentAggregatorAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_parentAggregatorAddress;
}

Address 
DatpAggregator::GetCollectorAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_collectorAddress;
}

void
DatpAggregator::Install (void)
{
  NS_LOG_FUNCTION (this);
  ObjectFactory factory;
  
  //Current aggregator logic does not support the scheduler off and function on
  //If the scheduler is off messages cannot be held onto and the function cannot operate on messages
  NS_ASSERT (!(m_schedulerOn == false && m_functionOn == true));
  

  factory.SetTypeId (m_treeControllerTypeId);
  factory.Set ("CollectorAddress", AddressValue (m_collectorAddress));
  m_treeController = factory.Create <DatpTreeController> ();
  m_treeController->SetParentAggregatorCallback (MakeCallback (&DatpAggregator::SetParentAggregatorAddress, this)); 
  
  factory.SetTypeId (m_schedulerTypeId);
  m_scheduler = factory.Create <DatpScheduler> ();
  GetNode ()->AggregateObject(m_scheduler);
  
  factory.SetTypeId (m_functionTypeId);
  m_function = factory.Create <DatpFunction> ();
  GetNode ()->AggregateObject(m_function);
  
  if (m_schedulerOn)
    {
      m_scheduler->SetPacketEjectCallback (MakeCallback (&DatpAggregator::Sender, this)); 

      if (m_functionOn)
        {     
          SetNextReceiverCallback (MakeCallback (&DatpFunction::ReceiveNewMessage, m_function));

          m_scheduler->SetQueryResponseCallback (MakeCallback (&DatpFunction::ReceiveQueryResponse, m_function)); 
          
          m_function->SetQueryCallback (MakeCallback (&DatpScheduler::ReceiveQuery, m_scheduler));
          m_function->SetNewMessageCallback (MakeCallback (&DatpScheduler::ReceiveNewMessage, m_scheduler));
          m_function->SetExistingMessageCallback (MakeCallback (&DatpScheduler::ReceiveExistingMessage, m_scheduler));
        }
      else
        {
          SetNextReceiverCallback (MakeCallback (&DatpScheduler::ReceiveNewMessage, m_scheduler));
        }
    }
  m_isInstalled = true;
}


Ptr<Application>
DatpAggregator::GetTreeControllerApplication (void) const
{
  NS_LOG_FUNCTION (this);
  return m_treeController;
}

void 
DatpAggregator::SetNextReceiverCallback (Callback<void, DatpHeader, Ptr<Packet> > nextReceiver)
{
  NS_LOG_FUNCTION (this << &nextReceiver);
  m_nextReceiver = nextReceiver;
}

void 
DatpAggregator::NotifyNextReceiver (DatpHeader datpHeader, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  if (!m_nextReceiver.IsNull ())
    m_nextReceiver (datpHeader, packet);
}

void
DatpAggregator::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
DatpAggregator::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  
  NS_ASSERT (m_isInstalled);
  
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_aggregatorPort);
      m_socket->Bind (local);
    }
  m_socket->SetRecvCallback (MakeCallback (&DatpAggregator::Receiver, this));
}

void
DatpAggregator::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  if (m_socket != 0)
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
}

void
DatpAggregator::Receiver (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Ptr<Packet> forwardPacket = Create<Packet> (0);
  Address from;
  while (packet = socket->RecvFrom (from))
    {
      ++m_packetsReceived;
      m_bytesReceived += packet->GetSize () ;
      while (packet->GetSize () > 0)
        {
          DatpHeader datpHeader;
          /* uint8_t headerBytesRemoved =  */packet->RemoveHeader (datpHeader);
          ++m_messagesReceived;
          // NS_LOG_INFO ("Removing message: headerBytes=" << (uint32_t)headerBytesRemoved 
                    // << " packetSizeLeft=" << packet->GetSize ()
                    // << " dataLength=" << (uint32_t) datpHeader.GetDataLength ());
          //check sanity of message
          // if (/*function to determine sanity of header*/)
            // break;  //bad rest of packet, effectively discards rest of packet
          // if (packet->GetSize () < (uint32_t) datpHeader.GetDataLength ())
            // break;  //bad rest of packet, effectively discards rest of packet
          NS_ASSERT (packet->GetSize () >= (uint32_t)datpHeader.GetDataLength ()); //bad rest of packet, never should have bad packet in simulator!
          
          //Build message descriptor
          datpHeader.SetInternalMessageIdentifier (m_messagesReceived);
          datpHeader.SetInternalReceiveTime (Simulator::Now ());
          
          Ptr<Packet> newPacket = packet->Copy (); //Explore CopyData instead
          newPacket->RemoveAtEnd (packet->GetSize () - datpHeader.GetDataLength ());
          
          packet->RemoveAtStart (datpHeader.GetDataLength ());
          
          if (m_schedulerOn == true)  //as long as scheduler is on, there is a next receiver
            {
              NotifyNextReceiver (datpHeader, newPacket);
            }
          else
            {
              newPacket->AddHeader (datpHeader);
              forwardPacket->AddAtEnd (newPacket);
            }
        }
      if (m_schedulerOn == false)
        Sender (forwardPacket);  //forward packet
    }
}

void 
DatpAggregator::Sender (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  if (m_parentAggregatorAddress.IsInvalid ())
    {
      ++m_packetsSentFailure;
    }
  else if (m_socket->SendTo ( packet, 0, InetSocketAddress (Ipv4Address::ConvertFrom(m_parentAggregatorAddress), m_aggregatorPort)) >= 0)
    {
      ++m_packetsSent;
      m_bytesSent += packet->GetSize ();
    }
  else
    {
      ++m_packetsSentFailure;
    }
}

} // Namespace ns3
