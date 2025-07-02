/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "cod-service.h"

#include "sqlite3.h"

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

#include <fstream>
#include <iostream>
#include <sstream>
#include <random>

#define SQL_PATH "all_data/cot.sql"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CoDServiceApplication");

NS_OBJECT_ENSURE_REGISTERED(CoDService);

TypeId
CoDService::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CoDService")
            .SetParent<SinkApplication>()
            .SetGroupName("Applications")
            .AddConstructor<CoDService>()
            .AddAttribute("Tos",
                          "The Type of Service used to send IPv4 packets. "
                          "All 8 bits of the TOS byte are set (including ECN bits).",
                          UintegerValue(0),
                          MakeUintegerAccessor(&CoDService::m_tos),
                          MakeUintegerChecker<uint8_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&CoDService::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&CoDService::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

CoDService::CoDService()
    : SinkApplication(DEFAULT_PORT),
      m_socket{nullptr},
      m_socket6{nullptr}
{
    NS_LOG_FUNCTION(this);
}

CoDService::~CoDService()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_socket6 = nullptr;
}

void
CoDService::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // inicia o banco
    if (sqlite3_open("all_data/cot.db", &m_banco))
    {
        NS_LOG_INFO("Não conectou no banco");
        abort();
    }
    else
    {
        NS_LOG_INFO("Conectou ao banco");
        SetupDatabase();
    }
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
        m_socket->SetRecvCallback(MakeCallback(&CoDService::HandleRead, this));
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
        m_socket6->SetRecvCallback(MakeCallback(&CoDService::HandleRead, this));
    }
}

void
CoDService::StopApplication()
{
    NS_LOG_FUNCTION(this);

    sqlite3_close(m_banco);

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
CoDService::HandleRead(Ptr<Socket> socket)
{
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
            packet->CopyData(raw_data, packet->GetSize());
            nlohmann::json data_json =
                nlohmann::json::parse(raw_data, raw_data + packet->GetSize());

            std::string req = data_json["req"].get<std::string>();

            // TODO: transformar num dicionário de funções
            if (req.compare("subscribe") == 0)
            {
                HandleSubscription(socket, from, data_json);
                NS_LOG_INFO("IP " << InetSocketAddress::ConvertFrom(from).GetIpv4() 
                                  << " se inscreveu");
            }
            else if (req.compare("update") == 0)
            {
                HandleUpdate(socket, from, data_json);
                NS_LOG_INFO("IP " << InetSocketAddress::ConvertFrom(from).GetIpv4() 
                                  << " atualizou dados");
            }
            else if (req.compare("getContext") == 0)
            {
                NS_LOG_INFO("Chegou requisição da aplicação");
            }
        }
        // trata no ipv6
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " server received "
                                   << packet->GetSize() << " bytes from "
                                   << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                                   << Inet6SocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

void
CoDService::HandleSubscription(ns3::Ptr<ns3::Socket> socket, Address from, nlohmann::json data_json)
{   

    // verifica se já foi feita inscrição pelo endereço de ip
    int valida = ValidateIP_Q(from);
    if(valida){
        // ip já inscrito, manda o id novamente.
        NS_LOG_INFO("IP já inscrito");
        NS_LOG_INFO("Id do ip inscrito: " << valida);
        nlohmann::json res = {
            { "status", 200 },
            { "id", valida }
        };

        std::string data = res.dump();
        Ptr<Packet> response = Create<Packet>((uint8_t*)data.c_str(), data.size());

        socket->SendTo(response, 0, from);
        return ;
    }

    // gera id seguro (vamos abstrair segurança)
    int precisa_gerar = 1;
    int id;
    while(precisa_gerar){
        id = RandomInt(20000, 20000000);
        // validar id
        if(!ValidateID_Q(id)){
            precisa_gerar = 0;
        }
    }

    // insere dados json
    InsertDataSub_Q(id, from, data_json);

    // retorna status ok com id ou error sem id
    nlohmann::json res = {
        { "status", 200 },
        { "id", id }
    };

    std::string data = res.dump();
    Ptr<Packet> response = Create<Packet>((uint8_t*)data.c_str(), data.size());
    socket->SendTo(response, 0, from);
}

void
CoDService::HandleUpdate(ns3::Ptr<ns3::Socket> socket, Address from, nlohmann::json data_json)
{
    // verifica se id é válido
    int id =  data_json["id"];
    if (!ValidateID_Q(id)){
        // id inválido
        nlohmann::json res = {
            { "status", 401 },
        };
        std::string data = res.dump();
        Ptr<Packet> response = Create<Packet>((uint8_t*)data.c_str(), data.size());

        socket->SendTo(response, 0, from);
        return ;
    }

    // pega dados no banco
    nlohmann::json db_json = GetDataJSON_Q(id);
    
    // atualiza os campos
    UpdtData_JSON(db_json, data_json);
    
    // volta pro banco
    UpdtData_Q(id, db_json);

    // retorna status ok ou error
    nlohmann::json res = {
        { "status", 200 },
        { "id", id }
    };

    std::string data = res.dump();
    Ptr<Packet> response = Create<Packet>((uint8_t*)data.c_str(), data.size());
    socket->SendTo(response, 0, from);
    
    return ;
}

void
CoDService::HandleRequest(ns3::Ptr<ns3::Socket> socket, Address from, nlohmann::json data_json)
{
    // verifica se id é válido
    // atualiza dados no banco
    // retorna status ok ou error
}

std::string
CoDService::ReadSqlArc()
{
    std::ifstream arquivo(SQL_PATH);
    if (!arquivo.is_open())
    {
        NS_LOG_INFO("Erro ao abrir o arquivo SQL: " << SQL_PATH);
        return "";
    }
    std::stringstream buffer;
    buffer << arquivo.rdbuf();
    return buffer.str();
}

void
CoDService::SetupDatabase()
{
    char* zErrMsg = 0;
    int rc;

    std::string sqlScript = ReadSqlArc();

    if (!sqlScript.empty())
    {
        // sqlite3_exec é usada para executar comandos SQL que não retornam dados
        // (ou quando não nos importamos com o retorno, como aqui)
        rc = sqlite3_exec(m_banco, sqlScript.c_str(), 0, 0, &zErrMsg);

        if (rc != SQLITE_OK)
        {
            std::cerr << "Erro no SQL: " << zErrMsg << std::endl;
            sqlite3_free(zErrMsg);
        }
        else
        {
            std::cout << "Script executado com sucesso!" << std::endl;
        }
    }
}

int
CoDService::Simple_Q()
{
    int rc;

    const char* sqlSelect = "SELECT * FROM objeto;";
    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(m_banco, sqlSelect, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        NS_LOG_INFO("Erro ao preparar a consulta: " << sqlite3_errmsg(m_banco));
        sqlite3_close(m_banco);
        return 1;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* raw_data = sqlite3_column_text(stmt, 1);

        std::string data = reinterpret_cast<const char*>(raw_data);

        NS_LOG_INFO("ID: " << id << "Data: " << data);
    }

    if (rc != SQLITE_DONE)
    {
        NS_LOG_INFO("Erro ao executar o passo: " << sqlite3_errmsg(m_banco));
    }

    sqlite3_finalize(stmt);
    return 0;
}

// se não existe ip com isso no banco retorna 0
// se existe retorna id
int
CoDService::ValidateIP_Q(Address ip)
{   
    // TODO: RESOLVER ESSE BO DO IP
    int rc;
    uint32_t ip_num = InetSocketAddress::ConvertFrom(ip).GetIpv4().Get();
    const char* sql = "SELECT id FROM objeto WHERE ip == ?;";
    sqlite3_stmt* stmt;
    int existe = 0;
    rc = sqlite3_prepare_v2(m_banco, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        NS_LOG_INFO("Erro ao preparar a consulta: " << sqlite3_errmsg(m_banco));
        sqlite3_close(m_banco);
        abort();
    }

    rc = sqlite3_bind_int(stmt, 1, ip_num);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        existe = sqlite3_column_int(stmt, 0);
    }
    if (rc != SQLITE_DONE)
    {
        NS_LOG_INFO("Erro ao executar o passo: " << sqlite3_errmsg(m_banco));
    }

    sqlite3_finalize(stmt);
    return existe;
}

// se o id já existe, ele retorna o próprio id
// se não, retorna 0
int
CoDService::ValidateID_Q(int id)
{
    int rc;

    const char* sql = "SELECT id FROM objeto WHERE id == ?;";
    sqlite3_stmt* stmt;
    int existe = 0;
    rc = sqlite3_prepare_v2(m_banco, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        NS_LOG_INFO("Erro ao preparar a consulta: " << sqlite3_errmsg(m_banco));
        sqlite3_close(m_banco);
        abort();
    }

    rc = sqlite3_bind_int(stmt, 1, id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        existe = sqlite3_column_int(stmt, 0);
    }
    if (rc != SQLITE_DONE)
    {
        NS_LOG_INFO("Erro ao executar o passo: " << sqlite3_errmsg(m_banco));
    }

    sqlite3_finalize(stmt);
    return existe;
}

int
CoDService::InsertDataSub_Q(int id, Address ip, nlohmann::json data_json)
{
    int rc;
    const char* sql = "INSERT INTO objeto (id, ip, dados) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    uint32_t ip_num = InetSocketAddress::ConvertFrom(ip).GetIpv4().Get();
    std::string data_s = data_json.dump();

    rc = sqlite3_prepare_v2(m_banco, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        NS_LOG_INFO("Erro ao preparar a consulta: " << sqlite3_errmsg(m_banco));
        sqlite3_close(m_banco);
        abort();
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, ip_num);
    sqlite3_bind_text(stmt, 3, data_s.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        NS_LOG_INFO("Erro ao executar o passo: " << sqlite3_errmsg(m_banco));
        abort();
    }
    // NS_LOG_INFO("Inseriu dados de " 
    //     << InetSocketAddress::ConvertFrom(ip).GetIpv4());
    sqlite3_finalize(stmt);
    return 1;
}

nlohmann::json
CoDService::GetDataJSON_Q(int id)
{
    int rc;

    const char* sql = "SELECT dados FROM objeto WHERE id == ?;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(m_banco, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        NS_LOG_INFO("Erro ao preparar a consulta: " << sqlite3_errmsg(m_banco));
        sqlite3_close(m_banco);
        abort();
    }

    rc = sqlite3_bind_int(stmt, 1, id);
    nlohmann::json data_json;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const unsigned char* raw_data = sqlite3_column_text(stmt, 0);
        std::string data = reinterpret_cast<const char*>(raw_data);

        data_json = nlohmann::json::parse(data);
    }
    if (rc != SQLITE_DONE)
    {
        NS_LOG_INFO("Erro ao executar o passo: " << sqlite3_errmsg(m_banco));
    }

    sqlite3_finalize(stmt);
    return data_json;
}

int
CoDService::RandomInt(int min, int max) 
{
    static std::random_device rd; 
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}

void
CoDService::UpdtData_JSON(nlohmann::json& db, nlohmann::json new_data) 
{
    // estamos usando o conceito de JSON Pointer
    new_data.erase("req");
    new_data.erase("id");
    for(auto& context : new_data.items()){
        try {
            nlohmann::json::json_pointer ptr("/state/"+context.key());
            db[ptr] = context.value();
        }catch (const nlohmann::json::exception& e){
            NS_LOG_INFO("Erro ao tentar alterar o valor: " << e.what());
            abort();
        }
    }
}

void
CoDService::UpdtData_Q(int id, nlohmann::json db) 
{
    int rc;
    const char* sql = "UPDATE objeto SET dados=? WHERE id=?";
    sqlite3_stmt* stmt;
    std::string data_s = db.dump();

    rc = sqlite3_prepare_v2(m_banco, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        NS_LOG_INFO("Erro ao preparar a consulta: " << sqlite3_errmsg(m_banco));
        sqlite3_close(m_banco);
        abort();
    }

    sqlite3_bind_text(stmt, 1, data_s.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        NS_LOG_INFO("Erro ao executar o passo: " << sqlite3_errmsg(m_banco));
        abort();
    }
    // NS_LOG_INFO("Inseriu dados de " 
    //     << InetSocketAddress::ConvertFrom(ip).GetIpv4());
    sqlite3_finalize(stmt);
    return ;
}


} // Namespace ns3
