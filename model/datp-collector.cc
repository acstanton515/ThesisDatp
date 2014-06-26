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
#include "ns3/ipv4.h"
#include "ns3/uinteger.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3/names.h"
#include "datp-headers.h"
#include "datp-collector.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpCollector");
NS_OBJECT_ENSURE_REGISTERED (DatpCollector);


TypeId
DatpCollector::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpCollector")
    .SetParent<Application> ()
    .AddConstructor<DatpCollector> ()
    .AddAttribute ("AggregatorPort",
                   "Set UDP port number for aggregation system",
                   UintegerValue (9999),
                   MakeUintegerAccessor (&DatpCollector::m_aggregatorPort),
                   MakeUintegerChecker<uint16_t> (1024)) 
    .AddAttribute ("ProbePort",
                   "Set UDP port number for probing port",
                   UintegerValue (10000),
                   MakeUintegerAccessor (&DatpCollector::m_probePort),
                   MakeUintegerChecker<uint16_t> (1024)) 
  ;
  return tid;
}

DatpCollector::DatpCollector ()
{
  NS_LOG_FUNCTION (this);
  m_packetsReceived=0;
  m_messagesReceived=0;
  m_probesReceived=0;
  m_bytesReceived=0;
  m_messagesMerged=0;
  m_bytesMerged=0;
  m_delayMessage = Seconds (0.0);
  m_stream = 0;
}

DatpCollector::~DatpCollector ()
{
  NS_LOG_FUNCTION (this);       
}

void 
DatpCollector::SetStream (Ptr<OutputStreamWrapper> stream)
{
  NS_LOG_FUNCTION (this);    
  m_stream = stream;
}

void
DatpCollector::PrintStream ()
{
  NS_LOG_FUNCTION (this);    
  NS_ASSERT (m_stream);

  uint32_t messsagesConcatenated = m_messagesReceived - m_packetsReceived;
  uint32_t messagesTotal = m_messagesReceived + m_messagesMerged;
  uint32_t bytesTotal = m_bytesMerged + m_bytesReceived;
  double messageRate = (messsagesConcatenated + m_messagesMerged) / (messagesTotal * 1.0) * 100;
  double byteRate = m_bytesMerged / (bytesTotal * 1.0) * 100;
  
  // NS_LOG_DEBUG ("\n===Raw===\nPackets Received:" << m_packetsReceived 
             // << "\nMessages Received:" << m_messagesReceived 
             // << "\nBytes Received:" << m_bytesReceived
             // << "\nMessages Merged:" << m_messagesMerged
             // << "\nBytes Merged:" << m_bytesMerged
             // << "\nMessage Delay:" << m_delayMessage.GetSeconds ()
             // << "\n\n===Results===\nTotal Messages:" << messagesTotal
             // << "\nTotal Bytes:" << bytesTotal
             // << "\nMessages Concatenated:" << messsagesConcatenated
             // << "\nAggregation Pkt  Rate:" <<  packetRate << "%"
             // << "\nAggregation Byte Rate:" << byteRate << "%"
             // << "\nAverage Message Delay:" << m_delayMessage.GetSeconds () / messagesTotal << "s"
             // << "\n\n===Probes===\nReceived:" << m_probesReceived << "\n");
  //"Id,Address,Name,Role,Mt,Bt,Pr,Mr,Br,Pp,Mm,Bm,Dm,Mc,Rm,Rb,Dma\n"
  if (m_stream)
    {
      *m_stream->GetStream () << GetNode ()->GetId () << ","
                              << GetNode ()->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()  << ","
                              << (Names::FindName (GetNode ()) != "" ? Names::FindName (GetNode ()) : "-") << ","
                              << "Collector" << ","
                              << messagesTotal << ","
                              << bytesTotal << ","
                              << m_packetsReceived << ","
                              << m_messagesReceived << ","
                              << m_bytesReceived << ","
                              << m_probesReceived << ","
                              << m_messagesMerged << ","
                              << m_bytesMerged << ","
                              << m_delayMessage.GetSeconds () << ","
                              << messsagesConcatenated << ","
                              << messageRate << ","
                              << byteRate << ","
                              << m_delayMessage.GetSeconds () / messagesTotal
                              << "\n";
    }    
}

void
DatpCollector::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
DatpCollector::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_aggregatorPort);
      m_socket->Bind (local);
    }
  m_socket->SetRecvCallback (MakeCallback (&DatpCollector::Receive, this));
  
  if (m_probe_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_probe_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (),
                                                   m_probePort);
      m_probe_socket->Bind (local);
    }
  m_probe_socket->SetRecvCallback (MakeCallback (&DatpCollector::ReceiveProbe, this));
}

void
DatpCollector::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  if (m_socket != 0)
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

}

void
DatpCollector::Receive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while (packet = socket->RecvFrom (from))
    {
      ++m_packetsReceived;
      m_bytesReceived += packet->GetSize ();

      while (packet->GetSize () > 0)
        {
          DatpHeader datpHeader;
          uint8_t headerBytesRemoved = packet->RemoveHeader (datpHeader);
          ++m_messagesReceived;

          NS_ASSERT (packet->GetSize () >= (uint32_t)datpHeader.GetDataLength ()); //bad rest of packet, never should have bad packet in simulator!
          
          uint32_t i = 0;
          uint32_t value = 0;
          bool firstIteration2 = true;
          DatpGenericApplicationDataHeader dataHeader;
          while (i < datpHeader.GetDataLength ())
            {
              i+= packet->RemoveHeader (dataHeader);
              if (!firstIteration2)
                NS_ASSERT (value == dataHeader.GetValue ());  //value should not change per current function operations
              else
                firstIteration2 = false;
              value = dataHeader.GetValue ();
            }
          
          if (dataHeader.GetValue () > 0)
            {
              m_delayMessage += NanoSeconds ((Simulator::Now ().GetNanoSeconds () - datpHeader.GetTimestamp ()) * dataHeader.GetValue ());
              m_messagesMerged += dataHeader.GetValue () -1;
              m_bytesMerged += (datpHeader.GetDataLength () + headerBytesRemoved) * (dataHeader.GetValue () - 1);
            }
          else
            {
              m_delayMessage += NanoSeconds (Simulator::Now ().GetNanoSeconds () - datpHeader.GetTimestamp ());
            }
      }
    }
}

void
DatpCollector::ReceiveProbe (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      ++m_probesReceived;
      NS_LOG_DEBUG ("probe received, " << m_probesReceived << ", " << InetSocketAddress::ConvertFrom (from).GetIpv4 ());
    }
}

} // Namespace ns3
