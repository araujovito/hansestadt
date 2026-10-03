#include "conteudo_de_teste.hpp"
#include "relatorio.hpp"
#include "verificar.hpp"

namespace {

// Depósito (2 vagas de carregador), casa para 4 e serraria (1 vaga), com a
// estrada ligando as portas.
Simulacao vila_com_trabalho(int moradores) {
    Cidade c(Mapa(12, 8), conteudo_de_teste());
    const IdEdificio deposito = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    c.construir_edificio(tipo("casa"), { 4, 1 });
    const IdEdificio serraria = *c.construir_edificio(tipo("serraria"), { 8, 1 });
    for (const Celula x : caminho_em_l(c.porta(deposito), c.porta(serraria))) c.construir_estrada(x);
    Simulacao sim(std::move(c));
    sim.populacao = Populacao(moradores);
    sim.populacao.alojar(sim.cidade);
    sim.populacao.empregar(sim.cidade, true);
    return sim;
}

}  // namespace

TESTE("relatório: emprego separa carregadores das vagas de produção") {
    const Simulacao sim = vila_com_trabalho(4);
    const RelatorioEmprego e = montar_relatorio(sim).emprego;
    VERIFICAR(e.adultos == 4 && e.criancas == 0);
    VERIFICAR(e.linhas.size() == 2);
    if (e.linhas.size() != 2) return;
    VERIFICAR(e.linhas[0].nome == "carregadores" && e.linhas[0].vagas == 3 && e.linhas[0].ocupadas == 3);
    VERIFICAR(e.linhas[1].nome == "serraria" && e.linhas[1].vagas == 1 && e.linhas[1].ocupadas == 1);
    VERIFICAR(e.vagas == 4 && e.ocupadas == 4);
    VERIFICAR(e.desempregados == 0);
}

TESTE("relatório: com pouca gente, produção ocupa primeiro") {
    const Simulacao sim = vila_com_trabalho(2);
    const RelatorioEmprego e = montar_relatorio(sim).emprego;
    VERIFICAR(e.linhas.size() == 2);
    if (e.linhas.size() != 2) return;
    VERIFICAR(e.linhas[0].ocupadas == 1);
    VERIFICAR(e.linhas[1].ocupadas == 1);
}

TESTE("relatório: comida lista todos os itens de comida e os dias como na barra") {
    Simulacao sim = vila_com_trabalho(4);
    sim.cidade.adicionar(0, item("pao"), 9);  // depósito
    const RelatorioComida c = montar_relatorio(sim).comida;
    VERIFICAR(c.estoque == 9 && c.moradores == 4 && c.dias == 2);
    VERIFICAR(c.itens.size() == 1 && c.itens[0].nome == "pão" && c.itens[0].estoque == 9);
}

TESTE("relatório: produção resume a situação de cada tipo") {
    Simulacao sim = vila_com_trabalho(4);
    // A serraria tem trabalhador e estrada, mas não tem tora.
    const std::vector<LinhaDeProducao> p = montar_relatorio(sim).producao;
    VERIFICAR(p.size() == 1 && p[0].nome == "serraria" && p[0].edificios == 1);
    VERIFICAR(p.size() == 1 && resumir_producao(p[0]) == "1 falta matéria-prima");

    sim.cidade.construir_edificio(tipo("serraria"), { 8, 5 });  // sem estrada
    const std::vector<LinhaDeProducao> q = montar_relatorio(sim).producao;
    VERIFICAR(q.size() == 1 && q[0].edificios == 2);
    VERIFICAR(q.size() == 1 && resumir_producao(q[0]) == "1 sem estrada, 1 falta matéria-prima");
}

TESTE("relatório: transporte conta carregadores, tarefas e entregas") {
    Simulacao sim = vila_com_trabalho(4);
    const RelatorioTransporte antes = montar_relatorio(sim).transporte;
    VERIFICAR(antes.carregadores == 0 && antes.entregas == 0);
    for (int i = 0; i < 3; ++i) sim.passo();
    const RelatorioTransporte depois = montar_relatorio(sim).transporte;
    VERIFICAR(depois.carregadores == static_cast<int>(sim.transporte.carregadores().size()));
    VERIFICAR(depois.do_deposito <= depois.carregadores);
    VERIFICAR(depois.ocupados <= depois.carregadores && depois.carregando <= depois.ocupados);
}
