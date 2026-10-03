#include "assinatura_desenho.hpp"
#include "conteudo_de_teste.hpp"
#include "verificar.hpp"

namespace {

// Dois pedaços de largura, para haver um vizinho.
Cidade cidade_de_dois_pedacos() {
    return Cidade(Mapa(2 * CELULAS_POR_PEDACO, CELULAS_POR_PEDACO), conteudo_de_teste());
}

}  // namespace

TESTE("assinatura: a mesma cidade dá a mesma assinatura") {
    const Cidade a = cidade_de_dois_pedacos();
    const Cidade b = cidade_de_dois_pedacos();
    VERIFICAR(assinatura_do_pedaco(a, 0, 0, Estacao::Verao) == assinatura_do_pedaco(b, 0, 0, Estacao::Verao));
}

TESTE("assinatura: estrada, terreno e edifício dentro do pedaço mudam a assinatura") {
    Cidade c = cidade_de_dois_pedacos();
    const std::uint64_t antes = assinatura_do_pedaco(c, 0, 0, Estacao::Verao);
    c.construir_estrada({ 5, 5 });
    const std::uint64_t com_estrada = assinatura_do_pedaco(c, 0, 0, Estacao::Verao);
    VERIFICAR(com_estrada != antes);
    c.construir_edificio(tipo("pequeno"), { 8, 8 });
    VERIFICAR(assinatura_do_pedaco(c, 0, 0, Estacao::Verao) != com_estrada);
}

TESTE("assinatura: mudança na faixa em volta conta, mais longe não") {
    Cidade c = cidade_de_dois_pedacos();
    const std::uint64_t antes = assinatura_do_pedaco(c, 0, 0, Estacao::Verao);
    // Coluna 16 é do pedaço 1, mas fica na faixa de 3 células do pedaço 0.
    c.construir_estrada({ CELULAS_POR_PEDACO, 4 });
    const std::uint64_t com_vizinho = assinatura_do_pedaco(c, 0, 0, Estacao::Verao);
    VERIFICAR(com_vizinho != antes);
    // Coluna 25 está a 10 células da borda do pedaço 0: não o afeta.
    c.construir_estrada({ CELULAS_POR_PEDACO + 9, 4 });
    VERIFICAR(assinatura_do_pedaco(c, 0, 0, Estacao::Verao) == com_vizinho);
}

TESTE("assinatura: a estação muda a assinatura; primavera e verão são iguais") {
    const Cidade c = cidade_de_dois_pedacos();
    VERIFICAR(assinatura_do_pedaco(c, 0, 0, Estacao::Verao) != assinatura_do_pedaco(c, 0, 0, Estacao::Inverno));
    VERIFICAR(assinatura_do_pedaco(c, 0, 0, Estacao::Outono) != assinatura_do_pedaco(c, 0, 0, Estacao::Inverno));
    VERIFICAR(estacao_do_mes(4) == estacao_do_mes(7));
    VERIFICAR(estacao_do_mes(10) == Estacao::Outono && estacao_do_mes(1) == Estacao::Inverno);
}
