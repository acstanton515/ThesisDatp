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
 *
 */

#ifndef DATP_APP_H
#define DATP_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup Datp
 * \class DatpApplication
 * \brief Pure base class for Datp Applications
 */
class DatpApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  DatpApplication ();
  virtual ~DatpApplication ();
  
  uint32_t GetMessagesSent (void);
  uint32_t GetBytesSent (void);

protected:
  virtual void DoDispose (void);
  
  EventId m_sendEvent;
  uint32_t m_origin;
  uint32_t m_messagesSent;
  uint32_t m_bytesSent;
  Ptr<Socket> m_socket;

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  virtual void Send (void)=0;
  Ipv4Address m_peerAddress;
  uint16_t m_peerPort;
  Ptr<UniformRandomVariable> m_uniformRandomVariable; 
};




/**
 * \ingroup Datp
 * \class DatpApplicationOne
 * \brief Sends 20 bytes of data every 100ms by default
 */
class DatpApplicationOne : public DatpApplication
{
public:
  static TypeId GetTypeId (void);

  DatpApplicationOne ();
  virtual ~DatpApplicationOne ();

private:
  void Send (void);

  Time m_interval;
  uint32_t m_dataLength;
  uint32_t m_priority;
  uint32_t m_application;
};




/**
 * \ingroup Datp
 * \class DatpApplicationTwo
 * \brief Sends 60 bytes of data every 500ms by default
 */
class DatpApplicationTwo : public DatpApplication
{
public:
  static TypeId GetTypeId (void);

  DatpApplicationTwo ();
  virtual ~DatpApplicationTwo ();

private:
  void Send (void);

  Time m_interval;
  uint32_t m_dataLength;
  uint32_t m_priority;
  uint32_t m_application;
};




/**
 * \ingroup Datp
 * \class DatpApplicationThree
 * \brief Sends 40 bytes of data every 1s by default
 */
class DatpApplicationThree : public DatpApplication
{
public:
  static TypeId GetTypeId (void);

  DatpApplicationThree ();
  virtual ~DatpApplicationThree ();

private:
  void Send (void);

  Time m_interval;
  uint32_t m_dataLength;
  uint32_t m_priority;
  uint32_t m_application;
};





} // namespace ns3

#endif /* DATP_APP_H */
