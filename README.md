# CoTaS@Home: Context of Things as a Service at Home

Um serviço de disponibilização objetos inteligentes a partir de seus perfis contextuais para casas inteligentes

## Table of contents

- [CoTaS@Home: Context of Things as a Service at Home](#cotashome-context-of-things-as-a-service-at-home)
  - [Table of contents](#table-of-contents)
  - [Como instalar](#como-instalar)
    - [NS3](#ns3)
    - [Jena Fuseki](#jena-fuseki)
    - [CoAP](#coap)
    - [json e httplib](#json-e-httplib)
    - [Dados do CoTaS@Home](#dados-do-cotashome)
  - [Como executar](#como-executar)

## Como instalar

O serviço CoTaS@Home possui algumas dependencias, sendo elas:

- Uma biblioteca para encapsular mensagens no formato de protocolo de aplicação CoAP <coap3/coap.h>
- Uma bilioteca para lidar com requisições http <httplib.h>
- Uma biblioteca para lidar com o formato de mensagens json <json.hpp>
- Network Simulator 3 (ns3) para simulações
- Jena Fuseki como banco de dados ontológico
  
### NS3

Para instalar o ns3, basta seguir os passos do tutorial no link da propria organização [ns3](https://www.nsnam.org/releases/).

A versão utilizada para o desenvolvimento do projeto é <3.42, atualmente estamos desenvolvendo na 3.46

### Jena Fuseki

Para instalar o jena fuseki basta seguir os passos do tutorial no link da propria organização [jena-fuseki](https://jena.apache.org/documentation/fuseki2/).

No desenvolvimento estamos usando a versão dockerizada do software, para baixar entre no  [repositorio jena-fuseki-docker](https://repo1.maven.org/maven2/org/apache/jena/jena-fuseki-docker/) e selecione a versão desejada.

A versão utilizada para o desenvolvimento do projeto é <5.5.0, atualmente estamos desenvolvendo na 5.5.0.

Neste repositório também existe os arquivos docker usados para o desenvolvimento, todos eles se encontram no diretório [jena-fuseki](/jena-fuseki/).

> recomendo a instalação do [docker rootless](https://docs.docker.com/engine/security/rootless/) para usos gerais

### CoAP

Para instalar o CoAP que usamos no projeto basta clonar [este fork](https://github.com/piface314/libcoap/tree/develop-4.3.5) do repositório original do libcoap, e, na branch develop-4.3.5 executar os seguintes comandos:

```./autogen.sh```

```./configure --prefix="${NS3_HOME}/build"  --disable-doxygen --disable-manpages```

```make```

```make install```

### json e httplib

Ambas bibliotecas são header only, então ao executar o script que transfere os dados para seu simulador ns3 ele irá já deixar configurado.

### Dados do CoTaS@Home

Para colocar os dados do CoTaS@Home em seu simulador, foi feito um script que copia os arquivos para o diretório da variavel de ambiente NS3_HOME.

Para que tudo de certo, crie a variável de ambiente na forma.

```NS3_HOME = "/home/username/diretorio/até/ns-3.46"```

e execute o script <nome_script>.sh

## Como executar

Feita todas devidas instalações das dependencias basta executar o conteiner do jena fuseki com as configurações usadas em desenvolvimento:

```docker compose build --build-arg JENA_VERSION=5.5.0```

depois executar o conteiner com:

```sudo docker compose up```

> obs: execute esse comando dentro do diretório do docker-compose.yml

entrar no diretório do ns3 e executar em seu terminal:

```./ns3 run casaInteligente_20.cc```
