#!/bin/bash

ALL_DATA="all_data"
SCRATCH="scratch"
SRC="src"
CMAKE_ORIGEM="src/applications/CMakeLists.txt"
CMAKE_DESTINO="${NS3_HOME}/src/applications/CMakeLists.txt"


if [ -v NS3_HOME ]; then
    echo "Pasta de destino: ${NS3_HOME}"
    echo "Copiando arquivos para dentro do ns3"
    
    echo "Copiando dados"
    cp "${ALL_DATA}" "${NS3_HOME}" -r --update=none
    
    echo "Copiando simulações"
    cp "${SCRATCH}" "${NS3_HOME}" -r --update=none

    echo "Copiando arquivos fonte"
    cp "${SRC}" "${NS3_HOME}" -r --update=none
    
    LIBS_DESTINO=$(grep "LIBRARIES_TO_LINK" "$CMAKE_DESTINO" | sed 's/LIBRARIES_TO_LINK *//; s/[()]//g')
    LIBS_ORIGEM=$(grep "LIBRARIES_TO_LINK" "$CMAKE_ORIGEM" | sed 's/LIBRARIES_TO_LINK *//; s/[()]//g')

    echo "Bibliotecas encontradas no arquivo de ORIGEM: [${LIBS_ORIGEM}]"
    echo "Bibliotecas encontradas no arquivo de DESTINO: [${LIBS_DESTINO}]"

    
    LISTA_COMPLETA="$LIBS_DESTINO $LIBS_ORIGEM"

    LIBS_FINAIS=$(echo "$LISTA_COMPLETA" | tr ' ' '\n' | grep -v '^$' | sort -u | tr '\n' ' ' | sed 's/ *$//')

    echo "Lista final de bibliotecas (mesclada e sem duplicatas): [${LIBS_FINAIS}]"

    NOVA_LINHA="  LIBRARIES_TO_LINK ${LIBS_FINAIS} "

    echo "$NOVA_LINHA"

    sed -i "s|^[[:space:]]*LIBRARIES_TO_LINK.*|$NOVA_LINHA|" "$CMAKE_ORIGEM"

    PATCH_FILE=$(mktemp)

    diff -u "$CMAKE_DESTINO" "$CMAKE_ORIGEM" > "$PATCH_FILE"

    if [ -s "$PATCH_FILE" ]; then
        echo "Diferenças encontradas. Aplicando atualizações em '$CMAKE_DESTINO'..."
        patch "$CMAKE_DESTINO" < "$PATCH_FILE"
        echo "CMakeLists.txt atualizado com sucesso."
    else
        echo "Arquivos CMakeLists.txt já estão sincronizados. Nenhuma ação necessária."
    fi

    rm "$PATCH_FILE"

else
    echo "A variavel de ambiente NS3_HOME não está definida."
    echo "por favor defina. Exemplo:"
    echo " export NS3_HOME=\"/home/username/diretorio/até/ns-3.46\" "
fi