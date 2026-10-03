# Contexto do projeto

Por que o projeto está como está: de onde veio a ideia, sob quais
restrições, o que foi decidido e descartado, o que ainda está em aberto e o
que vem a seguir. O README explica **o que é e como usar**; este arquivo
guarda o que não cabe lá.

Atualizado junto com os commits: toda mudança relevante ajusta as seções
afetadas e ganha uma entrada no **Histórico** (mais recente primeiro).

---

## De onde veio a ideia

O projeto mudou de forma três vezes antes da primeira linha de código:

1. **RTS no estilo Age of Empires 1.** Ponto de partida. Descartado pelo
   escopo: combate, IA inimiga, pathfinding de batalha e multiplayer são
   projetos inteiros cada um.
2. **Cadeias de produção no estilo Knights and Merchants.** A economia
   com carregadores levando cada item pelas estradas virou o centro.
3. **City builder sem adversários, no estilo Cities: Skylines, na Hansa
   medieval.** Formato final: a dificuldade vem da economia, do clima e da
   história real, não de um inimigo.

O código do AoE1 e do KaM não é aberto. Os projetos abertos usados como
**referência de leitura** (sem copiar código, para não herdar a GPL/AGPL)
são o Widelands (transporte no estilo Settlers II) e o KaM Remake.

## Restrições

| Restrição | Consequência |
|---|---|
| Projeto de portfólio, lido por avaliadores | Commits pequenos que explicam o porquê; README e este arquivo legíveis para quem chega sem contexto. |
| Repositório **privado** (`araujovito/hansestadt`) | Pode virar público com `gh repo edit --visibility public`; nada aqui deve depender de ser privado. |
| Sem arte própria | Formas geométricas provisórias. Candidato a substituir: pacote Medieval RTS da Kenney (CC0). |
| raylib fora do apt da distribuição | Baixada por script com versão e SHA-256 fixos, compilada em `vendor/` (fora do git). Mesmo esquema para o toml++. |
| Pasta de trabalho com espaço no nome (`~/jogos /hansestadt`) | O Makefile usa só caminhos relativos; o make não lida com espaços. |

## Decisões e por quê

Decisões de produto (fases, economia, população) estão no README. Aqui ficam
as de implementação, que o README não explica.

### Arquitetura
- **A lógica não inclui raylib.** Cidade, conteúdo, produção, rotas e
  transporte compilam e são testados sem janela. A lista `LOGICA` do
  Makefile é ao mesmo tempo o alvo de testes e a regra: se um desses
  arquivos incluir `raylib.h`, `make test` quebra.
- **Tudo roda em ticks fixos (20/s) e em ordem de id.** A velocidade 1×/2×/4×
  muda quantos ticks rodam por quadro, nunca o tamanho do tick. Há um teste
  que confere que a mesma partida dá o mesmo resultado.
- **Entidades referenciadas por índice, nunca por ponteiro.** Id de edifício
  não é reaproveitado após demolição: um carregador com destino 7 encontra
  o edifício 7 marcado como demolido, e não um edifício novo no mesmo lugar.
- **Progresso de produção em inteiros** (milésimos do fator do mês), para
  não acumular erro de ponto flutuante ao longo de anos de jogo.
- **Ruído do mapa com hash próprio** (finalizador do murmur3), e não `rand()`
  ou `<random>`, cujas distribuições variam entre compiladores.
- **Calendário juliano** (bissexto a cada 4 anos, inclusive 1300): datas das
  fontes entram sem conversão.

### Construção e rotas
- **Porta fixa** em cada edifício: a célula abaixo do meio da borda inferior
  da área total. Um ponto único dá às rotas um início claro e ao jogador um
  lugar visível para ligar a estrada.
- **Carregadores andam só na estrada.** Andar fora dela, mais devagar, foi
  considerado e deixado de lado: daria menos peso ao traçado da vila. Decisão
  tomada por Claude na ausência de resposta; pode ser revista.
- **Cache de rotas invalidado por inteiro** a cada estrada construída ou
  demolida. Descobrir quais rotas foram afetadas seria mais econômico, mas
  obras são raras perto do número de viagens e uma estrada nova pode criar
  atalho para rotas que nem passavam ali.

### Transporte
- **Planejamento a cada 10 ticks**, não a cada tick: estoques mudam na escala
  de dias, e planejar mais vezes só multiplicaria buscas.
- **Demandas antes de sobras.** A farinha vai direto do moinho à padaria
  quando há vaga, sem passar pelo armazém.
- **Reservas** (`reservado` e `a_caminho`) separadas do estoque físico: só o
  planejamento olha para elas.
- **Carga com destino perdido vai para o depósito**; só se perde quando não
  há depósito alcançável.

### Conteúdo
- **TOML lido com toml++**: um cabeçalho só, TOML 1.0 completo, erros com
  linha e coluna. Só `conteudo.cpp` o inclui (são 18 mil linhas).
- **Chave desconhecida é erro, não aviso**: `trabalhadore = 2` seria
  ignorada em silêncio.
- **Testes usam conteúdo próprio**, escrito no teste, para não quebrarem com
  mudanças de balanceamento.

### Convenções de código
- Identificadores em português, como nos outros projetos do portfólio.
- C++20, `-Wall -Wextra -Wpedantic -Wshadow` sem avisos; build de depuração
  com AddressSanitizer e UBSan.
- Mini-framework de testes próprio (`testes/verificar.hpp`), em vez de
  Catch2/doctest, enquanto os testes forem simples.

## O que foi tentado e descartado

| Tentativa | Por que saiu |
|---|---|
| Godot | Protótipo mais rápido, mas GDScript limita uma simulação com milhares de agentes. |
| Rust + Bevy | Custo alto de refatorar com o design ainda mudando; o Bevy muda muito entre versões. |
| C puro | Sem containers genéricos para listas de edifícios e itens. |
| Campos pintados pelo jogador (Banished, Manor Lords) | Trocado por anel fixo em volta da fazenda: mais simples de jogar e programar. |
| Cadeia do tijolo no primeiro marco | O gótico de tijolo começa no Báltico a partir de ~1160; uma vila de 1150 é de madeira. O primeiro marco virou a cadeia de comida. |
| Contador de portas por célula | Dois edifícios nunca dividem porta (a célula acima dela é sempre do próprio edifício); virou uma marca booleana. |
| Tipo de item `material` | Colidia com o tipo global `Material` da raylib; virou `insumo`. |

## Problemas conhecidos e limitações

- **Os 12 carregadores do armazém surgem do nada**, fora da população.
- **Os 6 moradores iniciais ainda não nascem nem envelhecem**; nesta etapa
  apenas procuram vagas nas casas.
- **Faixa de floresta na borda esquerda do mapa** é coincidência da semente
  1150, não defeito do ruído (média do hash por coluna conferida). Some
  quando o mapa da campanha for desenhado à mão.
- **Um ciclo da fazenda começado no fim de agosto fica pausado até o julho
  seguinte** (fator 0 de setembro a junho). Aceitável por ora.

## Em aberto

- **De onde vêm os carregadores?** Com a vila começando em 3 casais, faz
  sentido saírem da população, mas aí quase todos seriam carregadores no
  início. Decidir antes de implementar os moradores.
- Formato dos eventos históricos: TOML ou Lua (sol2 está sem versão nova
  desde 2022).
- Quando trocar a arte provisória pelos sprites da Kenney.

## Próximos passos

Tarefa 10, nesta ordem (decisões do Vitor em 2026-10-01: 3 casais que se
reproduzem, jogador escolhe onde fundar, fome mata, balanceamento em Python
agora, estamentos depois):

1. ~~Moradores nas casas e população na barra do topo.~~ Concluído.
2. ~~Trabalhadores limitam a produção.~~ Concluído.
3. ~~Casas pedem comida pelo sistema de pedidos; consumo diário.~~ Concluído.
4. ~~Fome mata.~~ Concluído: 30 dias consecutivos sem comida.
5. ~~Início da partida: jogador posiciona o primeiro armazém; estoque
   inicial.~~ Concluído, com 480 porções de pão após o balanceamento.
6. ~~Ferramenta de balanceamento em Python (simula anos sem janela, plota
   produção × consumo por mês) e ajuste do TOML.~~ Concluído.
7. ~~Interface de acompanhamento: estoque total, "comida para N dias",
   avisos.~~ Concluído.
8. ~~Teste do marco: a vila atravessa o inverno 1150–51 com a configuração
   certa e passa fome sem a fazenda.~~ Concluído.

**Primeiro marco concluído.** Salvar/carregar também está pronto. Próxima
ordem: lenhador desmatando, crescimento populacional, carregadores vindos da
população, eventos históricos, evolução de fase, custos de construção, arte,
estamentos e mapa histórico.

## Como trabalhamos

- Commits pequenos, um assunto por commit, mensagem com o porquê; **cada
  commit compila sem avisos e passa nos testes** (conferido com um worktree
  temporário percorrendo os commits novos).
- Sem coautoria nos commits.
- Mudanças visuais conferidas com captura de tela (programa auxiliar fora do
  repositório que renderiza uma cena e chama `TakeScreenshot`).
- Este arquivo é atualizado nos commits que mudam decisões, pendências,
  limitações ou próximos passos.

---

## Histórico

### 2026-10-03

- **Salvar/carregar**: `F5` grava mapa, calendário, edifícios, estoques,
  moradores e carregadores num arquivo binário versionado; `F9` restaura e
  a simulação continua deterministicamente. 93 testes.
- **Primeiro marco jogável**: teste integrado com conteúdo e distâncias reais
  atravessa o inverno de 1150–51 com a cadeia completa e registra fome sem a
  fazenda. O teste revelou o gargalo logístico; carregadores passaram a andar
  uma célula por tick e o armazém passou a abrigar 12. 92 testes.
- **Indicadores da vila**: a barra ganhou uma segunda linha para população,
  moradia, emprego, mortes, comida total e autonomia; a cor avisa abaixo de
  90 e 30 dias. 90 testes.
- **Balanceamento agregado**: `make balance` lê os TOMLs, simula a economia
  diária e gera CSV e SVG mensais. O cenário-base de três anos produz 6.804
  porções para 6.222 consumidas, sem falta; padaria, pesca, pocilga, estoques
  e provisões iniciais foram ajustados a partir do resultado. A confirmação
  com distâncias e transporte fica para o teste integrado.
- **Fundação escolhida pelo jogador**: o tempo aguarda o posicionamento do
  primeiro armazém, que recebe provisões e libera a barra normal
  de construção. 89 testes.
- **Morte por fome**: cada morador acumula dias sem alimentação e morre ao
  completar 30; a morte libera casa e emprego e aparece na barra. 88 testes.
- **Alimentação das casas**: despensa compartilhada entre os tipos de comida,
  pedidos atendidos pelos carregadores e consumo diário por morador. A falta
  já é contada individualmente, preparando as mortes por fome. 87 testes.
- **Trabalho limita a produção**: moradores alojados ocupam empregos em
  ordem determinística; edifícios sem a equipe completa param e explicam o
  motivo na interface. 84 testes.
- **População nas casas**: a simulação começa com 3 casais, moradores ocupam
  vagas livres, voltam a ficar sem teto após demolição e aparecem na barra
  superior e no painel da casa. 82 testes.

### 2026-10-01

- **Decisões de população** (`e3a752e`): 3 casais, jogador escolhe a
  fundação, fome mata, balanceamento em Python agora, estamentos depois.
- **Tarefa 9, carregadores e pedidos** (`44085a3`..`aa42048`): carregadores
  no depósito, reservas, planejamento de demandas e sobras, demolição no meio
  da viagem, integração e desenho. O AddressSanitizer pegou um acesso a
  vetor vazio num auxiliar de teste; um cenário de teste mal montado foi
  refeito. 79 testes.
- **Tarefa 8, rotas** (`37be3ae`..`18ab9e1`): portas, A*, cache por versão
  das estradas, alerta de edifício sem acesso.
- **Tarefa 7, produção** (`b8c6a98`..`85b52c9`): simulação dona da cidade,
  estoque por edifício, ciclo com sazonalidade, painel do edifício.
- **Ramos da economia e estações** (`b0c5817`); **tarefa 6, conteúdo em
  TOML** (`31ad561`..`956dd1f`): toml++, itens e edifícios em dados,
  regras `requer` e `campos`, lavoura que muda de cor com o mês.
- **Tarefa 5, construção** (`a8ac2cd`..`2959b92`): catálogo, ocupação,
  estradas em L, demolição, testes automáticos.
- **Tarefa 4, mapa** (`61c5c3f`..`c7e4c86`): costa báltica gerada por
  semente, câmera com zoom em direção ao cursor.
- **Tarefas 1 e 3** (`114184f`..`fbd6d81`): concepção, escolha de C++20 +
  raylib e calendário híbrido, build com raylib por script, loop de passo
  fixo e calendário juliano.
