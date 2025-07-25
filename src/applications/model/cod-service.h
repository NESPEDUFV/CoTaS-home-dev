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

    std::string HandleSubscription( 
      Address from,
      nlohmann::json data_json
    );

    std::string HandleUpdate( 
      Address from,
      nlohmann::json data_json
    );

    std::string HandleRequest( 
      Address from,
      nlohmann::json data_json
    );

    int RandomInt(int min, int max);

    int ValidateIP_Q(Address ip);

    int ValidateID_Q(int id);
    
    void SetupDatabase(mongocxx::client &client, std::string nome_banco);

    int Simple_Q();

    int InsertDataSub_Q(int id, Address ip, nlohmann::json data_json);
    
    mongocxx::v_noabi::database m_bancoMongo;
    mongocxx::instance m_instance{};
    std::optional<mongocxx::client> m_client;

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
