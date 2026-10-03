# Plano de ação

Roteiro dos grandes blocos restantes do Hansestadt. A ordem pode mudar se
um teste integrado revelar uma dependência ou um gargalo novo.

## Em andamento

1. ~~**Crescimento populacional**~~ — concluído
   - Idade e sexo por morador.
   - Nascimentos condicionados a casal, moradia e comida.
   - Crianças passam a trabalhar aos 16 anos.

2. **Integrar carregadores à população**
   - Carregadores deixam de surgir automaticamente no armazém.
   - Transporte disputa adultos com fazendas e oficinas.
   - Mortes e demolições liberam ou removem carregadores corretamente.

3. **Custos de construção**
   - Declarar custos de materiais no TOML.
   - Validar e consumir recursos ao construir.
   - Mostrar custos e faltas na interface.

## Próximos blocos

4. **Eventos históricos e climáticos**
   - Chuvas, invernos rigorosos, Grande Fome e Peste Negra.
   - Escolher e implementar o formato dos eventos (TOML ou Lua).

5. **Evolução das fases**
   - Vila, mercado, cidade, cidade hanseática e auge.
   - Requisitos e liberações próprios de cada fase.

6. **Edifícios institucionais**
   - Igreja, governo, mercado e porto.
   - Evolução dos edifícios ligada à progressão histórica.

7. **Completar as cadeias econômicas**
   - Cerveja, gado, ovelhas, tecidos, couro, arenque, barris e salga.
   - Cadeia completa de argila, combustível e tijolos.

8. **Agricultura, clima e recursos**
   - Plantio e colheita mais visíveis.
   - Esgotamento ou regeneração de recursos.
   - Efeitos sazonais e climáticos detalhados.

9. **Interface e experiência do jogador**
   - Menus de pausa, salvamento e carregamento.
   - Alertas clicáveis e histórico de acontecimentos.
   - Visões de emprego, fome, produção e logística.

10. **Arte e áudio**
    - Sprites, animações, música e sons ambientais.

11. **Estamentos sociais**
    - Camponeses, artesãos, mercadores e clero.
    - Necessidades, trabalhos e influência diferentes.

12. **Campanha e mapa histórico**
    - Mapa próprio no lugar do gerador provisório.
    - Objetivos e progressão histórica de 1150 a 1370.

13. **Acabamento técnico**
    - Migração de arquivos de salvamento.
    - Testes longos e desempenho com milhares de agentes.
    - Opções, controles, distribuição e preparação para lançamento.

## Concluído recentemente

- Primeiro marco jogável e inverno de 1150–51.
- Balanceamento agregado e indicadores de autonomia.
- Salvar/carregar partidas.
- Lenhador consumindo a floresta.
- Otimizações de moradia, emprego, rotas e transporte.
