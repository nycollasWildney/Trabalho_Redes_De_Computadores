## Objetivo rápido
Este repositório implementa um simulador simples de roteadores (em C) que
se comunicam via UDP localmente. Cada processo representa um roteador com
filas de entrada/saída, threads para enviar/receber/processar mensagens e
configuração por arquivos `roteador.config` e `enlaces.config`.

## Arquitetura e componentes principais
- `main.c` — inicializa um `roteador` a partir de `config.h` e dispara as
  threads (enviar, receber, processar). O programa espera um argumento: o
  id do roteador (ex: `./programa 1`).
- `roteador.h` — definição de `roteador` e funções UDP: `roteador_init`,
  `enviar`, `receber`. Mantém um socket UDP por roteador (bind na porta).
- `PDA.h` — definição da struct `mensagem` e utilitários para serializar/
  desserializar arrays de vizinhança em `payload` (`array_from_mensagem`,
  `mensagem_from_array`). Observação: essas funções usam símbolos de
  `roteador` (dependência implícita).
- `Fila.h` — implementação de fila circular com mutex/conds (buffer fixo 10).
- `config.h` — parser simples que lê `roteador.config` para porta/IP e
  `enlaces.config` para vizinhos e custos; retorna `config_t` usado por
  `roteador_init`.

## Como executar (exemplo)
- Compilar com um compilador C (gcc). Projeto não fornece Makefile. Exemplo:
  gcc -pthread main.c -o roteador
- Criar/editar `roteador.config` e `enlaces.config` (exemplos já no repositório).
- Iniciar cada roteador em um terminal distinto usando seu id: `./roteador 1`.

## Convenções de projeto importantes
- Rede é simulada localmente: todos os roteadores usam o mesmo IP (127.0.0.1)
  e portas baseadas em id (ex.: porta = 5000 + id). O `roteador.config` já
  pode sobrescrever a porta, mas o código assume portas como 25001/25002 nos
  exemplos.
- Mensagens têm tipo (`CONTROLE` ou `DADO`) e `payload` limitado a 100 bytes.
  Funções de controle usam o payload com prefixos exatos: `"PERGUNTA:"`
  para pedir vizinhança e `"RESPOSTA:"` + formato `id: id-custo;...!` para
  respostas. Modificar esses formatos requer alterar `PDA.h`.
- A fila tem capacidade fixa 10; o produtor/consumidor usam `pthread_cond`.
  Assuma bloqueio em push/pop se a fila estiver cheia/vazia.

## Padrões de código e pontos fracos conhecidos
- Socket único por roteador: `roteador_init` faz bind e guarda `r->socket`.
  Contudo `enviar` atualmente cria/usa um socket temporário se `r->socket`
  for inválido; revisar para usar sempre `r->socket` quando disponível.
- Serialização em `PDA.h` usa `strtok` e buffers estáticos; não é thread-safe
  se múltiplas threads manipularem a mesma mensagem simultaneamente.
- Bellman-Ford não implementado — o comentário em `main.c` indica intenção
  futura de calcular caminhos quando destino não for vizinho direto.

## Exemplos concretos para modificações frequentes
- Adicionar nova lógica de roteamento: atualizar `mensagem_from_array`,
  `array_from_mensagem` em `PDA.h`, e preencher/usar `matriz_custo` em
  `roteador.h`/`main.c` antes de enviar mensagens quando destino não for
  vizinho.
- Para instrumentação/log: `printf` colorido já é usado em `main.c` —
  siga esse padrão para mensagens de debug (ex.: payload em vermelho/verde).

## Pontos de integração e scripts/entradas
- `roteador.config` — linhas: `<id> <porta> <ip>`; `ler_config` escolhe a
  linha que corresponde ao id passado como argumento.
- `enlaces.config` — linhas: `<id_origem> <id_destino> <custo>`; `ler_config`
  interpreta entradas tanto na forma origem->destino quanto destino=ele,
  para popular vizinhos do roteador.

## O que um agente deve priorizar ao editar este repositório
1. Fazer mudanças pequenas e localizadas (manter APIs em `PDA.h` e
   `roteador.h` consistentes). Cite os arquivos afetados nos commits.
2. Evitar alterar a formatação do payload sem atualizar ambos
   `array_from_mensagem` e `mensagem_from_array` (cópia/parse).
3. Preservar o uso de mutex/conds em `Fila.h` se tocar na implementação.

## Exemplos rápidos (trechos relevantes)
- Inicialização: `config_t cfg = ler_config(id); roteador_init(&r,&cfg,id);`
- Envio: `fila_push(&r.fila_saida, m);` (thread de enviar pega e chama `enviar`).
- Recepção: `recvfrom(r->socket, m, sizeof(mensagem), ...)` em `receber`.

## Perguntas que você pode me pedir se precisar de mais contexto
- Deseja que eu adicione um Makefile ou scripts de execução?
- Prefere que eu implemente Bellman-Ford ou somente melhore a serialização
  e segurança de threads primeiro?

Se algo estiver impreciso, me diga qual seção quer que eu expanda ou corrija.
