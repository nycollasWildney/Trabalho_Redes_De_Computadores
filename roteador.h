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

    //nao vou usar multiplos roteadores por processo, entao nao vou usar threads
    // pthread_t thread_receber; // Thread para escutar mensagens
    // pthread_t thread_enviae; // Thread para enviar mensagens
    // pthread_t thread_processar; // Thread para processar mensagens    
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
    ssize_t bytes_enviados = sendto(sock, m, sizeof(mensagem), 0, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
    if (bytes_enviados < 0) {
        perror("Erro ao enviar mensagem");
    } else {
        printf("Roteador %d: Mensagem enviada para roteador %d\n", r->id, m->IDdestino);
    }
    close(sock); //fecha o socket
}

mensagem receber(roteador *r){
    mensagem m;
    struct sockaddr_in addr_remetente;
    socklen_t addr_len = sizeof(addr_remetente);

    //cria o socket UDP
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erro ao criar socket");
        // Retorna mensagem "vazia"
        memset(&m, 0, sizeof(mensagem));
        return m;
    }

    //prepara o endereço local
    struct sockaddr_in addr_local;
    memset(&addr_local, 0, sizeof(addr_local));
    addr_local.sin_family = AF_INET;
    addr_local.sin_port = htons(r->porta);
    addr_local.sin_addr.s_addr = INADDR_ANY;

    //faz o bind do socket à porta do roteador
    if (bind(sock, (struct sockaddr *)&addr_local, sizeof(addr_local)) < 0) {
        perror("Erro ao fazer bind");
        close(sock);
        memset(&m, 0, sizeof(mensagem));
        return m;
    }

    // Recebe a mensagem (bloqueia até chegar algo)
    ssize_t bytes_recebidos = recvfrom(sock, &m, sizeof(mensagem), 0,
                                       (struct sockaddr *)&addr_remetente, &addr_len);
    if (bytes_recebidos < 0) {
        perror("Erro ao receber mensagem");
        memset(&m, 0, sizeof(mensagem));
    }

    close(sock);
    return m;
}

#endif