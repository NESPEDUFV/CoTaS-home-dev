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
      m_objectId{0}
{
    std::ifstream fm("all_data/messages2.json");

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

    }

    ScheduleTransmit(Seconds(0.));
}

void
ContextProvider::StopApplication()
{
    NS_LOG_FUNCTION(this);

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
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_sendEvent.IsExpired());

    Ptr<Packet> p;

    std::string data;
    if (m_objectId == 0)
    {
        data = m_firstData.dump();

        NS_LOG_INFO("Enviando dados de inscrição");
    }
    else
    {
        data = RandomData();
        NS_LOG_INFO("Enviando dados de atualização");
    }

    p = Create<Packet>((uint8_t*)data.c_str(), data.size());

    TimestampTag timestampTag;
    timestampTag.SetTimestamp(Simulator::Now());
    p->AddPacketTag(timestampTag);

    Address localAddress;
    m_socket->GetSockName(localAddress);
    
    // call to the trace sinks before the packet is actually sent,
    // so that tags added to the packet can be sent as well
    
    m_txTrace(p);
    m_txTraceWithAddresses(p, localAddress, m_peer);
    m_socket->Send(p);
    ++m_sent;

    if (m_sent < m_count || m_count == 0)
    {
        ScheduleTransmit(m_interval);
    }
}

void
ContextProvider::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Address from;
    while (auto packet = socket->RecvFrom(from))
    {
        uint8_t *raw_data = new uint8_t[packet->GetSize()];
        packet->CopyData(raw_data, packet->GetSize());
        nlohmann::json data_json =
            nlohmann::json::parse(raw_data, raw_data + packet->GetSize());

        uint64_t res = data_json["status"];
        NS_LOG_INFO("Chegou resposta do servidor no provedor");
        switch (res)
        {
        case 200:
            m_objectId = data_json["id"];
            break;
        case 204:
            // resposta do update, não faz nada
        break;
        case 401:
            NS_LOG_INFO("Tentou enviar update sem inscrição");
            break;
        case 500:
            NS_LOG_INFO("Erro no servidor");
            break;
        default:
            NS_LOG_INFO("status não reconhecido");
        }

        TimestampTag timestampTag;

        if (packet->PeekPacketTag(timestampTag))
        {
            Time txTime = timestampTag.GetTimestamp();
            
            Time now = Simulator::Now();
            
            Time delay = now - txTime;

            NS_LOG_INFO ("RTT: " << delay.GetSeconds() << " segundos.");
        
        }


        Address localAddress;
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
    }
}

void
ContextProvider::SetDataMessage()
{   

    m_firstData = m_messages["firstMessages"][m_objectType];
    m_firstData["port"] = 19;
    m_updateData = m_messages["updateMessages"][m_objectType];
    
}

std::string
ContextProvider::RandomData()
{   
    nlohmann::json data = m_updateData[RandomInt(0, 3)];
    data["id"] = m_objectId;
    // add dados que variam com ns3 (localização)
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
