#include "producao.hpp"

#include <cmath>

int fator_do_mes(const Receita& r, int mes) {
    return static_cast<int>(std::lround(r.sazonalidade[static_cast<std::size_t>(mes - 1)] * ESCALA_FATOR));
}

std::int64_t progresso_total(const Receita& r, int ticks_por_dia) {
    return static_cast<std::int64_t>(r.dias) * ticks_por_dia * ESCALA_FATOR;
}

namespace {

bool pode_comecar(const Cidade& cidade, IdEdificio id, const Receita& r, int mes) {
    if (fator_do_mes(r, mes) == 0) return false;
    const Edificio& e = cidade.edificio(id);
    for (const Quantidade& q : r.entradas) {
        if (e.estoque[q.item] < q.quantidade) return false;
    }
    for (const Quantidade& q : r.saidas) {
        if (cidade.espaco_para(id, q.item) < q.quantidade) return false;
    }
    return true;
}

void tentar_comecar(Cidade& cidade, IdEdificio id, int mes) {
    const DefinicaoEdificio& def = cidade.definicao(id);
    for (std::size_t i = 0; i < def.receitas.size(); ++i) {
        const Receita& r = def.receitas[i];
        if (!pode_comecar(cidade, id, r, mes)) continue;
        for (const Quantidade& q : r.entradas) cidade.retirar(id, q.item, q.quantidade);
        Edificio& e = cidade.edificio(id);
        e.receita_ativa = static_cast<int>(i);
        e.progresso = 0;
        return;
    }
}

}  // namespace

void avancar_producao(Cidade& cidade, int mes, int ticks_por_dia) {
    // Por índice, em ordem crescente: a ordem de atualização faz parte do
    // resultado quando dois edifícios disputarem o mesmo recurso.
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo || cidade.definicao(id).receitas.empty()) continue;

        if (cidade.edificio(id).receita_ativa < 0) tentar_comecar(cidade, id, mes);

        Edificio& e = cidade.edificio(id);
        if (e.receita_ativa < 0) continue;

        const Receita& r = cidade.definicao(id).receitas[static_cast<std::size_t>(e.receita_ativa)];
        e.progresso += fator_do_mes(r, mes);
        if (e.progresso < progresso_total(r, ticks_por_dia)) continue;

        // O espaço foi conferido no início e só a produção põe saídas aqui,
        // então a entrega cabe.
        for (const Quantidade& q : r.saidas) cidade.adicionar(id, q.item, q.quantidade);
        e.receita_ativa = -1;
        e.progresso = 0;
    }
}

const char* descrever(Situacao s) {
    switch (s) {
        case Situacao::SemReceita: return "";
        case Situacao::Produzindo: return "produzindo";
        case Situacao::Pausado: return "pausado pela estação";
        case Situacao::ForaDeEstacao: return "fora de estação";
        case Situacao::FaltaEntrada: return "falta matéria-prima";
        case Situacao::SaidaCheia: return "estoque de saída cheio";
    }
    return "?";
}

Situacao situacao(const Cidade& cidade, IdEdificio id, int mes) {
    const DefinicaoEdificio& def = cidade.definicao(id);
    if (def.receitas.empty()) return Situacao::SemReceita;

    const Edificio& e = cidade.edificio(id);
    if (e.receita_ativa >= 0) {
        const Receita& r = def.receitas[static_cast<std::size_t>(e.receita_ativa)];
        return fator_do_mes(r, mes) > 0 ? Situacao::Produzindo : Situacao::Pausado;
    }

    for (const Receita& r : def.receitas) {
        if (fator_do_mes(r, mes) == 0) continue;
        for (const Quantidade& q : r.saidas) {
            if (cidade.espaco_para(id, q.item) < q.quantidade) return Situacao::SaidaCheia;
        }
        return Situacao::FaltaEntrada;
    }
    return Situacao::ForaDeEstacao;
}
