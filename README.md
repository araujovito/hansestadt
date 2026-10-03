# Hansestadt

Jogo de construção de cidade (*city builder*) ambientado na região da Liga
Hanseática, entre a fundação de uma vila na colonização do leste (~1150) e o
auge da Hansa (~1370). Não há adversários: a dificuldade vem da economia, do
clima e dos acontecimentos históricos.

> Decisões de implementação, coisas descartadas, pendências e histórico
> ficam em [`contexto.md`](contexto.md).
> O roteiro único do trabalho futuro fica em
> [`plano_de_ação.md`](plano_de_ação.md).

> Status: primeiro marco jogável com o primeiro corte do sistema de eventos.
> A Grande Fome (1315–17) reduz a produção agrícola e pesqueira; a Peste
> Negra (1347–51) reduz a produção geral. Os eventos ficam em
> `dados/eventos.toml` e ainda não alteram diretamente a população.

## Ideia central

- **Cadeias de produção** no estilo *Knights and Merchants*: matéria-prima →
  oficina → produto, com carregadores levando cada item pelas estradas.
- **Construção em grade** e crescimento da cidade no estilo *Cities: Skylines*,
  sem combate.
- **Campanha histórica em um mapa contínuo**: as fases não reiniciam a cidade.
  Cada uma exige alcançar seu ano histórico e evoluir as instituições
  necessárias ao nível máximo disponível; o calendário continua correndo
  mesmo se os objetivos ainda não foram cumpridos.

## Por que a Hansa

A planície do norte da Alemanha quase não tem pedra natural para construir.
As cidades hanseáticas foram erguidas em tijolo cozido (*Backsteingotik*), o
que leva a uma cadeia de produção própria (argila + combustível → olaria →
tijolo), diferente da pedreira dos jogos medievais mais comuns.

## Fases planejadas

| Fase | Período | Marco histórico |
|---|---|---|
| Vila | 1150 | *Ostsiedlung*: um *Lokator* funda a vila |
| Mercado | 1200 | *Marktrecht* |
| Cidade | 1250 | *Stadtrecht* (Direito de Lübeck) e entrada na Hansa |
| Cidade hanseática | 1300 | Grande Fome (1315–17), Peste Negra (1347–51) |
| Auge | 1370 | Paz de Stralsund |

As fases ocorrem no mesmo mapa. Cada marco requer alcançar o ano indicado e
ter todas as instituições disponíveis na etapa no nível máximo; o calendário
segue avançando enquanto esses requisitos não forem cumpridos. As instituições
são únicas: igreja e governo começam na Vila, mercado abre no Mercado e porto
na Hansa. Selecione uma instituição e pressione **U** para melhorá-la; os
custos e níveis disponíveis por fase estão em `dados/edificios.toml`. A
apresentação dos níveis no painel ainda é provisória.

## Economia

Quatro ramos, liberados aos poucos pelas fases:

| Ramo | Vila (~1150) | Mercado (~1200) | Cidade e Hansa (~1250+) |
|---|---|---|---|
| Construção | lenhador, toras | carvoaria, turfa | poço de argila, olaria (tijolo) |
| Agricultura | fazenda de centeio, moinho d'água, padaria | cevada → malte → cerveja | lúpulo, linho |
| Pecuária e ofícios | pocilga (porcos na floresta) | curral (carne e pele), ovelheiro (lã e carne) | tecelagem (lã → tecido), curtume (pele → couro) |
| Pesca e conservação | pescador de rio | salina (sal) | pesca de arenque → tanoeiro (barril) → salgadeira (arenque salgado) |

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

Os moradores existem como entidades da simulação. Eles ocupam as vagas
das casas em ordem de construção e ficam sem teto se a moradia for demolida;
a barra superior mostra a população total, os sem-casa e os desempregados.
Só moradores alojados trabalham. Os empregos são preenchidos na ordem de
construção dos edifícios, e uma oficina só produz com a equipe completa.
As casas têm uma despensa compartilhada, pedem qualquer alimento disponível
pelo mesmo sistema de transporte das oficinas e consomem uma unidade por
morador a cada dia. Após 30 dias consecutivos sem comer, o morador morre e
libera sua vaga na casa e no trabalho.

Cada morador tem idade e sexo. Crianças consomem comida, mas só entram no
mercado de trabalho aos 16 anos. No primeiro dia de cada ano, uma casa com
um casal adulto, comida na despensa e vaga disponível recebe uma criança.

A segunda linha da barra superior resume população, crianças, sem-casa, desempregados,
mortes, estoque total de comida e autonomia em dias. A autonomia fica
amarela abaixo de 90 dias e vermelha abaixo de 30.

A partida começa pausada no tempo, com apenas o armazém disponível. Ao
posicioná-lo, os colonos descarregam 600 porções de pão e o restante da barra
de construção é liberado.

## Balanceamento

`make balance` roda o primeiro ano da economia sem abrir a janela, lendo as mesmas
receitas e sazonalidades dos TOMLs. O cenário-base usa os 6 moradores e uma
fazenda, um moinho, uma padaria, um pescador e uma pocilga. O resultado fica
em `build/balanceamento/economia.csv`, com totais mensais, e
`economia.svg`, com produção e consumo acumulados.

O cenário atual produz 1.864 porções, consome 1.836 e não registra falta
no primeiro ano. A simulação é deliberadamente agregada: transporte, distância
e limites físicos também são cobertos pelo teste integrado do marco. Com a
cadeia completa, os seis moradores atravessam o inverno de 1150–51; sem a
fazenda, há refeições perdidas.

## Tecnologia

| Parte | Escolha | Motivo |
|---|---|---|
| Simulação e visual | C++20 + [raylib](https://www.raylib.com/) 5.5 | O custo do jogo está na simulação (milhares de carregadores, pedidos e rotas por tick), não no visual 2D em grade; C++ dá controle de memória sem coletor de lixo, e a raylib cobre janela, sprites e entrada sem trazer um motor inteiro |
| Conteúdo (edifícios, receitas, eventos) | TOML, lido com [toml++](https://github.com/marzer/tomlplusplus) 3.4 | Balancear sem recompilar, com o histórico de ajustes visível no git |
| Scripts de eventos | Lua (avaliar depois) | Só adicionar se um evento exigir regras que o TOML não expresse |
| Ferramentas | Python | Simular e plotar a economia fora do jogo |

Os eventos estão em `dados/eventos.toml`: cada `[[evento]]` tem `id`,
`nome`, `descricao`, datas julianas inclusivas (`[ano, mês, dia]`) e
efeitos opcionais. `multiplicador_geral` afeta toda produção;
`producao` permite ajustar edifícios por id; `mortalidade_anual` aplica uma
chance diária determinística de morte enquanto o evento estiver ativo. A
Peste Negra usa 10% ao ano durante cinco anos (cerca de 41% acumulados); os
efeitos da Grande Fome reduzem colheitas e pesca, e mortes acontecem pelo
sistema normal de fome quando falta comida. O contador de mortes por evento
é persistido em saves versão 5; versões 3 e 4 continuam carregáveis. A
interface pode ler o histórico pelas APIs de eventos.

O percentual da peste é um parâmetro agregado de jogo, não uma previsão para
cada vila: estudos estimam mortalidade de 30–50% na Europa durante a pandemia
([estudo bioarqueológico de Londres](https://pmc.ncbi.nlm.nih.gov/articles/PMC3094018/)).
A Grande Fome de 1315–1317 também teve crise de mortalidade associada às
colheitas e às condições climáticas ([estudo hidroclimático](https://www.nature.com/articles/s43247-020-00016-3)).

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

Edifícios extratores podem declarar `extrai`: a cada ciclo, uma célula do
terreno exigido é consumida. O lenhador transforma gradualmente a floresta
próxima em campo ao produzir toras e para quando não resta mata ao alcance.

Cada edifício também pode declarar `custo` no TOML. A prévia mostra os
materiais necessários; a construção só é aceita quando há itens disponíveis
nos estoques da cidade e os consome em ordem de edifício, sem tomar cargas já
reservadas pelo transporte. A expedição inicial traz 40 toras para erguer o
primeiro conjunto de casas e oficinas.

As estradas podem ser construídas manualmente. Pela regra definida para a
próxima etapa, os trajetos diários de casa ao trabalho também formarão trilhas
que se tornam estradas pelo uso. Carregadores serão pessoas separadas dos
trabalhadores e poderão atender vários edifícios quando faltarem adultos.
Os detalhes de movimento e de distribuição de vagas ainda estão em decisão;
até serem implementados, vale o comportamento atual da simulação.

A cadeia inicial de cerveja já pode ser construída na fase Mercado:
fazenda de cevada (colheita de verão) → malteria → cervejaria. A cerveja é,
por enquanto, um insumo produzido e transportado; ainda não é consumida pelos
moradores nem negociada por um sistema de mercado. Os valores de produção e
custos continuam provisórios.

O primeiro corte de pecuária também está disponível: curral produz carne e
pele, ovelheiro produz lã e carne; na fase Cidade, tecelagem converte lã em
tecido e curtume converte pele em couro. Rebanho, pastagem e abate são
agregados por enquanto, sem consumo de ração ou esgotamento de pasto.

Os rendimentos de carne do curral e do ovelheiro foram ajustados para não
ficarem muito abaixo da pocilga por trabalhador; a pesca de arenque entrega
uma quantidade de comida próxima à pesca de rio. Isso é só um primeiro
alinhamento de produtividade: mão de obra, transporte e custos ainda precisam
ser reavaliados com as novas regras de carregadores.

Na fase Cidade, a salina fornece sal de forma abstrata. Na Hansa, o pescador
de arenque, o tanoeiro e a salgadeira fecham a cadeia de conservação:
arenque + sal + barril → arenque salgado, que conta como alimento. A salina
ainda não depende de um terreno de sal, e a pesca usa a água próxima sem
distinguir rio de mar; isso fica para o bloco de recursos e mapa histórico.

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
3. **Transporte** (`src/transporte.cpp`). O armazém emprega um carregador da
   população, que leva até 8 unidades por viagem. A cada 10 ticks, cada vaga na entrada de um produtor pede um
   item à origem mais próxima; casas fazem o mesmo para encher a despensa.
   Depois, o que sobra nas saídas vai para o depósito. Item e vaga são
   reservados na hora da atribuição. Se a
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
| Evoluir a instituição selecionada (provisório) | `U` |
| Relatório de emprego, comida, produção e transporte | `V` ou o botão `Relatório` no canto de cima (o jogo continua andando) |
| Ir até um edifício com problema | clicar em `ver >` no alerta, no canto esquerdo (cliques seguidos percorrem os edifícios do alerta) |
| Salvar / carregar rápido | `F5` / `F9` (`partida.hanse`) |
| Menu de pausa (continuar, salvar e carregar em 4 vagas, sair) | `Esc` sem ferramenta nem painel aberto, ou o botão `Menu` no canto de cima |
| Soltar a ferramenta / fechar o painel | `Esc` |
