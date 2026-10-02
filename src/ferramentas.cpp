#include "ferramentas.hpp"

#include "desenho.hpp"
#include "raylib.h"

namespace {

constexpr float ALTURA_BARRA = 48.0f;
constexpr float MARGEM = 6.0f;
constexpr float FOLGA_TEXTO = 24.0f;
constexpr int TAMANHO_TEXTO = 16;

Rectangle area_da_barra() {
    const auto h = static_cast<float>(GetScreenHeight());
    return { 0.0f, h - ALTURA_BARRA, static_cast<float>(GetScreenWidth()), ALTURA_BARRA };
}

// Os botões têm a largura do próprio rótulo, porque os nomes vêm do
// conteúdo e variam de "casa" a "moinho d'água".
std::vector<Rectangle> areas_dos_botoes(const std::vector<Botao>& botoes) {
    const Rectangle barra = area_da_barra();
    std::vector<Rectangle> areas;
    float x = MARGEM;
    for (const Botao& b : botoes) {
        const float largura = static_cast<float>(MeasureText(b.rotulo.c_str(), TAMANHO_TEXTO)) + FOLGA_TEXTO;
        areas.push_back({ x, barra.y + MARGEM, largura, ALTURA_BARRA - 2.0f * MARGEM });
        x += largura + MARGEM;
    }
    return areas;
}

bool selecionado(const EstadoFerramenta& estado, const Botao& b) {
    if (estado.ativa != b.ferramenta) return false;
    return b.ferramenta != Ferramenta::Edificio || estado.tipo == b.tipo;
}

// Com o cursor no centro do edifício, e não no canto, a prévia fica onde o
// jogador está olhando.
Celula origem_centrada(const DefinicaoEdificio& def, Celula cursor) {
    return { cursor.x - (def.largura_total() - 1) / 2, cursor.y - (def.altura_total() - 1) / 2 };
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

EstadoFerramenta criar_ferramentas(const Conteudo& conteudo, Fase fase) {
    EstadoFerramenta estado;
    estado.botoes.push_back({ "estrada (R)", Ferramenta::Estrada, 0 });
    for (std::size_t i = 0; i < conteudo.edificios.size(); ++i) {
        const DefinicaoEdificio& def = conteudo.edificios[i];
        if (def.fase <= fase) {
            estado.botoes.push_back({ def.nome, Ferramenta::Edificio, static_cast<IdTipoEdificio>(i) });
        }
    }
    estado.botoes.push_back({ "demolir (X)", Ferramenta::Demolir, 0 });
    return estado;
}

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
        const std::vector<Rectangle> areas = areas_dos_botoes(estado.botoes);
        for (std::size_t i = 0; i < areas.size(); ++i) {
            if (CheckCollisionPointRec(mouse, areas[i])) {
                estado.ativa = estado.botoes[i].ferramenta;
                estado.tipo = estado.botoes[i].tipo;
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
                const DefinicaoEdificio& def = cidade.conteudo().edificio(estado.tipo);
                cidade.construir_edificio(estado.tipo, origem_centrada(def, cursor));
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
            const DefinicaoEdificio& def = cidade.conteudo().edificio(estado.tipo);
            const Celula origem = origem_centrada(def, cursor);
            const bool ok = cidade.validar_edificio(estado.tipo, origem) == Validacao::Ok;
            const Rectangle r = retangulo_mundo(origem, def.largura_total(), def.altura_total());
            const Color borda = ok ? Color{ 200, 255, 200, 220 } : Color{ 255, 170, 160, 220 };
            DrawRectangleRec(r, ok ? PREVIA_OK : PREVIA_ERRO);
            DrawRectangleLinesEx(r, 2.0f, borda);
            if (def.campos > 0) {
                // Contorno do núcleo, para separar a casa da lavoura na prévia.
                DrawRectangleLinesEx(retangulo_mundo({ origem.x + def.campos, origem.y + def.campos },
                                                     def.largura, def.altura),
                                     2.0f, borda);
            }
            break;
        }
        case Ferramenta::Demolir: {
            const IdEdificio id = cidade.edificio_em(cursor);
            if (id != SEM_EDIFICIO) {
                const Edificio& e = cidade.edificios()[id];
                const DefinicaoEdificio& def = cidade.conteudo().edificio(e.tipo);
                DrawRectangleRec(retangulo_mundo(e.origem, def.largura_total(), def.altura_total()), PREVIA_ERRO);
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
    const std::vector<Rectangle> areas = areas_dos_botoes(estado.botoes);
    for (std::size_t i = 0; i < areas.size(); ++i) {
        const Rectangle r = areas[i];
        const bool ativo = selecionado(estado, estado.botoes[i]);
        const bool sob_mouse = CheckCollisionPointRec(mouse, r);
        const Color fundo = ativo ? Color{ 150, 110, 60, 255 }
                          : sob_mouse ? Color{ 80, 64, 48, 255 }
                                      : Color{ 58, 48, 38, 255 };
        DrawRectangleRec(r, fundo);
        DrawText(estado.botoes[i].rotulo.c_str(), static_cast<int>(r.x + FOLGA_TEXTO / 2.0f),
                 static_cast<int>(r.y + 10.0f), TAMANHO_TEXTO, RAYWHITE);
    }
}

std::string dica_da_ferramenta(const EstadoFerramenta& estado, const Cidade& cidade, Celula cursor) {
    if (estado.ativa != Ferramenta::Edificio) return {};
    const DefinicaoEdificio& def = cidade.conteudo().edificio(estado.tipo);
    const Validacao v = cidade.validar_edificio(estado.tipo, origem_centrada(def, cursor));
    std::string dica = def.nome + " (" + std::to_string(def.largura_total()) + "x" +
                       std::to_string(def.altura_total()) + (def.campos > 0 ? " com campos" : "") + ")";
    if (v == Validacao::TerrenoErrado) {
        dica += ": precisa de " + std::string(nome_do_terreno(def.terreno));
    } else if (v == Validacao::LongeDoRecurso) {
        const Requisito& r = *def.requer;
        dica += ": precisa de " + std::string(nome_do_terreno(r.terreno)) +
                (r.distancia == 1 ? " encostada" : " a até " + std::to_string(r.distancia) + " células");
    } else if (v != Validacao::Ok) {
        dica += std::string(": ") + descrever(v);
    }
    return dica;
}
