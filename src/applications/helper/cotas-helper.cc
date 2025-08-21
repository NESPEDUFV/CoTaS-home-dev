/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "cotas-helper.h"

#include "ns3/address-utils.h"
#include "ns3/cotas.h"
#include "ns3/context-consumer.h"
#include "ns3/context-provider.h"
#include "ns3/generic-server.h"
#include "ns3/uinteger.h"

namespace ns3
{

CoTaSHelper::CoTaSHelper(uint16_t port)
    : ApplicationHelper(CoTaS::GetTypeId())
{
    SetAttribute("Port", UintegerValue(port));
}

CoTaSHelper::CoTaSHelper(const Address& address)
    : ApplicationHelper(CoTaS::GetTypeId())
{
    SetAttribute("Local", AddressValue(address));
}

GenericServerHelper::GenericServerHelper(uint16_t port)
    : ApplicationHelper(GenericServer::GetTypeId())
{
    SetAttribute("Port", UintegerValue(port));
}

GenericServerHelper::GenericServerHelper(const Address& address)
    : ApplicationHelper(GenericServer::GetTypeId())
{
    SetAttribute("Local", AddressValue(address));
}

ContextProviderHelper::ContextProviderHelper(const Address& address, uint16_t port)
    : ContextProviderHelper(addressUtils::ConvertToSocketAddress(address, port))
{
}

ContextProviderHelper::ContextProviderHelper(const Address& address)
    : ApplicationHelper(ContextProvider::GetTypeId())
{
    SetAttribute("Remote", AddressValue(address));
}

ContextConsumerHelper::ContextConsumerHelper(const Address& address, uint16_t port)
    : ContextConsumerHelper(addressUtils::ConvertToSocketAddress(address, port))
{
}

ContextConsumerHelper::ContextConsumerHelper(const Address& address)
    : ApplicationHelper(ContextConsumer::GetTypeId())
{
    SetAttribute("Remote", AddressValue(address));
}



} // namespace ns3
