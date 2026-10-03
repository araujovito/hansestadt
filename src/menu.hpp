#pragma once

#include <string>
#include <vector>

#include "conteudo.hpp"
#include "partidas.hpp"

// O que o jogo deve fazer depois de um quadro com o menu aberto. O menu só
// desenha e lê a entrada; quem salva, carrega e fecha a janela é o main, que
// tem a simulação.
struct PedidoDoMenu {
    enum class Acao { Nenhuma, Salvar, Carregar, Sair };
    Acao acao = Acao::Nenhuma;
    std::string arquivo;  // vaga escolhida, para Salvar e Carregar
};

// Menu de pausa: continuar, salvar, carregar e sair. Enquanto está aberto a
// simulação não avança e o mapa não recebe cliques.
class MenuDePausa {
public:
    bool aberto() const { return tela_ != Tela::Fechado; }
    // `pode_salvar` é falso antes da fundação: ainda não há vila para guardar.
    void abrir(const Conteudo& conteudo, bool pode_salvar);
    void fechar() { tela_ = Tela::Fechado; }

    // Lê teclado e mouse do quadro. Esc volta uma tela, e na principal fecha.
    PedidoDoMenu atualizar();
    void desenhar() const;

    // Resultado de um pedido, mostrado no rodapé do menu. Depois de salvar,
    // as vagas são examinadas de novo para o resumo aparecer.
    void informar(std::string mensagem) { mensagem_ = std::move(mensagem); }
    void reexaminar();

private:
    enum class Tela { Fechado, Principal, Salvar, Carregar, Confirmar };

    struct Opcao {
        std::string rotulo;
        std::string detalhe;  // segunda linha, menor (resumo da vaga)
        bool habilitada = true;
    };

    std::vector<Opcao> opcoes() const;
    void escolher(std::size_t indice, PedidoDoMenu& pedido);
    void pedir_confirmacao(std::string pergunta, std::string consequencia, PedidoDoMenu pedido);

    Tela tela_ = Tela::Fechado;
    const Conteudo* conteudo_ = nullptr;
    bool pode_salvar_ = false;
    std::vector<SituacaoDaVaga> vagas_;
    std::string mensagem_;
    // Tela de confirmação: a pergunta (no título), o que acontece se o
    // jogador disser sim (embaixo do "Sim"), o pedido feito nesse caso e a
    // tela para onde voltar se disser não.
    std::string pergunta_;
    std::string consequencia_;
    PedidoDoMenu pendente_;
    Tela voltar_para_ = Tela::Principal;
};
