#pragma once

#include "raylib.h"

#include "cidade.hpp"
#include "mapa.hpp"

// Tamanho de uma célula em pixels de mundo (zoom 1).
inline constexpr int TAMANHO_CELULA = 32;

// Marcas de jogo por cima da arte (chamar dentro de BeginMode2D). O mundo em
// si é desenhado pela Arte e pelo CenarioEmPedacos.

// Círculo vermelho com "!" no canto dos edifícios sem estrada na porta: sem
// ela nenhum carregador chega, e o aviso fica até o jogador ligar a estrada.
void desenhar_avisos_sem_acesso(const Cidade& cidade, Rectangle visivel);
// Nome do edifício sob o cursor, numa etiqueta acima dele. Cada edifício tem
// silhueta própria; o nome só aparece quando o jogador aponta.
void desenhar_nome_sob_cursor(const Cidade& cidade, Celula cursor, float zoom);
void destacar_celula(Celula c);

// Célula sob um ponto do mundo. Pode estar fora do mapa; conferir com
// Mapa::dentro.
Celula celula_em(Vector2 ponto_mundo);
