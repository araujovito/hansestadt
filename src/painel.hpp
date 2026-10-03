#pragma once

#include <optional>
#include <vector>

#include "raylib.h"

#include "alertas.hpp"
#include "cidade.hpp"
#include "populacao.hpp"

// Painel lateral com o estado do edifício selecionado: o que produz, por que
// parou, o ritmo, as técnicas e o que tem no estoque. Instituições mostram o
// nível, o custo da próxima melhoria e o botão de melhorar; edifícios com
// técnica, o botão de pesquisar a próxima.
Rectangle area_do_painel_edificio();
void desenhar_painel_edificio(const Cidade& cidade, const Populacao& populacao, IdEdificio id, int mes,
                              int ticks_por_dia, Fase fase);
// Se o clique deste quadro foi no botão de melhorar, liberado ou não; quem
// chama tenta a melhoria e mostra o resultado. Fora de instituição é falso.
bool clicou_em_melhorar(const Cidade& cidade, IdEdificio id);
// A técnica do botão de pesquisar, se o clique deste quadro foi nele;
// quem chama tenta a pesquisa e mostra o resultado. Edifício sem técnica
// pendente não tem botão.
std::optional<IdTecnica> clicou_em_pesquisar(const Cidade& cidade, IdEdificio id, Fase fase);

// Lista de alertas no canto esquerdo, abaixo da barra de cima. Mostra no
// máximo MAXIMO_ALERTAS_NA_TELA; os demais são resumidos numa última linha.
inline constexpr std::size_t MAXIMO_ALERTAS_NA_TELA = 6;
void desenhar_alertas(const std::vector<Alerta>& alertas);
// Índice do alerta sob o mouse, ou -1. A área inteira da lista também conta
// como "mouse na barra", para o clique não atravessar para o mapa.
int alerta_sob_mouse(const std::vector<Alerta>& alertas);
bool mouse_sobre_alertas(const std::vector<Alerta>& alertas);
