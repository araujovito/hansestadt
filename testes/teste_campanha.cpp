#include "campanha.hpp"
#include "verificar.hpp"

TESTE("campanha: fases usam os anos históricos definidos") {
    VERIFICAR(ano_de_abertura(Fase::Vila) == 1150);
    VERIFICAR(ano_de_abertura(Fase::Mercado) == 1200);
    VERIFICAR(ano_de_abertura(Fase::Cidade) == 1250);
    VERIFICAR(ano_de_abertura(Fase::Hansa) == 1300);
    VERIFICAR(ano_de_abertura(Fase::Auge) == 1370);
}

TESTE("campanha: data não basta sem todas as instituições no máximo") {
    const std::vector<EstadoInstituicao> instituicoes = {
        { "igreja", 2, 2 }, { "governo", 1, 2 }, { "mercado", 1, 1 }, { "porto", 1, 1 }
    };
    auto r = tentar_avancar_fase(Fase::Vila, { 1200, 1, 1 }, instituicoes);
    VERIFICAR(!r.avancou && r.fase == Fase::Vila);

    r = tentar_avancar_fase(Fase::Vila, { 1199, 12, 31 }, { { "igreja", 1, 1 } });
    VERIFICAR(!r.avancou);
}

TESTE("campanha: avança uma fase quando chegou o ano e todas as instituições estão no máximo") {
    const std::vector<EstadoInstituicao> instituicoes = {
        { "igreja", 2, 2 }, { "governo", 2, 2 }, { "mercado", 1, 1 }, { "porto", 3, 3 }
    };
    const auto r = tentar_avancar_fase(Fase::Vila, { 1200, 1, 1 }, instituicoes);
    VERIFICAR(r.avancou && r.fase == Fase::Mercado);
    const auto final = tentar_avancar_fase(Fase::Auge, { 1400, 1, 1 }, instituicoes);
    VERIFICAR(!final.avancou && final.fase == Fase::Auge);
}
