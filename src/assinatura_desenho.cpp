#include "assinatura_desenho.hpp"

Estacao estacao_do_mes(int mes) {
    if (mes >= 9 && mes <= 11) return Estacao::Outono;
    if (mes == 12 || mes <= 2) return Estacao::Inverno;
    return Estacao::Verao;
}

std::uint64_t assinatura_do_pedaco(const Cidade& cidade, int px, int py, Estacao estacao) {
    // FNV-1a de 64 bits: simples, rápido e com a mesma resposta em qualquer
    // máquina. Uma colisão só faria um pedaço deixar de ser refeito uma vez.
    std::uint64_t h = 1469598103934665603ull;
    auto misturar = [&h](std::uint64_t v) {
        for (int i = 0; i < 8; ++i) {
            h ^= (v >> (i * 8)) & 0xff;
            h *= 1099511628211ull;
        }
    };
    misturar(static_cast<std::uint64_t>(estacao));
    const Mapa& mapa = cidade.mapa();
    const int x0 = px * CELULAS_POR_PEDACO - VIZINHANCA_DO_PEDACO;
    const int y0 = py * CELULAS_POR_PEDACO - VIZINHANCA_DO_PEDACO;
    const int lado = CELULAS_POR_PEDACO + 2 * VIZINHANCA_DO_PEDACO;
    for (int y = y0; y < y0 + lado; ++y) {
        for (int x = x0; x < x0 + lado; ++x) {
            if (!mapa.dentro(x, y)) {
                misturar(0xffff);
                continue;
            }
            const Celula c{ x, y };
            misturar(static_cast<std::uint64_t>(mapa.terreno(x, y)) | (cidade.tem_estrada(c) ? 0x100u : 0u) |
                     (cidade.tem_trilha(c) ? 0x400u : 0u) |
                     (cidade.edificio_em(c) != SEM_EDIFICIO ? 0x200u : 0u));
        }
    }
    return h;
}
