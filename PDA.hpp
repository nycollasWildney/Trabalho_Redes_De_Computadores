#ifndef PDA_HPP
#define PDA_HPP
#include <iostream>
using namespace std;

// Estrutur para definir o valor possivel de uma mensagem 
enum class tipo_mensagem {ERRO ,CONTROLE ,DADO};

struct mensagem{
    tipo_mensagem tipo; //recebe CONTROLE ou DADO como entrada
    string payload; //conteudo da mensagem
    int IDfonte; //ID do nodo que enviou a mensagem
    int IDdestino; //ID do nodo que deve receber a mensagem

    // Construtor
    mensagem(tipo_mensagem t, const std::string &p, int fonte, int destino)
        : tipo(t), payload(p), IDfonte(fonte), IDdestino(destino){}
};
#endif
