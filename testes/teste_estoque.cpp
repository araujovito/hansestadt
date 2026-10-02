#include "cidade.hpp"
#include "conteudo_de_teste.hpp"
#include "verificar.hpp"

namespace {

Cidade cidade_vazia() {
    return Cidade(Mapa(8, 8), conteudo_de_teste());
}

}  // namespace

TESTE("estoque: edifício novo começa vazio") {
    Cidade c = cidade_vazia();
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    VERIFICAR(c.edificio(s).estoque.size() == conteudo_de_teste().itens.size());
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 0);
}

TESTE("estoque: produtor guarda só os itens das receitas, até o limite de cada") {
    Cidade c = cidade_vazia();
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    VERIFICAR(c.espaco_para(s, item("tora")) == 4);
    VERIFICAR(c.espaco_para(s, item("pao")) == 0);
    VERIFICAR(c.adicionar(s, item("tora"), 3));
    VERIFICAR(!c.adicionar(s, item("tora"), 2));  // passaria de 4
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 3);
    VERIFICAR(c.adicionar(s, item("tabua"), 4));  // limite é por item
    VERIFICAR(!c.adicionar(s, item("pao"), 1));
}

TESTE("estoque: depósito aceita qualquer item, com limite total") {
    Cidade c = cidade_vazia();
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    VERIFICAR(c.adicionar(d, item("tora"), 6));
    VERIFICAR(c.adicionar(d, item("pao"), 4));
    VERIFICAR(c.espaco_para(d, item("tabua")) == 0);
    VERIFICAR(!c.adicionar(d, item("tabua"), 1));
}

TESTE("estoque: retirar não deixa ficar negativo") {
    Cidade c = cidade_vazia();
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    c.adicionar(d, item("tora"), 2);
    VERIFICAR(!c.retirar(d, item("tora"), 3));
    VERIFICAR(c.retirar(d, item("tora"), 2));
    VERIFICAR(c.edificio(d).estoque[item("tora")] == 0);
}

TESTE("estoque: edifício demolido não recebe nada") {
    Cidade c = cidade_vazia();
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    c.demolir({ 1, 1 });
    VERIFICAR(c.espaco_para(d, item("tora")) == 0);
    VERIFICAR(!c.adicionar(d, item("tora"), 1));
}
