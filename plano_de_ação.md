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
   - Em andamento na `codex/simulacao`: primeiro corte em TOML com Grande
     Fome e Peste Negra, intervalos julianos inclusivos e modificadores de
     produção; efeitos demográficos ficam pendentes.
   - API para a interface: `sim.eventos.ativos(sim.data)` e
     `sim.eventos.ocorridos(sim.data)`; a lista inclui definições com nome,
     descrição e intervalo.
   - Chuvas, invernos rigorosos, Grande Fome e Peste Negra.
   - Começar em TOML para datas e modificadores simples.
   - Só introduzir Lua se um evento real exigir lógica que o TOML não expresse.
   - Registrar acontecimentos e seus efeitos para o jogador.

5. **Evolução das fases**
   - A campanha acontece em um único mapa contínuo, sem reiniciar a cidade
     ao trocar de fase.
   - Cada fase tem um ano-limite histórico e só é alcançada quando o
     calendário chega a esse ano e as instituições exigidas estão no maior
     nível disponível.
   - O calendário continua avançando independentemente; chegar ao ano sem
     cumprir os requisitos mantém a fase atual até cumpri-los.
   - Anos sugeridos pelo README (1150, 1200, 1250, 1300 e 1370) e quais
     edifícios contam como instituições aguardam confirmação.

6. **Edifícios institucionais**
   - Igreja, governo, mercado e porto.
   - Evolução dos edifícios ligada à progressão histórica.

7. **Completar as cadeias econômicas**
   - Cerveja, gado, ovelhas, tecidos, couro, arenque, barris e salga.
   - Cadeia completa de argila, combustível e tijolos.
   - Rebalancear mão de obra, transporte e custos após cada ramo.

8. **Agricultura, clima e recursos**
   - Plantio e colheita mais visíveis.
   - Esgotamento ou regeneração de recursos.
   - Efeitos sazonais e climáticos detalhados.

9. **Interface e experiência do jogador**
   - Menus de pausa, salvamento e carregamento.
   - Alertas clicáveis e histórico de acontecimentos.
   - Visões de emprego, fome, produção e logística.
   - Explicar por que uma construção, produção ou entrega está bloqueada.

10. **Arte e áudio**
    - Sprites, animações, música e sons ambientais.
    - Avaliar primeiro o pacote Medieval RTS da Kenney (CC0).

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

- Momento de substituir a arte provisória pelos sprites.
- Escopo exato dos estamentos sociais após a economia básica estar completa.
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

- Implementar o modelo de eventos históricos e climáticos em TOML, incluindo
  efeitos na simulação e testes de regras.
- Depois, implementar progressão de fases, requisitos e edifícios
  institucionais.
- Completar cadeias econômicas e ajustar custos, trabalho e transporte.
- Atualizar testes, README e contexto para cada bloco.

### Claude — interface, apresentação e conteúdo visual

- Implementar menus de pausa e telas de salvar/carregar usando as funções
  existentes de salvamento.
- Melhorar alertas, histórico visível de acontecimentos e painéis de
  emprego, fome, produção e logística.
- Integrar sprites e animações, avaliando primeiro os recursos Kenney CC0;
  acrescentar áudio quando o fluxo visual estiver estável.
- Trabalhar em `src/main.cpp`, `src/painel.*`, `src/desenho.*` e novos
  arquivos de interface/recursos. Combinar previamente qualquer mudança em
  cabeçalhos centrais da simulação.

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
