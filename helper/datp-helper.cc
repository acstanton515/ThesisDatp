/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
#include "datp-helper.h"
#include "ns3/datp-aggregator.h"
#include "ns3/datp-collector.h"
#include "ns3/datp-application.h"
#include "ns3/datp-tree-controller.h"
#include "ns3/boolean.h"
#include "ns3/ipv4.h"
#include "ns3/trace-helper.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/names.h"


namespace ns3 {

DatpHelper::DatpHelper ()
{
  m_aggregatorFactory.SetTypeId (DatpAggregator::GetTypeId ());
  m_collectorFactory.SetTypeId (DatpCollector::GetTypeId ());
}

void
DatpHelper::SetAggregatorAttribute (std::string name, const AttributeValue &value)
{
  m_aggregatorFactory.Set (name, value);
}

void
DatpHelper::SetCollectorAttribute (std::string name, const AttributeValue &value)
{
  m_collectorFactory.Set (name, value);
}

void 
DatpHelper::EnableLogComponents (LogLevel level)
{
  LogComponentEnable ("DatpAggregator", level);
  LogComponentEnable ("DatpCollector", level);
  LogComponentEnable ("DatpScheduler", level);
  LogComponentEnable ("DatpSchedulerSimple", level);
  LogComponentEnable ("DatpFunction", level);
  LogComponentEnable ("DatpFunctionSimple", level);
  LogComponentEnable ("DatpHeaders", level);
  LogComponentEnable ("DatpTreeController", level);
  LogComponentEnable ("DatpTreeControllerAodv", level);
}

ApplicationContainer
DatpHelper::Install (Ptr<Node> collectorNode, NodeContainer aggregatorNodes)
{
  NodeContainer emptyNodeContainer;
  return Install (collectorNode, aggregatorNodes, emptyNodeContainer);
}

ApplicationContainer
DatpHelper::Install (Ptr<Node> collectorNode, NodeContainer aggregatorNodes, NodeContainer forwardAggregatorNodes)
{
  //Install collector on the collector node
  Ptr<DatpCollector> app = m_collectorFactory.Create<DatpCollector> ();
  collectorNode->AddApplication (app);
  m_aggregationContainer.Add (app);
  
  // Set aggregators collector address attribute
  m_aggregatorFactory.Set ("CollectorAddress", AddressValue (collectorNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()));
  // Install aggregator on all aggregator nodes
  for (NodeContainer::Iterator i = aggregatorNodes.Begin (); i != aggregatorNodes.End (); ++i)
    {
      Ptr<Node> node = *i;
      if (node == collectorNode)
        continue;
      Ptr<DatpAggregator> app = m_aggregatorFactory.Create<DatpAggregator> ();
      node->AddApplication (app);
      m_aggregationContainer.Add (app);
      app->Install ();
      node->AddApplication (app->GetTreeControllerApplication ());
      m_aggregationContainer.Add (app->GetTreeControllerApplication ());
    }  
  // Install aggregator on all forwarding only aggregators, and ensure aggregation is turned off
  m_aggregatorFactory.Set ("SchedulerOn", BooleanValue (false) );
  m_aggregatorFactory.Set ("FunctionOn", BooleanValue (false) );
  for (NodeContainer::Iterator i = forwardAggregatorNodes.Begin (); i != forwardAggregatorNodes.End (); ++i)
    {
      Ptr<Node> node = *i;
      if (node == collectorNode)
        continue;
      Ptr<DatpAggregator> app = m_aggregatorFactory.Create<DatpAggregator> ();
      node->AddApplication (app);
      m_aggregationContainer.Add (app);
      app->Install ();
      node->AddApplication (app->GetTreeControllerApplication ());
      m_aggregationContainer.Add (app->GetTreeControllerApplication ());
    }    
    
  return m_aggregationContainer;
}

void
DatpHelper::DatpTrace (std::string fileName, Ptr<Node> collector, NodeContainer aggregators, NodeContainer applications)
{
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName);
  
  Ptr<DatpCollector> collectorApp = DynamicCast<DatpCollector> (collector->GetApplication (0));
  NS_ASSERT (collectorApp);
  collectorApp->SetStream (stream);
  *stream->GetStream () << "Id,Address,Name,Role,Mt,Bt,Pr,Mr,Br,Pp,Mm,Bm,Dm,Mc,Rp,Rb,Dma\n";
  collectorApp->PrintStream ();

  *stream->GetStream () << "\nId,Address,Name,Role,Ps,Bs,Pr,Mr,Br,Pf,Mm,Bm,Ds,Mc,Rp,Rb,Dsa,Mst\n";
  double c[11] = {0};
  uint32_t nNodes = aggregators.GetN ();
  for (uint32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = aggregators.Get (i);          

      Ptr<DatpAggregator> agg = DynamicCast<DatpAggregator> (node->GetApplication (0));
      NS_ASSERT (agg);
      
      *stream->GetStream () << node->GetId () << ","
                            << node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()  << ","
                            << (Names::FindName (node) != "" ? Names::FindName (node) : "-") << ","
                            << "Aggregator" << ","
                            << agg->GetPacketsSent () << ","
                            << agg->GetBytesSent () << ","
                            << agg->GetPacketsReceived () << ","
                            << agg->GetMessagesReceived () << ","
                            << agg->GetBytesReceived () << ","
                            << agg->GetPacketsSentFailure () << ","
                            << node->GetObject<DatpFunctionSimple> ()->GetMessagesMerged ()  << ","
                            << node->GetObject<DatpFunctionSimple> ()->GetBytesMerged ()  << ","
                            << node->GetObject<DatpSchedulerSimple> ()->GetSchedulerDelay ().GetSeconds () << ","
                            << node->GetObject<DatpSchedulerSimple> ()->GetMessagesConcatenated () << ","
                            << (agg->GetPacketsReceived () - agg->GetPacketsSent ()) / (agg->GetPacketsReceived () * 1.0) * 100 << ","
                            << (agg->GetBytesReceived () - agg->GetBytesSent ()) / (agg->GetBytesReceived () * 1.0) * 100 << ","
                            << node->GetObject<DatpSchedulerSimple> ()->GetSchedulerDelay ().GetSeconds () / node->GetObject<DatpSchedulerSimple> ()->GetMessagesTotal () << ","
                            << node->GetObject<DatpSchedulerSimple> ()->GetMessagesTotal ()
                            << "\n";

      c[0] += agg->GetPacketsSent ();
      c[1] += agg->GetBytesSent ();
      c[2] += agg->GetPacketsReceived ();
      c[3] += agg->GetMessagesReceived ();
      c[4] += agg->GetBytesReceived ();
      c[9] += agg->GetPacketsSentFailure ();
      c[5] += node->GetObject<DatpFunctionSimple> ()->GetMessagesMerged ();
      c[6] += node->GetObject<DatpFunctionSimple> ()->GetBytesMerged ();
      c[7] += node->GetObject<DatpSchedulerSimple> ()->GetSchedulerDelay ().GetSeconds ();
      c[10] += node->GetObject<DatpSchedulerSimple> ()->GetMessagesTotal ();
      c[8] += node->GetObject<DatpSchedulerSimple> ()->GetMessagesConcatenated ();
    }
    
  *stream->GetStream () << "!,!,Total,Aggregator," << c[0] << "," 
                                  << c[1] << "," 
                                  << c[2] << "," 
                                  << c[3] << "," 
                                  << c[4] << "," 
                                  << c[9] << ","
                                  << c[5] << "," 
                                  << c[6] << "," 
                                  << c[7] << ","
                                  << c[8] << ","
                                  << (c[2] - c[0]) / c[2] * 100 << ","
                                  << (c[4] - c[1]) / c[4] * 100 << ","
                                  <<  c[7] / c[10] << ","
                                  << c[10] 
                                  << "\n";
  

  *stream->GetStream () << "\nId,Address,Name,Role,Ms1,Bs1,Ms2,Bs2,Ms3,Bs3,Mt,Bt\n";
  uint32_t nApplicationNodes = applications.GetN ();
  uint32_t c2[10] = {0};
  for (uint32_t i = 0; i < nApplicationNodes; ++i)
    {
      Ptr<Node> node = applications.Get (i);
      Ptr<DatpApplicationOne> app1 = DynamicCast<DatpApplicationOne> (node->GetApplication (2));
      Ptr<DatpApplicationTwo> app2 = DynamicCast<DatpApplicationTwo> (node->GetApplication (3));
      Ptr<DatpApplicationThree> app3 = DynamicCast<DatpApplicationThree> (node->GetApplication (4));
      
      NS_ASSERT_MSG (app1 && app2 && app3, "app1=" << app1 << ",app2=" << app2 << ",app3=" << app3);
      //removed the application outputs since it is fairly redundany between applications on different nodes
      // *stream->GetStream () << node->GetId () << ","
                            // << node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ()  << ","
                            // << (Names::FindName (node) != "" ? Names::FindName (node) : "-") << ","
                            // << "Application" << ","
                            // << app1->GetMessagesSent ()  << ","
                            // << app1->GetBytesSent ()  << ","
                            // << app2->GetMessagesSent ()  << ","
                            // << app2->GetBytesSent ()  << ","
                            // << app3->GetMessagesSent ()  << ","
                            // << app3->GetBytesSent () << ","
                            // << app1->GetMessagesSent () + app2->GetMessagesSent () + app3->GetMessagesSent () << ","
                            // << app1->GetBytesSent () + app2->GetBytesSent () + app3->GetBytesSent ()
                            // << "\n";

      c2[0] += app1->GetMessagesSent ();
      c2[1] += app1->GetBytesSent ();
      c2[2] += app2->GetMessagesSent ();
      c2[3] += app2->GetBytesSent ();
      c2[4] += app3->GetMessagesSent ();
      c2[5] += app3->GetBytesSent ();
      c2[6] += app1->GetMessagesSent () + app2->GetMessagesSent () + app3->GetMessagesSent ();
      c2[7] += app1->GetBytesSent () + app2->GetBytesSent () + app3->GetBytesSent ();
    }
    
  *stream->GetStream () << "!,!,Total,Application," << c2[0] << "," 
                                  << c2[1] << "," 
                                  << c2[2] << "," 
                                  << c2[3] << "," 
                                  << c2[4] << "," 
                                  << c2[5] << ","
                                  << c2[6] << ","
                                  << c2[7]
                                  << "\n";
}

void 
DatpHelper::TreeTrace (std::string fileName, Ptr<Node> collector, NodeContainer aggregators)
{
  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (fileName);
  
  uint32_t nNodes = aggregators.GetN ();
  for (uint32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = aggregators.Get (i);          
      Ptr<DatpAggregator> agg = DynamicCast<DatpAggregator> (node->GetApplication (0));
      NS_ASSERT (agg);
      bool foundParent = false;
      Ptr<Node> parentNode;
      for (uint32_t j = 0; j < nNodes; ++j)
        {
          parentNode = aggregators.Get (j);  
          if (parentNode->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal () == agg->GetParentAggregatorAddress ())
            {
              foundParent = true;
              break;
            }
        }
      //Is parent, the collector?
      if (!foundParent)
        {
          if (agg->GetCollectorAddress () == agg->GetParentAggregatorAddress ())
            {
              parentNode = collector;
              foundParent = true;
            }
        }
      
      if (Names::FindName (node) == "")
        {
          char str[10];
          sprintf(str,"%d",node->GetId ());
          Names::Add (str, DynamicCast<Object> (node));
        }
      if (foundParent && Names::FindName (parentNode) == "")
        {
          char str[10];
          sprintf(str,"%d",parentNode->GetId ());
          Names::Add (str, DynamicCast<Object> (parentNode));
        }
      if (foundParent)
        {
          *stream->GetStream () << Names::FindName (node) << ","
                                << Names::FindName (parentNode) << "\n";
        }
      else
        {
          *stream->GetStream () << Names::FindName (node) << ","
                                << Names::FindName (node) << "\n";
        }
    }
}



DatpApplicationHelper::DatpApplicationHelper ()
{

}

void
DatpApplicationHelper::AddApplication (TypeId id)
{
  ObjectFactory factory;
  m_applicationsFactories[id] = factory;
  m_applicationsFactories[id].SetTypeId (id);
}

void
DatpApplicationHelper::SetApplicationAttribute (TypeId id, std::string name, const AttributeValue &value)
{
  m_applicationsFactories[id].Set (name, value);
}

void 
DatpApplicationHelper::EnableLogComponents (LogLevel level)
{
  LogComponentEnable ("DatpApplications", level);
}

ApplicationContainer
DatpApplicationHelper::Install (NodeContainer applicationNodes)
{
  NodeContainer emptyNodeContainer;
  return Install (applicationNodes, emptyNodeContainer);
}

ApplicationContainer
DatpApplicationHelper::Install (NodeContainer applicationNodes, NodeContainer applicationNodes2)
{
  //Install client applications
  std::map<TypeId, ObjectFactory>::iterator it;
  int count;
  for (it=m_applicationsFactories.begin (), count=0; it!=m_applicationsFactories.end(); ++it, ++count)
    {
      for (NodeContainer::Iterator i = applicationNodes.Begin (); i != applicationNodes.End (); ++i)
        {
          Ptr<Node> node = *i;

          Ptr<Application> app = it->second.Create<Application> ();
          node->AddApplication (app);
          m_applicationsContainer.Add (app);
        }  
      for (NodeContainer::Iterator i = applicationNodes2.Begin (); i != applicationNodes2.End (); ++i)
        {
          Ptr<Node> node = *i;

          Ptr<Application> app = it->second.Create<Application> ();
          node->AddApplication (app);
          m_applicationsContainer.Add (app);
        }  
    }
  return m_applicationsContainer;
}

} // namespace ns3
