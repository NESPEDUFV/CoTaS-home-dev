/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "cod-service-helper.h"

#include "ns3/address-utils.h"
#include "ns3/cod-service.h"
#include "ns3/context-consumer.h"
#include "ns3/context-provider.h"
#include "ns3/uinteger.h"

namespace ns3
{

CoDServiceHelper::CoDServiceHelper(uint16_t port)
    : ApplicationHelper(CoDService::GetTypeId())
{
    SetAttribute("Port", UintegerValue(port));
}

CoDServiceHelper::CoDServiceHelper(const Address& address)
    : ApplicationHelper(CoDService::GetTypeId())
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
