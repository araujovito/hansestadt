#pragma once

#include "raylib.h"

#include "cidade.hpp"
#include "mapa.hpp"
#include "transporte.hpp"

// Tamanho de uma célula em pixels de mundo (zoom 1).
inline constexpr int TAMANHO_CELULA = 32;

// Desenha só as células dentro de `visivel` (coordenadas de mundo).
void desenhar_mapa(const Mapa& mapa, Rectangle visivel, float zoom);
// Estradas e edifícios, por cima do terreno. O mês muda a cor das lavouras.
void desenhar_cidade(const Cidade& cidade, Rectangle visivel, float zoom, int mes);
// Carregadores, interpolados entre a célula atual e a próxima da rota.
void desenhar_carregadores(const Cidade& cidade, const Transporte& transporte, Rectangle visivel);
void destacar_celula(Celula c);

// Célula sob um ponto do mundo. Pode estar fora do mapa; conferir com
// Mapa::dentro.
Celula celula_em(Vector2 ponto_mundo);
