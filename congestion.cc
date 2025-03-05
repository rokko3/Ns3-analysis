#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThreeNodeTopology");

static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd
    << std::endl;
}

int
main(int argc, char* argv[])
{
    // Habilitar registro detallado para depuración
    LogComponentEnable("ThreeNodeTopology", LOG_LEVEL_INFO);
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    LogComponentEnable("TcpSocketBase", LOG_LEVEL_INFO);

    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    // Crear tres nodos
    NodeContainer nodes;
    nodes.Create(3);

    // Configurar enlaces punto a punto (topología triangular)
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices01, devices02, devices12;
    devices01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1)); // 0 <--> 1 (directo)
    devices02 = pointToPoint.Install(nodes.Get(0), nodes.Get(2)); // 0 <--> 2
    devices12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2)); // 1 <--> 2

    // HABILITACIÓN DE PCAP PARA CADA ENLACE
    pointToPoint.EnablePcapAll("three-node-topology");

    // Instalar pila de Internet
    InternetStackHelper stack;
    stack.Install(nodes);

    // Asignar direcciones IP
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer interfaces01, interfaces02, interfaces12;

    address.SetBase("10.1.1.0", "255.255.255.0");
    interfaces01 = address.Assign(devices01);

    address.SetBase("10.1.2.0", "255.255.255.0");
    interfaces02 = address.Assign(devices02);

    address.SetBase("10.1.3.0", "255.255.255.0");
    interfaces12 = address.Assign(devices12);

    // Configurar rutas
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Configurar puertos DIFERENTES para cada receptor y conexión
    uint16_t sinkPort1 = 8080; // Para la comunicación del nodo 0 al nodo 1
    uint16_t sinkPort2 = 8081; // Para la comunicación del nodo 0 al nodo 2
    uint16_t sinkPort3 = 8082; // Para la comunicación del nodo 1 al nodo 2
    uint16_t sinkPort4 = 8083; // Para la comunicación del nodo 2 al nodo 0
    uint16_t sinkPort5 = 8084; // Para la comunicación del nodo 2 al nodo 1

    // Instalar PacketSink en NODO 1 para recibir desde 0 (puerto 8080)
    PacketSinkHelper packetSinkHelper1("ns3::TcpSocketFactory",
                                       InetSocketAddress(Ipv4Address::GetAny(), sinkPort1));
    ApplicationContainer sinkApps1 = packetSinkHelper1.Install(nodes.Get(1));
    sinkApps1.Start(Seconds(0.));
    sinkApps1.Stop(Seconds(30.));

    // Instalar PacketSink en NODO 2 para recibir desde 0 (puerto 8081)
    PacketSinkHelper packetSinkHelper2("ns3::TcpSocketFactory",
                                       InetSocketAddress(Ipv4Address::GetAny(), sinkPort2));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install(nodes.Get(2));
    sinkApps2.Start(Seconds(0.));
    sinkApps2.Stop(Seconds(30.));

    // Instalar PacketSink en NODO 2 para recibir desde 1 (puerto 8082)
    PacketSinkHelper packetSinkHelper3("ns3::TcpSocketFactory",
                                       InetSocketAddress(Ipv4Address::GetAny(), sinkPort3));
    ApplicationContainer sinkApps3 = packetSinkHelper3.Install(nodes.Get(2));
    sinkApps3.Start(Seconds(0.));
    sinkApps3.Stop(Seconds(30.));

    // Instalar PacketSink en NODO 0 para recibir desde 2 (puerto 8083)
    PacketSinkHelper packetSinkHelper4("ns3::TcpSocketFactory",
                                       InetSocketAddress(Ipv4Address::GetAny(), sinkPort4));
    ApplicationContainer sinkApps4 = packetSinkHelper4.Install(nodes.Get(0));
    sinkApps4.Start(Seconds(0.));
    sinkApps4.Stop(Seconds(30.));

    // Instalar PacketSink en NODO 1 para recibir desde 2 (puerto 8084)
    PacketSinkHelper packetSinkHelper5("ns3::TcpSocketFactory",
                                       InetSocketAddress(Ipv4Address::GetAny(), sinkPort5));
    ApplicationContainer sinkApps5 = packetSinkHelper5.Install(nodes.Get(1));
    sinkApps5.Start(Seconds(0.));
    sinkApps5.Stop(Seconds(30.));

    // Aplicación del nodo 0 al nodo 1 (directo) - puerto 8080
    BulkSendHelper bulkSend01("ns3::TcpSocketFactory",
                              InetSocketAddress(interfaces01.GetAddress(1), sinkPort1));
    bulkSend01.SetAttribute("MaxBytes", UintegerValue(500000)); // Enviar 500 KB
    ApplicationContainer clientApps01 = bulkSend01.Install(nodes.Get(0));
    clientApps01.Start(Seconds(1.));
    clientApps01.Stop(Seconds(10.));

    // Aplicación del nodo 0 al nodo 2 - puerto 8081
    BulkSendHelper bulkSend02("ns3::TcpSocketFactory",
                              InetSocketAddress(interfaces02.GetAddress(1), sinkPort2));
    bulkSend02.SetAttribute("MaxBytes", UintegerValue(500000)); // Enviar 500 KB
    ApplicationContainer clientApps02 = bulkSend02.Install(nodes.Get(0));
    clientApps02.Start(Seconds(11.));
    clientApps02.Stop(Seconds(20.));

    // Aplicación del nodo 1 al nodo 2 - puerto 8082
    BulkSendHelper bulkSend12("ns3::TcpSocketFactory",
                              InetSocketAddress(interfaces12.GetAddress(1), sinkPort3));
    bulkSend12.SetAttribute("MaxBytes", UintegerValue(500000)); // Enviar 500 KB
    ApplicationContainer clientApps12 = bulkSend12.Install(nodes.Get(1));
    clientApps12.Start(Seconds(21.));
    clientApps12.Stop(Seconds(30.));

    // Aplicación del nodo 2 al nodo 0 - puerto 8083
    BulkSendHelper bulkSend20("ns3::TcpSocketFactory",
                              InetSocketAddress(interfaces02.GetAddress(0), sinkPort4));
    bulkSend20.SetAttribute("MaxBytes", UintegerValue(500000)); // Enviar 500 KB
    ApplicationContainer clientApps20 = bulkSend20.Install(nodes.Get(2));
    clientApps20.Start(Seconds(1.));
    clientApps20.Stop(Seconds(10.));

    // Aplicación del nodo 2 al nodo 1 - puerto 8084
    BulkSendHelper bulkSend21("ns3::TcpSocketFactory",
                              InetSocketAddress(interfaces12.GetAddress(0), sinkPort5));
    bulkSend21.SetAttribute("MaxBytes", UintegerValue(500000)); // Enviar 500 KB
    ApplicationContainer clientApps21 = bulkSend21.Install(nodes.Get(2));
    clientApps21.Start(Seconds(11.));
    clientApps21.Stop(Seconds(20.));

    // Configurar movilidad para animación (nodos en triángulo)
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Crear animación
    AnimationInterface anim("three-node-animation.xml");
    anim.SetConstantPosition(nodes.Get(0), 10, 20);
    anim.SetConstantPosition(nodes.Get(1), 50, 20);
    anim.SetConstantPosition(nodes.Get(2), 30, 40);
    anim.EnablePacketMetadata(true); // Habilitar metadatos de paquetes para mejor visualización

    // Configurar trazas
    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream01 = asciiTraceHelper.CreateFileStream("three-node-01.cwnd");
    Ptr<OutputStreamWrapper> stream02 = asciiTraceHelper.CreateFileStream("three-node-02.cwnd");
    Ptr<OutputStreamWrapper> stream12 = asciiTraceHelper.CreateFileStream("three-node-12.cwnd");
    Ptr<OutputStreamWrapper> stream20 = asciiTraceHelper.CreateFileStream("three-node-20.cwnd");
    Ptr<OutputStreamWrapper> stream21 = asciiTraceHelper.CreateFileStream("three-node-21.cwnd");

    // Reducir el tiempo de simulación para mejorar el rendimiento
    Simulator::Stop(Seconds(31));
    Simulator::Run();
    NS_LOG_INFO("Simulación completada.");
    Simulator::Destroy();

    return 0;
}