#include "painel.hpp"

#include <string>

#include "producao.hpp"

namespace {

constexpr float LARGURA = 280.0f;
constexpr float ALTURA = 330.0f;
constexpr int MARGEM = 12;
constexpr int TEXTO = 16;
constexpr int LINHA = 22;

constexpr Color FUNDO{ 30, 24, 18, 230 };
constexpr Color CLARO{ 240, 232, 214, 255 };
constexpr Color APAGADO{ 170, 160, 140, 255 };

Color cor_da_situacao(Situacao s) {
    switch (s) {
        case Situacao::Produzindo: return { 150, 210, 120, 255 };
        case Situacao::Pausado:
        case Situacao::ForaDeEstacao: return { 220, 200, 120, 255 };
        case Situacao::FaltaEntrada:
        case Situacao::SemTrabalhadores:
        case Situacao::SaidaCheia: return { 235, 130, 110, 255 };
        case Situacao::SemReceita: break;
    }
    return CLARO;
}

std::string lista(const Conteudo& conteudo, const std::vector<Quantidade>& qs) {
    std::string texto;
    for (const Quantidade& q : qs) {
        if (!texto.empty()) texto += " + ";
        texto += std::to_string(q.quantidade) + " " + conteudo.itens[q.item].nome;
    }
    return texto;
}

std::string descrever_receita(const Conteudo& conteudo, const Receita& r) {
    const std::string dias = std::to_string(r.dias) + (r.dias == 1 ? " dia" : " dias");
    if (r.entradas.empty()) return lista(conteudo, r.saidas) + " (" + dias + ")";
    return lista(conteudo, r.entradas) + " -> " + lista(conteudo, r.saidas) + " (" + dias + ")";
}

}  // namespace

Rectangle area_do_painel_edificio() {
    return { static_cast<float>(GetScreenWidth()) - LARGURA - 8.0f, 48.0f, LARGURA, ALTURA };
}

void desenhar_painel_edificio(const Cidade& cidade, const Populacao& populacao, IdEdificio id, int mes,
                              int ticks_por_dia) {
    const Rectangle area = area_do_painel_edificio();
    DrawRectangleRec(area, FUNDO);
    DrawRectangleLinesEx(area, 1.0f, Color{ 90, 72, 52, 255 });

    const Conteudo& conteudo = cidade.conteudo();
    const DefinicaoEdificio& def = cidade.definicao(id);
    const Edificio& e = cidade.edificio(id);
    const int x = static_cast<int>(area.x) + MARGEM;
    int y = static_cast<int>(area.y) + MARGEM;

    DrawText(def.nome.c_str(), x, y, 22, CLARO);
    y += 30;

    if (def.moradores > 0) {
        DrawText(TextFormat("moradores %d/%d", populacao.moradores_na_casa(id), def.moradores), x, y, TEXTO,
                 CLARO);
        y += LINHA;
    }

    if (!cidade.tem_acesso(id)) {
        DrawText("sem estrada na entrada", x, y, TEXTO, Color{ 235, 130, 110, 255 });
        y += LINHA;
    }

    const Situacao s = situacao(cidade, id, mes, &populacao);
    if (s != Situacao::SemReceita) {
        DrawText(descrever(s), x, y, TEXTO, cor_da_situacao(s));
        y += LINHA;
    }

    if (def.trabalhadores > 0) {
        DrawText(TextFormat("trabalhadores %d/%d", populacao.trabalhadores_em(id), def.trabalhadores), x, y,
                 TEXTO, CLARO);
        y += LINHA;
    }

    if (e.receita_ativa >= 0) {
        const Receita& r = def.receitas[static_cast<std::size_t>(e.receita_ativa)];
        const float fracao = static_cast<float>(e.progresso) / static_cast<float>(progresso_total(r, ticks_por_dia));
        const float largura_barra = area.width - 2.0f * MARGEM;
        DrawRectangle(x, y, static_cast<int>(largura_barra), 10, Color{ 60, 50, 40, 255 });
        DrawRectangle(x, y, static_cast<int>(largura_barra * fracao), 10, Color{ 150, 210, 120, 255 });
        y += 16;
        DrawText(TextFormat("ritmo neste mês: %d%%", fator_do_mes(r, mes) * 100 / ESCALA_FATOR), x, y, TEXTO,
                 APAGADO);
        y += LINHA;
    }

    if (!def.receitas.empty()) {
        y += 4;
        DrawText("receitas", x, y, TEXTO, APAGADO);
        y += LINHA;
        for (const Receita& r : def.receitas) {
            DrawText(descrever_receita(conteudo, r).c_str(), x, y, TEXTO, CLARO);
            y += LINHA;
        }
    }

    if (def.estoque > 0) {
        y += 4;
        if (def.deposito) {
            int total = 0;
            for (int q : e.estoque) total += q;
            DrawText(TextFormat("estoque %d/%d", total, def.estoque), x, y, TEXTO, APAGADO);
        } else {
            DrawText(TextFormat("estoque (até %d de cada)", def.estoque), x, y, TEXTO, APAGADO);
        }
        y += LINHA;
        for (IdItem i = 0; i < conteudo.itens.size(); ++i) {
            if (e.estoque[i] == 0 && !def.usa_item(i)) continue;
            DrawText(TextFormat("%s: %d", conteudo.itens[i].nome.c_str(), e.estoque[i]), x, y, TEXTO, CLARO);
            y += LINHA;
        }
    }
}
