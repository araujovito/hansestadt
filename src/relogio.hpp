#pragma once

#include <algorithm>

// Converte o tempo real dos quadros em passos fixos de simulação
// ("fix your timestep"). A velocidade só muda quantos passos são executados
// por quadro, nunca o tamanho do passo, por isso jogar em 4× não altera os
// resultados da economia.
class Relogio {
public:
    static constexpr int VELOCIDADES[] = { 1, 2, 4 };

    explicit Relogio(double segundos_por_passo) : passo_(segundos_por_passo) {}

    // Devolve quantos passos a simulação deve dar neste quadro.
    int passos_para(double dt_real) {
        if (pausado_) return 0;
        // Um quadro muito longo (janela arrastada, depurador parado) não pode
        // virar uma avalanche de passos atrasados.
        acumulado_ += std::min(dt_real, 0.25) * velocidade();
        int passos = 0;
        while (acumulado_ >= passo_ && passos < MAX_PASSOS_POR_QUADRO) {
            acumulado_ -= passo_;
            ++passos;
        }
        // Se o limite foi atingido, o atraso é descartado: o jogo fica mais
        // lento por um instante em vez de travar tentando alcançar.
        if (passos == MAX_PASSOS_POR_QUADRO) acumulado_ = 0.0;
        return passos;
    }

    void alternar_pausa() { pausado_ = !pausado_; }
    void definir_indice_velocidade(int i) { indice_ = i; }

    bool pausado() const { return pausado_; }
    int velocidade() const { return VELOCIDADES[indice_]; }

private:
    static constexpr int MAX_PASSOS_POR_QUADRO = 16;

    double passo_;
    double acumulado_ = 0.0;
    int indice_ = 0;
    bool pausado_ = false;
};
