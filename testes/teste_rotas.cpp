#include "conteudo_de_teste.hpp"
#include "rotas.hpp"
#include "verificar.hpp"

namespace {

Cidade campo_aberto() {
    return Cidade(Mapa(10, 10), conteudo_de_teste());
}

void estrada(Cidade& c, Celula a, Celula b) {
    for (const Celula x : caminho_em_l(a, b)) c.construir_estrada(x);
}

}  // namespace

TESTE("rota: segue a estrada de ponta a ponta") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 5, 1 });
    const auto r = buscar_rota(c, { 1, 1 }, { 5, 1 });
    VERIFICAR(r.has_value());
    VERIFICAR(r && r->size() == 5);
    VERIFICAR(r && r->front() == (Celula{ 1, 1 }) && r->back() == (Celula{ 5, 1 }));
}

TESTE("rota: contorna a esquina em L") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 4, 4 });
    const auto r = buscar_rota(c, { 4, 4 }, { 1, 1 });
    VERIFICAR(r && r->size() == 7);
}

TESTE("rota: sem ligação não há rota") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 3, 1 });
    estrada(c, { 5, 1 }, { 7, 1 });
    VERIFICAR(!buscar_rota(c, { 1, 1 }, { 7, 1 }).has_value());
}

TESTE("rota: quina não liga estradas") {
    Cidade c = campo_aberto();
    c.construir_estrada({ 1, 1 });
    c.construir_estrada({ 2, 2 });
    VERIFICAR(!buscar_rota(c, { 1, 1 }, { 2, 2 }).has_value());
}

TESTE("rota: ponta sem estrada não tem rota; mesma célula é rota de um passo") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 3, 1 });
    VERIFICAR(!buscar_rota(c, { 1, 1 }, { 1, 2 }).has_value());
    const auto r = buscar_rota(c, { 2, 1 }, { 2, 1 });
    VERIFICAR(r && r->size() == 1);
}

TESTE("rota: escolhe o caminho mais curto entre dois") {
    Cidade c = campo_aberto();
    // Anel: por cima são 5 células de (1,1) a (5,1); por baixo, 13.
    estrada(c, { 1, 1 }, { 5, 1 });
    estrada(c, { 1, 1 }, { 1, 5 });
    estrada(c, { 1, 5 }, { 5, 5 });
    estrada(c, { 5, 5 }, { 5, 1 });
    const auto r = buscar_rota(c, { 1, 1 }, { 5, 1 });
    VERIFICAR(r && r->size() == 5);
}

TESTE("rota: demolir um trecho corta a rota") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 5, 1 });
    c.demolir({ 3, 1 });
    VERIFICAR(!buscar_rota(c, { 1, 1 }, { 5, 1 }).has_value());
}

TESTE("rota: cada passo é vizinho do anterior") {
    Cidade c = campo_aberto();
    estrada(c, { 0, 0 }, { 9, 9 });
    estrada(c, { 0, 9 }, { 9, 0 });
    const auto r = buscar_rota(c, { 0, 0 }, { 0, 9 });
    bool vizinhos = r.has_value();
    for (std::size_t i = 1; r && i < r->size(); ++i) {
        const int dx = std::abs((*r)[i].x - (*r)[i - 1].x);
        const int dy = std::abs((*r)[i].y - (*r)[i - 1].y);
        vizinhos = vizinhos && dx + dy == 1 && c.tem_estrada((*r)[i]);
    }
    VERIFICAR(vizinhos);
}
