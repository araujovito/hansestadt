#include "pedacos.hpp"

#include <algorithm>
#include <cmath>

#include "rlgl.h"

namespace {

constexpr int C = 32;                               // célula do jogo (TAMANHO_CELULA)
constexpr int LADO = CELULAS_POR_PEDACO * C;        // pixels do pedaço, sem folga
// Folga em volta da imagem: copas de árvore e objetos do terreno passam um
// pouco da borda da célula. Pedaços vizinhos se sobrepõem nessa faixa.
constexpr int FOLGA = 16;
constexpr int IMAGEM = LADO + 2 * FOLGA;

}  // namespace

void CenarioEmPedacos::intervalo(Rectangle visivel, int& px0, int& py0, int& px1, int& py1) const {
    px0 = std::max(0, static_cast<int>(std::floor((visivel.x - FOLGA) / LADO)));
    py0 = std::max(0, static_cast<int>(std::floor((visivel.y - FOLGA) / LADO)));
    px1 = std::min(colunas_ - 1, static_cast<int>(std::floor((visivel.x + visivel.width + FOLGA) / LADO)));
    py1 = std::min(linhas_ - 1, static_cast<int>(std::floor((visivel.y + visivel.height + FOLGA) / LADO)));
}

void CenarioEmPedacos::esquecer() {
    for (Pedaco& p : pedacos_)
        if (p.imagem.id != 0) UnloadRenderTexture(p.imagem);
    pedacos_.clear();
    colunas_ = linhas_ = 0;
    filtro_ = -1;
}

void CenarioEmPedacos::refazer(Pedaco& p, int px, int py, const Arte& arte, const Cidade& cidade, Estacao estacao,
                               std::uint64_t assinatura) {
    if (p.imagem.id == 0) {
        p.imagem = LoadRenderTexture(IMAGEM, IMAGEM);
        filtro_ = -1;  // a imagem nova precisa receber o filtro atual
    }
    BeginTextureMode(p.imagem);
    ClearBackground(BLANK);
    // A transparência da imagem soma em vez de multiplicar: a neve e as
    // sombras, semitransparentes, não podem deixar o chão translúcido.
    rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD, RL_FUNC_ADD);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    Camera2D cam{};
    cam.target = { static_cast<float>(px * LADO - FOLGA), static_cast<float>(py * LADO - FOLGA) };
    cam.zoom = 1.0f;
    BeginMode2D(cam);
    const int x0 = px * CELULAS_POR_PEDACO, y0 = py * CELULAS_POR_PEDACO;
    arte.desenhar_terreno(cidade, x0, y0, x0 + CELULAS_POR_PEDACO - 1, y0 + CELULAS_POR_PEDACO - 1, estacao);
    EndMode2D();
    EndBlendMode();
    EndTextureMode();
    p.assinatura = assinatura;
    p.pronto = true;
}

void CenarioEmPedacos::atualizar(const Arte& arte, const Cidade& cidade, Estacao estacao, Rectangle visivel) {
    const Mapa& mapa = cidade.mapa();
    const int colunas = (mapa.largura() + CELULAS_POR_PEDACO - 1) / CELULAS_POR_PEDACO;
    const int linhas = (mapa.altura() + CELULAS_POR_PEDACO - 1) / CELULAS_POR_PEDACO;
    if (colunas != colunas_ || linhas != linhas_) {
        esquecer();
        colunas_ = colunas;
        linhas_ = linhas;
        pedacos_.resize(static_cast<std::size_t>(colunas * linhas));
    }
    refeitos_no_ultimo_quadro_ = 0;
    int px0, py0, px1, py1;
    intervalo(visivel, px0, py0, px1, py1);
    int desatualizados_refeitos = 0;
    for (int py = py0; py <= py1; ++py)
        for (int px = px0; px <= px1; ++px) {
            Pedaco& p = pedacos_[static_cast<std::size_t>(py * colunas_ + px)];
            const std::uint64_t a = assinatura_do_pedaco(cidade, px, py, estacao);
            if (p.pronto && p.assinatura == a) continue;
            // Sem imagem nenhuma, faz na hora; desatualizado, só alguns por quadro.
            if (p.pronto && desatualizados_refeitos >= refeitos_por_quadro_) continue;
            if (p.pronto) ++desatualizados_refeitos;
            refazer(p, px, py, arte, cidade, estacao, a);
            ++refeitos_no_ultimo_quadro_;
        }
}

void CenarioEmPedacos::desenhar(Rectangle visivel, float zoom) {
    // De perto, pixel nítido; de longe, suavizado, para o mapa reduzido não
    // cintilar ao mover a câmera.
    const int filtro = zoom >= 1.0f ? TEXTURE_FILTER_POINT : TEXTURE_FILTER_BILINEAR;
    if (filtro != filtro_) {
        for (Pedaco& p : pedacos_)
            if (p.pronto) SetTextureFilter(p.imagem.texture, filtro);
        filtro_ = filtro;
    }
    int px0, py0, px1, py1;
    intervalo(visivel, px0, py0, px1, py1);
    for (int py = py0; py <= py1; ++py)
        for (int px = px0; px <= px1; ++px) {
            Pedaco& p = pedacos_[static_cast<std::size_t>(py * colunas_ + px)];
            if (!p.pronto) continue;
            // A imagem de uma RenderTexture sai de cabeça para baixo: altura
            // negativa na origem a desvira.
            DrawTexturePro(p.imagem.texture, { 0, 0, static_cast<float>(IMAGEM), -static_cast<float>(IMAGEM) },
                           { static_cast<float>(px * LADO - FOLGA), static_cast<float>(py * LADO - FOLGA),
                             static_cast<float>(IMAGEM), static_cast<float>(IMAGEM) },
                           { 0, 0 }, 0, WHITE);
        }
}
