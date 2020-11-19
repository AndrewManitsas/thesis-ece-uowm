/*
* Modified and Commented by Andreas Manitsas
* Based on the NS3 example manet-routing-compare.cc and the work of Dr. Pradeep Kumar (https://www.nsnam.com/2019/05/comparison-of-adhoc-routing-protocols.html)
* Used for the Thesis "Supporting Real Time Data Processing in an Unmanned Aerial Vehicle Platform"
* University of Western Macedonia - Department of Electrical and Computer Engineering - 2020
*/

/*
* Current Configuration is:
* -------------------------
* Routing Protocol: AODV/DSR/OLSR
* Mobility Model: Gauss Markov
* Simulation Area: 2000x2000x150 m
* Number of nodes: 10/15/20/25
* Simulation Time: 60 sec
* UDP Packet Size: 1000 byte
* Wireless Standard: 802.11g
* Loss Model: Friis
* Node Speed: 10 m/s
* Time Node is stationary: 1 sec
* Bandwidth: 1Mbps
* Transmission Power: 27 dBm (500 mW)
*/

#define VERSION 0.13

//C++ Libraries
#include <fstream>
#include <iostream>

//NS3 Libraries
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/animation-interface.h"

using namespace ns3;
using namespace dsr;

NS_LOG_COMPONENT_DEFINE("routingProtocolsFANET");

class RoutingExperiment
{
	public:
		RoutingExperiment();
		void Run(int nSinks, double txp, std::string CSVfileName);
		//static void SetMACParam (ns3::NetDeviceContainer & devices,
		//                                 int slotDistance);
		std::string CommandSetup(int argc, char **argv);

	private:
		Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node);
		void ReceivePacket(Ptr<Socket> socket);
		void CheckThroughput();

		uint32_t port;
		uint32_t bytesTotal; //Bytes received counter
		uint32_t packetsReceived; //Packets received coutner

		std::string m_CSVfileName; //Output filename
		int m_nSinks; //Number of receivers
		std::string m_protocolName; //Routing protocol used (string)
		double m_txp; //Transmit power (dBm)
		bool m_traceMobility; //Enable-Disable mobility tracing
		uint32_t m_protocol; //Routing protocol selector (number)
};

//Constuctor with default values. those can be overwritten with cmd arguments
RoutingExperiment::RoutingExperiment()
{
	port = 9;                                     //<<<--- MODIFY THIS OR USE CMD ARGUMENTS
	bytesTotal = 0;                               //<<<--- MODIFY THIS OR USE CMD ARGUMENTS
	packetsReceived = 0;                          //<<<--- MODIFY THIS OR USE CMD ARGUMENTS
	m_CSVfileName = "routingProtocolsFANET.csv";  //<<<--- MODIFY THIS OR USE CMD ARGUMENTS
	m_traceMobility = false;                      //<<<--- MODIFY THIS OR USE CMD ARGUMENTS
	m_protocol = 2; // AODV                       //<<<--- MODIFY THIS OR USE CMD ARGUMENTS
}

//Print when each packet is received, on which port and from which sender
static inline std::string PrintReceivedPacket(Ptr<Socket> socket, Ptr<Packet> packet, Address senderAddress) //Print when each packet is received, on which port and from which sender
{
	std::ostringstream oss; //Output String Stream

	oss << Simulator::Now().GetSeconds() << " " << socket->GetNode()->GetId();

	if (InetSocketAddress::IsMatchingType(senderAddress))
	{
		InetSocketAddress addr = InetSocketAddress::ConvertFrom(senderAddress);
		oss << " received one packet from " << addr.GetIpv4();
	}
	else
	{
		oss << " received one packet!";
	}

	return oss.str();
}

//Count how packets are received from each sender
void RoutingExperiment::ReceivePacket(Ptr<Socket> socket) //Count how packets are received from each sender
{
	Ptr<Packet> packet;
	Address senderAddress;
	while(packet = socket->RecvFrom(senderAddress))
	{
		bytesTotal += packet->GetSize();
		packetsReceived += 1;
		NS_LOG_UNCOND(PrintReceivedPacket(socket, packet, senderAddress));
	}
}

//Write simulation data at regular interval to the file
void RoutingExperiment::CheckThroughput()
{
	double intervalTime = 1.0; //How often to write data to file (seconds)   <<<--- MODIFY THIS
	double kbs = (bytesTotal * 8.0) / 1000;
	bytesTotal = 0;

	std::ofstream out(m_CSVfileName.c_str(), std::ios::app);

	out << (Simulator::Now()).GetSeconds() << "," << kbs << "," << packetsReceived << "," << m_nSinks << "," << m_protocolName << "," << m_txp << "" << std::endl;

	out.close();
	packetsReceived = 0;
	Simulator::Schedule(Seconds(intervalTime), &RoutingExperiment::CheckThroughput, this); //Schedule to run this function every X seconds
}

Ptr<Socket> RoutingExperiment::SetupPacketReceive(Ipv4Address addr, Ptr<Node> node)
{
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	Ptr<Socket> sink = Socket::CreateSocket(node, tid);
	InetSocketAddress local = InetSocketAddress(addr, port);
	sink->Bind(local);
	sink->SetRecvCallback(MakeCallback(&RoutingExperiment::ReceivePacket, this));

	return sink;
}

//CMD arguments
std::string RoutingExperiment::CommandSetup(int argc, char **argv)
{
	CommandLine cmd(__FILE__);
	cmd.AddValue("CSVfileName", "The name of the CSV output file name", m_CSVfileName);
	cmd.AddValue("traceMobility", "Enable mobility tracing", m_traceMobility);
	cmd.AddValue("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);
	cmd.Parse(argc, argv);
	return m_CSVfileName;
}

void RoutingExperiment::Run(int nSinks, double txp, std::string CSVfileName)
{
	Packet::EnablePrinting();
	m_nSinks = nSinks;
	m_txp = txp;
	m_CSVfileName = CSVfileName;

	int nWifis = 10; //Number of nodes in the simulation                 <<<--- MODIFY THIS

	double TotalTime = 60.0; //Total simulation time (sec)               <<<--- MODIFY THIS
	std::string rate("1000000bps"); //Data rate of wireless link (bps)   <<<--- MODIFY THIS
	std::string phyMode("DsssRate11Mbps");
	std::string tr_name("routingProtocolsFANET");
	int nodeSpeed = 10; //Speed of a node's movement (m/s)               <<<--- MODIFY THIS
	int nodePause = 1; //Time a node can stay stationary (sec)           <<<--- MODIFY THIS
	m_protocolName = "protocol";

	Config::SetDefault("ns3::OnOffApplication::PacketSize", StringValue("1000")); //Packet size   <<<--- MODIFY THIS
	Config::SetDefault("ns3::OnOffApplication::DataRate",  StringValue(rate));

	//Set Non-unicastMode rate to unicast mode
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

	NodeContainer adhocNodes;
	adhocNodes.Create(nWifis);

	// setting up wifi phy and channel using helpers
	WifiHelper wifi;
	wifi.SetStandard(WIFI_PHY_STANDARD_80211b); //WiFi standard. In this case, 802.11b

	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default();
	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel"); //Friis Loss Model
	wifiPhy.SetChannel(wifiChannel.Create());

	// Add a mac and disable rate control
	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",StringValue(phyMode), "ControlMode",StringValue(phyMode));

	wifiPhy.Set("TxPowerStart",DoubleValue(txp));
	wifiPhy.Set("TxPowerEnd", DoubleValue(txp));

	wifiMac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer adhocDevices = wifi.Install(wifiPhy, wifiMac, adhocNodes);

	MobilityHelper mobilityAdhoc;
	int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomBoxPositionAllocator");
	pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=2000.0]")); //Grid limit on X axis   <<<--- MODIFY THIS
	pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=2000.0]")); //Grid limit on Y axis   <<<--- MODIFY THIS
	pos.Set("Z", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=150.0]")); //Grid limit on Z axis    <<<--- MODIFY THIS

	Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
	streamIndex += taPositionAlloc->AssignStreams(streamIndex);

	std::stringstream ssSpeed;
	ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
	std::stringstream ssPause;
	ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";

	mobilityAdhoc.SetMobilityModel("ns3::GaussMarkovMobilityModel", "Bounds", BoxValue(Box (0, 2000, 0, 2000, 0, 100)), "TimeStep", TimeValue(Seconds(0.5)), "Alpha", DoubleValue(0.85), "MeanVelocity", StringValue("ns3::UniformRandomVariable[Min=800|Max=1200]"), "MeanDirection", StringValue("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"), "MeanPitch", StringValue("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"), "NormalVelocity", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0|Bound=0.0]"), "NormalDirection", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=0.4]"), "NormalPitch", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));

	mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
	mobilityAdhoc.Install(adhocNodes);
	streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex);
	NS_UNUSED(streamIndex); //From this point, streamIndex is unused

	AodvHelper aodv;
	OlsrHelper olsr;
	DsdvHelper dsdv;
	DsrHelper dsr;
	DsrMainHelper dsrMain;
	Ipv4ListRoutingHelper list;
	InternetStackHelper internet;

	switch(m_protocol)
	{
		case 1:
			list.Add(olsr, 100);
			m_protocolName = "OLSR";
			break;
		case 2:
			list.Add(aodv, 100);
			m_protocolName = "AODV";
			break;
		case 3:
			list.Add(dsdv, 100);
			m_protocolName = "DSDV";
			break;
		case 4:
			m_protocolName = "DSR";
			break;
		default:
			NS_FATAL_ERROR("No such protocol:" << m_protocol);
	}

	if(m_protocol < 4)
	{
		internet.SetRoutingHelper(list);
		internet.Install(adhocNodes);
	}
	else if(m_protocol == 4)
	{
		internet.Install(adhocNodes);
		dsrMain.Install(dsr, adhocNodes);
	}

	NS_LOG_INFO("assigning ip address");

	Ipv4AddressHelper addressAdhoc;
	addressAdhoc.SetBase("10.1.1.0", "255.255.255.0"); //IP adress range and subnet mask
	Ipv4InterfaceContainer adhocInterfaces;
	adhocInterfaces = addressAdhoc.Assign(adhocDevices);

	OnOffHelper onoff1("ns3::UdpSocketFactory",Address());
	onoff1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

	for(int i = 0; i < nSinks; i++)
	{
		Ptr<Socket> sink = SetupPacketReceive(adhocInterfaces.GetAddress(i), adhocNodes.Get(i));

		AddressValue remoteAddress(InetSocketAddress(adhocInterfaces.GetAddress(i), port));
		onoff1.SetAttribute("Remote", remoteAddress);

		Ptr<UniformRandomVariable> var = CreateObject<UniformRandomVariable>();
		ApplicationContainer temp = onoff1.Install(adhocNodes.Get(i + nSinks));
		//temp.Start(Seconds(var->GetValue(100.0,101.0))); //Delay to start at 100 sec
		temp.Start(Seconds(0.0));
		temp.Stop(Seconds(TotalTime));
	}

	std::stringstream ss;
	ss << nWifis;
	std::string nodes = ss.str();

	std::stringstream ss2;
	ss2 << nodeSpeed;
	std::string sNodeSpeed = ss2.str();

	std::stringstream ss3;
	ss3 << nodePause;
	std::string sNodePause = ss3.str();

	std::stringstream ss4;
	ss4 << rate;
	std::string sRate = ss4.str();

	AsciiTraceHelper ascii;
	MobilityHelper::EnableAsciiAll(ascii.CreateFileStream(tr_name + ".mob"));

	Ptr<FlowMonitor> flowmon; //Flowmonitor tracks the flow of data packets and outputs them in XML file. Then we use a python tool to analyze the XML.
	FlowMonitorHelper flowmonHelper;
	flowmon = flowmonHelper.InstallAll(); //Install the flowmonitor probe to all the nodes


	NS_LOG_INFO("Run Simulation.");

	CheckThroughput();
	std::cout << "Creating XML Animation File: " << m_CSVfileName << " ...\n";
	AnimationInterface anim("routingProtocolsFANET.xml"); //Create XML file for NetAnim visualisation
	Simulator::Stop(Seconds(TotalTime));
	Simulator::Run();

	flowmon->SerializeToXmlFile((tr_name + ".flowmon").c_str(), false, false); //Name of the XML file storing the Flowmonitor data

	Simulator::Destroy();
}

//-----------------------------------------------------------------------------
int main (int argc, char *argv[])
{
	RoutingExperiment experiment;
	std::string CSVfileName = experiment.CommandSetup(argc,argv);

	//blank out the last output file and write the column headers
	std::ofstream out(CSVfileName.c_str());
	out << "SimulationSecond," << "ReceiveRate," << "PacketsReceived," << "NumberOfSinks," << "RoutingProtocol," <<	"TransmissionPower" << std::endl;
	out.close();

	int nSinks = 2; //Number of receivers       <<<--- MODIFY THIS
	double txp = 27.0; //Transmitt power (dBm)   <<<--- MODIFY THIS

	experiment.Run(nSinks, txp, CSVfileName);
}