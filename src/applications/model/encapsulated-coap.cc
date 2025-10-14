#include "encapsulated-coap.h"
#include <iostream>

encoded_data 
EncodePduRequest(const char *uri_path, 
    coap_pdu_code_t request_code, std::string data)
{
    coap_pdu_t *pdu;
    encoded_data dados;
    uint8_t check;

    // printf("A codificar %s \n Tamanho : %d\n\n", (const uint8_t*)data.c_str(), data.size());

    pdu = coap_pdu_init(COAP_MESSAGE_CON, request_code, 0, BUFSIZE);
    if (!pdu)
    {
        printf("falha em criar PDU CoAP no provedor.");
        abort();
    }

    check = coap_add_option(pdu, COAP_OPTION_URI_PATH, strlen(uri_path), 
                            (const uint8_t*)uri_path);
    if (!check){
        printf("falha em colocar um path na PDU CoAP no provedor.");
        abort();
    }

    check = coap_add_data(pdu, data.size(), (const uint8_t*)data.c_str());
    if(!check){
        printf("falha em colocar dados na PDU CoAP no provedor.");
        abort();
    }

    coap_pdu_encode_header(pdu, COAP_PROTO_UDP);
    dados.size = coap_pdu_dump(pdu, dados.buffer, BUFSIZE); 
    return dados;
}

encoded_data 
EncodePduResponse(coap_pdu_code_t response_code, std::string data){
    coap_pdu_t *pdu;
    encoded_data dados;
    uint8_t check;

    pdu = coap_pdu_init(COAP_MESSAGE_CON, response_code, 0, BUFSIZE);
    if (!pdu)
    {
        printf("falha em criar PDU CoAP no provedor.");
        abort();
    }

    check = coap_add_data(pdu, data.size(), (const uint8_t*)data.c_str());
    if(!check){
        printf("falha em colocar dados na PDU CoAP no provedor.");
        abort();
    }

    coap_pdu_encode_header(pdu, COAP_PROTO_UDP);
    dados.size = coap_pdu_dump(pdu, dados.buffer, BUFSIZE); 
    return dados;
}

std::string 
GetPduPath(coap_pdu_t* pdu)
{
    std::stringstream path_stream;
    coap_opt_iterator_t opt_iter;
    coap_opt_t* opt;
    std::string path;

    opt = coap_check_option(pdu, COAP_OPTION_URI_PATH, &opt_iter);
    if(opt){
        path_stream.write(reinterpret_cast<const char*>(coap_opt_value(opt)), 
            coap_opt_length(opt));
    }else{
        printf("Erro na interpretação do path");
        abort();
    }
    
    path = path_stream.str();
    if (path.empty())
    {
        return "/"; // Se nenhum path foi encontrado, é uma requisição para a raiz.
    }

    return path;
}

nlohmann::json
GetPduPayloadJson(coap_pdu_t* pdu)
{
    const uint8_t *pdu_data;
    size_t pdu_data_offset;
    size_t pdu_data_total_length;
    size_t pdu_data_length;
    uint8_t check;
    nlohmann::json payload;

    check = coap_get_data_large(pdu, &pdu_data_length, &pdu_data,
                            &pdu_data_offset, &pdu_data_total_length);

    if(!check){
        printf("Ocorreu um erro ao pegar o payload da pdu");
        abort();
    }


    try
    {
        payload = nlohmann::json::parse(pdu_data, pdu_data + pdu_data_total_length);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Erro no parse do Json GetPduPayloadJson" << '\n';
        std::cerr << e.what() << '\n';
    }

    return payload;
}

std::string
GetPduPayloadString(coap_pdu_t* pdu)
{
    const uint8_t *pdu_data;
    size_t pdu_data_offset;
    size_t pdu_data_total_length;
    size_t pdu_data_length;
    uint8_t check;
    std::string payload;

    check = coap_get_data_large(pdu, &pdu_data_length, &pdu_data,
                            &pdu_data_offset, &pdu_data_total_length);

    if(!check){
        printf("Ocorreu um erro ao pegar o payload do pdu");
        abort();
    }
    try
    {
        std::string s (reinterpret_cast<const char*>(pdu_data), pdu_data_total_length);
        payload = s;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Ocorreu um erro no parse String GetPduPayloadString" << '\n';
        std::cerr << e.what() << '\n';
    }
    
    return payload;
}
