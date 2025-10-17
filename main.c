#include <stdio.h>
#include "Fila.h"
#include "PDA.h"
#include "roteador.h"

#include <unistd.h>//biblioteca para manipulação de arquivos
#include <arpa/inet.h>//biblioteca para manipulação de endereços IP
#include <sys/socket.h>//biblioteca para sockets
#include <pthread.h>//biblioteca para threads

//implementa a função para receber mensagens e colocar na fila de entrada ou saida
void thread_receber(roteador *r){
    mensagem m;
    ssize_t bytes_recebidos;//usando para saber se a mensagem foi recebida ou não
    while(1){
        m = receber(r);
        bytes_recebidos = sizeof(m);
        if (bytes_recebidos > 0) {
            if(m.IDdestino != r->id){
                printf("Roteador %d: mensagem não é pra mim enviando para fila de saída\n", r->id);
                fila_push(&r->fila_saida, m);
                continue;
            }
            printf("Roteador %d: Mensagem recebida de roteador %d:\n", r->id, m.IDfonte);
            //debug print da mensagem recebida
            printf("  Tipo: %s\n", (m.tipo == CONTROLE) ? "CONTROLE" : "DADO");
            printf("  Payload: %s\n", m.payload);

            // Adiciona a mensagem recebida à fila de entrada
            fila_push(&r->fila_entrada, m);
            continue;
        } else {
            printf("Roteador %d: Nenhuma mensagem recebida.\n", r->id);
        }
    }
}

//implementa a função para enviar mensagens da fila de saída
void thread_enviar(roteador *r){
    mensagem m;
    while(1){
        if (fila_empty(&r->fila_saida)) {
            // Se a fila de saída estiver vazia espera um pouco para voltar a verificar
            usleep(100000); // Dorme por 100 ms
            continue;
        }
        m = fila_pop(&r->fila_saida);
        if(m.tipo == "CONTROLE"){
            //nada ainda , vai ser usado para bellman-ford futuramente para saber o custo dos vizinhos
        }
        if(m.tipo == "DADO"){   
            enviar(r, &m);
        }
    }
}

void thread_processar(roteador *r){}

int main(int argc, char *argv[]){

}