#include "simulacao.hpp"

#include "producao.hpp"

void Simulacao::passo() {
    avancar_producao(cidade, data.mes, TICKS_POR_DIA);
    ++tick;
    if (tick % TICKS_POR_DIA == 0) {
        data = dia_seguinte(data);
    }
}
