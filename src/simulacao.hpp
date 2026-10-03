#pragma once

#include <cstdint>

#include "calendario.hpp"
#include "campanha.hpp"
#include "cidade.hpp"
#include "eventos.hpp"
#include "populacao.hpp"
#include "transporte.hpp"

// Estado do mundo, avançado em passos de duração fixa. Nada aqui depende do
// tempo real nem do desenho: dado o mesmo estado inicial e a mesma sequência
// de comandos, a simulação chega sempre ao mesmo resultado, em qualquer
// velocidade ou taxa de quadros.
struct Simulacao {
    static constexpr int TICKS_POR_SEGUNDO = 20;
    // Um dia por segundo em velocidade 1×: um ano dura cerca de seis minutos.
    static constexpr int TICKS_POR_DIA = 20;

    explicit Simulacao(Cidade c) : cidade(std::move(c)) {}

    Cidade cidade;
    Populacao populacao;
    Transporte transporte;
    Eventos eventos;
    Fase fase = Fase::Vila;
    std::uint64_t tick = 0;
    // Primavera de 1150: início da estação de desmatamento e plantio, quando
    // os colonos da Ostsiedlung chegavam para fundar uma vila.
    Data data{ 1150, 3, 1 };

    void passo();
};
