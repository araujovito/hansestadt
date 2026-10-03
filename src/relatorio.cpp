#include "relatorio.hpp"

#include <algorithm>

namespace {

RelatorioEmprego montar_emprego(const Simulacao& sim) {
    RelatorioEmprego r;
    for (const Morador& m : sim.populacao.moradores()) {
        if (!m.vivo) continue;
        if (m.adulto()) ++r.adultos;
        else ++r.criancas;
    }
    r.sem_casa = sim.populacao.sem_casa();
    r.desempregados = sim.populacao.desempregados();

    const Cidade& cidade = sim.cidade;
    const Conteudo& conteudo = cidade.conteudo();
    // Uma linha por tipo de edifício, mais a dos carregadores na frente.
    LinhaDeEmprego carregadores{ "carregadores", 0, 0, 0 };
    std::vector<LinhaDeEmprego> por_tipo(conteudo.edificios.size());
    for (IdTipoEdificio t = 0; t < conteudo.edificios.size(); ++t) por_tipo[t].nome = conteudo.edificios[t].nome;

    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo) continue;
        const DefinicaoEdificio& def = cidade.definicao(id);
        // As vagas de carregador e de produção do mesmo edifício entram na
        // mesma contagem da população; os carregadores são preenchidos
        // primeiro (ver Populacao::empregar).
        const int ocupadas = sim.populacao.trabalhadores_em(id);
        const int no_transporte = std::min(ocupadas, def.carregadores);
        if (def.carregadores > 0) {
            ++carregadores.edificios;
            carregadores.vagas += def.carregadores;
            carregadores.ocupadas += no_transporte;
        }
        if (def.trabalhadores > 0) {
            LinhaDeEmprego& l = por_tipo[cidade.edificio(id).tipo];
            ++l.edificios;
            l.vagas += def.trabalhadores;
            l.ocupadas += ocupadas - no_transporte;
        }
    }
    if (carregadores.vagas > 0) r.linhas.push_back(carregadores);
    for (LinhaDeEmprego& l : por_tipo)
        if (l.vagas > 0) r.linhas.push_back(std::move(l));
    for (const LinhaDeEmprego& l : r.linhas) {
        r.vagas += l.vagas;
        r.ocupadas += l.ocupadas;
    }
    return r;
}

RelatorioComida montar_comida(const Simulacao& sim) {
    RelatorioComida r;
    const Conteudo& conteudo = sim.cidade.conteudo();
    for (IdItem i = 0; i < conteudo.itens.size(); ++i) {
        if (conteudo.itens[i].tipo != TipoItem::Comida) continue;
        r.itens.push_back({ conteudo.itens[i].nome, sim.cidade.estoque_total(i) });
        r.estoque += r.itens.back().estoque;
    }
    r.moradores = sim.populacao.total();
    r.dias = r.moradores > 0 ? r.estoque / r.moradores : 0;
    r.refeicoes_perdidas = sim.populacao.refeicoes_perdidas();
    r.mortos_de_fome = sim.populacao.mortos_de_fome();
    return r;
}

std::vector<LinhaDeProducao> montar_producao(const Simulacao& sim) {
    const Cidade& cidade = sim.cidade;
    const Conteudo& conteudo = cidade.conteudo();
    std::vector<LinhaDeProducao> por_tipo(conteudo.edificios.size());
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo || cidade.definicao(id).receitas.empty()) continue;
        LinhaDeProducao& l = por_tipo[cidade.edificio(id).tipo];
        ++l.edificios;
        if (!cidade.tem_acesso(id)) {
            ++l.sem_estrada;
            continue;
        }
        ++l.por_situacao[static_cast<std::size_t>(situacao(cidade, id, sim.data.mes, &sim.populacao))];
    }
    std::vector<LinhaDeProducao> r;
    for (IdTipoEdificio t = 0; t < por_tipo.size(); ++t) {
        if (por_tipo[t].edificios == 0) continue;
        por_tipo[t].nome = conteudo.edificios[t].nome;
        r.push_back(std::move(por_tipo[t]));
    }
    return r;
}

RelatorioTransporte montar_transporte(const Simulacao& sim) {
    RelatorioTransporte r;
    for (const Carregador& c : sim.transporte.carregadores()) {
        ++r.carregadores;
        if (c.casa < sim.cidade.edificios().size() && sim.cidade.definicao(c.casa).deposito) ++r.do_deposito;
        if (!c.livre()) ++r.ocupados;
        if (c.carregando) ++r.carregando;
    }
    r.entregas = sim.transporte.entregas();
    r.perdidos = sim.transporte.perdidos();
    return r;
}

}  // namespace

Relatorio montar_relatorio(const Simulacao& sim) {
    return { montar_emprego(sim), montar_comida(sim), montar_producao(sim), montar_transporte(sim) };
}

std::string resumir_producao(const LinhaDeProducao& linha) {
    std::string texto;
    auto juntar = [&texto](int n, const char* o_que) {
        if (n == 0) return;
        if (!texto.empty()) texto += ", ";
        texto += std::to_string(n) + " " + o_que;
    };
    // Produzindo primeiro; depois do mais grave (sem estrada) ao mais normal
    // (pausa da estação), na ordem em que o jogador costuma resolver.
    juntar(linha.por_situacao[static_cast<std::size_t>(Situacao::Produzindo)], "produzindo");
    juntar(linha.sem_estrada, "sem estrada");
    for (const Situacao s : { Situacao::SemTrabalhadores, Situacao::FaltaEntrada, Situacao::SaidaCheia,
                              Situacao::ForaDeEstacao, Situacao::Pausado }) {
        juntar(linha.por_situacao[static_cast<std::size_t>(s)], descrever(s));
    }
    return texto;
}
