#pragma once

#include <vector>

#include "calendario.hpp"
#include "cidade.hpp"

struct Morador {
    static constexpr int DIAS_POR_ANO = 365;
    static constexpr int IDADE_ADULTA = 16 * DIAS_POR_ANO;
    IdEdificio casa = SEM_EDIFICIO;
    IdEdificio trabalho = SEM_EDIFICIO;
    int dias_sem_comida = 0;
    int idade_dias = 20 * DIAS_POR_ANO;
    bool mulher = false;
    bool vivo = true;

    bool adulto() const { return idade_dias >= IDADE_ADULTA; }
};

// Moradores pertencem à simulação, não aos edifícios: assim continuam
// existindo quando uma casa é demolida e podem procurar outra vaga.
class Populacao {
public:
    static constexpr int DIAS_ATE_MORRER_DE_FOME = 30;
    explicit Populacao(int quantidade_inicial = 6);

    void alojar(Cidade& cidade);
    void empregar(const Cidade& cidade);
    void consumir_comida(Cidade& cidade);
    void avancar_dia(Cidade& cidade, Data data);

    int total() const;
    int alojados() const;
    int sem_casa() const { return total() - alojados(); }
    int moradores_na_casa(IdEdificio casa) const;
    int trabalhadores_em(IdEdificio edificio) const;
    int desempregados() const;
    int mortos_de_fome() const { return mortos_de_fome_; }
    int refeicoes_perdidas() const { return refeicoes_perdidas_; }
    int criancas() const;
    const std::vector<Morador>& moradores() const { return moradores_; }
    static Populacao restaurar(std::vector<Morador> moradores, int mortos_de_fome, int refeicoes_perdidas);

private:
    std::vector<Morador> moradores_;
    // Por IdEdificio, quantos vivos trabalham lá. Montado em empregar e
    // ajustado nas mortes: a produção consulta todo edifício a cada tick, e
    // contar percorrendo a população custava edifícios × moradores.
    std::vector<int> trabalhadores_;
    int mortos_de_fome_ = 0;
    int refeicoes_perdidas_ = 0;
};
