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
 
#ifndef __DATP_TREECONTROLLER_AODV_H__
#define __DATP_TREECONTROLLER_AODV_H__

#include "ns3/timer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/aodv-routing-protocol.h"
#include "datp-tree-controller.h"
#include "ns3/application.h"

namespace ns3 {

class DatpTreeControllerAodv : public DatpTreeController
{
public:
  static TypeId GetTypeId (void);

  DatpTreeControllerAodv ();
  virtual ~DatpTreeControllerAodv ();

protected:
  virtual void DoDispose (void);
  
private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void SendRouteProbe (void);

  Ptr<ns3::aodv::RoutingProtocol> m_aodvRp;
  Ptr<UniformRandomVariable> m_uniformRandomVariable; 
  uint32_t m_probesSent;
  uint32_t m_gatewayChanges;
  Timer m_probeTimer;
  Ptr<Socket> m_socket;
};

} // namespace ns3

#endif /* __DATP_TREECONTROLLER_AODV_H__ */

