#pragma once

#include <algorithm>
#include <vector>

#include "calendario.hpp"
#include "cidade.hpp"
#include "eventos.hpp"

struct Morador {
    static constexpr int DIAS_POR_ANO = 365;
    static constexpr int IDADE_ADULTA = 16 * DIAS_POR_ANO;
    // Com o trabalho infantil permitido, a partir desta idade a criança
    // ocupa as vagas de produção que os adultos deixaram.
    static constexpr int IDADE_TRABALHO_INFANTIL = 10 * DIAS_POR_ANO;
    // Cada geração nascida na vila rende 5% a mais que a anterior, até a
    // quinta (+25%). Os colonos fundadores são a geração 0.
    static constexpr int GANHO_POR_GERACAO = 50;  // milésimos
    static constexpr int GERACOES_COM_GANHO = 5;
    IdEdificio casa = SEM_EDIFICIO;
    IdEdificio trabalho = SEM_EDIFICIO;
    int dias_sem_comida = 0;
    int idade_dias = 20 * DIAS_POR_ANO;
    int geracao = 0;
    bool mulher = false;
    bool vivo = true;

    bool adulto() const { return idade_dias >= IDADE_ADULTA; }
    // Quanto rende no trabalho, em milésimos de um colono fundador adulto.
    // Criança rende metade do que renderia adulta.
    int produtividade() const {
        const int adulta = 1000 + GANHO_POR_GERACAO * std::min(geracao, GERACOES_COM_GANHO);
        return adulto() ? adulta : adulta / 2;
    }
};

// Moradores pertencem à simulação, não aos edifícios: assim continuam
// existindo quando uma casa é demolida e podem procurar outra vaga.
class Populacao {
public:
    static constexpr int DIAS_ATE_MORRER_DE_FOME = 30;
    explicit Populacao(int quantidade_inicial = 6);

    void alojar(Cidade& cidade);
    // Adultos primeiro, em todas as vagas. Com `trabalho_infantil`, crianças
    // a partir de 10 anos completam as vagas de produção que sobraram; as
    // vagas de carregador do armazém continuam só com adultos.
    void empregar(const Cidade& cidade, bool incluir_carregadores = false, bool trabalho_infantil = false);
    void consumir_comida(Cidade& cidade);
    void avancar_dia(Cidade& cidade, Data data, const Eventos* eventos = nullptr);

    int total() const;
    int alojados() const;
    int sem_casa() const { return total() - alojados(); }
    int moradores_na_casa(IdEdificio casa) const;
    int trabalhadores_em(IdEdificio edificio) const;
    // Soma da produtividade de quem trabalha no edifício, em milésimos
    // (dois fundadores adultos = 2000).
    int produtividade_em(IdEdificio edificio) const;
    int desempregados() const;
    int mortos_de_fome() const { return mortos_de_fome_; }
    int mortos_por_eventos() const { return mortos_por_eventos_; }
    int refeicoes_perdidas() const { return refeicoes_perdidas_; }
    int criancas() const;
    int criancas_trabalhando() const;
    // Consumo diário em meias porções: adulto vale 2, criança vale 1.
    int consumo_diario_x2() const { return 2 * total() - criancas(); }
    const std::vector<Morador>& moradores() const { return moradores_; }
    static Populacao restaurar(std::vector<Morador> moradores, int mortos_de_fome, int refeicoes_perdidas,
                               int mortos_por_eventos = 0);

private:
    std::vector<Morador> moradores_;
    // Por IdEdificio, quantos vivos trabalham lá. Montado em empregar e
    // ajustado nas mortes: a produção consulta todo edifício a cada tick, e
    // contar percorrendo a população custava edifícios × moradores.
    std::vector<int> trabalhadores_;
    // Mesma ideia, somando Morador::produtividade.
    std::vector<int> produtividade_;
    int mortos_de_fome_ = 0;
    int mortos_por_eventos_ = 0;
    int refeicoes_perdidas_ = 0;
};
