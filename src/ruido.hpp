#pragma once

#include <cstdint>

// Ruído de valor determinístico para gerar terreno. Implementado aqui em vez
// de usar rand() ou <random> porque a distribuição dessas bibliotecas pode
// variar entre compiladores; com o hash próprio, a mesma semente gera o mesmo
// mapa em qualquer máquina.
namespace ruido {

// Valor pseudoaleatório em [0, 1) para uma coordenada inteira.
float hash(int x, int y, std::uint32_t semente);

// Ruído suave em [0, 1): interpola os valores de hash de uma grade de
// espaçamento `escala`.
float valor(float x, float y, float escala, std::uint32_t semente);

// Soma de oitavas de `valor`, cada uma com o dobro da frequência e metade da
// amplitude da anterior. Gera manchas com borda irregular (florestas, brejos).
float fractal(float x, float y, float escala, int oitavas, std::uint32_t semente);

}  // namespace ruido
