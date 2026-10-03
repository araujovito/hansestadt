#pragma once

#include <string>
#include <vector>

#include "simulacao.hpp"

// Problemas da vila que o jogador precisa ver sem procurar edifício por
// edifício. Só lê a simulação e não depende da raylib, para poder ser testado
// sem janela; o desenho e o clique ficam no painel.
struct Alerta {
    enum class Tipo {
        Fome,
        SemCasa,
        SemEstrada,
        SemTrabalhadores,
        FaltaEntrada,
        SaidaCheia,
    };

    Tipo tipo;
    bool grave;  // vermelho; os outros aparecem em amarelo
    std::string texto;
    // Edifícios envolvidos, em ordem de id. Clicar no alerta leva a eles, um
    // por clique. Vazio nos alertas da vila inteira (fome, sem casa).
    std::vector<IdEdificio> edificios;
};

// Abaixo destes dias de comida a barra de cima já fica amarela e vermelha;
// o alerta usa os mesmos limites para as duas coisas dizerem o mesmo.
inline constexpr int DIAS_COMIDA_AVISO = 90;
inline constexpr int DIAS_COMIDA_GRAVE = 30;

// Alertas na ordem em que aparecem: graves primeiro e, entre eles, os da
// vila antes dos de edifício. Pausa de inverno e fora de estação não entram:
// fazem parte do ciclo normal e o painel do edifício já os explica.
std::vector<Alerta> levantar_alertas(const Simulacao& sim);
