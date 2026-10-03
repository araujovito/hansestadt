#include "simulacao.hpp"

#include "producao.hpp"

void Simulacao::passo() {
    populacao.alojar(cidade);
    populacao.empregar(cidade);
    // Produção antes do transporte: o que ficou pronto neste tick já pode
    // ser pedido na mesma rodada de planejamento.
    avancar_producao(cidade, data.mes, TICKS_POR_DIA, &populacao);
    transporte.passo(cidade);
    ++tick;
    if (tick % TICKS_POR_DIA == 0) {
        populacao.consumir_comida(cidade);
        data = dia_seguinte(data);
    }
}
