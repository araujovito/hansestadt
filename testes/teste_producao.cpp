#include "cidade.hpp"
#include "conteudo_de_teste.hpp"
#include "eventos.hpp"
#include "producao.hpp"
#include "populacao.hpp"
#include "verificar.hpp"

namespace {

// Dias curtos deixam os testes rápidos sem mudar a lógica.
constexpr int TICKS_POR_DIA = 10;

void rodar(Cidade& c, int ticks, int mes) {
    for (int i = 0; i < ticks; ++i) avancar_producao(c, mes, TICKS_POR_DIA);
}

}  // namespace

TESTE("produção: consome no início e entrega ao fim do ciclo") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    c.adicionar(s, item("tora"), 2);

    rodar(c, TICKS_POR_DIA - 1, 3);
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 0);
    VERIFICAR(c.edificio(s).estoque[item("tabua")] == 0);
    VERIFICAR(c.edificio(s).receita_ativa == 0);

    rodar(c, 1, 3);
    VERIFICAR(c.edificio(s).estoque[item("tabua")] == 1);
    VERIFICAR(c.edificio(s).receita_ativa == -1);
}

TESTE("produção: sem entradas não começa") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    c.adicionar(s, item("tora"), 1);  // precisa de 2
    rodar(c, TICKS_POR_DIA * 3, 3);
    VERIFICAR(c.edificio(s).receita_ativa == -1);
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 1);
}

TESTE("produção: saída cheia não começa nem consome") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    c.adicionar(s, item("tabua"), 4);
    c.adicionar(s, item("tora"), 2);
    rodar(c, TICKS_POR_DIA * 3, 3);
    VERIFICAR(c.edificio(s).estoque[item("tora")] == 2);
}

TESTE("produção: fora da estação não produz, em julho sim") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    rodar(c, TICKS_POR_DIA * 5, 3);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
    rodar(c, TICKS_POR_DIA * 3, 7);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 3);
}

TESTE("produção: fator 0,5 dobra a duração do ciclo") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    rodar(c, TICKS_POR_DIA * 2 - 1, 8);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
    rodar(c, 1, 8);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 1);
}

TESTE("produção: evento reduz o ritmo durante o intervalo histórico") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    EventoHistorico evento;
    evento.id = "chuvas";
    evento.inicio = { 1150, 7, 1 };
    evento.fim = { 1150, 7, 31 };
    evento.producao_por_edificio.emplace_back(tipo("colheita"), 0.5f);
    const Eventos eventos({ evento });

    for (int i = 0; i < TICKS_POR_DIA; ++i)
        avancar_producao(c, 7, TICKS_POR_DIA, nullptr, &eventos, { 1150, 7, 15 });
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
    for (int i = 0; i < TICKS_POR_DIA; ++i)
        avancar_producao(c, 7, TICKS_POR_DIA, nullptr, &eventos, { 1150, 7, 15 });
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 1);
}

TESTE("produção: usa a receita alternativa quando a primeira não tem entrada") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("forno"), { 1, 1 });
    c.adicionar(f, item("tabua"), 1);
    rodar(c, 1, 3);
    VERIFICAR(c.edificio(f).receita_ativa == 1);
    VERIFICAR(c.edificio(f).estoque[item("tabua")] == 0);
}

TESTE("produção: edifício demolido para de produzir") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 1, 1 });
    c.demolir({ 1, 1 });
    rodar(c, TICKS_POR_DIA * 3, 7);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 0);
}

TESTE("produção: extrator consome o recurso do terreno") {
    Mapa mapa(8, 8);
    mapa.definir_terreno(4, 4, Terreno::Floresta);
    Cidade c(std::move(mapa), conteudo_de_teste());
    const IdEdificio extrator = *c.construir_edificio(tipo("perto_mata"), { 2, 2 });
    // O edifício de teste não tem receita: a operação de extração ainda é
    // testada diretamente, separada do ritmo das receitas.
    VERIFICAR(c.tem_recurso_proximo(extrator));
    VERIFICAR(c.extrair_recurso_proximo(extrator));
    VERIFICAR(c.mapa().terreno(4, 4) == Terreno::Campo);
    VERIFICAR(!c.tem_recurso_proximo(extrator));
}

TESTE("produção: lenhador transforma uma floresta em tora e campo") {
    static const Conteudo real = *carregar_conteudo("dados").conteudo;
    Mapa mapa(10, 10);
    mapa.definir_terreno(4, 2, Terreno::Floresta);
    Cidade c(std::move(mapa), real);
    const IdEdificio armazem = *c.construir_edificio(*real.buscar_edificio("armazem"), { 6, 5 });
    c.adicionar(armazem, *real.buscar_item("tora"), 4);
    const IdEdificio lenhador = *c.construir_edificio(*real.buscar_edificio("lenhador"), { 2, 2 });
    for (int i = 0; i < 4 * TICKS_POR_DIA; ++i) avancar_producao(c, 3, TICKS_POR_DIA);
    VERIFICAR(c.edificio(lenhador).estoque[*real.buscar_item("tora")] == 1);
    VERIFICAR(c.mapa().terreno(4, 2) == Terreno::Campo);
}

TESTE("produção: só avança com a equipe completa") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio casa = *c.construir_edificio(tipo("casa"), { 1, 1 });
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 4, 1 });
    Populacao p(1);

    avancar_producao(c, 7, TICKS_POR_DIA, &p);
    VERIFICAR(c.edificio(f).receita_ativa == -1);
    VERIFICAR(situacao(c, f, 7, &p) == Situacao::SemTrabalhadores);

    p.alojar(c);
    p.empregar(c);
    VERIFICAR(p.moradores_na_casa(casa) == 1);
    for (int i = 0; i < TICKS_POR_DIA; ++i) avancar_producao(c, 7, TICKS_POR_DIA, &p);
    VERIFICAR(c.edificio(f).estoque[item("pao")] == 1);
}

TESTE("situação: explica por que o edifício está parado") {
    Cidade c(Mapa(8, 8), conteudo_de_teste());
    const IdEdificio s = *c.construir_edificio(tipo("serraria"), { 1, 1 });
    const IdEdificio f = *c.construir_edificio(tipo("colheita"), { 3, 3 });
    const IdEdificio d = *c.construir_edificio(tipo("deposito"), { 5, 5 });

    VERIFICAR(situacao(c, d, 3) == Situacao::SemReceita);
    VERIFICAR(situacao(c, s, 3) == Situacao::FaltaEntrada);
    VERIFICAR(situacao(c, f, 3) == Situacao::ForaDeEstacao);

    c.adicionar(s, item("tabua"), 4);
    VERIFICAR(situacao(c, s, 3) == Situacao::SaidaCheia);

    rodar(c, 1, 7);
    VERIFICAR(situacao(c, f, 7) == Situacao::Produzindo);
    VERIFICAR(situacao(c, f, 9) == Situacao::Pausado);
}
