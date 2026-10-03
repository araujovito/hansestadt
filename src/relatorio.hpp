#pragma once

#include <array>
#include <string>
#include <vector>

#include "producao.hpp"
#include "simulacao.hpp"

// Números da vila para a janela de relatório (emprego, comida, produção e
// transporte). Só lê a simulação e não depende da raylib, para ser testado
// sem janela; o desenho fica em relatorio_tela.

struct LinhaDeEmprego {
    std::string nome;   // tipo de edifício, ou "carregadores" no armazém
    int edificios = 0;  // quantos desse tipo estão ativos
    int vagas = 0;
    int ocupadas = 0;
};

struct RelatorioEmprego {
    int adultos = 0;
    int criancas = 0;
    int sem_casa = 0;
    int desempregados = 0;
    int vagas = 0;
    int ocupadas = 0;
    // Só tipos com vaga, na ordem do conteúdo; os carregadores vêm primeiro
    // porque o transporte preenche as vagas antes da produção.
    std::vector<LinhaDeEmprego> linhas;
};

struct LinhaDeComida {
    std::string nome;
    int estoque = 0;
};

struct RelatorioComida {
    int estoque = 0;
    int moradores = 0;
    int dias = 0;  // estoque ÷ moradores, como na barra de cima
    int refeicoes_perdidas = 0;
    int mortos_de_fome = 0;
    std::vector<LinhaDeComida> itens;  // todos os itens de comida, mesmo zerados
};

inline constexpr std::size_t N_SITUACOES = 7;  // valores de Situacao

struct LinhaDeProducao {
    std::string nome;
    int edificios = 0;
    // Quantos edifícios desse tipo estão em cada Situacao, indexado pelo
    // valor do enum. Edifício sem estrada conta à parte, como nos alertas.
    std::array<int, N_SITUACOES> por_situacao{};
    int sem_estrada = 0;
};

struct RelatorioTransporte {
    // Agentes de transporte. Os do depósito vêm das vagas de carregador; os
    // outros moram num edifício de produção (o transporte cria um agente por
    // trabalhador de cada edifício, ver Transporte::criar_carregadores).
    int carregadores = 0;
    int do_deposito = 0;
    int ocupados = 0;    // com tarefa (indo buscar ou entregando)
    int carregando = 0;  // com item na mão
    unsigned long long entregas = 0;
    unsigned long long perdidos = 0;
};

struct Relatorio {
    RelatorioEmprego emprego;
    RelatorioComida comida;
    std::vector<LinhaDeProducao> producao;  // só tipos com receita e ao menos um ativo
    RelatorioTransporte transporte;
};

Relatorio montar_relatorio(const Simulacao& sim);

// "2 produzindo, 1 sem trabalhadores": resume uma linha de produção.
std::string resumir_producao(const LinhaDeProducao& linha);
