#include "menu.hpp"

#include "raylib.h"

namespace {

constexpr float LARGURA = 460.0f;
constexpr float ALTURA_OPCAO = 48.0f;
constexpr float ESPACO = 8.0f;
constexpr float MARGEM = 20.0f;
constexpr float ALTURA_TITULO = 44.0f;
constexpr float ALTURA_RODAPE = 30.0f;

// Mesma paleta da barra de ferramentas.
constexpr Color FUNDO_PAINEL{ 30, 24, 18, 240 };
constexpr Color FUNDO_OPCAO{ 58, 48, 38, 255 };
constexpr Color FUNDO_SOB_MOUSE{ 80, 64, 48, 255 };
constexpr Color FUNDO_DESABILITADA{ 42, 36, 30, 255 };
constexpr Color TEXTO_DETALHE{ 200, 186, 160, 255 };
constexpr Color TEXTO_DESABILITADO{ 120, 110, 96, 255 };

Rectangle area_do_painel(std::size_t n_opcoes) {
    const float altura = MARGEM + ALTURA_TITULO + static_cast<float>(n_opcoes) * (ALTURA_OPCAO + ESPACO) +
                         ALTURA_RODAPE + MARGEM;
    return { (static_cast<float>(GetScreenWidth()) - LARGURA) / 2.0f,
             (static_cast<float>(GetScreenHeight()) - altura) / 2.0f, LARGURA, altura };
}

Rectangle area_da_opcao(const Rectangle& painel, std::size_t i) {
    return { painel.x + MARGEM,
             painel.y + MARGEM + ALTURA_TITULO + static_cast<float>(i) * (ALTURA_OPCAO + ESPACO),
             LARGURA - 2.0f * MARGEM, ALTURA_OPCAO };
}

std::string descrever(const SituacaoDaVaga& v) {
    switch (v.estado) {
        case SituacaoDaVaga::Estado::Vazia: return "vazia";
        case SituacaoDaVaga::Estado::Valida: return v.texto;
        case SituacaoDaVaga::Estado::Invalida: return "não abre: " + v.texto;
    }
    return {};
}

}  // namespace

void MenuDePausa::abrir(const Conteudo& conteudo, bool pode_salvar) {
    conteudo_ = &conteudo;
    pode_salvar_ = pode_salvar;
    mensagem_.clear();
    tela_ = Tela::Principal;
    reexaminar();
}

void MenuDePausa::reexaminar() {
    vagas_.clear();
    if (!conteudo_) return;
    for (const VagaDePartida& v : vagas_de_partida()) vagas_.push_back(examinar_vaga(v.arquivo, *conteudo_));
}

std::vector<MenuDePausa::Opcao> MenuDePausa::opcoes() const {
    const auto& vagas = vagas_de_partida();
    std::vector<Opcao> r;
    switch (tela_) {
        case Tela::Fechado:
            break;
        case Tela::Principal:
            r.push_back({ "Continuar", "", true });
            r.push_back({ "Salvar partida", pode_salvar_ ? "" : "funde a vila primeiro", pode_salvar_ });
            r.push_back({ "Carregar partida", "", true });
            r.push_back({ "Sair do jogo", "", true });
            break;
        case Tela::Salvar:
            for (std::size_t i = 0; i < vagas.size(); ++i) r.push_back({ vagas[i].nome, descrever(vagas_[i]), true });
            r.push_back({ "Voltar", "", true });
            break;
        case Tela::Carregar:
            for (std::size_t i = 0; i < vagas.size(); ++i) {
                r.push_back({ vagas[i].nome, descrever(vagas_[i]),
                              vagas_[i].estado == SituacaoDaVaga::Estado::Valida });
            }
            r.push_back({ "Voltar", "", true });
            break;
        case Tela::Confirmar:
            r.push_back({ "Sim", consequencia_, true });
            r.push_back({ "Não", "", true });
            break;
    }
    return r;
}

void MenuDePausa::pedir_confirmacao(std::string pergunta, std::string consequencia, PedidoDoMenu pedido) {
    pergunta_ = std::move(pergunta);
    consequencia_ = std::move(consequencia);
    pendente_ = std::move(pedido);
    voltar_para_ = tela_;
    tela_ = Tela::Confirmar;
}

void MenuDePausa::escolher(std::size_t i, PedidoDoMenu& pedido) {
    const auto& vagas = vagas_de_partida();
    mensagem_.clear();
    switch (tela_) {
        case Tela::Fechado:
            break;
        case Tela::Principal:
            if (i == 0) fechar();
            else if (i == 1) tela_ = Tela::Salvar;
            else if (i == 2) tela_ = Tela::Carregar;
            else pedir_confirmacao("Sair do jogo?", "o que não foi salvo se perde", { PedidoDoMenu::Acao::Sair, {} });
            break;
        case Tela::Salvar:
            if (i == vagas.size()) { tela_ = Tela::Principal; break; }
            if (vagas_[i].estado == SituacaoDaVaga::Estado::Vazia) {
                pedido = { PedidoDoMenu::Acao::Salvar, vagas[i].arquivo };
            } else {
                pedir_confirmacao("Substituir " + vagas[i].nome + "?", "a partida que estava lá se perde",
                                  { PedidoDoMenu::Acao::Salvar, vagas[i].arquivo });
            }
            break;
        case Tela::Carregar:
            if (i == vagas.size()) { tela_ = Tela::Principal; break; }
            // Antes da fundação não há partida em andamento para perder.
            if (!pode_salvar_) {
                pedido = { PedidoDoMenu::Acao::Carregar, vagas[i].arquivo };
            } else {
                pedir_confirmacao("Carregar " + vagas[i].nome + "?", "o que não foi salvo na partida atual se perde",
                                  { PedidoDoMenu::Acao::Carregar, vagas[i].arquivo });
            }
            break;
        case Tela::Confirmar:
            tela_ = voltar_para_;
            if (i == 0) pedido = pendente_;
            break;
    }
}

PedidoDoMenu MenuDePausa::atualizar() {
    PedidoDoMenu pedido;
    if (!aberto()) return pedido;

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (tela_ == Tela::Principal) fechar();
        else if (tela_ == Tela::Confirmar) tela_ = voltar_para_;
        else tela_ = Tela::Principal;
        mensagem_.clear();
        return pedido;
    }

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return pedido;
    const std::vector<Opcao> lista = opcoes();
    const Rectangle painel = area_do_painel(lista.size());
    for (std::size_t i = 0; i < lista.size(); ++i) {
        if (lista[i].habilitada && CheckCollisionPointRec(GetMousePosition(), area_da_opcao(painel, i))) {
            escolher(i, pedido);
            break;
        }
    }
    return pedido;
}

void MenuDePausa::desenhar() const {
    if (!aberto()) return;
    // Escurece o jogo por trás: deixa claro que ele está parado.
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0, 0, 0, 140 });

    const std::vector<Opcao> lista = opcoes();
    const Rectangle painel = area_do_painel(lista.size());
    DrawRectangleRec(painel, FUNDO_PAINEL);
    DrawRectangleLinesEx(painel, 2.0f, FUNDO_SOB_MOUSE);

    const char* titulo = tela_ == Tela::Principal ? "Pausa"
                       : tela_ == Tela::Salvar    ? "Salvar partida"
                       : tela_ == Tela::Carregar  ? "Carregar partida"
                                                  : pergunta_.c_str();
    DrawText(titulo, static_cast<int>(painel.x + MARGEM), static_cast<int>(painel.y + MARGEM), 20, RAYWHITE);

    const Vector2 mouse = GetMousePosition();
    for (std::size_t i = 0; i < lista.size(); ++i) {
        const Opcao& o = lista[i];
        const Rectangle r = area_da_opcao(painel, i);
        const bool sob_mouse = o.habilitada && CheckCollisionPointRec(mouse, r);
        DrawRectangleRec(r, !o.habilitada ? FUNDO_DESABILITADA : sob_mouse ? FUNDO_SOB_MOUSE : FUNDO_OPCAO);
        const int x = static_cast<int>(r.x + 12.0f);
        if (o.detalhe.empty()) {
            DrawText(o.rotulo.c_str(), x, static_cast<int>(r.y + 15.0f), 18,
                     o.habilitada ? RAYWHITE : TEXTO_DESABILITADO);
        } else {
            DrawText(o.rotulo.c_str(), x, static_cast<int>(r.y + 6.0f), 18,
                     o.habilitada ? RAYWHITE : TEXTO_DESABILITADO);
            DrawText(o.detalhe.c_str(), x, static_cast<int>(r.y + 28.0f), 14,
                     o.habilitada ? TEXTO_DETALHE : TEXTO_DESABILITADO);
        }
    }

    const std::string rodape = mensagem_.empty() ? "Esc volta" : mensagem_;
    DrawText(rodape.c_str(), static_cast<int>(painel.x + MARGEM),
             static_cast<int>(painel.y + painel.height - MARGEM - 16.0f), 16,
             mensagem_.empty() ? TEXTO_DESABILITADO : Color{ 255, 220, 160, 255 });
}
