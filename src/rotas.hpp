#pragma once

#include <optional>
#include <vector>

#include "cidade.hpp"

// Rota mais curta pelas estradas, de `de` até `para`, com as duas pontas
// incluídas. Anda só em células com estrada e só nas quatro direções: uma
// diagonal entre duas estradas que só se tocam pela quina não é caminho.
// Devolve nullopt se uma das pontas não tem estrada ou se não há ligação.
std::optional<std::vector<Celula>> buscar_rota(const Cidade& cidade, Celula de, Celula para);
