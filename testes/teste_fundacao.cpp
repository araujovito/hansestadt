#include "fundacao.hpp"
#include "conteudo_de_teste.hpp"
#include "verificar.hpp"

TESTE("fundação: o primeiro depósito recebe provisões uma única vez") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    Fundacao f(conteudo_de_teste(), 6);
    VERIFICAR(!f.concluir_se_possivel(c));

    const IdEdificio deposito = *c.construir_edificio(tipo("deposito"), { 1, 1 });
    VERIFICAR(f.concluir_se_possivel(c));
    VERIFICAR(f.concluida());
    VERIFICAR(f.armazem() == deposito);
    VERIFICAR(c.edificio(deposito).estoque[item("pao")] == 6);
    VERIFICAR(!f.concluir_se_possivel(c));
    VERIFICAR(c.edificio(deposito).estoque[item("pao")] == 6);
}
