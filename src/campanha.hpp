#pragma once

#include <string>
#include <vector>

#include "calendario.hpp"
#include "conteudo.hpp"

struct EstadoInstituicao {
    std::string id;
    int nivel = 0;
    int nivel_maximo_disponivel = 0;
};

struct ResultadoProgressao {
    Fase fase;
    bool avancou = false;
};

// Tenta avançar no máximo uma fase. A data abre a oportunidade, mas todas as
// instituições disponíveis devem estar no maior nível da fase atual.
ResultadoProgressao tentar_avancar_fase(Fase fase_atual, Data data,
                                         const std::vector<EstadoInstituicao>& instituicoes);
int ano_de_abertura(Fase fase);
