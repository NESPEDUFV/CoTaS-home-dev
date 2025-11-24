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
NS_LOG_COMPONENT_DEFINE("smartHouse_80");

int main(int argc, char* argv[])
{
// ----------------- Configura logs do terminal -------------------------

    LogComponentEnable("smartHouse_80", LOG_LEVEL_INFO); 
    LogComponentEnable("CoTaSApplication", LOG_LEVEL_INFO); 
    LogComponentEnable("ContextProviderApplication", LOG_LEVEL_INFO);
    LogComponentEnable("ContextConsumerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("GenericServerApplication", LOG_LEVEL_INFO);

// ----------------- Cria grupos dos nós de cada objeto -----------------
    NS_LOG_INFO("Configurando nós");

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

    NodeContainer armarioCozinhaNode;
    armarioCozinhaNode.Create(1);

    NodeContainer armarioBanheiroNode;
    armarioBanheiroNode.Create(1);

    NodeContainer aCNodes;
    aCNodes.Create(6);

    NodeContainer anelNode;
    anelNode.Create(2);

    NodeContainer cafeteiraNode;
    cafeteiraNode.Create(1);

    NodeContainer chuveiroNode;
    chuveiroNode.Create(1);

    NodeContainer carroNode;
    carroNode.Create(1);

    NodeContainer colarNode;
    colarNode.Create(2);

    NodeContainer coleiraNode;
    coleiraNode.Create(1);

    NodeContainer comedouroNode;
    comedouroNode.Create(1);

    NodeContainer escovaNode;
    escovaNode.Create(1);

    NodeContainer fogaoNode;
    fogaoNode.Create(1);

    NodeContainer geladeiraNode;
    geladeiraNode.Create(1);

    NodeContainer gotejadorNode;
    gotejadorNode.Create(1);

    NodeContainer janelaCortinaNodes;
    janelaCortinaNodes.Create(10);

    NodeContainer lampadaNodes;
    lampadaNodes.Create(7);
    
    NodeContainer interruptorNodes;
    interruptorNodes.Create(7);

    NodeContainer lavaLouçasNode;
    lavaLouçasNode.Create(1);

    NodeContainer panelaNode;
    panelaNode.Create(1);

    NodeContainer portaNodes;
    portaNodes.Create(8);

    NodeContainer relogioNode;
    relogioNode.Create(2);

    // Robô aspirador de pó
    NodeContainer RAPNode;
    RAPNode.Create(1);

    // Sensores: Umidade, MonoxidoCarbono, Nitrogenio, Fosforo, Potassio, Ph, DioxidoCarbono, Salinidade, Precipitacao
    NodeContainer jardimNode;
    jardimNode.Create(1);

    NodeContainer SFumaca;
    SFumaca.Create(7);

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
        armarioCozinhaNode, armarioBanheiroNode, 
        chuveiroNode, carroNode, fogaoNode, geladeiraNode, gotejadorNode,
        colarNode, coleiraNode, comedouroNode, escovaNode, 
        lavaLouçasNode, panelaNode, portaNodes, relogioNode, RAPNode, 
        jardimNode, SFumaca, interruptorNodes, aCNodes, 
        anelNode, cafeteiraNode, janelaCortinaNodes, lampadaNodes, 
        roteadorNodes, CoTaSNode, ApplicationsNodes
    ); //PisoNodes,

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
    cameraDev, guardaRoupaDev, armarioCozinhaDev, 
    armarioBanheiroDev, aCNDev, anelDev, cafeteiraDev, 
    chuveiroDev, carroDev, colarDev, coleiraDev, 
    comedouroDev, escovaDev, fogaoDev, geladeiraDev, 
    gotejadorDev, janelaCortinaDev, lampadaDev, 
    lavaLouçasDev, panelaDev, portaDev, relogioDev, 
    RAPDev, jardimDev, SFumacaDev, interruptorDev,
    CoTaSDev, ApplicationsDev;

    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), 
                "ActiveProbing", BooleanValue(false));

    computadorDev = wifi.Install(phy, mac, computadorNode);
    espelhoDev = wifi.Install(phy, mac, espelhoNodes);
    televisaoDev = wifi.Install(phy, mac, televisaoNode);
    echoDotDev = wifi.Install(phy, mac, echoDotNodes);
    cameraDev = wifi.Install(phy, mac, cameraNodes);
    guardaRoupaDev = wifi.Install(phy, mac, guardaRoupaNode);
    armarioCozinhaDev = wifi.Install(phy, mac, armarioCozinhaNode);
    armarioBanheiroDev = wifi.Install(phy, mac, armarioBanheiroNode);
    aCNDev = wifi.Install(phy, mac, aCNodes);
    anelDev = wifi.Install(phy, mac, anelNode);
    cafeteiraDev = wifi.Install(phy, mac, cafeteiraNode);
    chuveiroDev = wifi.Install(phy, mac, chuveiroNode);
    carroDev = wifi.Install(phy, mac, carroNode);
    colarDev = wifi.Install(phy, mac, colarNode);
    coleiraDev = wifi.Install(phy, mac, coleiraNode);
    comedouroDev = wifi.Install(phy, mac, comedouroNode);
    escovaDev = wifi.Install(phy, mac, escovaNode);
    fogaoDev = wifi.Install(phy, mac, fogaoNode);
    geladeiraDev = wifi.Install(phy, mac, geladeiraNode);
    gotejadorDev = wifi.Install(phy, mac, gotejadorNode);
    janelaCortinaDev = wifi.Install(phy, mac, janelaCortinaNodes);
    lampadaDev = wifi.Install(phy, mac, lampadaNodes);
    lavaLouçasDev = wifi.Install(phy, mac, lavaLouçasNode);
    panelaDev = wifi.Install(phy, mac, panelaNode);
    portaDev = wifi.Install(phy, mac, portaNodes);
    relogioDev = wifi.Install(phy, mac, relogioNode);
    RAPDev = wifi.Install(phy, mac, RAPNode);
    jardimDev = wifi.Install(phy, mac, jardimNode);
    SFumacaDev = wifi.Install(phy, mac, SFumaca);
    interruptorDev = wifi.Install(phy, mac, interruptorNodes);
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
    NS_LOG_INFO("Posicionando nós");

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

    position = armarioCozinhaNode.Get(0)->GetObject<MobilityModel>(); //15
    position->SetPosition(Vector(18, 4.1, 0.6)); // armario Cozinha

    position = armarioBanheiroNode.Get(0)->GetObject<MobilityModel>(); //16
    position->SetPosition(Vector(8, 2, 0.6)); // armario Banheiro

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
    
    mobility.Install(anelNode);
    position = anelNode.Get(0)->GetObject<MobilityModel>(); //23
    position->SetPosition(Vector(2, 6, 0.0)); // anel
    
    mobility.Install(anelNode);
    position = anelNode.Get(1)->GetObject<MobilityModel>(); //24
    position->SetPosition(Vector(2, 7, 0.0)); // anel

    position = cafeteiraNode.Get(0)->GetObject<MobilityModel>(); //25
    position->SetPosition(Vector(16.5, 3.5, 0.8)); // Cafeteira

    position = chuveiroNode.Get(0)->GetObject<MobilityModel>(); //26
    position->SetPosition(Vector(11, 1, 0.9)); // Chuveiro

    position = carroNode.Get(0)->GetObject<MobilityModel>(); //27
    position->SetPosition(Vector(15, 14, 1.0)); // Carro

    mobility.Install(colarNode);
    position = colarNode.Get(0)->GetObject<MobilityModel>(); //28
    position->SetPosition(Vector(2.3, 6, 0.0)); // Colar
    
    position = colarNode.Get(1)->GetObject<MobilityModel>(); //29
    position->SetPosition(Vector(2.3, 7, 0.0)); // Colar
    
    mobility.Install(coleiraNode);
    position = coleiraNode.Get(0)->GetObject<MobilityModel>(); //28
    position->SetPosition(Vector(2.3, 6.3, 0.0)); // Coleira

    position = comedouroNode.Get(0)->GetObject<MobilityModel>(); //29
    position->SetPosition(Vector(14, 4, 1.1)); // Comedouro

    position = escovaNode.Get(0)->GetObject<MobilityModel>(); //30
    position->SetPosition(Vector(8, 2.1, 1.2)); // Escova
    
    position = fogaoNode.Get(0)->GetObject<MobilityModel>(); //31
    position->SetPosition(Vector(18, 1, 1.3)); // Fogão
    
    position = geladeiraNode.Get(0)->GetObject<MobilityModel>(); //32
    position->SetPosition(Vector(13, 1, 1.4)); // Geladeira

    position = gotejadorNode.Get(0)->GetObject<MobilityModel>(); //33
    position->SetPosition(Vector(5, 12, 1.5)); // Gotejador

    position = janelaCortinaNodes.Get(0)->GetObject<MobilityModel>(); //34
    position->SetPosition(Vector(2, 12, 1.6)); // JanelaCortina sala 1 1

    position = janelaCortinaNodes.Get(1)->GetObject<MobilityModel>(); //35
    position->SetPosition(Vector(0, 9, 1.6)); // JanelaCortina sala 2 2
    
    position = janelaCortinaNodes.Get(2)->GetObject<MobilityModel>(); //36
    position->SetPosition(Vector(0, 6, 1.6)); // JanelaCortina corredor 1 3
    
    position = janelaCortinaNodes.Get(3)->GetObject<MobilityModel>(); //37
    position->SetPosition(Vector(0, 3, 1.6)); // JanelaCortina quarto 1 4
    
    position = janelaCortinaNodes.Get(4)->GetObject<MobilityModel>(); //38
    position->SetPosition(Vector(4, 0, 0)); // JanelaCortina quarto 2 5
    
    position = janelaCortinaNodes.Get(5)->GetObject<MobilityModel>(); //39
    position->SetPosition(Vector(9, 0, 0)); // JanelaCortina banheiro 6
    
    position = janelaCortinaNodes.Get(6)->GetObject<MobilityModel>(); //40
    position->SetPosition(Vector(15, 0, 1.6)); // JanelaCortina cozinha large 7
    
    position = janelaCortinaNodes.Get(7)->GetObject<MobilityModel>(); //41
    position->SetPosition(Vector(19, 2, 1.6)); // JanelaCortina cozinha small 8
    
    position = janelaCortinaNodes.Get(8)->GetObject<MobilityModel>(); //42
    position->SetPosition(Vector(19, 5, 1.6)); // JanelaCortina corredor 2 9
    
    position = janelaCortinaNodes.Get(9)->GetObject<MobilityModel>(); //43
    position->SetPosition(Vector(10, 14, 1.6)); // JanelaCortina garagem 10
    
    position = lampadaNodes.Get(0)->GetObject<MobilityModel>(); //44
    position->SetPosition(Vector(5, 9, 1.7)); // lampada sala 1
    
    position = lampadaNodes.Get(1)->GetObject<MobilityModel>(); //45
    position->SetPosition(Vector(8, 5.1, 1.7)); // lampada corredor 2
    
    position = lampadaNodes.Get(2)->GetObject<MobilityModel>(); //46
    position->SetPosition(Vector(4, 2, 1.7)); // lampada quarto 3
    
    position = lampadaNodes.Get(3)->GetObject<MobilityModel>(); //47
    position->SetPosition(Vector(9, 2, 1.7)); // lampada banheiro 4
    
    position = lampadaNodes.Get(4)->GetObject<MobilityModel>(); //48
    position->SetPosition(Vector(14, 3, 1.7)); // lampada cozinha 5
    
    position = lampadaNodes.Get(5)->GetObject<MobilityModel>(); //49
    position->SetPosition(Vector(15, 8, 1.7)); // lampada dispensa 6
    
    position = lampadaNodes.Get(6)->GetObject<MobilityModel>(); //50
    position->SetPosition(Vector(14, 14, 1.7)); // lampada garagem 7

    position = interruptorNodes.Get(0)->GetObject<MobilityModel>(); //51
    position->SetPosition(Vector(4, 12, 1.8)); // interruptor sala 1
    
    position = interruptorNodes.Get(1)->GetObject<MobilityModel>(); //52
    position->SetPosition(Vector(4, 7, 1.8)); // interruptor corredor 2
    
    position = interruptorNodes.Get(2)->GetObject<MobilityModel>(); //53
    position->SetPosition(Vector(11, 8, 1.8)); // interruptor quarto 3
    
    position = interruptorNodes.Get(3)->GetObject<MobilityModel>(); //54
    position->SetPosition(Vector(2.1, 4, 1.8)); // interruptor banheiro 4
    
    position = interruptorNodes.Get(4)->GetObject<MobilityModel>(); //55
    position->SetPosition(Vector(9, 4, 1.8)); // interruptor cozinha 5
    
    position = interruptorNodes.Get(5)->GetObject<MobilityModel>(); //56
    position->SetPosition(Vector(15, 4, 1.8)); // interruptor dispensa 6
    
    position = interruptorNodes.Get(6)->GetObject<MobilityModel>(); //57
    position->SetPosition(Vector(14.8, 5.8, 1.8)); // interruptor garagem 7

    position = lavaLouçasNode.Get(0)->GetObject<MobilityModel>(); //58
    position->SetPosition(Vector(17, 1, 1.9)); // lavaLouças

    position = panelaNode.Get(0)->GetObject<MobilityModel>(); //59
    position->SetPosition(Vector(17, 4.1, 2.0)); // Panela 1

    position = portaNodes.Get(0)->GetObject<MobilityModel>(); //61
    position->SetPosition(Vector(4, 12.1, 2.1)); // Porta sala ▽ 1
    
    position = portaNodes.Get(1)->GetObject<MobilityModel>(); //62
    position->SetPosition(Vector(4, 7.1, 2.1)); // Porta sala ▲ 2
    
    position = portaNodes.Get(2)->GetObject<MobilityModel>(); //63
    position->SetPosition(Vector(11, 8.1, 2.1)); // Porta sala ▻ 3
    
    position = portaNodes.Get(3)->GetObject<MobilityModel>(); //64
    position->SetPosition(Vector(2, 4.1, 2.1)); // Porta quarto  4
    
    position = portaNodes.Get(4)->GetObject<MobilityModel>(); //65
    position->SetPosition(Vector(9, 4.1, 2.1)); // Porta banheiro 5

    position = portaNodes.Get(5)->GetObject<MobilityModel>(); //66
    position->SetPosition(Vector(15, 4.1, 2.1)); // Porta cozinha 6
    
    position = portaNodes.Get(6)->GetObject<MobilityModel>(); //67
    position->SetPosition(Vector(15, 6.1, 2.1)); // Porta dispensa corredor 7
    
    position = portaNodes.Get(7)->GetObject<MobilityModel>(); //68
    position->SetPosition(Vector(16, 10, 2.1)); // Porta garagem 8

    mobility.Install(relogioNode);
    position = relogioNode.Get(0)->GetObject<MobilityModel>(); //69
    position->SetPosition(Vector(3, 7, 0.0)); // Relogio

    position = relogioNode.Get(1)->GetObject<MobilityModel>(); //69
    position->SetPosition(Vector(3, 8, 0.0)); // Relogio

    mobility.Install(RAPNode);
    position = RAPNode.Get(0)->GetObject<MobilityModel>(); //70
    position->SetPosition(Vector(8, 6, 0.0)); // RAP

    position = jardimNode.Get(0)->GetObject<MobilityModel>(); //71
    position->SetPosition(Vector(4, 16, 2.2)); // Jardim <<<<<<<<<<

    position = SFumaca.Get(0)->GetObject<MobilityModel>(); //72
    position->SetPosition(Vector(5, 9.1, 2.3)); // Sensor Fumaca sala 1
    
    position = SFumaca.Get(1)->GetObject<MobilityModel>(); //73
    position->SetPosition(Vector(8, 5.2, 2.3)); // Sensor Fumaca corredor 2
    
    position = SFumaca.Get(2)->GetObject<MobilityModel>(); //74
    position->SetPosition(Vector(4, 2.1, 2.3)); // Sensor Fumaca quarto 3
    
    position = SFumaca.Get(3)->GetObject<MobilityModel>(); //75
    position->SetPosition(Vector(9, 2.1, 2.3)); // Sensor Fumaca banheiro 4
    
    position = SFumaca.Get(4)->GetObject<MobilityModel>(); //76
    position->SetPosition(Vector(14, 3.1, 2.3)); // Sensor Fumaca cozinha 5
    
    position = SFumaca.Get(5)->GetObject<MobilityModel>(); //77
    position->SetPosition(Vector(15, 8.1, 2.3)); // Sensor Fumaca dispensa 6
    
    position = SFumaca.Get(6)->GetObject<MobilityModel>(); //78
    position->SetPosition(Vector(14, 14.1, 2.3)); // Sensor Fumaca garagem 7
    
//  -------------------------- Roteadores ---------------------------

    position = roteadorNodes.Get(0)->GetObject<MobilityModel>(); // 100
    position->SetPosition(Vector(8, 5.3, 0.0)); // roteador 1
    
    position = roteadorNodes.Get(1)->GetObject<MobilityModel>(); // 101
    position->SetPosition(Vector(4, 5.3, 0.0)); // roteador 2
    
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
    Ipv4InterfaceContainer armarioCozinhaInterface = ipv4.Assign(armarioCozinhaDev);
    Ipv4InterfaceContainer armarioBanheiroInterface = ipv4.Assign(armarioBanheiroDev);
    Ipv4InterfaceContainer aCInterface = ipv4.Assign(aCNDev);
    Ipv4InterfaceContainer anelInterface = ipv4.Assign(anelDev); 
    Ipv4InterfaceContainer cafeteiraInterface = ipv4.Assign(cafeteiraDev);
    Ipv4InterfaceContainer chuveiroInterface = ipv4.Assign(chuveiroDev);
    Ipv4InterfaceContainer carroInterface = ipv4.Assign(carroDev);
    Ipv4InterfaceContainer colarInterface = ipv4.Assign(colarDev);
    Ipv4InterfaceContainer coleiraInterface = ipv4.Assign(coleiraDev);
    Ipv4InterfaceContainer comedouroInterface = ipv4.Assign(comedouroDev);
    Ipv4InterfaceContainer escovaInterface = ipv4.Assign(escovaDev);
    Ipv4InterfaceContainer fogaoInterface = ipv4.Assign(fogaoDev);
    Ipv4InterfaceContainer geladeiraInterface = ipv4.Assign(geladeiraDev);
    Ipv4InterfaceContainer gotejadorInterface = ipv4.Assign(gotejadorDev);
    Ipv4InterfaceContainer janelaCortinaInterface = ipv4.Assign(janelaCortinaDev);
    Ipv4InterfaceContainer lampadaInterface = ipv4.Assign(lampadaDev);
    Ipv4InterfaceContainer interruptorInterface = ipv4.Assign(interruptorDev); 
    Ipv4InterfaceContainer lavaLouçasInterface = ipv4.Assign(lavaLouçasDev);
    Ipv4InterfaceContainer panelaInterface = ipv4.Assign(panelaDev);
    Ipv4InterfaceContainer portaInterface = ipv4.Assign(portaDev);
    Ipv4InterfaceContainer relogioInterface = ipv4.Assign(relogioDev);
    Ipv4InterfaceContainer RAPInterface = ipv4.Assign(RAPDev);
    Ipv4InterfaceContainer jardimInterface = ipv4.Assign(jardimDev);
    Ipv4InterfaceContainer SFuInterface = ipv4.Assign(SFumacaDev);
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

    // -------------------------- CoTaS APP ---------------------------------
    NS_LOG_INFO("Configurando CoTaS");

    CoTaSHelper Service(9);
    ApplicationContainer serverApplications;
    ApplicationContainer serverApp = Service.Install(CoTaSNode.Get(0));
    serverApplications.Add(serverApp);

    // ------------------------- Serviço dos objetos inteligentes-----------
    NS_LOG_INFO("Configurando serviço dos objetos");
    GenericServerHelper ObjectService(19);

    ApplicationContainer computadorServerApp, espelhoServerApp, 
        televisaoServerApp, echoDotServerApp, cameraServerApp, 
        guardaRoupaServerApp , armarioCozinhaServerApp, 
        armarioBanheiroServerApp, aCServerApp, anelServerApp, 
        cafeteiraServerApp, chuveiroServerApp, carroServerApp, 
        colarServerApp, coleiraServerApp, comedouroServerApp, 
        escovaServerApp, fogaoServerApp, geladeiraServerApp, 
        gotejadorServerApp, janelaCortinaServerApp, lampadaServerApp, 
        interruptorServerApp, lavaLouçasServerApp, panelaServerApp, 
        portaServerApp, relogioServerApp, RAPServerApp, jardimServerApp, 
        SFuServerApp ;

    ObjectService.SetAttribute("ObjectType", UintegerValue(0));
    computadorServerApp = ObjectService.Install(computadorNode);

    ObjectService.SetAttribute("ObjectType", UintegerValue(1));
    espelhoServerApp = ObjectService.Install(espelhoNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(2));
    televisaoServerApp = ObjectService.Install(televisaoNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(3));
    echoDotServerApp = ObjectService.Install(echoDotNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(4));
    cameraServerApp = ObjectService.Install(cameraNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(5));
    guardaRoupaServerApp = ObjectService.Install(guardaRoupaNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(6));
    armarioCozinhaServerApp = ObjectService.Install(armarioCozinhaNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(7));
    armarioBanheiroServerApp = ObjectService.Install(armarioBanheiroNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(8));
    aCServerApp = ObjectService.Install(aCNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(9));
    anelServerApp = ObjectService.Install(anelNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(10));
    cafeteiraServerApp = ObjectService.Install(cafeteiraNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(11));
    chuveiroServerApp = ObjectService.Install(chuveiroNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(12));
    carroServerApp = ObjectService.Install(carroNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(13));
    colarServerApp = ObjectService.Install(colarNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(14));
    coleiraServerApp = ObjectService.Install(coleiraNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(15));
    comedouroServerApp = ObjectService.Install(comedouroNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(16));
    escovaServerApp = ObjectService.Install(escovaNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(17));
    fogaoServerApp = ObjectService.Install(fogaoNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(18));
    geladeiraServerApp = ObjectService.Install(geladeiraNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(19));
    gotejadorServerApp = ObjectService.Install(gotejadorNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(20));
    janelaCortinaServerApp = ObjectService.Install(janelaCortinaNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(21));
    lampadaServerApp = ObjectService.Install(lampadaNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(22));
    interruptorServerApp = ObjectService.Install(interruptorNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(23));
    lavaLouçasServerApp = ObjectService.Install(lavaLouçasNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(24));
    panelaServerApp = ObjectService.Install(panelaNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(26));
    portaServerApp = ObjectService.Install(portaNodes);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(27));
    relogioServerApp = ObjectService.Install(relogioNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(28));
    RAPServerApp = ObjectService.Install(RAPNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(29));
    jardimServerApp = ObjectService.Install(jardimNode);
    
    ObjectService.SetAttribute("ObjectType", UintegerValue(30));
    SFuServerApp = ObjectService.Install(SFumaca);

    serverApplications.Add(computadorServerApp);
    serverApplications.Add(espelhoServerApp);
    serverApplications.Add(televisaoServerApp);
    serverApplications.Add(echoDotServerApp);
    serverApplications.Add(cameraServerApp);
    serverApplications.Add(guardaRoupaServerApp);
    serverApplications.Add(armarioCozinhaServerApp);
    serverApplications.Add(armarioBanheiroServerApp);
    serverApplications.Add(aCServerApp);
    serverApplications.Add(anelServerApp);
    serverApplications.Add(cafeteiraServerApp);
    serverApplications.Add(chuveiroServerApp);
    serverApplications.Add(carroServerApp);
    serverApplications.Add(colarServerApp);
    serverApplications.Add(coleiraServerApp);
    serverApplications.Add(comedouroServerApp);
    serverApplications.Add(escovaServerApp);
    serverApplications.Add(fogaoServerApp);
    serverApplications.Add(geladeiraServerApp);
    serverApplications.Add(gotejadorServerApp);
    serverApplications.Add(janelaCortinaServerApp);
    serverApplications.Add(lampadaServerApp);
    serverApplications.Add(interruptorServerApp);
    serverApplications.Add(lavaLouçasServerApp);
    serverApplications.Add(panelaServerApp);
    serverApplications.Add(portaServerApp);
    serverApplications.Add(relogioServerApp);
    serverApplications.Add(RAPServerApp);
    serverApplications.Add(jardimServerApp);
    serverApplications.Add(SFuServerApp);

    serverApplications.Start(Seconds(1));
    serverApplications.Stop(Seconds(20));

    // ------------------------- Clientes provedores -----------------------

    NS_LOG_INFO("Configurando aplicação cliente dos objetos");
    ApplicationContainer providerApplications;
    ContextProviderHelper ClientProvider(CoTaSInterface.GetAddress(0), 9);
    ClientProvider.SetAttribute("MaxPackets", UintegerValue(200));
    ClientProvider.SetAttribute("Interval", TimeValue(MilliSeconds(50)));
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(0));
    ApplicationContainer computadorApp = ClientProvider.Install(computadorNode);

    ClientProvider.SetAttribute("ObjectType", UintegerValue(1));
    ApplicationContainer espelhoApp = ClientProvider.Install(espelhoNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(2));
    ApplicationContainer televisaoApp = ClientProvider.Install(televisaoNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(3));
    ApplicationContainer echoDotApp = ClientProvider.Install(echoDotNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(4));
    ApplicationContainer cameraApp = ClientProvider.Install(cameraNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(5));
    ApplicationContainer guardaRoupaApp  = ClientProvider.Install(guardaRoupaNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(6));
    ApplicationContainer armarioCozinhaApp = ClientProvider.Install(armarioCozinhaNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(7));
    ApplicationContainer armarioBanheiroApp = ClientProvider.Install(armarioBanheiroNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(8));
    ApplicationContainer aCApp = ClientProvider.Install(aCNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(9));
    ApplicationContainer anelApp = ClientProvider.Install(anelNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(10));
    ApplicationContainer cafeteiraApp = ClientProvider.Install(cafeteiraNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(11));
    ApplicationContainer chuveiroApp = ClientProvider.Install(chuveiroNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(12));
    ApplicationContainer carroApp = ClientProvider.Install(carroNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(13));
    ApplicationContainer colarApp = ClientProvider.Install(colarNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(14));
    ApplicationContainer coleiraApp = ClientProvider.Install(coleiraNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(15));
    ApplicationContainer comedouroApp = ClientProvider.Install(comedouroNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(16));
    ApplicationContainer escovaApp = ClientProvider.Install(escovaNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(17));
    ApplicationContainer fogaoApp = ClientProvider.Install(fogaoNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(18));
    ApplicationContainer geladeiraApp = ClientProvider.Install(geladeiraNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(19));
    ApplicationContainer gotejadorApp = ClientProvider.Install(gotejadorNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(20));
    ApplicationContainer janelaCortinaApp = ClientProvider.Install(janelaCortinaNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(21));
    ApplicationContainer lampadaApp = ClientProvider.Install(lampadaNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(22));
    ApplicationContainer interruptorApp = ClientProvider.Install(interruptorNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(23));
    ApplicationContainer lavaLouçasApp = ClientProvider.Install(lavaLouçasNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(24));
    ApplicationContainer panelaApp = ClientProvider.Install(panelaNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(26));
    ApplicationContainer portaApp = ClientProvider.Install(portaNodes);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(27));
    ApplicationContainer relogioApp = ClientProvider.Install(relogioNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(28));
    ApplicationContainer RAPApp = ClientProvider.Install(RAPNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(29));
    ApplicationContainer jardimApp = ClientProvider.Install(jardimNode);
    
    ClientProvider.SetAttribute("ObjectType", UintegerValue(30));
    ApplicationContainer SFuApp = ClientProvider.Install(SFumaca);

    providerApplications.Add(computadorApp);
    providerApplications.Add(espelhoApp);
    providerApplications.Add(televisaoApp);
    providerApplications.Add(echoDotApp);
    providerApplications.Add(cameraApp);
    providerApplications.Add(guardaRoupaApp );
    providerApplications.Add(armarioCozinhaApp);
    providerApplications.Add(armarioBanheiroApp);
    providerApplications.Add(aCApp);
    providerApplications.Add(anelApp);
    providerApplications.Add(cafeteiraApp);
    providerApplications.Add(chuveiroApp);
    providerApplications.Add(carroApp);
    providerApplications.Add(colarApp);
    providerApplications.Add(coleiraApp);
    providerApplications.Add(comedouroApp);
    providerApplications.Add(escovaApp);
    providerApplications.Add(fogaoApp);
    providerApplications.Add(geladeiraApp);
    providerApplications.Add(gotejadorApp);
    providerApplications.Add(janelaCortinaApp);
    providerApplications.Add(lampadaApp);
    providerApplications.Add(interruptorApp);
    providerApplications.Add(lavaLouçasApp);
    providerApplications.Add(panelaApp);
    providerApplications.Add(portaApp);
    providerApplications.Add(relogioApp);
    providerApplications.Add(RAPApp);
    providerApplications.Add(jardimApp);
    providerApplications.Add(SFuApp);

    providerApplications.Start(Seconds(10));
    providerApplications.Stop(Seconds(20));

    // -------------------------- Clientes consumidores ----------------
    
    NS_LOG_INFO("Configurando aplicação cliente dos consumidores");
    ApplicationContainer consumerApplications;
    ContextConsumerHelper ClientConsumer(CoTaSInterface.GetAddress(0), 9);
    ClientConsumer.SetAttribute("MaxPackets", UintegerValue(80));
    ClientConsumer.SetAttribute("Interval", TimeValue(MilliSeconds(100)));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(0));
    ApplicationContainer FallDetection = ClientConsumer.Install(ApplicationsNodes.Get(0));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(1));
    ApplicationContainer MicroControl = ClientConsumer.Install(ApplicationsNodes.Get(1));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(2));
    ApplicationContainer PetCare = ClientConsumer.Install(ApplicationsNodes.Get(2));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(3));
    ApplicationContainer EnergyManegement = ClientConsumer.Install(ApplicationsNodes.Get(3));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(4));
    ApplicationContainer WaterManegement = ClientConsumer.Install(ApplicationsNodes.Get(4));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(5));
    ApplicationContainer Security = ClientConsumer.Install(ApplicationsNodes.Get(5));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(6));
    ApplicationContainer Localization = ClientConsumer.Install(ApplicationsNodes.Get(6));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(7));
    ApplicationContainer GasSec = ClientConsumer.Install(ApplicationsNodes.Get(7));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(8));
    ApplicationContainer HealthCare = ClientConsumer.Install(ApplicationsNodes.Get(8));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(9));
    ApplicationContainer LightControl = ClientConsumer.Install(ApplicationsNodes.Get(9));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(10));
    ApplicationContainer TempControl = ClientConsumer.Install(ApplicationsNodes.Get(10));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(11));
    ApplicationContainer SmartCleaning = ClientConsumer.Install(ApplicationsNodes.Get(11));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(12));
    ApplicationContainer Garden = ClientConsumer.Install(ApplicationsNodes.Get(12));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(13));
    ApplicationContainer SmartMobility = ClientConsumer.Install(ApplicationsNodes.Get(13));

    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(14));
    ApplicationContainer SmartCooking = ClientConsumer.Install(ApplicationsNodes.Get(14));
    
    ClientConsumer.SetAttribute("ApplicationType", UintegerValue(15));
    ApplicationContainer InventoryManagement = ClientConsumer.Install(ApplicationsNodes.Get(15));

    consumerApplications.Add(FallDetection);
    consumerApplications.Add(MicroControl);
    consumerApplications.Add(PetCare);
    consumerApplications.Add(EnergyManegement);
    consumerApplications.Add(WaterManegement);
    consumerApplications.Add(Security);
    consumerApplications.Add(Localization);
    consumerApplications.Add(GasSec);
    consumerApplications.Add(HealthCare);
    consumerApplications.Add(LightControl);
    consumerApplications.Add(TempControl);
    consumerApplications.Add(SmartCleaning);
    consumerApplications.Add(Garden);
    consumerApplications.Add(SmartMobility);
    consumerApplications.Add(SmartCooking);
    consumerApplications.Add(InventoryManagement);
    
    consumerApplications.Start(Seconds(12));
    consumerApplications.Stop(Seconds(20));
    
    Simulator::Stop(Seconds(21.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}