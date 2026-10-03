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

TESTE("população: sem-teto preenchem primeiro a casa de menor id com vaga") {
    Cidade c = cidade_para_populacao();
    c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio segunda = *c.construir_edificio(tipo("casa"), { 5, 1 });
    Populacao p(6);
    p.alojar(c);  // 4 na primeira, 2 na segunda
    c.demolir({ 1, 1 });
    const IdEdificio terceira = *c.construir_edificio(tipo("casa"), { 1, 5 });
    p.alojar(c);
    // Os 4 da casa demolida: dois completam a segunda, dois vão para a nova.
    VERIFICAR(p.moradores()[0].casa == segunda);
    VERIFICAR(p.moradores()[1].casa == segunda);
    VERIFICAR(p.moradores()[2].casa == terceira);
    VERIFICAR(p.moradores()[3].casa == terceira);
    VERIFICAR(p.moradores_na_casa(segunda) == 4);
    VERIFICAR(p.sem_casa() == 0);
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

TESTE("população: criança consome meia porção por dia") {
    Cidade c = cidade_para_populacao();
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    Morador crianca;
    crianca.idade_dias = 0;
    Populacao p = Populacao::restaurar({ crianca }, 0, 0);
    p.alojar(c);
    c.adicionar(casa, item("pao"), 2);

    for (int dia = 0; dia < 4; ++dia) {
        p.consumir_comida(c);
        p.avancar_dia(c, { 1150, 3, 2 });
    }
    VERIFICAR(c.edificio(casa).estoque[item("pao")] == 0);
    VERIFICAR(p.moradores()[0].dias_sem_comida == 0);
    VERIFICAR(p.refeicoes_perdidas() == 0);
}

TESTE("população: sem-teto come no armazém ou no abrigo") {
    Cidade c = cidade_para_populacao();
    const IdEdificio deposito = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    const IdEdificio abrigo = *c.construir_edificio(tipo("abrigo"), { 4, 1 });
    Populacao p(2);
    c.adicionar(deposito, item("pao"), 1);
    c.adicionar(abrigo, item("pao"), 1);

    p.consumir_comida(c);
    VERIFICAR(p.sem_casa() == 2);
    VERIFICAR(p.refeicoes_perdidas() == 0);
    VERIFICAR(c.edificio(deposito).estoque[item("pao")] == 0);
    VERIFICAR(c.edificio(abrigo).estoque[item("pao")] == 0);
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

TESTE("população: casal com comida e vaga tem uma criança no ano novo") {
    Cidade c = cidade_para_populacao();
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    Populacao p(2);
    p.alojar(c);
    c.adicionar(casa, item("pao"), 1);
    p.avancar_dia(c, {1151, 1, 1});
    VERIFICAR(p.total() == 3);
    VERIFICAR(p.criancas() == 1);
    VERIFICAR(p.moradores_na_casa(casa) == 3);
}

TESTE("população: criança não trabalha antes dos dezesseis anos") {
    Cidade c = cidade_para_populacao();
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio trabalho = *c.construir_edificio(tipo("serraria"), { 5, 1 });
    Populacao p(2);
    p.alojar(c);
    c.adicionar(casa, item("pao"), 1);
    p.avancar_dia(c, {1151, 1, 1});
    // Os dois adultos: um ocupa a única vaga, o outro fica desempregado.
    p.empregar(c);
    VERIFICAR(p.trabalhadores_em(trabalho) == 1);
    VERIFICAR(p.criancas() == 1);
    for (int dia = 0; dia < Morador::IDADE_ADULTA; ++dia) p.avancar_dia(c, {1151, 2, 1});
    VERIFICAR(p.criancas() == 0);
}

TESTE("população: carregador ocupa um adulto antes das oficinas") {
    Cidade c = cidade_para_populacao();
    c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio deposito = *c.construir_edificio(tipo("deposito"), { 5, 1 });
    const IdEdificio serraria = *c.construir_edificio(tipo("serraria"), { 7, 1 });
    Populacao p(2);
    p.alojar(c);
    p.empregar(c, true);
    VERIFICAR(p.trabalhadores_em(deposito) == 2);
    VERIFICAR(p.trabalhadores_em(serraria) == 0);
}
