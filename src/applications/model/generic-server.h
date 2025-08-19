/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef GENERIC_SERVER_H
#define GENERIC_SERVER_H

#include "sink-application.h"

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "json.hpp"

namespace ns3
{

class Socket;
class Packet;

/**
 * @ingroup applications
 * @defgroup cotas cotas
 */

/**
 * @ingroup cotas
 * @brief A Generic Data Server
 *
 * If an Client send messages to the specific interface
 * it'll response with the message in HandleRead.
 */
class GenericServer : public SinkApplication
{
  public:
    static constexpr uint16_t DEFAULT_PORT{9}; //!< default port

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    GenericServer();
    ~GenericServer() override;

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

    int RandomInt(int min, int max);

    void SetDataMessage();

    std::string RandomData();

    uint32_t m_objectType;

    nlohmann::json m_updateData;

    uint8_t m_tos;         //!< The packets Type of Service
    Ptr<Socket> m_socket;  //!< Socket
    Ptr<Socket> m_socket6; //!< IPv6 Socket (used if only port is specified)

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* GENERIC_SERVER*/
