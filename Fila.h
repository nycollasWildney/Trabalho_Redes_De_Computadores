#ifndef FILA_H
#define FILA_H
#include <stdio.h>
#include <string.h>
#include "PDA.h"
#include <pthread.h>

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
void fila_init(Fila *f) {
    f->inicio = 0;
    f->fim = -1;
    f->tamanho = 0;
    pthread_mutex_init(&f->mutex, NULL);
    pthread_cond_init(&f->cond_vazio, NULL);
    pthread_cond_init(&f->cond_cheio, NULL);
}

//verifica se a fila está vazia
int fila_empty(Fila *f) {
    return f->tamanho == 0;
}
//verifica se a fila está cheia
int fila_full(Fila *f) {
    return f->tamanho == 10;
}

//função para adicionar uma mensagem na fila
void fila_push(Fila *f, mensagem m) {
    pthread_mutex_lock(&f->mutex); //bloqueia o mutex para evitar concorrencia
    while (fila_full(f)) {
        //espera pelo sinal de que a fila não está cheia
        pthread_cond_wait(&f->cond_cheio, &f->mutex);
    }
    f->fim = (f->fim + 1) % 10;//calcula a posição circular
    f->buffer[f->fim] = m;
    f->tamanho++;
    //destrava o mutex caso a fila não esteja cheia
    pthread_cond_signal(&f->cond_vazio);//condição de fila vazia
    pthread_mutex_unlock(&f->mutex);
}

//função para remover uma mensagem da fila
mensagem fila_pop(Fila *f) {
    pthread_mutex_lock(&f->mutex); //bloqueia o mutex para evitar concorrencia
    while (fila_empty(f)) {
        //espera pelo sinal de que a fila não está vazia
        pthread_cond_wait(&f->cond_vazio, &f->mutex);
    }
    mensagem m = f->buffer[f->inicio];
    f->inicio = (f->inicio + 1) % 10;//calcula a posição circular
    f->tamanho--;
    //destrava o mutex caso a fila não esteja vazia
    pthread_cond_signal(&f->cond_cheio);//condição de fila cheia
    pthread_mutex_unlock(&f->mutex);
    return m;
}

#endif