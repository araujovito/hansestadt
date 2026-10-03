#include "eventos.hpp"
#include "verificar.hpp"

TESTE("eventos: conteúdo histórico carrega do TOML") {
    const ResultadoCarga carga = carregar_conteudo("dados");
    VERIFICAR(carga.conteudo.has_value());
    if (!carga.conteudo) return;
    const ResultadoEventos r = carregar_eventos("dados", *carga.conteudo);
    VERIFICAR(r.eventos.has_value());
    if (!r.eventos) return;
    VERIFICAR(r.eventos->size() == 2);
    VERIFICAR((*r.eventos)[0].id == "grande_fome");
    VERIFICAR((*r.eventos)[1].id == "peste_negra");
}

TESTE("eventos: intervalo inclusivo e multiplicadores por edifício") {
    const ResultadoCarga carga = carregar_conteudo("dados");
    if (!carga.conteudo) { VERIFICAR(false); return; }
    const auto fazenda = carga.conteudo->buscar_edificio("fazenda");
    const auto pescador = carga.conteudo->buscar_edificio("pescador");
    VERIFICAR(fazenda.has_value() && pescador.has_value());
    if (!fazenda || !pescador) return;
    EventoHistorico fome;
    fome.id = "grande_fome";
    fome.inicio = { 1315, 1, 1 };
    fome.fim = { 1317, 12, 31 };
    fome.producao_por_edificio.emplace_back(*fazenda, 0.25f);
    Eventos eventos({ fome });

    VERIFICAR(eventos.ativos({ 1314, 12, 31 }).empty());
    VERIFICAR(eventos.ativos({ 1315, 1, 1 }).size() == 1);
    VERIFICAR(eventos.ativos({ 1317, 12, 31 }).size() == 1);
    VERIFICAR(eventos.ativos({ 1318, 1, 1 }).empty());
    VERIFICAR(eventos.ocorridos({ 1316, 1, 1 }).size() == 1);
    VERIFICAR(eventos.multiplicador_producao({ 1316, 6, 1 }, *fazenda) == 0.25f);
    VERIFICAR(eventos.multiplicador_producao({ 1316, 6, 1 }, *pescador) == 1.0f);
}
