#pragma once

#include <cstdint>

#include "cidade.hpp"
#include "calendario.hpp"

class Populacao;
class Eventos;

// O fator de sazonalidade vira inteiro em milésimos: o progresso de um ciclo
// é uma soma de inteiros, sem erro de arredondamento acumulado ao longo de
// anos de jogo e igual em qualquer máquina.
inline constexpr int ESCALA_FATOR = 1000;

int fator_do_mes(const Receita& r, int mes);
// Progresso necessário para completar um ciclo da receita.
std::int64_t progresso_total(const Receita& r, int ticks_por_dia);

// Avança um tick da produção de todos os edifícios.
//
// Um edifício parado começa a primeira receita (na ordem do conteúdo) que
// tem entradas no estoque, espaço para as saídas e fator > 0 no mês; as
// entradas são consumidas no início. A cada tick o progresso cresce pelo
// fator do mês, então o mesmo ciclo dura o dobro com fator 0,5 e pausa com
// fator 0. Ao completar, as saídas vão para o estoque do próprio edifício.
void avancar_producao(Cidade& cidade, int mes, int ticks_por_dia, const Populacao* populacao = nullptr,
                      const Eventos* eventos = nullptr, Data data = { 1, 1, 1 });

// Produtividade média de quem trabalha no edifício, em milésimos: 1000 com
// fundadores adultos, mais com gerações nascidas na vila, menos com
// crianças. Sem população (testes de produção), 1000.
int ritmo_da_equipe(const Cidade& cidade, IdEdificio id, const Populacao* populacao);

enum class Situacao : std::uint8_t {
    SemReceita,     // armazém, casa
    SemTrabalhadores,
    Produzindo,
    Pausado,        // ciclo começado, mas o fator do mês é 0
    ForaDeEstacao,  // parado porque nenhuma receita rende neste mês
    FaltaEntrada,
    SaidaCheia,
};

const char* descrever(Situacao s);

// O que o edifício está fazendo, para o jogador entender uma cadeia parada.
// Se nenhuma receita pode começar, devolve o motivo da primeira receita que
// rende no mês, conferindo primeiro a saída: com a saída cheia, faltar
// entrada não importa.
Situacao situacao(const Cidade& cidade, IdEdificio id, int mes, const Populacao* populacao = nullptr);
