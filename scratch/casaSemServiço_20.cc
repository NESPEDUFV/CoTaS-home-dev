#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/netanim-module.h"
#include "ns3/csma-module.h"
#include "ns3/random-walk-2d-mobility-model.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

// ----------------- Casa inteligente em um grid 21x21 ------------------
NS_LOG_COMPONENT_DEFINE("smartHouse_20");

int main(int argc, char* argv[])
{
// ----------------- Configura logs do terminal -------------------------

    LogComponentEnable("CoTaSApplication", LOG_LEVEL_INFO); 
    LogComponentEnable("ContextProviderApplication", LOG_LEVEL_INFO);
    LogComponentEnable("ContextConsumerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("GenericApplication", LOG_LEVEL_INFO);

// ----------------- Cria grupos dos nós de cada objeto -----------------
// 97 instancias
// 16 aplicações + CoTaS

    NodeContainer computadorNode;
    computadorNode.Create(1);

    NodeContainer espelhoNodes;
    espelhoNodes.Create(2);

    NodeContainer televisaoNode;
    televisaoNode.Create(1);

    NodeContainer echoDotNodes;
    echoDotNodes.Create(3);

    NodeContainer cameraNodes;
    cameraNodes.Create(6);

    NodeContainer guardaRoupaNode;
    guardaRoupaNode.Create(1);

    NodeContainer aCNodes;
    aCNodes.Create(6);

    NodeContainer roteadorNodes;
    roteadorNodes.Create(3);

    NodeContainer CoTaSNode;
    CoTaSNode.Create(1);

    NodeContainer ApplicationsNodes;
    ApplicationsNodes.Create(16);


// ------------------------ Agrupa todos os nós -------------------------

    NodeContainer allNodes = NodeContainer( 
        computadorNode, espelhoNodes, televisaoNode, 
        echoDotNodes, cameraNodes, guardaRoupaNode, 
        aCNodes, roteadorNodes, CoTaSNode, ApplicationsNodes
    );

// -------------- Configura pilha de protocolos da internet -------------

    InternetStackHelper internet;
    internet.Install(allNodes);
    
// --------------------- Liga pontos de acesso --------------------------

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("500Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(2560)));

    NetDeviceContainer csmaDevices;
    
    csmaDevices = csma.Install(roteadorNodes);

    
// --------------------- Configurando rede wifi --------------------------

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    
    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;

    NetDeviceContainer 
    computadorDev, espelhoDev, televisaoDev, echoDotDev,
    cameraDev, guardaRoupaDev, aCNDev,
    CoTaSDev, ApplicationsDev;

    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), 
                "ActiveProbing", BooleanValue(false));

    computadorDev = wifi.Install(phy, mac, computadorNode);
    espelhoDev = wifi.Install(phy, mac, espelhoNodes);
    televisaoDev = wifi.Install(phy, mac, televisaoNode);
    echoDotDev = wifi.Install(phy, mac, echoDotNodes);
    cameraDev = wifi.Install(phy, mac, cameraNodes);
    aCNDev = wifi.Install(phy, mac, aCNodes);
    guardaRoupaDev = wifi.Install(phy, mac, guardaRoupaNode);
    CoTaSDev = wifi.Install(phy, mac, CoTaSNode);
    ApplicationsDev = wifi.Install(phy, mac, ApplicationsNodes);

    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    apDevices = wifi.Install(phy, mac, roteadorNodes.Get(0));
    
//  ----- Configura outro numero de canal para não dar interferencia ----
    phy.Set("ChannelSettings", StringValue("{6, 20, BAND_2_4GHZ, 0}"));
    apDevices.Add(wifi.Install(phy, mac, roteadorNodes.Get(1)));
    
    phy.Set("ChannelSettings", StringValue("{11, 20, BAND_2_4GHZ, 0}"));
    apDevices.Add(wifi.Install(phy, mac, roteadorNodes.Get(2)));
    
// ----------------------- Posicionando os nós --------------------------

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(allNodes);

    // Posicionar os objetos manualmente 
    // (considerando y de baixo para cima)
    
// ------------------------- Objetos inteligentes ----------------------

    Ptr<MobilityModel> position;
    position = computadorNode.Get(0)->GetObject<MobilityModel>(); // 1
    position->SetPosition(Vector(6, 4, 0.1)); // Computador

    position = espelhoNodes.Get(0)->GetObject<MobilityModel>(); // 2
    position->SetPosition(Vector(6, 1, 0.2)); // Espelho Inteligente quarto 1

    position = espelhoNodes.Get(1)->GetObject<MobilityModel>(); // 3
    position->SetPosition(Vector(11, 3, 0.2)); // Espelho Inteligente banheiro 2

    position = televisaoNode.Get(0)->GetObject<MobilityModel>(); // 4
    position->SetPosition(Vector(8.5, 11, 0.3)); // Televisão

    position = echoDotNodes.Get(0)->GetObject<MobilityModel>(); // 5
    position->SetPosition(Vector(6, 4.5, 0.4)); // Echo Dot Alexa quarto 1

    position = echoDotNodes.Get(1)->GetObject<MobilityModel>(); // 6
    position->SetPosition(Vector(17.5, 4.5, 0.4)); // Echo Dot Alexa cozinha 2

    position = echoDotNodes.Get(2)->GetObject<MobilityModel>(); // 7
    position->SetPosition(Vector(7, 7, 0.4)); // Echo Dot Alexa sala 3

    position = cameraNodes.Get(0)->GetObject<MobilityModel>(); // 8
    position->SetPosition(Vector(1.5, 5, 0.5)); // Câmera corredor 1

    position = cameraNodes.Get(1)->GetObject<MobilityModel>(); // 9
    position->SetPosition(Vector(1.3, 12.5, 0.5)); // Câmera externa 2

    position = cameraNodes.Get(2)->GetObject<MobilityModel>(); //10
    position->SetPosition(Vector(11, 7.0, 0.5)); // Câmera dispensa 3

    position = cameraNodes.Get(3)->GetObject<MobilityModel>(); //11
    position->SetPosition(Vector(11.2, 11.5, 0.5)); // Câmera garagem 4

    position = cameraNodes.Get(4)->GetObject<MobilityModel>(); //12
    position->SetPosition(Vector(10, 11, 0.5)); // Câmera sala 5

    position = cameraNodes.Get(5)->GetObject<MobilityModel>(); //13
    position->SetPosition(Vector(12, 4, 0.5)); // Câmera cozinha 6

    position = guardaRoupaNode.Get(0)->GetObject<MobilityModel>(); //14
    position->SetPosition(Vector(7, 2, 0.6)); // guarda Roupa

    position = aCNodes.Get(0)->GetObject<MobilityModel>(); //17
    position->SetPosition(Vector(7, 7.1, 0.7)); // Ar condicionado 1 sala

    position = aCNodes.Get(1)->GetObject<MobilityModel>(); // 18
    position->SetPosition(Vector(8, 5, 0.7)); // Ar condicionado 2 corredor

    position = aCNodes.Get(2)->GetObject<MobilityModel>(); //19
    position->SetPosition(Vector(7, 2.1, 0.7)); // Ar condicionado 3 quarto

    position = aCNodes.Get(3)->GetObject<MobilityModel>(); //20
    position->SetPosition(Vector(12, 3, 0.7)); // Ar condicionado 4 cozinha

    position = aCNodes.Get(4)->GetObject<MobilityModel>(); //21
    position->SetPosition(Vector(18, 8, 0.7)); // Ar condicionado 5 dispensa

    position = aCNodes.Get(5)->GetObject<MobilityModel>(); //22
    position->SetPosition(Vector(18.5, 14, 0.7)); // Ar condicionado 6 garagem
    

    // Configura nós que andam pela casa 
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Mode",
                            StringValue("Time"),
                            "Time",
                            StringValue("1s"),
                            "Speed",
                            StringValue("ns3::ConstantRandomVariable[Constant=1.0]"),
                            "Bounds",
                            StringValue("0|21|0|21"));
    
    
//  -------------------------- Roteadores ---------------------------

    position = roteadorNodes.Get(0)->GetObject<MobilityModel>(); // 100
    position->SetPosition(Vector(8, 5.3, 0.0)); // roteador 1
    
    position = roteadorNodes.Get(1)->GetObject<MobilityModel>(); // 101
    position->SetPosition(Vector(4, 5.3, 0.0)); // roteador 2scratch/casaInteligente_40.cc
    
    position = roteadorNodes.Get(2)->GetObject<MobilityModel>(); // 102
    position->SetPosition(Vector(12, 5.3, 0.0)); // roteador 3
    
// --------------------------- Aplicações ---------------------------

    position = CoTaSNode.Get(0)->GetObject<MobilityModel>(); // 99
    position->SetPosition(Vector(8, 5.5, 0.0)); // aplicação

    position = ApplicationsNodes.Get(0)->GetObject<MobilityModel>(); // 103 ?????????
    position->SetPosition(Vector(9, 4.0, 0.0)); // aplicação "fallDetection"

    position = ApplicationsNodes.Get(1)->GetObject<MobilityModel>(); // 104
    position->SetPosition(Vector(9, 4.2, 0.0)); // aplicação "microControl"

    position = ApplicationsNodes.Get(2)->GetObject<MobilityModel>(); // 105
    position->SetPosition(Vector(9, 4.4, 0.0)); // aplicação "petCare"

    position = ApplicationsNodes.Get(3)->GetObject<MobilityModel>(); // 106
    position->SetPosition(Vector(9, 4.6, 0.0)); // aplicação "energyManegement"

    position = ApplicationsNodes.Get(4)->GetObject<MobilityModel>(); // 107
    position->SetPosition(Vector(9, 4.8, 0.0)); // aplicação "waterManegement"

    position = ApplicationsNodes.Get(5)->GetObject<MobilityModel>(); // 108
    position->SetPosition(Vector(10, 4, 0.0)); // aplicação "security"

    position = ApplicationsNodes.Get(6)->GetObject<MobilityModel>(); // 109
    position->SetPosition(Vector(10, 4.2, 0.0)); // aplicação "localization"

    position = ApplicationsNodes.Get(7)->GetObject<MobilityModel>(); // 110
    position->SetPosition(Vector(10, 4.4, 0.0)); // aplicação "gasSec"

    position = ApplicationsNodes.Get(8)->GetObject<MobilityModel>(); // 11
    position->SetPosition(Vector(10, 4.6, 0.0)); // aplicação "healthCare"

    position = ApplicationsNodes.Get(9)->GetObject<MobilityModel>(); // 112
    position->SetPosition(Vector(10, 4.8, 0.0)); // aplicação "lightControl"

    position = ApplicationsNodes.Get(10)->GetObject<MobilityModel>(); // 113
    position->SetPosition(Vector(10, 3, 0.0)); // aplicação "tempControl"

    position = ApplicationsNodes.Get(11)->GetObject<MobilityModel>(); // 114
    position->SetPosition(Vector(10, 3.2, 0.0)); // aplicação "smartCleaning"

    position = ApplicationsNodes.Get(12)->GetObject<MobilityModel>(); // 115
    position->SetPosition(Vector(10, 3.4, 0.0)); // aplicação "garden"

    position = ApplicationsNodes.Get(13)->GetObject<MobilityModel>(); // 116
    position->SetPosition(Vector(10, 3.6, 0.0)); // aplicação "mobility"

    position = ApplicationsNodes.Get(14)->GetObject<MobilityModel>(); // 117
    position->SetPosition(Vector(10, 3.8, 0.0)); // aplicação "Smart cooking"

    position = ApplicationsNodes.Get(15)->GetObject<MobilityModel>(); // 118
    position->SetPosition(Vector(9, 3, 0.0)); // aplicação "inventoryManagement"

// ------------------------ Atribuir endereços IP -------------------

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.0.0", "255.255.255.0");
    Ipv4InterfaceContainer computadorInterface = ipv4.Assign(computadorDev);
    Ipv4InterfaceContainer espelhoInterface = ipv4.Assign(espelhoDev);
    Ipv4InterfaceContainer televisaoInterface = ipv4.Assign(televisaoDev);
    Ipv4InterfaceContainer echoDotInterface = ipv4.Assign(echoDotDev);
    Ipv4InterfaceContainer cameraInterface = ipv4.Assign(cameraDev);
    Ipv4InterfaceContainer guardaRoupaInterface = ipv4.Assign(guardaRoupaDev);
    Ipv4InterfaceContainer aCInterface = ipv4.Assign(aCNDev);
    Ipv4InterfaceContainer CoTaSInterface = ipv4.Assign(CoTaSDev);
    Ipv4InterfaceContainer apInterfaces = ipv4.Assign(apDevices);
    Ipv4InterfaceContainer csmaInterfaces = ipv4.Assign(csmaDevices);
    Ipv4InterfaceContainer AppInterfaces = ipv4.Assign(ApplicationsDev);
    
    // por padrão os nós APs não repassam pacotes, 
    // então precisamos ligar esse "forward"
    for (uint32_t i = 0; i < roteadorNodes.GetN(); ++i)
    {
        Ptr<Ipv4> ipv4 = roteadorNodes.Get(i)->GetObject<Ipv4>();
        ipv4->SetAttribute("IpForward", BooleanValue(true));
    }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // -------------------------- Aplicações consumidoras --------------
    
    GenericApplicationHelper Service(9);
    ApplicationContainer allServiceApp;
    ApplicationContainer FallDetection = Service.Install(ApplicationsNodes.Get(0));
    ApplicationContainer MicroControl = Service.Install(ApplicationsNodes.Get(1));
    ApplicationContainer PetCare = Service.Install(ApplicationsNodes.Get(2));
    ApplicationContainer EnergyManegement = Service.Install(ApplicationsNodes.Get(3));
    ApplicationContainer WaterManegement = Service.Install(ApplicationsNodes.Get(4));
    ApplicationContainer Security = Service.Install(ApplicationsNodes.Get(5));
    ApplicationContainer Localization = Service.Install(ApplicationsNodes.Get(6));
    ApplicationContainer GasSec = Service.Install(ApplicationsNodes.Get(7));
    ApplicationContainer HealthCare = Service.Install(ApplicationsNodes.Get(8));
    ApplicationContainer LightControl = Service.Install(ApplicationsNodes.Get(9));
    ApplicationContainer TempControl = Service.Install(ApplicationsNodes.Get(10));
    ApplicationContainer SmartCleaning = Service.Install(ApplicationsNodes.Get(11));
    ApplicationContainer Garden = Service.Install(ApplicationsNodes.Get(12));
    ApplicationContainer SmartMobility = Service.Install(ApplicationsNodes.Get(13));
    ApplicationContainer SmartCooking = Service.Install(ApplicationsNodes.Get(14));
    ApplicationContainer InventoryManagement = Service.Install(ApplicationsNodes.Get(15));
    allServiceApp.Add(FallDetection);
    allServiceApp.Add(MicroControl);
    allServiceApp.Add(PetCare);
    allServiceApp.Add(EnergyManegement);
    allServiceApp.Add(WaterManegement);
    allServiceApp.Add(Security);
    allServiceApp.Add(Localization);
    allServiceApp.Add(GasSec);
    allServiceApp.Add(HealthCare);
    allServiceApp.Add(LightControl);
    allServiceApp.Add(TempControl);
    allServiceApp.Add(SmartCleaning);
    allServiceApp.Add(Garden);
    allServiceApp.Add(SmartMobility);
    allServiceApp.Add(SmartCooking);
    allServiceApp.Add(InventoryManagement);

    allServiceApp.Start(Seconds(1));
    allServiceApp.Stop(Seconds(20));

    // ------------------------- Clientes provedores -----------------------

    // Configurando aplicações ---------------------------------------------

    ContextProviderHelper FallDetectionHelper(AppInterfaces.GetAddress(0), 9);
    FallDetectionHelper.SetAttribute("MaxPackets", UintegerValue(200));
    FallDetectionHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper MicroControlHelper(AppInterfaces.GetAddress(1), 9);
    MicroControlHelper.SetAttribute("MaxPackets", UintegerValue(200));
    MicroControlHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper PetCareHelper(AppInterfaces.GetAddress(2), 9);
    PetCareHelper.SetAttribute("MaxPackets", UintegerValue(200));
    PetCareHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper EnergyManegementHelper(AppInterfaces.GetAddress(3), 9);
    EnergyManegementHelper.SetAttribute("MaxPackets", UintegerValue(200));
    EnergyManegementHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper WaterManegementHelper(AppInterfaces.GetAddress(4), 9);
    WaterManegementHelper.SetAttribute("MaxPackets", UintegerValue(200));
    WaterManegementHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper SecurityHelper(AppInterfaces.GetAddress(5), 9);
    SecurityHelper.SetAttribute("MaxPackets", UintegerValue(200));
    SecurityHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));
    
    ContextProviderHelper LocalizationHelper(AppInterfaces.GetAddress(6), 9);
    LocalizationHelper.SetAttribute("MaxPackets", UintegerValue(200));
    LocalizationHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper GasSecHelper(AppInterfaces.GetAddress(7), 9);
    GasSecHelper.SetAttribute("MaxPackets", UintegerValue(200));
    GasSecHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper HealthCareHelper(AppInterfaces.GetAddress(8), 9);
    HealthCareHelper.SetAttribute("MaxPackets", UintegerValue(200));
    HealthCareHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper LightControlHelper(AppInterfaces.GetAddress(9), 9);
    LightControlHelper.SetAttribute("MaxPackets", UintegerValue(200));
    LightControlHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper TempControlHelper(AppInterfaces.GetAddress(10), 9);
    TempControlHelper.SetAttribute("MaxPackets", UintegerValue(200));
    TempControlHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper SmartCleaningHelper(AppInterfaces.GetAddress(11), 9);
    SmartCleaningHelper.SetAttribute("MaxPackets", UintegerValue(200));
    SmartCleaningHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper GardenHelper(AppInterfaces.GetAddress(12), 9);
    GardenHelper.SetAttribute("MaxPackets", UintegerValue(200));
    GardenHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper SmartMobilityHelper(AppInterfaces.GetAddress(13), 9);
    SmartMobilityHelper.SetAttribute("MaxPackets", UintegerValue(200));
    SmartMobilityHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper SmartCookingHelper(AppInterfaces.GetAddress(14), 9);
    SmartCookingHelper.SetAttribute("MaxPackets", UintegerValue(200));
    SmartCookingHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    ContextProviderHelper InventoryManagementHelper(AppInterfaces.GetAddress(15), 9);
    InventoryManagementHelper.SetAttribute("MaxPackets", UintegerValue(200));
    InventoryManagementHelper.SetAttribute("Interval", TimeValue(MilliSeconds(50)));

    // Instalando aplicações -----------------------------------------------

    ApplicationContainer allClientApp;
    MicroControlHelper.SetAttribute("ObjectType", UintegerValue(0));
    EnergyManegementHelper.SetAttribute("ObjectType", UintegerValue(0));
    ApplicationContainer computadorApp = MicroControlHelper.Install(computadorNode);
    computadorApp.Add(EnergyManegementHelper.Install(computadorNode));
    allClientApp.Add(computadorApp); 
    
    LightControlHelper.SetAttribute("ObjectType", UintegerValue(1));
    ApplicationContainer espelhoApp = LightControlHelper.Install(espelhoNodes);
    allClientApp.Add(espelhoApp); 
    
    EnergyManegementHelper.SetAttribute("ObjectType", UintegerValue(2));
    ApplicationContainer televisaoApp = EnergyManegementHelper.Install(televisaoNode);
    allClientApp.Add(televisaoApp);

    MicroControlHelper.SetAttribute("ObjectType", UintegerValue(3));
    ApplicationContainer echoDotApp = MicroControlHelper.Install(echoDotNodes);
    allClientApp.Add(echoDotApp);

    SecurityHelper.SetAttribute("ObjectType", UintegerValue(4));
    LocalizationHelper.SetAttribute("ObjectType", UintegerValue(4));
    FallDetectionHelper.SetAttribute("ObjectType", UintegerValue(4));
    PetCareHelper.SetAttribute("ObjectType", UintegerValue(4));
    ApplicationContainer cameraApp = SecurityHelper.Install(cameraNodes);
    cameraApp.Add(LocalizationHelper.Install(cameraNodes));
    cameraApp.Add(FallDetectionHelper.Install(cameraNodes));
    cameraApp.Add(PetCareHelper.Install(cameraNodes));
    allClientApp.Add(cameraApp);

    InventoryManagementHelper.SetAttribute("ObjectType", UintegerValue(5));
    ApplicationContainer guardaRoupaApp = InventoryManagementHelper.Install(guardaRoupaNode);
    allClientApp.Add(guardaRoupaApp);

    TempControlHelper.SetAttribute("ObjectType", UintegerValue(8));
    EnergyManegementHelper.SetAttribute("ObjectType", UintegerValue(8));
    ApplicationContainer aCApp = TempControlHelper.Install(aCNodes);
    aCApp.Add(EnergyManegementHelper.Install(aCNodes));
    allClientApp.Add(aCApp);

    allClientApp.Start(Seconds(10));
    allClientApp.Stop(Seconds(20));
    
    Simulator::Stop(Seconds(21.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}