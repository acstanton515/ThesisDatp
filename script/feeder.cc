#include <string>
#include <stdio.h>
#include <math.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-module.h"
#include "ns3/internet-module.h"
#include "ns3/datp-module.h"

using namespace ns3;
using namespace std;

double GetNodePositionScaler (string fileName, int minimumDistance=100);


NS_LOG_COMPONENT_DEFINE ("Feeder");

int 
main (int argc, char *argv[])
{

  //Starting Variables
  double time = 10.0;
  int distanceReductionFactor = 50;
  string nodeConfigFilename = "node-list.txt";
  string trial = "0";
  bool enableNetiTrace = false;
  bool enablePcapTrace = false;
  bool enableNodeTrace = false;
  bool enableDatpTrace = false;
  bool enableTreeTrace = false;
  bool enableApplications = true;
  bool enablePcapAll = false;

  //Default Changes
  Config::SetDefault ("ns3::aodv::RoutingProtocol::EnableHello", BooleanValue (false));
  Config::SetDefault ("ns3::aodv::RoutingProtocol::ActiveRouteTimeout", TimeValue (Seconds (time*4)));
  Config::SetDefault ("ns3::aodv::RoutingProtocol::GratuitousReply", BooleanValue (false));   
  Config::SetDefault ("ns3::Ipv4L3Protocol::DefaultTtl", UintegerValue (128));
  
  //CLI
  CommandLine cmd;
  cmd.AddValue ("trial", "trial number for output files (0)", trial);
  cmd.AddValue ("time", "how long the applications should run in seconds (10.0)", time);
  cmd.AddValue ("config", "node configuration filename, each line:[nodeName pos_x pos_y] - first node is collector (node-list.txt)", nodeConfigFilename);
  cmd.AddValue ("pcap", "take packet captures of collector node (false)", enablePcapTrace);
  cmd.AddValue ("datp", "take traces of aggregation node stats (false)", enableDatpTrace);
  cmd.AddValue ("neti", "enable ASCII trace for wifiPhy and Internet (false)", enableNetiTrace);
  cmd.AddValue ("node", "enable node information trace (false)", enableNodeTrace);
  cmd.AddValue ("tree", "enable tree information trace (false)", enableTreeTrace);
  cmd.AddValue ("apps", "enable applications to send data (true)", enableApplications);
  cmd.AddValue ("drf", "distance reduction factor percentage - set to 0 to automatically calculate (50)", distanceReductionFactor);
  cmd.AddValue ("pall", "take packet captures of all nodes (false)", enablePcapAll);

  //
  // The system global variables and the local values added to the argument
  // system can be overridden by command line arguments by using this call.
  //
  cmd.Parse (argc, argv);
  
  /////////////////////////////////////////////////////////////////////////// 
  //                                                                       //
  // Create the nodes, name the nodes, position the nodes                  //
  //                                                                       //
  /////////////////////////////////////////////////////////////////////////// 
  
  NS_LOG_DEBUG ("Obtain DRF");
  
  if (distanceReductionFactor == 0)
    distanceReductionFactor = GetNodePositionScaler (nodeConfigFilename);
  
  if (distanceReductionFactor == -1)
    {
      NS_LOG_DEBUG ("drf failure");
      exit (1);
    }
   
  NS_LOG_DEBUG ("drf=" << distanceReductionFactor);
  
  ifstream nodeIn;
  nodeIn.open (nodeConfigFilename.c_str (), ios::in);
  if (nodeIn.is_open () == false)
    {
      NS_LOG_DEBUG ("Node list file open failed");
      exit (1);
    }

  NodeContainer nodes;
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAllocator = CreateObject<ListPositionAllocator> ();
  uint32_t index = 0;
  while (!nodeIn.eof () )
    {
      string nodeName;
      double x_pos = -1.0, y_pos = -1.0;
      nodeIn >> nodeName >> x_pos >> y_pos;
      if (isalpha (nodeName[0]) != 0 && x_pos >= 0.0 && y_pos >= 0.0)
        {
          nodes.Create (1);
          Names::Add ( nodeName, nodes.Get (index++));
          x_pos = x_pos - distanceReductionFactor / 100.0 * x_pos;
          y_pos = y_pos - distanceReductionFactor / 100.0 * y_pos;
          positionAllocator->Add (Vector (x_pos, y_pos, 0.0));
        }
    }
  nodeIn.close ();

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAllocator);
  mobility.Install (nodes);
  
  Ptr<Node> collectorNode;
  NodeContainer aggregatorNodes;

  //Separate collector and aggregators
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
    {
      Ptr<Node> node = *i;
      if (node != nodes.Get (0))
        aggregatorNodes.Add (node);
      else
        collectorNode = node;
    }  
  
  NS_LOG_DEBUG ("Created " << nodes.GetN () << " nodes");
  NS_LOG_DEBUG ("Collector Node ID=" << collectorNode->GetId () << ", Name=" << Names::FindName (collectorNode));

  /////////////////////////////////////////////////////////////////////////// 
  //                                                                       //
  // Configure wifi and create node devices                                //
  //                                                                       //
  /////////////////////////////////////////////////////////////////////////// 
  
  NS_LOG_DEBUG ("Configure wifi and create node devices");
  WifiHelper wifi;
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  mac.SetType ("ns3::AdhocWifiMac");
  wifi.SetRemoteStationManager ("ns3::AarfcdWifiManager");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  //Default = NistErrorRateModel
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  //Defaults = ConstantSpeedPropagationDelayModel and LogDistancePropagationLossModel
  wifiPhy.SetChannel (wifiChannel.Create ());
  NetDeviceContainer nodesDevices = wifi.Install (wifiPhy, mac, nodes);

  /////////////////////////////////////////////////////////////////////////// 
  //                                                                       //
  // Routing, Addressing, Interneting                                      //
  //                                                                       //
  /////////////////////////////////////////////////////////////////////////// 
  
  NS_LOG_DEBUG ("Enabling aodv routing on all nodes");
  AodvHelper aodv;

  InternetStackHelper internet;
  internet.SetRoutingHelper (aodv);
  internet.Install (nodes);

  Ipv4AddressHelper ipAddrs;
  ipAddrs.SetBase ("10.1.0.0", "255.255.0.0");
  
  Ipv4InterfaceContainer nodeInterfaces;
  nodeInterfaces = ipAddrs.Assign (nodesDevices);
  
  /////////////////////////////////////////////////////////////////////////// 
  //                                                                       //
  // Application configuration                                             //
  //                                                                       //
  /////////////////////////////////////////////////////////////////////////// 

  
  NS_LOG_DEBUG ("Create applications");
  
  DatpHelper datp;
  datp.SetAggregatorAttribute ("TreeControllerType", TypeIdValue(TypeId::LookupByName ("ns3::DatpTreeControllerAodv")));
  datp.SetAggregatorAttribute ("FunctionType", TypeIdValue(TypeId::LookupByName ("ns3::DatpFunctionSimple")));
  datp.SetAggregatorAttribute ("SchedulerType", TypeIdValue(TypeId::LookupByName ("ns3::DatpSchedulerSimple")));
  ApplicationContainer datpAggregationApplications = datp.Install (collectorNode, aggregatorNodes);

  DatpApplicationHelper datpApplication;
  datpApplication.AddApplication (TypeId::LookupByName ("ns3::DatpApplicationOne"));
  datpApplication.AddApplication (TypeId::LookupByName ("ns3::DatpApplicationTwo"));
  datpApplication.AddApplication (TypeId::LookupByName ("ns3::DatpApplicationThree"));
  ApplicationContainer datpApplications = datpApplication.Install (aggregatorNodes);
  
  datpAggregationApplications.Start (Seconds (0.0));
  if (enableApplications)
    {
      datpApplications.Start (Seconds (time));
      datpApplications.Stop (Seconds (time*2));  
    }
  else
    {
      NS_LOG_DEBUG ("Turn off DATP applications");
      datpApplications.Stop (Seconds (0.0001));  
    }
  datpAggregationApplications.Stop (Seconds (time*2));
  
  /////////////////////////////////////////////////////////////////////////// 
  //                                                                       //
  // Trace configuration                                                 //
  //                                                                       //
  /////////////////////////////////////////////////////////////////////////// 

  NS_LOG_DEBUG ("Configure tracing");
  if (enableNetiTrace == true)
    {
      NS_LOG_DEBUG ("Enabling network interface trace");
      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("feeder-neti-trace-" + trial + ".tr");
      // wifiPhy.EnableAsciiAll (stream);
      internet.EnableAsciiIpv4All (stream);
    }
    
  if (enablePcapTrace == true || enablePcapAll == true)
    {
      NS_LOG_DEBUG ("Enable packet capture tracing");
      NodeContainer collectorContainer (collectorNode);
      if (!enablePcapAll)
        wifiPhy.EnablePcap ("feeder-pcap-trace-"+trial, collectorContainer, 0);
      else
        wifiPhy.EnablePcapAll ("feeder-pcap-all-trace-"+trial, 0);
    }
  if (enableNodeTrace == true)
    {
      NS_LOG_DEBUG ("Enable node information trace");
      AsciiTraceHelper asciiTraceHelper;
      Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("feeder-node-trace-" + trial + ".csv");
      
      uint32_t nNodes = nodes.GetN ();
      *stream->GetStream () << "ID,Name,Address,Position\n";
      for (uint32_t i = 0; i < nNodes; ++i)
        {
          Ptr<Node> p = nodes.Get (i);                   
          *stream->GetStream () << p->GetId ();
          *stream->GetStream () << "," << Names::FindName (p);
          *stream->GetStream () << "," << p->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
          *stream->GetStream () << "," << p->GetObject<MobilityModel> ()->GetPosition ();
          *stream->GetStream () << endl;        
        }
    }
    
  if (enableDatpTrace == true)
    {
      NS_LOG_DEBUG ("Enable datp information trace");
      string fileName = "feeder-datp-trace-" + trial + ".csv";
      Simulator::Schedule (Seconds (time*2.99), &DatpHelper::DatpTrace, &datp, fileName, collectorNode, aggregatorNodes, aggregatorNodes);
    }

  if (enableTreeTrace == true)
    {
      NS_LOG_DEBUG ("Enable tree information trace");
      string fileName = "feeder-tree-trace-" + trial + ".csv";
      Simulator::Schedule (Seconds (time*2.99), &DatpHelper::TreeTrace, &datp, fileName, collectorNode, aggregatorNodes);
    }
    
  /////////////////////////////////////////////////////////////////////////// 
  //                                                                       //
  // Run simulation                                                        //
  //                                                                       //
  /////////////////////////////////////////////////////////////////////////// 

  NS_LOG_DEBUG ("Run simulation");
  Simulator::Stop (Seconds (time*3));
  Simulator::Run ();
  Simulator::Destroy ();
}




double
GetNodePositionScaler (string fileName, int minimumDistance)
{
  int i, j;
  FILE *nodesIn;
  FILE *listIn;
  double nodeX;
  double nodeY;
  char nodeName[128];
  int drf = 0;
  bool distanceAnyFail = true;

  if ( ( nodesIn = fopen (fileName.c_str (), "r" )) == NULL)
    return -1;

  if ( ( listIn = fopen (fileName.c_str (), "r" )) == NULL)
    return -1;

  while (distanceAnyFail && drf < 100)
    {
      distanceAnyFail = false;
      for (i=0; fscanf (nodesIn, "%s %lf %lf", nodeName, &nodeX, &nodeY) == 3 ;i++) 
        {
          double nodeCheckX;
          double nodeCheckY;
          char nodeCheckName[128];
          bool distanceFail = true;
          
          nodeX = nodeX - drf / 100.0 * nodeX;
          nodeY = nodeY - drf / 100.0 * nodeY;
          
          for (j=0; fscanf (listIn, "%s %lf %lf", nodeCheckName, &nodeCheckX, &nodeCheckY) == 3 ;j++)
            {
              nodeCheckX = nodeCheckX - drf / 100.0 * nodeCheckX;
              nodeCheckY = nodeCheckY - drf / 100.0 * nodeCheckY;
              if ( strcmp (nodeName, nodeCheckName) != 0)
                {
                  if (((sqrt ( pow (fabs (nodeX - nodeCheckX), 2) + pow (fabs (nodeY - nodeCheckY), 2)))  ) < minimumDistance )
                    {
                      distanceFail = false;
                    }
                  //printf ("%s is %lf away from %s\n", nodeName, x, nodeCheckName);
                }
              if (!distanceFail)
                {
                  break;
                }
            }
          if (distanceFail)
            {
              distanceAnyFail = true;
              break;
            }
          rewind (listIn);
        }
      NS_LOG_INFO ("DRF " << drf << " not enough");
      if (distanceAnyFail)
        drf++;
      rewind (listIn);
      rewind (nodesIn);
    }
  fclose (listIn);
  fclose (nodesIn);

  return drf;
}