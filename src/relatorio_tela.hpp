#pragma once

#include <optional>
#include <vector>

#include "raylib.h"

#include "relatorio.hpp"

// Janela de relatório com seis abas. Não para o jogo: dá para acompanhar a
// vila andando. Abre e fecha pela tecla V ou pelo botão na barra de cima.
class JanelaDeRelatorio {
public:
    enum class Aba { Emprego, Comida, Producao, Transporte, Fase, Historia };

    bool aberta() const { return aberta_; }
    void alternar() { aberta_ = !aberta_; }
    void fechar() { aberta_ = false; }

    // Guarda o estoque de comida uma vez por dia de jogo, para o gráfico da
    // aba Comida. O histórico é só da interface: não vai para o salvamento e
    // recomeça ao carregar uma partida.
    void registrar_dia(const Simulacao& sim);
    void esquecer_historico() { comida_por_dia_.clear(); }

    Rectangle area() const;
    // Trata cliques nas abas e no botão de fechar.
    void atualizar();
    // Se o clique deste quadro foi no botão da lei do trabalho infantil, na
    // aba Emprego; quem chama troca a lei na simulação.
    bool clicou_na_lei() const;
    void desenhar(const Relatorio& r) const;

private:
    void desenhar_emprego(const RelatorioEmprego& e, float x, float y) const;
    void desenhar_comida(const RelatorioComida& c, float x, float y) const;
    void desenhar_producao(const std::vector<LinhaDeProducao>& p, float x, float y) const;
    void desenhar_transporte(const RelatorioTransporte& t, float x, float y) const;
    void desenhar_fase(Fase fase, const std::optional<RumoAProximaFase>& rumo, float x, float y) const;
    void desenhar_historia(const std::vector<EntradaDoHistorico>& h, float x, float y) const;

    bool aberta_ = false;
    Aba aba_ = Aba::Emprego;
    std::vector<int> comida_por_dia_;
    Data ultimo_dia_{ 0, 0, 0 };
};
