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

#ifndef DATP_AGG_H
#define DATP_AGG_H

#include "ns3/application.h"
// #include "ns3/event-id.h"
// #include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/type-id.h"
#include "datp-headers.h"
#include "datp-scheduler.h"
#include "datp-scheduler-simple.h"
#include "datp-function.h"
#include "datp-function-simple.h"
#include "datp-tree-controller.h"
#include "datp-tree-controller-aodv.h"

namespace ns3 {

/**
 * \ingroup Datp
 * \class DatpAggregator
 * \brief The Datp aggregator creates a scheduler, function, and a tree controller
 */
class DatpAggregator : public Application
{
public:
  static TypeId GetTypeId (void);
  DatpAggregator ();
  virtual ~DatpAggregator ();
  
  uint32_t GetPacketsSent (void);
  uint32_t GetBytesSent (void);
  uint32_t GetPacketsSentFailure (void);
  uint32_t GetPacketsReceived (void);
  uint32_t GetMessagesReceived (void);
  uint32_t GetBytesReceived (void);
  
  virtual void SetParentAggregatorAddress (Address parentAggregatorAddress);
  virtual Address GetParentAggregatorAddress (void) const;
  virtual Address GetCollectorAddress (void) const;
 
  virtual void Install (void);
  
  virtual Ptr<Application> GetTreeControllerApplication (void) const;
  
  virtual void SetNextReceiverCallback (Callback<void, DatpHeader, Ptr<Packet> > nextReceiver);

protected:
  virtual void DoDispose (void);
  
  void NotifyNextReceiver (DatpHeader datpHeader, Ptr<Packet> packet);
  
private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  virtual void Receiver (Ptr<Socket> socket);
  virtual void Sender (Ptr<Packet> packet);

  uint16_t m_aggregatorPort;
  Ptr<Socket> m_socket;
  Address m_parentAggregatorAddress;
  Address m_collectorAddress;
  

  uint32_t m_packetsSent;
  uint32_t m_bytesSent;
  uint32_t m_packetsSentFailure;
  uint32_t m_packetsReceived;
  uint32_t m_messagesReceived;
  uint32_t m_bytesReceived;
  
  
  //attribute members
  bool m_schedulerOn;
  bool m_functionOn;
  bool m_isInstalled;
  TypeId m_treeControllerTypeId;
  Ptr<DatpTreeController> m_treeController;
  TypeId m_functionTypeId;
  Ptr<DatpFunction> m_function;
  TypeId m_schedulerTypeId;
  Ptr<DatpScheduler> m_scheduler;

  Callback<void, DatpHeader, Ptr<Packet> > m_nextReceiver;
  
  /// Provides uniform random variables.
  // Ptr<UniformRandomVariable> m_uniformRandomVariable; 
  
};

} // namespace ns3

#endif /* DATP_AGG_H */
