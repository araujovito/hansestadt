# Hansestadt

Jogo de construção de cidade (*city builder*) ambientado na região da Liga
Hanseática, entre a fundação de uma vila na colonização do leste (~1150) e o
auge da Hansa (~1370). Não há adversários: a dificuldade vem da economia, do
clima e dos acontecimentos históricos.

> Status: concepção. Ainda não há código jogável.

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

## Tecnologia

| Parte | Escolha | Motivo |
|---|---|---|
| Simulação e visual | C++20 + [raylib](https://www.raylib.com/) 5.5 | O custo do jogo está na simulação (milhares de carregadores, pedidos e rotas por tick), não no visual 2D em grade; C++ dá controle de memória sem coletor de lixo, e a raylib cobre janela, sprites e entrada sem trazer um motor inteiro |
| Conteúdo (edifícios, receitas, eventos) | TOML | Balancear sem recompilar, com o histórico de ajustes visível no git |
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
make run          # baixa e compila a raylib na primeira vez, depois roda
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
| Soltar a ferramenta | `Esc` |
