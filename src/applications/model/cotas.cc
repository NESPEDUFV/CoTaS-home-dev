/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "cotas.h"

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


using bsoncxx::builder::basic::kvp;

using bsoncxx::builder::basic::make_document;
namespace ns3
{
    
#define BUFSIZE 1500
NS_LOG_COMPONENT_DEFINE("CoTaSApplication");

NS_OBJECT_ENSURE_REGISTERED(CoTaS);

TypeId
CoTaS::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CoTaS")
            .SetParent<SinkApplication>()
            .SetGroupName("Applications")
            .AddConstructor<CoTaS>()
            .AddAttribute("Tos",
                          "The Type of Service used to send IPv4 packets. "
                          "All 8 bits of the TOS byte are set (including ECN bits).",
                          UintegerValue(0),
                          MakeUintegerAccessor(&CoTaS::m_tos),
                          MakeUintegerChecker<uint8_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&CoTaS::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&CoTaS::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

CoTaS::CoTaS()
    : SinkApplication(DEFAULT_PORT),
      m_socket{nullptr},
      m_socket6{nullptr},
      //   m_cli{"localhost", 3030}
      m_cli{"www.google.com"}
{
    NS_LOG_FUNCTION(this);
}

CoTaS::~CoTaS()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_socket6 = nullptr;
    m_cli.stop();
}

void
CoTaS::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // inicia a conexão com o banco
    try
    {
        // faz put dos dados iniciais
        if (auto res = m_cli.Get("/")) {
        NS_LOG_INFO("Foi" << res->status << "\n" 
                    << res->get_header_value("Content-Type") << "\n" 
                    << res->body);
        } else {
            NS_LOG_INFO("error code: " << res.error());
        }

        mongocxx::uri uri("mongodb://localhost:27017/");
        m_client.emplace(uri);
        std::string nome_banco = "cotas_db";

        SetupDatabase(*m_client, nome_banco);

        NS_LOG_INFO("Tentando pingar no servidor de dados:");
        m_bancoMongo.run_command(bsoncxx::from_json(R"({ "ping": 1 })"));
        NS_LOG_INFO("Banco de dados criado e conectado com sucesso.");
    }
    catch (const mongocxx::exception& e)
    {
        NS_LOG_ERROR("An exception occurred: " << e.what());
        return;
    }

    StartHandlerDict();

    // coisas do ns3
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
        m_socket->SetRecvCallback(MakeCallback(&CoTaS::HandleRead, this));
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
        m_socket6->SetRecvCallback(MakeCallback(&CoTaS::HandleRead, this));
    }
}

void
CoTaS::StopApplication()
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
CoTaS::HandleRead(Ptr<Socket> socket)
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
            coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_CODE_GET, 0, BUFSIZE);
            u_int8_t check;
            std::string path;
            nlohmann::json response_data;
            TimestampTag timestampTag;
            nlohmann::json data;
            encoded_data data_pdu;
            Ptr<Packet> response;

            packet->CopyData(raw_data, packet->GetSize());

            check = coap_pdu_parse(COAP_PROTO_UDP, raw_data, packet->GetSize(), pdu);
            if (!check){
                NS_LOG_INFO("Falha ao decodificar a pdu" <<  check);
                delete[] raw_data;
                abort();
            }
            
            path = GetPduPath(pdu);
            // NS_LOG_INFO("caminho que chegou no cotas:" << path);

            data = GetPduPayload(pdu);
            // NS_LOG_INFO("json resultante: " << data.dump());
            
            NS_LOG_INFO("Chegou requisição no servidor");
            if(m_handlerDict.count(path)){ // existe a operação que responde a requisição:
                // usa o dicionário de funções
                HandlersFunctions handler = m_handlerDict[path];
                response_data = handler(from, data);

            }else{
                // não existe a operação 
                // TODO: tratar
                // respose_data = HandleBadRequest();
            }

            // TODO: encapsular pdu com coap
            data_pdu = EncodePduResponse(response_data["status"], response_data.dump());

            response = Create<Packet>(data_pdu.buffer, data_pdu.size);
            
            if(packet->PeekPacketTag(timestampTag))
            {
                response->AddPacketTag(timestampTag);
            }
            
            NS_LOG_INFO("Enviando resposta do servidor");
            socket->SendTo(response, 0, from);
            delete[] raw_data;
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

nlohmann::json
CoTaS::HandleSubscription(Address from, nlohmann::json data_json)
{
    // verifica se já foi feita inscrição pelo endereço de ip
    int valida = ValidateIP_Q(from);
    data_json.erase("req");
    if (valida)
    {
        // ip já inscrito, manda o id novamente.
        // NS_LOG_INFO("IP já inscrito");
        // NS_LOG_INFO("Id do ip inscrito: " << valida);
        nlohmann::json res = {{"status", COAP_RESPONSE_CODE_CREATED}, {"id", valida}};
        
        return res;
    }

    // gera id seguro (vamos abstrair segurança)
    int precisa_gerar = 1;
    int id;
    while (precisa_gerar)
    {
        id = RandomInt(20000, 20000000);
        // validar id
        if (!ValidateID_Q(id))
        {
            precisa_gerar = 0;
        }
    }

    // insere dados json
    InsertDataSub_Q(id, from, data_json);

    // retorna status ok com id ou error sem id
    nlohmann::json res = {{"status", COAP_RESPONSE_CODE_CREATED}, {"id", id}};

    return res;
}

nlohmann::json
CoTaS::HandleUpdate(Address from, nlohmann::json data_json)
{
    auto collection = m_bancoMongo["object"];
    // verifica se id é válido
    int id = data_json["id"];
    data_json.erase("id");
    data_json.erase("req");
    data_json.erase("object");
    if (!ValidateID_Q(id))
    {
        // id inválido
        nlohmann::json res = {
            {"status", COAP_RESPONSE_CODE_UNAUTHORIZED},
        };

        return res;
    }

    auto query_filter = make_document(kvp("id", id));
    std::string json_string = data_json.dump();
    try
    {
        auto bson_documento = bsoncxx::from_json(json_string);
        auto update_doc = make_document(kvp("$set", bson_documento));

        auto result = collection.update_one(query_filter.view(), update_doc.view());
    }
    catch (const std::exception& e)
    {
        NS_LOG_INFO("Exceção: " << e.what());
        abort();
        nlohmann::json res = {{"status", COAP_RESPONSE_CODE_INTERNAL_ERROR}, 
                            {"msg", "Erro na consulta dso dados"}};
        
        return res;
    }

    // retorna status ok ou error
    nlohmann::json res = {{"status", COAP_RESPONSE_CODE_CHANGED}, {"id", id}};

    return res;
}

nlohmann::json
CoTaS::HandleRequest(Address from, nlohmann::json data_json)
{
    auto collection = m_bancoMongo["object"];
    nlohmann::json res = nlohmann::json::array();
    nlohmann::json cap;
    std::vector<bsoncxx::document::value> resultados;

    std::string json_string = data_json["query"].dump();
    if (json_string.empty())
    {
        cap = {{"status", COAP_RESPONSE_CODE_BAD_REQUEST}, {"info", "bad request"}};
        NS_LOG_INFO("Consumidor enviou cabeçalho errado");

        return cap;
    }
    auto bson_query = bsoncxx::from_json(json_string);

    mongocxx::options::find opts{};
    auto projection_doc = make_document(kvp("ip", 1), kvp("port", 1));
    opts.projection(projection_doc.view());

    auto cursor = collection.find(bson_query.view(), opts);

    // libera o cursor o quanto antes
    try
    {
        for (auto&& doc : cursor)
        {
            resultados.emplace_back(bsoncxx::document::value(doc)); // Copia o documento
        }
    }
    catch (const std::exception& e)
    {
        NS_LOG_INFO("Exceção: " << e.what());
        abort();
        nlohmann::json res = {{"status", COAP_RESPONSE_CODE_INTERNAL_ERROR}, 
                                {"msg", "Erro na consulta dso dados"}};
        
        return res;
    }

    // faz a lógica que precisa
    for (const auto& doc_value : resultados)
    {
        std::string json_db = bsoncxx::to_json(doc_value.view());
        res.push_back(nlohmann::json::parse(json_db));
    }
    // NS_LOG_INFO("Servidor respondendo " << data_json["info"].dump());
    cap = {{"status", COAP_RESPONSE_CODE_CONTENT}, {"response", res}};

    return cap;

    // retorna dados
}

void
CoTaS::SetupDatabase(mongocxx::client& client, std::string nome_banco)
{
    try
    {
        auto databases = client.list_database_names();
        auto it = std::find(databases.begin(), databases.end(), nome_banco);
        if (it != databases.end())
        {
            NS_LOG_INFO("Banco de dados encontrado. Resetando (drop)...");
            client[nome_banco].drop();
            NS_LOG_INFO("Banco de dados '" << nome_banco << "' foi dropado com sucesso.");
        }
        else
        {
            NS_LOG_INFO("Banco de dados '" << nome_banco << "' não existe.");
        }
        m_bancoMongo = client[nome_banco];
        auto collection = m_bancoMongo["object"];

        nlohmann::json j_documento = {{"nome", "Carlos"},
                                      {"cidade", "Juatuba"},
                                      {"ativo", true},
                                      {"timestamp", "2025-07-04T10:15:38-03:00"},
                                      {"id", 72},
                                      {"ip", 27}};

        std::string json_string = j_documento.dump();
        auto bson_documento = bsoncxx::from_json(json_string);
        auto result = collection.insert_one(bson_documento.view());
    }
    catch (const std::exception& e)
    {
        NS_LOG_INFO("Exceção: " << e.what());
        return;
    }
}

int
CoTaS::Simple_Q()
{
    auto collection = m_bancoMongo["object"];
    std::vector<bsoncxx::document::value> resultados;
    auto cursor = collection.find({});

    // libera o cursor o quanto antes
    try
    {
        for (auto&& doc : cursor)
        {
            resultados.emplace_back(bsoncxx::document::value(doc)); // Copia o documento
        }
    }
    catch (const std::exception& e)
    {
        NS_LOG_INFO("Exceção: " << e.what());
        return 0;
    }

    // faz a lógica que precisa
    for (const auto& doc_value : resultados)
    {
        std::cout << bsoncxx::to_json(doc_value) << std::endl;
    }

    return 0;
}

// se não existe ip com isso no banco retorna 0
// se existe retorna id
int
CoTaS::ValidateIP_Q(Address ip)
{
    try
    {
        auto collection = m_bancoMongo["object"];
        uint32_t ip_num = InetSocketAddress::ConvertFrom(ip).GetIpv4().Get();
        nlohmann::json j_query = {{"ip", ip_num}};
        std::string json_string = j_query.dump();

        auto bson_query = bsoncxx::from_json(json_string);
        auto result = collection.find_one(bson_query.view());
        if (result)
        {
            auto view = (*result).view();
            int id_retornado = view["id"].get_int32().value;
            return id_retornado;
        }
    }
    catch (const std::exception& e)
    {
        NS_LOG_INFO("Exceção: " << e.what());
        abort();
        return 0;
    }

    return 0;
}

// se o id já existe, ele retorna o próprio id
// se não, retorna 0
int
CoTaS::ValidateID_Q(int id)
{
    auto collection = m_bancoMongo["object"];
    nlohmann::json j_query = {{"id", id}};

    std::string json_string = j_query.dump();
    auto bson_query = bsoncxx::from_json(json_string);
    auto result = collection.find_one(bson_query.view());
    if (result)
    {
        auto view = (*result).view();
        int id_retornado = view["id"].get_int32().value;
        return id_retornado;
    }
    return 0;
}

int
CoTaS::InsertDataSub_Q(int id, Address ip, nlohmann::json data_json)
{
    auto collection = m_bancoMongo["object"];
    uint32_t ip_num = InetSocketAddress::ConvertFrom(ip).GetIpv4().Get();

    data_json["id"] = id;
    data_json["ip"] = ip_num;

    std::string json_string = data_json.dump();
    try
    {
        auto bson_documento = bsoncxx::from_json(json_string);
        auto result = collection.insert_one(bson_documento.view());
    }
    catch (const std::exception& e)
    {
        NS_LOG_INFO("Exceção: " << e.what());
        return 0;
    }

    return 1;
}

int
CoTaS::RandomInt(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}

void
CoTaS::StartHandlerDict(){
    m_handlerDict["/subscribe/object"] = [this](Address from, nlohmann::json data_json) {
        return this->HandleSubscription(from, data_json);
    };

    m_handlerDict["/subscribe/application"] = [this](Address from, nlohmann::json data_json) {
        return this->HandleSubscription(from, data_json);
    };

    m_handlerDict["/update/object"] = [this](Address from, nlohmann::json data_json) {
        return this->HandleUpdate(from, data_json);
    };

    m_handlerDict["/search"] = [this](Address from, nlohmann::json data_json) {
        return this->HandleRequest(from, data_json);
    };
}
} // Namespace ns3
