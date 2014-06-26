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
#ifndef DATP_HELPER_H
#define DATP_HELPER_H

#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/log.h"
#include <map>
#include <string>
//#include "ns3/type-id.h"

namespace ns3 {
/**
 * \brief
 */
class DatpHelper
{
public:
  /**
   * Create DatpHelper which will make life easier for people trying
   * to set up simulations with my super complex applications.
   *
   * 1) Takes a node pointer for the collector, a node container for aggregators, and an optional node container for non-aggregators
   *
   */
  DatpHelper ();
  
  void SetAggregatorAttribute (std::string name, const AttributeValue &value);
  void SetCollectorAttribute (std::string name, const AttributeValue &value);
  static void EnableLogComponents (LogLevel level= LOG_LEVEL_ALL);
  
  /**
   * Create one collector application on the collector node
   * Create one aggregator app on every aggregator and regular node, but turn off aggregation on regular nodes
   *
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (Ptr<Node> collector, NodeContainer aggregators, NodeContainer nodes);
  ApplicationContainer Install (Ptr<Node> collector, NodeContainer aggregators);

  void DatpTrace (std::string fileName, Ptr<Node> collector, NodeContainer aggregators, NodeContainer applications);
  void TreeTrace (std::string fileName, Ptr<Node> collector, NodeContainer aggregators);
private:
  ObjectFactory m_collectorFactory;
  ObjectFactory m_aggregatorFactory;
  ApplicationContainer m_aggregationContainer;
};





class DatpApplicationHelper
{
public:
  /**
   * Create DatpHelper which will make life easier for people trying
   * to set up simulations with my super complex applications.
   *
   * 1) Takes a node pointer for the collector, a node container for aggregators, and an optional node container for non-aggregators
   *
   */
  DatpApplicationHelper ();

  
  void AddApplication (TypeId id);
  
  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetApplicationAttribute (TypeId id, std::string name, const AttributeValue &value);
  static void EnableLogComponents (LogLevel level= LOG_LEVEL_ALL);
  
  ApplicationContainer Install (NodeContainer applicationNodes);
  ApplicationContainer Install (NodeContainer applicationNodes, NodeContainer applicationNodes2);

private:

  std::map<TypeId, ObjectFactory> m_applicationsFactories;
  ApplicationContainer m_applicationsContainer;
};

} // namespace ns3

#endif /* DATP_HELPER_H */
