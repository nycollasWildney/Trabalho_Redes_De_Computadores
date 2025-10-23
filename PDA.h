#ifndef PDA_H
#define PDA_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Estrutur para definir o valor possivel de uma mensagem 
typedef enum {
    ERRO ,
    CONTROLE ,
    DADO
}tipo_mensagem;

typedef struct {
    tipo_mensagem tipo; //recebe CONTROLE ou DADO como entrada
    char payload[101]; //conteudo da mensagem
    int IDfonte; //ID do nodo que enviou a mensagem
    int IDdestino; //ID do nodo que deve receber a mensagem
}mensagem;

void mensagem_init(mensagem *m, tipo_mensagem t, const char *p, int fonte, int destino) {
    m->tipo = t;
    m->IDfonte = fonte;
    m->IDdestino = destino;
    strncpy(m->payload, p, sizeof(m->payload) - 1);
    m->payload[sizeof(m->payload) - 1] = '\0'; // Garantir terminação nula
}

//funcao para tornar o array em payload para ser enviado
void array_from_mensagem(roteador *r,mensagem *m){
    char c[15];
    strcpy(m->payload,"RESPOSTA:");//RESPOSTA:
    sprintf(c,"%d:",r->id);
    strcat(m->payload,c);//RESPOSTA:id:

    for(int i=0;i< r->num_vizinhos;i++){
        sprintf(c,"%d-%d;",r->vizinhos[i],r->custos[i]);
        strcat(m->payload,c);
    }
    strcat(m->payload,"!");//indicar o fim do array
}

//funcao para tornar o payload em array
void mensagem_from_array(roteador *r,mensagem *m,int *caminhos[][10]){
    //"PERGUNTA: 1: 1-2;2-1;3-6;!"
    char temp[101];
    strcpy(temp,m->payload);

    char *token = strtok(temp, ":");//token = RESPOSTA:
    token = strtok(NULL, ":");//token = 1
    token = strtok(NULL,"!");// token = 1-2;2-1;3-6;

    char *subtoken = strtok(token,";");//subtoken = 1-2
    while(subtoken != NULL){
        char *id_str = strtok(subtoken,"-");// 1-2 => id_str = 1
        char *custo_str = strtok(NULL,"-"); // 1-2 => custo_str = 2
        if(id_str != NULL && custo_str != NULL){
            int id = atoi(id_str);
            int custo = atoi(custo_str);
            caminhos[m->IDfonte][id] = custo;
        }
        subtoken = strtok(NULL,";");
    }
}

#endif