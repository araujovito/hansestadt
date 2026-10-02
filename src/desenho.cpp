#include "desenho.hpp"

#include <algorithm>
#include <cmath>

#include "ruido.hpp"

namespace {

Color cor_base(Terreno t) {
    switch (t) {
        case Terreno::Campo: return { 126, 156, 84, 255 };
        case Terreno::Floresta: return { 52, 92, 48, 255 };
        case Terreno::Barreiro: return { 168, 112, 76, 255 };
        case Terreno::Agua: return { 58, 98, 140, 255 };
        case Terreno::Turfa: return { 92, 82, 58, 255 };
    }
    return MAGENTA;
}

// Varia levemente o tom de cada célula para o mapa não parecer um tabuleiro.
// A variação vem do hash da posição, então é sempre a mesma em cada quadro.
Color com_variacao(Color c, int x, int y) {
    const float v = 0.92f + 0.16f * ruido::hash(x, y, 0xC0FFEE);
    auto canal = [v](unsigned char k) {
        return static_cast<unsigned char>(std::min(255.0f, static_cast<float>(k) * v));
    };
    return { canal(c.r), canal(c.g), canal(c.b), c.a };
}

void desenhar_detalhe(Terreno t, int x, int y, float px, float py) {
    constexpr float m = TAMANHO_CELULA / 2.0f;
    switch (t) {
        case Terreno::Floresta: {
            // Duas copas por célula, em posições fixas sorteadas pelo hash.
            for (int i = 0; i < 2; ++i) {
                const float ox = 6.0f + 20.0f * ruido::hash(x * 2 + i, y, 0xA11);
                const float oy = 6.0f + 20.0f * ruido::hash(x * 2 + i, y, 0xB22);
                DrawCircleV({ px + ox, py + oy }, 6.0f, Color{ 34, 70, 34, 255 });
            }
            break;
        }
        case Terreno::Turfa:
            DrawLineEx({ px + 6, py + m }, { px + 14, py + m }, 2.0f, Color{ 60, 72, 48, 255 });
            DrawLineEx({ px + 18, py + m + 6 }, { px + 26, py + m + 6 }, 2.0f, Color{ 60, 72, 48, 255 });
            break;
        case Terreno::Barreiro:
            DrawCircleV({ px + m, py + m }, 4.0f, Color{ 140, 90, 60, 255 });
            break;
        case Terreno::Campo:
        case Terreno::Agua:
            break;
    }
}

Color escurecer(Color c, float f) {
    return { static_cast<unsigned char>(static_cast<float>(c.r) * f),
             static_cast<unsigned char>(static_cast<float>(c.g) * f),
             static_cast<unsigned char>(static_cast<float>(c.b) * f), c.a };
}

struct Intervalo {
    int x0, y0, x1, y1;
};

Intervalo celulas_visiveis(const Mapa& mapa, Rectangle visivel) {
    return {
        std::max(0, static_cast<int>(std::floor(visivel.x / TAMANHO_CELULA))),
        std::max(0, static_cast<int>(std::floor(visivel.y / TAMANHO_CELULA))),
        std::min(mapa.largura() - 1, static_cast<int>((visivel.x + visivel.width) / TAMANHO_CELULA)),
        std::min(mapa.altura() - 1, static_cast<int>((visivel.y + visivel.height) / TAMANHO_CELULA)),
    };
}

void desenhar_estrada(const Cidade& cidade, int x, int y) {
    constexpr Color terra{ 176, 150, 104, 255 };
    constexpr float borda = 7.0f;
    constexpr float miolo = TAMANHO_CELULA - 2.0f * borda;
    const float px = static_cast<float>(x * TAMANHO_CELULA);
    const float py = static_cast<float>(y * TAMANHO_CELULA);

    DrawRectangleV({ px + borda, py + borda }, { miolo, miolo }, terra);
    // Prolonga o miolo até cada vizinho com estrada, para os trechos se
    // ligarem em vez de virarem quadrados soltos.
    if (cidade.tem_estrada({ x - 1, y })) DrawRectangleV({ px, py + borda }, { borda, miolo }, terra);
    if (cidade.tem_estrada({ x + 1, y })) DrawRectangleV({ px + borda + miolo, py + borda }, { borda, miolo }, terra);
    if (cidade.tem_estrada({ x, y - 1 })) DrawRectangleV({ px + borda, py }, { miolo, borda }, terra);
    if (cidade.tem_estrada({ x, y + 1 })) DrawRectangleV({ px + borda, py + borda + miolo }, { miolo, borda }, terra);
}

// Cor da lavoura de centeio de inverno ao longo do ano: semeado no outono,
// brota antes do frio, cresce na primavera, amadurece dourado e é colhido
// em julho-agosto, deixando o restolho até a lavra de setembro.
Color cor_da_lavoura(int mes) {
    switch (mes) {
        case 1: case 2: return { 118, 116, 72, 255 };              // brotos sob o frio
        case 3: case 4: return { 112, 150, 64, 255 };              // verde novo
        case 5: case 6: return { 150, 162, 70, 255 };              // espigando
        case 7: return { 214, 178, 82, 255 };                      // maduro
        case 8: return { 196, 170, 110, 255 };                     // colheita, restolho
        case 9: return { 128, 98, 66, 255 };                       // lavrado e semeado
        default: return { 120, 112, 68, 255 };                     // outubro-dezembro: brotando
    }
}

void desenhar_lavoura(const Edificio& e, const DefinicaoEdificio& def, int mes) {
    const Color cor = cor_da_lavoura(mes);
    const Color sulco = escurecer(cor, 0.82f);
    for (int dy = 0; dy < def.altura_total(); ++dy) {
        for (int dx = 0; dx < def.largura_total(); ++dx) {
            if (!eh_lavoura(def, dx, dy)) continue;
            const float px = static_cast<float>((e.origem.x + dx) * TAMANHO_CELULA);
            const float py = static_cast<float>((e.origem.y + dy) * TAMANHO_CELULA);
            DrawRectangleV({ px, py }, { TAMANHO_CELULA, TAMANHO_CELULA }, cor);
            for (int i = 1; i < 4; ++i) {
                const float y = py + static_cast<float>(i * TAMANHO_CELULA) / 4.0f;
                DrawLineEx({ px + 2.0f, y }, { px + TAMANHO_CELULA - 2.0f, y }, 1.5f, sulco);
            }
        }
    }
}

// Porta: um vão escuro na borda inferior, na coluna da célula de entrada.
void desenhar_porta(Celula porta) {
    const float x = static_cast<float>(porta.x * TAMANHO_CELULA) + TAMANHO_CELULA / 2.0f - 5.0f;
    const float y = static_cast<float>(porta.y * TAMANHO_CELULA) - 7.0f;
    DrawRectangleV({ x, y }, { 10.0f, 7.0f }, Color{ 50, 36, 24, 255 });
}

// Sem estrada na porta, nenhum carregador chega: o alerta fica no canto do
// edifício até o jogador ligar a estrada.
void desenhar_alerta_sem_acesso(const Rectangle& nucleo) {
    const Vector2 centro{ nucleo.x + nucleo.width - 6.0f, nucleo.y + 6.0f };
    DrawCircleV(centro, 9.0f, Color{ 200, 50, 40, 255 });
    DrawCircleLinesV(centro, 9.0f, Color{ 255, 230, 220, 255 });
    DrawText("!", static_cast<int>(centro.x) - 2, static_cast<int>(centro.y) - 8, 16, RAYWHITE);
}

void desenhar_edificio(const Cidade& cidade, IdEdificio id, float zoom, int mes) {
    const Edificio& e = cidade.edificio(id);
    const DefinicaoEdificio& def = cidade.definicao(id);
    if (def.campos > 0) desenhar_lavoura(e, def, mes);
    const Celula nucleo{ e.origem.x + def.campos, e.origem.y + def.campos };
    const Rectangle r{ static_cast<float>(nucleo.x * TAMANHO_CELULA) + 2.0f,
                       static_cast<float>(nucleo.y * TAMANHO_CELULA) + 2.0f,
                       static_cast<float>(def.largura * TAMANHO_CELULA) - 4.0f,
                       static_cast<float>(def.altura * TAMANHO_CELULA) - 4.0f };
    const Color cor = GetColor(def.cor);
    DrawRectangleRec(r, cor);
    // Cumeeira: uma linha no meio sugere o telhado de duas águas visto de cima.
    DrawLineEx({ r.x + 4.0f, r.y + r.height / 2.0f }, { r.x + r.width - 4.0f, r.y + r.height / 2.0f }, 2.0f,
               escurecer(cor, 0.7f));
    DrawRectangleLinesEx(r, 2.0f, escurecer(cor, 0.55f));

    desenhar_porta(cidade.porta(id));
    if (!cidade.tem_acesso(id)) desenhar_alerta_sem_acesso(r);

    if (zoom >= 0.8f) {
        constexpr int tamanho = 10;
        const int largura = MeasureText(def.nome.c_str(), tamanho);
        DrawText(def.nome.c_str(), static_cast<int>(r.x + (r.width - static_cast<float>(largura)) / 2.0f),
                 static_cast<int>(r.y + 4.0f), tamanho, Color{ 255, 250, 235, 230 });
    }
}

}  // namespace

void desenhar_cidade(const Cidade& cidade, Rectangle visivel, float zoom, int mes) {
    const Intervalo v = celulas_visiveis(cidade.mapa(), visivel);
    for (int y = v.y0; y <= v.y1; ++y) {
        for (int x = v.x0; x <= v.x1; ++x) {
            if (cidade.tem_estrada({ x, y })) desenhar_estrada(cidade, x, y);
        }
    }

    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        const Edificio& e = cidade.edificio(id);
        if (!e.ativo) continue;
        const DefinicaoEdificio& def = cidade.definicao(id);
        const Rectangle r{ static_cast<float>(e.origem.x * TAMANHO_CELULA),
                           static_cast<float>(e.origem.y * TAMANHO_CELULA),
                           static_cast<float>(def.largura_total() * TAMANHO_CELULA),
                           static_cast<float>(def.altura_total() * TAMANHO_CELULA) };
        if (CheckCollisionRecs(r, visivel)) desenhar_edificio(cidade, id, zoom, mes);
    }
}

void desenhar_carregadores(const Cidade& cidade, const Transporte& transporte, Rectangle visivel) {
    constexpr float meia = TAMANHO_CELULA / 2.0f;
    for (const Carregador& c : transporte.carregadores()) {
        Vector2 p{ static_cast<float>(c.posicao.x * TAMANHO_CELULA) + meia,
                   static_cast<float>(c.posicao.y * TAMANHO_CELULA) + meia };
        if (c.andando()) {
            const Celula prox = c.rota[c.passo];
            const float f = static_cast<float>(c.ticks_na_celula) / Transporte::TICKS_POR_CELULA;
            p.x += (static_cast<float>(prox.x - c.posicao.x) * TAMANHO_CELULA) * f;
            p.y += (static_cast<float>(prox.y - c.posicao.y) * TAMANHO_CELULA) * f;
        }
        if (!CheckCollisionPointRec(p, visivel)) continue;

        // Parado em casa fica escondido dentro do depósito.
        if (c.livre() && !c.andando()) continue;

        DrawCircleV(p, 6.0f, Color{ 222, 200, 160, 255 });
        DrawCircleLinesV(p, 6.0f, Color{ 70, 52, 34, 255 });
        if (c.carregando) {
            // Comida em laranja, insumo em marrom: dá para ver de longe o
            // que está circulando na vila.
            const bool comida = cidade.conteudo().itens[c.item].tipo == TipoItem::Comida;
            DrawRectangleV({ p.x - 4.0f, p.y - 13.0f }, { 8.0f, 7.0f },
                           comida ? Color{ 230, 150, 60, 255 } : Color{ 130, 90, 50, 255 });
        }
    }
}

void desenhar_mapa(const Mapa& mapa, Rectangle visivel, float zoom) {
    const Intervalo v = celulas_visiveis(mapa, visivel);

    // Abaixo deste zoom os detalhes viram ruído visual e só custam desenho.
    const bool com_detalhes = zoom >= 0.6f;

    for (int y = v.y0; y <= v.y1; ++y) {
        for (int x = v.x0; x <= v.x1; ++x) {
            const Terreno t = mapa.terreno(x, y);
            const float px = static_cast<float>(x * TAMANHO_CELULA);
            const float py = static_cast<float>(y * TAMANHO_CELULA);
            DrawRectangleV({ px, py }, { TAMANHO_CELULA, TAMANHO_CELULA }, com_variacao(cor_base(t), x, y));
            if (com_detalhes) desenhar_detalhe(t, x, y, px, py);
        }
    }
}

void destacar_celula(Celula c) {
    DrawRectangleLinesEx({ static_cast<float>(c.x * TAMANHO_CELULA), static_cast<float>(c.y * TAMANHO_CELULA),
                           TAMANHO_CELULA, TAMANHO_CELULA },
                         2.0f, Color{ 255, 240, 180, 220 });
}

Celula celula_em(Vector2 p) {
    return { static_cast<int>(std::floor(p.x / TAMANHO_CELULA)),
             static_cast<int>(std::floor(p.y / TAMANHO_CELULA)) };
}
