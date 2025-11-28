/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef GENERIC_APP_H
#define GENERIC_APP_H

#include "sink-application.h"

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

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
class GenericApplication : public SinkApplication
{
  public:
    static constexpr uint16_t DEFAULT_PORT{9}; //!< default port

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    GenericApplication();
    ~GenericApplication() override;

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

    uint8_t m_tos;         //!< The packets Type of Service
    Ptr<Socket> m_socket;  //!< Socket
    Ptr<Socket> m_socket6; //!< IPv6 Socket (used if only port is specified)

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;

    uint8_t m_recived_messages;
    uint8_t m_send_messages;
};

} // namespace ns3

#endif /* GENERIC_APP_H */
