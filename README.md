# Hansestadt

Jogo de construção de cidade (*city builder*) ambientado na região da Liga
Hanseática, entre a fundação de uma vila na colonização do leste (~1150) e o
auge da Hansa (~1370). Não há adversários: a dificuldade vem da economia, do
clima e dos acontecimentos históricos.

> Decisões de implementação, coisas descartadas, pendências e histórico
> ficam em [`contexto.md`](contexto.md).

> Status: protótipo. Dá para construir a vila, e os edifícios produzem
> e trocam mercadorias por carregadores. A vila começa com 3 casais, que
> ocupam as casas e trabalham nos edifícios; ainda não há consumo nem eventos.

## Ideia central

- **Cadeias de produção** no estilo *Knights and Merchants*: matéria-prima →
  oficina → produto, com carregadores levando cada item pelas estradas.
- **Construção em grade** e crescimento da cidade no estilo *Cities: Skylines*,
  sem combate.
- **Campanha histórica**: os edifícios principais (igreja, governo, porto)
  evoluem em estágios, e cada estágio libera uma fase da história com eventos
  em datas reais.

## Por que a Hansa

A planície do norte da Alemanha quase não tem pedra natural para construir.
As cidades hanseáticas foram erguidas em tijolo cozido (*Backsteingotik*), o
que leva a uma cadeia de produção própria (argila + combustível → olaria →
tijolo), diferente da pedreira dos jogos medievais mais comuns.

## Fases planejadas

| Fase | Período | Marco histórico |
|---|---|---|
| Vila | ~1150 | *Ostsiedlung*: um *Lokator* funda a vila |
| Mercado | ~1200 | *Marktrecht* |
| Cidade | ~1250 | *Stadtrecht* (Direito de Lübeck) e entrada na Hansa |
| Cidade hanseática | 1300–1350 | Grande Fome (1315–17), Peste Negra (1347–51) |
| Auge | ~1370 | Paz de Stralsund |

## Economia

Quatro ramos, liberados aos poucos pelas fases:

| Ramo | Vila (~1150) | Mercado (~1200) | Cidade e Hansa (~1250+) |
|---|---|---|---|
| Construção | lenhador, toras | carvoaria, turfa | poço de argila, olaria (tijolo) |
| Agricultura | fazenda de centeio e aveia, moinho d'água, padaria | cervejaria (cevada) | lúpulo, linho |
| Pecuária | pocilga (porcos na floresta) | gado, ovelhas, açougue, tecelão | curtume |
| Pesca | pescador de rio | | pescador de mar (arenque), tanoeiro, salgadeira |

Duas regras de projeto:

- **Campos fixos em volta da fazenda**, como em *Knights and Merchants*.
  O jogador posiciona a fazenda, e ela ocupa a área de cultivo junto. É
  mais simples de jogar e de programar do que campos pintados à mão, e a
  rotação de três campos (*Dreifelderwirtschaft*) fica interna à fazenda.
- **As estações afetam a produção**: o grão é semeado no outono (centeio)
  e na primavera (aveia, cevada) e colhido no verão, nada cresce no
  inverno, e a pesca rende menos no frio. É o que dá peso histórico à
  Grande Fome de 1315–17, causada por anos seguidos de chuva que
  arruinaram as colheitas.

O tijolo fica para a fase de cidade porque, historicamente, o gótico de
tijolo começa no Báltico a partir da década de 1160 (catedral de Lübeck,
1173). Uma vila recém-fundada era de madeira.

## População (planejado)

Decisões tomadas para o primeiro marco jogável:

- **A vila começa com 3 casais.** É uma fundação pequena de propósito: a
  população cresce por **nascimentos**, e cada nova boca pesa no estoque
  de comida antes de virar mão de obra.
- **O jogador escolhe onde fundar a vila**, posicionando o primeiro
  armazém. Escolher o lugar (perto do rio, da argila, da floresta) é a
  primeira decisão estratégica da Ostsiedlung.
- **A fome mata.** Sem comida, moradores morrem. É o que dá peso ao
  inverno e, mais adiante, à Grande Fome de 1315–17.
- **O balanceamento é feito com uma ferramenta em Python** que simula anos
  inteiros sem janela e plota produção e consumo por mês. Os números do
  TOML são ajustados por ela, não no chute.
- **Estamentos ficam para depois.** A divisão da sociedade medieval
  (camponeses, artesãos, mercadores, clero) muda as necessidades e a
  economia, e entra como um passo próprio depois do primeiro marco.

Os moradores já existem como entidades da simulação. Eles ocupam as vagas
das casas em ordem de construção e ficam sem teto se a moradia for demolida;
a barra superior mostra a população total, os sem-casa e os desempregados.
Só moradores alojados trabalham. Os empregos são preenchidos na ordem de
construção dos edifícios, e uma oficina só produz com a equipe completa.

## Tecnologia

| Parte | Escolha | Motivo |
|---|---|---|
| Simulação e visual | C++20 + [raylib](https://www.raylib.com/) 5.5 | O custo do jogo está na simulação (milhares de carregadores, pedidos e rotas por tick), não no visual 2D em grade; C++ dá controle de memória sem coletor de lixo, e a raylib cobre janela, sprites e entrada sem trazer um motor inteiro |
| Conteúdo (edifícios, receitas, eventos) | TOML, lido com [toml++](https://github.com/marzer/tomlplusplus) 3.4 | Balancear sem recompilar, com o histórico de ajustes visível no git |
| Scripts de eventos | Lua (planejado) | Regras históricas fora do código C++ |
| Ferramentas | Python | Simular e plotar a economia fora do jogo |

Alternativas consideradas: **Godot**, que dá o protótipo mais rápido, mas
o GDScript limita a simulação com muitos agentes; **Rust**, que tem memória
segura, mas pesa na fase de design instável de uma simulação com muitas
referências cruzadas; e **C puro**, que não tem containers genéricos para
listas de edifícios e itens.

Regras para conter o risco de memória do C++: nada de `new`/`delete` direto,
entidades referenciadas por índice/ID em vez de ponteiro, e build de
depuração com AddressSanitizer e UBSan.

## Conteúdo

Itens e edifícios ficam em [`dados/itens.toml`](dados/itens.toml) e
[`dados/edificios.toml`](dados/edificios.toml), com o formato documentado
no cabeçalho do arquivo. Cada edifício declara tamanho, terreno, fase em
que é liberado, trabalhadores, estoque, receitas (entradas, saídas,
duração em dias) e um fator de produção para cada mês. Regras de
posicionamento também são dados: `requer` (terreno próximo, como água
para o moinho) e `campos` (o anel de lavoura da fazenda).

O jogo valida os arquivos ao abrir e lista todos os erros encontrados
(item inexistente, chave escrita errada, sazonalidade sem 12 meses) em
vez de abrir com conteúdo pela metade.

## Como a economia funciona

1. **Produção** (`src/producao.cpp`). Um edifício começa a primeira
   receita que tem entradas no estoque, espaço para as saídas e fator
   maior que zero no mês. O ritmo do ciclo é multiplicado pelo fator do
   mês: a fazenda só colhe em julho e agosto, e o moinho perde força com
   o rio congelado.
2. **Rotas** (`src/rotas.cpp`). Carregadores andam só por estrada. O
   caminho é o A* mais curto entre as portas, guardado em cache até a
   próxima mudança nas estradas. Cada edifício tem uma porta (a célula
   abaixo do meio da borda inferior), e sem estrada nela ele fica isolado.
3. **Transporte** (`src/transporte.cpp`). Os carregadores moram no
   armazém. A cada 10 ticks, cada vaga na entrada de um produtor pede um
   item à origem mais próxima, e depois o que sobra nas saídas vai para o
   depósito. Item e vaga são reservados na hora da atribuição. Se a
   origem é demolida, o carregador desiste. Se o destino some ou lota, a
   carga vai para o depósito.

Tudo roda em ticks fixos e em ordem de id. A mesma partida dá o mesmo
resultado em qualquer velocidade, e há um teste que confere isso.

## Calendário

Modelo **híbrido**. Dentro de uma fase o ano corre e os eventos disparam nas
datas reais. A fase seguinte só começa quando os edifícios principais
evoluem. Assim a Grande Fome de 1315 chega estando a cidade preparada ou não,
mas uma vila de madeira não enfrenta a Peste de 1347, que pertence a uma fase
posterior.

## Compilar e rodar

Requisitos (Debian/Ubuntu):

```sh
sudo apt install g++ make curl libx11-dev libgl1-mesa-dev libxrandr-dev \
    libxinerama-dev libxcursor-dev libxi-dev libasound2-dev
```

```sh
make run          # baixa raylib e toml++ na primeira vez, depois roda
make debug        # build com AddressSanitizer e UBSan
make test         # testes da lógica, sem janela
```

Controles:

| Ação | Comando |
|---|---|
| Mover a câmera | `WASD`, setas, ou arrastar com o botão do meio/direito |
| Zoom | roda do mouse (em direção ao cursor) |
| Pausar / velocidade | `Espaço` / `1` `2` `3` (1×, 2×, 4×) |
| Estrada | `R`, depois clicar e arrastar; constrói ao soltar |
| Edifício | botão na barra inferior, depois clicar no mapa |
| Demolir | `X`, depois clicar ou arrastar |
| Ver produção, estoque e acesso | clicar num edifício (sem ferramenta ativa) |
| Soltar a ferramenta / fechar o painel | `Esc` |
