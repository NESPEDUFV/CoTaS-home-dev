/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef COD_SERVICE_H
#define COD_SERVICE_H

#include "sink-application.h"

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "json.hpp"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

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
class CoDService : public SinkApplication
{
  public:
    static constexpr uint16_t DEFAULT_PORT{9}; //!< default port

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    CoDService();
    ~CoDService() override;

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

    void HandleSubscription(
      ns3::Ptr<ns3::Socket> socket, 
      Address from,
      nlohmann::json data_json
    );

    void HandleUpdate(
      ns3::Ptr<ns3::Socket> socket, 
      Address from,
      nlohmann::json data_json
    );

    void HandleRequest(
      ns3::Ptr<ns3::Socket> socket, 
      Address from,
      nlohmann::json data_json
    );

    int RandomInt(int min, int max);

    std::string ReadSqlArc();

    int ValidateIP_Q(Address ip);

    int ValidateID_Q(int id);
    
    void SetupDatabase(mongocxx::client &client, std::string nome_banco);

    int Simple_Q();

    int InsertDataSub_Q(int id, Address ip, nlohmann::json data_json);
    
    nlohmann::json GetDataJSON_Q(int id);

    void UpdtData_JSON(nlohmann::json& db, nlohmann::json new_data); 

    void UpdtData_Q(int id, nlohmann::json db);

    mongocxx::v_noabi::database m_bancoMongo;

    uint8_t m_tos;         //!< The packets Type of Service
    Ptr<Socket> m_socket;  //!< Socket
    Ptr<Socket> m_socket6; //!< IPv6 Socket (used if only port is specified)

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* COD_SERVICE_H */
