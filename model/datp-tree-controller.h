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
 
#ifndef __DATP_TREECONTROLLER_H__
#define __DATP_TREECONTROLLER_H__

#include "ns3/address.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/application.h"
#include <map>

namespace ns3 {

class DatpTreeController : public Application
{
public:
  static TypeId GetTypeId (void);

  DatpTreeController ();
  virtual ~DatpTreeController ();
  
  void SetParentAggregatorCallback (Callback<void, Address > parentAggregator);

protected:

  virtual void DoDispose (void) = 0;
  void NotifyParentAggregator ();
  
  Address m_collector;
  Address m_parentAggregatorAddress;
  
private:

  virtual void StartApplication (void) = 0;
  virtual void StopApplication (void) = 0;


  Callback<void, Address > m_parentAggregator;
};

} // namespace ns3

#endif /* __DATP_TREECONTROLLER_H__ */

