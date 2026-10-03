#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "calendario.hpp"
#include "conteudo.hpp"

struct EventoHistorico {
    std::string id;
    std::string nome;
    std::string descricao;
    Data inicio;
    Data fim;
    float multiplicador_geral = 1.0f;
    // Probabilidade de mortalidade acumulada em um ano juliano, enquanto ativo.
    float mortalidade_anual = 0.0f;
    std::vector<std::pair<IdTipoEdificio, float>> producao_por_edificio;
};

struct ResultadoEventos {
    std::optional<std::vector<EventoHistorico>> eventos;
    std::vector<std::string> erros;
};

ResultadoEventos carregar_eventos(const std::string& pasta, const Conteudo& conteudo);

class Eventos {
public:
    Eventos() = default;
    explicit Eventos(std::vector<EventoHistorico> definicoes) : definicoes_(std::move(definicoes)) {}

    const std::vector<EventoHistorico>& definicoes() const { return definicoes_; }
    std::vector<const EventoHistorico*> ativos(Data data) const;
    std::vector<const EventoHistorico*> ocorridos(Data data) const;
    float multiplicador_producao(Data data, IdTipoEdificio edificio) const;
    float probabilidade_morte_diaria(Data data) const;

private:
    std::vector<EventoHistorico> definicoes_;
};
