#pragma once

#include <string>

#include "raylib.h"

#include "assinatura_desenho.hpp"
#include "cidade.hpp"
#include "transporte.hpp"

// Desenho em pixel art: peças de 16x16 dos pacotes Tiny Town e Tiny Farm da
// Kenney (CC0, baixados por scripts/baixar-kenney.sh) e peças próprias
// (dados/arte/pecas.png, geradas por ferramentas/gerar_pecas.py). Cada
// célula do jogo tem 2x2 peças, sempre desenhadas em tamanho 1x: um tamanho
// de pixel só para o chão, os edifícios e as pessoas.
//
// As três folhas viram uma textura só ao carregar, para a raylib juntar as
// peças em poucas chamadas à placa de vídeo. Cada peça ganha 1 pixel de folga
// com a borda repetida, para não aparecer a peça vizinha da folha nas bordas
// quando o zoom não é inteiro.
class Arte {
public:
    // `raiz` é a pasta do projeto (onde ficam vendor/ e dados/). Devolve false e
    // preenche `erro` se faltar alguma folha.
    bool carregar(const std::string& raiz, std::string& erro);
    void descarregar();

    // Chão, bordas, caminhos, neve e objetos do terreno (árvores, montes de
    // argila, poças) das células [x0, x1] x [y0, y1]. É o que fica guardado
    // nos pedaços do cenário: não depende de nada que mude a cada quadro.
    void desenhar_terreno(const Cidade& cidade, int x0, int y0, int x1, int y1, Estacao estacao) const;

    // Edifícios (com lavouras, cercas e objetos de cada ofício) que encostam
    // em `visivel`. Desenhados a cada quadro, por cima do cenário guardado.
    void desenhar_edificios(const Cidade& cidade, Fase fase, int mes, Rectangle visivel) const;

    // Carregadores andando, com sombra; quem leva carga aparece com um saco.
    void desenhar_carregadores(const Cidade& cidade, const Transporte& transporte, Rectangle visivel) const;

    bool carregada() const { return atlas_.id != 0; }

private:
    Texture2D atlas_{};
};
