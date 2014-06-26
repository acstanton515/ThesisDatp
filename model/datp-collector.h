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

#ifndef DATP_COLLECTOR_H
#define DATP_COLLECTOR_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/output-stream-wrapper.h"

namespace ns3 {

/**
 * \ingroup datp
 * \class DatpCollector
 * \brief Datp collector
 */
class DatpCollector : public Application
{
public:
  static TypeId GetTypeId (void);
  DatpCollector ();
  virtual ~DatpCollector ();
  void SetStream (Ptr<OutputStreamWrapper> stream);
  void PrintStream ();

protected:
  virtual void DoDispose (void);
  
private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ReceiveProbe (Ptr<Socket> socket);
  void Receive (Ptr<Socket> socket);
  
  Ptr<Socket> m_probe_socket;
  uint16_t m_probePort;

  Ptr<Socket> m_socket;
  uint16_t m_aggregatorPort;
  
  uint32_t m_packetsReceived;
  uint32_t m_messagesReceived;
  uint32_t m_bytesReceived;
  uint32_t m_messagesMerged;
  uint32_t m_bytesMerged;
  Time m_delayMessage;
  
  uint32_t m_probesReceived;
  Ptr<OutputStreamWrapper> m_stream;
  
};

} // namespace ns3

#endif /* DATP_COLLECTOR_H */
