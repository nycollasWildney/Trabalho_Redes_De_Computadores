#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "PDA.h"
#include "fila.h"

void fila_init(Fila *f) {
    f->inicio = 0;
    f->fim = -1;
    f->tamanho = 0;
    pthread_mutex_init(&f->mutex, NULL);
    pthread_cond_init(&f->cond_vazio, NULL);
    pthread_cond_init(&f->cond_cheio, NULL);
}

int fila_empty(Fila *f) {
    return f->tamanho == 0;
}

int fila_full(Fila *f) {
    return f->tamanho == 10;
}

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