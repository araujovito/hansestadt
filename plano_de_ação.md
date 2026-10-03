# Plano de ação

Este é o **plano oficial e a única fonte para o trabalho futuro** do
Hansestadt. O `contexto.md` preserva decisões, medições e histórico; quando
uma decisão de lá gerar trabalho, a tarefa correspondente entra aqui.

A ordem pode mudar se um teste integrado revelar uma dependência ou um
gargalo novo. Todo bloco termina com testes, build sem avisos, atualização
do README e do contexto e um ou mais commits pequenos.

## Marco atual concluído

1. ~~**Crescimento populacional**~~ — concluído
   - Idade e sexo por morador.
   - Nascimentos condicionados a casal, moradia e comida.
   - Crianças passam a trabalhar aos 16 anos.

2. ~~**Integrar carregadores à população**~~ — concluído
   - Carregadores deixam de surgir automaticamente no armazém.
   - Transporte disputa adultos com fazendas e oficinas.
   - Mortes e demolições liberam ou removem carregadores corretamente.

3. ~~**Custos de construção**~~ — concluído
   - Declarar custos de materiais no TOML.
   - Validar e consumir recursos ao construir.
   - Mostrar custos e faltas na interface.

## Próximos blocos, em ordem

4. **Eventos históricos e climáticos**
   - TOML implementado: Grande Fome, Peste Negra e inverno rigoroso de
     1322–1323; datas julianas inclusivas, modificadores de produção e
     mortalidade determinística da peste. A Grande Fome pode causar morte
     por falta de comida no sistema atual.
   - Ainda falta modelar efeitos demográficos de longo prazo, como fecundidade
     e recuperação após as crises.
   - API para a interface: `sim.eventos.ativos(sim.data)` e
     `sim.eventos.ocorridos(sim.data)`; a lista inclui definições com nome,
     descrição e intervalo.
   - Outros eventos de chuva e clima ainda precisam ser escolhidos/modelados.
   - Começar em TOML para datas e modificadores simples.
   - Só introduzir Lua se um evento real exigir lógica que o TOML não expresse.
   - ~~Registrar acontecimentos e seus efeitos para o jogador~~ — concluído
     na interface: aba História do relatório e aviso quando um evento começa.

5. **Evolução das fases**
   - Implementado na `codex/simulacao`: os cinco anos fixos e a condição de
     todas as instituições disponíveis no nível máximo; a fase alcançada é
     persistida.
   - A campanha acontece em um único mapa contínuo, sem reiniciar a cidade
     ao trocar de fase.
   - Cada fase tem um ano-limite histórico e só é alcançada quando o
     calendário chega a esse ano e as instituições exigidas estão no maior
     nível disponível.
   - O calendário continua avançando independentemente; chegar ao ano sem
     cumprir os requisitos mantém a fase atual até cumpri-los.
   - A condição considera todas as instituições disponíveis naquela fase,
     não casas, oficinas ou demais edifícios econômicos.
   - Marcos confirmados: Vila (1150), Mercado (1200), Cidade (1250), Hansa
     (1300) e Auge (1370).

6. **Edifícios institucionais**
   - Primeiro corte implementado: igreja, governo, mercado e porto são
     únicos; níveis, disponibilidade por fase e custos ficam no TOML.
   - ~~Painel com nível e custo~~ — concluído na interface: botão de
     melhoria no painel da instituição (U ficou como atalho) e aba Fase no
     relatório.

7. **Completar as cadeias econômicas**
   - Implementado primeiro corte dos ramos: cevada → malte → cerveja;
     gado/ovelhas → carne, pele e lã → tecido/couro; arenque + sal + barril
     → arenque salgado (alimento).
   - O combustível/argila/tijolo já tem cadeia de carvoaria/turfa → olaria.
   - Primeiro ajuste de produtividade: carne de gado/ovelha e arenque
     aproximados às taxas de comida existentes por trabalhador; `make balance`
     estima 1,50; 1,50; e 1,47 porções por trabalhador/dia, respectivamente.
     Rebalanceamento integrado de mão de obra, transporte e custos ainda pendente.
   - Cadeia completa de argila, combustível e tijolos.
   - Rebalancear mão de obra, transporte e custos após cada ramo.

7a. **Carregadores por edifício e trilhas** — decisão registrada; regras
     operacionais pendentes antes de implementar
   - Cada edifício deve ter um carregador separado de seus trabalhadores;
     quando faltarem adultos, um carregador atende vários edifícios; adultos
     que sobrarem aumentam essa cobertura até um por edifício.
   - O percurso casa–trabalho dos funcionários cria trilhas que, com o uso,
     viram estradas automaticamente. Estradas manuais continuam disponíveis.
   - Falta decidir: se o armazém mantém carregadores próprios; prioridade
     entre vagas de produção e transporte; limiares e desgaste das trilhas;
     velocidade em campo/trilha/estrada; se carregadores também criam trilhas;
     e se trilhas atravessam floresta.

7b. **Comida para a vila crescer** — em andamento na `codex/simulacao`
   - Motivo: a vila inicial produz no máximo 2.303 porções por ano (6,3
     pessoas) e passa fome no primeiro ano; e morador sem casa morre de fome
     com o armazém cheio (ver contexto.md, Problemas conhecidos).
   - Implementado: moradores sem casa comem no armazém ou na igreja; a igreja
     guarda comida e é abastecida pelos carregadores.
   - Implementado: criança (menos de 16 anos) come meia porção; autonomia,
     alertas e relatório consideram esse consumo.
   - Edifícios podem receber melhorias de tecnologia que aumentam a
     produtividade, para a comida acompanhar o crescimento da vila.
   - Falta decidir: melhoria por edifício ou para todos do mesmo tipo; quanto
     cada nível aumenta e quantos níveis existem; custo (material, tempo,
     trabalhadores) e se depende da fase; quais edifícios podem melhorar; e
     quando a comida é pouca, quem tem preferência (com ou sem casa).

8. **Agricultura, clima e recursos**
   - Plantio e colheita mais visíveis.
   - Esgotamento ou regeneração de recursos.
   - Efeitos sazonais e climáticos detalhados.

9. **Interface e experiência do jogador** — em andamento na `claude/interface`
   - ~~Menus de pausa, salvamento e carregamento~~ — concluído: menu pelo Esc
     ou botão, quatro vagas de partida com resumo, confirmações.
   - ~~Alertas clicáveis~~ — concluído: comida, sem casa, sem estrada, sem
     trabalhadores, sem insumo e saída cheia; o clique leva ao edifício.
   - ~~Visões de emprego, fome, produção e logística~~ — concluído: janela de
     relatório (tecla V) com quatro abas e gráfico da comida.
   - ~~Histórico de acontecimentos~~ — concluído: aba História no relatório
     (só eventos já iniciados, com período, dias restantes e efeitos) e aviso
     na barra quando um evento começa.
   - ~~Painel das instituições~~ — concluído: o painel da instituição mostra
     nível, materiais da próxima melhoria e botão de melhorar (U vira atalho);
     a aba Fase do relatório lista o que falta para a próxima fase.
   - Explicar por que uma construção, produção ou entrega está bloqueada —
     **próximo bloco do Claude** (detalhes na divisão de trabalho): os
     alertas e o painel já explicam a produção; faltam construção (qual
     material falta e quanto a vila tem) e entrega (edifício com estrada na
     porta, mas sem ligação pela estrada com um armazém).
   - Acompanhar a simulação nova: despensa da igreja no painel da
     instituição e mortes por evento separadas das mortes por fome.

10. **Arte e áudio**
    - Em andamento na `claude/interface`: o mundo já é desenhado em pixel art
      (Kenney Tiny Town e Tiny Farm, CC0, mais peças próprias), com os 29
      edifícios, estações e o cenário guardado em pedaços. O Medieval RTS foi
      avaliado e descartado (ver contexto.md, Visual).
    - Falta: animações (água, fumaça, roda d'água, caminhada), prévia de
      construção no estilo novo, música e sons ambientais.

11. **Estamentos sociais**
    - Camponeses, artesãos, mercadores e clero.
    - Necessidades, trabalhos e influência diferentes.

12. **Campanha e mapa histórico**
    - Mapa próprio no lugar do gerador provisório.
    - Manter a campanha de fases no mapa contínuo, avançando por ano e
      requisitos de nível das instituições (regra detalhada no bloco 5).

13. **Acabamento técnico**
    - Migração de arquivos de salvamento.
    - Testes longos e desempenho com milhares de agentes.
    - Opções, controles, distribuição e preparação para lançamento.

## Regras de desempenho incorporadas do plano técnico

- Medir antes de otimizar; corrigir algoritmo antes de considerar SIMD ou
  assembly.
- Manter a mesma assinatura do estado final ao comparar desempenho.
- O mapa 128×128 atual tem folga. Só reabrir otimizações estruturais se uma
  medição real mostrar regressão.
- Se o mapa crescer além disso, avaliar nesta ordem:
  1. indexar apenas estoques atualmente disponíveis;
  2. guardar distâncias entre portas;
  3. distribuir o planejamento de pedidos entre ticks.
- Cada mudança de população, logística ou conteúdo deve rodar o teste do
  inverno e ao menos uma simulação longa sem janela.

## Decisões ainda abertas

- Regras operacionais de carregadores e trilhas listadas no item 7a.
- Detalhes das melhorias de tecnologia e da comida dos sem-teto (item 7b).
- Escopo exato dos estamentos sociais após a economia básica estar completa.
- Se a aba História deve anunciar eventos antes de começarem (hoje só mostra
  os já iniciados, escolha provisória registrada no contexto.md).
- Formato definitivo dos eventos depois do primeiro conjunto em TOML.

## Divisão de trabalho em paralelo

Para avançar com Codex e Claude ao mesmo tempo, a decisão de fluxo é usar
uma branch de integração e um worktree/branch exclusivo para cada agente.
Nunca executar os dois na mesma pasta de trabalho: os arquivos são
compartilhados e alterações simultâneas poderiam se misturar.

### Fluxo decidido de branches

- `main` recebe somente trabalho integrado e validado.
- `integracao/plano-de-acao` é a branch de integração dos próximos blocos;
  ambos os agentes partem dela.
- `codex/simulacao` recebe eventos, regras de jogo, fases e economia.
- `claude/interface` recebe menus, painéis, alertas e apresentação visual.
- Cada branch tem seu próprio worktree. O Vitor integra primeiro
  `codex/simulacao` em `integracao/plano-de-acao`, roda testes e build; depois
  integra `claude/interface` e valida novamente.
- Após validar o conjunto, a branch de integração pode ser mesclada em
  `main`. Não fazer push ou merge de uma branch de agente diretamente em
  `main`.
- Se um nome já estiver ocupado, criar um nome com sufixo de tarefa/data,
  sem reutilizar nem sobrescrever a branch existente.

### Codex — simulação e regras do jogo

Arquivos: tudo o que é regra (`src/simulacao.*`, `populacao.*`,
`producao.*`, `transporte.*`, `cidade.*`, `campanha.*`, `eventos.*`,
`conteudo.*`, `salvamento.*`), `dados/*.toml` e `ferramentas/balanceamento`.

Feito: eventos em TOML com mortalidade (item 4), fases por ano e
instituições (itens 5 e 6), primeiro corte das cadeias (item 7), comida de
crianças e sem-teto (parte do 7b, `de33955`).

**Próximo bloco do Codex** (atualizado em 2026-10-03; o Vitor confirma ou
troca): as frentes abaixo não dependem de decisão nova; as outras esperam
o Vitor.
- Sem decisão pendente, em ordem sugerida:
  1. Item 7: fechar a cadeia de argila, combustível e tijolo e rodar o
     rebalanceamento integrado de mão de obra, transporte e custos, com
     `make balance` e o teste do inverno.
  2. Item 13: teste longo sem janela (1150–1370) para medir desempenho e
     conferir que nada quebra ao longo da campanha.
- Esperando o Vitor:
  - 7b: melhorias de tecnologia (por edifício ou por tipo, níveis, ganho,
    custo, quais edifícios) e quem come primeiro quando falta comida.
  - 7a: as seis regras de carregadores e trilhas.
  - 4: efeitos demográficos de longo prazo (fecundidade, recuperação).
- Ao terminar cada parte: avisar no recado qual API nova a interface pode
  mostrar (como foi com `mortos_por_eventos()`).

### Claude — interface, apresentação e conteúdo visual

Arquivos: `src/main.cpp`, `src/painel.*`, `src/menu.*`, `src/relatorio*`,
`src/alertas.*`, `src/ferramentas.*`, `src/historico.*`, `src/melhorias.*`,
`src/arte.*`, `src/pedacos.*`, `src/desenho.*`, `src/assinatura_desenho.*`
e `ferramentas/gerar_pecas.py`. A interface só lê a simulação; qualquer
mudança em cabeçalho de regra é combinada antes pelo recado.

Feito: menu e partidas, alertas clicáveis, relatório (emprego, comida,
produção, transporte, fase e história), painel das instituições com botão
de melhoria, visual em pixel art.

**Próximo bloco do Claude** (registrado em 2026-10-03, passo 1 do
protocolo): explicar bloqueios de construção e entrega, e acompanhar a
simulação nova (item 9).
- Ramo: `claude/interface`, worktree `hansestadt-claude`.
- Construção: a dica da ferramenta diz qual material falta e quanto a vila
  tem ("faltam 3 tora: tem 5 de 8"), e por que uma instituição não pode ser
  construída de novo.
- Entrega: alerta novo para edifício com estrada na porta mas sem ligação
  pela estrada com nenhum armazém, calculado com `DistanciasPelaEstrada`
  (já existe em `rotas.hpp`) e só refeito quando as estradas mudam.
- Painel da instituição volta a mostrar o estoque (a igreja agora guarda
  comida para os sem-teto).
- Relatório e barra de cima: mortes por evento (`mortos_por_eventos()`)
  separadas das mortes por fome.
- Arquivos: `src/ferramentas.cpp`, `src/alertas.*`, `src/painel.cpp`,
  `src/relatorio*`, `src/main.cpp`. Nenhuma mudança em cabeçalho da
  simulação. `alertas.cpp` foi tocado pelo Codex em `de33955` (dias de
  comida); este bloco parte dessa versão.
- Depois deste: animações (água, fumaça, roda d'água, caminhada) e prévia
  de construção no estilo novo, do item 10.

### Vitor — direção e integração

- Abrir os dois worktrees, um para cada branch, e iniciar as tarefas
  independentes em paralelo.
- Definir dúvidas de produto que mudem regras históricas ou balanceamento e
  testar as mudanças no jogo.
- Integrar um branch por vez: revisar commits, executar testes e build, e
  resolver conflitos antes de pedir o próximo bloco.

### Protocolo de integração

1. Antes de cada bloco, registrar o escopo, a branch e os arquivos principais atribuídos.
2. Não editar o mesmo arquivo nos dois worktrees. Se surgir dependência,
   combinar uma interface primeiro e integrar a mudança central em um só
   branch.
3. Fazer commits pequenos e deixar cada branch compilável e testada.
4. Atualizar este plano para marcar progresso; `contexto.md` guarda decisões
   e histórico, e o README explica o estado atual do jogo.
5. Mesclar primeiro a parte de simulação/API em `integracao/plano-de-acao`;
   depois, integrar a interface usando a API já consolidada.

## Concluído recentemente

- Primeiro marco jogável e inverno de 1150–51.
- Balanceamento agregado e indicadores de autonomia.
- Salvar/carregar partidas.
- Lenhador consumindo a floresta.
- Otimizações de moradia, emprego, rotas e transporte.
- Crescimento populacional e maioridade.
- Carregadores integrados à população.
- Custos de construção e provisões iniciais.
- Eventos históricos em TOML (Grande Fome, inverno de 1322–23, Peste Negra
  com mortalidade), fases por ano e instituições, cadeias de cerveja,
  pecuária, tecidos, couro e arenque salgado.
- Menu de pausa com vagas de partida, alertas clicáveis e relatório.
- Visual em pixel art no jogo, com o cenário guardado em pedaços.
- Primeira integração dos dois ramos no `main` (032695e).
- Crianças comem meia porção; sem-teto comem no armazém ou na igreja
  (`de33955`, parte do item 7b).
- Histórico de acontecimentos e painel das instituições na interface
  (`09b7b1e`..`06c73df`, ainda no ramo `claude/interface`).
