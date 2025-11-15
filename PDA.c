#include <stdio.h>
#include <string.h>
#include "PDA.h" // inclui o cabeçalho

void mensagem_init(mensagem *m, tipo_mensagem t, const char *p, int fonte, int destino){
    m->tipo = t;
    m->IDfonte = fonte;
    m->IDdestino = destino;
    strncpy(m->payload, p, sizeof(m->payload) - 1);
    m->payload[sizeof(m->payload) - 1] = '\0'; // Garantir terminação nula
    memset(m->array, -1, sizeof(m->array)); // Inicializa o array com zeros
}
