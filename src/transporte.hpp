#pragma once

#include <cstdint>
#include <vector>

#include "cidade.hpp"
#include "rotas.hpp"

enum class Tarefa : std::uint8_t {
    Nenhuma,  // livre: parado em casa ou voltando para lá
    Buscar,   // indo à origem pegar um item
    Entregar, // levando o item ao destino
};

// Mora num depósito e anda só por estrada, uma célula a cada
// Transporte::TICKS_POR_CELULA. Sempre está numa célula; o desenho
// interpola entre ela e a próxima da rota.
struct Carregador {
    IdEdificio casa;
    Celula posicao;
    // Caminho em andamento: rota[passo] é a próxima célula a pisar.
    std::vector<Celula> rota;
    std::size_t passo = 0;
    int ticks_na_celula = 0;

    Tarefa tarefa = Tarefa::Nenhuma;
    IdEdificio origem = SEM_EDIFICIO;
    IdEdificio destino = SEM_EDIFICIO;
    IdItem item = 0;
    bool carregando = false;

    bool andando() const { return passo < rota.size(); }
    bool livre() const { return tarefa == Tarefa::Nenhuma; }
};

class Transporte {
public:
    static constexpr int TICKS_POR_CELULA = 3;

    // Um tick do transporte: cria os carregadores de depósitos novos, muda
    // a casa de quem perdeu o depósito e move todo mundo.
    void passo(Cidade& cidade);

    const std::vector<Carregador>& carregadores() const { return carregadores_; }
    CacheDeRotas& rotas() { return rotas_; }

private:
    void criar_carregadores(const Cidade& cidade);
    void mudar_de_casa(const Cidade& cidade, Carregador& c);
    // Põe o carregador a caminho de `alvo`. Falso se não há rota.
    bool enviar(const Cidade& cidade, Carregador& c, Celula alvo);
    void mover(Cidade& cidade, Carregador& c);
    void chegou(Cidade& cidade, Carregador& c);

    std::vector<Carregador> carregadores_;
    // Por IdEdificio: se os carregadores daquele depósito já foram criados.
    std::vector<std::uint8_t> ja_criou_;
    CacheDeRotas rotas_;
};
