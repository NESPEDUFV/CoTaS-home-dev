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

void
ContextProviderHelper::SetFill(Ptr<Application> app, const std::string& fill)
{
    app->GetObject<ContextProvider>()->SetFill(fill);
}

void
ContextProviderHelper::SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
    app->GetObject<ContextProvider>()->SetFill(fill, dataLength);
}

void
ContextProviderHelper::SetFill(Ptr<Application> app,
                             uint8_t* fill,
                             uint32_t fillLength,
                             uint32_t dataLength)
{
    app->GetObject<ContextProvider>()->SetFill(fill, fillLength, dataLength);
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

void
ContextConsumerHelper::SetFill(Ptr<Application> app, const std::string& fill)
{
    app->GetObject<ContextConsumer>()->SetFill(fill);
}

void
ContextConsumerHelper::SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
    app->GetObject<ContextConsumer>()->SetFill(fill, dataLength);
}

void
ContextConsumerHelper::SetFill(Ptr<Application> app,
                             uint8_t* fill,
                             uint32_t fillLength,
                             uint32_t dataLength)
{
    app->GetObject<ContextConsumer>()->SetFill(fill, fillLength, dataLength);
}


} // namespace ns3
