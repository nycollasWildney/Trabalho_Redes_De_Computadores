#include "roteador.h"
#include "PDA.h"
#include "fila.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>//biblioteca para manipulação de arquivos
#include <arpa/inet.h>//biblioteca para manipulação de endereços IP
#include <sys/socket.h>//biblioteca para sockets
#include <pthread.h>//biblioteca para threads
#define MAX_VIZINHOS 10
#define PORTA 5000
#define CONTROLE 10

void roteador_init(roteador *r, config_t *config, int id){
    r->id = id;
    r->porta = config->porta + id;
    strncpy(r->ip, config->ip, sizeof(r->ip) - 1);
    r->ip[sizeof(r->ip) - 1] = '\0';
    r->num_vizinhos = 0;
    memset(r->vizinhos, -1, sizeof(r->vizinhos));
    memset(r->custos, -1, sizeof(r->custos));
    for (int i = 0; i < MAX_VIZINHOS && config->vizinho_id[i] != -1; i++) {
        r->vizinhos[config->vizinho_id[i]] = config->vizinho_id[i];
        r->custos[config->vizinho_id[i]] = config->custo[i];
        r->num_vizinhos++;
    }
    r->msg_controle_recebidas=0;    
    r->controle_intervalo=CONTROLE; //tempo em segundos do intervalo para mensagem de controle
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

    memset(r->matriz_custo, -1, sizeof(r->matriz_custo));
    for(int i=0;i < MAX_VIZINHOS;i++){
        r->matriz_custo[r->id][r->vizinhos[i]] = r->custos[i];
    }
    r->matriz_custo[r->id][r->id] = 0;
    r->matriz_mudou = 0;
}

void enviar(roteador *r, mensagem *m){
    char *ip_dest = NULL;
    int porta_dest = -1;

    for(int i=0;i < MAX_VIZINHOS;i++){
        if(r->vizinhos[i] == m->IDdestino){
            ip_dest = r->ip; //usando o proprio ip, pois todos os roteadores estao na mesma maquina
            porta_dest = PORTA + r->vizinhos[i]; //porta do roteador destino
            // printf("\033[1;33mRoteador %d: Enviando mensagem para roteador %d na porta %d\n\033[0m",r->id, m->IDdestino, porta_dest);
            break;
        }
    }
    //caso não tenha vizinhos diretos(fazer belmman-ford futuramente)
    if(ip_dest == NULL || porta_dest == -1){
        // printf("\033[1:31mRoteador %d: Roteador destino %d não é vizinho direto. Mensagem não enviada.\n\033[0m", r->id, m->IDdestino);
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
    // int sock_to_use = (r->socket >= 0) ? r->socket : socket(AF_INET, SOCK_DGRAM, 0);
    // sendto(sock_to_use, m, sizeof(mensagem), 0, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
    // if (sock_to_use != r->socket) close(sock_to_use);
    ssize_t sent = sendto(r->socket,m,sizeof(mensagem),0,(struct sockaddr *)&addr_dest,sizeof(addr_dest));
    if(sent < 0){
        perror("erro ao enviar a mensagem");
        return 0;
    }
}

int receber(roteador *r, mensagem *m) {
    if (r->socket < 0) return 0;
    struct sockaddr_in rem;
    socklen_t len = sizeof(rem);
    memset(m,0,sizeof(mensagem));
    //nao precisa retornar o -m- pois ele entra como ponteiro entao ele sai om os valores
    
    ssize_t n = recvfrom(r->socket, m, sizeof(mensagem), 0, (struct sockaddr *)&rem, &len);
    if(n <= 0){
        perror("erro ao receber a mensagem");
        return 0;
    }
    return 1;
}

void send_all_roteadores(roteador *r, mensagem *m){
    m->tipo = CONTROLE;
    m->IDfonte = r->id;
    for(int i=0;i<r->num_vizinhos;i++){
        if(r->vizinhos[i] == -1)continue;
        m->IDdestino = r->vizinhos[i];
        // printf("\033[1;31mPayload: %s\n\033[0m",m->payload);
        fila_push(&r->fila_saida,*m);
    }
}

void menor_caminho(roteador *r, mensagem *m,int mudou){
    for(int i=0;i<MAX_VIZINHOS;i++){
        //não quero ver ele mesmo
        if(i == r->id){
            continue;
        }
        //não precisa olhar quem não tem ligação direta
        if(r->custos[i] == -1){
            continue;
        }
        for(int j=0;j<MAX_VIZINHOS;j++){
            if(r->matriz_custo[i][j] != -1 && r->matriz_custo[r->id][j] != -1){
                int novo_custos = r->matriz_custo[i][j];
                // printf("");
                if(novo_custos < r->matriz_custo[r->id][j]){
                    r->matriz_custo[r->id][j] = novo_custos;
                    mudou = 1;
                }
            }
            if(r->matriz_custo[i][j] != -1 && r->matriz_custo[r->id][j] == -1){
                r->matriz_custo[r->id][j] = r->matriz_custo[i][j] + r->custos[i];
                mudou = 1;
            }
        }     
    }
    if(mudou){
        strcpy(m->payload,"RESPOSTA_MODIFICADA:");
        array_from_mensagem(r,m);
        send_all_roteadores(r,m);
        r->matriz_mudou = 0;
        return 0;
    }
}





