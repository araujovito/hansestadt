#pragma once

#include <cstdio>

#include "conteudo.hpp"

// Conteúdo mínimo para testes de regra, independente de dados/*.toml: uma
// mudança de balanceamento no jogo não pode quebrar um teste de construção.
inline constexpr const char* ITENS_DE_TESTE = R"(
[[item]]
id = "tora"
nome = "tora"
tipo = "insumo"

[[item]]
id = "tabua"
nome = "tábua"
tipo = "insumo"

[[item]]
id = "pao"
nome = "pão"
tipo = "comida"
)";

inline constexpr const char* EDIFICIOS_DE_TESTE = R"(
[[edificio]]
id = "grande"
nome = "grande"
fase = "vila"
tamanho = [3, 3]
terreno = "campo"
cor = "#808080"

[[edificio]]
id = "pequeno"
nome = "pequeno"
fase = "vila"
tamanho = [2, 2]
terreno = "campo"
cor = "#808080"

[[edificio]]
id = "extrator"
nome = "extrator"
fase = "cidade"
tamanho = [2, 2]
terreno = "barreiro"
cor = "#808080"

[[edificio]]
id = "beira_rio"
nome = "beira-rio"
fase = "vila"
tamanho = [2, 2]
terreno = "campo"
cor = "#808080"
requer = { terreno = "agua", distancia = 1 }

[[edificio]]
id = "perto_mata"
nome = "perto da mata"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
requer = { terreno = "floresta", distancia = 2 }
extrai = "floresta"

[[edificio]]
id = "fazendinha"
nome = "fazendinha"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
campos = 1

[[edificio]]
id = "deposito"
nome = "depósito"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
estoque = 10
deposito = true
alimenta_sem_teto = true
carregadores = 2

[[edificio]]
id = "abrigo"
nome = "abrigo"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
estoque = 10
alimenta_sem_teto = true

[[edificio]]
id = "casa"
nome = "casa"
fase = "vila"
tamanho = [2, 2]
terreno = "campo"
cor = "#808080"
estoque = 8
moradores = 4

[[edificio]]
id = "com_custo"
nome = "com custo"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
custo = { tora = 2 }

[[edificio]]
id = "serraria"
nome = "serraria"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
trabalhadores = 1
estoque = 4

[[edificio.receita]]
dias = 1
entradas = { tora = 2 }
saidas = { tabua = 1 }

[[edificio]]
id = "colheita"
nome = "colheita"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
trabalhadores = 1
estoque = 4

[[edificio.receita]]
dias = 1
saidas = { pao = 1 }
# Só julho (fator 1) e agosto (fator 0,5).
sazonalidade = [0, 0, 0, 0, 0, 0, 1, 0.5, 0, 0, 0, 0]

[[edificio]]
id = "forno"
nome = "forno"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
trabalhadores = 1
estoque = 4

[[edificio.receita]]
dias = 1
entradas = { tora = 1 }
saidas = { pao = 1 }

[[edificio.receita]]
dias = 1
entradas = { tabua = 1 }
saidas = { pao = 1 }
)";

inline const Conteudo& conteudo_de_teste() {
    static const Conteudo conteudo = [] {
        ResultadoCarga r = carregar_conteudo_de_texto(ITENS_DE_TESTE, EDIFICIOS_DE_TESTE);
        for (const auto& e : r.erros) std::printf("conteúdo de teste inválido: %s\n", e.c_str());
        return std::move(*r.conteudo);
    }();
    return conteudo;
}

inline IdTipoEdificio tipo(const char* id) {
    return *conteudo_de_teste().buscar_edificio(id);
}

inline IdItem item(const char* id) {
    return *conteudo_de_teste().buscar_item(id);
}
