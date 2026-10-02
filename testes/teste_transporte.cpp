#include "conteudo_de_teste.hpp"
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
