/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef COD_SERVICE_HELPER_H
#define COD_SERVICE_HELPER_H

#include "ns3/application-helper.h"

#include <stdint.h>

namespace ns3
{

/**
 * @ingroup udpecho
 * @brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class CoDServiceHelper : public ApplicationHelper
{
  public:
    /**
     * Create CoDServiceHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param port The port the server will wait on for incoming packets
     */
    CoDServiceHelper(uint16_t port);

    /**
     * Create CoDServiceHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param address The address the server will bind to
     */
    CoDServiceHelper(const Address& address);
};

/**
 * @ingroup udpecho
 * @brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class ContextProviderHelper : public ApplicationHelper
{
  public:
    /**
     * Create ContextProviderHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * not include a port value (e.g., Ipv4Address and Ipv6Address).
     *
     * @param ip The IP address of the remote udp echo server
     * @param port The port number of the remote udp echo server
     */
    ContextProviderHelper(const Address& ip, uint16_t port);

    /**
     * Create ContextProviderHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
     *
     * @param addr The address of the remote udp echo server
     */
    ContextProviderHelper(const Address& addr);

    /**
     * Given a pointer to a UdpEchoClient application, set the data fill of the
     * packet (what is sent as data to the server) to the contents of the fill
     * string (including the trailing zero terminator).
     *
     * @warning The size of resulting echo packets will be automatically adjusted
     * to reflect the size of the fill string -- this means that the PacketSize
     * attribute may be changed as a result of this call.
     *
     * @param app Smart pointer to the application (real type must be UdpEchoClient).
     * @param fill The string to use as the actual echo data bytes.
     */
    void SetFill(Ptr<Application> app, const std::string& fill);

    /**
     * Given a pointer to a UdpEchoClient application, set the data fill of the
     * packet (what is sent as data to the server) to the contents of the fill
     * byte.
     *
     * The fill byte will be used to initialize the contents of the data packet.
     *
     * @warning The size of resulting echo packets will be automatically adjusted
     * to reflect the dataLength parameter -- this means that the PacketSize
     * attribute may be changed as a result of this call.
     *
     * @param app Smart pointer to the application (real type must be UdpEchoClient).
     * @param fill The byte to be repeated in constructing the packet data..
     * @param dataLength The desired length of the resulting echo packet data.
     */
    void SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength);

    /**
     * Given a pointer to a UdpEchoClient application, set the data fill of the
     * packet (what is sent as data to the server) to the contents of the fill
     * buffer, repeated as many times as is required.
     *
     * Initializing the fill to the contents of a single buffer is accomplished
     * by providing a complete buffer with fillLength set to your desired
     * dataLength
     *
     * @warning The size of resulting echo packets will be automatically adjusted
     * to reflect the dataLength parameter -- this means that the PacketSize
     * attribute of the Application may be changed as a result of this call.
     *
     * @param app Smart pointer to the application (real type must be UdpEchoClient).
     * @param fill The fill pattern to use when constructing packets.
     * @param fillLength The number of bytes in the provided fill pattern.
     * @param dataLength The desired length of the final echo data.
     */
    void SetFill(Ptr<Application> app, uint8_t* fill, uint32_t fillLength, uint32_t dataLength);
};

} // namespace ns3

#endif  /* COD_SERVICE_HELPER_H */
