/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef COTAS_H
#define COTAS_H

#include <coap3/coap.h>
#ifdef LOG_INFO
#undef LOG_INFO
#endif
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif

#include "sink-application.h"

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "json.hpp"
#include "encapsulated-coap.h"
#include "httplib.h"

#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <vector>

namespace ns3
{

class Socket;
class Packet;

/**
 * @ingroup applications
 * @defgroup udpecho UdpEcho
 */

/**
 * @ingroup udpecho
 * @brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class CoTaS : public SinkApplication
{
  public:
    static constexpr uint16_t DEFAULT_PORT{9}; //!< default port

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    CoTaS();
    ~CoTaS() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * @brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * @param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    nlohmann::json HandleSubscription( 
      Address from,
      coap_pdu_t* pdu
    );

    nlohmann::json HandleUpdate( 
      Address from,
      coap_pdu_t* pdu
    );

    nlohmann::json HandleRequest( 
      Address from,
      coap_pdu_t* pdu
    );

    int RandomInt(int min, int max);

    int ValidateIP_Q(Address ip);

    int ValidateID_Q(int id);
    
    void SetupDatabase();

    std::string ReadFile(std::string filename);

    int Simple_Q();

    int InsertDataSub_Q(int id, Address ip, std::string payload);

    void StartHandlerDict();

    std::string JsonToSparqlUpdateParser(nlohmann::json payload);

    std::string SparqlPrefix();

     
    void SafeName(std::vector<std::string> tokens, 
        size_t start_index, std::vector<std::string> &safename);

    void UpdateElementHandler(std::ostringstream &sparql_delete,
                              std::ostringstream &sparql_insert,
                              std::unordered_set<std::string> &where_set,
                              std::string chave,
                              nlohmann::json valor);    
    
    using HandlersFunctions = std::function<nlohmann::json(Address, coap_pdu_t*)>;
    std::unordered_map<std::string, HandlersFunctions> m_handlerDict;
    
    uint8_t m_tos;         //!< The packets Type of Service
    Ptr<Socket> m_socket;  //!< Socket
    Ptr<Socket> m_socket6; //!< IPv6 Socket (used if only port is specified)

    httplib::Client m_cli; //!< cliente http (aqui coloca a Uri do jena fuseki)

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* COTAS_H */
