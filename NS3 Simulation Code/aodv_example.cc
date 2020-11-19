/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2009 IITP RAS
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
* This is an example script for AODV manet routing protocol. 
*
* Authors: Pavel Boyko <boyko@iitp.ru>
*/

/*
* Modified and Commented by Andreas Manitsas (andrewmanitsas@gmail.com)
* Used for the Thesis "Supporting Real Time Data Processing in an Unmanned Aerial Vehicle Platform"
* University of Western Macedonia - Department of Electrical and Computer Engineering - 2020
*/

#define VERSION 0.3
 
//C++ Libraries
#include <iostream>
#include <cmath>
#include <string>

//NS3 Libraries
#include "ns3/rectangle.h"
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/random-walk-2d-mobility-model.h"
#include "ns3/position-allocator.h"

using namespace ns3;
 
class AodvExample
{
	public: //Public member functions
		AodvExample(uint32_t sz, double st, double dm, bool pc, bool pr); //Constructor declaration. Modified it to accept arguments
		bool Configure(int argc, char **argv); //Configure script parameters. "argc" is the command line argument count, "argv" is the command line arguments
		void Run(); //Run simulation
		void Report(std::ostream & os); //Report results

	private: //Private member functions
		void CreateNodes(); //Create the nodes
		void CreateDevices(); //Create the devices
		void InstallInternetStack(); //Create the network
		void InstallApplications(); //Create the simulation applications

	private: //Private attributes
		// parameters
		uint32_t size; //Number of nodes in the network
		double dimension; //Grid square dimensions X by X meters
		double totalTime; //Total simulation time in seconds
		bool pcap; //Write per-device PCAP traces if true
		bool printRoutes; //Print routing table dumps in file if true
		// network
		NodeContainer nodes; //Nodes used in the example
		NetDeviceContainer devices; //Devices used in the example
		Ipv4InterfaceContainer interfaces; //Interfaces used in the example
};
	
int main(int argc, char **argv)
{
	uint32_t size = 10; //Number of nodes in the network                    <<<--- MODIFY THIS OR USE CMD ARGUMENTS
   	double dimension = 50; //Grid square dimensions X by X meters           <<<--- MODIFY THIS OR USE CMD ARGUMENTS
   	double totalTime = 100; //Total simulation time in seconds              <<<--- MODIFY THIS OR USE CMD ARGUMENTS
   	bool pcap = false; //Write per-device PCAP trace files if true          <<<--- MODIFY THIS OR USE CMD ARGUMENTS
   	bool printRoutes = false; //Print routing table dumps in file if true   <<<--- MODIFY THIS OR USE CMD ARGUMENTS
	
	AodvExample test(size, dimension, totalTime, pcap, printRoutes); //Call constructor with above parameters

	if (!test.Configure(argc, argv)) //Chech if configuration is succesfull
	{
		NS_FATAL_ERROR("Configuration failed. Aborted.");
	} //If it is, continue to simulation

	test.Run(); //Run simulation

	return 0;
}
 
//-----------------------------------------------------------------------------
AodvExample::AodvExample(uint32_t sz, double dm, double tt, bool pc, bool pr) //Modified the constructor to accept argument from call on main
{
	size = sz;
	dimension = dm;
	totalTime = tt;
	pcap = pc;
	printRoutes = pr;
}

bool AodvExample::Configure(int argc, char **argv)
 {
	//Enable AODV logs by default. Comment this if too noisy
	//LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);

	SeedManager::SetSeed(12345); //<<<--- ???
	CommandLine cmd(__FILE__);

	cmd.AddValue("size", "Number of nodes in the network.", size);
	cmd.AddValue("dimension", "Grid square dimensions X by X meters.", dimension);
	cmd.AddValue("time", "Total simulation time in seconds.", totalTime);
	cmd.AddValue("pcap", "Write per-device PCAP trace files if true.", pcap);
	cmd.AddValue("printRoutes", "Print routing table dumps in file if true.", printRoutes);

	cmd.Parse(argc, argv); //Command line arguments can be set up like: $./waf --run "scratch/aodv_example --size=20 --dimension=20 --time=150 --pcap=false --printRoutes=false". If no command line arguments are provided, it runs with the default values set up in main. Execute as $./waf --run "scratch/aodv_example --help" for details.
	return true;
 }
 
void AodvExample::Run()
{
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(1)); //Enable RTS/CTS all the time. (https://resources.infosecinstitute.com/rts-threshold-configuration-improved-wireless-network-performance/)
	CreateNodes();
	CreateDevices();
	InstallInternetStack();
	InstallApplications();

	std::cout << "AODV Routing Simulation Version: " << VERSION << std::endl;
	std::cout << "---\n" << "Number of Nodes: " << size << "\nGrid Dimensions: " << dimension << "x" << dimension << " meters" << "\nSimulation Time: " << totalTime << " seconds" << "\nPCAP Trace Files per Node: " << pcap << "\nRouting Table Dump: " << printRoutes << "\n---\n";

	std::string animationFileName = "aodv_example.xml";
	std::cout << "Creating XML Animation File: " << animationFileName << " ...\n";
	AnimationInterface anim(animationFileName); //Create XML file for NetAnim visualisation

	std::cout << "---\n" << "Starting simulation for " << totalTime << " s ...\n---\n";

	Simulator::Stop(Seconds(totalTime));
	Simulator::Run();
	Simulator::Destroy();
}
 
void AodvExample::CreateNodes()
{
	//std::cout << "Creating " << (unsigned) size << " nodes " << dimension << " m apart.\n";
	nodes.Create(size);
	
	// Name nodes
	for(uint32_t i = 0; i < size; ++i)
	{
		std::ostringstream os;
		os << "node-" << i;
		Names::Add(os.str(), nodes.Get(i));
	}

	//Create grid and set mobility model for moving nodes
	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::GridPositionAllocator",
									"MinX", DoubleValue(0.0),
									"MinY", DoubleValue(0.0),
									"DeltaX", DoubleValue(0.0),
									"DeltaY", DoubleValue(0.0),
									"GridWidth", UintegerValue(size),
									"LayoutType", StringValue("RowFirst"));
	//mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	//mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue(Rectangle(0, 500, 0, 500)));
	mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel", "Bounds", RectangleValue(Rectangle(0, dimension, 0, dimension)));
	//mobility.Install(nodes);
	for(uint32_t i = 1; i < size; i++)
	{
		mobility.Install(nodes.Get(i));
	}

	//Create grid and set mobility model for static node(s)
	MobilityHelper mobility2;
	mobility2.SetPositionAllocator("ns3::GridPositionAllocator",
									"MinX", DoubleValue(0.0),
									"MinY", DoubleValue(0.0),
									"DeltaX", DoubleValue(0.0),
									"DeltaY", DoubleValue(0.0),
									"GridWidth", UintegerValue(size),
									"LayoutType", StringValue("RowFirst"));
	mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility2.Install(nodes.Get(0));
}
 
void AodvExample::CreateDevices()
{
	WifiMacHelper wifiMac;
	wifiMac.SetType("ns3::AdhocWifiMac");
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
	wifiPhy.SetChannel(wifiChannel.Create());
	WifiHelper wifi;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue(0));
	devices = wifi.Install(wifiPhy, wifiMac, nodes); 

	if(pcap) //Per device PCAP trace files
	{
		wifiPhy.EnablePcapAll(std::string("aodv"));
	}
}
 
void AodvExample::InstallInternetStack()
{
	AodvHelper aodv;
	//you can configure AODV attributes here using aodv.Set(name, value)
	InternetStackHelper stack;
	stack.SetRoutingHelper(aodv); //has effect on the next Install()
	stack.Install(nodes);
	Ipv4AddressHelper address;
	address.SetBase("10.0.0.0", "255.0.0.0");
	interfaces = address.Assign(devices);

	if(printRoutes) //Routing tables
	{
		Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("aodv.routes", std::ios::out);
		aodv.PrintRoutingTableAllAt(Seconds(8), routingStream);
	}
}
 
void AodvExample::InstallApplications()
{
	V4PingHelper ping(interfaces.GetAddress(size - 1));
	ping.SetAttribute("Verbose", BooleanValue(true));

	ApplicationContainer p = ping.Install(nodes.Get(0));
	p.Start(Seconds(0));
	p.Stop(Seconds(totalTime) - Seconds(0.001));

	// move node away
	/*
	Ptr<Node> node = nodes.Get(size/2);
	Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
	Simulator::Schedule(Seconds (totalTime/3), &MobilityModel::SetPosition, mob, Vector(1e5, 1e5, 1e5));
	*/
}