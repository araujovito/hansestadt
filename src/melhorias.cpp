#include "melhorias.hpp"

#include <algorithm>

#include "campanha.hpp"

FichaDaInstituicao ficha_da_instituicao(const Cidade& cidade, IdTipoEdificio tipo, Fase fase) {
    const Conteudo& conteudo = cidade.conteudo();
    const DefinicaoEdificio& def = conteudo.edificio(tipo);
    FichaDaInstituicao ficha;
    ficha.nome = def.nome;
    ficha.nivel_da_fase = nivel_maximo_disponivel(def, fase);
    ficha.nivel_final = static_cast<int>(def.melhorias.size()) + 1;
    // Instituição só pode ser construída uma vez: o primeiro ativo é ela.
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        const Edificio& e = cidade.edificio(id);
        if (e.ativo && e.tipo == tipo) {
            ficha.edificio = id;
            ficha.nivel = e.nivel;
            break;
        }
    }
    if (!ficha.edificio) return ficha;

    ficha.pode_melhorar = cidade.validar_melhoria(*ficha.edificio, fase);
    if (ficha.nivel < 1 || ficha.nivel >= ficha.nivel_final) return ficha;
    const MelhoriaEdificio& m = def.melhorias[static_cast<std::size_t>(ficha.nivel - 1)];
    ficha.fase_da_proxima = m.fase;
    for (const Quantidade& q : m.custo) {
        int tem = 0;
        for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) tem += cidade.disponivel(id, q.item);
        ficha.materiais.push_back({ conteudo.itens[q.item].nome, q.quantidade, tem });
    }
    return ficha;
}

bool RumoAProximaFase::instituicoes_prontas() const {
    // Como em tentar_avancar_fase: sem nenhuma instituição, não avança.
    return !instituicoes.empty() &&
           std::all_of(instituicoes.begin(), instituicoes.end(),
                       [](const FichaDaInstituicao& f) { return f.pronta_para_a_fase(); });
}

std::optional<RumoAProximaFase> rumo_a_proxima_fase(const Cidade& cidade, Fase fase, Data hoje) {
    if (fase == Fase::Auge) return std::nullopt;
    RumoAProximaFase rumo;
    rumo.fase = static_cast<Fase>(static_cast<int>(fase) + 1);
    rumo.ano = ano_de_abertura(rumo.fase);
    rumo.ano_chegou = hoje.ano >= rumo.ano;
    const Conteudo& conteudo = cidade.conteudo();
    for (IdTipoEdificio tipo = 0; tipo < conteudo.edificios.size(); ++tipo) {
        const DefinicaoEdificio& def = conteudo.edificio(tipo);
        if (def.instituicao && def.fase <= fase) rumo.instituicoes.push_back(ficha_da_instituicao(cidade, tipo, fase));
    }
    return rumo;
}
