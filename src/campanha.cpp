#include "campanha.hpp"

#include "cidade.hpp"

namespace {

bool todas_no_maximo(const std::vector<EstadoInstituicao>& instituicoes) {
    if (instituicoes.empty()) return false;
    for (const EstadoInstituicao& instituicao : instituicoes) {
        if (instituicao.nivel <= 0 || instituicao.nivel != instituicao.nivel_maximo_disponivel) return false;
    }
    return true;
}

}  // namespace

int ano_de_abertura(Fase fase) {
    switch (fase) {
        case Fase::Vila: return 1150;
        case Fase::Mercado: return 1200;
        case Fase::Cidade: return 1250;
        case Fase::Hansa: return 1300;
        case Fase::Auge: return 1370;
    }
    return 0;
}

std::vector<EstadoInstituicao> estados_das_instituicoes(const Cidade& cidade, Fase fase) {
    std::vector<EstadoInstituicao> resultado;
    for (IdTipoEdificio tipo = 0; tipo < cidade.conteudo().edificios.size(); ++tipo) {
        const DefinicaoEdificio& def = cidade.conteudo().edificio(tipo);
        if (!def.instituicao || def.fase > fase) continue;
        int nivel = 0;
        for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
            const Edificio& edificio = cidade.edificio(id);
            if (edificio.ativo && edificio.tipo == tipo) {
                nivel = edificio.nivel;
                break;
            }
        }
        resultado.push_back({ *def.instituicao, nivel, nivel_maximo_disponivel(def, fase) });
    }
    return resultado;
}

ResultadoProgressao tentar_avancar_fase(Fase fase_atual, Data data,
                                         const std::vector<EstadoInstituicao>& instituicoes) {
    if (fase_atual == Fase::Auge || data.ano < ano_de_abertura(static_cast<Fase>(static_cast<int>(fase_atual) + 1)) ||
        !todas_no_maximo(instituicoes)) {
        return { fase_atual, false };
    }
    return { static_cast<Fase>(static_cast<int>(fase_atual) + 1), true };
}
