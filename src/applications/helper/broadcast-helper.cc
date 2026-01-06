/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "broadcast-helper.h"

#include "ns3/address-utils.h"
#include "ns3/cotas.h"
#include "ns3/broadcast-consumer.h"
#include "ns3/broadcast-provider.h"
#include "ns3/generic-server.h"
#include "ns3/uinteger.h"

namespace ns3
{

BroadcastProviderHelper::BroadcastProviderHelper(uint16_t port)
    : ApplicationHelper(BroadcastProvider::GetTypeId())
{
    SetAttribute("Port", UintegerValue(port));
}

BroadcastProviderHelper::BroadcastProviderHelper(const Address& address)
    : ApplicationHelper(BroadcastProvider::GetTypeId())
{
    SetAttribute("Local", AddressValue(address));
}

BroadcastConsumerHelper::BroadcastConsumerHelper(const Address& address, uint16_t port)
    : BroadcastConsumerHelper(addressUtils::ConvertToSocketAddress(address, port))
{
}

BroadcastConsumerHelper::BroadcastConsumerHelper(const Address& address)
    : ApplicationHelper(BroadcastConsumer::GetTypeId())
{
    SetAttribute("Remote", AddressValue(address));
}


} // namespace ns3
