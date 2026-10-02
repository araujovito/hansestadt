#pragma once

#include <cstdint>

#include "mapa.hpp"

// Catálogo provisório: os valores vão para arquivos TOML na tarefa de
// conteúdo, quando os edifícios ganharem receitas e estoques.
enum class TipoEdificio : std::uint8_t {
    Armazem,
    Casa,
    Lenhador,
    Carvoaria,
    PocoDeArgila,
    CortadorDeTurfa,
    Olaria,
    Quantidade,
};

struct DefinicaoEdificio {
    const char* nome;
    int largura;
    int altura;
    // Todas as células do edifício precisam ser deste terreno. Quem extrai um
    // recurso fica em cima dele; o resto exige terra aberta.
    Terreno terreno_exigido;
};

const DefinicaoEdificio& definicao(TipoEdificio tipo);
