#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "mapa.hpp"

// Itens, edifícios e receitas carregados de dados/*.toml. Este cabeçalho não
// expõe o toml++: só conteudo.cpp o inclui.

enum class Fase : std::uint8_t { Vila, Mercado, Cidade, Hansa, Auge };
const char* nome_da_fase(Fase f);

enum class TipoItem : std::uint8_t { Insumo, Comida };

using IdItem = std::uint16_t;
using IdTipoEdificio = std::uint16_t;

struct DefinicaoItem {
    std::string id;
    std::string nome;
    TipoItem tipo;
};

struct Quantidade {
    IdItem item;
    int quantidade;
};

struct Receita {
    int dias;
    std::vector<Quantidade> entradas;
    std::vector<Quantidade> saidas;
    // Fator de produção por mês (índice 0 = janeiro).
    std::array<float, 12> sazonalidade;
};

// Pelo menos uma célula de `terreno` a até `distancia` células da borda.
struct Requisito {
    Terreno terreno;
    int distancia;
};

struct DefinicaoEdificio {
    std::string id;
    std::string nome;
    Fase fase;
    int largura;  // do edifício, sem os campos
    int altura;
    Terreno terreno;
    std::uint32_t cor;  // 0xRRGGBBAA
    int trabalhadores;
    int estoque;
    int moradores;
    int campos;  // largura do anel de lavoura em volta
    // Depósito guarda qualquer item até `estoque` no total; os demais
    // guardam só o que suas receitas usam, até `estoque` de cada.
    bool deposito;
    // Carregadores que moram no depósito e fazem todo o transporte.
    int carregadores;
    std::optional<Requisito> requer;
    // Recurso do terreno consumido a cada ciclo (ex.: floresta do lenhador).
    std::optional<Terreno> extrai;
    std::vector<Receita> receitas;
    std::vector<Quantidade> custo;

    bool usa_item(IdItem item) const;

    // Área ocupada no mapa, incluindo os campos. A origem de um edifício
    // construído é o canto dessa área; o núcleo começa `campos` células
    // para dentro.
    int largura_total() const { return largura + 2 * campos; }
    int altura_total() const { return altura + 2 * campos; }
};

// Se a célula (dx, dy), relativa à origem da área total, é lavoura e não o
// edifício em si.
inline bool eh_lavoura(const DefinicaoEdificio& d, int dx, int dy) {
    return dx < d.campos || dy < d.campos || dx >= d.campos + d.largura || dy >= d.campos + d.altura;
}

struct Conteudo {
    std::vector<DefinicaoItem> itens;
    std::vector<DefinicaoEdificio> edificios;

    const DefinicaoEdificio& edificio(IdTipoEdificio id) const { return edificios[id]; }
    std::optional<IdTipoEdificio> buscar_edificio(std::string_view id) const;
    std::optional<IdItem> buscar_item(std::string_view id) const;
};

// Todos os erros encontrados, não só o primeiro: corrigir um arquivo de
// conteúdo editado à mão é mais rápido vendo a lista inteira.
struct ResultadoCarga {
    std::optional<Conteudo> conteudo;
    std::vector<std::string> erros;
};

ResultadoCarga carregar_conteudo(const std::string& pasta);
// Mesmo que o anterior, a partir do texto dos dois arquivos (para testes).
ResultadoCarga carregar_conteudo_de_texto(std::string_view itens_toml, std::string_view edificios_toml);
