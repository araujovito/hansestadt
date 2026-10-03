#pragma once

#include <vector>

#include "cidade.hpp"

struct Morador {
    IdEdificio casa = SEM_EDIFICIO;
};

// Moradores pertencem à simulação, não aos edifícios: assim continuam
// existindo quando uma casa é demolida e podem procurar outra vaga.
class Populacao {
public:
    explicit Populacao(int quantidade_inicial = 6);

    void alojar(Cidade& cidade);

    int total() const { return static_cast<int>(moradores_.size()); }
    int alojados() const;
    int sem_casa() const { return total() - alojados(); }
    int moradores_na_casa(IdEdificio casa) const;
    const std::vector<Morador>& moradores() const { return moradores_; }

private:
    std::vector<Morador> moradores_;
};
