
/*
 * Copyright 2007 Universidade Federal de Viçosa
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */


#ifndef ENCAPSULATED_COAP_H
#define ENCAPSULATED_COAP_H

#include <coap3/coap.h>
#ifdef LOG_INFO
#undef LOG_INFO
#endif
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif

#include "json.hpp"

#include <string>
#include <sstream>

#define BUFSIZE 1500

typedef struct encoded_data{
  uintptr_t size;
  uint8_t buffer[BUFSIZE];
}encoded_data;

encoded_data EncodePduRequest( const char *uri_path, coap_pdu_code_t request_code, 
      std::string data);

nlohmann::json GetPduPayloadJson(coap_pdu_t* pdu);

std::string GetPduPayloadString(coap_pdu_t* pdu);

std::string GetPduPath(coap_pdu_t* pdu);

encoded_data EncodePduResponse(coap_pdu_code_t response_code, std::string data);


#endif /* ENCAPSULATED_COAP_H */