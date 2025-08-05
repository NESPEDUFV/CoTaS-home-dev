/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef GENERIC_APPLICATION_HELPER_H
#define GENERIC_APPLICATION_HELPER_H

#include "ns3/application-helper.h"

#include <stdint.h>

namespace ns3
{

/**
 * @ingroup udpecho
 * @brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class GenericApplicationHelper : public ApplicationHelper
{
  public:
    /**
     * Create GenericApplicationHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param port The port the server will wait on for incoming packets
     */
    GenericApplicationHelper(uint16_t port);

    /**
     * Create GenericApplicationHelper which will make life easier for people trying
     * to set up simulations with echos.
     *
     * @param address The address the server will bind to
     */
    GenericApplicationHelper(const Address& address);
};

} // namespace ns3

#endif /* GENERIC_APPLICATION_HELPER_H */
