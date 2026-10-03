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

    return Simulacao(std::move(cidade));
}

void ate_marco(Simulacao& sim) {
    while (sim.data.ano < 1151 || sim.data.mes < 3) sim.passo();
}

}  // namespace

TESTE("marco: a cadeia de comida atravessa o inverno de 1150") {
    Simulacao sim = vila_do_marco(true);
    ate_marco(sim);
    VERIFICAR(sim.populacao.total() >= 6);
    VERIFICAR(sim.populacao.mortos_de_fome() == 0);
}

TESTE("marco: sem fazenda a vila passa fome") {
    Simulacao sim = vila_do_marco(false);
    ate_marco(sim);
    VERIFICAR(sim.populacao.refeicoes_perdidas() > 0);
}
