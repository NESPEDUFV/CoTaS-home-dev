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
      m_cli{"localhost", 3030}
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
        SetupDatabase();

        NS_LOG_INFO("Banco de dados criado e conectado com sucesso.");
    }
    catch ( const std::exception& e )
    {
        NS_LOG_ERROR("An exception occurred in setup database " << e.what() );
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
            std::string data;
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
            NS_LOG_INFO("caminho que chegou no cotas:" << path);

            NS_LOG_INFO("Chegou requisição no servidor");
            if(m_handlerDict.count(path)){ // existe a operação que responde a requisição:
                // usa o dicionário de funções
                HandlersFunctions handler = m_handlerDict[path];
                response_data = handler(from, pdu);

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
CoTaS::HandleSubscription(Address from, coap_pdu_t* pdu)
{
    std::string payload = GetPduPayloadString(pdu);

    // verifica se já foi feita inscrição pelo endereço de ip
    int valida = ValidateIP_Q(from);
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
    InsertDataSub_Q(id, from, payload);

    // retorna status ok com id ou error sem id
    nlohmann::json res = {{"status", COAP_RESPONSE_CODE_CREATED}, {"id", id}};

    return res;
}

nlohmann::json
CoTaS::HandleUpdate(Address from, coap_pdu_t* pdu)
{   

    nlohmann::json payload = GetPduPayloadJson(pdu);
    nlohmann::json response;
    // NS_LOG_INFO("payload em json que chegou: " << payload.dump() );
    
    // verifica se id é válido garante que o json tem id
    if (payload.contains("objectId") && !ValidateID_Q(payload["objectId"]))
    {
        // id inválido
        NS_LOG_INFO("ID inválido, enviando mensagem de não autorizado");
        nlohmann::json res = {
            {"status", COAP_RESPONSE_CODE_UNAUTHORIZED},
        };
        
        return res;
    }
    
    // constroi mensagem
    std::string update_query = JsonToSparqlUpdateParser(payload);
    
    // NS_LOG_INFO("ultima query obtida: \n" << update_query);

    // envia consulta para o fuseki
    auto res = m_cli.Post("/dataset/update", update_query, "application/sparql-update");

    if (res && (res->status == 200 || res->status == 204)) {
        // NS_LOG_INFO("DADOS ATUALIZADOS COM SUCESSO!");
    } else {
        // se deu erro
        // NS_LOG_INFO("Erro na atualizacao");
        if (res) {
            NS_LOG_INFO("Status: " << res->status << " Body: " << res->body);
            
        } else {
            NS_LOG_INFO("Erro de conexao: " << httplib::to_string(res.error()));
        }
        response = {{"status", COAP_RESPONSE_CODE_INTERNAL_ERROR}};
        
        return response;
    }

    // se não deu erro
    response = {{"status", COAP_RESPONSE_CODE_CHANGED}};
    return response;
}

nlohmann::json
CoTaS::HandleRequest(Address from, coap_pdu_t* payload)
{
    NS_LOG_INFO("chegou uma requisição de uma aplicação ");
    auto collection = m_bancoMongo["object"];
    // nlohmann::json res = nlohmann::json::array();
    // nlohmann::json cap;
    // std::vector<bsoncxx::document::value> resultados;

    // std::string json_string = data_json["query"].dump();
    // if (json_string.empty())
    // {
    //     cap = {{"status", COAP_RESPONSE_CODE_BAD_REQUEST}, {"info", "bad request"}};
    //     NS_LOG_INFO("Consumidor enviou cabeçalho errado");

    //     return cap;
    // }
    // auto bson_query = bsoncxx::from_json(json_string);

    // mongocxx::options::find opts{};
    // auto projection_doc = make_document(kvp("ip", 1), kvp("port", 1));
    // opts.projection(projection_doc.view());

    // auto cursor = collection.find(bson_query.view(), opts);

    // // libera o cursor o quanto antes
    // try
    // {
    //     for (auto&& doc : cursor)
    //     {
    //         resultados.emplace_back(bsoncxx::document::value(doc)); // Copia o documento
    //     }
    // }
    // catch (const std::exception& e)
    // {
    //     NS_LOG_INFO("Exceção: " << e.what());
    //     abort();
    //     nlohmann::json res = {{"status", COAP_RESPONSE_CODE_INTERNAL_ERROR}, 
    //                             {"msg", "Erro na consulta dso dados"}};
        
    //     return res;
    // }

    // // faz a lógica que precisa
    // for (const auto& doc_value : resultados)
    // {
    //     std::string json_db = bsoncxx::to_json(doc_value.view());
    //     res.push_back(nlohmann::json::parse(json_db));
    // }
    // // NS_LOG_INFO("Servidor respondendo " << data_json["info"].dump());
    nlohmann::json cap = {{"status", COAP_RESPONSE_CODE_CONTENT}, {"response", "só pra parar de chorar"}};

    return cap;

    // retorna dados
    
}

void
CoTaS::SetupDatabase(){

    // le os dados do primeiro arquivo
    std::string payload;
    std::vector<std::string> arquivos;

    payload = ReadFile("definition.ttl");
    
    // primeiro arquivo
    if (auto res = m_cli.Put("/dataset/data?default", payload, "text/turtle;charset=utf-8")) 
    {
        NS_LOG_INFO("Arquivo definition.ttl" << res->status << "\n" 
                    << res->get_header_value("Content-Type") << "\n" 
                    << res->body);
    } else 
    {
        NS_LOG_INFO("error code: " << res.error());
    }

    // restante dos arquivos
    arquivos = {"application.ttl", "context.ttl",
                "object.ttl", "unit.ttl"}; 
    
    for(auto nome_arquivo : arquivos){
        payload = ReadFile(nome_arquivo);
        if (auto res = m_cli.Post("/dataset/data?default", payload, "text/turtle;charset=utf-8")) 
        {
            NS_LOG_INFO("Arquivo" << nome_arquivo << res->status << "\n" 
                        << res->get_header_value("Content-Type") << "\n" 
                        << res->body);
        } else 
        {
            NS_LOG_INFO("error code: " << res.error());
        }
    }
}

std::string 
CoTaS::ReadFile(std::string filename){
    filename = "all_data/data_ttl/"+filename;
    std::ifstream arquivo(filename);
    if (!arquivo.is_open()) {
        NS_LOG_INFO("Erro: Nao foi possivel abrir o arquivo: " << filename);
        return "";
    }
    std::stringstream buffer;
    buffer << arquivo.rdbuf();
    return buffer.str();
}

void
CoTaS::SetupDatabase_old(mongocxx::client& client, std::string nome_banco)
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
    try
    {
        std::ostringstream sparql_stream;
        sparql_stream << SparqlPrefix()
                      << "SELECT ?device WHERE { "
                      << "  ?device a cot:SmartObjectCategory . "
                      << "}";
        std::string sparql_query = sparql_stream.str();

        httplib::Params params;
        params.emplace("query", sparql_query);

        httplib::Headers headers = {
            { "Accept", "application/sparql-results+json" }
        };

        // envia a query para o fuseki
        auto res = m_cli.Post("/dataset/query", headers, params);
        
        if (res && res->status == httplib::OK_200) 
        {
            try 
            {
                // Parse da string da resposta para um objeto JSON
                nlohmann::json j = nlohmann::json::parse(res->body);

                const auto& bindings = j["results"]["bindings"];

                if (bindings.empty()) 
                {
                    NS_LOG_INFO("Nenhum resultado encontrado na simple query");
                    return 0;
                } else 
                {
                    NS_LOG_INFO("Resultados encontrados: ");
                    NS_LOG_INFO("bindings " << bindings.dump());
                    for (const auto& item : bindings) 
                    {
                        // Pega o valor da variável "?device"
                        std::string value = item["device"]["value"];

                        NS_LOG_INFO("dispositivo: " << value);
                    }
                }
            } catch (const nlohmann::json::parse_error& e) 
            {
                NS_LOG_ERROR("Erro no parse da resposta JSON: " << e.what());
                NS_LOG_ERROR("Resposta recebida: " << res->body);
                return 0;
            }
        } else 
        {
            NS_LOG_INFO("Erro na requisição, status:" << res->status << 
                "\n cabeçalho:" << res->get_header_value("Content-Type") << 
                "\n corpo:" << res->body);
            NS_LOG_INFO("error code: " << res.error());
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

// se não existe ip com isso no banco retorna 0
// se existe retorna id
int
CoTaS::ValidateIP_Q(Address ip)
{
    try
    {
        // fazer requisição de dados no fuseki

        uint32_t ip_num = InetSocketAddress::ConvertFrom(ip).GetIpv4().Get();

        std::ostringstream sparql_stream;
        sparql_stream << SparqlPrefix()
                      << "SELECT ?device ?id WHERE { "
                      << "  ?device cot:ipAddress " << ip_num << " . "
                      << "  ?device cot:objectId ?id ."
                      << "}";
        std::string sparql_query = sparql_stream.str();

        httplib::Params params;
        params.emplace("query", sparql_query);

        httplib::Headers headers = {
            { "Accept", "application/sparql-results+json" }
        };

        // envia a query para o fuseki
        auto res = m_cli.Post("/dataset/query", headers, params);
        
        if (res && res->status == httplib::OK_200) 
        {
            try 
            {
                // Parse da string da resposta para um objeto JSON
                nlohmann::json j = nlohmann::json::parse(res->body);

                const auto& bindings = j["results"]["bindings"];

                if (bindings.empty()) 
                {
                    NS_LOG_INFO("Nenhum resultado encontrado para o IP: " << ip_num );
                    return 0;
                } else 
                {
                    NS_LOG_INFO("Resultados encontrados: ");
                    NS_LOG_INFO("bindings " << bindings.dump());
                    // Itera sobre cada "linha" de resultado
                    // aqui é pra ter só um 
                    // não há iteração
                    for (const auto& item : bindings) 
                    {
                        // Pega o valor da variável "?device"
                        std::string raw_id = item["id"]["value"];
                        int id = std::stoi(raw_id);

                        NS_LOG_INFO("Dispositivo de IP: " << ip_num 
                                    << "já cadastrado com id" << id 
                                    << ", reenviando");
                        return id;
                    }
                }
            } catch (const nlohmann::json::parse_error& e) 
            {
                NS_LOG_ERROR("Erro no parse da resposta JSON: " << e.what());
                NS_LOG_ERROR("Resposta recebida: " << res->body);
                return 0;
            }
        } else 
        {
            NS_LOG_INFO("Erro na requisição, status:" << res->status << 
                "\n cabeçalho:" << res->get_header_value("Content-Type") << 
                "\n corpo:" << res->body);
            NS_LOG_INFO("error code: " << res.error());
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
    try
    {
        // fazer requisição de dados no fuseki

        std::ostringstream sparql_stream;
        sparql_stream << SparqlPrefix()
                      << "SELECT ?device WHERE { "
                      << "  ?device cot:objectId " << id << " . "
                      << "}";
        std::string sparql_query = sparql_stream.str();

        httplib::Params params;
        params.emplace("query", sparql_query);

        httplib::Headers headers = {
            { "Accept", "application/sparql-results+json" }
        };

        // envia a query para o fuseki
        auto res = m_cli.Post("/dataset/query", headers, params);
        
        if (res && res->status == httplib::OK_200) 
        {
            try 
            {
                // Parse da string da resposta para um objeto JSON
                nlohmann::json j = nlohmann::json::parse(res->body);
                // NS_LOG_INFO("json que chegou do fuseki: (ID)" << j.dump());

                const auto& bindings = j["results"]["bindings"];

                if (bindings.empty()) 
                {
                    // NS_LOG_INFO("Nenhum resultado encontrado para o ID: " << id );
                    return 0;
                } else 
                {
                    // NS_LOG_INFO("Dispositivo já registrado " << bindings.dump());
                    return id;
                }
            } catch (const nlohmann::json::parse_error& e) 
            {
                NS_LOG_ERROR("Erro no parse da resposta JSON: " << e.what());
                NS_LOG_ERROR("Resposta recebida: " << res->body);
                return 0;
            }
        } else 
        {
            NS_LOG_INFO("Erro na requisição, status:" << res->status << 
                "\n cabeçalho:" << res->get_header_value("Content-Type") << 
                "\n corpo:" << res->body);
            NS_LOG_INFO("error code: " << res.error());
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

int
CoTaS::InsertDataSub_Q(int id, Address ip, std::string payload)
{
    uint32_t ip_num = InetSocketAddress::ConvertFrom(ip).GetIpv4().Get();

    // trata payload adicionando id e ip
    std::string idip = " cot:objectId " + std::to_string(id) + 
                       "; cot:ipAddress " + std::to_string(ip_num) + " ; .";
    
    // adiciona os prefixos necessários 

    // junta tudo
    payload.erase(payload.find_last_of("."));
    payload = payload+idip;
    payload = SparqlPrefix()+payload;

    // NS_LOG_INFO("Payload pós tratamento: " << payload);

    if (auto res = m_cli.Post("/dataset/data?default", payload, "text/turtle;charset=utf-8")) 
    {
        if(res->status != 200){
            NS_LOG_INFO("======\nDEU PAU AQUI\n======");
            NS_LOG_INFO("Inseriu dados no fuseki? " << res->status << "\n" 
                << res->get_header_value("Content-Type") << "\n" 
                << res->body);
        }
    } else 
    {
        NS_LOG_INFO("Erro na inserção de dados no fuseki");
        NS_LOG_INFO("error code: " << res.error());
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
    m_handlerDict["/subscribe/object"] = [this](Address from, coap_pdu_t* pdu) {
        return this->HandleSubscription(from, pdu);
    };

    m_handlerDict["/subscribe/application"] = [this](Address from, coap_pdu_t* pdu) {
        return this->HandleSubscription(from, pdu);
    };

    m_handlerDict["/update/object"] = [this](Address from, coap_pdu_t* pdu) {
        return this->HandleUpdate(from, pdu);
    };

    m_handlerDict["/search"] = [this](Address from, coap_pdu_t* pdu) {
        return this->HandleRequest(from, pdu);
    };
}

std::string 
CoTaS::JsonToSparqlUpdateParser(nlohmann::json payload){
    // consultas sparql update é composo por 3 clausulas:
    // - delete: apaga somente a informação que irá mudar
    // - insert: insere somente a informação que irá mudar
    // - where: anda pelos nós do grafo
    //
    // para conseguir fazer o parse "/" significa que o nó
    // anterior a ele "é um nó" do que está depois dele
    // ex: physicalStorage/CoatHanger
    // esse physicalStorage é um CoatHanger
    // cot:physicalStorage a cot:CoatHanger
    // 
    // no parse "." significa que está acessando um nó 
    // mais profundo do grafo, avançando nele
    
    std::ostringstream sparql_delete;
    std::ostringstream sparql_insert;
    std::ostringstream sparql_where;

    // Set usado para não criar linhas redundantes 
    // na clausula where
    std::unordered_set<std::string> where_set; 
    
    // inicia as clausulas
    sparql_delete << "DELETE { " ;
    sparql_insert << "INSERT { " ;
    sparql_where << "WHERE { ?device cot:objectId " << payload["objectId"] << " ." ;

    payload.erase("objectId");
    // preenche as clausulas
    for (auto& elemento : payload.items()) {
        std::string chave = elemento.key();
        nlohmann::json valor = elemento.value();

        UpdateElementHandler(sparql_delete, sparql_insert, sparql_where, 
                             where_set, chave, valor);
    }
    
    // fecha as clausulas e junta elas
    sparql_delete << " }" ;
    sparql_insert << " }" ;
    sparql_where << " }" ;
    
    std::string sparql_query =  SparqlPrefix() + "\n" +
                                sparql_delete.str() + "\n" +
                                sparql_insert.str() + "\n" +
                                sparql_where.str();
    return sparql_query;
}

// prefixos usados na ontologia
std::string 
CoTaS::SparqlPrefix(){
    std::string prefix = "BASE         <http://nesped1.caf.ufv.br/od4cot>\n"
                         "PREFIX cot:  <#>\n"
                         "PREFIX rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n"
                         "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>\n"
                         "PREFIX xsd:  <http://www.w3.org/2001/XMLSchema#>\n"
                         "PREFIX owl:  <http://www.w3.org/2002/07/owl#>\n"
                         "PREFIX qu:   <http://purl.oclc.org/NET/ssnx/qu/qu#>\n"
                         "PREFIX dim:  <http://purl.oclc.org/NET/ssnx/qu/dim#>\n"
                         "PREFIX unit: <http://purl.oclc.org/NET/ssnx/qu/unit#>\n"
                         "PREFIX lang: <https://id.loc.gov/vocabulary/iso639-1/>\n";

    return prefix;
}

// Transpilador de json para sparql
// - separa em tokens (palavras ou / ou .)
// constrói consulta de acordo com cada token 
void 
CoTaS::UpdateElementHandler(
    std::ostringstream &sparql_delete,
    std::ostringstream &sparql_insert,
    std::ostringstream &sparql_where,
    std::unordered_set<std::string> &where_set,
    std::string chave,
    nlohmann::json valor
) {

    // ------------------ ANALISE LEXICA ------------------
    // obtem os tokens da chave fazendo um "split" em . e /
   
    int i_esq = 0;
    int i_dir = 0;
    std::vector<std::string> tokens;
    
    for(auto& caractere : chave){
        // anda até encontrar um '.' ou um '/' e 
        // salva a palavra 
        if(caractere == '.' || caractere == '/'){
            tokens.push_back(chave.substr(i_esq, i_dir-i_esq));
            i_esq = i_dir+1;
            std::string s(1, caractere);
            tokens.push_back(s);
        }
        i_dir++;
    }
    tokens.push_back(chave.substr(i_esq, i_dir));
    
    // ----------------- CONSTROI SINTAXE -----------------
    // variaveis iniciais 
    std::string node = "device";
    std::string oldValue = "oldValue";
    std::string where_string;
    // se tiver apenas um token então é uma propriedade direta
    // constroi só o que precisa, bem simples e retorna
    if(tokens.size() == 1){
        oldValue+=tokens[0];
        where_string = " ?" + node
                + " cot:" + tokens[0] + " ?" + oldValue 
                + " . ";
        sparql_where << where_string ;

        where_set.insert(where_string);
            
        sparql_delete << " ?" << node 
            << " cot:" << tokens[0] << " ?" 
            << oldValue << " .";
        
        sparql_insert << " ?" << node 
            << " cot:" << tokens[0] << " " 
            << valor.dump() << " . ";
        return;
    }

    // se tiver aninhamento de qualquer tipo
    // então é mais complexo => tratar

    // garante que não haja nome igual
    std::vector<std::string> safename;
    SafeName(tokens, 1, safename);

    // Atribuição do nó inicial
    where_string = " ?" + node
                    + " cot:" + tokens[0] + " ?" + node+tokens[0]+safename.back()
                    + " . ";
    
    if(!where_set.count(where_string)){
        sparql_where << where_string;
        where_set.insert(where_string);
    }

    node+=tokens[0]+safename.back();
    
    // percorre os nós emitindo variaveis de nome seguro
    // emite dados para '.' e '/' até o penultimo nó
    for(size_t i = 0; i<tokens.size()-3;i++){ // termina antes??
        if(tokens[i+1] == "/")
        {
            where_string = " ?" + node 
                + " a cot:" + tokens[i+2] + " . ";
            if(!where_set.count(where_string)){
                sparql_where << where_string;
                where_set.insert(where_string);
            }
            if(safename.empty()) node += "a"+tokens[i+2];
            else safename.pop_back();
            i++;
        }
        else if (tokens[i+1] == ".")
        {
            SafeName(tokens, i+3, safename);

            where_string = " ?" + node
                + " cot:" + tokens[i+2] + " ?" 
                + node+tokens[i+2]+safename.back() + " . ";
                
            if(!where_set.count(where_string)){
                sparql_where << where_string;
                where_set.insert(where_string);
            }
            node+=tokens[i+2]+safename.back();
            i++;
        }
    }

    // no ultimo token terá percorrido todo o grafo
    // então alcança o valor antigo e faz a 
    // substituição pelo novo valor
    size_t ultimo = tokens.size()-1;
    
    oldValue += node;

    where_string = " ?" + node
        + " cot:" + tokens[ultimo] + " ?" + oldValue 
        + " . ";

    if(!where_set.count(where_string)){
        sparql_where << where_string;
        where_set.insert(where_string);
    }
    
    sparql_delete << " ?" << node 
        << " cot:" << tokens[ultimo] << " ?" 
        << oldValue << " .";
    
    sparql_insert << " ?" << node 
        << " cot:" << tokens[ultimo] << " " 
        << valor.dump() << " . ";
}

// anda pelos tokens garantindo um bom nome
void
CoTaS::SafeName(std::vector<std::string> tokens, 
                size_t start_index, 
                std::vector<std::string> &safename){
    
    // caso base
    std::string name = "";
    safename.push_back(name);

    // empilha atribuições "/" garantindo nome unico ao final
    // porque usar pilha? para não concatenar nome redundante
    // poderia usar um contador, mas achei pilha mais legível
    for(size_t i=start_index; i+1 < tokens.size(); i=i+2){
        if(tokens[i] == "/") {
            name+="a"+tokens[i+1];
            safename.push_back(name);
        } 
        else break;
    }
    return;
}

} // Namespace ns3
