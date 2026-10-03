#pragma once

#include <cstdint>

#include "cidade.hpp"

// O cenário parado (chão, bordas, caminhos, árvores, montes de argila) é
// desenhado uma vez em pedaços de CELULAS_POR_PEDACO x CELULAS_POR_PEDACO
// células e guardado em imagem; cada quadro só copia as imagens. Este
// arquivo diz quando um pedaço precisa ser refeito, sem depender da raylib.
inline constexpr int CELULAS_POR_PEDACO = 16;
// O desenho de uma célula olha até três células em volta (margem do rio,
// franja de grama, cantos da mata, árvores que escapam da mata para o campo).
// A assinatura cobre essa faixa para o pedaço ser refeito quando um vizinho
// de fora dele muda.
inline constexpr int VIZINHANCA_DO_PEDACO = 3;

// A estação muda as cores e a neve, por isso entra na assinatura. Primavera e
// verão têm o mesmo desenho.
enum class Estacao : std::uint8_t { Verao, Outono, Inverno };
Estacao estacao_do_mes(int mes);

// Resumo de tudo o que o desenho do pedaço (px, py) usa: terreno, estrada e
// ocupação por edifício de cada célula do pedaço e da faixa em volta, mais a
// estação. Se a assinatura não mudou, a imagem guardada ainda vale.
std::uint64_t assinatura_do_pedaco(const Cidade& cidade, int px, int py, Estacao estacao);
