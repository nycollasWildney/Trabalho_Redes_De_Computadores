#ifndef ROTEADOR_H
#define ROTEADOR_H
#include "PDA.h"
#include "fila.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>//biblioteca para manipulação de arquivos
#include <arpa/inet.h>//biblioteca para manipulação de endereços IP
#include <sys/socket.h>//biblioteca para sockets
#include <pthread.h>//biblioteca para threads
#define MAX_VIZINHOS 10
#define PORTA 5000

/*
o array vizihos e custos são usados para ter os valores iniciais que são ligados diretos
a matriz vai ter o custo de todos que ele recebeu
*/

typedef struct {
    int id; // ID do roteador
    int porta; //porta do roteador
    char ip[16]; // Endereço IP do roteador como o valor (127.0.0.1)

    //usar futuramnte quando tiver bellman-ford, atualmente não tem uso total
    int num_vizinhos; // Número de vizinhos
    int vizinhos[MAX_VIZINHOS]; // IDs dos roteadores vizinhos (máximo de 10)
    int custos[MAX_VIZINHOS]; // Custos para os roteadores vizinhos
    Fila fila_entrada; // Fila de entrada de mensagens
    Fila fila_saida; // Fila de saída de mensagens
    int matriz_custo[10][10]; // matriz dos custos 
    int matriz_mudou; //indica se a matriz mudou

    //adiciona socket compartilhado do roteador
    int socket;
    pthread_mutex_t rt_mutex; // Mutex para proteger a tabela de roteamento

    int msg_controle_recebidas;
    int controle_intervalo;
    /*
      talvez eu use essa parte para caso a matrix mude 
      e avise as threads que a matrix mudou apos ter
      recebido todas as alterações das mensagens de controle  
    */
    // int mudanca;
    // pthread_mutex_t mutex_mudanca;
    // pthread_cond_t cond_mudanca;


}roteador;

//todos funcionais como unidades sozinhas(FALTA TRESTAR COM VARIOS FUNCIONA :)
void roteador_init(roteador *r, config_t *config, int id);
void enviar(roteador *r, mensagem *m);
int receber(roteador *r, mensagem *m);
void send_all_roteadores(roteador *r, mensagem *m);

#endif