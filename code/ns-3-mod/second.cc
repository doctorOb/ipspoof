/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

/* Network topology:

  For our first iteration of analysis, we create a simple topology 
  consisting of two LAN subnets connected over two intermediary
  network subnets. n_n0 and n_n2 act as edge routers, and forward
  their traffic to each other through n_n1.
 
   192.168.1.x/24                                           170.215.1.x/24
      =======            215.23.50.x    230.25.25.x           ========
      |     |-------n_n0----------n_n1--------------n_n2------|      |
    l_n0  l_n1                                                l_n2   l_n3
  
    the l_ nodes are connected over CSMA locally. the n_ nodes are 
    connected over point to point links
*/


#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <time.h>
#include <stdlib.h>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

#define PERCENT_DEPLOYMENT 100 //100 percent deployment

NS_LOG_COMPONENT_DEFINE ("SimpleGlobalRoutingExample");

int 
main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if 1 
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
#endif
  srand(time(NULL));

  // Set up some default values for the simulation.  Use the 
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);

  // Allow the user to override any of the defaults and the above
  // DefaultValue::Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  bool enableFlowMonitor = false;
  cmd.AddValue ("EnableMonitor", "Enable Flow Monitor", enableFlowMonitor);
  cmd.Parse (argc, argv);

  bool useChaining = true; //use the hash chaining approach for packet tag generation/inspection

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer csma_a;
  NodeContainer csma_b;
  NodeContainer network;
  NodeContainer n0n1;
  NodeContainer n1n2;

  csma_a.Create(2);
  csma_b.Create(2);
  network.Create(3);

  n0n1.Add(network.Get(0));
  n0n1.Add(network.Get(1));

  n1n2.Add(network.Get(1));
  n1n2.Add(network.Get(2));

  csma_a.Add(network.Get(0)); //add the edge router for network a
  csma_b.Add(network.Get(2)); //add the edge router for network b

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer p2p_n0n1,p2p_n1n2;
  p2p_n0n1 = p2p.Install(n0n1);
  p2p_n1n2 = p2p.Install(n1n2);

  CsmaHelper csma;
  NetDeviceContainer csma_lan_a;
  NetDeviceContainer csma_lan_b;

  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  
  csma_lan_a = csma.Install(csma_a);
  csma_lan_b = csma.Install(csma_b);

  InternetStackHelper stack;
  stack.Install(csma_a);
  stack.Install(csma_b);
  stack.Install (network.Get(1)); //n0 and n2 are already in the csma containers

  Ipv4AddressHelper ipv4a;
  ipv4a.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csma_a_interfaces;
  csma_a_interfaces = ipv4a.Assign(csma_lan_a);

  Ipv4AddressHelper ipv4b;
  ipv4b.SetBase ("215.23.50.0", "255.255.255.0");
  Ipv4InterfaceContainer n0_iface, n1_iface, n2_iface;
  n0_iface = ipv4b.Assign(p2p_n0n1);
  ipv4b.SetBase("230.25.25.0","255.255.255.0");
  n1_iface = ipv4b.Assign(p2p_n1n2);


  Ipv4AddressHelper ipv4c;
  ipv4c.SetBase ("170.215.1.0", "255.255.255.0");
  Ipv4InterfaceContainer csma_b_interfaces;
  csma_b_interfaces = ipv4c.Assign(csma_lan_b);


  //Assign the IP address generated for each node to it's global router instance
  NodeContainer::Iterator i;
  int count = 0;
  NS_LOG_INFO("Bind IP to Router object");
  for (i = network.Begin (); i != network.End (); ++i, count++){
      Ipv4Address ip = (*i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
      Ptr<GlobalRouter> rtr = (*i)->GetObject<GlobalRouter>();
      NS_LOG_INFO("Binding GlobalRouter IP address for node " << count << ": " << ip);
      rtr->SetRouterAddress(ip);
      if ((rand() % 100 + 1) < PERCENT_DEPLOYMENT) {
        NS_LOG_INFO("Enabling Tag Mode for node:" << count);
        rtr->EnableTagMode(useChaining);
      }

  }

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  NS_LOG_INFO("Populating tables");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps = echoServer.Install (csma_b.Get(0));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(4.0));

  UdpEchoClientHelper echoClient(csma_b_interfaces.GetAddress(0),9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install(csma_a.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(4.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
