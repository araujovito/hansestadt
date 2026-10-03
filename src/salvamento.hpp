#pragma once

#include <optional>
#include <string>

#include "simulacao.hpp"

struct ResultadoSalvamento {
    std::optional<Simulacao> simulacao;
    std::string erro;
};

bool salvar_partida(const Simulacao& simulacao, const std::string& caminho, std::string& erro);
ResultadoSalvamento carregar_partida(const std::string& caminho, const Conteudo& conteudo);
