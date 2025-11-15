#include <stdio.h>
#include "PDA.h"
#include "fila.h"
#include "config.h"
#include "roteador.h"
#include "mensagem_utils.h"
#define MAX_VIZINHOS 10

/*
ERRO TA NA FUNÇAÕ menor_caminho() NELA EU CONSIDERO QUE
O i VAI PEGAR A POSSIÇÃO CORRETA COMO SE TVESSE NA COORDENADA CERTA 
MAIS ELES NÃO ESTÃO ORGANIZADOS
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
        // printf("\033[1;31m Payload: %s\n \033[0m\n",m.payload);//printa payload em VERMELHO
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
                printf("\033[1;33mRoteador %d: mensagem não é pra mim enviando para fila de saída\n\033[0m", r->id);
                fila_push(&r->fila_saida, m);
                continue;
            }
            printf("\033[1;32mRoteador %d: Mensagem recebida de roteador %d:\n\033[0m", r->id, m.IDfonte);
            //debug print da mensagem recebida
            // printf("\033[1;31m Tipo: %s\n \033[0m\n", (m.tipo == CONTROLE) ? "CONTROLE" : "DADO");
            printf("\033[1;32mPayload: %s\n \033[0m\n",m.payload);

            // Adiciona a mensagem recebida à fila de entrada
            fila_push(&r->fila_entrada, m);
            continue;
        } else {
            printf("\033[1;33mRoteador %d: Nenhuma mensagem recebida.\n\033[0m", r->id);
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
            printf("\033[1;32mRoteador %d: Processando mensagem recebida de roteador %d\n\033[0m", r->id, m.IDfonte);
            printf("\033[1;32mPayload: %s\n\033[0m",m.payload);
            continue;
        }
    }
    return NULL;
}

void *thread_terminal(void *arg){
    char opcao[10];
    roteador *r = (roteador *)arg;
    /*
    1. mostrar tabela de roteamento
    2. mostrar arrays de vizinhos e custos(+ recentes)
    3. enviar mensagem
    4. alterar intervalo de mensagem de controle
    5. sair
    */
    while(1){
        // printf("\033[1;32mRoteador %d: Processando mensagem recebida de roteador %d\n\033[0m", r->id, m.IDfonte);
        printf("\033[1;35mOpções:\n");
        printf("1. Mostrar tabela de roteamento\n");
        printf("2. Mostrar arrays de vizinhos e custos diretos\n");
        printf("3. Enviar mensagem\n");
        printf("4. Alterar intervalo de mensagem de controle\n");
        printf("5. Sair\n");
        printf("Escolha uma opção: \033[0m");
        fgets(opcao, sizeof(opcao), stdin);
        int escolha = atoi(opcao);
        switch(escolha){
            case 1:
                //mostrar tabela de roteamento
                printf("Tabela de roteamento:\n");
                printf("             0  1  2  3  4  5  6  7  8  9\n");
                for(int i=0;i<10;i++){
                    printf("Roteador %d: ", i);
                    for(int j=0;j<10;j++){
                        if(r->matriz_custo[i][j] == -1){
                            printf(" - ");
                        } else {
                            printf(" %d ", r->matriz_custo[i][j]);
                        }
                    }
                    printf("\n");
                }
                break;
            case 2:
                // print dos custos diretos
                printf("Vizinhos e custos diretos:\n");
                for(int i=0;i<MAX_VIZINHOS;i++){
                    printf("Vizinho: %d, Custo: %d\n", r->vizinhos[i], r->custos[i]);
                }
                break;
            case 3:
                //enviar mensagem
                char msg[101];
                int destino; //id do destino
                printf("Digite o ID do roteador destino: ");
                fgets(opcao, sizeof(opcao), stdin);
                destino = atoi(opcao);
                printf("Digite a mensagem: ");
                fgets(msg, sizeof(msg), stdin);
                //criar a mensagem
                mensagem m;
                m.tipo = DADO;
                m.IDfonte = r->id;
                m.IDdestino = destino;
                strncpy(m.payload, msg, sizeof(m.payload) - 1);
                m.payload[sizeof(m.payload) - 1] = '\0'; // Garantir
                fila_push(&r->fila_saida, m);
                break;
            case 4:
                //alterar intervalo de mensagem de controle
                int novo_intervalo;
                printf("Digite o novo intervalo em segundos: ");
                fgets(opcao, sizeof(opcao), stdin);
                novo_intervalo = atoi(opcao);
                printf("Intervalo antigo: %d segundos\n", r->controle_intervalo);
                r->controle_intervalo = novo_intervalo;
                printf("O novo intervalo será aplicado após a proxima iteração.\n");
                break;
            case 5:
                printf("Saindo...\n");
                exit(0);
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    }
}

//AINDA NÃO TESTEI
void *time_controle(void *arg){
    roteador *r = (roteador*) arg;
    
    while(1){
        int tempo = r->controle_intervalo;
        sleep(tempo);
        mensagem m;
        m.tipo = CONTROLE;
        m.IDfonte = r->id;
        //colocando no payload de quem é a pergutna e a resposta: mesmo que nao precise 
        snprintf(m.payload,sizeof(m.payload),"PERGUNTA:");
        
        for(int i=0;i < MAX_VIZINHOS; i++){
            m.IDdestino = r->vizinhos[i];
            fila_push(&r->fila_saida,m);
            // printf("\033[1;31m Payload: %s\n \033[0m\n",m.payload);
        }
        //dorme por 10s antes da proxima mensagem de controle
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
    //criar as threads colocar depois => ...
    pthread_t tid_enviar, tid_receber, tid_processar, tid_terminal,tid_time_controle;
    pthread_create(&tid_enviar, NULL, thread_enviar, (void *)&r);
    pthread_create(&tid_receber, NULL, thread_receber, (void *)&r);
    pthread_create(&tid_processar, NULL, thread_processar, (void *)&r);
    pthread_create(&tid_terminal, NULL, thread_terminal, (void *)&r);
    pthread_create(&tid_time_controle, NULL, time_controle, (void *)&r);

    pthread_join(tid_enviar, NULL);
    pthread_join(tid_receber, NULL);
    pthread_join(tid_processar, NULL);
    pthread_join(tid_terminal, NULL);
    pthread_join(tid_time_controle, NULL);
    return 0;
}