#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//========================================
config_t ler_config(int id_roteador){

    config_t cfg;
    // inicializa com valores padrão / sentinela
    cfg.id = -1;
    cfg.porta = -1;
    cfg.ip[0] = '\0';
    for (int i = 0; i < 100; i++) {
        cfg.vizinho_id[i] = -1;
        cfg.custo[i] = 0;
    }

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
                cfg.id = id;
                cfg.porta = porta;
                strncpy(cfg.ip, ip, sizeof(cfg.ip) - 1);
                cfg.ip[sizeof(cfg.ip) - 1] = '\0';
                break;
            }
        }
    }
    fclose(file);
    //==============================================================================

    //leitura do arquivo enlaces.config para carregar os vizinhos e custos
    file = fopen("enlaces.config", "r");
    if (file == NULL) {
        perror("Erro ao abrir enlaces.config");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    while (fgets(line, sizeof(line), file) && index < 100) {
        int id, id_vizinho, custo;
        // formato esperado: "<id_origem> <id_destino> <custo>"
        if (sscanf(line, "%d %d %d", &id, &id_vizinho, &custo) == 3) {
            // caso o vizinho seja ele mesmo, adiciona de quem ele é vizinho
            if (id_vizinho == id_roteador) {
                cfg.vizinho_id[index] = id;
                cfg.custo[index] = custo;
                index++;
                continue;
            }
            // caso o id do roteador na linha seja igual ao id do roteador atual, adiciona o vizinho
            if (id == id_roteador) {
                cfg.vizinho_id[index] = id_vizinho;
                cfg.custo[index] = custo;
                index++;
            }
        }
    }
    // Marca o fim dos vizinhos (se ainda houver espaço)
    if (index < 100) {
        cfg.vizinho_id[index] = -1;
    }
    fclose(file);

    return cfg;
}
//========================================