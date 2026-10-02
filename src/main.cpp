#include <string>

#include "raylib.h"

#include "calendario.hpp"
#include "camera.hpp"
#include "cidade.hpp"
#include "desenho.hpp"
#include "ferramentas.hpp"
#include "mapa.hpp"
#include "relogio.hpp"
#include "simulacao.hpp"

namespace {

void ler_comandos(Relogio& relogio) {
    if (IsKeyPressed(KEY_SPACE)) relogio.alternar_pausa();
    if (IsKeyPressed(KEY_ONE)) relogio.definir_indice_velocidade(0);
    if (IsKeyPressed(KEY_TWO)) relogio.definir_indice_velocidade(1);
    if (IsKeyPressed(KEY_THREE)) relogio.definir_indice_velocidade(2);
}

constexpr int LARGURA_MAPA = 128;
constexpr int ALTURA_MAPA = 128;
// Semente fixa enquanto o mapa da campanha não é desenhado à mão.
constexpr unsigned SEMENTE_MAPA = 1150;

void desenhar_painel(const Simulacao& sim, const Relogio& relogio, const Mapa& mapa, Celula sob_cursor,
                     const std::string& dica) {
    const Data& d = sim.data;
    DrawRectangle(0, 0, GetScreenWidth(), 40, Color{ 30, 24, 18, 220 });
    DrawText(TextFormat("%d de %s de %d", d.dia, nome_do_mes(d.mes), d.ano),
             12, 10, 20, RAYWHITE);

    const char* ritmo = relogio.pausado() ? "PAUSADO"
                                          : TextFormat("%dx", relogio.velocidade());
    DrawText(ritmo, 340, 10, 20, relogio.pausado() ? GOLD : RAYWHITE);
    DrawText(TextFormat("tick %llu  |  %d fps",
                        static_cast<unsigned long long>(sim.tick), GetFPS()),
             GetScreenWidth() - 260, 12, 16, LIGHTGRAY);

    if (mapa.dentro(sob_cursor.x, sob_cursor.y)) {
        DrawText(TextFormat("(%d, %d) %s", sob_cursor.x, sob_cursor.y,
                            nome_do_terreno(mapa.terreno(sob_cursor.x, sob_cursor.y))),
                 460, 12, 16, RAYWHITE);
    }
    if (!dica.empty()) DrawText(dica.c_str(), 680, 12, 16, Color{ 255, 220, 160, 255 });
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Hansestadt");
    // Esc solta a ferramenta; fechar o jogo fica só com o botão da janela.
    SetExitKey(KEY_NULL);

    Cidade cidade(gerar_mapa(LARGURA_MAPA, ALTURA_MAPA, SEMENTE_MAPA));
    const Mapa& mapa = cidade.mapa();
    EstadoFerramenta ferramenta;
    CameraJogo camera(static_cast<float>(LARGURA_MAPA * TAMANHO_CELULA),
                      static_cast<float>(ALTURA_MAPA * TAMANHO_CELULA));
    Simulacao sim;
    Relogio relogio(1.0 / Simulacao::TICKS_POR_SEGUNDO);

    while (!WindowShouldClose()) {
        ler_comandos(relogio);
        ler_atalhos(ferramenta);
        const bool mouse_na_barra = atualizar_barra(ferramenta);
        camera.atualizar(GetFrameTime());
        const Celula sob_cursor = celula_em(camera.mouse_no_mundo());
        if (!mouse_na_barra) aplicar_ferramenta(ferramenta, cidade, sob_cursor);

        const int passos = relogio.passos_para(GetFrameTime());
        for (int i = 0; i < passos; ++i) sim.passo();

        BeginDrawing();
        ClearBackground(Color{ 20, 26, 34, 255 });

        BeginMode2D(camera.camera2d());
        desenhar_mapa(mapa, camera.area_visivel(), camera.camera2d().zoom);
        desenhar_cidade(cidade, camera.area_visivel(), camera.camera2d().zoom);
        if (!mouse_na_barra) {
            desenhar_previa(ferramenta, cidade, sob_cursor);
            if (mapa.dentro(sob_cursor.x, sob_cursor.y)) destacar_celula(sob_cursor);
        }
        EndMode2D();

        desenhar_painel(sim, relogio, mapa, sob_cursor,
                        mouse_na_barra ? std::string{} : dica_da_ferramenta(ferramenta, cidade, sob_cursor));
        desenhar_barra(ferramenta);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
