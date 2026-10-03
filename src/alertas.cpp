#include "alertas.hpp"

#include <algorithm>

#include "producao.hpp"

namespace {

std::string contar(std::size_t n, const char* singular, const char* plural) {
    return std::to_string(n) + " " + (n == 1 ? singular : plural);
}

}  // namespace

std::vector<Alerta> levantar_alertas(const Simulacao& sim) {
    std::vector<Alerta> alertas;

    const int moradores = sim.populacao.total();
    if (moradores > 0) {
        const int dias = sim.cidade.estoque_total(TipoItem::Comida) / moradores;
        if (dias < DIAS_COMIDA_AVISO) {
            alertas.push_back({ Alerta::Tipo::Fome, dias < DIAS_COMIDA_GRAVE,
                                "comida para " + contar(static_cast<std::size_t>(dias), "dia", "dias"), {} });
        }
    }
    if (const int sem_casa = sim.populacao.sem_casa(); sem_casa > 0) {
        alertas.push_back({ Alerta::Tipo::SemCasa, false,
                            contar(static_cast<std::size_t>(sem_casa), "morador sem casa", "moradores sem casa"),
                            {} });
    }

    // Um edifício entra em um só alerta, o primeiro motivo que o impede:
    // sem estrada nada chega nem sai, então os outros motivos ficam para
    // depois de ligar a estrada.
    std::vector<IdEdificio> sem_estrada, sem_trabalhadores, falta_entrada, saida_cheia;
    for (IdEdificio id = 0; id < sim.cidade.edificios().size(); ++id) {
        if (!sim.cidade.edificio(id).ativo) continue;
        if (!sim.cidade.tem_acesso(id)) {
            sem_estrada.push_back(id);
            continue;
        }
        switch (situacao(sim.cidade, id, sim.data.mes, &sim.populacao)) {
            case Situacao::SemTrabalhadores: sem_trabalhadores.push_back(id); break;
            case Situacao::FaltaEntrada: falta_entrada.push_back(id); break;
            case Situacao::SaidaCheia: saida_cheia.push_back(id); break;
            case Situacao::SemReceita:
            case Situacao::Produzindo:
            case Situacao::Pausado:
            case Situacao::ForaDeEstacao: break;
        }
    }
    auto por_edificio = [&alertas](Alerta::Tipo tipo, bool grave, std::vector<IdEdificio> ids, const char* singular,
                                   const char* plural) {
        if (ids.empty()) return;
        alertas.push_back({ tipo, grave, contar(ids.size(), singular, plural), std::move(ids) });
    };
    por_edificio(Alerta::Tipo::SemEstrada, true, std::move(sem_estrada), "edifício sem estrada na entrada",
                 "edifícios sem estrada na entrada");
    por_edificio(Alerta::Tipo::SemTrabalhadores, false, std::move(sem_trabalhadores), "edifício sem trabalhadores",
                 "edifícios sem trabalhadores");
    por_edificio(Alerta::Tipo::FaltaEntrada, false, std::move(falta_entrada), "edifício parado por falta de insumo",
                 "edifícios parados por falta de insumo");
    por_edificio(Alerta::Tipo::SaidaCheia, false, std::move(saida_cheia), "edifício parado com a saída cheia",
                 "edifícios parados com a saída cheia");

    // stable_sort mantém a ordem de inserção dentro de cada grupo.
    std::stable_sort(alertas.begin(), alertas.end(),
                     [](const Alerta& a, const Alerta& b) { return a.grave && !b.grave; });
    return alertas;
}
