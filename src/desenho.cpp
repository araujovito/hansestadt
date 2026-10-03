#include "desenho.hpp"

#include <algorithm>
#include <cmath>

namespace {

// Parte construída do lote, sem o anel de lavoura.
Rectangle nucleo_de(const Cidade& cidade, IdEdificio id) {
    const Edificio& e = cidade.edificio(id);
    const DefinicaoEdificio& def = cidade.definicao(id);
    return { static_cast<float>((e.origem.x + def.campos) * TAMANHO_CELULA),
             static_cast<float>((e.origem.y + def.campos) * TAMANHO_CELULA),
             static_cast<float>(def.largura * TAMANHO_CELULA), static_cast<float>(def.altura * TAMANHO_CELULA) };
}

}  // namespace

void desenhar_avisos_sem_acesso(const Cidade& cidade, Rectangle visivel) {
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo || cidade.tem_acesso(id)) continue;
        const Rectangle r = nucleo_de(cidade, id);
        if (!CheckCollisionRecs(r, visivel)) continue;
        const Vector2 centro{ r.x + r.width - 6.0f, r.y + 6.0f };
        DrawCircleV(centro, 9.0f, Color{ 200, 50, 40, 255 });
        DrawCircleLinesV(centro, 9.0f, Color{ 255, 230, 220, 255 });
        DrawText("!", static_cast<int>(centro.x) - 2, static_cast<int>(centro.y) - 8, 16, RAYWHITE);
    }
}

void desenhar_nome_sob_cursor(const Cidade& cidade, Celula cursor, float zoom) {
    if (!cidade.mapa().dentro(cursor.x, cursor.y)) return;
    const IdEdificio id = cidade.edificio_em(cursor);
    if (id == SEM_EDIFICIO) return;
    const DefinicaoEdificio& def = cidade.definicao(id);
    const Rectangle r = nucleo_de(cidade, id);
    // O texto mantém o tamanho na tela em qualquer zoom.
    const float escala = 1.0f / zoom;
    const int tamanho = std::max(8, static_cast<int>(std::round(14 * escala)));
    const int largura = MeasureText(def.nome.c_str(), tamanho);
    const float pad = 4 * escala;
    const Rectangle etiqueta{ r.x + (r.width - static_cast<float>(largura)) / 2.0f - pad,
                              r.y - static_cast<float>(tamanho) - 3 * pad, static_cast<float>(largura) + 2 * pad,
                              static_cast<float>(tamanho) + 2 * pad };
    DrawRectangleRec(etiqueta, Color{ 30, 24, 18, 220 });
    DrawText(def.nome.c_str(), static_cast<int>(etiqueta.x + pad), static_cast<int>(etiqueta.y + pad), tamanho,
             Color{ 255, 250, 235, 255 });
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
