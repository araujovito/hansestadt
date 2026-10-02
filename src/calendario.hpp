#pragma once

// Calendário juliano, o que vigorava na Europa até 1582: ano bissexto a cada
// quatro anos, sem a exceção dos séculos do gregoriano. Usá-lo permite que
// eventos históricos caiam exatamente nas datas registradas nas fontes.
struct Data {
    int ano;
    int mes;  // 1..12
    int dia;  // 1..dias_no_mes

    auto operator<=>(const Data&) const = default;
};

bool bissexto(int ano);
int dias_no_mes(int ano, int mes);
Data dia_seguinte(Data d);
const char* nome_do_mes(int mes);
