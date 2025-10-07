#include "../src/applications/model/json.hpp"
#include "../src/applications/model/httplib.h"
#include <sstream>
#include <unordered_map>
#include <unordered_set>

std::string 
SparqlPrefix(){
    std::string prefix = "BASE         <http://nesped1.caf.ufv.br/od4cot>\n"
                         "PREFIX cot:  <#>\n"
                         "PREFIX rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n"
                         "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>\n"
                         "PREFIX xsd:  <http://www.w3.org/2001/XMLSchema#>\n"
                         "PREFIX owl:  <http://www.w3.org/2002/07/owl#>\n"
                         "PREFIX qu:   <http://purl.oclc.org/NET/ssnx/qu/qu#>\n"
                         "PREFIX dim:  <http://purl.oclc.org/NET/ssnx/qu/dim#>\n"
                         "PREFIX unit: <http://purl.oclc.org/NET/ssnx/qu/unit#>\n"
                         "PREFIX lang: <https://id.loc.gov/vocabulary/iso639-1/>\n";

    return prefix;
}

int main(){
    std::ostringstream sparql_query;
    nlohmann::json response;
    httplib::Client m_cli ("localhost", 3030);

    std::string consulta_teste = " ?device a cot:Computer . ";
    // Prepara a consulta
    sparql_query << SparqlPrefix()
                 << "SELECT ?ip ?port "
                 << "WHERE { "
                 << "?device cot:ipAddress ?ip . "
                 << "?device cot:port ?port . "
                 << consulta_teste
                 << " }";

    // envia consulta
    httplib::Params params;
    params.emplace("query", sparql_query.str());

    httplib::Headers headers = {
        { "Accept", "application/sparql-results+json" }
    };

    // envia a query para o fuseki
    auto res = m_cli.Post("/dataset/query", headers, params);
    
    // trata resposta
    if (res && res->status == httplib::OK_200) 
    {
        try 
        {
            // Parse da string da resposta para um objeto JSON
            nlohmann::json j = nlohmann::json::parse(res->body);

            std::cout << "Json resposta" << j.dump() << std::endl ;

            const auto& bindings = j["results"]["bindings"];

            if (bindings.empty()) 
            {
                std::cout  << "Nenhum resultado encontrado" << std::endl ;
                response = {{"status", 1}};
            } else 
            {
                std::cout << "Resultados de objetos encontrados: " << std::endl ;
                std::cout << "bindings " << bindings.dump() << std::endl ;
                // Itera sobre cada "linha" de resultado
                // aqui é pra ter só um 
                // não há iteração
                for (const auto& item : bindings) 
                {
                    // Pega o valor da variável "?device"
                    std::string raw_ip = item["ip"]["value"];
                    uint32_t ip = std::stoi(raw_ip);
                    
                    std::string raw_port = item["port"]["value"];
                    uint32_t port = std::stoi(raw_port);
                    
                    response = {{"status", 2}};
                    response["ip"] = ip;
                    response["port"] = port;

                    std::cout << "Dispositivo de IP: " << ip
                                << " e de porta " << port 
                                << "sendo enviado para aplicação" << std::endl ;
                    break; // retorna só o primeiro por enquanto
                }
            }
        } catch (const nlohmann::json::parse_error& e) 
        {
            std::cout << "Erro no parse da resposta JSON: " << e.what() << std::endl ;
            std::cout << "Resposta recebida: " << res->body << std::endl ;
            response = {{"status", 3}};
        }
    } else 
    {
        std::cout << "Erro na requisição, status:" << res->status <<   
            "\n cabeçalho:" << res->get_header_value("Content-Type") << 
            "\n corpo:" << res->body << std::endl;
        std::cout << "error code: " << res.error() << std::endl ;
        response = {{"status", 4}};
    }
    return 0;
}