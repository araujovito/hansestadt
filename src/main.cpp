#include <cstdio>
#include <string>

#include "raylib.h"

#include "calendario.hpp"
#include "camera.hpp"
#include "cidade.hpp"
#include "conteudo.hpp"
#include "desenho.hpp"
#include "ferramentas.hpp"
#include "fundacao.hpp"
#include "mapa.hpp"
#include "painel.hpp"
#include "relogio.hpp"
#include "salvamento.hpp"
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
constexpr const char* ARQUIVO_PARTIDA = "partida.hanse";

void desenhar_painel(const Simulacao& sim, const Relogio& relogio, const Mapa& mapa, Celula sob_cursor,
                     const std::string& dica) {
    const Data& d = sim.data;
    DrawRectangle(0, 0, GetScreenWidth(), 64, Color{ 30, 24, 18, 230 });
    DrawText(TextFormat("%d de %s de %d", d.dia, nome_do_mes(d.mes), d.ano),
             12, 10, 20, RAYWHITE);

    const char* ritmo = relogio.pausado() ? "PAUSADO"
                                          : TextFormat("%dx", relogio.velocidade());
    DrawText(ritmo, 300, 10, 20, relogio.pausado() ? GOLD : RAYWHITE);
    int ocupados = 0;
    for (const Carregador& c : sim.transporte.carregadores()) ocupados += c.livre() ? 0 : 1;
    DrawText(TextFormat("população %d (%d crianças) | sem casa %d | sem trabalho %d | mortos %d",
                        sim.populacao.total(), sim.populacao.criancas(), sim.populacao.sem_casa(),
                        sim.populacao.desempregados(), sim.populacao.mortos_de_fome()),
             12, 40, 16, LIGHTGRAY);

    const int comida = sim.cidade.estoque_total(TipoItem::Comida);
    const int dias = sim.populacao.total() > 0 ? comida / sim.populacao.total() : 0;
    const Color cor_comida = dias < 30 ? Color{ 240, 110, 90, 255 }
                           : dias < 90 ? GOLD
                                      : LIGHTGRAY;
    DrawText(TextFormat("comida %d (%d dias)", comida, dias), 455, 40, 16, cor_comida);
    DrawText(TextFormat("carregadores %d/%d | %d fps", ocupados,
                        static_cast<int>(sim.transporte.carregadores().size()), GetFPS()),
             GetScreenWidth() - 250, 40, 16, LIGHTGRAY);

    if (mapa.dentro(sob_cursor.x, sob_cursor.y)) {
        DrawText(TextFormat("(%d, %d) %s", sob_cursor.x, sob_cursor.y,
                            nome_do_terreno(mapa.terreno(sob_cursor.x, sob_cursor.y))),
                 390, 12, 16, RAYWHITE);
    }
    if (!dica.empty()) DrawText(dica.c_str(), 680, 12, 16, Color{ 255, 220, 160, 255 });
}

// Rodando da raiz do projeto (make run), os dados estão em ./dados; rodando
// o binário direto, ficam dois níveis acima dele (build/<modo>/).
std::string pasta_de_dados() {
    if (DirectoryExists("dados")) return "dados";
    return std::string(GetApplicationDirectory()) + "../../dados";
}

}  // namespace

int main() {
    const ResultadoCarga carga = carregar_conteudo(pasta_de_dados());
    if (!carga.conteudo) {
        std::fprintf(stderr, "erro ao carregar o conteúdo do jogo:\n");
        for (const std::string& erro : carga.erros) std::fprintf(stderr, "  %s\n", erro.c_str());
        return 1;
    }
    const Conteudo& conteudo = *carga.conteudo;
    const ResultadoEventos carga_eventos = carregar_eventos(pasta_de_dados(), conteudo);
    if (!carga_eventos.eventos) {
        std::fprintf(stderr, "erro ao carregar eventos históricos:\n");
        for (const std::string& erro : carga_eventos.erros) std::fprintf(stderr, "  %s\n", erro.c_str());
        return 1;
    }
    const Eventos configuracao_eventos(*carga_eventos.eventos);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Hansestadt");
    // Esc solta a ferramenta; fechar o jogo fica só com o botão da janela.
    SetExitKey(KEY_NULL);

    Simulacao sim(Cidade(gerar_mapa(LARGURA_MAPA, ALTURA_MAPA, SEMENTE_MAPA), conteudo));
    sim.eventos = configuracao_eventos;
    Cidade& cidade = sim.cidade;
    const Mapa& mapa = cidade.mapa();
    // A campanha começa na vila; os marcos históricos e as instituições
    // determinam quando as ferramentas de cada fase são liberadas.
    Fundacao fundacao(conteudo);
    EstadoFerramenta ferramenta = criar_ferramentas_fundacao(conteudo);
    Fase fase_da_barra = sim.fase;
    CameraJogo camera(static_cast<float>(LARGURA_MAPA * TAMANHO_CELULA),
                      static_cast<float>(ALTURA_MAPA * TAMANHO_CELULA));
    Relogio relogio(1.0 / Simulacao::TICKS_POR_SEGUNDO);
    std::string aviso;
    int aviso_quadros = 0;

    while (!WindowShouldClose()) {
        ler_comandos(relogio);
        if (IsKeyPressed(KEY_F5)) {
            std::string erro;
            aviso = fundacao.concluida() && salvar_partida(sim, ARQUIVO_PARTIDA, erro)
                        ? "partida salva"
                        : "não foi possível salvar: " + (erro.empty() ? std::string("funde a vila primeiro") : erro);
            aviso_quadros = 180;
        }
        if (IsKeyPressed(KEY_F9)) {
            ResultadoSalvamento carga_partida = carregar_partida(ARQUIVO_PARTIDA, conteudo);
            if (carga_partida.simulacao) {
                sim = std::move(*carga_partida.simulacao);
                sim.eventos = configuracao_eventos;
                IdEdificio armazem = SEM_EDIFICIO;
                for (IdEdificio id = 0; id < sim.cidade.edificios().size(); ++id)
                    if (sim.cidade.edificio(id).ativo && sim.cidade.definicao(id).deposito) { armazem = id; break; }
                fundacao.restaurar(armazem);
                ferramenta = criar_ferramentas(conteudo, sim.fase);
                fase_da_barra = sim.fase;
                aviso = "partida carregada";
            } else aviso = "não foi possível carregar: " + carga_partida.erro;
            aviso_quadros = 180;
        }
        ler_atalhos(ferramenta);
        const bool painel_aberto = ferramenta.selecionado && cidade.edificio(*ferramenta.selecionado).ativo;
        // Clique sobre a barra ou o painel não pode atravessar para o mapa.
        const bool mouse_na_barra = atualizar_barra(ferramenta) ||
                                    (painel_aberto && CheckCollisionPointRec(GetMousePosition(),
                                                                             area_do_painel_edificio()));
        camera.atualizar(GetFrameTime());
        const Celula sob_cursor = celula_em(camera.mouse_no_mundo());
        if (!mouse_na_barra) aplicar_ferramenta(ferramenta, cidade, sob_cursor);
        if (IsKeyPressed(KEY_U) && ferramenta.selecionado) {
            const ResultadoMelhoria r = cidade.melhorar_edificio(*ferramenta.selecionado, sim.fase);
            aviso = r == ResultadoMelhoria::Ok
                        ? "instituição evoluiu para o nível " +
                              std::to_string(cidade.edificio(*ferramenta.selecionado).nivel)
                        : descrever(r);
            aviso_quadros = 180;
        }
        if (fundacao.concluir_se_possivel(cidade)) {
            ferramenta = criar_ferramentas(conteudo, sim.fase);
            fase_da_barra = sim.fase;
        }

        const int passos = relogio.passos_para(GetFrameTime());
        if (fundacao.concluida()) {
            for (int i = 0; i < passos; ++i) sim.passo();
        }
        if (fundacao.concluida() && fase_da_barra != sim.fase) {
            ferramenta = criar_ferramentas(conteudo, sim.fase);
            fase_da_barra = sim.fase;
            aviso = std::string("nova fase: ") + nome_da_fase(sim.fase);
            aviso_quadros = 240;
        }

        BeginDrawing();
        ClearBackground(Color{ 20, 26, 34, 255 });

        BeginMode2D(camera.camera2d());
        desenhar_mapa(mapa, camera.area_visivel(), camera.camera2d().zoom);
        desenhar_cidade(cidade, camera.area_visivel(), camera.camera2d().zoom, sim.data.mes);
        desenhar_carregadores(cidade, sim.transporte, camera.area_visivel());
        if (!mouse_na_barra) {
            desenhar_previa(ferramenta, cidade, sob_cursor);
            if (mapa.dentro(sob_cursor.x, sob_cursor.y)) destacar_celula(sob_cursor);
        }
        EndMode2D();

        std::string dica = mouse_na_barra ? std::string{} : dica_da_ferramenta(ferramenta, cidade, sob_cursor);
        if (!fundacao.concluida() && dica.empty()) dica = "escolha onde fundar a vila";
        if (aviso_quadros > 0) { dica = aviso; --aviso_quadros; }
        desenhar_painel(sim, relogio, mapa, sob_cursor, dica);
        desenhar_barra(ferramenta);
        if (ferramenta.selecionado && cidade.edificio(*ferramenta.selecionado).ativo) {
            desenhar_painel_edificio(cidade, sim.populacao, *ferramenta.selecionado, sim.data.mes,
                                     Simulacao::TICKS_POR_DIA);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
