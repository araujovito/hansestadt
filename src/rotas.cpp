#include "rotas.hpp"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <tuple>

namespace {

int distancia_manhattan(Celula a, Celula b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

}  // namespace

// A* com custo 1 por célula e heurística de Manhattan, que é admissível em
// grade de quatro direções: a rota encontrada é sempre a mais curta.
std::optional<std::vector<Celula>> buscar_rota(const Cidade& cidade, Celula de, Celula para) {
    if (!cidade.tem_estrada(de) || !cidade.tem_estrada(para)) return std::nullopt;
    if (de == para) return std::vector<Celula>{ de };

    const int largura = cidade.mapa().largura();
    const auto total = static_cast<std::size_t>(largura) * static_cast<std::size_t>(cidade.mapa().altura());
    auto indice = [largura](Celula c) { return static_cast<std::size_t>(c.y * largura + c.x); };
    auto celula = [largura](std::size_t i) {
        return Celula{ static_cast<int>(i) % largura, static_cast<int>(i) / largura };
    };

    constexpr int INFINITO = std::numeric_limits<int>::max();
    std::vector<int> custo(total, INFINITO);
    std::vector<std::size_t> veio_de(total, total);

    // (f, h, índice): empates em f são decididos por h e depois pelo
    // índice, para que duas rotas de mesmo comprimento saiam sempre iguais.
    using No = std::tuple<int, int, std::size_t>;
    std::priority_queue<No, std::vector<No>, std::greater<No>> fronteira;

    custo[indice(de)] = 0;
    fronteira.emplace(distancia_manhattan(de, para), distancia_manhattan(de, para), indice(de));

    constexpr Celula VIZINHOS[] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };

    while (!fronteira.empty()) {
        const auto [f, h, atual] = fronteira.top();
        fronteira.pop();
        const Celula c = celula(atual);
        if (c == para) break;
        // Entrada velha na fila: a célula já foi alcançada por um caminho
        // mais curto depois que esta foi enfileirada.
        if (f - h > custo[atual]) continue;

        for (const Celula d : VIZINHOS) {
            const Celula v{ c.x + d.x, c.y + d.y };
            if (!cidade.tem_estrada(v)) continue;
            const std::size_t iv = indice(v);
            const int novo = custo[atual] + 1;
            if (novo >= custo[iv]) continue;
            custo[iv] = novo;
            veio_de[iv] = atual;
            const int hv = distancia_manhattan(v, para);
            fronteira.emplace(novo + hv, hv, iv);
        }
    }

    if (custo[indice(para)] == INFINITO) return std::nullopt;

    std::vector<Celula> rota;
    for (std::size_t i = indice(para); i != total; i = veio_de[i]) rota.push_back(celula(i));
    std::reverse(rota.begin(), rota.end());
    return rota;
}

const std::optional<std::vector<Celula>>& CacheDeRotas::rota(const Cidade& cidade, Celula de, Celula para) {
    if (versao_ != cidade.versao_estradas() || rotas_.size() >= MAXIMO) {
        rotas_.clear();
        versao_ = cidade.versao_estradas();
    }
    const auto largura = static_cast<std::uint64_t>(cidade.mapa().largura());
    const std::uint64_t chave = (static_cast<std::uint64_t>(de.y) * largura + static_cast<std::uint64_t>(de.x)) << 32 |
                                (static_cast<std::uint64_t>(para.y) * largura + static_cast<std::uint64_t>(para.x));
    auto it = rotas_.find(chave);
    if (it == rotas_.end()) {
        ++buscas_;
        it = rotas_.emplace(chave, buscar_rota(cidade, de, para)).first;
    }
    return it->second;
}
