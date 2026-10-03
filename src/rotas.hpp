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

// Quantos passos pela estrada separam uma célula de todas as outras, numa
// busca só. Escolher o mais perto entre muitos candidatos (o carregador
// livre, o depósito com vaga) com buscar_rota custa um A* por candidato;
// com isto custa uma busca em largura a partir do alvo. Como todo passo
// custa 1 e as estradas valem nos dois sentidos, `ate(c)` é sempre
// buscar_rota(de, c)->size() - 1.
class DistanciasPelaEstrada {
public:
    static constexpr int SEM_LIGACAO = -1;

    // Refaz a busca a partir de `de`, a menos que já seja a última feita
    // para a mesma cidade, a mesma origem e a mesma versão das estradas.
    // Sem estrada em `de`, nada é alcançável.
    void calcular(const Cidade& cidade, Celula de);
    // SEM_LIGACAO fora do mapa ou sem caminho até `c`.
    int ate(Celula c) const;

    std::uint64_t buscas_feitas() const { return buscas_; }

private:
    const Cidade* cidade_ = nullptr;
    Celula de_{ -1, -1 };
    std::uint64_t versao_ = UINT64_MAX;
    std::uint64_t buscas_ = 0;
    int largura_ = 0;
    int altura_ = 0;
    std::vector<int> passos_;
    std::vector<std::size_t> fila_;
};

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
