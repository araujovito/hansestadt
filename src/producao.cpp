#include "producao.hpp"

#include <cmath>

#include "populacao.hpp"
#include "eventos.hpp"

int fator_do_mes(const Receita& r, int mes) {
    return static_cast<int>(std::lround(r.sazonalidade[static_cast<std::size_t>(mes - 1)] * ESCALA_FATOR));
}

std::int64_t progresso_total(const Receita& r, int ticks_por_dia) {
    return static_cast<std::int64_t>(r.dias) * ticks_por_dia * ESCALA_FATOR;
}

namespace {

int fator_efetivo(const Cidade& cidade, IdEdificio id, const Receita& r, int mes, const Eventos* eventos, Data data) {
    const float evento = eventos ? eventos->multiplicador_producao(data, cidade.edificio(id).tipo) : 1.0f;
    return static_cast<int>(std::lround(fator_do_mes(r, mes) * evento));
}

bool pode_comecar(const Cidade& cidade, IdEdificio id, const Receita& r, int mes, const Eventos* eventos, Data data) {
    if (fator_efetivo(cidade, id, r, mes, eventos, data) == 0) return false;
    const Edificio& e = cidade.edificio(id);
    if (cidade.definicao(id).extrai && !cidade.tem_recurso_proximo(id)) return false;
    for (const Quantidade& q : r.entradas) {
        if (e.estoque[q.item] < q.quantidade) return false;
    }
    for (const Quantidade& q : r.saidas) {
        if (cidade.espaco_para(id, q.item) < q.quantidade) return false;
    }
    return true;
}

void tentar_comecar(Cidade& cidade, IdEdificio id, int mes, const Eventos* eventos, Data data) {
    const DefinicaoEdificio& def = cidade.definicao(id);
    for (std::size_t i = 0; i < def.receitas.size(); ++i) {
        const Receita& r = def.receitas[i];
        if (!pode_comecar(cidade, id, r, mes, eventos, data)) continue;
        for (const Quantidade& q : r.entradas) cidade.retirar(id, q.item, q.quantidade);
        Edificio& e = cidade.edificio(id);
        e.receita_ativa = static_cast<int>(i);
        e.progresso = 0;
        return;
    }
}

}  // namespace

void avancar_producao(Cidade& cidade, int mes, int ticks_por_dia, const Populacao* populacao,
                      const Eventos* eventos, Data data) {
    // Por índice, em ordem crescente: a ordem de atualização faz parte do
    // resultado quando dois edifícios disputarem o mesmo recurso.
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo || cidade.definicao(id).receitas.empty()) continue;
        if (populacao && populacao->trabalhadores_em(id) < cidade.definicao(id).trabalhadores) continue;

        if (cidade.edificio(id).receita_ativa < 0) tentar_comecar(cidade, id, mes, eventos, data);

        Edificio& e = cidade.edificio(id);
        if (e.receita_ativa < 0) continue;

        const Receita& r = cidade.definicao(id).receitas[static_cast<std::size_t>(e.receita_ativa)];
        // Técnicas aceleram o ciclo em vez de aumentar a saída: o estoque
        // continua em unidades inteiras e a sazonalidade segue valendo.
        e.progresso += static_cast<std::int64_t>(fator_efetivo(cidade, id, r, mes, eventos, data)) *
                       cidade.produtividade_tecnica(e.tipo) / PRODUTIVIDADE_NORMAL;
        if (e.progresso < progresso_total(r, ticks_por_dia)) continue;

        // O espaço foi conferido no início e só a produção põe saídas aqui,
        // então a entrega cabe.
        if (cidade.definicao(id).extrai && !cidade.extrair_recurso_proximo(id)) {
            e.receita_ativa = -1;
            e.progresso = 0;
            continue;
        }
        for (const Quantidade& q : r.saidas) cidade.adicionar(id, q.item, q.quantidade);
        e.receita_ativa = -1;
        e.progresso = 0;
    }
}

const char* descrever(Situacao s) {
    switch (s) {
        case Situacao::SemReceita: return "";
        case Situacao::SemTrabalhadores: return "faltam trabalhadores";
        case Situacao::Produzindo: return "produzindo";
        case Situacao::Pausado: return "pausado pela estação";
        case Situacao::ForaDeEstacao: return "fora de estação";
        case Situacao::FaltaEntrada: return "falta matéria-prima";
        case Situacao::SaidaCheia: return "estoque de saída cheio";
    }
    return "?";
}

Situacao situacao(const Cidade& cidade, IdEdificio id, int mes, const Populacao* populacao) {
    const DefinicaoEdificio& def = cidade.definicao(id);
    if (def.receitas.empty()) return Situacao::SemReceita;
    if (populacao && populacao->trabalhadores_em(id) < def.trabalhadores) return Situacao::SemTrabalhadores;

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
