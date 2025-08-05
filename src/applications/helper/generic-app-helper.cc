/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "generic-app-helper.h"

#include "ns3/address-utils.h"
#include "ns3/generic-app.h"
#include "ns3/context-consumer.h"
#include "ns3/context-provider.h"
#include "ns3/uinteger.h"

namespace ns3
{

GenericApplicationHelper::GenericApplicationHelper(uint16_t port)
    : ApplicationHelper(GenericApplication::GetTypeId())
{
    SetAttribute("Port", UintegerValue(port));
}

GenericApplicationHelper::GenericApplicationHelper(const Address& address)
    : ApplicationHelper(GenericApplication::GetTypeId())
{
    SetAttribute("Local", AddressValue(address));
}

} // namespace ns3
