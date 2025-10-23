#include <stdio.h>
#include "Fila.h"
#include "PDA.h"
#include "roteador.h"
#include "config.h"

#include <unistd.h>//biblioteca para manipulação de arquivos
#include <arpa/inet.h>//biblioteca para manipulação de endereços IP
#include <sys/socket.h>//biblioteca para sockets
#include <pthread.h>//biblioteca para threads
#define MAX_VIZINHOS 10

/*
    Essas funções são ponteiro para ser usadas na função de criar threads
    o argumento é ponteiro arg para receber o roteador ou outros argumentos
*/

/*
    FALTA COLOCAR UMA FUNCAO PARA CALCULAR QUAL MELHOR CAMINHO ATE UM 
    DESTINO USANDO BELLMAN-FORD E VERIFICAR SE O ARRAY ESTA CERTO
*/

void *thread_enviar(void *arg){
    //tornar o *arg em roteador
    roteador *r = (roteador *)arg;
    while(1){
        //Caso a fila esteja vazia, aguarda um pouco antes de verificar novamente
        if(fila_empty(&r->fila_saida)){
            usleep(100000);
            continue;
        }
        mensagem m = fila_pop(&r->fila_saida);
        // if(m.tipo == CONTROLE){
        //     continue;
        //     //poor equanto não faz nada
        // }
        // else if(m.tipo == DADO){
            enviar(r, &m);
            printf("\033[1;31m Payload: %s\n \033[0m\n",m.payload);//printa payload em VERMELHO
            // continue;
        // }
    }
    return NULL;
}

void *thread_receber(void *arg){
    roteador *r = (roteador *)arg;
    mensagem m;
    while(1){
        int receiver = receber(r,&m);
        if(receiver){
            if(m.IDdestino != r->id){
                printf("Roteador %d: mensagem não é pra mim enviando para fila de saída\n", r->id);
                fila_push(&r->fila_saida, m);
                continue;
            }
            printf("Roteador %d: Mensagem recebida de roteador %d:\n", r->id, m.IDfonte);
            //debug print da mensagem recebida
            printf("\033[1;31m Tipo: %s\n \033[0m\n", (m.tipo == CONTROLE) ? "CONTROLE" : "DADO");
            printf("\033[1;31m Payload: %s\n \033[0m\n",m.payload);

            // Adiciona a mensagem recebida à fila de entrada
            fila_push(&r->fila_entrada, m);
            continue;
        } else {
            printf("Roteador %d: Nenhuma mensagem recebida.\n", r->id);
        }
    }
    return NULL;
}

void *thread_processar(void *arg){
    roteador *r = (roteador *)arg;
    mensagem m;
    while(1){
        //sempre manter essa verificação para evitar travar o processo
        if (fila_empty(&r->fila_entrada)){
            usleep(100000); // Dorme por 100 ms
            continue;
        }
        
        m = fila_pop(&r->fila_entrada);
        if(m.tipo == CONTROLE){
            mensagem_controle(r,&m);
            continue;
        }
        if(m.tipo == DADO){
            //processar a mensagem recebida
            printf("Roteador %d: Processando mensagem recebida de roteador %d\n", r->id, m.IDfonte);
            printf("\033[1;32m Payload: %s\n \033[0m\n",m.payload);
            continue;
        }
    }
    return NULL;
}

//usada para periodicamente manda rmensagem de controle
void *time_controle(void *arg){
    roteador *r = (roteador*) arg;

    while(1){
        mensagem m;
        m.tipo = CONTROLE;
        m.IDfonte = r->id;
        //colocando no payload de quem é a pergutna e a resposta: mesmo que nao precise 
        snprintf(m.payload,sizeof(m.payload),"PERGUNTA:");

        for(int i=0;i < r->num_vizinhos; i++){
            m.IDdestino = r->vizinhos[i];
            fila_push(&r->fila_saida,m);
            printf("\033[1;31m Payload: %s\n \033[0m\n",m.payload);
        }
        //dorme por 10s antes da proxima mensagem de controle
        sleep(10);
    }
}

//usada quando receber a mensagem de controle pede decidir se manda com o array ou manda a solicitação de array
void mensagem_controle(roteador *r, mensagem *m){
    if(m->tipo != CONTROLE){
        printf("\033[1;31m a mensagem recebida não é de controle \n \033[0m\n");
        return;
    }

    if(strcmp(m->payload,"PERGUNTA:") == 0){
        array_from_mensagem(r,m);
        pritnf("\033[1;31m Payload %s\n \033[0m\n",m->payload);
        return;
    }
    if(strncmp(m->payload,"RESPOSTA:",9) == 0){
        int caminhos[MAX_VIZINHOS][MAX_VIZINHOS];
        mensagem_from_array(r,m,caminhos);
        /*
            aqui colocar um controle de quantas mensagens de controle
            sairam para controlar se ja recebeu todas as respostas 
            e apos isso verificar caminhos curtos e atualiza a matriz 
            e depois avisa as threads que a matriz mudou e manda mensagens de 
            controle dizendo que mudou
        */
        return;
    }
}

int main(int argc, char *argv[]){
    roteador r;
    config_t cfg;    
    //verifica se o id do roteador foi passado como argumento
    if(argc < 2){
        printf("Uso: %s <id_roteador>\n", argv[0]);
        return 1;
    }
    
    r.id = atoi(argv[1]);
    cfg = ler_config(r.id); 
    roteador_init(&r, &cfg, r.id);
    printf("Roteador %d iniciado na porta %d com IP %s\n", r.id, r.porta, r.ip);
}