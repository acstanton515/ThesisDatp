#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-module.h"
#include "ns3/internet-module.h"
#include "ns3/datp-module.h"

using namespace ns3;
using namespace std;

/**
 * \brief Datp example using the simple classes in a wireless network deploying AODV.  
 * Certain AODV attributes are modified based on the setup of the simulation.
 * Nodes are created in a 2-dimensional grid topology (10 wide).
 * The collector is the last node in the grid.  With default settings it is only one-dimensional and it looks like:
 * 
 * n1 <-- 80 --> n2 <-- 80 --> n3 ... n9 <-- 80 --> collector
 */

NS_LOG_COMPONENT_DEFINE ("DatpExample");



int 
main (int argc, char *argv[])
{

  uint32_t size = 9;
  double time = 10.0;
  uint32_t verbose = 0;
  bool pcaps = false;
  bool enableAggregationTracing = false;
  bool enableTreeTracing = false;
  string trial = "0";
  
  CommandLine cmd;
  cmd.AddValue ("time", "how long the applications should run in seconds (10.0)", time);
  cmd.AddValue ("size", "how many aggregators are created (9)", size);
  cmd.AddValue ("verbose", "log level 0-4 increasing 0=none,1+aggs_debug,2+apps_debug,3+aggs_all,4+apps_all (0)", verbose);
  cmd.AddValue ("pcaps", "take packet captures of all nodes (false)", pcaps);
  cmd.AddValue ("tracing", "take traces of aggregation node stats (false)", enableAggregationTracing);
  cmd.AddValue ("tree", "take output of tree (false)", enableTreeTracing);
  cmd.AddValue ("trial", "trial number for output files (0)", trial);
  
  cmd.Parse (argc, argv);
  NS_LOG_DEBUG ("Command-Line Arguments: time="<<time<<", size="<<size<<", verbose="<<verbose<<", pcaps="<<pcaps<<", tracing="<<enableAggregationTracing<<", trial="<<trial);
  
  Config::SetDefault ("ns3::aodv::RoutingProtocol::EnableHello", BooleanValue (false));                     //no hellos because don't need neighbor awareness
  Config::SetDefault ("ns3::aodv::RoutingProtocol::ActiveRouteTimeout", TimeValue (Seconds (time+10.0)));   //no timeouts because no mobility
  Config::SetDefault ("ns3::aodv::RoutingProtocol::GratuitousReply", BooleanValue (false));                 //no replies because one-way communication
  
  NodeContainer aggregators;
  aggregators.Create (size);
  NodeContainer collector;
  collector.Create (1);
  NodeContainer nodes (aggregators, collector);
  
  WifiHelper wifi;
  
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  mac.SetType ("ns3::AdhocWifiMac");
  wifi.SetRemoteStationManager ("ns3::AarfcdWifiManager");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  NetDeviceContainer nodesDevices = wifi.Install (wifiPhy, mac, nodes);
  
  
  MobilityHelper mobility;
  
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (80.0),
                                 "DeltaY", DoubleValue (40.0),
                                 "GridWidth", UintegerValue (10),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  
  AodvHelper aodv;
  InternetStackHelper internet;
  internet.SetRoutingHelper (aodv);
  internet.Install (nodes);
  
  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.1.0.0", "255.255.0.0");
  
  Ipv4InterfaceContainer nodeInterfaces;
  nodeInterfaces = ipAddrs.Assign (nodesDevices);

  DatpHelper datp;
  DatpApplicationHelper datpApplication;

  datpApplication.AddApplication (TypeId::LookupByName ("ns3::DatpApplicationOne"));
  datpApplication.AddApplication (TypeId::LookupByName ("ns3::DatpApplicationTwo"));
  datpApplication.AddApplication (TypeId::LookupByName ("ns3::DatpApplicationThree"));

  ApplicationContainer aggregationSystemApplications = datp.Install (collector.Get (0), aggregators);
  ApplicationContainer aggregatorApplication = datpApplication.Install (aggregators);
  
  aggregationSystemApplications.Start (Seconds (1.0));
  aggregatorApplication.Start (Seconds (10.0));
  aggregatorApplication.Stop (Seconds (10.0+time)); 
  aggregationSystemApplications.Stop (Seconds (1.0+10.0+time));  

  
  LogLevel level_all = (LogLevel)(LOG_LEVEL_ALL|LOG_PREFIX_TIME|LOG_PREFIX_NODE|LOG_PREFIX_FUNC);
  switch (verbose)
    {
      case 4: datpApplication.EnableLogComponents (level_all);
      case 3: datp.EnableLogComponents (level_all);
              break;
      case 2: datpApplication.EnableLogComponents (LOG_DEBUG);
      case 1: datp.EnableLogComponents (LOG_DEBUG);
    }
  
  if (pcaps)
    wifiPhy.EnablePcapAll ("datp-example-" + trial, 0);
  
  if (enableAggregationTracing == true)
    {
      NS_LOG_DEBUG ("Enable datp information trace");
      string fileName = "datp-nodes-" + trial + ".tr";
      Simulator::Schedule (Seconds (1.0+10.0+time), &DatpHelper::DatpTrace, &datp, fileName, collector.Get (0), aggregators, aggregators);
    }

  if (enableTreeTracing == true)
    {
      NS_LOG_DEBUG ("Enable tree information trace");
      string fileName = "datp-tree-" + trial + ".tr";
      Simulator::Schedule (Seconds (1.0+10.0+time), &DatpHelper::TreeTrace, &datp, fileName, collector.Get (0), aggregators);
    }
    
  Simulator::Stop (Seconds (1.0+10.0+1.0+time));
  Simulator::Run ();
  Simulator::Destroy ();
}
