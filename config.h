#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
    //para o roteador.config
    int id;
    int porta;
    char ip[16];

    //para o enlaces.config
    int vizinho_id[100];
    int custo[100];
} config_t;

/*
 * Lê as configurações para o roteador com id 'id_roteador'
 * Retorna uma estrutura config_t com os valores carregados.
*/
config_t ler_config(int id_roteador);

#endif