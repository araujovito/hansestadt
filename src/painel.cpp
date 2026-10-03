#include "painel.hpp"

#include <algorithm>
#include <string>

#include "melhorias.hpp"
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
constexpr Color VERMELHO{ 235, 130, 110, 255 };
constexpr float ALTURA_BOTAO = 28.0f;

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

// No pé do painel, numa posição fixa, para o clique ser conferido antes do
// desenho sem depender de quantas linhas o painel tem.
Rectangle area_do_botao_melhorar() {
    const Rectangle area = area_do_painel_edificio();
    return { area.x + MARGEM, area.y + area.height - MARGEM - ALTURA_BOTAO, area.width - 2.0f * MARGEM,
             ALTURA_BOTAO };
}

// Nível, próxima melhoria com os materiais e o botão de melhorar.
void desenhar_instituicao(const Cidade& cidade, IdEdificio id, Fase fase, int x, int y) {
    const FichaDaInstituicao f = ficha_da_instituicao(cidade, cidade.edificio(id).tipo, fase);
    DrawText(TextFormat("nível %d de %d", f.nivel, f.nivel_final), x, y, TEXTO, CLARO);
    y += LINHA;
    if (f.fase_da_proxima) {
        const bool liberado = *f.fase_da_proxima <= fase;
        DrawText(liberado ? TextFormat("nível %d liberado nesta fase", f.nivel + 1)
                          : TextFormat("nível %d libera na fase %s", f.nivel + 1, nome_da_fase(*f.fase_da_proxima)),
                 x, y, TEXTO, liberado ? CLARO : APAGADO);
        y += LINHA;
        for (const MaterialDaMelhoria& m : f.materiais) {
            DrawText(TextFormat("%s %d/%d", m.nome.c_str(), m.tem, m.precisa), x + 12, y, TEXTO,
                     m.tem >= m.precisa ? CLARO : VERMELHO);
            y += LINHA;
        }
    }

    const Rectangle b = area_do_botao_melhorar();
    const bool pode = f.pode_melhorar == ResultadoMelhoria::Ok;
    const bool sob_mouse = pode && CheckCollisionPointRec(GetMousePosition(), b);
    DrawRectangleRec(b, pode ? (sob_mouse ? Color{ 170, 128, 70, 255 } : Color{ 150, 110, 60, 255 })
                             : Color{ 58, 48, 38, 255 });
    // Os textos de descrever() não cabem no botão; aqui vão versões curtas.
    const char* rotulo = TextFormat("Melhorar para o nível %d (U)", f.nivel + 1);
    switch (f.pode_melhorar) {
        case ResultadoMelhoria::Ok: break;
        case ResultadoMelhoria::FaseBloqueada:
            rotulo = TextFormat("aguarda a fase %s", nome_da_fase(f.fase_da_proxima.value_or(fase)));
            break;
        case ResultadoMelhoria::SemRecursos: rotulo = "faltam materiais"; break;
        case ResultadoMelhoria::NivelMaximo: rotulo = "nível máximo"; break;
        case ResultadoMelhoria::EdificioInativo:
        case ResultadoMelhoria::NaoInstituicao: rotulo = descrever(f.pode_melhorar); break;
    }
    DrawText(rotulo, static_cast<int>(b.x + (b.width - static_cast<float>(MeasureText(rotulo, TEXTO))) / 2.0f),
             static_cast<int>(b.y) + 6, TEXTO, pode ? CLARO : APAGADO);
}

}  // namespace

Rectangle area_do_painel_edificio() {
    return { static_cast<float>(GetScreenWidth()) - LARGURA - 8.0f, 72.0f, LARGURA, ALTURA };
}

bool clicou_em_melhorar(const Cidade& cidade, IdEdificio id) {
    return cidade.definicao(id).instituicao && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(GetMousePosition(), area_do_botao_melhorar());
}

void desenhar_painel_edificio(const Cidade& cidade, const Populacao& populacao, IdEdificio id, int mes,
                              int ticks_por_dia, Fase fase) {
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

    if (def.instituicao) {
        desenhar_instituicao(cidade, id, fase, x, y);
        return;
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

    if (!def.produtividade.empty()) {
        const int nivel = cidade.nivel_produtividade(e.tipo);
        DrawText(TextFormat("tecnologia %d/%d (+%d%%) — T", nivel,
                            static_cast<int>(def.produtividade.size()), cidade.bonus_produtividade(e.tipo)),
                 x, y, TEXTO, CLARO);
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

namespace {

constexpr float ALERTA_X = 8.0f;
constexpr float ALERTA_Y = 72.0f;
constexpr float ALERTA_LARGURA = 420.0f;
constexpr float ALERTA_ALTURA = 24.0f;
constexpr float ALERTA_ESPACO = 4.0f;

std::size_t linhas_de_alerta(const std::vector<Alerta>& alertas) {
    // Com alertas demais, a última linha vira "e mais N".
    return alertas.size() > MAXIMO_ALERTAS_NA_TELA ? MAXIMO_ALERTAS_NA_TELA + 1 : alertas.size();
}

Rectangle area_do_alerta(std::size_t i) {
    return { ALERTA_X, ALERTA_Y + static_cast<float>(i) * (ALERTA_ALTURA + ALERTA_ESPACO), ALERTA_LARGURA,
             ALERTA_ALTURA };
}

}  // namespace

int alerta_sob_mouse(const std::vector<Alerta>& alertas) {
    const std::size_t n = std::min(alertas.size(), MAXIMO_ALERTAS_NA_TELA);
    for (std::size_t i = 0; i < n; ++i)
        if (CheckCollisionPointRec(GetMousePosition(), area_do_alerta(i))) return static_cast<int>(i);
    return -1;
}

bool mouse_sobre_alertas(const std::vector<Alerta>& alertas) {
    for (std::size_t i = 0; i < linhas_de_alerta(alertas); ++i)
        if (CheckCollisionPointRec(GetMousePosition(), area_do_alerta(i))) return true;
    return false;
}

void desenhar_alertas(const std::vector<Alerta>& alertas) {
    const int sob_mouse = alerta_sob_mouse(alertas);
    for (std::size_t i = 0; i < linhas_de_alerta(alertas); ++i) {
        const Rectangle r = area_do_alerta(i);
        if (i == MAXIMO_ALERTAS_NA_TELA) {
            DrawRectangleRec(r, FUNDO);
            DrawText(TextFormat("e mais %d", static_cast<int>(alertas.size() - MAXIMO_ALERTAS_NA_TELA)),
                     static_cast<int>(r.x) + 26, static_cast<int>(r.y) + 4, TEXTO, APAGADO);
            break;
        }
        const Alerta& a = alertas[i];
        const bool clicavel = !a.edificios.empty();
        const bool destacado = clicavel && static_cast<int>(i) == sob_mouse;
        DrawRectangleRec(r, destacado ? Color{ 70, 56, 42, 240 } : FUNDO);
        const Color cor = a.grave ? Color{ 235, 110, 90, 255 } : Color{ 230, 196, 100, 255 };
        DrawCircle(static_cast<int>(r.x) + 13, static_cast<int>(r.y + r.height / 2.0f), 5.0f, cor);
        DrawText(a.texto.c_str(), static_cast<int>(r.x) + 26, static_cast<int>(r.y) + 4, TEXTO, CLARO);
        // Só quem leva a algum lugar ganha a seta, e ela diz quantos cliques
        // percorrem a lista.
        if (clicavel) {
            const char* seta = a.edificios.size() > 1 ? TextFormat("ver (%d) >", static_cast<int>(a.edificios.size()))
                                                       : "ver >";
            DrawText(seta, static_cast<int>(r.x + r.width) - MeasureText(seta, TEXTO) - 8,
                     static_cast<int>(r.y) + 4, TEXTO, destacado ? CLARO : APAGADO);
        }
    }
}
