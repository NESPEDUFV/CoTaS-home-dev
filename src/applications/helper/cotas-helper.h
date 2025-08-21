/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef COTAS_HELPER_H
#define COTAS_HELPER_H

#include "ns3/application-helper.h"

#include <stdint.h>

namespace ns3
{

/**
 * @ingroup cotas
 * @brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class CoTaSHelper : public ApplicationHelper
{
  public:
    /**
     * Create CoTaSHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param port The port the server will wait on for incoming packets
     */
    CoTaSHelper(uint16_t port);

    /**
     * Create CoTaSHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param address The address the server will bind to
     */
    CoTaSHelper(const Address& address);
};

class GenericServerHelper : public ApplicationHelper
{
  public:
    /**
     * Create GenericServerHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param port The port the server will wait on for incoming packets
     */
    GenericServerHelper(uint16_t port);

    /**
     * Create GenericServerHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param address The address the server will bind to
     */
    GenericServerHelper(const Address& address);
};

/**
 * @ingroup cotas
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

};

/**
 * @ingroup cotas
 * @brief Create an application which sends a UDP packet and waits for an echo of this packet
 */
class ContextConsumerHelper : public ApplicationHelper
{
  public:
    /**
     * Create ContextConsumerHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * not include a port value (e.g., Ipv4Address and Ipv6Address).
     *
     * @param ip The IP address of the remote udp echo server
     * @param port The port number of the remote udp echo server
     */
    ContextConsumerHelper(const Address& ip, uint16_t port);

    /**
     * Create ContextConsumerHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
     *
     * @param addr The address of the remote udp echo server
     */
    ContextConsumerHelper(const Address& addr);

};


} // namespace ns3

#endif  /* COTAS_HELPER_H */
