#pragma once

#include "cidade.hpp"

// A partida começa antes de existir a cidade: o jogador escolhe onde pôr o
// primeiro armazém. Ao construí-lo, as provisões da viagem são descarregadas
// uma única vez e a construção normal pode ser liberada.
class Fundacao {
public:
    static constexpr int COMIDA_INICIAL = 120;

    explicit Fundacao(const Conteudo& conteudo, int comida_inicial = COMIDA_INICIAL);

    bool concluir_se_possivel(Cidade& cidade);
    bool concluida() const { return concluida_; }
    IdEdificio armazem() const { return armazem_; }

private:
    IdItem comida_;
    int comida_inicial_;
    bool concluida_ = false;
    IdEdificio armazem_ = SEM_EDIFICIO;
};
