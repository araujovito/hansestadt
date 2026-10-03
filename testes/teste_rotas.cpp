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

TESTE("cache: a mesma viagem só é buscada uma vez") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 5, 1 });
    CacheDeRotas cache;
    for (int i = 0; i < 10; ++i) VERIFICAR(cache.rota(c, { 1, 1 }, { 5, 1 }).has_value());
    VERIFICAR(cache.buscas_feitas() == 1);
    cache.rota(c, { 5, 1 }, { 1, 1 });  // sentido contrário é outra viagem
    VERIFICAR(cache.buscas_feitas() == 2);
}

TESTE("cache: guarda também a ausência de rota") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 2, 1 });
    estrada(c, { 5, 1 }, { 6, 1 });
    CacheDeRotas cache;
    VERIFICAR(!cache.rota(c, { 1, 1 }, { 6, 1 }).has_value());
    VERIFICAR(!cache.rota(c, { 1, 1 }, { 6, 1 }).has_value());
    VERIFICAR(cache.buscas_feitas() == 1);
}

TESTE("cache: estrada nova ou demolida invalida as rotas") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 2, 1 });
    estrada(c, { 4, 1 }, { 5, 1 });
    CacheDeRotas cache;
    VERIFICAR(!cache.rota(c, { 1, 1 }, { 5, 1 }).has_value());
    c.construir_estrada({ 3, 1 });
    VERIFICAR(cache.rota(c, { 1, 1 }, { 5, 1 }).has_value());
    c.demolir({ 3, 1 });
    VERIFICAR(!cache.rota(c, { 1, 1 }, { 5, 1 }).has_value());
    VERIFICAR(cache.buscas_feitas() == 3);
}

TESTE("cache: construir edifício não invalida as rotas") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 5, 1 });
    CacheDeRotas cache;
    cache.rota(c, { 1, 1 }, { 5, 1 });
    c.construir_edificio(tipo("pequeno"), { 1, 3 });
    cache.rota(c, { 1, 1 }, { 5, 1 });
    VERIFICAR(cache.buscas_feitas() == 1);
}

TESTE("distâncias: dá o mesmo comprimento que o A* para toda célula") {
    Cidade c = campo_aberto();
    // Pente com desvios: várias rotas de comprimentos diferentes e um trecho
    // solto, sem ligação com o resto.
    estrada(c, { 0, 0 }, { 9, 0 });
    for (int x = 0; x <= 8; x += 2) estrada(c, { x, 0 }, { x, 6 });
    estrada(c, { 2, 6 }, { 8, 6 });
    estrada(c, { 0, 9 }, { 3, 9 });
    const Celula de{ 4, 6 };
    DistanciasPelaEstrada d;
    d.calcular(c, de);
    bool iguais = true;
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            const auto r = buscar_rota(c, de, { x, y });
            const int esperado = r ? static_cast<int>(r->size()) - 1 : DistanciasPelaEstrada::SEM_LIGACAO;
            iguais = iguais && d.ate({ x, y }) == esperado;
        }
    }
    VERIFICAR(iguais);
    VERIFICAR(d.ate({ 4, 6 }) == 0);
    VERIFICAR(d.ate({ 1, 9 }) == DistanciasPelaEstrada::SEM_LIGACAO);
    VERIFICAR(d.ate({ -1, 0 }) == DistanciasPelaEstrada::SEM_LIGACAO);
}

TESTE("distâncias: origem sem estrada não alcança nada") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 5, 1 });
    DistanciasPelaEstrada d;
    d.calcular(c, { 1, 2 });
    VERIFICAR(d.ate({ 1, 1 }) == DistanciasPelaEstrada::SEM_LIGACAO);
    VERIFICAR(d.ate({ 1, 2 }) == DistanciasPelaEstrada::SEM_LIGACAO);
}

TESTE("distâncias: repete a busca só quando a origem ou as estradas mudam") {
    Cidade c = campo_aberto();
    estrada(c, { 1, 1 }, { 5, 1 });
    DistanciasPelaEstrada d;
    d.calcular(c, { 1, 1 });
    d.calcular(c, { 1, 1 });
    VERIFICAR(d.buscas_feitas() == 1);
    d.calcular(c, { 5, 1 });
    VERIFICAR(d.buscas_feitas() == 2);
    c.construir_estrada({ 6, 1 });
    d.calcular(c, { 5, 1 });
    VERIFICAR(d.buscas_feitas() == 3);
    VERIFICAR(d.ate({ 6, 1 }) == 1);
}
