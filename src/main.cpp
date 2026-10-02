#include "raylib.h"

#include "calendario.hpp"
#include "relogio.hpp"
#include "simulacao.hpp"

namespace {

void ler_comandos(Relogio& relogio) {
    if (IsKeyPressed(KEY_SPACE)) relogio.alternar_pausa();
    if (IsKeyPressed(KEY_ONE)) relogio.definir_indice_velocidade(0);
    if (IsKeyPressed(KEY_TWO)) relogio.definir_indice_velocidade(1);
    if (IsKeyPressed(KEY_THREE)) relogio.definir_indice_velocidade(2);
}

void desenhar_painel(const Simulacao& sim, const Relogio& relogio) {
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
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Hansestadt");

    Simulacao sim;
    Relogio relogio(1.0 / Simulacao::TICKS_POR_SEGUNDO);

    while (!WindowShouldClose()) {
        ler_comandos(relogio);

        const int passos = relogio.passos_para(GetFrameTime());
        for (int i = 0; i < passos; ++i) sim.passo();

        BeginDrawing();
        ClearBackground(Color{ 74, 110, 60, 255 });
        desenhar_painel(sim, relogio);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
