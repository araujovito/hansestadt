#include "conteudo_de_teste.hpp"
#include "producao.hpp"
#include "simulacao.hpp"
#include "transporte.hpp"
#include "verificar.hpp"

namespace {

Cidade campo_aberto() {
    return Cidade(Mapa(16, 10), conteudo_de_teste());
}

void estrada(Cidade& c, Celula a, Celula b) {
    for (const Celula x : caminho_em_l(a, b)) c.construir_estrada(x);
}

void rodar(Transporte& t, Cidade& c, int ticks) {
    for (int i = 0; i < ticks; ++i) t.passo(c);
}

}  // namespace

TESTE("carregadores: o depósito cria os seus na porta, uma vez só") {
    Cidade c = campo_aberto();
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    Transporte t;
    rodar(t, c, 5);
    VERIFICAR(t.carregadores().size() == 2);
    VERIFICAR(t.carregadores()[0].posicao == c.porta(d));
    VERIFICAR(t.carregadores()[0].casa == d);
}

TESTE("carregadores: sem depósito não há carregador") {
    Cidade c = campo_aberto();
    c.construir_edificio(tipo("serraria"), { 1, 1 });
    Transporte t;
    rodar(t, c, 5);
    VERIFICAR(t.carregadores().empty());
}

TESTE("carregadores: vêm dos adultos empregados no depósito") {
    Cidade c = campo_aberto();
    c.construir_edificio(tipo("casa"), { 4, 4 });
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    Populacao p(1);
    p.alojar(c);
    p.empregar(c, true);
    Transporte t;
    t.passo(c, &p);
    VERIFICAR(t.carregadores().size() == 1);
    VERIFICAR(p.trabalhadores_em(d) == 1);
    for (int dia = 0; dia < Populacao::DIAS_ATE_MORRER_DE_FOME; ++dia) p.consumir_comida(c);
    p.empregar(c, true);
    t.passo(c, &p);
    VERIFICAR(t.carregadores().empty());
}

TESTE("carregadores: depósito demolido, mudam para o outro andando pela estrada") {
    Cidade c = campo_aberto();
    const IdEdificio a = *c.construir_edificio(tipo("deposito"), { 1, 1 });  // porta (1,2)
    const IdEdificio b = *c.construir_edificio(tipo("deposito"), { 9, 1 });  // porta (9,2)
    estrada(c, { 1, 2 }, { 9, 2 });
    Transporte t;
    rodar(t, c, 1);
    c.demolir({ 1, 1 });

    // 8 células até a outra porta, TICKS_POR_CELULA ticks cada.
    rodar(t, c, 8 * Transporte::TICKS_POR_CELULA - 1);
    VERIFICAR(!(t.carregadores()[0].posicao == c.porta(b)));
    rodar(t, c, 1);
    for (const Carregador& k : t.carregadores()) {
        VERIFICAR(k.casa == b);
        VERIFICAR(k.posicao == c.porta(b));
        VERIFICAR(!k.andando());
    }
    (void)a;
}

TESTE("carregadores: estrada cortada no caminho, recalculam por outro lado") {
    Cidade c = campo_aberto();
    c.construir_edificio(tipo("deposito"), { 1, 1 });                     // porta (1,2)
    const IdEdificio b = *c.construir_edificio(tipo("deposito"), { 9, 1 }); // porta (9,2)
    // Caminho curto pela linha 2 e um desvio pela linha 5.
    estrada(c, { 1, 2 }, { 9, 2 });
    estrada(c, { 1, 2 }, { 1, 5 });
    estrada(c, { 1, 5 }, { 9, 5 });
    estrada(c, { 9, 5 }, { 9, 2 });
    Transporte t;
    rodar(t, c, 1);
    c.demolir({ 1, 1 });
    rodar(t, c, 2 * Transporte::TICKS_POR_CELULA);  // andou duas células
    c.demolir({ 5, 2 });                             // corta à frente
    rodar(t, c, 40 * Transporte::TICKS_POR_CELULA);
    VERIFICAR(t.carregadores()[0].posicao == c.porta(b));
}

namespace {

// Depósito à esquerda e serraria à direita, ligados pela linha 2.
struct Vila {
    Cidade c = campo_aberto();
    IdEdificio deposito = *c.construir_edificio(tipo("deposito"), { 1, 1 });  // porta (1,2)
    IdEdificio serraria = *c.construir_edificio(tipo("serraria"), { 9, 1 });  // porta (9,2)
    Transporte t;
    Vila() { estrada(c, { 1, 2 }, { 9, 2 }); }
};

// Tempo de sobra para planejar, ir e voltar pela estrada de 8 células.
constexpr int VIAGEM = Transporte::PLANEJAR_A_CADA + 2 * 9 * Transporte::TICKS_POR_CELULA;

}  // namespace

TESTE("pedidos: a sobra do produtor vai para o depósito") {
    Vila v;
    v.c.adicionar(v.serraria, item("tabua"), 2);
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tabua")] == 0);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tabua")] == 2);
    VERIFICAR(v.t.entregas() == 2);
}

TESTE("pedidos: o depósito abastece a entrada do produtor") {
    Vila v;
    v.c.adicionar(v.deposito, item("tora"), 3);
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tora")] == 3);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tora")] == 0);
}

TESTE("pedidos: casa pede comida para a despensa") {
    Vila v;
    const IdEdificio casa = *v.c.construir_edificio(tipo("casa"), { 5, 4 });  // porta (5,6)
    estrada(v.c, { 5, 6 }, { 7, 2 });
    v.c.adicionar(v.deposito, item("pao"), 3);
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(casa).estoque[item("pao")] == 3);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("pao")] == 0);
}

TESTE("pedidos: não leva mais do que cabe na entrada") {
    Vila v;
    v.c.adicionar(v.deposito, item("tora"), 9);
    rodar(v.t, v.c, VIAGEM * 3);
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tora")] == 4);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tora")] == 5);
}

TESTE("pedidos: produtor sem estrada na porta não recebe nem envia") {
    Vila v;
    v.c.demolir({ 9, 2 });  // porta da serraria
    v.c.adicionar(v.deposito, item("tora"), 2);
    v.c.adicionar(v.serraria, item("tabua"), 1);
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tora")] == 0);
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tabua")] == 1);
}

TESTE("pedidos: busca na origem mais perto") {
    Vila v;
    // Segundo depósito colado na serraria: porta (7,2), já na estrada.
    const IdEdificio perto = *v.c.construir_edificio(tipo("deposito"), { 7, 1 });
    v.c.adicionar(v.serraria, item("tora"), 2);  // só cabem mais 2
    v.c.adicionar(v.deposito, item("tora"), 2);
    v.c.adicionar(perto, item("tora"), 2);
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(perto).estoque[item("tora")] == 0);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tora")] == 2);
}

TESTE("pedidos: depósito construído depois do primeiro planejamento entra na busca") {
    Vila v;
    rodar(v.t, v.c, VIAGEM);  // planejou com um depósito só
    const IdEdificio perto = *v.c.construir_edificio(tipo("deposito"), { 7, 1 });  // porta (7,2)
    v.c.adicionar(perto, item("tora"), 2);
    v.c.adicionar(v.serraria, item("tabua"), 1);
    rodar(v.t, v.c, VIAGEM * 2);
    // O novo é a única origem de tora e o depósito mais perto para a sobra.
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tora")] == 2);
    VERIFICAR(v.c.edificio(perto).estoque[item("tora")] == 0);
    VERIFICAR(v.c.edificio(perto).estoque[item("tabua")] == 1);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tabua")] == 0);
}

TESTE("pedidos: um item só é prometido a um destino") {
    Vila v;
    const IdEdificio outra = *v.c.construir_edificio(tipo("serraria"), { 5, 3 });  // porta (5,5)
    estrada(v.c, { 5, 2 }, { 5, 5 });
    v.c.adicionar(v.deposito, item("tora"), 1);
    rodar(v.t, v.c, VIAGEM * 2);
    const int total = v.c.edificio(v.serraria).estoque[item("tora")] + v.c.edificio(outra).estoque[item("tora")];
    VERIFICAR(total == 1);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tora")] == 0);
    VERIFICAR(v.c.edificio(v.deposito).reservado[item("tora")] == 0);
}

TESTE("pedidos: a cadeia anda sozinha (depósito, serraria, depósito)") {
    Vila v;
    v.c.adicionar(v.deposito, item("tora"), 4);
    // Produção e transporte juntos, como na simulação.
    for (int i = 0; i < VIAGEM * 6; ++i) {
        avancar_producao(v.c, 3, 10);
        v.t.passo(v.c);
    }
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tabua")] == 2);
}

TESTE("pedidos: mesma partida, mesmo resultado") {
    auto jogar = [] {
        Vila v;
        v.c.adicionar(v.deposito, item("tora"), 6);
        v.c.adicionar(v.serraria, item("tabua"), 3);
        rodar(v.t, v.c, 137);
        std::vector<int> estado;
        for (const Carregador& k : v.t.carregadores()) {
            estado.push_back(k.posicao.x);
            estado.push_back(static_cast<int>(k.tarefa));
        }
        for (int q : v.c.edificio(v.serraria).estoque) estado.push_back(q);
        return estado;
    };
    VERIFICAR(jogar() == jogar());
}

namespace {

// Roda até o primeiro carregador estar no estado pedido (ou desistir). Os
// carregadores só existem depois do primeiro passo.
void ate(Vila& v, Tarefa tarefa) {
    for (int i = 0; i < VIAGEM * 2; ++i) {
        v.t.passo(v.c);
        if (v.t.carregadores()[0].tarefa == tarefa) return;
    }
}

int a_caminho_total(const Cidade& c, IdEdificio id) {
    int total = 0;
    for (int q : c.edificio(id).a_caminho) total += q;
    return total;
}

}  // namespace

TESTE("demolição: origem demolida antes da coleta libera a vaga do destino") {
    Vila v;
    // Um forno à direita pede a tábua pronta da serraria. Os carregadores
    // saem do depósito da esquerda e passam 8 células indo buscar.
    const IdEdificio forno = *v.c.construir_edificio(tipo("forno"), { 12, 1 });  // porta (12,2)
    estrada(v.c, { 9, 2 }, { 12, 2 });
    v.c.adicionar(v.serraria, item("tabua"), 1);
    ate(v, Tarefa::Buscar);
    VERIFICAR(v.t.carregadores()[0].tarefa == Tarefa::Buscar);
    VERIFICAR(v.t.carregadores()[0].destino == forno);
    v.c.demolir({ 9, 1 });  // a serraria, origem
    rodar(v.t, v.c, 1);
    VERIFICAR(v.t.carregadores()[0].tarefa == Tarefa::Nenhuma);
    VERIFICAR(a_caminho_total(v.c, forno) == 0);
    VERIFICAR(v.t.perdidos() == 0);
}

TESTE("demolição: destino demolido com a carga na mão, leva ao depósito") {
    Vila v;
    v.c.adicionar(v.deposito, item("tora"), 1);
    ate(v, Tarefa::Entregar);
    VERIFICAR(v.t.carregadores()[0].carregando);
    v.c.demolir({ 9, 1 });  // serraria
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tora")] == 1);
    VERIFICAR(v.t.perdidos() == 0);
    VERIFICAR(a_caminho_total(v.c, v.deposito) == 0);
}

TESTE("demolição: destino lotado na chegada, leva ao depósito") {
    Vila v;
    v.c.adicionar(v.deposito, item("tora"), 1);
    ate(v, Tarefa::Entregar);
    v.c.adicionar(v.serraria, item("tora"), 4);  // lota por fora do transporte
    rodar(v.t, v.c, VIAGEM * 2);
    VERIFICAR(v.c.edificio(v.serraria).estoque[item("tora")] == 4);
    VERIFICAR(v.c.edificio(v.deposito).estoque[item("tora")] == 1);
    VERIFICAR(v.t.perdidos() == 0);
}

TESTE("demolição: sem depósito alcançável, a carga se perde e as reservas somem") {
    Vila v;
    v.c.adicionar(v.deposito, item("tora"), 1);
    ate(v, Tarefa::Entregar);
    v.c.demolir({ 9, 1 });  // serraria
    v.c.demolir({ 1, 1 });  // e o único depósito
    rodar(v.t, v.c, 2);
    VERIFICAR(v.t.carregadores()[0].tarefa == Tarefa::Nenhuma);
    VERIFICAR(!v.t.carregadores()[0].carregando);
    VERIFICAR(v.t.perdidos() == 1);
}

TESTE("simulação: produção e transporte rodam juntos no passo") {
    Vila v;
    v.c.construir_edificio(tipo("casa"), { 4, 4 });
    v.c.adicionar(v.deposito, item("tora"), 4);
    Simulacao sim(std::move(v.c));
    for (int i = 0; i < Simulacao::TICKS_POR_DIA * 10; ++i) sim.passo();
    VERIFICAR(sim.cidade.edificio(v.deposito).estoque[item("tabua")] == 2);
    VERIFICAR(sim.transporte.entregas() >= 6);  // 4 toras e 2 tábuas
}
