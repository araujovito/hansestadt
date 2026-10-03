#include <array>
#include <cstdio>
#include <string>

#include "raylib.h"

#include "alertas.hpp"
#include "arte.hpp"
#include "calendario.hpp"
#include "camera.hpp"
#include "cidade.hpp"
#include "conteudo.hpp"
#include "desenho.hpp"
#include "ferramentas.hpp"
#include "fundacao.hpp"
#include "historico.hpp"
#include "mapa.hpp"
#include "menu.hpp"
#include "painel.hpp"
#include "partidas.hpp"
#include "pedacos.hpp"
#include "relatorio_tela.hpp"
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

    // Segunda linha: cada trecho começa onde o anterior terminou. Com
    // posições fixas, um número com mais dígitos fazia um trecho cobrir o
    // seguinte ("mortos" ficava por baixo de "comida").
    constexpr int TAMANHO = 16;
    int x = 12;
    auto escrever = [&x](const char* texto, Color cor) {
        if (x > 12) {
            DrawText("|", x, 40, TAMANHO, GRAY);
            x += MeasureText("|", TAMANHO) + 8;
        }
        DrawText(texto, x, 40, TAMANHO, cor);
        x += MeasureText(texto, TAMANHO) + 8;
    };
    escrever(TextFormat("população %d (%d crianças)", sim.populacao.total(), sim.populacao.criancas()), LIGHTGRAY);
    escrever(TextFormat("sem casa %d", sim.populacao.sem_casa()), LIGHTGRAY);
    escrever(TextFormat("sem trabalho %d", sim.populacao.desempregados()), LIGHTGRAY);
    escrever(TextFormat("mortos %d", sim.populacao.mortos_de_fome()), LIGHTGRAY);

    const int comida = sim.cidade.estoque_total(TipoItem::Comida);
    // Sem moradores não há reserva em dias para medir: "0 dias" em vermelho
    // com o armazém cheio dava a impressão de fome.
    if (sim.populacao.total() == 0) {
        escrever(TextFormat("comida %d", comida), LIGHTGRAY);
    } else {
        const int dias = comida / sim.populacao.total();
        const Color cor_comida = dias < 30 ? Color{ 240, 110, 90, 255 }
                               : dias < 90 ? GOLD
                                          : LIGHTGRAY;
        escrever(TextFormat("comida %d (%d dias)", comida, dias), cor_comida);
    }
    escrever(TextFormat("carregadores %d/%d", ocupados, static_cast<int>(sim.transporte.carregadores().size())),
             LIGHTGRAY);
    const char* fps = TextFormat("%d fps", GetFPS());
    DrawText(fps, GetScreenWidth() - MeasureText(fps, TAMANHO) - 12, 40, TAMANHO, GRAY);

    if (mapa.dentro(sob_cursor.x, sob_cursor.y)) {
        DrawText(TextFormat("(%d, %d) %s", sob_cursor.x, sob_cursor.y,
                            nome_do_terreno(mapa.terreno(sob_cursor.x, sob_cursor.y))),
                 390, 12, 16, RAYWHITE);
    }
    if (!dica.empty()) DrawText(dica.c_str(), 680, 12, 16, Color{ 255, 220, 160, 255 });
}

// Botão no canto da barra de cima, para quem não sabe que o Esc abre o menu.
Rectangle area_do_botao_menu() {
    return { static_cast<float>(GetScreenWidth()) - 110.0f, 6.0f, 100.0f, 26.0f };
}

Rectangle area_do_botao_relatorio() {
    return { static_cast<float>(GetScreenWidth()) - 240.0f, 6.0f, 124.0f, 26.0f };
}

void desenhar_botao(Rectangle r, const char* rotulo, bool ativo) {
    const bool sob_mouse = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRec(r, ativo ? Color{ 150, 110, 60, 255 } : sob_mouse ? Color{ 80, 64, 48, 255 } : Color{ 58, 48, 38, 255 });
    DrawText(rotulo, static_cast<int>(r.x + 8.0f), static_cast<int>(r.y + 6.0f), 16, RAYWHITE);
}

std::string salvar(const Simulacao& sim, const Fundacao& fundacao, const std::string& arquivo) {
    if (!fundacao.concluida()) return "não foi possível salvar: funde a vila primeiro";
    std::string erro;
    return salvar_partida(sim, arquivo, erro) ? "partida salva" : "não foi possível salvar: " + erro;
}

// Troca a partida em andamento pela do arquivo. A fundação e a barra de
// ferramentas não vão para o arquivo: são refeitas a partir da cidade.
// Os eventos históricos também não vão para o arquivo (são dados do jogo) e
// precisam ser postos de volta; sem isso a partida carregada ficaria sem
// Grande Fome e sem Peste.
bool carregar(Simulacao& sim, Fundacao& fundacao, EstadoFerramenta& ferramenta, Fase& fase_da_barra,
              const Conteudo& conteudo, const Eventos& eventos, const std::string& arquivo, std::string& mensagem) {
    ResultadoSalvamento carga = carregar_partida(arquivo, conteudo);
    if (!carga.simulacao) {
        mensagem = "não foi possível carregar: " + carga.erro;
        return false;
    }
    sim = std::move(*carga.simulacao);
    sim.eventos = eventos;
    IdEdificio armazem = SEM_EDIFICIO;
    for (IdEdificio id = 0; id < sim.cidade.edificios().size(); ++id)
        if (sim.cidade.edificio(id).ativo && sim.cidade.definicao(id).deposito) { armazem = id; break; }
    fundacao.restaurar(armazem);
    ferramenta = criar_ferramentas(conteudo, sim.fase);
    fase_da_barra = sim.fase;
    mensagem = "partida carregada";
    return true;
}

// Rodando da raiz do projeto (make run), os dados estão em ./dados; rodando
// o binário direto, ficam dois níveis acima dele (build/<modo>/).
std::string pasta_raiz() {
    if (DirectoryExists("dados")) return ".";
    return std::string(GetApplicationDirectory()) + "../..";
}
std::string pasta_de_dados() { return pasta_raiz() + "/dados"; }

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
    // Esc solta a ferramenta ou abre o menu; fechar o jogo fica com o botão
    // da janela ou com "Sair do jogo" no menu.
    SetExitKey(KEY_NULL);

    Arte arte;
    {
        std::string erro;
        if (!arte.carregar(pasta_raiz(), erro)) {
            std::fprintf(stderr, "erro ao carregar a arte: %s\n", erro.c_str());
            CloseWindow();
            return 1;
        }
    }
    CenarioEmPedacos cenario;

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
    MenuDePausa menu;
    JanelaDeRelatorio relatorio;
    std::vector<Alerta> alertas;
    // Por tipo de alerta, qual edifício da lista o próximo clique mostra:
    // cliques seguidos percorrem os edifícios um a um.
    std::array<std::size_t, 6> proximo_do_alerta{};
    const std::string& arquivo_rapido = vagas_de_partida().front().arquivo;
    bool sair = false;

    while (!sair && !WindowShouldClose()) {
        // Com o menu aberto o jogo para: nada de atalhos, câmera, cliques no
        // mapa nem passos da simulação.
        bool mouse_na_barra = true;
        Celula sob_cursor{ -1, -1 };
        // O quadro em que o menu fecha também fica sem entrada do jogo: o
        // clique em "Continuar" não pode cair no mapa por baixo.
        const bool menu_estava_aberto = menu.aberto();
        if (menu_estava_aberto) {
            const PedidoDoMenu pedido = menu.atualizar();
            switch (pedido.acao) {
                case PedidoDoMenu::Acao::Nenhuma:
                    break;
                case PedidoDoMenu::Acao::Salvar:
                    menu.informar(salvar(sim, fundacao, pedido.arquivo));
                    menu.reexaminar();
                    break;
                case PedidoDoMenu::Acao::Carregar:
                    if (carregar(sim, fundacao, ferramenta, fase_da_barra, conteudo, configuracao_eventos, pedido.arquivo, aviso)) {
                        relatorio.esquecer_historico();
                        cenario.esquecer();
                        menu.fechar();
                        aviso_quadros = 180;
                    } else {
                        menu.informar(aviso);  // fica no menu para escolher outra vaga
                    }
                    break;
                case PedidoDoMenu::Acao::Sair:
                    sair = true;
                    break;
            }
        } else {
            // Esc é de quem estiver usando: primeiro solta a ferramenta e fecha
            // o painel do edifício; só com nada disso abre o menu.
            const bool clicou_menu = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                                     CheckCollisionPointRec(GetMousePosition(), area_do_botao_menu());
            const bool esc_livre = IsKeyPressed(KEY_ESCAPE) && ferramenta.ativa == Ferramenta::Nenhuma &&
                                   !ferramenta.selecionado;
            // Com o relatório aberto, o Esc livre fecha o relatório primeiro.
            if (esc_livre && relatorio.aberta()) relatorio.fechar();
            else if (clicou_menu || esc_livre) menu.abrir(conteudo, fundacao.concluida());
        }

        if (!menu_estava_aberto && !menu.aberto()) {
            ler_comandos(relogio);
            if (IsKeyPressed(KEY_F5)) {
                aviso = salvar(sim, fundacao, arquivo_rapido);
                aviso_quadros = 180;
            }
            if (IsKeyPressed(KEY_F9)) {
                if (carregar(sim, fundacao, ferramenta, fase_da_barra, conteudo, configuracao_eventos, arquivo_rapido, aviso)) {
                    relatorio.esquecer_historico();
                    cenario.esquecer();
                }
                aviso_quadros = 180;
            }
            ler_atalhos(ferramenta);
            const bool clicou_relatorio = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                                          CheckCollisionPointRec(GetMousePosition(), area_do_botao_relatorio());
            if (IsKeyPressed(KEY_V) || clicou_relatorio) relatorio.alternar();
            relatorio.atualizar();
            // A lista de alertas fica escondida atrás do relatório aberto.
            if (!relatorio.aberta() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                const int i = alerta_sob_mouse(alertas);
                if (i >= 0 && !alertas[static_cast<std::size_t>(i)].edificios.empty()) {
                    const Alerta& a = alertas[static_cast<std::size_t>(i)];
                    std::size_t& proximo = proximo_do_alerta[static_cast<std::size_t>(a.tipo)];
                    const IdEdificio id = a.edificios[proximo++ % a.edificios.size()];
                    const Edificio& e = cidade.edificio(id);
                    const DefinicaoEdificio& def = cidade.definicao(id);
                    camera.centralizar({ (static_cast<float>(e.origem.x) + def.largura_total() / 2.0f) * TAMANHO_CELULA,
                                         (static_cast<float>(e.origem.y) + def.altura_total() / 2.0f) * TAMANHO_CELULA });
                    ferramenta.ativa = Ferramenta::Nenhuma;
                    ferramenta.selecionado = id;
                }
            }
            const bool painel_aberto = ferramenta.selecionado && cidade.edificio(*ferramenta.selecionado).ativo;
            // Clique sobre a barra, o painel ou o botão do menu não pode
            // atravessar para o mapa.
            mouse_na_barra = atualizar_barra(ferramenta) ||
                             (painel_aberto && CheckCollisionPointRec(GetMousePosition(),
                                                                      area_do_painel_edificio())) ||
                             CheckCollisionPointRec(GetMousePosition(), area_do_botao_menu()) ||
                             CheckCollisionPointRec(GetMousePosition(), area_do_botao_relatorio()) ||
                             (relatorio.aberta() ? CheckCollisionPointRec(GetMousePosition(), relatorio.area())
                                                 : mouse_sobre_alertas(alertas));
            camera.atualizar(GetFrameTime());
            sob_cursor = celula_em(camera.mouse_no_mundo());
            if (!mouse_na_barra) aplicar_ferramenta(ferramenta, cidade, sob_cursor);
            // Botão do painel ou tecla U, que fica como atalho.
            if (ferramenta.selecionado && cidade.edificio(*ferramenta.selecionado).ativo &&
                (IsKeyPressed(KEY_U) || clicou_em_melhorar(cidade, *ferramenta.selecionado))) {
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
                const Data antes_dos_passos = sim.data;
                for (int i = 0; i < passos; ++i) sim.passo();
                // A data de antes é deste quadro, então carregar uma partida
                // de outro ano não anuncia eventos que já tinham passado.
                for (const EventoHistorico* e : comecaram_entre(sim.eventos, antes_dos_passos, sim.data)) {
                    aviso = "começou: " + e->nome + " (Relatório > História)";
                    aviso_quadros = 300;
                }
            }
        }
        // Antes da fundação não há vila para ter problema.
        if (fundacao.concluida()) {
            alertas = levantar_alertas(sim);
            relatorio.registrar_dia(sim);
        } else {
            alertas.clear();
        }
        if (fundacao.concluida() && fase_da_barra != sim.fase) {
            ferramenta = criar_ferramentas(conteudo, sim.fase);
            fase_da_barra = sim.fase;
            aviso = std::string("nova fase: ") + nome_da_fase(sim.fase);
            aviso_quadros = 240;
        }

        // Pedaços do cenário que mudaram são redesenhados antes do quadro:
        // desenhar num pedaço troca o alvo do desenho.
        cenario.atualizar(arte, cidade, estacao_do_mes(sim.data.mes), camera.area_visivel());

        BeginDrawing();
        ClearBackground(Color{ 20, 26, 34, 255 });

        BeginMode2D(camera.camera2d());
        cenario.desenhar(camera.area_visivel(), camera.camera2d().zoom);
        arte.desenhar_edificios(cidade, sim.fase, sim.data.mes, camera.area_visivel());
        arte.desenhar_carregadores(cidade, sim.transporte, camera.area_visivel());
        desenhar_avisos_sem_acesso(cidade, camera.area_visivel());
        if (!mouse_na_barra) {
            desenhar_previa(ferramenta, cidade, sob_cursor);
            if (mapa.dentro(sob_cursor.x, sob_cursor.y)) destacar_celula(sob_cursor);
            if (ferramenta.ativa == Ferramenta::Nenhuma) desenhar_nome_sob_cursor(cidade, sob_cursor, camera.camera2d().zoom);
        }
        EndMode2D();

        std::string dica = mouse_na_barra ? std::string{} : dica_da_ferramenta(ferramenta, cidade, sob_cursor);
        if (!fundacao.concluida() && dica.empty()) dica = "escolha onde fundar a vila";
        if (aviso_quadros > 0) { dica = aviso; --aviso_quadros; }
        desenhar_painel(sim, relogio, mapa, sob_cursor, dica);
        desenhar_botao(area_do_botao_relatorio(), "Relatório (V)", relatorio.aberta());
        desenhar_botao(area_do_botao_menu(), "Menu (Esc)", false);
        if (relatorio.aberta()) relatorio.desenhar(montar_relatorio(sim));
        else desenhar_alertas(alertas);
        desenhar_barra(ferramenta);
        if (ferramenta.selecionado && cidade.edificio(*ferramenta.selecionado).ativo) {
            desenhar_painel_edificio(cidade, sim.populacao, *ferramenta.selecionado, sim.data.mes,
                                     Simulacao::TICKS_POR_DIA, sim.fase);
        }
        menu.desenhar();
        EndDrawing();
    }

    // As imagens da placa de vídeo saem antes de fechar a janela, que leva a
    // placa junto.
    cenario.esquecer();
    arte.descarregar();
    CloseWindow();
    return 0;
}
