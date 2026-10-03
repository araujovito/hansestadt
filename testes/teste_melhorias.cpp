#include "campanha.hpp"
#include "melhorias.hpp"
#include "verificar.hpp"

namespace {

// O conteúdo de verdade, para as fases e custos baterem com o jogo.
std::optional<Conteudo> conteudo_do_jogo() {
    ResultadoCarga r = carregar_conteudo("dados");
    if (!r.conteudo) return std::nullopt;
    return std::move(*r.conteudo);
}

IdTipoEdificio tipo_de(const Conteudo& c, const char* id) { return *c.buscar_edificio(id); }

}  // namespace

TESTE("melhorias: instituição não construída aparece sem nível e sem materiais") {
    const auto conteudo = conteudo_do_jogo();
    VERIFICAR(conteudo.has_value());
    if (!conteudo) return;
    const Cidade cidade(Mapa(16, 16), *conteudo);
    const FichaDaInstituicao f = ficha_da_instituicao(cidade, tipo_de(*conteudo, "igreja"), Fase::Vila);
    VERIFICAR(f.nome == "igreja");
    VERIFICAR(!f.edificio && f.nivel == 0);
    VERIFICAR(f.nivel_da_fase == 1 && f.nivel_final == 5);
    VERIFICAR(!f.fase_da_proxima && f.materiais.empty());
    VERIFICAR(f.pode_melhorar == ResultadoMelhoria::EdificioInativo);
    VERIFICAR(!f.pronta_para_a_fase());
}

TESTE("melhorias: ficha mostra a próxima melhoria, a fase dela e o que a vila tem") {
    const auto conteudo = conteudo_do_jogo();
    if (!conteudo) { VERIFICAR(false); return; }
    Cidade cidade(Mapa(16, 16), *conteudo);
    const IdEdificio armazem = *cidade.construir_edificio(tipo_de(*conteudo, "armazem"), { 1, 1 });
    cidade.adicionar(armazem, *conteudo->buscar_item("tora"), 13);
    const IdEdificio igreja = *cidade.construir_edificio(tipo_de(*conteudo, "igreja"), { 6, 1 });
    // A construção gastou 8 toras: sobram 5 para uma melhoria que pede 8.

    FichaDaInstituicao f = ficha_da_instituicao(cidade, tipo_de(*conteudo, "igreja"), Fase::Vila);
    VERIFICAR(f.edificio == std::optional<IdEdificio>(igreja) && f.nivel == 1);
    VERIFICAR(f.pronta_para_a_fase());
    VERIFICAR(f.fase_da_proxima == std::optional<Fase>(Fase::Mercado));
    VERIFICAR(f.materiais.size() == 1);
    if (f.materiais.size() != 1) return;
    VERIFICAR(f.materiais[0].nome == "tora" && f.materiais[0].precisa == 8);
    VERIFICAR(f.materiais[0].tem == 5);
    VERIFICAR(f.pode_melhorar == ResultadoMelhoria::FaseBloqueada);

    // Na fase Mercado a igreja deixa de estar pronta: o nível 2 foi liberado.
    f = ficha_da_instituicao(cidade, tipo_de(*conteudo, "igreja"), Fase::Mercado);
    VERIFICAR(f.nivel_da_fase == 2 && !f.pronta_para_a_fase());
    VERIFICAR(f.pode_melhorar == cidade.validar_melhoria(igreja, Fase::Mercado));
}

TESTE("melhorias: no último nível não há próxima melhoria") {
    const auto conteudo = conteudo_do_jogo();
    if (!conteudo) { VERIFICAR(false); return; }
    Cidade cidade(Mapa(16, 16), *conteudo);
    const IdEdificio armazem = *cidade.construir_edificio(tipo_de(*conteudo, "armazem"), { 1, 1 });
    cidade.adicionar(armazem, *conteudo->buscar_item("tora"), 50);
    cidade.adicionar(armazem, *conteudo->buscar_item("tijolo"), 50);
    const IdEdificio igreja = *cidade.construir_edificio(tipo_de(*conteudo, "igreja"), { 6, 1 });
    for (int i = 0; i < 4; ++i) VERIFICAR(cidade.melhorar_edificio(igreja, Fase::Auge) == ResultadoMelhoria::Ok);

    const FichaDaInstituicao f = ficha_da_instituicao(cidade, tipo_de(*conteudo, "igreja"), Fase::Auge);
    VERIFICAR(f.nivel == 5 && f.nivel == f.nivel_final);
    VERIFICAR(!f.fase_da_proxima && f.materiais.empty());
    VERIFICAR(f.pode_melhorar == ResultadoMelhoria::NivelMaximo);
}

TESTE("melhorias: rumo à próxima fase concorda com a regra da campanha") {
    const auto conteudo = conteudo_do_jogo();
    if (!conteudo) { VERIFICAR(false); return; }
    Cidade cidade(Mapa(24, 16), *conteudo);
    const IdEdificio armazem = *cidade.construir_edificio(tipo_de(*conteudo, "armazem"), { 1, 1 });
    cidade.adicionar(armazem, *conteudo->buscar_item("tora"), 100);

    auto confere = [&](Data hoje) {
        const auto rumo = rumo_a_proxima_fase(cidade, Fase::Vila, hoje);
        VERIFICAR(rumo.has_value());
        if (!rumo) return;
        const bool avanca = tentar_avancar_fase(Fase::Vila, hoje, estados_das_instituicoes(cidade, Fase::Vila)).avancou;
        VERIFICAR((rumo->ano_chegou && rumo->instituicoes_prontas()) == avanca);
    };

    const auto rumo = rumo_a_proxima_fase(cidade, Fase::Vila, { 1150, 3, 1 });
    VERIFICAR(rumo && rumo->fase == Fase::Mercado && rumo->ano == 1200 && !rumo->ano_chegou);
    // Na vila, só igreja e conselho; o mercado coberto é da fase seguinte.
    VERIFICAR(rumo && rumo->instituicoes.size() == 2 && !rumo->instituicoes_prontas());
    confere({ 1200, 1, 1 });

    cidade.construir_edificio(tipo_de(*conteudo, "igreja"), { 6, 1 });
    confere({ 1200, 1, 1 });
    cidade.construir_edificio(tipo_de(*conteudo, "governo"), { 10, 1 });
    confere({ 1199, 12, 31 });
    confere({ 1200, 1, 1 });
    const auto pronto = rumo_a_proxima_fase(cidade, Fase::Vila, { 1200, 1, 1 });
    VERIFICAR(pronto && pronto->ano_chegou && pronto->instituicoes_prontas());
}

TESTE("melhorias: na última fase não há próxima") {
    const auto conteudo = conteudo_do_jogo();
    if (!conteudo) { VERIFICAR(false); return; }
    const Cidade cidade(Mapa(8, 8), *conteudo);
    VERIFICAR(!rumo_a_proxima_fase(cidade, Fase::Auge, { 1370, 1, 1 }).has_value());
}
