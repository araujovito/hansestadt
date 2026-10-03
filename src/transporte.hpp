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

// Quem leva o quê para onde.
//
// A cada PLANEJAR_A_CADA ticks o transporte distribui pedidos:
// 1. Demandas: cada vaga livre na entrada de um produtor ou na despensa de
//    uma casa (descontado o que já está a caminho) pede um item. A origem é o edifício de rota mais
//    curta que tem o item disponível, seja produtor (na saída) ou depósito.
// 2. Sobras: o que está na saída de um produtor e ninguém pediu vai para o
//    depósito de rota mais curta com vaga.
// Cada pedido leva um item e vai para o carregador livre mais perto da
// origem. Item e vaga são reservados na hora, para não serem prometidos
// duas vezes. Só entram edifícios com estrada na porta.
class Transporte {
public:
    static constexpr int TICKS_POR_CELULA = 1;
    // Planejar a cada tick gastaria buscas à toa: estoques mudam devagar.
    static constexpr int PLANEJAR_A_CADA = 10;

    // Um tick do transporte: cria os carregadores de depósitos novos,
    // distribui pedidos, muda a casa de quem perdeu o depósito e move todo
    // mundo.
    void passo(Cidade& cidade);

    const std::vector<Carregador>& carregadores() const { return carregadores_; }
    CacheDeRotas& rotas() { return rotas_; }
    std::uint64_t entregas() const { return entregas_; }
    std::uint64_t perdidos() const { return perdidos_; }
    const std::vector<std::uint8_t>& depositos_criados() const { return ja_criou_; }
    std::uint64_t tick() const { return tick_; }
    static Transporte restaurar(std::vector<Carregador> carregadores, std::vector<std::uint8_t> depositos_criados,
                                std::uint64_t tick, std::uint64_t entregas, std::uint64_t perdidos);

private:
    void planejar(Cidade& cidade);
    bool atender_demandas(Cidade& cidade);
    bool escoar_sobras(Cidade& cidade);
    IdEdificio origem_mais_proxima(const Cidade& cidade, IdItem item, IdEdificio destino);
    IdEdificio deposito_mais_proximo(const Cidade& cidade, IdItem item, Celula de);
    Carregador* carregador_mais_proximo(const Cidade& cidade, Celula alvo);
    void atribuir(Cidade& cidade, Carregador& c, IdEdificio origem, IdEdificio destino, IdItem item);

    void criar_carregadores(const Cidade& cidade);
    void mudar_de_casa(const Cidade& cidade, Carregador& c);
    // Põe o carregador a caminho de `alvo`. Falso se não há rota.
    bool enviar(const Cidade& cidade, Carregador& c, Celula alvo);
    void mover(Cidade& cidade, Carregador& c);
    void chegou(Cidade& cidade, Carregador& c);
    // Desiste da tarefa: libera as reservas e, se estava carregando, o item
    // se perde.
    void cancelar(Cidade& cidade, Carregador& c);
    // O destino sumiu ou lotou com a carga na mão: leva ao depósito mais
    // perto com vaga. Sem nenhum alcançável, a carga se perde.
    void redirecionar(Cidade& cidade, Carregador& c);
    // Reage a origem ou destino demolidos antes de dar o próximo passo.
    void conferir_tarefa(Cidade& cidade, Carregador& c);

    std::vector<Carregador> carregadores_;
    // Por IdEdificio: se os carregadores daquele depósito já foram criados.
    std::vector<std::uint8_t> ja_criou_;
    CacheDeRotas rotas_;
    // Para escolher o mais perto: uma busca em largura a partir do alvo
    // mede todos os candidatos de uma vez (ver DistanciasPelaEstrada). São
    // duas porque o planejamento alterna entre medir até o destino (qual
    // origem) e até a origem (qual carregador, qual depósito); com uma só,
    // cada pedido refaria as duas buscas.
    DistanciasPelaEstrada perto_do_destino_;
    DistanciasPelaEstrada perto_da_origem_;
    std::uint64_t tick_ = 0;
    std::uint64_t entregas_ = 0;
    std::uint64_t perdidos_ = 0;
};
