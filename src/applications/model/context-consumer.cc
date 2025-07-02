/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "context-consumer.h"

#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include <random>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ContextConsumerApplication");

NS_OBJECT_ENSURE_REGISTERED(ContextConsumer);

TypeId
ContextConsumer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ContextConsumer")
            .SetParent<SourceApplication>()
            .SetGroupName("Applications")
            .AddConstructor<ContextConsumer>()
            .AddAttribute(
                "MaxPackets",
                "The maximum number of packets the application will send (zero means infinite)",
                UintegerValue(100),
                MakeUintegerAccessor(&ContextConsumer::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&ContextConsumer::m_interval),
                          MakeTimeChecker())
            .AddAttribute(
                "RemoteAddress",
                "The destination Address of the outbound packets",
                AddressValue(),
                MakeAddressAccessor(
                    (void (ContextConsumer::*)(
                        const Address&))&ContextConsumer::SetRemote, // this is needed to indicate
                                                                     // which version of the
                                                                     // function overload to use
                    &ContextConsumer::GetRemote),
                MakeAddressChecker(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            .AddAttribute(
                "RemotePort",
                "The destination port of the outbound packets",
                UintegerValue(ContextConsumer::DEFAULT_PORT),
                MakeUintegerAccessor(&ContextConsumer::SetPort, &ContextConsumer::GetPort),
                MakeUintegerChecker<uint16_t>(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            .AddAttribute(
                "PacketSize",
                "Size of echo data in outbound packets",
                UintegerValue(100),
                MakeUintegerAccessor(&ContextConsumer::SetDataSize, &ContextConsumer::GetDataSize),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("ApplicationType",
                          "Which Application the node refers to",
                          UintegerValue(1),
                          MakeUintegerAccessor(&ContextConsumer::m_applicationType),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Tx",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&ContextConsumer::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&ContextConsumer::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("TxWithAddresses",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&ContextConsumer::m_txTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&ContextConsumer::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

ContextConsumer::ContextConsumer()
    : m_dataSize{0},
      m_data{nullptr},
      m_sent{0},
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

ContextConsumer::~ContextConsumer()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;

    delete[] m_data;
    m_data = nullptr;
    m_dataSize = 0;
}

void
ContextConsumer::SetRemote(const Address& ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    SetRemote(ip);
    SetPort(port);
}

void
ContextConsumer::SetRemote(const Address& addr)
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
ContextConsumer::GetRemote() const
{
    return m_peer;
}

void
ContextConsumer::SetPort(uint16_t port)
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
ContextConsumer::GetPort() const
{
    if (m_peer.IsInvalid())
    {
        return m_peerPort.value_or(ContextConsumer::DEFAULT_PORT);
    }
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        return InetSocketAddress::ConvertFrom(m_peer).GetPort();
    }
    else if (Inet6SocketAddress::IsMatchingType(m_peer))
    {
        return Inet6SocketAddress::ConvertFrom(m_peer).GetPort();
    }
    return ContextConsumer::DEFAULT_PORT;
}

void
ContextConsumer::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // set the messages that the object will send
    SetDataMessage();

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
        m_socket->SetRecvCallback(MakeCallback(&ContextConsumer::HandleRead, this));
        m_socket->SetAllowBroadcast(true);
    }

    ScheduleTransmit(Seconds(0.));
}

void
ContextConsumer::StopApplication()
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
ContextConsumer::SetDataSize(uint32_t dataSize)
{
    NS_LOG_FUNCTION(this << dataSize);

    //
    // If the client is setting the echo packet data size this way, we infer
    // that she doesn't care about the contents of the packet at all, so
    // neither will we.
    //
    delete[] m_data;
    m_data = nullptr;
    m_dataSize = 0;
    m_size = dataSize;
}

uint32_t
ContextConsumer::GetDataSize() const
{
    NS_LOG_FUNCTION(this);
    return m_size;
}

void
ContextConsumer::SetFill(std::string fill)
{
    NS_LOG_FUNCTION(this << fill);

    uint32_t dataSize = fill.size() + 1;

    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    memcpy(m_data, fill.c_str(), dataSize);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
ContextConsumer::SetFill(uint8_t fill, uint32_t dataSize)
{
    NS_LOG_FUNCTION(this << fill << dataSize);
    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    memset(m_data, fill, dataSize);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
ContextConsumer::SetFill(uint8_t* fill, uint32_t fillSize, uint32_t dataSize)
{
    NS_LOG_FUNCTION(this << fill << fillSize << dataSize);
    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    if (fillSize >= dataSize)
    {
        memcpy(m_data, fill, dataSize);
        m_size = dataSize;
        return;
    }

    //
    // Do all but the final fill.
    //
    uint32_t filled = 0;
    while (filled + fillSize < dataSize)
    {
        memcpy(&m_data[filled], fill, fillSize);
        filled += fillSize;
    }

    //
    // Last fill may be partial
    //
    memcpy(&m_data[filled], fill, dataSize - filled);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
ContextConsumer::ScheduleTransmit(Time dt)
{
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &ContextConsumer::Send, this);
}

void
ContextConsumer::Send()
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_sendEvent.IsExpired());

    Ptr<Packet> p;

    std::string data;

    data = m_reqData.dump();
    NS_LOG_INFO("Enviando dados de requisição consumidor");
    

    p = Create<Packet>((uint8_t*)data.c_str(), data.size());
    m_size = data.size();

    Address localAddress;
    m_socket->GetSockName(localAddress);
    
    // call to the trace sinks before the packet is actually sent,
    // so that tags added to the packet can be sent as well
    
    m_txTrace(p);
    m_txTraceWithAddresses(p, localAddress, m_peer);
    m_socket->Send(p);
    ++m_sent;

    // if (InetSocketAddress::IsMatchingType(m_peer))
    // {
    //     NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size
    //                            << " bytes to " <<
    //                            InetSocketAddress::ConvertFrom(m_peer).GetIpv4()
    //                            << " port " << InetSocketAddress::ConvertFrom(m_peer).GetPort());
    // }
    // else if (Inet6SocketAddress::IsMatchingType(m_peer))
    // {
    //     NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client sent " << m_size
    //                            << " bytes to " <<
    //                            Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6()
    //                            << " port " << Inet6SocketAddress::ConvertFrom(m_peer).GetPort());
    // }

    if (m_sent < m_count || m_count == 0)
    {
        ScheduleTransmit(m_interval);
    }
}

void
ContextConsumer::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_INFO("lidando com dados consumidor");
    NS_LOG_FUNCTION(this << socket);
    Address from;
    while (auto packet = socket->RecvFrom(from))
    {
        uint8_t *raw_data = new uint8_t[packet->GetSize()];
        packet->CopyData(raw_data, packet->GetSize());
        nlohmann::json data_json =
            nlohmann::json::parse(raw_data, raw_data + packet->GetSize());

        uint64_t res = data_json["status"];
        
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


        if (InetSocketAddress::IsMatchingType(from))
        {
            // NS_LOG_INFO("Aceitou inscrição");
            // NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client received "
            //                        << packet->GetSize() << " bytes from "
            //                        << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
            //                        << InetSocketAddress::ConvertFrom(from).GetPort());
        }
        // else if (Inet6SocketAddress::IsMatchingType(from))
        // {
        //     NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " client received "
        //                            << packet->GetSize() << " bytes from "
        //                            << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
        //                            << Inet6SocketAddress::ConvertFrom(from).GetPort());
        // }
        Address localAddress;
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
    }
}

void
ContextConsumer::SetDataMessage()
{   
    m_reqData = m_messages["requestMessages"][m_applicationType];
}

std::string
ContextConsumer::RandomData()
{   
    nlohmann::json data = m_reqData[RandomInt(0, 3)];
    data["id"] = m_objectId;
    // add dados que variam com ns3 (localização)
    return data.dump();
}

int 
ContextConsumer::RandomInt(int min, int max) 
{
    static std::random_device rd; 
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}
} // Namespace ns3
