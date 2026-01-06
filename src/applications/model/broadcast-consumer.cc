/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "broadcast-consumer.h"

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

NS_LOG_COMPONENT_DEFINE("BroadcastConsumerApplication");

NS_OBJECT_ENSURE_REGISTERED(BroadcastConsumer);

TypeId
BroadcastConsumer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::BroadcastConsumer")
            .SetParent<SourceApplication>()
            .SetGroupName("Applications")
            .AddConstructor<BroadcastConsumer>()
            .AddAttribute(
                "MaxPackets",
                "The maximum number of packets the application will send (zero means infinite)",
                UintegerValue(100),
                MakeUintegerAccessor(&BroadcastConsumer::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&BroadcastConsumer::m_interval),
                          MakeTimeChecker())
            .AddAttribute(
                "RemoteAddress",
                "The destination Address of the outbound packets",
                AddressValue(),
                MakeAddressAccessor(
                    (void (BroadcastConsumer::*)(
                        const Address&))&BroadcastConsumer::SetRemote, // this is needed to indicate
                                                                     // which version of the
                                                                     // function overload to use
                    &BroadcastConsumer::GetRemote),
                MakeAddressChecker(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            .AddAttribute(
                "RemotePort",
                "The destination port of the outbound packets",
                UintegerValue(BroadcastConsumer::DEFAULT_PORT),
                MakeUintegerAccessor(&BroadcastConsumer::SetPort, &BroadcastConsumer::GetPort),
                MakeUintegerChecker<uint16_t>(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            .AddAttribute("ApplicationType",
                          "Which Application the node refers to",
                          UintegerValue(1),
                          MakeUintegerAccessor(&BroadcastConsumer::m_applicationType),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Tx",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&BroadcastConsumer::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&BroadcastConsumer::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("TxWithAddresses",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&BroadcastConsumer::m_txTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&BroadcastConsumer::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

BroadcastConsumer::BroadcastConsumer()
    : m_sent{0},
      m_socket{nullptr},
      m_peerPort{},
      m_sendEvent{},
      m_state{Searching},
      m_objectAdress{},
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
        NS_LOG_INFO("[App.Cli] Erro ao abrir messages.json");
    }

    NS_LOG_FUNCTION(this);
}

BroadcastConsumer::~BroadcastConsumer()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    
    coap_cleanup();

}

void
BroadcastConsumer::SetRemote(const Address& ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    SetRemote(ip);
    SetPort(port);
}

void
BroadcastConsumer::SetRemote(const Address& addr)
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
BroadcastConsumer::GetRemote() const
{
    return m_peer;
}

void
BroadcastConsumer::SetPort(uint16_t port)
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
BroadcastConsumer::GetPort() const
{
    if (m_peer.IsInvalid())
    {
        return m_peerPort.value_or(BroadcastConsumer::DEFAULT_PORT);
    }
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        return InetSocketAddress::ConvertFrom(m_peer).GetPort();
    }
    else if (Inet6SocketAddress::IsMatchingType(m_peer))
    {
        return Inet6SocketAddress::ConvertFrom(m_peer).GetPort();
    }
    return BroadcastConsumer::DEFAULT_PORT;
}

void
BroadcastConsumer::StartApplication()
{
    NS_LOG_INFO("[App.Cli] Inicia Aplicação tipo: " << m_applicationType);
    NS_LOG_FUNCTION(this);

    // set the messages that the object will send
    SetDataMessage();

    coap_startup();

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
        m_socket->SetRecvCallback(MakeCallback(&BroadcastConsumer::HandleRead, this));
        m_socket->SetAllowBroadcast(true);
    }

    ScheduleTransmit(Seconds(0.));
}

void
BroadcastConsumer::StopApplication()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Durante a simulação chegou " << m_recived_messages << " na aplicação " << m_applicationType);
    NS_LOG_INFO("Durante a simulação foram enviadas " << m_send_messages << " da aplicação " << m_applicationType);

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = nullptr;
    }

    Simulator::Cancel(m_sendEvent);
}

void
BroadcastConsumer::ScheduleTransmit(Time dt)
{
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &BroadcastConsumer::Send, this);
}

void
BroadcastConsumer::Send()
{
    // NS_LOG_INFO("[App.Cli] Prepara para enviar dados");

    m_send_messages++;

    //[App.Cli]
    // ns3
    Ptr<Packet> p;
    std::string data;
    TimestampTag timestampTag;
    Address localAddress;
 
    // coap
    coap_pdu_code_t request_code;
    const char *uri_path;
    encoded_data data_pdu;

    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    data = m_reqData.dump();
    uri_path = "/search";
    request_code = COAP_REQUEST_CODE_GET;

    data_pdu = EncodePduRequest(uri_path, request_code, data);
    
    p = Create<Packet>(data_pdu.buffer, data_pdu.size);
    
    timestampTag.SetTimestamp(Simulator::Now());
    p->AddPacketTag(timestampTag);

    // NS_LOG_INFO("[App.Cli] Enviando dados de requisição");
    
    m_socket->GetSockName(localAddress);
    
    m_txTrace(p);
    
    // escolhe pra onde vai mandar 
    // TODO: talvez usar um padrão de projeto chamado state
    // https://refactoring.guru/pt-br/design-patterns/state
    // usar apenas se aumentar muito os estados

    switch (m_state)
    {
    case Searching:
        m_txTraceWithAddresses(p, localAddress, m_peer);
        m_socket->Send(p);
        break;
    case Find:
        m_txTraceWithAddresses(p, localAddress, m_objectAdress);
        m_socket->SendTo(p, 0, m_objectAdress);
        break;
    default:
        NS_LOG_INFO("[App.Cli] Consumidor em estado infefinido");
        break;
    }
    
    ++m_sent;

    if (m_sent < m_count || m_count == 0)
    {
        ScheduleTransmit(m_interval);
    }
}

void
BroadcastConsumer::HandleRead(Ptr<Socket> socket)
{
    // NS_LOG_INFO("[App.Cli] Chegou mensagem no consumidor de contexto");
    NS_LOG_FUNCTION(this << socket);
    Address from;
    m_recived_messages++;

    while (auto packet = socket->RecvFrom(from))
    {
        // recebe dados e faz parse
        uint8_t *raw_data = new uint8_t[packet->GetSize()];
        coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_CODE_GET, 0, BUFSIZE);
        u_int8_t check;
        coap_pdu_code_t pdu_code;

        Address localAddress;
        TimestampTag timestampTag;

        nlohmann::json data_json;

        packet->CopyData(raw_data, packet->GetSize());
        
        check = coap_pdu_parse(COAP_PROTO_UDP, raw_data, packet->GetSize(), pdu);
        if (!check){
            NS_LOG_INFO("[App.Cli] Falha ao decodificar a pdu" <<  check);
            delete[] raw_data;
            abort();
        }

        // pdu_code = coap_pdu_get_code(pdu);
        
        data_json = GetPduPayloadJson(pdu);

        // NS_LOG_INFO("[App.Cli] json que chegou no cliente aplicação " << data_json.dump());

       HandleOK(data_json["response"], from);

        if (packet->PeekPacketTag(timestampTag))
        {
            Time txTime = timestampTag.GetTimestamp();
            
            Time now = Simulator::Now();
            
            Time delay = now - txTime;

            NS_LOG_INFO ("[App.Cli] RTT: " << delay.GetSeconds() << " segundos.");
        
        }
        
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        delete[] raw_data;
    }
}

void
BroadcastConsumer::SetDataMessage()
{    
    m_reqData = m_messages["broadcast_messages"][m_applicationType]; 
    // TODO: enviar mensagens corretas e fazer o objeto sink
}

void 
BroadcastConsumer::HandleOK(nlohmann::json response, Address from)
{
    switch (m_state)
    {
        case Searching:
            if(response.empty()){
                // NS_LOG_INFO("[App.Cli] Chegou resposta do cotas vazio,"
                //             << "não há objetos que correspondem a pesquisa");
            }
            else{ // não chegou vazio, existe objeto que corresponde a pesquisa
                // se comunica com o objeto em si (abstraido pra pedir só para o
                // primeiro objeto)

                m_objectAdress = from;
                
                // atualiza o estado da aplicação
                m_state = Find;
                // NS_LOG_INFO("[App.Cli] Chegou resposta do cotas com objeto");
            }
            break;
        case Find:
            // NS_LOG_INFO("[App.Cli] Chegou resposta do objeto inteligente no consumidor");
            // data_json["response"]
            break;
        default:
            break;
    }
}

} // Namespace ns3
