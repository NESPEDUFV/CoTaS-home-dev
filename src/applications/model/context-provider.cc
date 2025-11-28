/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "context-provider.h"

#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/timestamp-tag.h"
#include <random>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ContextProviderApplication");

NS_OBJECT_ENSURE_REGISTERED(ContextProvider);

TypeId
ContextProvider::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ContextProvider")
            .SetParent<SourceApplication>()
            .SetGroupName("Applications")
            .AddConstructor<ContextProvider>()
            .AddAttribute(
                "MaxPackets",
                "The maximum number of packets the application will send (zero means infinite)",
                UintegerValue(100),
                MakeUintegerAccessor(&ContextProvider::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&ContextProvider::m_interval),
                          MakeTimeChecker())
            .AddAttribute(
                "RemoteAddress",
                "The destination Address of the outbound packets",
                AddressValue(),
                MakeAddressAccessor(
                    (void (ContextProvider::*)(
                        const Address&))&ContextProvider::SetRemote, // this is needed to indicate
                                                                     // which version of the
                                                                     // function overload to use
                    &ContextProvider::GetRemote),
                MakeAddressChecker(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            .AddAttribute(
                "RemotePort",
                "The destination port of the outbound packets",
                UintegerValue(ContextProvider::DEFAULT_PORT),
                MakeUintegerAccessor(&ContextProvider::SetPort, &ContextProvider::GetPort),
                MakeUintegerChecker<uint16_t>(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            .AddAttribute("ObjectType",
                          "Which object the node refers to",
                          UintegerValue(1),
                          MakeUintegerAccessor(&ContextProvider::m_objectType),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Tx",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&ContextProvider::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&ContextProvider::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("TxWithAddresses",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&ContextProvider::m_txTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&ContextProvider::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

ContextProvider::ContextProvider()
    : m_sent{0},
      m_socket{nullptr},
      m_peerPort{},
      m_sendEvent{},
      m_objectId{0},
      m_recived_messages{0},
      m_send_messages{0}
{
    std::ifstream fm("all_data/messages3.json");

    if (fm.is_open())
    {
        fm >> m_messages;
    }
    else
    {
        std::cerr << "Erro ao abrir messages.json\n";
    }

    NS_LOG_FUNCTION(this);
}

ContextProvider::~ContextProvider()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;

    coap_cleanup();
}

void
ContextProvider::SetRemote(const Address& ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    SetRemote(ip);
    SetPort(port);
}

void
ContextProvider::SetRemote(const Address& addr)
{
    NS_LOG_FUNCTION(this << addr);
    if (!addr.IsInvalid())
    {
        m_peer = addr;
        if (m_peerPort)
        {
            SetPort(*m_peerPort);
        }
    }
}

Address
ContextProvider::GetRemote() const
{
    return m_peer;
}

void
ContextProvider::SetPort(uint16_t port)
{
    NS_LOG_FUNCTION(this << port);
    if (m_peer.IsInvalid())
    {
        // save for later
        m_peerPort = port;
        return;
    }
    if (Ipv4Address::IsMatchingType(m_peer) || Ipv6Address::IsMatchingType(m_peer))
    {
        m_peer = addressUtils::ConvertToSocketAddress(m_peer, port);
    }
}

uint16_t
ContextProvider::GetPort() const
{
    if (m_peer.IsInvalid())
    {
        return m_peerPort.value_or(ContextProvider::DEFAULT_PORT);
    }
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        return InetSocketAddress::ConvertFrom(m_peer).GetPort();
    }
    else if (Inet6SocketAddress::IsMatchingType(m_peer))
    {
        return Inet6SocketAddress::ConvertFrom(m_peer).GetPort();
    }
    return ContextProvider::DEFAULT_PORT;
}

void
ContextProvider::StartApplication()
{
    NS_LOG_INFO("[S.O.Cli] Inicia cliente do objeto inteligente");
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        auto tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        NS_ABORT_MSG_IF(m_peer.IsInvalid(), "Remote address not properly set");
        if (!m_local.IsInvalid())
        {
            NS_ABORT_MSG_IF((Inet6SocketAddress::IsMatchingType(m_peer) &&
                             InetSocketAddress::IsMatchingType(m_local)) ||
                                (InetSocketAddress::IsMatchingType(m_peer) &&
                                 Inet6SocketAddress::IsMatchingType(m_local)),
                            "Incompatible peer and local address IP version");
            if (m_socket->Bind(m_local) == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
        }
        else
        {
            if (InetSocketAddress::IsMatchingType(m_peer))
            {
                if (m_socket->Bind() == -1)
                {
                    NS_FATAL_ERROR("Failed to bind socket");
                }
            }
            else if (Inet6SocketAddress::IsMatchingType(m_peer))
            {
                if (m_socket->Bind6() == -1)
                {
                    NS_FATAL_ERROR("Failed to bind socket");
                }
            }
            else
            {
                NS_ASSERT_MSG(false, "Incompatible address type: " << m_peer);
            }
        }
        m_socket->SetIpTos(m_tos); // Affects only IPv4 sockets.
        m_socket->Connect(m_peer);
        m_socket->SetRecvCallback(MakeCallback(&ContextProvider::HandleRead, this));
        m_socket->SetAllowBroadcast(true);

        // set the messages that the object will send
        SetDataMessage();

        coap_startup();

    }

    ScheduleTransmit(Seconds(0.));
}

void
ContextProvider::StopApplication()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Durante a simulação chegou " << m_recived_messages << " no objeto " << m_objectType);
    NS_LOG_INFO("Durante a simulação foram enviadas " << m_send_messages << " do objeto " << m_objectType);

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = nullptr;
    }

    Simulator::Cancel(m_sendEvent);
}

void
ContextProvider::ScheduleTransmit(Time dt)
{
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &ContextProvider::Send, this);
}

void
ContextProvider::Send()
{
    m_send_messages++;
    // NS_LOG_INFO("[S.O.Cli] Prepara para enviar dados");
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_sendEvent.IsExpired());

    // ns3
    Ptr<Packet> p;
    std::string data;
    TimestampTag timestampTag;
    Address localAddress;
    
    // coap
    coap_pdu_code_t request_code;
    const char *uri_path;
    encoded_data data_pdu;

    // seleciona caso específico
    switch (m_objectId)
    {
    case 0:
        data = m_firstData.dump();
        data.erase(0, 1);
        data.erase(data.find_last_of("\""));
        data.erase(std::remove(data.begin(), data.end(), '\\'), data.end());
        uri_path = "/subscribe/object";
        request_code = COAP_REQUEST_CODE_POST;

        // NS_LOG_INFO("[S.O.Cli] Selecionou dados de inscrição de objeto");
        break;
    
    default:
        data = RandomData();
        uri_path = "/update/object";
        request_code = COAP_REQUEST_CODE_PUT;

        // NS_LOG_INFO("[S.O.Cli] Selecionou dados de atualização de objeto");
    }

    // configura mensagens a serem trafegadas
    data_pdu = EncodePduRequest(uri_path, request_code, data);
    
    // cria o pacote
    p = Create<Packet>(data_pdu.buffer, data_pdu.size);

    timestampTag.SetTimestamp(Simulator::Now());
    p->AddPacketTag(timestampTag);

    m_socket->GetSockName(localAddress);
    
    // envia o pacote
    m_txTrace(p);
    m_txTraceWithAddresses(p, localAddress, m_peer);
    m_socket->Send(p);
    // NS_LOG_INFO("[S.O.Cli] Enviou dados do objeto inteligente cliente para cotas");
    ++m_sent;

    if (m_sent < m_count || m_count == 0)
    {
        ScheduleTransmit(m_interval);
    }
}

void
ContextProvider::HandleRead(Ptr<Socket> socket)
{
    // NS_LOG_INFO("[S.O.Cli] Chegou resposta no objeto inteligente cliente");
    NS_LOG_FUNCTION(this << socket);
    Address from;
    m_recived_messages++;

    while (auto packet = socket->RecvFrom(from))
    {
        uint8_t *raw_data = new uint8_t[packet->GetSize()];
        coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_CODE_GET, 0, BUFSIZE);
        u_int8_t check;
        coap_pdu_code_t pdu_code;

        TimestampTag timestampTag;
        Address localAddress;
        
        nlohmann::json data_json;

        packet->CopyData(raw_data, packet->GetSize());
        
        check = coap_pdu_parse(COAP_PROTO_UDP, raw_data, packet->GetSize(), pdu);
        if (!check){
            NS_LOG_INFO("[S.O.Cli] Falha ao decodificar a pdu" <<  check);
            delete[] raw_data;
            abort();
        }

        pdu_code = coap_pdu_get_code(pdu);
        
        data_json = GetPduPayloadJson(pdu);

        switch (pdu_code)
        {
        case COAP_RESPONSE_CODE_CREATED:
            m_objectId = data_json["id"];
            break;
        case COAP_RESPONSE_CODE_CHANGED:
            // resposta do update, não faz nada
            NS_LOG_INFO("[S.O.Cli] Atualizou dados com sucesso!");
            break;
        case COAP_RESPONSE_CODE_UNAUTHORIZED:
            NS_LOG_INFO("[S.O.Cli] Tentou enviar update sem inscrição");
            break;
        case COAP_RESPONSE_CODE_INTERNAL_ERROR:
            NS_LOG_INFO("[S.O.Cli] Erro no servidor");
            break;
        case COAP_RESPONSE_CODE_CONTENT:
            break;
        default:
            NS_LOG_INFO("[S.O.Cli] status não reconhecido");
        }



        if (packet->PeekPacketTag(timestampTag))
        {
            Time txTime = timestampTag.GetTimestamp();
            
            Time now = Simulator::Now();
            
            Time delay = now - txTime;

            NS_LOG_INFO ("[S.O.Cli] RTT: " << delay.GetSeconds() << " segundos.");
        
        }


        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        delete[] raw_data;
    }
}

void
ContextProvider::SetDataMessage()
{   
    // NS_LOG_INFO("[S.O.Cli] tenta setar mensagem");
    m_firstData = m_messages["subscribeMessagesObjects"][m_objectType];
    // NS_LOG_INFO("[S.O.Cli] seta mensagem" << m_firstData.dump());
    m_updateData = m_messages["updateMessages"][m_objectType];
    
}

std::string
ContextProvider::RandomData()
{   
    nlohmann::json data = m_updateData[RandomInt(0, 3)];
    data["objectId"] = m_objectId;
    // add dados que variam com ns3 (ex: localização)
    return data.dump();
}

int 
ContextProvider::RandomInt(int min, int max) 
{
    static std::random_device rd; 
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}


} // Namespace ns3
