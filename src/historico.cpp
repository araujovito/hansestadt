#include "historico.hpp"

#include <algorithm>
#include <cmath>

namespace {

std::string porcentagem(float fator) {
    return std::to_string(static_cast<int>(std::lround(fator * 100.0f))) + "%";
}

// Edifícios com o mesmo fator saem numa linha só: o arenque e o peixe de rio
// caem juntos e não precisam de duas linhas iguais.
std::vector<std::string> descrever_efeitos(const EventoHistorico& e, const Conteudo& conteudo) {
    std::vector<std::string> efeitos;
    if (e.multiplicador_geral != 1.0f) efeitos.push_back("produção geral a " + porcentagem(e.multiplicador_geral));

    std::vector<bool> usado(e.producao_por_edificio.size(), false);
    for (std::size_t i = 0; i < e.producao_por_edificio.size(); ++i) {
        if (usado[i]) continue;
        const float fator = e.producao_por_edificio[i].second;
        std::vector<std::string> nomes;
        for (std::size_t j = i; j < e.producao_por_edificio.size(); ++j) {
            if (usado[j] || e.producao_por_edificio[j].second != fator) continue;
            usado[j] = true;
            nomes.push_back(conteudo.edificio(e.producao_por_edificio[j].first).nome);
        }
        std::string lista = nomes[0];
        for (std::size_t k = 1; k < nomes.size(); ++k) lista += (k + 1 == nomes.size() ? " e " : ", ") + nomes[k];
        efeitos.push_back(lista + " a " + porcentagem(fator));
    }

    if (e.mortalidade_anual > 0.0f) efeitos.push_back(porcentagem(e.mortalidade_anual) + " de mortes ao ano");
    return efeitos;
}

int dias_ate(Data hoje, Data fim) {
    int dias = 1;
    for (Data d = hoje; d < fim; d = dia_seguinte(d)) ++dias;
    return dias;
}

}  // namespace

std::string escrever_data(Data d) {
    return std::to_string(d.dia) + " de " + nome_do_mes(d.mes) + " de " + std::to_string(d.ano);
}

std::vector<EntradaDoHistorico> montar_historico(const Eventos& eventos, Data hoje, const Conteudo& conteudo) {
    // Do início mais recente para o mais antigo; os em andamento vão antes.
    std::vector<const EventoHistorico*> ocorridos = eventos.ocorridos(hoje);
    std::stable_sort(ocorridos.begin(), ocorridos.end(),
                     [](const EventoHistorico* a, const EventoHistorico* b) { return b->inicio < a->inicio; });
    std::vector<EntradaDoHistorico> resultado;
    std::vector<EntradaDoHistorico> encerrados;
    for (const EventoHistorico* e : ocorridos) {
        EntradaDoHistorico entrada;
        entrada.nome = e->nome;
        entrada.descricao = e->descricao;
        entrada.periodo = escrever_data(e->inicio) + " a " + escrever_data(e->fim);
        entrada.em_andamento = hoje <= e->fim;
        if (entrada.em_andamento) entrada.dias_restantes = dias_ate(hoje, e->fim);
        entrada.efeitos = descrever_efeitos(*e, conteudo);
        (entrada.em_andamento ? resultado : encerrados).push_back(std::move(entrada));
    }
    for (EntradaDoHistorico& e : encerrados) resultado.push_back(std::move(e));
    return resultado;
}

std::vector<const EventoHistorico*> comecaram_entre(const Eventos& eventos, Data antes, Data hoje) {
    std::vector<const EventoHistorico*> resultado;
    for (const EventoHistorico& e : eventos.definicoes())
        if (antes < e.inicio && e.inicio <= hoje) resultado.push_back(&e);
    return resultado;
}
