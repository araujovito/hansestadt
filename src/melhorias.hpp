#pragma once

#include <optional>
#include <string>
#include <vector>

#include "calendario.hpp"
#include "cidade.hpp"
#include "conteudo.hpp"

// O que cada instituição precisa para melhorar e o que falta para a próxima
// fase, para o painel do edifício e para a aba Fase do relatório. Só lê a
// cidade, pelas mesmas regras de validar_melhoria e tentar_avancar_fase, e
// não depende da raylib.

struct MaterialDaMelhoria {
    std::string nome;
    int precisa = 0;
    int tem = 0;  // disponível em todos os estoques da vila, como na melhoria
};

struct FichaDaInstituicao {
    std::string nome;
    std::optional<IdEdificio> edificio;  // vazio enquanto não foi construída
    int nivel = 0;                       // 0 enquanto não foi construída
    int nivel_da_fase = 0;               // o maior nível liberado na fase atual
    int nivel_final = 0;                 // o maior nível da campanha
    // Próxima melhoria, se houver: a fase que a libera e os materiais.
    std::optional<Fase> fase_da_proxima;
    std::vector<MaterialDaMelhoria> materiais;
    // Resultado de validar_melhoria; EdificioInativo se não foi construída.
    ResultadoMelhoria pode_melhorar = ResultadoMelhoria::EdificioInativo;

    // Construída e no maior nível da fase: é o que a progressão exige.
    bool pronta_para_a_fase() const { return nivel > 0 && nivel == nivel_da_fase; }
};

FichaDaInstituicao ficha_da_instituicao(const Cidade& cidade, IdTipoEdificio tipo, Fase fase);

struct RumoAProximaFase {
    Fase fase;              // a fase seguinte
    int ano = 0;            // a partir de quando ela abre
    bool ano_chegou = false;
    // As instituições já disponíveis, na ordem do conteúdo; todas precisam
    // estar prontas para a fase.
    std::vector<FichaDaInstituicao> instituicoes;

    bool instituicoes_prontas() const;
};

// Vazio na última fase.
std::optional<RumoAProximaFase> rumo_a_proxima_fase(const Cidade& cidade, Fase fase, Data hoje);
