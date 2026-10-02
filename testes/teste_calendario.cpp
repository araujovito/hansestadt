#include "calendario.hpp"
#include "relogio.hpp"
#include "simulacao.hpp"
#include "verificar.hpp"

TESTE("calendário juliano: todo ano divisível por 4 é bissexto, inclusive 1300") {
    VERIFICAR(bissexto(1348));
    VERIFICAR(bissexto(1300));  // no gregoriano não seria
    VERIFICAR(!bissexto(1349));
    VERIFICAR((dia_seguinte({ 1300, 2, 28 }) == Data{ 1300, 2, 29 }));
    VERIFICAR((dia_seguinte({ 1349, 2, 28 }) == Data{ 1349, 3, 1 }));
}

TESTE("calendário: virada de mês e de ano") {
    VERIFICAR((dia_seguinte({ 1150, 4, 30 }) == Data{ 1150, 5, 1 }));
    VERIFICAR((dia_seguinte({ 1349, 12, 31 }) == Data{ 1350, 1, 1 }));
}

TESTE("calendário: 1150 a 1370 tem 220 anos julianos de 365,25 dias") {
    Data d{ 1150, 3, 1 };
    long dias = 0;
    while (d < Data{ 1370, 3, 1 }) {
        d = dia_seguinte(d);
        ++dias;
    }
    VERIFICAR(dias == 220L * 365 + 55);
}

TESTE("simulação: um dia de jogo a cada TICKS_POR_DIA ticks") {
    Simulacao sim;
    for (int i = 0; i < Simulacao::TICKS_POR_DIA * 10; ++i) sim.passo();
    VERIFICAR((sim.data == Data{ 1150, 3, 11 }));
}

TESTE("relógio: 4× dá os mesmos ticks em um quarto do tempo real") {
    Relogio normal(0.05);
    Relogio rapido(0.05);
    rapido.definir_indice_velocidade(2);
    int ticks_normal = 0;
    int ticks_rapido = 0;
    for (int i = 0; i < 240; ++i) ticks_normal += normal.passos_para(1.0 / 60.0);
    for (int i = 0; i < 60; ++i) ticks_rapido += rapido.passos_para(1.0 / 60.0);
    // Pode diferir em um tick pelo arredondamento do acumulador.
    VERIFICAR(ticks_normal >= 79 && ticks_normal <= 80);
    VERIFICAR(ticks_rapido >= 79 && ticks_rapido <= 80);
}

TESTE("relógio: pausado não avança") {
    Relogio r(0.05);
    r.alternar_pausa();
    VERIFICAR(r.passos_para(1.0) == 0);
}
