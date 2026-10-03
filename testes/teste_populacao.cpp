#include "populacao.hpp"
#include "conteudo_de_teste.hpp"
#include "verificar.hpp"

namespace {

Cidade cidade_para_populacao() {
    return Cidade(Mapa(12, 12), conteudo_de_teste());
}

}  // namespace

TESTE("população: a vila começa com três casais") {
    Populacao p;
    VERIFICAR(p.total() == 6);
    VERIFICAR(p.alojados() == 0);
    VERIFICAR(p.sem_casa() == 6);
}

TESTE("população: moradores ocupam as vagas das casas") {
    Cidade c = cidade_para_populacao();
    const IdEdificio primeira = *c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio segunda = *c.construir_edificio(tipo("casa"), { 5, 1 });
    Populacao p;
    p.alojar(c);
    VERIFICAR(p.moradores_na_casa(primeira) == 4);
    VERIFICAR(p.moradores_na_casa(segunda) == 2);
    VERIFICAR(p.alojados() == 6);
}

TESTE("população: casa demolida deixa moradores sem teto até haver vaga") {
    Cidade c = cidade_para_populacao();
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    Populacao p(4);
    p.alojar(c);
    VERIFICAR(p.moradores_na_casa(casa) == 4);
    c.demolir({ 1, 1 });
    p.alojar(c);
    VERIFICAR(p.sem_casa() == 4);

    const IdEdificio nova = *c.construir_edificio(tipo("casa"), { 5, 1 });
    p.alojar(c);
    VERIFICAR(p.moradores_na_casa(nova) == 4);
}

TESTE("população: moradores alojados ocupam empregos em ordem") {
    Cidade c = cidade_para_populacao();
    c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio serraria = *c.construir_edificio(tipo("serraria"), { 5, 1 });
    const IdEdificio colheita = *c.construir_edificio(tipo("colheita"), { 7, 1 });
    Populacao p(1);
    p.alojar(c);
    p.empregar(c);
    VERIFICAR(p.trabalhadores_em(serraria) == 1);
    VERIFICAR(p.trabalhadores_em(colheita) == 0);
    VERIFICAR(p.desempregados() == 0);

    c.demolir({ 5, 1 });
    p.empregar(c);
    VERIFICAR(p.trabalhadores_em(colheita) == 1);
}
