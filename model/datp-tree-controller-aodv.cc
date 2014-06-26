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
 
#include "datp-tree-controller-aodv.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/aodv-rtable.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/nstime.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpTreeControllerAodv");

NS_OBJECT_ENSURE_REGISTERED (DatpTreeControllerAodv);

TypeId DatpTreeControllerAodv::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpTreeControllerAodv")
    .SetParent<DatpTreeController> ()
    .AddConstructor<DatpTreeControllerAodv> ()
  ;
  return tid;
}

DatpTreeControllerAodv::DatpTreeControllerAodv () :
  m_probeTimer (Timer::CANCEL_ON_DESTROY)
{
  NS_LOG_FUNCTION (this);
  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();
}

DatpTreeControllerAodv::~DatpTreeControllerAodv()
{
  NS_LOG_FUNCTION (this);
}

void
DatpTreeControllerAodv::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
DatpTreeControllerAodv::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_collector) == true)
        {
          // m_socket->Bind ();
          // NS_ASSERT (m_collector != Ipv4Address ("127.0.0.1"));
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_collector), 10000));
        }
    }
  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  
  NS_ASSERT (GetNode ()->GetObject<ns3::aodv::RoutingProtocol> ());
  m_aodvRp = GetNode ()->GetObject<ns3::aodv::RoutingProtocol> ();
  
  m_probeTimer.SetFunction (&DatpTreeControllerAodv::SendRouteProbe, this);
  m_probeTimer.SetDelay (MilliSeconds (m_uniformRandomVariable->GetInteger (50, 100) * 10));   //check interval between 500-1000ms
  uint32_t randomStartDelay = m_uniformRandomVariable->GetInteger (0, 40) * 100 + 1000;       //start between 1-5s
  Simulator::Schedule (Time (MilliSeconds (randomStartDelay)), &DatpTreeControllerAodv::SendRouteProbe, this);
}

void
DatpTreeControllerAodv::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  m_probeTimer.Cancel ();
  if (m_socket != 0)
    m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());

}

void
DatpTreeControllerAodv::SendRouteProbe ()
{
  NS_LOG_FUNCTION (this);

  Ptr<Packet> myPck = Create<Packet> (20);;
  Ipv4Header myHeader;                         
  Socket::SocketErrno mySckError;
  myHeader.SetDestination ( Ipv4Address::ConvertFrom(m_collector) );
  Ptr<Ipv4Route> routeToCollector = m_aodvRp->RouteOutput (myPck, myHeader, 0, mySckError);
  NS_LOG_INFO ("route to collector has dst " << routeToCollector->GetDestination () 
                                             << " and gw "
                                             << routeToCollector->GetGateway () 
                                             << " and src " << routeToCollector->GetSource ());
                                             
  if (routeToCollector->GetGateway () == Ipv4Address ("127.0.0.1") )
    {
      //Create a small probe packet to send to the collector
      Ptr<Packet> p = Create<Packet> (12);
      if (m_socket->Send (p, 0) >= 0)
        {
          ++m_probesSent;
          NS_LOG_DEBUG ("Sending route probe: " << m_probesSent);
        }
      else
        {
          NS_LOG_INFO ("Error while sending probe");
        }

    }
  else if (routeToCollector->GetGateway () != m_parentAggregatorAddress)
    {
      m_parentAggregatorAddress = routeToCollector->GetGateway ();
      NotifyParentAggregator ();
      ++m_gatewayChanges;
      NS_LOG_DEBUG ("Updating gateway to " << Ipv4Address::ConvertFrom(m_parentAggregatorAddress) << " with " << m_gatewayChanges << " gateway changes");
    }
  m_probeTimer.Cancel ();
  m_probeTimer.Schedule ();
}

} // namespace ns3
