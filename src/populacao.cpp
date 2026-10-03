#include "populacao.hpp"

#include <algorithm>

Populacao::Populacao(int quantidade_inicial)
    : moradores_(static_cast<std::size_t>(std::max(0, quantidade_inicial))) {}

int Populacao::alojados() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [](const Morador& m) { return m.casa != SEM_EDIFICIO; }));
}

int Populacao::moradores_na_casa(IdEdificio casa) const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [casa](const Morador& m) { return m.casa == casa; }));
}

void Populacao::alojar(Cidade& cidade) {
    for (Morador& m : moradores_) {
        if (m.casa != SEM_EDIFICIO && !cidade.edificio(m.casa).ativo) m.casa = SEM_EDIFICIO;
    }

    for (Morador& m : moradores_) {
        if (m.casa != SEM_EDIFICIO) continue;
        for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
            const Edificio& e = cidade.edificio(id);
            const DefinicaoEdificio& def = cidade.definicao(id);
            if (e.ativo && def.moradores > moradores_na_casa(id)) {
                m.casa = id;
                break;
            }
        }
    }
}
