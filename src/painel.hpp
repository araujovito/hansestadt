#pragma once

#include "raylib.h"

#include "cidade.hpp"

// Painel lateral com o estado do edifício selecionado: o que produz, por que
// parou e o que tem no estoque.
Rectangle area_do_painel_edificio();
void desenhar_painel_edificio(const Cidade& cidade, IdEdificio id, int mes, int ticks_por_dia);
