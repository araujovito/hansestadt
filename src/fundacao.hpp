#pragma once

#include "cidade.hpp"

// A partida começa antes de existir a cidade: o jogador escolhe onde pôr o
// primeiro armazém. Ao construí-lo, as provisões da viagem são descarregadas
// uma única vez e a construção normal pode ser liberada.
class Fundacao {
public:
    static constexpr int COMIDA_INICIAL = 600;
    static constexpr int TORAS_INICIAIS = 40;

    explicit Fundacao(const Conteudo& conteudo, int comida_inicial = COMIDA_INICIAL,
                      int toras_iniciais = TORAS_INICIAIS);

    bool concluir_se_possivel(Cidade& cidade);
    bool concluida() const { return concluida_; }
    IdEdificio armazem() const { return armazem_; }
    void restaurar(IdEdificio armazem) { concluida_ = true; armazem_ = armazem; }

private:
    IdItem comida_;
    IdItem tora_;
    int comida_inicial_;
    int toras_iniciais_;
    bool concluida_ = false;
    IdEdificio armazem_ = SEM_EDIFICIO;
};
