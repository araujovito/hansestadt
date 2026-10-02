#include "ferramentas.hpp"

#include <array>

#include "desenho.hpp"
#include "raylib.h"

namespace {

constexpr float ALTURA_BARRA = 48.0f;
constexpr float LARGURA_BOTAO = 132.0f;
constexpr float MARGEM = 6.0f;

struct Botao {
    const char* rotulo;
    Ferramenta ferramenta;
    TipoEdificio tipo;
};

constexpr std::array BOTOES = {
    Botao{ "estrada (R)", Ferramenta::Estrada, TipoEdificio::Armazem },
    Botao{ "armazém", Ferramenta::Edificio, TipoEdificio::Armazem },
    Botao{ "casa", Ferramenta::Edificio, TipoEdificio::Casa },
    Botao{ "lenhador", Ferramenta::Edificio, TipoEdificio::Lenhador },
    Botao{ "carvoaria", Ferramenta::Edificio, TipoEdificio::Carvoaria },
    Botao{ "poço de argila", Ferramenta::Edificio, TipoEdificio::PocoDeArgila },
    Botao{ "turfa", Ferramenta::Edificio, TipoEdificio::CortadorDeTurfa },
    Botao{ "olaria", Ferramenta::Edificio, TipoEdificio::Olaria },
    Botao{ "demolir (X)", Ferramenta::Demolir, TipoEdificio::Armazem },
};

Rectangle area_da_barra() {
    const auto h = static_cast<float>(GetScreenHeight());
    return { 0.0f, h - ALTURA_BARRA, static_cast<float>(GetScreenWidth()), ALTURA_BARRA };
}

Rectangle area_do_botao(std::size_t i) {
    const Rectangle barra = area_da_barra();
    return { MARGEM + static_cast<float>(i) * (LARGURA_BOTAO + MARGEM), barra.y + MARGEM,
             LARGURA_BOTAO, ALTURA_BARRA - 2.0f * MARGEM };
}

bool selecionado(const EstadoFerramenta& estado, const Botao& b) {
    if (estado.ativa != b.ferramenta) return false;
    return b.ferramenta != Ferramenta::Edificio || estado.tipo == b.tipo;
}

// Com o cursor no centro do edifício, e não no canto, a prévia fica onde o
// jogador está olhando.
Celula origem_centrada(TipoEdificio tipo, Celula cursor) {
    const DefinicaoEdificio& def = definicao(tipo);
    return { cursor.x - (def.largura - 1) / 2, cursor.y - (def.altura - 1) / 2 };
}

Rectangle retangulo_mundo(Celula origem, int largura, int altura) {
    return { static_cast<float>(origem.x * TAMANHO_CELULA), static_cast<float>(origem.y * TAMANHO_CELULA),
             static_cast<float>(largura * TAMANHO_CELULA), static_cast<float>(altura * TAMANHO_CELULA) };
}

constexpr Color PREVIA_OK{ 120, 220, 120, 110 };
constexpr Color PREVIA_ERRO{ 230, 80, 70, 120 };

bool estrada_aceitavel(Validacao v) {
    return v == Validacao::Ok || v == Validacao::JaExiste;
}

}  // namespace

void ler_atalhos(EstadoFerramenta& estado) {
    if (IsKeyPressed(KEY_R)) estado.ativa = Ferramenta::Estrada;
    if (IsKeyPressed(KEY_X)) estado.ativa = Ferramenta::Demolir;
    if (IsKeyPressed(KEY_ESCAPE)) estado.ativa = Ferramenta::Nenhuma;
    if (estado.ativa != Ferramenta::Estrada) estado.inicio_estrada.reset();
}

bool atualizar_barra(EstadoFerramenta& estado) {
    const Vector2 mouse = GetMousePosition();
    if (!CheckCollisionPointRec(mouse, area_da_barra())) return false;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (std::size_t i = 0; i < BOTOES.size(); ++i) {
            if (CheckCollisionPointRec(mouse, area_do_botao(i))) {
                estado.ativa = BOTOES[i].ferramenta;
                estado.tipo = BOTOES[i].tipo;
                estado.inicio_estrada.reset();
            }
        }
    }
    return true;
}

void aplicar_ferramenta(EstadoFerramenta& estado, Cidade& cidade, Celula cursor) {
    switch (estado.ativa) {
        case Ferramenta::Nenhuma:
            break;
        case Ferramenta::Estrada:
            // A estrada só é construída ao soltar o botão, para o jogador
            // poder ajustar o traçado olhando a prévia.
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) estado.inicio_estrada = cursor;
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && estado.inicio_estrada) {
                for (const Celula c : caminho_em_l(*estado.inicio_estrada, cursor)) {
                    cidade.construir_estrada(c);
                }
                estado.inicio_estrada.reset();
            }
            break;
        case Ferramenta::Edificio:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                cidade.construir_edificio(estado.tipo, origem_centrada(estado.tipo, cursor));
            }
            break;
        case Ferramenta::Demolir:
            // Segurar e arrastar demole várias células seguidas.
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) cidade.demolir(cursor);
            break;
    }
}

void desenhar_previa(const EstadoFerramenta& estado, const Cidade& cidade, Celula cursor) {
    switch (estado.ativa) {
        case Ferramenta::Nenhuma:
            break;
        case Ferramenta::Estrada: {
            const Celula inicio = estado.inicio_estrada.value_or(cursor);
            for (const Celula c : caminho_em_l(inicio, cursor)) {
                const bool ok = estrada_aceitavel(cidade.validar_estrada(c));
                DrawRectangleRec(retangulo_mundo(c, 1, 1), ok ? PREVIA_OK : PREVIA_ERRO);
            }
            break;
        }
        case Ferramenta::Edificio: {
            const DefinicaoEdificio& def = definicao(estado.tipo);
            const Celula origem = origem_centrada(estado.tipo, cursor);
            const bool ok = cidade.validar_edificio(estado.tipo, origem) == Validacao::Ok;
            const Rectangle r = retangulo_mundo(origem, def.largura, def.altura);
            DrawRectangleRec(r, ok ? PREVIA_OK : PREVIA_ERRO);
            DrawRectangleLinesEx(r, 2.0f, ok ? Color{ 200, 255, 200, 220 } : Color{ 255, 170, 160, 220 });
            break;
        }
        case Ferramenta::Demolir: {
            const IdEdificio id = cidade.edificio_em(cursor);
            if (id != SEM_EDIFICIO) {
                const Edificio& e = cidade.edificios()[id];
                const DefinicaoEdificio& def = definicao(e.tipo);
                DrawRectangleRec(retangulo_mundo(e.origem, def.largura, def.altura), PREVIA_ERRO);
            } else if (cidade.tem_estrada(cursor)) {
                DrawRectangleRec(retangulo_mundo(cursor, 1, 1), PREVIA_ERRO);
            }
            break;
        }
    }
}

void desenhar_barra(const EstadoFerramenta& estado) {
    DrawRectangleRec(area_da_barra(), Color{ 30, 24, 18, 230 });
    const Vector2 mouse = GetMousePosition();
    for (std::size_t i = 0; i < BOTOES.size(); ++i) {
        const Rectangle r = area_do_botao(i);
        const bool ativo = selecionado(estado, BOTOES[i]);
        const bool sob_mouse = CheckCollisionPointRec(mouse, r);
        const Color fundo = ativo ? Color{ 150, 110, 60, 255 }
                          : sob_mouse ? Color{ 80, 64, 48, 255 }
                                      : Color{ 58, 48, 38, 255 };
        DrawRectangleRec(r, fundo);
        const int largura_texto = MeasureText(BOTOES[i].rotulo, 16);
        DrawText(BOTOES[i].rotulo, static_cast<int>(r.x + (r.width - static_cast<float>(largura_texto)) / 2.0f),
                 static_cast<int>(r.y + 10.0f), 16, RAYWHITE);
    }
}

std::string dica_da_ferramenta(const EstadoFerramenta& estado, const Cidade& cidade, Celula cursor) {
    if (estado.ativa != Ferramenta::Edificio) return {};
    const DefinicaoEdificio& def = definicao(estado.tipo);
    const Validacao v = cidade.validar_edificio(estado.tipo, origem_centrada(estado.tipo, cursor));
    std::string dica = std::string(def.nome) + " (" + std::to_string(def.largura) + "x" +
                       std::to_string(def.altura) + ")";
    if (v == Validacao::TerrenoErrado) {
        dica += ": precisa de " + std::string(nome_do_terreno(def.terreno_exigido));
    } else if (v != Validacao::Ok) {
        dica += std::string(": ") + descrever(v);
    }
    return dica;
}
