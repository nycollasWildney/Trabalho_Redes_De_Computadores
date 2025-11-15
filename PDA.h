#ifndef PDA_H
#define PDA_H

typedef enum {
    ERRO,
    CONTROLE,
    DADO
} tipo_mensagem;

typedef struct {
    tipo_mensagem tipo;
    char payload[101];
    int IDfonte;
    int IDdestino;
    int array[10];
} mensagem;

void mensagem_init(mensagem *m, tipo_mensagem t, const char *p, int fonte, int destino);

/*
\033[<estilo>;<cor>m

stilos:
Código	Efeito
0	Reset / Normal
1	Negrito
4	Sublinhado
7	Invertido
Cores do texto:
Código	Cor do texto
30	Preto
31	Vermelho
32	Verde
33	Amarelo
34	Azul
35	Magenta
36	Ciano
37	Branco
Cores de fundo:
Código	Cor do fundo
40	Preto
41	Vermelho
42	Verde
43	Amarelo
44	Azul
45	Magenta
46	Ciano
47	Branco
*/

#endif