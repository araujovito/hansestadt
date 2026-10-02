#include "mapa.hpp"

#include <cmath>

#include "ruido.hpp"

const char* nome_do_terreno(Terreno t) {
    switch (t) {
        case Terreno::Campo: return "campo";
        case Terreno::Floresta: return "floresta";
        case Terreno::Barreiro: return "barreiro";
        case Terreno::Agua: return "água";
        case Terreno::Turfa: return "turfa";
    }
    return "?";
}

Mapa::Mapa(int largura, int altura)
    : largura_(largura), altura_(altura),
      celulas_(static_cast<std::size_t>(largura) * static_cast<std::size_t>(altura), Terreno::Campo) {}

namespace {

// Sementes derivadas para cada camada, para que costa, rio e florestas não
// compartilhem o mesmo padrão de ruído.
constexpr std::uint32_t CAMADA_COSTA = 0x1000;
constexpr std::uint32_t CAMADA_RIO = 0x2000;
constexpr std::uint32_t CAMADA_FLORESTA = 0x3000;
constexpr std::uint32_t CAMADA_TURFA = 0x4000;
constexpr std::uint32_t CAMADA_ARGILA = 0x5000;

// Linha da costa: o mar ocupa as primeiras linhas do mapa, com recortes.
int linha_da_costa(int x, int altura, std::uint32_t semente) {
    const float base = static_cast<float>(altura) * 0.10f;
    const float variacao = ruido::fractal(static_cast<float>(x), 0.0f, 24.0f, 3, semente + CAMADA_COSTA);
    return static_cast<int>(base + (variacao - 0.5f) * static_cast<float>(altura) * 0.10f);
}

// Centro do rio em cada linha: serpenteia em torno do meio do mapa.
float centro_do_rio(int y, int largura, std::uint32_t semente) {
    const float meandro = ruido::fractal(0.0f, static_cast<float>(y), 32.0f, 3, semente + CAMADA_RIO);
    return static_cast<float>(largura) * (0.35f + 0.30f * meandro);
}

}  // namespace

Mapa gerar_mapa(int largura, int altura, std::uint32_t semente) {
    Mapa mapa(largura, altura);

    for (int y = 0; y < altura; ++y) {
        const float rio = centro_do_rio(y, largura, semente);
        // O rio alarga perto da foz.
        const float meia_largura = y < altura / 4 ? 2.0f : 1.2f;

        for (int x = 0; x < largura; ++x) {
            const float fx = static_cast<float>(x);
            const float fy = static_cast<float>(y);
            const float distancia_rio = std::fabs(fx - rio);

            Terreno t = Terreno::Campo;
            if (y <= linha_da_costa(x, altura, semente) || distancia_rio <= meia_largura) {
                t = Terreno::Agua;
            } else if (distancia_rio <= meia_largura + 4.0f &&
                       ruido::fractal(fx, fy, 6.0f, 2, semente + CAMADA_ARGILA) > 0.55f) {
                // A argila se deposita nas várzeas, perto do rio.
                t = Terreno::Barreiro;
            } else if (ruido::fractal(fx, fy, 20.0f, 4, semente + CAMADA_FLORESTA) > 0.56f) {
                t = Terreno::Floresta;
            } else if (ruido::fractal(fx, fy, 14.0f, 3, semente + CAMADA_TURFA) > 0.66f) {
                t = Terreno::Turfa;
            }
            mapa.definir_terreno(x, y, t);
        }
    }
    return mapa;
}
