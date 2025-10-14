/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "generic-server.h"

#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"
#include "ns3/timestamp-tag.h"

#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GenericServerApplication");

NS_OBJECT_ENSURE_REGISTERED(GenericServer);

TypeId
GenericServer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GenericServer")
            .SetParent<SinkApplication>()
            .SetGroupName("Applications")
            .AddConstructor<GenericServer>()
            .AddAttribute("Tos",
                          "The Type of Service used to send IPv4 packets. "
                          "All 8 bits of the TOS byte are set (including ECN bits).",
                          UintegerValue(0),
                          MakeUintegerAccessor(&GenericServer::m_tos),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("ObjectType",
                          "Which object the node refers to",
                          UintegerValue(1),
                          MakeUintegerAccessor(&GenericServer::m_objectType),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&GenericServer::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&GenericServer::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

GenericServer::GenericServer()
    : SinkApplication(DEFAULT_PORT),
      m_socket{nullptr},
      m_socket6{nullptr}
{
    NS_LOG_FUNCTION(this);
}

GenericServer::~GenericServer()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_socket6 = nullptr;

    coap_cleanup();

}

void
GenericServer::StartApplication()
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("[S.O.Srv] Inicia aplicação de tipo: " << m_objectType);

    SetDataMessage();

    if (!m_socket)
    {
        auto tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        auto local = m_local;
        if (local.IsInvalid())
        {
            local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        }
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(m_local))
        {
            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket);
            if (udpSocket)
            {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, m_local);
            }
            else
            {
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
        m_socket->SetIpTos(m_tos); // Affects only IPv4 sockets.
        m_socket->SetRecvCallback(MakeCallback(&GenericServer::HandleRead, this));
    }

    if (m_local.IsInvalid() && !m_socket6)
    {
        // local address is not specified, so create another socket to also listen to all IPv6
        // addresses
        auto tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket6 = Socket::CreateSocket(GetNode(), tid);
        auto local = Inet6SocketAddress(Ipv6Address::GetAny(), m_port);
        if (m_socket6->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        if (addressUtils::IsMulticast(local))
        {
            Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket6);
            if (udpSocket)
            {
                // equivalent to setsockopt (MCAST_JOIN_GROUP)
                udpSocket->MulticastJoinGroup(0, local);
            }
            else
            {
                NS_FATAL_ERROR("Error: Failed to join multicast group");
            }
        }
        m_socket6->SetRecvCallback(MakeCallback(&GenericServer::HandleRead, this));
    }

    coap_startup();
}

void
GenericServer::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    if (m_socket6)
    {
        m_socket6->Close();
        m_socket6->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
GenericServer::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_INFO("[S.O.Srv] Chegou requisição no objeto inteligente " << m_objectType);

    NS_LOG_FUNCTION(this << socket);

    Address from;
    while (auto packet = socket->RecvFrom(from))
    {
        Address localAddress;
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);

        // trata no ipv4
        if (InetSocketAddress::IsMatchingType(from))
        {
            // raw_data são os dados que os objetos mandaram para o serviço
            uint8_t* raw_data = new uint8_t[packet->GetSize()];
            nlohmann::json data;
            std::string response_data;
            nlohmann::json res;
            TimestampTag timestampTag;
            Ptr<Packet> response;
            encoded_data data_pdu;

            //* o seguinte trecho de código desembrulha os dados de requisição
            //* mas não faz nada com eles até agora (abstraído)
            coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_CODE_GET, 0, BUFSIZE);
            u_int8_t check;
            std::string path;

            packet->CopyData(raw_data, packet->GetSize());
            
            check = coap_pdu_parse(COAP_PROTO_UDP, raw_data, packet->GetSize(), pdu);
            if (!check){
                NS_LOG_INFO("[S.O.Srv] Falha ao decodificar a pdu" <<  check);
                delete[] raw_data;
                abort();
            }

            data = GetPduPayloadString(pdu);
            //* acaba aqui

            response_data = RandomData();
            res = {{"status", COAP_RESPONSE_CODE_CONTENT}, {"response", response_data}};

            data_pdu = EncodePduResponse(res["status"], res.dump());


            response = Create<Packet>(data_pdu.buffer, data_pdu.size);
            
            if(packet->PeekPacketTag(timestampTag))
            {
                response->AddPacketTag(timestampTag);
            }
            
            NS_LOG_INFO("[S.O.Srv] Enviando resposta do objeto inteligente");
            socket->SendTo(response, 0, from);
            
            delete[] raw_data;
        }
        // trata no ipv6
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("[S.O.Srv] At time " << Simulator::Now().As(Time::S) << " server received "
                                   << packet->GetSize() << " bytes from "
                                   << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                                   << Inet6SocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

int
GenericServer::RandomInt(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}

std::string
GenericServer::RandomData()
{   
    nlohmann::json data = m_updateData[RandomInt(0, 3)];
    // add dados que variam com ns3 (localização, bateria...)
    return data.dump();
}
void 
GenericServer::SetDataMessage(){

    std::ifstream fm("all_data/messages2.json");

    if (fm.is_open())
    {
        fm >> m_updateData;
        m_updateData = m_updateData["updateMessages"][m_objectType];
    }
    else
    {
        std::cerr << "Erro ao abrir messages.json\n";
    }
}
} // Namespace ns3
