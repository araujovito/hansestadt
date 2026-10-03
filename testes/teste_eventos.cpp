#include "eventos.hpp"
#include "populacao.hpp"
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
    VERIFICAR((*r.eventos)[0].mortalidade_anual == 0.0f);
    VERIFICAR((*r.eventos)[1].mortalidade_anual == 0.10f);
}

TESTE("eventos: mortalidade anual vira risco diário só no intervalo ativo") {
    EventoHistorico peste;
    peste.id = "peste";
    peste.inicio = {1347, 1, 1};
    peste.fim = {1351, 12, 31};
    peste.mortalidade_anual = 0.10f;
    Eventos eventos({peste});

    VERIFICAR(eventos.probabilidade_morte_diaria({1346, 12, 31}) == 0.0f);
    VERIFICAR(eventos.probabilidade_morte_diaria({1347, 1, 1}) > 0.0f);
    VERIFICAR(eventos.probabilidade_morte_diaria({1351, 12, 31}) > 0.0f);
    VERIFICAR(eventos.probabilidade_morte_diaria({1352, 1, 1}) == 0.0f);
}

TESTE("eventos: mortalidade da peste é determinística e acumulada") {
    const ResultadoCarga carga = carregar_conteudo("dados");
    if (!carga.conteudo) { VERIFICAR(false); return; }
    const ResultadoEventos definidos = carregar_eventos("dados", *carga.conteudo);
    if (!definidos.eventos) { VERIFICAR(false); return; }
    const Eventos eventos(*definidos.eventos);
    const auto simular = [&carga, &eventos]() {
        Cidade cidade(Mapa(2, 2), *carga.conteudo);
        Populacao populacao(1000);
        Data data{1347, 1, 1};
        while (data <= Data{1351, 12, 31}) {
            populacao.avancar_dia(cidade, data, &eventos);
            data = dia_seguinte(data);
        }
        return populacao.mortos_por_eventos();
    };
    const int mortes1 = simular();
    const int mortes2 = simular();
    VERIFICAR(mortes1 == mortes2);
    VERIFICAR(mortes1 >= 300 && mortes1 <= 500);
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
