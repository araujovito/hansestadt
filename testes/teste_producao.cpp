#include "cidade.hpp"
#include "conteudo_de_teste.hpp"
#include "producao.hpp"
#include "populacao.hpp"
#include "verificar.hpp"

namespace {

// Dias curtos deixam os testes rápidos sem mudar a lógica.
constexpr int TICKS_POR_DIA = 10;

void rodar(Cidade& c, int ticks, int mes) {
    for (int i = 0; i < ticks; ++i) avancar_producao(c, mes, TICKS_POR_DIA);
}

}  // namespace

TESTE("produção: consome no início e entrega ao fim do ciclo") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    c.adicionar(s, item("tora"), 2);

    rodar(c, TICKS_POR_DIA - 1, 3);
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 0);
    VERIFICAR(c.edificio(s).estoque[item("tabua")] == 0);
    VERIFICAR(c.edificio(s).receita_ativa == 0);

    rodar(c, 1, 3);
    VERIFICAR(c.edificio(s).estoque[item("tabua")] == 1);
    VERIFICAR(c.edificio(s).receita_ativa == -1);
}

TESTE("produção: sem entradas não começa") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    c.adicionar(s, item("tora"), 1);  // precisa de 2
    rodar(c, TICKS_POR_DIA * 3, 3);
    VERIFICAR(c.edificio(s).receita_ativa == -1);
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 1);
}

TESTE("produção: saída cheia não começa nem consome") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    c.adicionar(s, item("tabua"), 4);
    c.adicionar(s, item("tora"), 2);
    rodar(c, TICKS_POR_DIA * 3, 3);
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 2);
}

TESTE("produção: fora da estação não produz, em julho sim") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    rodar(c, TICKS_POR_DIA * 5, 3);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
    rodar(c, TICKS_POR_DIA * 3, 7);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 3);
}

TESTE("produção: fator 0,5 dobra a duração do ciclo") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    rodar(c, TICKS_POR_DIA * 2 - 1, 8);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
    rodar(c, 1, 8);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 1);
}

TESTE("produção: usa a receita alternativa quando a primeira não tem entrada") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("forno"), { 1, 1 });
    c.adicionar(f, item("tabua"), 1);
    rodar(c, 1, 3);
    VERIFICAR(c.edificio(f).receita_ativa == 1);
    VERIFICAR(c.edificio(f).estoque[item("tabua")] == 0);
}

TESTE("produção: edifício demolido para de produzir") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    c.demolir({ 1, 1 });
    rodar(c, TICKS_POR_DIA * 3, 7);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
}

TESTE("produção: só avança com a equipe completa") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 4, 1 });
    Populacao p(1);

    avancar_producao(c, 7, TICKS_POR_DIA, &p);
    VERIFICAR(c.edificio(f).receita_ativa == -1);
    VERIFICAR(situacao(c, f, 7, &p) == Situacao::SemTrabalhadores);

    p.alojar(c);
    p.empregar(c);
    VERIFICAR(p.moradores_na_casa(casa) == 1);
    for (int i = 0; i < TICKS_POR_DIA; ++i) avancar_producao(c, 7, TICKS_POR_DIA, &p);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 1);
}

TESTE("situação: explica por que o edifício está parado") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 3, 3 });
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 5, 5 });

    VERIFICAR(situacao(c, d, 3) == Situacao::SemReceita);
    VERIFICAR(situacao(c, s, 3) == Situacao::FaltaEntrada);
    VERIFICAR(situacao(c, f, 3) == Situacao::ForaDeEstacao);

    c.adicionar(s, item("tabua"), 4);
    VERIFICAR(situacao(c, s, 3) == Situacao::SaidaCheia);

    rodar(c, 1, 7);
    VERIFICAR(situacao(c, f, 7) == Situacao::Produzindo);
    VERIFICAR(situacao(c, f, 9) == Situacao::Pausado);
}
