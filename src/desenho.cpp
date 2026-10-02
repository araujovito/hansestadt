#include "desenho.hpp"

#include <algorithm>
#include <cmath>

#include "ruido.hpp"

namespace {

Color cor_base(Terreno t) {
    switch (t) {
        case Terreno::Campo: return { 126, 156, 84, 255 };
        case Terreno::Floresta: return { 52, 92, 48, 255 };
        case Terreno::Barreiro: return { 168, 112, 76, 255 };
        case Terreno::Agua: return { 58, 98, 140, 255 };
        case Terreno::Turfa: return { 92, 82, 58, 255 };
    }
    return MAGENTA;
}

// Varia levemente o tom de cada célula para o mapa não parecer um tabuleiro.
// A variação vem do hash da posição, então é sempre a mesma em cada quadro.
Color com_variacao(Color c, int x, int y) {
    const float v = 0.92f + 0.16f * ruido::hash(x, y, 0xC0FFEE);
    auto canal = [v](unsigned char k) {
        return static_cast<unsigned char>(std::min(255.0f, static_cast<float>(k) * v));
    };
    return { canal(c.r), canal(c.g), canal(c.b), c.a };
}

void desenhar_detalhe(Terreno t, int x, int y, float px, float py) {
    constexpr float m = TAMANHO_CELULA / 2.0f;
    switch (t) {
        case Terreno::Floresta: {
            // Duas copas por célula, em posições fixas sorteadas pelo hash.
            for (int i = 0; i < 2; ++i) {
                const float ox = 6.0f + 20.0f * ruido::hash(x * 2 + i, y, 0xA11);
                const float oy = 6.0f + 20.0f * ruido::hash(x * 2 + i, y, 0xB22);
                DrawCircleV({ px + ox, py + oy }, 6.0f, Color{ 34, 70, 34, 255 });
            }
            break;
        }
        case Terreno::Turfa:
            DrawLineEx({ px + 6, py + m }, { px + 14, py + m }, 2.0f, Color{ 60, 72, 48, 255 });
            DrawLineEx({ px + 18, py + m + 6 }, { px + 26, py + m + 6 }, 2.0f, Color{ 60, 72, 48, 255 });
            break;
        case Terreno::Barreiro:
            DrawCircleV({ px + m, py + m }, 4.0f, Color{ 140, 90, 60, 255 });
            break;
        case Terreno::Campo:
        case Terreno::Agua:
            break;
    }
}

}  // namespace

void desenhar_mapa(const Mapa& mapa, Rectangle visivel, float zoom) {
    const int x0 = std::max(0, static_cast<int>(std::floor(visivel.x / TAMANHO_CELULA)));
    const int y0 = std::max(0, static_cast<int>(std::floor(visivel.y / TAMANHO_CELULA)));
    const int x1 = std::min(mapa.largura() - 1, static_cast<int>((visivel.x + visivel.width) / TAMANHO_CELULA));
    const int y1 = std::min(mapa.altura() - 1, static_cast<int>((visivel.y + visivel.height) / TAMANHO_CELULA));

    // Abaixo deste zoom os detalhes viram ruído visual e só custam desenho.
    const bool com_detalhes = zoom >= 0.6f;

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            const Terreno t = mapa.terreno(x, y);
            const float px = static_cast<float>(x * TAMANHO_CELULA);
            const float py = static_cast<float>(y * TAMANHO_CELULA);
            DrawRectangleV({ px, py }, { TAMANHO_CELULA, TAMANHO_CELULA }, com_variacao(cor_base(t), x, y));
            if (com_detalhes) desenhar_detalhe(t, x, y, px, py);
        }
    }
}

void destacar_celula(Celula c) {
    DrawRectangleLinesEx({ static_cast<float>(c.x * TAMANHO_CELULA), static_cast<float>(c.y * TAMANHO_CELULA),
                           TAMANHO_CELULA, TAMANHO_CELULA },
                         2.0f, Color{ 255, 240, 180, 220 });
}

Celula celula_em(Vector2 p) {
    return { static_cast<int>(std::floor(p.x / TAMANHO_CELULA)),
             static_cast<int>(std::floor(p.y / TAMANHO_CELULA)) };
}
