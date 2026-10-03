#include "cidade.hpp"
#include "conteudo.hpp"
#include "verificar.hpp"

namespace {

constexpr const char* ITENS = R"(
[[item]]
id = "tora"
nome = "tora"
tipo = "insumo"

[[item]]
id = "pao"
nome = "pão"
tipo = "comida"
)";

constexpr const char* EDIFICIOS = R"(
[[edificio]]
id = "deposito"
nome = "depósito"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
estoque = 100
deposito = true

[[edificio]]
id = "padaria"
nome = "padaria"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
trabalhadores = 1
estoque = 100
[[edificio.receita]]
dias = 4
saidas = { pao = 1 }

[[edificio]]
id = "forja"
nome = "forja"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
trabalhadores = 1
estoque = 100
[[edificio.receita]]
dias = 4
saidas = { tora = 1 }

[[tecnica]]
id = "forno"
nome = "forno"
fase = "vila"
edificios = ["padaria"]
custo = { tora = 2 }
bonus = 0.75

[[tecnica]]
id = "guilda"
nome = "guilda"
fase = "cidade"
edificios = ["padaria", "forja"]
requer = "forno"
custo = { tora = 3 }
bonus = 0.75
)";

const Conteudo& conteudo() {
    static const Conteudo c = *carregar_conteudo_de_texto(ITENS, EDIFICIOS).conteudo;
    return c;
}

}  // namespace

TESTE("técnicas: pesquisar consome materiais e respeita fase e requisito") {
    const Conteudo& c = conteudo();
    Cidade cidade(Mapa(10, 10), c);
    const IdEdificio deposito = *cidade.construir_edificio(*c.buscar_edificio("deposito"), { 1, 1 });
    const IdItem tora = *c.buscar_item("tora");
    cidade.adicionar(deposito, tora, 4);
    const IdTecnica forno = *c.buscar_tecnica("forno");
    const IdTecnica guilda = *c.buscar_tecnica("guilda");

    VERIFICAR(cidade.validar_tecnica(guilda, Fase::Cidade) == ResultadoTecnica::FaltaRequisito);
    VERIFICAR(cidade.pesquisar_tecnica(forno, Fase::Vila) == ResultadoTecnica::Ok);
    VERIFICAR(cidade.tecnica_pesquisada(forno));
    VERIFICAR(cidade.edificio(deposito).estoque[tora] == 2);
    VERIFICAR(cidade.validar_tecnica(forno, Fase::Vila) == ResultadoTecnica::JaPesquisada);
    VERIFICAR(cidade.validar_tecnica(guilda, Fase::Vila) == ResultadoTecnica::FaseBloqueada);
    VERIFICAR(cidade.pesquisar_tecnica(guilda, Fase::Cidade) == ResultadoTecnica::SemRecursos);
    VERIFICAR(cidade.edificio(deposito).estoque[tora] == 2);
    VERIFICAR(!cidade.tecnica_pesquisada(guilda));
}

TESTE("técnicas: bônus vale para o tipo inteiro e soma entre níveis") {
    const Conteudo& c = conteudo();
    Cidade cidade(Mapa(10, 10), c);
    const IdEdificio deposito = *cidade.construir_edificio(*c.buscar_edificio("deposito"), { 1, 1 });
    cidade.adicionar(deposito, *c.buscar_item("tora"), 10);
    const IdTipoEdificio padaria = *c.buscar_edificio("padaria");
    const IdTipoEdificio forja = *c.buscar_edificio("forja");

    VERIFICAR(cidade.produtividade_tecnica(padaria) == PRODUTIVIDADE_NORMAL);
    cidade.pesquisar_tecnica(*c.buscar_tecnica("forno"), Fase::Cidade);
    VERIFICAR(cidade.produtividade_tecnica(padaria) == 1750);
    VERIFICAR(cidade.produtividade_tecnica(forja) == PRODUTIVIDADE_NORMAL);
    cidade.pesquisar_tecnica(*c.buscar_tecnica("guilda"), Fase::Cidade);
    VERIFICAR(cidade.produtividade_tecnica(padaria) == 2500);
    VERIFICAR(cidade.produtividade_tecnica(forja) == 1750);
}

TESTE("técnicas: restaurar recalcula a produtividade e completa lista curta") {
    const Conteudo& c = conteudo();
    Cidade cidade(Mapa(10, 10), c);
    cidade.restaurar_tecnicas({ 1 });
    VERIFICAR(cidade.tecnicas().size() == c.tecnicas.size());
    VERIFICAR(cidade.tecnica_pesquisada(*c.buscar_tecnica("forno")));
    VERIFICAR(!cidade.tecnica_pesquisada(*c.buscar_tecnica("guilda")));
    VERIFICAR(cidade.produtividade_tecnica(*c.buscar_edificio("padaria")) == 1750);
}
