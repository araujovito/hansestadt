#pragma once

#include <optional>
#include <string>
#include <vector>

#include "cidade.hpp"

enum class Ferramenta : std::uint8_t {
    Nenhuma,
    Estrada,
    Edificio,
    Demolir,
};

struct Botao {
    std::string rotulo;
    Ferramenta ferramenta;
    IdTipoEdificio tipo;  // só vale para Ferramenta::Edificio
};

struct EstadoFerramenta {
    Ferramenta ativa = Ferramenta::Nenhuma;
    IdTipoEdificio tipo = 0;
    // Célula onde o arraste da estrada começou, enquanto o botão está apertado.
    std::optional<Celula> inicio_estrada;
    std::vector<Botao> botoes;
};

// Monta a barra com estrada, os edifícios liberados até `fase` e demolir.
EstadoFerramenta criar_ferramentas(const Conteudo& conteudo, Fase fase);

// Atalhos de teclado: R estrada, X demolir, Esc solta a ferramenta.
void ler_atalhos(EstadoFerramenta& estado);

// Trata cliques na barra inferior. Devolve true se o mouse está sobre ela,
// para que o clique não atravesse e chegue ao mapa.
bool atualizar_barra(EstadoFerramenta& estado);

// Aplica a ferramenta ativa à célula sob o cursor conforme o mouse.
void aplicar_ferramenta(EstadoFerramenta& estado, Cidade& cidade, Celula cursor);

// Prévia no mundo (chamar dentro de BeginMode2D).
void desenhar_previa(const EstadoFerramenta& estado, const Cidade& cidade, Celula cursor);
// Barra de ferramentas na tela.
void desenhar_barra(const EstadoFerramenta& estado);
// Texto de ajuda sobre a ação sob o cursor, vazio se não houver.
std::string dica_da_ferramenta(const EstadoFerramenta& estado, const Cidade& cidade, Celula cursor);
