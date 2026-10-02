#pragma once

#include <cstdint>
#include <vector>

enum class Terreno : std::uint8_t {
    Campo,     // terra aberta: casas, lavouras, estradas
    Floresta,  // madeira e carvão; precisa ser derrubada para construir
    Barreiro,  // depósito de argila, base do tijolo
    Agua,      // mar Báltico e rio; navegável mais tarde
    Turfa,     // brejo de turfa: combustível típico da planície do norte
};

const char* nome_do_terreno(Terreno t);

struct Celula {
    int x;
    int y;

    bool operator==(const Celula&) const = default;
};

// Grade do mapa guardada num vetor contíguo, linha a linha. Células são
// endereçadas por (x, y); nenhuma outra estrutura guarda ponteiros para elas.
class Mapa {
public:
    Mapa(int largura, int altura);

    int largura() const { return largura_; }
    int altura() const { return altura_; }

    bool dentro(int x, int y) const {
        return x >= 0 && y >= 0 && x < largura_ && y < altura_;
    }

    Terreno terreno(int x, int y) const { return celulas_[indice(x, y)]; }
    void definir_terreno(int x, int y, Terreno t) { celulas_[indice(x, y)] = t; }

private:
    int indice(int x, int y) const { return y * largura_ + x; }

    int largura_;
    int altura_;
    std::vector<Terreno> celulas_;
};

// Gera um trecho de costa báltica: mar ao norte, um rio descendo do sul até
// a foz, argila nas margens, florestas e brejos de turfa no interior.
Mapa gerar_mapa(int largura, int altura, std::uint32_t semente);
