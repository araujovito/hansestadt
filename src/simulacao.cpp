#include "simulacao.hpp"

void Simulacao::passo() {
    ++tick;
    if (tick % TICKS_POR_DIA == 0) {
        data = dia_seguinte(data);
    }
}
