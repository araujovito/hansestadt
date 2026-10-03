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

### Desempenho
- **Medir antes de otimizar, e algoritmo antes de assembly.** Em 2026-10-03
  o Vitor perguntou se valia escrever partes em assembly. A medição (abaixo,
  em Problemas conhecidos) mostrou que o custo vinha do *número* de buscas e
  contagens, que crescia mais rápido que a cidade, e não da velocidade de
  cada uma. Assembly deixaria cada busca talvez 1,5–2× mais rápida sem mudar
  essa curva, e ficaria fora do alcance do ASan/UBSan e preso a x86-64.
  Decisão: corrigir o algoritmo e medir de novo; SIMD ou assembly só se um
  laço medido continuar dominando depois disso.
- **Como medir**: programa auxiliar fora do repositório que monta vilas
  sintéticas em blocos de 24×16 células (armazém, 2 fazendas, moinho,
  padaria, pescador, lenhador, pocilga, 5 casas, 20 moradores por bloco),
  roda um ano de jogo sem janela e cronometra cada `Simulacao::passo()`.
  Uma assinatura do estado final (população, cargas perdidas, soma dos
  estoques, entregas) confere que uma otimização não mudou o resultado.

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

- **Os 6 moradores iniciais ainda não nascem nem envelhecem**; nesta etapa
  apenas procuram vagas nas casas.
- **Faixa de floresta na borda esquerda do mapa** é coincidência da semente
  1150, não defeito do ruído (média do hash por coluna conferida). Some
  quando o mapa da campanha for desenhado à mão.
- **Um ciclo da fazenda começado no fim de agosto fica pausado até o julho
  seguinte** (fator 0 de setembro a junho). Aceitável por ora.

- **Desempenho da simulação: folga até o mapa 128×128 cheio.** Tempo médio
  de um passo (um quadro a 60 fps tem 16,7 ms; na velocidade 4× entra cerca
  de 1,3 passo por quadro). Mesmo cenário e mesma assinatura do estado final
  em todas as versões:

  | Blocos | Edifícios | Moradores | `b4435ee` | `86e01ae` | agora | p99 agora |
  |---|---|---|---|---|---|---|
  | 1 | 13 | 20 | 0,004 ms | 0,002 ms | 0,002 ms | 0,006 ms |
  | 4 | 52 | 80 | 0,09 ms | 0,006 ms | 0,006 ms | 0,07 ms |
  | 20 | 260 | 400 | 9,7 ms | 0,06 ms | 0,024 ms | 0,3 ms |
  | 40 | 520 | 800 | 79 ms | 0,21 ms | 0,08 ms | 1,0 ms |
  | 160 (mapa 256×256) | 2.080 | 3.200 | não medido* | 4,1 ms | 1,6 ms | 18 ms |

  \* Na base, um ano com 40 blocos levou 9,6 minutos para simular; 160
  blocos levariam horas.

  Com 160 blocos um passo em cada cem ainda leva cerca de um quadro inteiro
  (pior caso 73 ms, nos ticks em que planejamento e virada do dia
  coincidem). O tempo está dividido, sem um ponto dominante: consultas de
  estoque em `origem_mais_proxima` (22%; todo depósito entra na lista de
  todo item), buscas em largura (18%) e A* para os caminhos (18%). Saídas
  possíveis, se o mapa crescer além de 128×128: indexar só quem tem o item
  disponível no momento, guardar as distâncias por porta, ou planejar uma
  fatia dos pedidos por tick.

  O desenho não é gargalo: 40 blocos no zoom mínimo levam 6,8 ms por quadro
  (Radeon 680M).

## Em aberto

- As decisões ainda abertas e sua ordem de resolução ficam no
  [`plano_de_ação.md`](plano_de_ação.md). Este arquivo registra o motivo das
  decisões depois que forem tomadas.

## Próximos passos

O primeiro marco e suas extensões estão concluídos. A fila oficial, incluindo
o plano de produto e as salvaguardas de desempenho, fica em
[`plano_de_ação.md`](plano_de_ação.md). Ela não é duplicada aqui para os dois
documentos não voltarem a divergir.

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

- **Cadeia da cerveja**: adicionados cevada, malte e cerveja e as oficinas
  de cultivo, maltagem e produção de cerveja na fase Mercado. A cerveja ainda
  não participa das necessidades da população ou de comércio. Saves antigos
  com menos itens ganham slots vazios no fim dos vetores, sem mudar IDs
  existentes. Teste de carga de conteúdo e migração incluídos.
- **Pecuária e ofícios**: curral e ovelheiro abastecem carne, pele e lã;
  tecelagem e curtume completam tecido e couro. Rebanhos/pastagem estão
  agregados, sem ração nem esgotamento do terreno nesta etapa.
- **Pesca e conservação**: salina abstrata, pesca de arenque, tanoeiro e
  salgadeira completam arenque + sal + barril → alimento conservado. A pesca
  ainda não distingue água doce de mar, e o sal ainda não usa recurso do mapa.

### 2026-10-03

- **Núcleo da progressão histórica**: `campanha.cpp` define os anos-limite
  1150, 1200, 1250, 1300 e 1370 e permite avançar uma etapa quando todas as
  instituições disponíveis informadas estão no nível máximo. O calendário
  não é alterado. A condição está ligada às instituições da cidade, com
  níveis e custos definidos em `dados/edificios.toml`; a fase e os níveis
  são preservados no salvamento v4, que também aceita arquivos v3 iniciando
  a fase em Vila. A melhoria provisória pode ser acionada com U sobre uma
  instituição selecionada; a apresentação no painel ainda precisa evoluir.
- **Primeiro corte de eventos históricos**: `dados/eventos.toml` define
  intervalos julianos inclusivos e multiplicadores combináveis por edifício
  ou gerais. A Grande Fome reduz colheitas e pesca; a Peste Negra reduz a
  produção em geral. O efeito incide diariamente na produção e é reaplicado
  ao carregar uma partida, sem mudar o formato do salvamento. Ainda não há
  mortalidade nem registro visual; a interface/histórico fica com a tarefa
  paralela, e efeitos demográficos ficam para uma extensão posterior.

- **Custos de construção**: edifícios declaram materiais no TOML; a cidade
  valida e consome estoques disponíveis sem violar reservas do transporte.
  A fundação traz 40 toras e a interface mostra custo ou material ausente.
  107 testes.
- **Carregadores na população**: a vaga do armazém ocupa um adulto e disputa
  mão de obra com a produção. Um carregador leva até 8 unidades por viagem;
  morte ou perda do emprego remove o agente e libera reservas. O salvamento
  passou para a versão 3.
- **Crescimento populacional**: moradores têm idade e sexo, tornam-se aptos
  ao trabalho aos 16 anos e casais alojados com comida e vaga têm uma criança
  no primeiro dia do ano. O salvamento passou para a versão 2.
- **Índices de busca** (`2792449`, `cd0d1bc`): `alojar` conta os moradores de
  cada casa uma vez (o pico de 5,7 s no primeiro passo com 160 blocos vinha
  daqui, e não do cenário construir tudo de uma vez) e o transporte mantém
  por item a lista de quem pode cedê-lo. Com 160 blocos o passo médio caiu
  de 4,1 ms para 1,6 ms. 102 testes.
- **Correções de desempenho** (`fdb83e5`..`054f8c5`): empregos distribuídos
  numa passada só, contagem de trabalhadores guardada por edifício e busca em
  largura (`DistanciasPelaEstrada`) no lugar de um A* por candidato no
  planejamento do transporte. Com 40 blocos o passo médio caiu de 79 ms
  para 0,21 ms, com o mesmo estado final. 100 testes.
- **Medição de desempenho**: vilas sintéticas de 1 a 160 blocos, um ano de
  jogo, gprof. O tempo do passo cresce bem mais rápido que a cidade (9,7 ms
  com 20 blocos); as causas são a contagem de trabalhadores e o A* por
  carregador. O desenho tem folga. Assembly descartado por ora.
- **Desmatamento**: `extrai` tornou o consumo de terreno uma regra de dados;
  cada ciclo do lenhador converte uma célula de floresta próxima em campo e
  a produção para quando o recurso acaba. O mapa alterado já é preservado
  pelo salvamento. 95 testes.
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
  diária e gera CSV e SVG mensais. O cenário-base do primeiro ano produz
  1.864 porções para 1.836 consumidas, sem falta; padaria, pesca, estoques
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
