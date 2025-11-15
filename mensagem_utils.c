#include "mensagem_utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_VIZINHOS 10

void array_from_mensagem(roteador *r, mensagem *m) {
    if (m == NULL || r == NULL) return;
    // Copia a matriz de custos da linha do roteador para o array da mensagem
    for (int i = 0; i < MAX_VIZINHOS; i++) {
        m->array[i] = r->matriz_custo[r->id][i];
    }
    printf("\033[1;32m[ROTEADOR %d] Array copiado para mensagem: ", r->id);
    for (int i = 0; i < MAX_VIZINHOS; i++) {
        printf("%d ", m->array[i]);
    }
    printf("\n\033[0m");
    // char c[15];
    // for (int i = 0; i < MAX_VIZINHOS; i++) {
    //     // printf("\033[1;31mMATRIZ CUSTO[%d][%d] = %d\n\033[0m",r->id,i, r->matriz_custo[r->id][i]);
    //     if(r->matriz_custo[r->id][i] != -1){
    //         // printf("\033[1;31mVALORES: %d - %d\n\033[0m",i, r->matriz_custo[r->id][i]);
    //         sprintf(c, "%d-%d;", i,r->matriz_custo[r->id][i]);
    //         strcat(m->payload, c);
    //     }
    // }
    // strcat(m->payload, "!");
    // printf("\033[1;31mAdicionando ao payload: %s\n\033[0m",m->payload);
}

void mensagem_from_array(roteador *r, mensagem *m, int caminhos[MAX_VIZINHOS][MAX_VIZINHOS]) {
      if (m == NULL || r == NULL) return;
    
    char *payload = m->payload;
    int remetente = m->IDfonte;
    int eh_modificada = 0;
    
    // Identifica o tipo de resposta
    if (strncmp(payload, "RESPOSTA_MODIFICADA:", 20) == 0) {
        eh_modificada = 1;
    } else if (strncmp(payload, "RESPOSTA:", 9) == 0) {
        eh_modificada = 0;
    } else {
        printf("\033[1;31m[ROTEADOR %d] Tipo de payload desconhecido: %s\n\033[0m", r->id, payload);
        return;
    }

    printf("\033[4;33m[ROTEADOR %d] Processando array do roteador %d (Tipo: %s)\n\033[0m", 
           r->id, remetente, eh_modificada ? "RESPOSTA_MODIFICADA" : "RESPOSTA");

    // Valida o remetente
    if (remetente < 0 || remetente >= MAX_VIZINHOS) {
        printf("\033[1;31m[ROTEADOR %d] Remetente inválido: %d\n\033[0m", r->id, remetente);
        return;
    }
    // Se não tem ligação direta com o remetente, ignora
    if (r->custos[remetente] == -1) {
        printf("\033[1;31m[ROTEADOR %d] Sem ligação direta com remetente %d, ignorando\n\033[0m", r->id, remetente);
        return;
    }
    // Processa cada elemento do array
    for (int i = 0; i < MAX_VIZINHOS; i++) {
        // Se tem custo válido (diferente de -1)
        if (m->array[i] != -1) {
            // Calcula o novo custo: custo recebido + custo até o remetente
            int custo_total = m->array[i] + r->custos[remetente];
            caminhos[remetente][i] = custo_total;
            
            // Atualiza a matriz de custos
            r->matriz_custo[remetente][i] = custo_total;
            
            printf("\033[1;36m[ROTEADOR %d] Atualizado: roteador %d->%d = %d (recebido:%d + conexão:%d)\n\033[0m",
                   r->id, remetente, i, custo_total, m->array[i], r->custos[remetente]);
        }
    }
}

void mensagem_controle(roteador *r, mensagem *m){
    int mudou=r->matriz_mudou;
    if(m->tipo != CONTROLE){
        printf("\033[1;31m a mensagem recebida não é de controle \n\033[0m\n");
        return;
    }

    if(strcmp(m->payload,"PERGUNTA:") == 0){
        //muda para RESPOSTA: a mensagem
        snprintf(m->payload,sizeof(m->payload),"RESPOSTA:");
        array_from_mensagem(r,m);
        // printf("\033[1;31m Payload %s\n \033[0m\n",m->payload);
        m->tipo = CONTROLE;
        m->IDdestino = m->IDfonte; //responder para quem perguntou
        m->IDfonte = r->id;
        fila_push(&r->fila_saida,*m);
        return;
    }
    
    if(strncmp(m->payload,"RESPOSTA:",9) == 0){
        int caminhos[MAX_VIZINHOS][MAX_VIZINHOS];
        mensagem_from_array(r,m,caminhos);
        r->msg_controle_recebidas++;
        if(r->msg_controle_recebidas == r->num_vizinhos){
            /*
            aqui colocar um controle de quantas mensagens de controle
            sairam para controlar se ja recebeu todas as respostas 
            e apos isso verificar caminhos curtos e atualiza a matriz 
            e depois avisa as threads que a matriz mudou e manda mensagens de 
            controle dizendo que mudou
            */
            r->msg_controle_recebidas = 0;
            menor_caminho(r,m,mudou);
        }
        return;
    }
}
