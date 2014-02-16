/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ipv4-routing-table-entry.h"
#include "ns3/assert.h"
#include "ns3/log.h"

#define TAG_LEN 128

NS_LOG_COMPONENT_DEFINE ("Ipv4RoutingTableEntry");

namespace ns3 {



/*****************************************************
      Packet Tag implementation
******************************************************/



/* 
  Actual tag object. Stored in some base format, hook here to perform hashing
*/
//generate a base tag from the incoming network address
//this is used to deterministically build all tag tables at each router
PTag::PTag(Ipv4Address routerAddr) {
  uint8_t * m_baseTag = (uint8_t *) malloc(4);
  routerAddr.Serialize(m_baseTag);
  m_chain_start_time = 0;
  m_seed_counter = 0;
}

PTag::~PTag() {
  if (m_baseTag != NULL){
    free(m_baseTag);
  }
}

//Get the hashed variant of the base tag. The interval argument dictates which
//unix time interval to use. If this is 0, the current interval is used. Any other 
//value will rewind the interval (for edge cases where the interval changes durring transport)
t_tag
PTag::GetTransportTag(int interval) {
  uint32_t raw_time = (uint32_t) (time(NULL);
  uint32_t offset = (uint32_t) interval * 16;
  uint32_t t_interval = (raw_time & 0xfffffff0) - offset;

  return XXH_small(m_baseTag, 4, t_interval);

  time_t current_time = time(NULL) & 0xfffffff0;
  int n = (current_time - m_chain_start_time) >> 8;
  if (n >= 2880){
    m_seed_counter++;
    m_chain_start_time = current_time;
    m_chain[0] = XXH_small(XXH_small(hash, 4, m_seed_counter), 4, m_seed_counter);
    for (n = 1; n < 2880; n++){
      m_chain[n] = XXH_small(m_chain[n-1], 4, m_seed_counter);
    }
    n = 0;
  }

  return m_chain[n];
}

uint32_t
PTag::XXH_small(const void* key, int len, unsigned int seed)
{
        const unsigned char* p = (unsigned char*)key;
        const unsigned char* const bEnd = p + len;
        unsigned int idx = seed + PRIME1;
        unsigned int crc = PRIME5;
        const unsigned char* const limit = bEnd - 4;

        while (p<limit)
        {
                crc += ((*(unsigned int*)p) + idx++);
                crc += (crc << 17) * PRIME4;
                crc *= PRIME1;
                p+=4;
        }

        while (p<bEnd)
        {
                crc += ((*p) + idx++);
                crc *= PRIME1;
                p++;
        }

        crc += len;

        crc ^= crc >> 15;
        crc *= PRIME2;
        crc ^= crc >> 13;
        crc *= PRIME3;
        crc ^= crc >> 16;

        return (uint32_t) crc;
}

/*
  Network tag table entry class. Tags are stored as unsigned long longs
*/


//NOTE: It may not be necessary to explicitly pass both address and subnet mask
//as the address may already be a network address
TagTableEntry::TagTableEntry(uint32_t interface, Ipv4Address addr, Ipv4Mask subnet) {
  m_tag = new PTag(addr.CombineMask(subnet));
  m_interface = interface;
  m_addr = addr;
  m_subnet = subnet;
}

PTag*
TagTableEntry::GetTag(void) {
  return m_tag;
}

uint32_t
TagTableEntry::GetInterface(void) {
  return m_interface;
}

Ipv4Address
TagTableEntry::GetAddress(void) {
  return m_addr;
}

Ipv4Mask
TagTableEntry::GetSubnet(void) {
  return m_subnet;
}

Ipv4Address
TagTableEntry::GetNetAddress(void) {
  return m_addr.CombineMask(m_subnet);
}

bool
TagTableEntry::Matches(Ipv4Address addr) {
  Ipv4Address combined = GetNetAddress();
  return addr.CombineMask(m_subnet).IsEqual(combined);
}

bool
TagTableEntry::VerifyTag(t_tag other) {
  //hash, timestamp, ect
  return m_tag->GetTransportTag(0) != other || m_tag->GetTransportTag(1) != other)
}


/*****************************************************
 *     Network Ipv4RoutingTableEntry
 *****************************************************/

Ipv4RoutingTableEntry::Ipv4RoutingTableEntry ()
{
  NS_LOG_FUNCTION (this);
}

Ipv4RoutingTableEntry::Ipv4RoutingTableEntry (Ipv4RoutingTableEntry const &route)
  : m_dest (route.m_dest),
    m_destNetworkMask (route.m_destNetworkMask),
    m_gateway (route.m_gateway),
    m_interface (route.m_interface)
{
  NS_LOG_FUNCTION (this << route);
}

Ipv4RoutingTableEntry::Ipv4RoutingTableEntry (Ipv4RoutingTableEntry const *route)
  : m_dest (route->m_dest),
    m_destNetworkMask (route->m_destNetworkMask),
    m_gateway (route->m_gateway),
    m_interface (route->m_interface)
{
  NS_LOG_FUNCTION (this << route);
}

Ipv4RoutingTableEntry::Ipv4RoutingTableEntry (Ipv4Address dest,
                                              Ipv4Address gateway,
                                              uint32_t interface)
  : m_dest (dest),
    m_destNetworkMask (Ipv4Mask::GetOnes ()),
    m_gateway (gateway),
    m_interface (interface)
{
}
Ipv4RoutingTableEntry::Ipv4RoutingTableEntry (Ipv4Address dest,
                                              uint32_t interface)
  : m_dest (dest),
    m_destNetworkMask (Ipv4Mask::GetOnes ()),
    m_gateway (Ipv4Address::GetZero ()),
    m_interface (interface)
{
}
Ipv4RoutingTableEntry::Ipv4RoutingTableEntry (Ipv4Address network,
                                              Ipv4Mask networkMask,
                                              Ipv4Address gateway,
                                              uint32_t interface)
  : m_dest (network),
    m_destNetworkMask (networkMask),
    m_gateway (gateway),
    m_interface (interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << gateway << interface);
}
Ipv4RoutingTableEntry::Ipv4RoutingTableEntry (Ipv4Address network,
                                              Ipv4Mask networkMask,
                                              uint32_t interface)
  : m_dest (network),
    m_destNetworkMask (networkMask),
    m_gateway (Ipv4Address::GetZero ()),
    m_interface (interface)
{
  NS_LOG_FUNCTION (this << network << networkMask << interface);
}

bool
Ipv4RoutingTableEntry::IsHost (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_destNetworkMask.IsEqual (Ipv4Mask::GetOnes ()))
    {
      return true;
    }
  else
    {
      return false;
    }
}
Ipv4Address
Ipv4RoutingTableEntry::GetDest (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dest;
}
bool
Ipv4RoutingTableEntry::IsNetwork (void) const
{
  NS_LOG_FUNCTION (this);
  return !IsHost ();
}
bool
Ipv4RoutingTableEntry::IsDefault (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_dest.IsEqual (Ipv4Address::GetZero ()))
    {
      return true;
    }
  else
    {
      return false;
    }
}
Ipv4Address
Ipv4RoutingTableEntry::GetDestNetwork (void) const
{
  NS_LOG_FUNCTION (this);
  return m_dest;
}
Ipv4Mask
Ipv4RoutingTableEntry::GetDestNetworkMask (void) const
{
  NS_LOG_FUNCTION (this);
  return m_destNetworkMask;
}
bool
Ipv4RoutingTableEntry::IsGateway (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_gateway.IsEqual (Ipv4Address::GetZero ()))
    {
      return false;
    }
  else
    {
      return true;
    }
}
Ipv4Address
Ipv4RoutingTableEntry::GetGateway (void) const
{
  NS_LOG_FUNCTION (this);
  return m_gateway;
}
uint32_t
Ipv4RoutingTableEntry::GetInterface (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interface;
}

Ipv4RoutingTableEntry 
Ipv4RoutingTableEntry::CreateHostRouteTo (Ipv4Address dest, 
                                          Ipv4Address nextHop,
                                          uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return Ipv4RoutingTableEntry (dest, nextHop, interface);
}
Ipv4RoutingTableEntry 
Ipv4RoutingTableEntry::CreateHostRouteTo (Ipv4Address dest,
                                          uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return Ipv4RoutingTableEntry (dest, interface);
}
Ipv4RoutingTableEntry 
Ipv4RoutingTableEntry::CreateNetworkRouteTo (Ipv4Address network, 
                                             Ipv4Mask networkMask,
                                             Ipv4Address nextHop,
                                             uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return Ipv4RoutingTableEntry (network, networkMask, 
                                nextHop, interface);
}
Ipv4RoutingTableEntry 
Ipv4RoutingTableEntry::CreateNetworkRouteTo (Ipv4Address network, 
                                             Ipv4Mask networkMask,
                                             uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return Ipv4RoutingTableEntry (network, networkMask, 
                                interface);
}
Ipv4RoutingTableEntry 
Ipv4RoutingTableEntry::CreateDefaultRoute (Ipv4Address nextHop, 
                                           uint32_t interface)
{
  NS_LOG_FUNCTION_NOARGS ();
  return Ipv4RoutingTableEntry (Ipv4Address::GetZero (), nextHop, interface);
}


std::ostream& operator<< (std::ostream& os, Ipv4RoutingTableEntry const& route)
{
  if (route.IsDefault ())
    {
      NS_ASSERT (route.IsGateway ());
      os << "default out=" << route.GetInterface () << ", next hop=" << route.GetGateway ();
    }
  else if (route.IsHost ())
    {
      if (route.IsGateway ())
        {
          os << "host="<< route.GetDest () << 
          ", out=" << route.GetInterface () <<
          ", next hop=" << route.GetGateway ();
        }
      else
        {
          os << "host="<< route.GetDest () << 
          ", out=" << route.GetInterface ();
        }
    }
  else if (route.IsNetwork ()) 
    {
      if (route.IsGateway ())
        {
          os << "network=" << route.GetDestNetwork () <<
          ", mask=" << route.GetDestNetworkMask () <<
          ",out=" << route.GetInterface () <<
          ", next hop=" << route.GetGateway ();
        }
      else
        {
          os << "network=" << route.GetDestNetwork () <<
          ", mask=" << route.GetDestNetworkMask () <<
          ",out=" << route.GetInterface ();
        }
    }
  else
    {
      NS_ASSERT (false);
    }
  return os;
}

/*****************************************************
 *     Ipv4MulticastRoutingTableEntry
 *****************************************************/

Ipv4MulticastRoutingTableEntry::Ipv4MulticastRoutingTableEntry ()
{
  NS_LOG_FUNCTION (this);
}

Ipv4MulticastRoutingTableEntry::Ipv4MulticastRoutingTableEntry (Ipv4MulticastRoutingTableEntry const &route)
  :
    m_origin (route.m_origin),
    m_group (route.m_group),
    m_inputInterface (route.m_inputInterface),
    m_outputInterfaces (route.m_outputInterfaces)
{
  NS_LOG_FUNCTION (this << route);
}

Ipv4MulticastRoutingTableEntry::Ipv4MulticastRoutingTableEntry (Ipv4MulticastRoutingTableEntry const *route)
  :
    m_origin (route->m_origin),
    m_group (route->m_group),
    m_inputInterface (route->m_inputInterface),
    m_outputInterfaces (route->m_outputInterfaces)
{
  NS_LOG_FUNCTION (this << route);
}

Ipv4MulticastRoutingTableEntry::Ipv4MulticastRoutingTableEntry (
  Ipv4Address origin, 
  Ipv4Address group, 
  uint32_t inputInterface, 
  std::vector<uint32_t> outputInterfaces)
{
  NS_LOG_FUNCTION (this << origin << group << inputInterface << &outputInterfaces);
  m_origin = origin;
  m_group = group;
  m_inputInterface = inputInterface;
  m_outputInterfaces = outputInterfaces;
}

Ipv4Address 
Ipv4MulticastRoutingTableEntry::GetOrigin (void) const
{
  NS_LOG_FUNCTION (this);
  return m_origin;
}

Ipv4Address 
Ipv4MulticastRoutingTableEntry::GetGroup (void) const
{
  NS_LOG_FUNCTION (this);
  return m_group;
}

uint32_t 
Ipv4MulticastRoutingTableEntry::GetInputInterface (void) const
{
  NS_LOG_FUNCTION (this);
  return m_inputInterface;
}

uint32_t
Ipv4MulticastRoutingTableEntry::GetNOutputInterfaces (void) const
{
  NS_LOG_FUNCTION (this);
  return m_outputInterfaces.size ();
}

uint32_t
Ipv4MulticastRoutingTableEntry::GetOutputInterface (uint32_t n) const
{
  NS_LOG_FUNCTION (this << n);
  NS_ASSERT_MSG (n < m_outputInterfaces.size (),
                 "Ipv4MulticastRoutingTableEntry::GetOutputInterface (): index out of bounds");

  return m_outputInterfaces[n];
}

std::vector<uint32_t>
Ipv4MulticastRoutingTableEntry::GetOutputInterfaces (void) const
{
  NS_LOG_FUNCTION (this);
  return m_outputInterfaces;
}

Ipv4MulticastRoutingTableEntry 
Ipv4MulticastRoutingTableEntry::CreateMulticastRoute (
  Ipv4Address origin, 
  Ipv4Address group, 
  uint32_t inputInterface,
  std::vector<uint32_t> outputInterfaces)
{
  NS_LOG_FUNCTION_NOARGS ();
  return Ipv4MulticastRoutingTableEntry (origin, group, inputInterface, outputInterfaces);
}

std::ostream& 
operator<< (std::ostream& os, Ipv4MulticastRoutingTableEntry const& route)
{
  os << "origin=" << route.GetOrigin () << 
  ", group=" << route.GetGroup () <<
  ", input interface=" << route.GetInputInterface () <<
  ", output interfaces=";

  for (uint32_t i = 0; i < route.GetNOutputInterfaces (); ++i)
    {
      os << route.GetOutputInterface (i) << " ";

    }

  return os;
}

} // namespace ns3
