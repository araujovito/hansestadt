#include "relatorio_tela.hpp"

#include <algorithm>
#include <string>

#include "alertas.hpp"

namespace {

constexpr float LARGURA = 640.0f;
constexpr float ALTURA = 440.0f;
constexpr float TOPO = 80.0f;
constexpr float MARGEM = 16.0f;
constexpr float ALTURA_ABA = 30.0f;
constexpr int TEXTO = 16;
constexpr int LINHA = 22;
// Um ano de jogo cabe no gráfico; dias mais antigos saem pela esquerda.
constexpr std::size_t DIAS_NO_GRAFICO = 365;

constexpr Color FUNDO{ 30, 24, 18, 240 };
constexpr Color BORDA{ 90, 72, 52, 255 };
constexpr Color ABA{ 58, 48, 38, 255 };
constexpr Color ABA_ATIVA{ 150, 110, 60, 255 };
constexpr Color CLARO{ 240, 232, 214, 255 };
constexpr Color APAGADO{ 170, 160, 140, 255 };
constexpr Color VERDE{ 150, 210, 120, 255 };
constexpr Color AMARELO{ 230, 196, 100, 255 };
constexpr Color VERMELHO{ 235, 110, 90, 255 };

constexpr const char* NOMES_DAS_ABAS[] = { "Emprego", "Comida", "Produção", "Transporte" };

Rectangle area_da_aba(const Rectangle& janela, int i) {
    const float largura = (LARGURA - 2.0f * MARGEM - 40.0f) / 4.0f;
    return { janela.x + MARGEM + static_cast<float>(i) * largura, janela.y + MARGEM, largura - 4.0f, ALTURA_ABA };
}

Rectangle area_de_fechar(const Rectangle& janela) {
    return { janela.x + janela.width - MARGEM - 32.0f, janela.y + MARGEM, 32.0f, ALTURA_ABA };
}

void texto(const std::string& t, float x, float y, Color cor = CLARO, int tamanho = TEXTO) {
    DrawText(t.c_str(), static_cast<int>(x), static_cast<int>(y), tamanho, cor);
}

// Barra de preenchimento: vagas ocupadas, por exemplo.
void barra(float x, float y, float largura, int valor, int total) {
    DrawRectangle(static_cast<int>(x), static_cast<int>(y + 5), static_cast<int>(largura), 10, Color{ 60, 50, 40, 255 });
    if (total <= 0) return;
    const float f = std::clamp(static_cast<float>(valor) / static_cast<float>(total), 0.0f, 1.0f);
    DrawRectangle(static_cast<int>(x), static_cast<int>(y + 5), static_cast<int>(largura * f), 10,
                  valor < total ? AMARELO : VERDE);
}

Color cor_dos_dias(int dias) {
    return dias < DIAS_COMIDA_GRAVE ? VERMELHO : dias < DIAS_COMIDA_AVISO ? AMARELO : CLARO;
}

}  // namespace

Rectangle JanelaDeRelatorio::area() const {
    return { (static_cast<float>(GetScreenWidth()) - LARGURA) / 2.0f, TOPO, LARGURA, ALTURA };
}

void JanelaDeRelatorio::registrar_dia(const Simulacao& sim) {
    if (sim.data == ultimo_dia_ && !comida_por_dia_.empty()) return;
    ultimo_dia_ = sim.data;
    comida_por_dia_.push_back(sim.cidade.estoque_total(TipoItem::Comida));
    if (comida_por_dia_.size() > DIAS_NO_GRAFICO) comida_por_dia_.erase(comida_por_dia_.begin());
}

void JanelaDeRelatorio::atualizar() {
    if (!aberta_ || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
    const Rectangle janela = area();
    const Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, area_de_fechar(janela))) {
        fechar();
        return;
    }
    for (int i = 0; i < 4; ++i)
        if (CheckCollisionPointRec(mouse, area_da_aba(janela, i))) aba_ = static_cast<Aba>(i);
}

void JanelaDeRelatorio::desenhar(const Relatorio& r) const {
    if (!aberta_) return;
    const Rectangle janela = area();
    DrawRectangleRec(janela, FUNDO);
    DrawRectangleLinesEx(janela, 2.0f, BORDA);

    const Vector2 mouse = GetMousePosition();
    for (int i = 0; i < 4; ++i) {
        const Rectangle a = area_da_aba(janela, i);
        const bool ativa = static_cast<int>(aba_) == i;
        DrawRectangleRec(a, ativa ? ABA_ATIVA : CheckCollisionPointRec(mouse, a) ? Color{ 80, 64, 48, 255 } : ABA);
        const int largura_texto = MeasureText(NOMES_DAS_ABAS[i], TEXTO);
        texto(NOMES_DAS_ABAS[i], a.x + (a.width - static_cast<float>(largura_texto)) / 2.0f, a.y + 7.0f);
    }
    const Rectangle x = area_de_fechar(janela);
    DrawRectangleRec(x, CheckCollisionPointRec(mouse, x) ? Color{ 120, 60, 50, 255 } : ABA);
    texto("X", x.x + 11.0f, x.y + 7.0f);

    const float cx = janela.x + MARGEM;
    const float cy = janela.y + MARGEM + ALTURA_ABA + 16.0f;
    switch (aba_) {
        case Aba::Emprego: desenhar_emprego(r.emprego, cx, cy); break;
        case Aba::Comida: desenhar_comida(r.comida, cx, cy); break;
        case Aba::Producao: desenhar_producao(r.producao, cx, cy); break;
        case Aba::Transporte: desenhar_transporte(r.transporte, cx, cy); break;
    }
    texto("V ou X fecha. O jogo continua andando.", cx, janela.y + janela.height - MARGEM - 14.0f, APAGADO, 14);
}

void JanelaDeRelatorio::desenhar_emprego(const RelatorioEmprego& e, float x, float y) const {
    texto(TextFormat("adultos %d  |  crianças %d  |  sem casa %d  |  sem trabalho %d", e.adultos, e.criancas,
                     e.sem_casa, e.desempregados),
          x, y);
    y += LINHA;
    texto(TextFormat("vagas ocupadas %d de %d", e.ocupadas, e.vagas), x, y, e.ocupadas < e.vagas ? AMARELO : CLARO);
    y += LINHA + 8;
    if (e.linhas.empty()) {
        texto("nenhum edifício com vagas de trabalho ainda", x, y, APAGADO);
        return;
    }
    texto("ofício", x, y, APAGADO);
    texto("edifícios", x + 200, y, APAGADO);
    texto("ocupadas", x + 300, y, APAGADO);
    y += LINHA;
    for (const LinhaDeEmprego& l : e.linhas) {
        texto(l.nome, x, y);
        texto(std::to_string(l.edificios), x + 200, y);
        texto(TextFormat("%d/%d", l.ocupadas, l.vagas), x + 300, y, l.ocupadas < l.vagas ? AMARELO : CLARO);
        barra(x + 380, y, 200, l.ocupadas, l.vagas);
        y += LINHA;
    }
}

void JanelaDeRelatorio::desenhar_comida(const RelatorioComida& c, float x, float y) const {
    texto(TextFormat("%d porções para %d moradores:", c.estoque, c.moradores), x, y);
    texto(TextFormat("%d dias", c.dias), x + 300, y, cor_dos_dias(c.dias));
    y += LINHA;
    texto(TextFormat("refeições perdidas %d  |  mortes por fome %d", c.refeicoes_perdidas, c.mortos_de_fome), x, y,
          c.refeicoes_perdidas > 0 ? AMARELO : APAGADO);
    y += LINHA + 8;
    for (const LinhaDeComida& l : c.itens) {
        texto(l.nome, x, y);
        texto(std::to_string(l.estoque), x + 200, y, l.estoque > 0 ? CLARO : APAGADO);
        y += LINHA;
    }

    // Gráfico do estoque total por dia. A linha tracejada marca o estoque
    // que daria DIAS_COMIDA_AVISO dias para a população de hoje.
    y += 10;
    const float largura = LARGURA - 2.0f * MARGEM;
    const float altura = 120.0f;
    DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(largura), static_cast<int>(altura),
                  Color{ 22, 18, 14, 255 });
    if (comida_por_dia_.size() < 2) {
        texto("o gráfico começa a encher conforme os dias passam", x + 10, y + altura / 2 - 8, APAGADO, 14);
        return;
    }
    const int aviso = DIAS_COMIDA_AVISO * c.moradores;
    const int maximo = std::max({ 1, aviso, *std::max_element(comida_por_dia_.begin(), comida_por_dia_.end()) });
    // Enquanto há menos de um ano guardado, os dias se espalham pela largura
    // toda; depois, cada ponto é um dia e os mais antigos saem pela esquerda.
    const float passos = static_cast<float>(comida_por_dia_.size() - 1);
    auto ponto = [&](std::size_t i, int valor) {
        return Vector2{ x + largura * static_cast<float>(i) / passos,
                        y + altura - altura * static_cast<float>(valor) / static_cast<float>(maximo) };
    };
    if (aviso > 0) {
        const float ya = ponto(0, aviso).y;
        for (float xa = x; xa < x + largura; xa += 12.0f) DrawLineEx({ xa, ya }, { xa + 6.0f, ya }, 1.0f, AMARELO);
    }
    for (std::size_t i = 1; i < comida_por_dia_.size(); ++i)
        DrawLineEx(ponto(i - 1, comida_por_dia_[i - 1]), ponto(i, comida_por_dia_[i]), 2.0f, VERDE);
    texto(TextFormat("comida nos últimos %d dias (máx. %d)", static_cast<int>(comida_por_dia_.size()), maximo), x,
          y + altura + 4, APAGADO, 14);
}

void JanelaDeRelatorio::desenhar_producao(const std::vector<LinhaDeProducao>& p, float x, float y) const {
    if (p.empty()) {
        texto("nenhum edifício de produção ainda", x, y, APAGADO);
        return;
    }
    texto("edifício", x, y, APAGADO);
    texto("situação", x + 190, y, APAGADO);
    y += LINHA;
    for (const LinhaDeProducao& l : p) {
        texto(TextFormat("%s (%d)", l.nome.c_str(), l.edificios), x, y);
        const int produzindo = l.por_situacao[static_cast<std::size_t>(Situacao::Produzindo)];
        texto(resumir_producao(l), x + 190, y + 2, produzindo == l.edificios ? CLARO : AMARELO, 14);
        y += LINHA;
    }
}

void JanelaDeRelatorio::desenhar_transporte(const RelatorioTransporte& t, float x, float y) const {
    texto(TextFormat("carregadores %d", t.carregadores), x, y);
    if (t.carregadores > 0) {
        texto(TextFormat("(%d do armazém, %d dos edifícios de produção)", t.do_deposito,
                         t.carregadores - t.do_deposito),
              x + 160, y, APAGADO);
    }
    y += LINHA;
    texto(TextFormat("com tarefa %d  |  levando carga %d  |  livres %d", t.ocupados, t.carregando,
                     t.carregadores - t.ocupados),
          x, y);
    barra(x + 440, y, 160, t.ocupados, t.carregadores);
    y += LINHA + 8;
    texto(TextFormat("entregas feitas %llu", t.entregas), x, y);
    y += LINHA;
    texto(TextFormat("cargas perdidas %llu", t.perdidos), x, y, t.perdidos > 0 ? AMARELO : CLARO);
    y += LINHA + 8;
    if (t.carregadores == 0) {
        texto("Sem carregadores nada se move: cada armazém precisa de um adulto com casa.", x, y, AMARELO, 14);
    } else if (t.ocupados == t.carregadores) {
        texto("Todos os carregadores estão ocupados: entregas podem atrasar.", x, y, AMARELO, 14);
    }
}
