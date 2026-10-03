#include "partidas.hpp"

#include <filesystem>
#include <system_error>

#include "salvamento.hpp"

const std::vector<VagaDePartida>& vagas_de_partida() {
    static const std::vector<VagaDePartida> vagas = {
        { "Salvamento rápido (F5)", "partida.hanse" },
        { "Partida 1", "partida-1.hanse" },
        { "Partida 2", "partida-2.hanse" },
        { "Partida 3", "partida-3.hanse" },
    };
    return vagas;
}

std::string resumir_partida(const Simulacao& sim) {
    const int moradores = sim.populacao.total();
    return std::to_string(sim.data.dia) + " de " + nome_do_mes(sim.data.mes) + " de " +
           std::to_string(sim.data.ano) + ", " + std::to_string(moradores) +
           (moradores == 1 ? " morador" : " moradores");
}

SituacaoDaVaga examinar_vaga(const std::string& arquivo, const Conteudo& conteudo) {
    std::error_code erro;
    if (!std::filesystem::exists(arquivo, erro)) return {};
    // Carregar a partida inteira só para o resumo é desperdício em tese, mas
    // o arquivo de uma vila tem poucos KB e o menu só faz isso ao abrir a
    // tela; um cabeçalho separado mudaria o formato do salvamento.
    const ResultadoSalvamento r = carregar_partida(arquivo, conteudo);
    if (!r.simulacao) return { SituacaoDaVaga::Estado::Invalida, r.erro };
    return { SituacaoDaVaga::Estado::Valida, resumir_partida(*r.simulacao) };
}
