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

TESTE("população: empregos seguem a ordem dos moradores de um edifício ao outro") {
    Cidade c = cidade_para_populacao();
    c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio serraria = *c.construir_edificio(tipo("serraria"), { 5, 1 });
    c.construir_edificio(tipo("deposito"), { 7, 1 });  // sem vagas de trabalho
    const IdEdificio colheita = *c.construir_edificio(tipo("colheita"), { 9, 1 });
    const IdEdificio forno = *c.construir_edificio(tipo("forno"), { 5, 4 });
    Populacao p(6);  // 4 cabem na casa, 2 ficam sem teto e não trabalham
    p.alojar(c);
    p.empregar(c);
    VERIFICAR(p.moradores()[0].trabalho == serraria);
    VERIFICAR(p.moradores()[1].trabalho == colheita);
    VERIFICAR(p.moradores()[2].trabalho == forno);
    VERIFICAR(p.moradores()[3].trabalho == SEM_EDIFICIO);
    VERIFICAR(p.moradores()[4].trabalho == SEM_EDIFICIO);
    VERIFICAR(p.desempregados() == 1);
}

TESTE("população: cada morador consome uma comida por dia") {
    Cidade c = cidade_para_populacao();
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    Populacao p(2);
    p.alojar(c);
    c.adicionar(casa, item("pao"), 3);

    p.consumir_comida(c);
    VERIFICAR(c.edificio(casa).estoque[item("pao")] == 1);
    VERIFICAR(p.moradores()[0].dias_sem_comida == 0);
    p.consumir_comida(c);
    VERIFICAR(c.edificio(casa).estoque[item("pao")] == 0);
    VERIFICAR(p.moradores()[0].dias_sem_comida == 0);
    VERIFICAR(p.moradores()[1].dias_sem_comida == 1);
}

TESTE("população: trinta dias sem comida matam e liberam casa e trabalho") {
    Cidade c = cidade_para_populacao();
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio trabalho = *c.construir_edificio(tipo("serraria"), { 5, 1 });
    Populacao p(1);
    p.alojar(c);
    p.empregar(c);

    for (int dia = 1; dia < Populacao::DIAS_ATE_MORRER_DE_FOME; ++dia) p.consumir_comida(c);
    VERIFICAR(p.total() == 1);
    VERIFICAR(p.moradores_na_casa(casa) == 1);
    p.consumir_comida(c);
    VERIFICAR(p.total() == 0);
    VERIFICAR(p.mortos_de_fome() == 1);
    VERIFICAR(p.moradores_na_casa(casa) == 0);
    VERIFICAR(p.trabalhadores_em(trabalho) == 0);
}

TESTE("população: restaurada do arquivo mantém a contagem de trabalhadores") {
    Cidade c = cidade_para_populacao();
    c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio serraria = *c.construir_edificio(tipo("serraria"), { 5, 1 });
    Populacao p(1);
    p.alojar(c);
    p.empregar(c);
    Populacao r = Populacao::restaurar(p.moradores(), p.mortos_de_fome(), p.refeicoes_perdidas());
    VERIFICAR(r.trabalhadores_em(serraria) == 1);
    // Morrer antes do próximo empregar também tem de atualizar a contagem.
    for (int dia = 0; dia < Populacao::DIAS_ATE_MORRER_DE_FOME; ++dia) r.consumir_comida(c);
    VERIFICAR(r.total() == 0);
    VERIFICAR(r.trabalhadores_em(serraria) == 0);
}
