#ifndef FILA_H
#define FILA_H
#include <string.h>
#include <pthread.h>
#include "PDA.h"

typedef struct {
    mensagem buffer[10]; //buffer para armazenar ate 10 mensagens 
    int inicio; // Índice do início da fila
    int fim; // Índice do fim da fila
    int tamanho; // Tamanho atual da fila
    pthread_mutex_t mutex; //mutex para controlar a fila
    pthread_cond_t cond_vazio; //controle para fila vazia
    pthread_cond_t cond_cheio; //controle para fila cheia
}Fila;

//inicializa a fila
void fila_init(Fila *f);
//verifica se a fila está vazia
int fila_empty(Fila *f);
//verifica se a fila está cheia
int fila_full(Fila *f);
//função para adicionar uma mensagem na fila
void fila_push(Fila *f, mensagem m);
//função para remover uma mensagem da fila
mensagem fila_pop(Fila *f);

#endif