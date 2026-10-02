#include "calendario.hpp"

bool bissexto(int ano) {
    return ano % 4 == 0;
}

int dias_no_mes(int ano, int mes) {
    static constexpr int dias[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (mes == 2 && bissexto(ano)) return 29;
    return dias[mes - 1];
}

Data dia_seguinte(Data d) {
    if (++d.dia > dias_no_mes(d.ano, d.mes)) {
        d.dia = 1;
        if (++d.mes > 12) {
            d.mes = 1;
            ++d.ano;
        }
    }
    return d;
}

const char* nome_do_mes(int mes) {
    static constexpr const char* nomes[] = {
        "janeiro", "fevereiro", "março", "abril", "maio", "junho",
        "julho", "agosto", "setembro", "outubro", "novembro", "dezembro",
    };
    return nomes[mes - 1];
}
