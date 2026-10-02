#include "ruido.hpp"

#include <cmath>

namespace ruido {

float hash(int x, int y, std::uint32_t semente) {
    std::uint32_t h = semente;
    h ^= static_cast<std::uint32_t>(x) * 0x27d4eb2dU;
    h ^= static_cast<std::uint32_t>(y) * 0x165667b1U;
    // Finalizador do murmur3: espalha cada bit de entrada por toda a saída.
    h ^= h >> 15;
    h *= 0x85ebca6bU;
    h ^= h >> 13;
    h *= 0xc2b2ae35U;
    h ^= h >> 16;
    return static_cast<float>(h >> 8) / static_cast<float>(1U << 24);
}

namespace {

float suavizar(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float misturar(float a, float b, float t) {
    return a + (b - a) * t;
}

}  // namespace

float valor(float x, float y, float escala, std::uint32_t semente) {
    const float gx = x / escala;
    const float gy = y / escala;
    const int x0 = static_cast<int>(std::floor(gx));
    const int y0 = static_cast<int>(std::floor(gy));
    const float tx = suavizar(gx - static_cast<float>(x0));
    const float ty = suavizar(gy - static_cast<float>(y0));

    const float topo = misturar(hash(x0, y0, semente), hash(x0 + 1, y0, semente), tx);
    const float base = misturar(hash(x0, y0 + 1, semente), hash(x0 + 1, y0 + 1, semente), tx);
    return misturar(topo, base, ty);
}

float fractal(float x, float y, float escala, int oitavas, std::uint32_t semente) {
    float soma = 0.0f;
    float amplitude = 1.0f;
    float total = 0.0f;
    for (int i = 0; i < oitavas; ++i) {
        soma += valor(x, y, escala, semente + static_cast<std::uint32_t>(i) * 101U) * amplitude;
        total += amplitude;
        escala *= 0.5f;
        amplitude *= 0.5f;
    }
    return soma / total;
}

}  // namespace ruido
