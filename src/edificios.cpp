#include "edificios.hpp"

#include <iterator>

namespace {

constexpr DefinicaoEdificio DEFINICOES[] = {
    { "armazém", 3, 3, Terreno::Campo },
    { "casa", 2, 2, Terreno::Campo },
    { "lenhador", 2, 2, Terreno::Campo },
    { "carvoaria", 2, 2, Terreno::Campo },
    { "poço de argila", 2, 2, Terreno::Barreiro },
    { "cortador de turfa", 2, 2, Terreno::Turfa },
    { "olaria", 3, 2, Terreno::Campo },
};

static_assert(std::size(DEFINICOES) == static_cast<std::size_t>(TipoEdificio::Quantidade),
              "cada TipoEdificio precisa de uma definição");

}  // namespace

const DefinicaoEdificio& definicao(TipoEdificio tipo) {
    return DEFINICOES[static_cast<std::size_t>(tipo)];
}
