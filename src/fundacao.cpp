#include "fundacao.hpp"

#include <stdexcept>

Fundacao::Fundacao(const Conteudo& conteudo, int comida_inicial)
    : comida_(conteudo.buscar_item("pao").value_or(0)), comida_inicial_(comida_inicial) {
    if (!conteudo.buscar_item("pao")) throw std::runtime_error("a fundação precisa do item 'pao'");
}

bool Fundacao::concluir_se_possivel(Cidade& cidade) {
    if (concluida_) return false;
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo || !cidade.definicao(id).deposito) continue;
        if (!cidade.adicionar(id, comida_, comida_inicial_)) return false;
        armazem_ = id;
        concluida_ = true;
        return true;
    }
    return false;
}
