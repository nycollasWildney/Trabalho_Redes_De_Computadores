#ifndef ROTEADOR_H
#define ROTEADOR_H
#include <stdio.h>
#include <string.h>
#include <unistd.h>//biblioteca para manipulação de arquivos
#include <arpa/inet.h>//biblioteca para manipulação de endereços IP
#include <sys/socket.h>//biblioteca para sockets
#include <pthread.h>//biblioteca para threads
#include "PDA.h"
#include "Fila.h"
#include "config.h"
#define MAX_VIZINHOS 10

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

    //adiciona socket compartilhado do roteador
    int socket;

    pthread_mutex_t rt_mutex; // Mutex para proteger a tabela de roteamento
}roteador;

void roteador_init(roteador *r, config_t *config, int id){
    r->id = id;
    r->porta = config->porta;
    strncpy(r->ip, config->ip, sizeof(r->ip) - 1);
    r->ip[sizeof(r->ip) - 1] = '\0';
    r->num_vizinhos = 0;
    for (int i = 0; i < MAX_VIZINHOS && config->vizinho_id[i] != -1; i++) {
        r->vizinhos[i] = config->vizinho_id[i];
        r->custos[i] = config->custo[i];
        r->num_vizinhos++;
    }
    pthread_mutex_init(&r->rt_mutex, NULL);
    fila_init(&r->fila_entrada);
    fila_init(&r->fila_saida);

    /* 
    cria um socket UDP único e faz bind na porta do roteador (uma vez) 
    e nao deixa lento por sempre criar e fechar o socket em enviar e receber
    */
    r->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (r->socket < 0) {
        perror("Erro ao criar socket do roteador");
        r->socket = -1;
    } else {
        struct sockaddr_in addr_local;
        memset(&addr_local, 0, sizeof(addr_local));
        addr_local.sin_family = AF_INET;
        addr_local.sin_port = htons(r->porta);
        addr_local.sin_addr.s_addr = INADDR_ANY;
        if (bind(r->socket, (struct sockaddr *)&addr_local, sizeof(addr_local)) < 0) {
            perror("Erro ao dar bind no socket do roteador");
            close(r->socket);
            r->socket = -1;
        }
    }
}

void enviar(roteador *r, mensagem *m){
    char *ip_dest = NULL;
    int porta_dest = -1;

    for(int i=0;i < r->num_vizinhos;i++){
        if(r->vizinhos[i] == m->IDdestino){
            ip_dest = r->ip; //usando o proprio ip, pois todos os roteadores estao na mesma maquina
            porta_dest = 5000 + r->vizinhos[i]; //porta do roteador destino
            printf("Roteador %d: Enviando mensagem para roteador %d na porta %d\n", r->id, m->IDdestino, porta_dest);
            break;
        }
    }
    //caso não tenha vizinhos diretos(fazer belmman-ford futuramente)
    if(ip_dest == NULL || porta_dest == -1){
        printf("Roteador %d: Roteador destino %d não é vizinho direto. Mensagem não enviada.\n", r->id, m->IDdestino);
        return;
    }

    //montar o endereço para enviar a mensagem
    struct sockaddr_in addr_dest; // Endereço do roteador destino com ipv4
    memset(&addr_dest, 0, sizeof(addr_dest));//limpa a struct para evitar lixo de memória
    addr_dest.sin_family = AF_INET; // IPv4
    addr_dest.sin_port = htons(porta_dest); // Porta do roteador destino
    inet_pton(AF_INET, ip_dest, &addr_dest.sin_addr); //converte string para formato binário

    //criar o socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    if (sock < 0) {
        perror("Erro ao criar socket");
        return; 
    }
    //enviar a mensagem
    // use r->sock se for válido:
    int sock_to_use = (r->socket >= 0) ? r->socket : socket(AF_INET, SOCK_DGRAM, 0);
    sendto(sock_to_use, m, sizeof(mensagem), 0, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
    if (sock_to_use != r->socket) close(sock_to_use);
}

//recebe a struct de mensagem ja inicializada fora da função
int receber(roteador *r, mensagem *m) {
    if (r->socket < 0) return 0;
    struct sockaddr_in rem;
    socklen_t len = sizeof(rem);
    //nao precisa retornar o -m- pois ele entra como ponteiro entao ele sai om os valores
    ssize_t n = recvfrom(r->socket, m, sizeof(mensagem), 0, (struct sockaddr *)&rem, &len);
    if(n < 0){
        perror("erro ao receber a mensagem");
        return 0;
    }
    return 1;
}

#endif