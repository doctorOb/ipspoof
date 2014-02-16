ipspoof
=======

An adaption and reanalysis of IP Spoofing techniques proposed in "Packet Forwarding With Source Verification" by Shue et. al. 

Our schedule for the course of the project is maintained <a href="https://docs.google.com/spreadsheet/ccc?key=0AsqPkKbbLFV_dEV4emlsMlNjdml2eXkwcEdMZlduZ2c&usp=sharing">Here</a>

Authors:
	Dan Robertson
	Mike Calder
Date:
	2/16/14


!! Below is a list of .cc and header files that we modified in the NS-3 source. We tested our project using
the latest version of ns-3 (v.3.19), which can be found at https://www.nsnam.org/release/ns-allinone-3.19.tar.bz2


In order to run our test script (called second.cc), you must first build ns-3 using the build.py file in the root directory
(typically, ns-allinone-3.19, assuming you haven't renamed it). You'll need to pass '--enable-examples' to 
build.py in order to compile our demo script. In order to view output and log information pertaining to packet tagging, 
you need to enable the log component for Ipv4GlobalRouting. Do this from the command line by executing:
	export NS_LOG=Ipv4GlobalRouting

Next, to run the script, you'll need to run:
	./waf --run second

Out of the ns-3.19 directory. You should see output coming from each router as it receives each packet.

====Global Router Interface====
"This class turns a node into a global router, implementing the globalrouting protocol. We hooked in here to 
set each router's subnet masked IP address, so that it could deterministically compute its own tag"

ns-allinone-3.19/ns-3.19/src/internet/model/global-router-interface.cc
ns-allinone-3.19/ns-3.19/src/internet/model/global-router-interface.h

====Ipv4 Routing Table Entry=====
"We define our tag table and underlying tag class here."

ns-allinone-3.19/ns-3.19/src/internet/model/ipv4-routing-table-entry.cc
ns-allinone-3.19/ns-3.19/src/internet/model/ipv4-routing-table-entry.h

====Ipv4 Header=======
"We added an additional class definition for IPSpoofing_Packet_Tag here. The Packet class exposes a
list managed tag API that couple attribute based classes with each packet. This is where the actual
raw tag is stored in each tagged packet"

ns-allinone-3.19/ns-3.19/src/internet/model/ipv4-header.cc
ns-allinone-3.19/ns-3.19/src/internet/model/ipv4-header.h

=====Ipv4 Global Routing===
"We extended the Ipv4GlobalRouting Protocol to support our implementation of packet tagging."
ns-allinone-3.19/ns-3.19/src/internet/model/ipv4-global-routing.cc
ns-allinone-3.19/ns-3.19/src/internet/model/ipv4-global-routing.h

=====Second======
"This is a modified example script that we use to Demo our protocol. It creates a UDP echoServer/client 
exchange between two LAN subnets, connected over a few intermediary nodes (with different network addresses)."
ns-allinone-3.19/ns-3.19/examples/tutorial/second.cc


