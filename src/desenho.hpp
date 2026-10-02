#pragma once

#include "raylib.h"

#include "mapa.hpp"

// Tamanho de uma célula em pixels de mundo (zoom 1).
inline constexpr int TAMANHO_CELULA = 32;

struct Celula {
    int x;
    int y;
};

// Desenha só as células dentro de `visivel` (coordenadas de mundo).
void desenhar_mapa(const Mapa& mapa, Rectangle visivel, float zoom);
void destacar_celula(Celula c);

// Célula sob um ponto do mundo. Pode estar fora do mapa; conferir com
// Mapa::dentro.
Celula celula_em(Vector2 ponto_mundo);
