/*
 * Copyright 2007 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#ifndef CONTEXT_PROVIDER_H
#define CONTEXT_PROVIDER_H

#include "source-application.h"

#include "ns3/deprecated.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "json.hpp"
#include <optional>
#include <vector>
#include <fstream>

namespace ns3
{

class Socket;
class Packet;

/**
 * @ingroup udpecho
 * @brief A Context Provider client
 *
 * Every packet sent should be returned by the server and received here.
 */
class ContextProvider : public SourceApplication
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    ContextProvider();
    ~ContextProvider() override;

    static constexpr uint16_t DEFAULT_PORT{0}; //!< default port

    /**
     * @brief set the remote address and port
     * @param ip remote IP address
     * @param port remote port
     */
    NS_DEPRECATED_3_44("Use SetRemote without port parameter instead")
    void SetRemote(const Address& ip, uint16_t port);
    void SetRemote(const Address& addr) override;

    uint32_t GetDataSize() const;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * @brief Set the remote port (temporary function until deprecated attributes are removed)
     * @param port remote port
     */
    void SetPort(uint16_t port);

    /**
     * @brief Get the remote port (temporary function until deprecated attributes are removed)
     * @return the remote port
     */
    uint16_t GetPort() const;

    /**
     * @brief Get the remote address (temporary function until deprecated attributes are removed)
     * @return the remote address
     */
    Address GetRemote() const;

    /**
     * @brief Schedule the next packet transmission
     * @param dt time interval between packets.
     */
    void ScheduleTransmit(Time dt);
    /**
     * @brief Send a packet
     */
    void Send();

    /**
     * @brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * @param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    void SetDataMessage();

    std::string RandomData();
    
    int RandomInt(int min, int max) ;

    uint32_t m_count; //!< Maximum number of packets the application will send
    Time m_interval;  //!< Packet inter-send time

    uint32_t m_sent;                    //!< Counter for sent packets
    Ptr<Socket> m_socket;               //!< Socket
    std::optional<uint16_t> m_peerPort; //!< Remote peer port (deprecated) // NS_DEPRECATED_3_44
    EventId m_sendEvent;                //!< Event to send the next packet
    uint32_t m_objectType;
    uint32_t m_objectId;
    nlohmann::json m_firstData;
    nlohmann::json m_updateData;
    nlohmann::json m_messages;

    /// Callbacks for tracing the packet Tx events
    TracedCallback<Ptr<const Packet>> m_txTrace;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    /// Callbacks for tracing the packet Tx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_txTraceWithAddresses;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* CONTEXT_PROVIDER_H */
