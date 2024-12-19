

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Star");

int main (int argc, char *argv[])
{
    // Parameters for the simulation
    uint32_t nSpokes;
    uint16_t port = 50000;

    // Taking input from the user
    std::cout << "Enter the number of spoke nodes in the star topology: ";
    std::cin >> nSpokes;
    std::cout << "the number of spokes entered is : "<< nSpokes<< std::endl ;
    // Logging
    NS_LOG_INFO ("Build star topology.");
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    
    // Create the star topology with user-defined number of spokes
    PointToPointStarHelper star (nSpokes, pointToPoint);

    NS_LOG_INFO ("Install internet stack on all nodes.");
    InternetStackHelper internet;
    star.InstallStack (internet);

    NS_LOG_INFO ("Assign IP Addresses.");
    star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

    NS_LOG_INFO ("Create applications.");
    Address hubLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", hubLocalAddress);
    ApplicationContainer hubApp = packetSinkHelper.Install (star.GetHub ());
    hubApp.Start (Seconds (1.0));
    hubApp.Stop (Seconds (10.0));

    OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

    ApplicationContainer spokeApps;
    for (uint32_t i = 0; i < star.SpokeCount (); ++i)
    {
        AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port));
        onOffHelper.SetAttribute ("Remote", remoteAddress);
        spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
    }
    spokeApps.Start (Seconds (1.0));
    spokeApps.Stop (Seconds (10.0));

    NS_LOG_INFO ("Enable static global routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    NS_LOG_INFO ("Enable tracing.");
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("star.tr"));
    pointToPoint.EnablePcapAll("star");

    AnimationInterface anim("startopo.xml");
    anim.SetMaxPktsPerTraceFile(5000);

    // Set constant positions for the nodes in the animation
    anim.SetConstantPosition(star.GetHub(), 300, 300);
    double radius = 100.0;
    for (uint32_t i = 0; i < star.SpokeCount(); ++i)
    {
        double angle = 2 * M_PI * i / star.SpokeCount();
        double x = 300 + radius * cos(angle);
        double y = 300 + radius * sin(angle);
        anim.SetConstantPosition(star.GetSpokeNode(i), x, y);
    }

    NS_LOG_INFO ("Run Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");
    return 0;
}
