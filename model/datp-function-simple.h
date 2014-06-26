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
 
#ifndef __DATP_FUNCTION_SIMPLE_H__
#define __DATP_FUNCTION_SIMPLE_H__

#include "datp-function.h"

namespace ns3 {

class DatpFunctionSimple : public DatpFunction
{
public:
  static TypeId GetTypeId (void);

  DatpFunctionSimple ();
  virtual ~DatpFunctionSimple ();
  
  uint32_t GetMessagesMerged ();
  uint32_t GetBytesMerged ();
  
  virtual void ReceiveQueryResponse (DatpHeader datpHeader, Ptr<Packet> packet);
  virtual void ReceiveNewMessage (DatpHeader datpHeader, Ptr<Packet> packet);

private:
  
  uint32_t m_messagesMerged;
  uint32_t m_bytesMerged;
  
};

} // namespace ns3

#endif /* __DATP_FUNCTION_SIMPLE_H__ */

