#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "cidade.hpp"

// Rota mais curta pelas estradas, de `de` até `para`, com as duas pontas
// incluídas. Anda só em células com estrada e só nas quatro direções: uma
// diagonal entre duas estradas que só se tocam pela quina não é caminho.
// Devolve nullopt se uma das pontas não tem estrada ou se não há ligação.
std::optional<std::vector<Celula>> buscar_rota(const Cidade& cidade, Celula de, Celula para);

// Guarda as rotas já calculadas. Uma vila repete as mesmas viagens (fazenda
// para moinho, moinho para padaria) milhares de vezes, e cada A* percorre o
// mapa inteiro no pior caso. Qualquer mudança nas estradas descarta tudo: é
// mais simples e seguro do que descobrir quais rotas passavam pelo trecho.
class CacheDeRotas {
public:
    // A referência vale até a próxima chamada.
    const std::optional<std::vector<Celula>>& rota(const Cidade& cidade, Celula de, Celula para);

    std::size_t guardadas() const { return rotas_.size(); }
    std::uint64_t buscas_feitas() const { return buscas_; }

private:
    // Limite para o cache não crescer sem fim numa partida longa sem obras.
    static constexpr std::size_t MAXIMO = 4096;

    std::uint64_t versao_ = UINT64_MAX;
    std::uint64_t buscas_ = 0;
    // Só consultado por chave, nunca percorrido, então a ordem interna do
    // unordered_map não afeta o resultado da simulação.
    std::unordered_map<std::uint64_t, std::optional<std::vector<Celula>>> rotas_;
};
