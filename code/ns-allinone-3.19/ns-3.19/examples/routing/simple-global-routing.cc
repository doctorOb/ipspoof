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

//
// Network topology:
//  ..Network A...                                                ..Network B..
//  n0 192.168.1.0                                                n5 170.215.1.5
//     \                                                        /
//      \   192.168.x.x/24                     170.215.y.y/24  /
//       n2 -------------------------------------------------n3
//      /                                                     \
//     /                                                       \
//   n1 192.168.1.1                                             n4 170.215.1.4
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n3, and from n3 to n1
// - FTP/TCP flow from n0 to n3, starting at time 1.2 to time 1.35 sec.
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues 
// - Tracing of queues and packet receptions to file "simple-global-routing.tr"

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
  LogComponentEnable ("SimpleGlobalRoutingExample", LOG_LEVEL_INFO);
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

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create(6);

  NodeContainer n0n2 = NodeContainer (c.Get (0), c.Get (2));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n3n2 = NodeContainer (c.Get(3), c.Get(2));
  NodeContainer n4n3 = NodeContainer (c.Get(4), c.Get(3));
  NodeContainer n5n3 = NodeContainer (c.Get(5), c.Get(3));

  InternetStackHelper internet;
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer d0d2 = p2p.Install (n0n2);
  NetDeviceContainer d1d2 = p2p.Install (n1n2);
  NetDeviceContainer d4d3 = p2p.Install(n4n3);
  NetDeviceContainer d5d3 = p2p.Install(n5n3);

  p2p.SetDeviceAttribute ("DataRate", StringValue ("1500kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer d3d2 = p2p.Install (n3n2);



  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i2 = ipv4.Assign (d0d2);
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);
  Ipv4InterfaceContainer i3i2 = ipv4.Assign (d3d2);

  ipv4.SetBase ("170.215.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i4i3 = ipv4.Assign (d4d3);
  Ipv4InterfaceContainer i5i3 = ipv4.Assign (d5d3);


  //Assign the IP address generated for each node to it's global router instance
  NodeContainer::Iterator i;
  int count = 0;
  NS_LOG_INFO("Bind IP to Router object");
  for (i = c.Begin (); i != c.End (); ++i, count++){
      Ipv4Address ip = (*i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
      Ptr<GlobalRouter> rtr = (*i)->GetObject<GlobalRouter>();
      NS_LOG_INFO("Binding GlobalRouter IP address for node " << count << ": " << ip);
      rtr->SetRouterAddress(ip);
      if ((rand() % 100 + 1) < PERCENT_DEPLOYMENT) {
        NS_LOG_INFO("Enabling Tag Mode for node:" << count);
        rtr->EnableTagMode();
      }

  }

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  NS_LOG_INFO("Populating tables");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  NS_LOG_INFO("First sim, using address: " << i3i2.GetAddress(0));
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (i3i2.GetAddress (0), port)));
  onoff.SetConstantRate (DataRate ("448kb/s"));
  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (2.0));


  NS_LOG_INFO("Second sim");
  // Create a packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (i3i2.GetAddress(1), port)));
  apps = sink.Install (c.Get (4));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (2.0));

  
  // Create a similar flow from n3 to n1, starting at time 1.1 seconds
  onoff.SetAttribute ("Remote", 
                      AddressValue (InetSocketAddress (i1i2.GetAddress (0), port)));
  apps = onoff.Install (c.Get (4));
  apps.Start (Seconds (1.1));
  apps.Stop (Seconds (2.0));

  // Create a packet sink to receive these packets
  apps = sink.Install (c.Get (1));
  apps.Start (Seconds (1.1));
  apps.Stop (Seconds (2.0));

  
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("simple-global-routing.tr"));
  p2p.EnablePcapAll ("simple-global-routing");

  // Flow Monitor
  Ptr<FlowMonitor> flowmon;
  FlowMonitorHelper flowmonHelper;
  if (enableFlowMonitor)
    {
      flowmon = flowmonHelper.InstallAll ();
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (11));
  Simulator::Run ();
  NS_LOG_INFO ("Done.");

  if (enableFlowMonitor)
    {
      flowmon->SerializeToXmlFile ("simple-global-routing.flowmon", false, false);
    }

  Simulator::Destroy ();
  return 0;
}
