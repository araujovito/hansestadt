#include "fundacao.hpp"
#include "simulacao.hpp"
#include "verificar.hpp"

namespace {

void estrada(Cidade& cidade, Celula a, Celula b) {
    for (const Celula c : caminho_em_l(a, b)) cidade.construir_estrada(c);
}

Simulacao vila_do_marco(bool com_fazenda) {
    static const Conteudo conteudo = *carregar_conteudo("dados").conteudo;
    Mapa mapa(32, 18);
    mapa.definir_terreno(12, 5, Terreno::Agua);
    mapa.definir_terreno(16, 5, Terreno::Agua);
    mapa.definir_terreno(23, 6, Terreno::Floresta);
    Cidade cidade(std::move(mapa), conteudo);

    cidade.construir_edificio(*conteudo.buscar_edificio("armazem"), { 1, 1 });
    Fundacao fundacao(conteudo);
    fundacao.concluir_se_possivel(cidade);
    cidade.construir_edificio(*conteudo.buscar_edificio("casa"), { 5, 1 });
    cidade.construir_edificio(*conteudo.buscar_edificio("casa"), { 8, 1 });
    if (com_fazenda) cidade.construir_edificio(*conteudo.buscar_edificio("fazenda"), { 5, 6 });
    cidade.construir_edificio(*conteudo.buscar_edificio("moinho"), { 12, 6 });
    cidade.construir_edificio(*conteudo.buscar_edificio("padaria"), { 9, 6 });
    cidade.construir_edificio(*conteudo.buscar_edificio("pescador"), { 16, 6 });
    cidade.construir_edificio(*conteudo.buscar_edificio("pocilga"), { 20, 6 });

    // Tronco na linha 10; cada porta ganha um ramal curto.
    estrada(cidade, { 2, 4 }, { 2, 10 });
    estrada(cidade, { 2, 10 }, { 20, 10 });
    estrada(cidade, { 5, 3 }, { 4, 10 });
    estrada(cidade, { 8, 3 }, { 11, 10 });
    if (com_fazenda) estrada(cidade, { 6, 10 }, { 6, 10 });
    estrada(cidade, { 9, 8 }, { 9, 10 });
    estrada(cidade, { 12, 8 }, { 12, 10 });
    estrada(cidade, { 16, 8 }, { 16, 10 });
    estrada(cidade, { 20, 8 }, { 20, 10 });

    for (int id = 1; id <= 2; ++id) {
        const auto r = buscar_rota(cidade, cidade.porta(0), cidade.porta(id));
        int custo = 0;
        if (r) for (std::size_t i = 1; i < r->size(); ++i) custo += cidade.custo_movimento((*r)[i]);
        std::fprintf(stderr, "DEBUG rota casa%d passos=%zu custo=%d\n", id, r ? r->size() : 0, custo);
    }

    return Simulacao(std::move(cidade));
}

void ate_marco(Simulacao& sim) {
    int ultimas = 0;
    while (sim.data.ano < 1151 || sim.data.mes < 3) {
        sim.passo();
        if (sim.populacao.refeicoes_perdidas() != ultimas && sim.tick % Simulacao::TICKS_POR_DIA == 0) {
            ultimas = sim.populacao.refeicoes_perdidas();
            std::fprintf(stderr, "DEBUG falta %d-%02d-%02d pop=%d perd=%d entregas=%llu casas=%d/%d carr=%zu\n",
                         sim.data.ano, sim.data.mes, sim.data.dia, sim.populacao.total(), ultimas,
                         static_cast<unsigned long long>(sim.transporte.entregas()),
                         sim.cidade.edificio(1).estoque[3] + sim.cidade.edificio(1).estoque[4],
                         sim.cidade.edificio(2).estoque[3] + sim.cidade.edificio(2).estoque[4],
                         sim.transporte.carregadores().size());
        }
    }
}

}  // namespace

TESTE("marco: a cadeia de comida atravessa o inverno de 1150") {
    Simulacao sim = vila_do_marco(true);
    ate_marco(sim);
    std::fprintf(stderr, "DEBUG marco pop=%d fome=%d perdidas=%d entregas=%llu\n",
                 sim.populacao.total(), sim.populacao.mortos_de_fome(), sim.populacao.refeicoes_perdidas(),
                 static_cast<unsigned long long>(sim.transporte.entregas()));
    for (IdEdificio id = 0; id < sim.cidade.edificios().size(); ++id) {
        const auto& e = sim.cidade.edificio(id);
        std::fprintf(stderr, "DEBUG id=%u %s trab=%d carr=%d est=", id,
                     sim.cidade.definicao(id).id.c_str(), sim.populacao.trabalhadores_em(id),
                     sim.populacao.carregadores_em(id));
        for (int q : e.estoque) std::fprintf(stderr, "%d,", q);
        std::fprintf(stderr, "\n");
    }
    VERIFICAR(sim.populacao.total() >= 6);
    VERIFICAR(sim.populacao.mortos_de_fome() == 0);
}

TESTE("marco: sem fazenda a vila passa fome") {
    Simulacao sim = vila_do_marco(false);
    ate_marco(sim);
    VERIFICAR(sim.populacao.refeicoes_perdidas() > 0);
}
