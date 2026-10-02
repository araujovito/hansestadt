#include "mapa.hpp"
#include "verificar.hpp"

TESTE("mapa: a mesma semente gera sempre o mesmo mapa") {
    const Mapa a = gerar_mapa(64, 64, 1150);
    const Mapa b = gerar_mapa(64, 64, 1150);
    bool iguais = true;
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x) iguais = iguais && a.terreno(x, y) == b.terreno(x, y);
    VERIFICAR(iguais);
}

TESTE("mapa: tem os cinco terrenos e mar na primeira linha") {
    const Mapa m = gerar_mapa(128, 128, 1150);
    int contagem[5] = {};
    for (int y = 0; y < 128; ++y)
        for (int x = 0; x < 128; ++x) ++contagem[static_cast<int>(m.terreno(x, y))];
    for (int c : contagem) VERIFICAR(c > 0);
    VERIFICAR(m.terreno(64, 0) == Terreno::Agua);
}
