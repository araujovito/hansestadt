#include "historico.hpp"
#include "verificar.hpp"

namespace {

struct Dados {
    Conteudo conteudo;
    Eventos eventos;
};

// Os eventos de verdade do jogo, para os textos baterem com o que o jogador vê.
std::optional<Dados> dados_do_jogo() {
    const ResultadoCarga carga = carregar_conteudo("dados");
    if (!carga.conteudo) return std::nullopt;
    const ResultadoEventos r = carregar_eventos("dados", *carga.conteudo);
    if (!r.eventos) return std::nullopt;
    return Dados{ *carga.conteudo, Eventos(*r.eventos) };
}

}  // namespace

TESTE("histórico: antes do primeiro evento fica vazio, sem anunciar o futuro") {
    const auto d = dados_do_jogo();
    VERIFICAR(d.has_value());
    if (!d) return;
    VERIFICAR(montar_historico(d->eventos, { 1314, 12, 31 }, d->conteudo).empty());
}

TESTE("histórico: evento em andamento mostra período, dias restantes e efeitos") {
    const auto d = dados_do_jogo();
    if (!d) { VERIFICAR(false); return; }
    const auto h = montar_historico(d->eventos, { 1317, 12, 30 }, d->conteudo);
    VERIFICAR(h.size() == 1);
    if (h.size() != 1) return;
    VERIFICAR(h[0].nome == "Grande Fome");
    VERIFICAR(h[0].em_andamento);
    VERIFICAR(h[0].dias_restantes == 2);  // 30 e 31 de dezembro
    VERIFICAR(h[0].periodo == "1 de janeiro de 1315 a 31 de dezembro de 1317");
    VERIFICAR(h[0].efeitos.size() == 2);
    if (h[0].efeitos.size() != 2) return;
    VERIFICAR(h[0].efeitos[0] == "fazenda e fazenda de cevada a 25%");
    VERIFICAR(h[0].efeitos[1] == "pescador e pesca de arenque a 75%");
}

TESTE("histórico: em andamento primeiro, depois encerrados do mais recente ao mais antigo") {
    const auto d = dados_do_jogo();
    if (!d) { VERIFICAR(false); return; }
    const auto h = montar_historico(d->eventos, { 1348, 6, 1 }, d->conteudo);
    VERIFICAR(h.size() == 3);
    if (h.size() != 3) return;
    VERIFICAR(h[0].nome == "Peste Negra" && h[0].em_andamento);
    VERIFICAR(h[1].nome == "Inverno rigoroso do Báltico" && !h[1].em_andamento && h[1].dias_restantes == 0);
    VERIFICAR(h[2].nome == "Grande Fome" && !h[2].em_andamento);
    VERIFICAR(h[0].efeitos.size() == 2);
    if (h[0].efeitos.size() != 2) return;
    VERIFICAR(h[0].efeitos[0] == "produção geral a 65%");
    VERIFICAR(h[0].efeitos[1] == "10% de mortes ao ano");
}

TESTE("histórico: no último dia o evento ainda está em andamento") {
    const auto d = dados_do_jogo();
    if (!d) { VERIFICAR(false); return; }
    const auto h = montar_historico(d->eventos, { 1323, 3, 6 }, d->conteudo);
    VERIFICAR(!h.empty() && h[0].nome == "Inverno rigoroso do Báltico" && h[0].dias_restantes == 1);
    const auto depois = montar_historico(d->eventos, { 1323, 3, 7 }, d->conteudo);
    VERIFICAR(!depois.empty() && !depois[0].em_andamento);
}

TESTE("histórico: eventos fora de ordem saem pela data de início") {
    const auto d = dados_do_jogo();
    if (!d) { VERIFICAR(false); return; }
    EventoHistorico antigo;
    antigo.nome = "antigo";
    antigo.inicio = { 1200, 1, 1 };
    antigo.fim = { 1200, 2, 1 };
    EventoHistorico novo = antigo;
    novo.nome = "novo";
    novo.inicio = { 1210, 1, 1 };
    novo.fim = { 1210, 2, 1 };
    const auto h = montar_historico(Eventos({ novo, antigo }), { 1300, 1, 1 }, d->conteudo);
    VERIFICAR(h.size() == 2 && h[0].nome == "novo" && h[1].nome == "antigo");
    VERIFICAR(h.size() == 2 && h[0].efeitos.empty());
}

TESTE("histórico: começo de evento é visto mesmo pulando vários dias") {
    const auto d = dados_do_jogo();
    if (!d) { VERIFICAR(false); return; }
    VERIFICAR(comecaram_entre(d->eventos, { 1314, 12, 31 }, { 1315, 1, 1 }).size() == 1);
    VERIFICAR(comecaram_entre(d->eventos, { 1314, 12, 20 }, { 1315, 1, 5 }).size() == 1);
    VERIFICAR(comecaram_entre(d->eventos, { 1315, 1, 1 }, { 1315, 1, 2 }).empty());  // já tinha começado
    VERIFICAR(comecaram_entre(d->eventos, { 1315, 1, 1 }, { 1315, 1, 1 }).empty());  // mesmo dia
}

TESTE("histórico: data por extenso") {
    VERIFICAR(escrever_data({ 1322, 11, 30 }) == "30 de novembro de 1322");
}
