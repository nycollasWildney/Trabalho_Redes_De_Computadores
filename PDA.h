#ifndef PDA_H
#define PDA_H
#include <stdio.h>
#include <string.h>

// Estrutur para definir o valor possivel de uma mensagem 
typedef enum {
    ERRO ,
    CONTROLE ,
    DADO
}tipo_mensagem;

typedef struct {
    tipo_mensagem tipo; //recebe CONTROLE ou DADO como entrada
    char payload[101]; //conteudo da mensagem
    int IDfonte; //ID do nodo que enviou a mensagem
    int IDdestino; //ID do nodo que deve receber a mensagem
}mensagem;

void mensagem_init(mensagem *m, tipo_mensagem t, const char *p, int fonte, int destino) {
    m->tipo = t;
    m->IDfonte = fonte;
    m->IDdestino = destino;
    strncpy(m->payload, p, sizeof(m->payload) - 1);
    m->payload[sizeof(m->payload) - 1] = '\0'; // Garantir terminação nula
}

#endif