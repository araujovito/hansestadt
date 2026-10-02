#include "cidade.hpp"
#include "conteudo_de_teste.hpp"
#include "verificar.hpp"

namespace {

// Mapa 10×10 de campo com uma faixa conhecida de cada terreno, para os testes
// não dependerem do gerador.
Cidade cidade_de_teste() {
    Mapa m(10, 10);
    for (int x = 0; x < 10; ++x) {
        m.definir_terreno(x, 0, Terreno::Agua);
        m.definir_terreno(x, 8, Terreno::Barreiro);
        m.definir_terreno(x, 9, Terreno::Barreiro);
    }
    m.definir_terreno(5, 5, Terreno::Floresta);
    return Cidade(std::move(m), conteudo_de_teste());
}

}  // namespace

TESTE("cidade: constrói em campo livre e ocupa todas as células") {
    Cidade c = cidade_de_teste();
    const auto id = c.construir_edificio(tipo("grande"), { 1, 1 });
    VERIFICAR(id.has_value());
    VERIFICAR(c.edificio_em({ 1, 1 }) == *id);
    VERIFICAR(c.edificio_em({ 3, 3 }) == *id);
    VERIFICAR(c.edificio_em({ 4, 4 }) == SEM_EDIFICIO);
}

TESTE("cidade: recusa sobreposição, terreno errado e borda do mapa") {
    Cidade c = cidade_de_teste();
    c.construir_edificio(tipo("grande"), { 1, 1 });
    VERIFICAR(c.validar_edificio(tipo("pequeno"), { 3, 3 }) == Validacao::Ocupado);
    VERIFICAR(c.validar_edificio(tipo("pequeno"), { 0, 0 }) == Validacao::TerrenoErrado);  // água
    VERIFICAR(c.validar_edificio(tipo("pequeno"), { 4, 4 }) == Validacao::TerrenoErrado);  // floresta em (5,5)
    VERIFICAR(c.validar_edificio(tipo("pequeno"), { 9, 2 }) == Validacao::ForaDoMapa);
    VERIFICAR(!c.construir_edificio(tipo("pequeno"), { 3, 3 }).has_value());
}

TESTE("cidade: extratores só sobre o próprio recurso") {
    Cidade c = cidade_de_teste();
    VERIFICAR(c.validar_edificio(tipo("extrator"), { 2, 8 }) == Validacao::Ok);
    VERIFICAR(c.validar_edificio(tipo("extrator"), { 2, 6 }) == Validacao::TerrenoErrado);
    VERIFICAR(c.validar_edificio(tipo("pequeno"), { 2, 8 }) == Validacao::TerrenoErrado);
}

TESTE("cidade: estrada em campo e argila, não em água, floresta ou edifício") {
    Cidade c = cidade_de_teste();
    c.construir_edificio(tipo("pequeno"), { 1, 1 });
    VERIFICAR(c.validar_estrada({ 4, 2 }) == Validacao::Ok);
    VERIFICAR(c.validar_estrada({ 4, 8 }) == Validacao::Ok);
    VERIFICAR(c.validar_estrada({ 4, 0 }) == Validacao::TerrenoErrado);
    VERIFICAR(c.validar_estrada({ 5, 5 }) == Validacao::TerrenoErrado);
    VERIFICAR(c.validar_estrada({ 1, 1 }) == Validacao::Ocupado);
    VERIFICAR(c.construir_estrada({ 4, 2 }));
    VERIFICAR(c.validar_estrada({ 4, 2 }) == Validacao::JaExiste);
    VERIFICAR(c.validar_edificio(tipo("pequeno"), { 4, 2 }) == Validacao::Ocupado);
}

TESTE("cidade: demolir libera as células e não reaproveita o id") {
    Cidade c = cidade_de_teste();
    const auto primeiro = c.construir_edificio(tipo("pequeno"), { 1, 1 });
    VERIFICAR(c.demolir({ 2, 2 }));
    VERIFICAR(c.edificio_em({ 1, 1 }) == SEM_EDIFICIO);
    VERIFICAR(!c.edificios()[*primeiro].ativo);
    const auto segundo = c.construir_edificio(tipo("pequeno"), { 1, 1 });
    VERIFICAR(segundo.has_value() && *segundo != *primeiro);
    VERIFICAR(!c.demolir({ 6, 6 }));  // nada ali
}

TESTE("cidade: demolir estrada antes de edifício na mesma operação") {
    Cidade c = cidade_de_teste();
    c.construir_estrada({ 6, 6 });
    VERIFICAR(c.demolir({ 6, 6 }));
    VERIFICAR(!c.tem_estrada({ 6, 6 }));
}

TESTE("caminho em L: horizontal primeiro, sem repetir a quina") {
    const auto caminho = caminho_em_l({ 1, 1 }, { 3, 3 });
    const std::vector<Celula> esperado{ { 1, 1 }, { 2, 1 }, { 3, 1 }, { 3, 2 }, { 3, 3 } };
    VERIFICAR(caminho == esperado);
}

TESTE("caminho em L: funciona para trás e com uma célula só") {
    const std::vector<Celula> para_tras{ { 3, 2 }, { 2, 2 }, { 1, 2 }, { 1, 1 } };
    VERIFICAR(caminho_em_l({ 3, 2 }, { 1, 1 }) == para_tras);
    const std::vector<Celula> unica{ { 4, 4 } };
    VERIFICAR(caminho_em_l({ 4, 4 }, { 4, 4 }) == unica);
}
