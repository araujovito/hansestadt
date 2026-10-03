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
using IdTecnica = std::uint16_t;

struct DefinicaoItem {
    std::string id;
    std::string nome;
    TipoItem tipo;
};

struct Quantidade {
    IdItem item;
    int quantidade;
};

struct MelhoriaEdificio {
    Fase fase;
    std::vector<Quantidade> custo;
};

struct Receita {
    int dias;
    std::vector<Quantidade> entradas;
    std::vector<Quantidade> saidas;
    // Fator de produção por mês (índice 0 = janeiro).
    std::array<float, 12> sazonalidade;
};

// Onde o edifício fica na cadeia de produção, calculado das receitas ao
// carregar o conteúdo. Decide quem come primeiro quando falta comida.
enum class EloDaCadeia : std::uint8_t {
    MateriaPrima,  // receitas sem entradas (fazenda, lenhador), ou sem receita
    Insumo,        // tudo o que produz é entrada de outra receita (moinho)
    ProdutoFinal,  // produz algo que nenhuma receita consome (padaria)
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
    // Distribui comida diretamente a moradores sem casa.
    bool alimenta_sem_teto;
    // Carregadores que moram no depósito e fazem todo o transporte.
    int carregadores;
    std::optional<Requisito> requer;
    // Recurso do terreno consumido a cada ciclo (ex.: floresta do lenhador).
    std::optional<Terreno> extrai;
    // Instituição evolui por fases; cada melhoria libera o nível seguinte.
    std::optional<std::string> instituicao;
    std::vector<MelhoriaEdificio> melhorias;
    std::vector<Receita> receitas;
    std::vector<Quantidade> custo;
    EloDaCadeia elo = EloDaCadeia::MateriaPrima;

    bool usa_item(IdItem item) const;

    // Área ocupada no mapa, incluindo os campos. A origem de um edifício
    // construído é o canto dessa área; o núcleo começa `campos` células
    // para dentro.
    int largura_total() const { return largura + 2 * campos; }
    int altura_total() const { return altura + 2 * campos; }
};

int nivel_maximo_disponivel(const DefinicaoEdificio& edificio, Fase fase);

// Técnica de produção: pesquisada uma vez, vale para todos os edifícios dos
// tipos listados, inclusive os construídos depois (como nos Age of Empires).
struct Tecnica {
    std::string id;
    std::string nome;
    std::string descricao;
    Fase fase;  // a partir de quando pode ser pesquisada
    std::vector<IdTipoEdificio> edificios;
    // Técnica que precisa ter sido pesquisada antes (o nível anterior).
    std::optional<IdTecnica> requer;
    std::vector<Quantidade> custo;
    // Produtividade a mais, em milésimos (750 = +75%). Somada às outras
    // técnicas pesquisadas do mesmo tipo de edifício.
    int bonus;
};

// Se a célula (dx, dy), relativa à origem da área total, é lavoura e não o
// edifício em si.
inline bool eh_lavoura(const DefinicaoEdificio& d, int dx, int dy) {
    return dx < d.campos || dy < d.campos || dx >= d.campos + d.largura || dy >= d.campos + d.altura;
}

struct Conteudo {
    std::vector<DefinicaoItem> itens;
    std::vector<DefinicaoEdificio> edificios;
    std::vector<Tecnica> tecnicas;

    const DefinicaoEdificio& edificio(IdTipoEdificio id) const { return edificios[id]; }
    std::optional<IdTipoEdificio> buscar_edificio(std::string_view id) const;
    std::optional<IdItem> buscar_item(std::string_view id) const;
    std::optional<IdTecnica> buscar_tecnica(std::string_view id) const;
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
