#pragma once

#include <cstdint>
#include <vector>

#include "raylib.h"

#include "arte.hpp"
#include "assinatura_desenho.hpp"

// Cenário parado guardado em imagens, um pedaço de CELULAS_POR_PEDACO x
// CELULAS_POR_PEDACO células por imagem. Medido no protótipo: desenhar o mapa
// inteiro peça por peça custava 36 ms por quadro; copiar os pedaços prontos,
// menos de 1 ms.
//
// Um pedaço é refeito quando a assinatura dele muda (ver assinatura_desenho).
// Pedaço que nunca foi desenhado é feito na hora, para não aparecer buraco;
// pedaço desatualizado continua mostrando a imagem antiga e é refeito aos
// poucos, alguns por quadro, para a troca de estação não travar o jogo.
class CenarioEmPedacos {
public:
    explicit CenarioEmPedacos(int refeitos_por_quadro = 4) : refeitos_por_quadro_(refeitos_por_quadro) {}
    ~CenarioEmPedacos() { esquecer(); }
    CenarioEmPedacos(const CenarioEmPedacos&) = delete;
    CenarioEmPedacos& operator=(const CenarioEmPedacos&) = delete;

    // Confere os pedaços que encostam em `visivel` e refaz os necessários.
    // Chamar antes de BeginDrawing: desenhar num pedaço troca o alvo do desenho.
    void atualizar(const Arte& arte, const Cidade& cidade, Estacao estacao, Rectangle visivel);
    // Copia os pedaços visíveis para a tela (dentro de BeginMode2D).
    void desenhar(Rectangle visivel, float zoom);
    // Descarta todas as imagens (ao carregar outra partida, por exemplo).
    void esquecer();

    int refeitos_no_ultimo_quadro() const { return refeitos_no_ultimo_quadro_; }

private:
    struct Pedaco {
        RenderTexture2D imagem{};
        std::uint64_t assinatura = 0;
        bool pronto = false;
    };

    void refazer(Pedaco& p, int px, int py, const Arte& arte, const Cidade& cidade, Estacao estacao, std::uint64_t assinatura);
    void intervalo(Rectangle visivel, int& px0, int& py0, int& px1, int& py1) const;

    int refeitos_por_quadro_;
    int refeitos_no_ultimo_quadro_ = 0;
    int colunas_ = 0, linhas_ = 0;
    std::vector<Pedaco> pedacos_;
    int filtro_ = -1;  // filtro de textura atual das imagens
};
