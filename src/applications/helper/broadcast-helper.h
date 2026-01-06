/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef BROADCAST_HELPER_H
#define BROADCAST_HELPER_H

#include "ns3/application-helper.h"

#include <stdint.h>

namespace ns3
{

/**
 * @ingroup cotas
 * @brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class BroadcastProviderHelper : public ApplicationHelper
{
  public:
    /**
     * Create BroadcastProviderHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param port The port the server will wait on for incoming packets
     */
    BroadcastProviderHelper(uint16_t port);

    /**
     * Create BroadcastProviderHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param address The address the server will bind to
     */
    BroadcastProviderHelper(const Address& address);
};

class BroadcastConsumerHelper : public ApplicationHelper
{
  public:
    /**
     * Create BroadcastConsumerHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * not include a port value (e.g., Ipv4Address and Ipv6Address).
     *
     * @param ip The IP address of the remote udp echo server
     * @param port The port number of the remote udp echo server
     */
    BroadcastConsumerHelper(const Address& ip, uint16_t port);

    /**
     * Create BroadcastConsumerHelper which will make life easier for people trying
     * to set up simulations with echos. Use this variant with addresses that do
     * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
     *
     * @param addr The address of the remote udp echo server
     */
    BroadcastConsumerHelper(const Address& addr);

};


} // namespace ns3

#endif  /* BROADCAST_HELPER_H */
