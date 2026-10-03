#include "simulacao.hpp"

#include "producao.hpp"

void Simulacao::passo() {
    populacao.alojar(cidade);
    populacao.empregar(cidade, true, trabalho_infantil);
    // Produção antes do transporte: o que ficou pronto neste tick já pode
    // ser pedido na mesma rodada de planejamento.
    avancar_producao(cidade, data.mes, TICKS_POR_DIA, &populacao, &eventos, data);
    transporte.passo(cidade, &populacao);
    ++tick;
    if (tick % TICKS_POR_DIA == 0) {
        populacao.consumir_comida(cidade);
        data = dia_seguinte(data);
        populacao.avancar_dia(cidade, data, &eventos);
        fase = tentar_avancar_fase(fase, data, estados_das_instituicoes(cidade, fase)).fase;
    }
}
