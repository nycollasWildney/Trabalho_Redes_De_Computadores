#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct {
    //para o roteador.config
    int id;
    int porta;
    char ip[16];

    //para o enlaces.config
    int vizinho_id[100];
    int custo[100];
}config;

void ler_config(int id_roteador){

    //leitura do arquivo roteador.config para carregar as configurações do roteador
    FILE *file = fopen("roteador.config", "r");
    if (file == NULL) {
        perror("Erro ao abrir roteador.config");
        exit(EXIT_FAILURE);
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        int id, porta;
        char ip[16];
        if (sscanf(line, "%d %d %15s", &id, &porta, ip) == 3) {
            if (id == id_roteador) {
                config.id = id;
                config.porta = porta;
                strncpy(config.ip, ip, sizeof(config.ip) - 1);
                config.ip[sizeof(config.ip) - 1] = '\0';
                break;
            }
        }
    }
    fclose(file);
//==============================================================================

    //leitura do arquivo enlaces.confi para carregar os vizinhos e custos
    file = fopen("enlaces.config", "r");
    if (file == NULL) {
        perror("Erro ao abrir enlaces.config");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    while (fgets(line, sizeof(line), file) && index < 100) {
        int id,id_vizinho, custo;
        if (sscanf(line, "%d %d %*s",&id, &id_vizinho, &custo) == 2) {
            //caso o vizinho seja ele mesmo, adiciona de quem ele é vizinho
            if(id_vizinho == id_roteador){
                config.vizinho_id[index] = id;
                config.custo[index] = custo;
                index++;
                continue;
            }
            //caso o id do roteador na linha seja igual ao id do roteador atual, adiciona o vizinho
            config.vizinho_id[index] = id_vizinho;
            config.custo[index] = custo;
            index++;
        }
    }
    // Marca o fim dos vizinhos
    if (index < 100) {
        config.vizinho_id[index] = -1;
    }
    fclose(file);
}

#endif