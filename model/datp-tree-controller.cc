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
 
#include "datp-tree-controller.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DatpTreeController");

NS_OBJECT_ENSURE_REGISTERED (DatpTreeController);

TypeId DatpTreeController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DatpTreeController")
    .SetParent<Application> ()
    .AddAttribute ("CollectorAddress",
                   "The address of the collector in the aggregation system",
                   AddressValue (),
                   MakeAddressAccessor (&DatpTreeController::m_collector),
                   MakeAddressChecker ())
  ;
  return tid;
}

DatpTreeController::DatpTreeController ()
{
  NS_LOG_FUNCTION (this);

}

DatpTreeController::~DatpTreeController()
{
  NS_LOG_FUNCTION (this);
}

void 
DatpTreeController::SetParentAggregatorCallback (Callback<void, Address > parentAggregator)
{
  NS_LOG_FUNCTION (this << &parentAggregator);
  m_parentAggregator = parentAggregator;
}

void 
DatpTreeController::NotifyParentAggregator ()
{
  NS_LOG_FUNCTION (this);
  if (!m_parentAggregator.IsNull ())
    m_parentAggregator (m_parentAggregatorAddress);
}



} // namespace ns3

