#ifndef MENSAGEM_UTILS_H
#define MENSAGEM_UTILS_H

#include "roteador.h"
#include "PDA.h"
#define MAX_VIZINHOS 10

// transforma dados do roteador em string no payload
void array_from_mensagem(roteador *r, mensagem *m);
// transforma o payload recebido em matriz de caminhos
void mensagem_from_array(roteador *r, mensagem *m, int caminhos[MAX_VIZINHOS][MAX_VIZINHOS]);

void mensagem_controle(roteador *r, mensagem *m);

#endif
