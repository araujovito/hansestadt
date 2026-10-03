#include <filesystem>
#include <fstream>
#include <iterator>

#include "cidade.hpp"
#include "conteudo.hpp"
#include "melhorias.hpp"
#include "populacao.hpp"
#include "producao.hpp"
#include "salvamento.hpp"
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

TESTE("técnicas: produção fica mais rápida com a técnica pesquisada") {
    const Conteudo& c = conteudo();
    const IdItem pao = *c.buscar_item("pao");
    // Sem população, todo edifício conta como cheio de trabalhadores.
    const auto ticks_ate_o_primeiro_pao = [&](bool com_forno) {
        Cidade cidade(Mapa(10, 10), c);
        const IdEdificio deposito = *cidade.construir_edificio(*c.buscar_edificio("deposito"), { 1, 1 });
        cidade.adicionar(deposito, *c.buscar_item("tora"), 10);
        if (com_forno) cidade.pesquisar_tecnica(*c.buscar_tecnica("forno"), Fase::Vila);
        const IdEdificio padaria = *cidade.construir_edificio(*c.buscar_edificio("padaria"), { 4, 1 });
        int ticks = 0;
        while (cidade.edificio(padaria).estoque[pao] == 0 && ticks < 100) {
            avancar_producao(cidade, 1, 10);
            ++ticks;
        }
        return ticks;
    };
    // Ciclo de 4 dias × 10 ticks: 40 ticks; a +75%, 4000/175 arredonda para 23.
    VERIFICAR(ticks_ate_o_primeiro_pao(false) == 40);
    VERIFICAR(ticks_ate_o_primeiro_pao(true) == 23);
}

TESTE("técnicas: equipe de geração mais nova acelera o ciclo junto com a técnica") {
    const Conteudo& c = conteudo();
    const IdItem pao = *c.buscar_item("pao");
    Cidade cidade(Mapa(10, 10), c);
    const IdEdificio deposito = *cidade.construir_edificio(*c.buscar_edificio("deposito"), { 1, 1 });
    cidade.adicionar(deposito, *c.buscar_item("tora"), 10);
    cidade.pesquisar_tecnica(*c.buscar_tecnica("forno"), Fase::Vila);
    const IdEdificio padaria = *cidade.construir_edificio(*c.buscar_edificio("padaria"), { 4, 1 });
    // O padeiro mora no depósito só para ter casa: o conteúdo daqui não tem
    // casas, e empregar exige um endereço.
    Morador neto;
    neto.geracao = 5;
    neto.casa = deposito;
    Populacao p = Populacao::restaurar({ neto }, 0, 0);
    p.empregar(cidade);
    VERIFICAR(ritmo_da_equipe(cidade, padaria, &p) == 1250);
    int ticks = 0;
    while (cidade.edificio(padaria).estoque[pao] == 0 && ticks < 100) {
        avancar_producao(cidade, 1, 10, &p);
        ++ticks;
    }
    // 40000 / (1750 × 1,25 = 2187) arredonda para 19 ticks.
    VERIFICAR(ticks == 19);
}

TESTE("técnicas: salvamento guarda técnicas, gerações e a lei, e a versão 5 migra") {
    const Conteudo& c = conteudo();
    Cidade cidade(Mapa(10, 10), c);
    const IdEdificio deposito = *cidade.construir_edificio(*c.buscar_edificio("deposito"), { 1, 1 });
    cidade.adicionar(deposito, *c.buscar_item("tora"), 10);
    cidade.pesquisar_tecnica(*c.buscar_tecnica("forno"), Fase::Vila);
    const IdEdificio padaria = *cidade.construir_edificio(*c.buscar_edificio("padaria"), { 4, 1 });
    Simulacao sim(std::move(cidade));
    sim.trabalho_infantil = true;
    Morador neto;
    neto.geracao = 3;
    neto.casa = deposito;
    neto.trabalho = padaria;
    sim.populacao = Populacao::restaurar({ neto }, 0, 0);

    const std::string caminho = "/tmp/hansestadt-teste-v6.hanse";
    std::string erro;
    VERIFICAR(salvar_partida(sim, caminho, erro));
    const ResultadoSalvamento carga = carregar_partida(caminho, c);
    VERIFICAR(carga.simulacao.has_value());
    if (carga.simulacao) {
        VERIFICAR(carga.simulacao->trabalho_infantil);
        VERIFICAR(carga.simulacao->cidade.tecnica_pesquisada(*c.buscar_tecnica("forno")));
        VERIFICAR(carga.simulacao->cidade.produtividade_tecnica(*c.buscar_edificio("padaria")) == 1750);
        VERIFICAR(carga.simulacao->populacao.moradores()[0].geracao == 3);
        VERIFICAR(carga.simulacao->populacao.produtividade_em(padaria) == 1150);
    }

    // A versão 5 é o mesmo arquivo sem o bloco do fim: lei (1 byte),
    // técnicas (8 + 1 por técnica) e gerações (8 + 4 por morador).
    std::ifstream entrada(caminho, std::ios::binary);
    std::string bytes{ std::istreambuf_iterator<char>(entrada), std::istreambuf_iterator<char>() };
    entrada.close();
    bytes.resize(bytes.size() - (1 + 8 + c.tecnicas.size() + 8 + 4));
    bytes[10] = '\5';
    std::ofstream saida(caminho, std::ios::binary | std::ios::trunc);
    saida.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    saida.close();
    const ResultadoSalvamento v5 = carregar_partida(caminho, c);
    VERIFICAR(v5.simulacao.has_value());
    if (v5.simulacao) {
        VERIFICAR(!v5.simulacao->trabalho_infantil);
        VERIFICAR(!v5.simulacao->cidade.tecnica_pesquisada(*c.buscar_tecnica("forno")));
        VERIFICAR(v5.simulacao->populacao.moradores()[0].geracao == 0);
    }
    std::filesystem::remove(caminho);
}

TESTE("técnicas: ficha do painel lista níveis, materiais e a próxima a pesquisar") {
    const Conteudo& c = conteudo();
    Cidade cidade(Mapa(10, 10), c);
    const IdEdificio deposito = *cidade.construir_edificio(*c.buscar_edificio("deposito"), { 1, 1 });
    cidade.adicionar(deposito, *c.buscar_item("tora"), 4);
    const IdTipoEdificio padaria = *c.buscar_edificio("padaria");

    TecnicasDoEdificio t = tecnicas_do_edificio(cidade, padaria, Fase::Vila);
    VERIFICAR(t.tecnicas.size() == 2);
    VERIFICAR(t.proxima == std::optional<std::size_t>(0));
    VERIFICAR(t.ritmo_porcento == 100);
    VERIFICAR(t.tecnicas[0].bonus_porcento == 75);
    VERIFICAR(t.tecnicas[0].materiais.size() == 1 && t.tecnicas[0].materiais[0].tem == 4);
    VERIFICAR(t.tecnicas[0].pode_pesquisar == ResultadoTecnica::Ok);
    VERIFICAR(t.tecnicas[1].pode_pesquisar == ResultadoTecnica::FaseBloqueada);

    cidade.pesquisar_tecnica(t.tecnicas[0].id, Fase::Vila);
    t = tecnicas_do_edificio(cidade, padaria, Fase::Vila);
    VERIFICAR(t.tecnicas[0].pesquisada);
    VERIFICAR(t.proxima == std::optional<std::size_t>(1));
    VERIFICAR(t.ritmo_porcento == 175);
    VERIFICAR(t.tecnicas[1].materiais[0].tem == 2);

    // A forja só tem a segunda técnica.
    VERIFICAR(tecnicas_do_edificio(cidade, *c.buscar_edificio("forja"), Fase::Vila).tecnicas.size() == 1);
    VERIFICAR(tecnicas_do_edificio(cidade, *c.buscar_edificio("deposito"), Fase::Vila).tecnicas.empty());
}
