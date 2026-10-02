#include "cidade.hpp"

#include <utility>

const char* descrever(Validacao v) {
    switch (v) {
        case Validacao::Ok: return "ok";
        case Validacao::ForaDoMapa: return "fora do mapa";
        case Validacao::TerrenoErrado: return "terreno inadequado";
        case Validacao::Ocupado: return "lugar ocupado";
        case Validacao::LongeDoRecurso: return "longe do recurso";
        case Validacao::JaExiste: return "já existe";
    }
    return "?";
}

Cidade::Cidade(Mapa mapa, const Conteudo& conteudo)
    : mapa_(std::move(mapa)),
      conteudo_(&conteudo),
      ocupacao_(static_cast<std::size_t>(mapa_.largura()) * static_cast<std::size_t>(mapa_.altura()),
                SEM_EDIFICIO),
      estradas_(ocupacao_.size(), 0) {}

Validacao Cidade::validar_edificio(IdTipoEdificio tipo, Celula origem) const {
    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    for (int dy = 0; dy < def.altura; ++dy) {
        for (int dx = 0; dx < def.largura; ++dx) {
            const Celula c{ origem.x + dx, origem.y + dy };
            if (!mapa_.dentro(c.x, c.y)) return Validacao::ForaDoMapa;
            if (mapa_.terreno(c.x, c.y) != def.terreno) return Validacao::TerrenoErrado;
            if (ocupacao_[indice(c)] != SEM_EDIFICIO || estradas_[indice(c)]) return Validacao::Ocupado;
        }
    }
    if (def.requer && !atende_requisito(*def.requer, origem, def.largura, def.altura)) {
        return Validacao::LongeDoRecurso;
    }
    return Validacao::Ok;
}

// Procura o terreno num retângulo que é o edifício expandido de `distancia`
// células em cada direção, diagonais incluídas. O próprio edifício nunca
// atende, porque já foi validado como outro terreno.
bool Cidade::atende_requisito(const Requisito& r, Celula origem, int largura, int altura) const {
    for (int y = origem.y - r.distancia; y < origem.y + altura + r.distancia; ++y) {
        for (int x = origem.x - r.distancia; x < origem.x + largura + r.distancia; ++x) {
            if (mapa_.dentro(x, y) && mapa_.terreno(x, y) == r.terreno) return true;
        }
    }
    return false;
}

std::optional<IdEdificio> Cidade::construir_edificio(IdTipoEdificio tipo, Celula origem) {
    if (validar_edificio(tipo, origem) != Validacao::Ok) return std::nullopt;

    const auto id = static_cast<IdEdificio>(edificios_.size());
    edificios_.push_back({ tipo, origem, true });

    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    for (int dy = 0; dy < def.altura; ++dy) {
        for (int dx = 0; dx < def.largura; ++dx) {
            ocupacao_[indice({ origem.x + dx, origem.y + dy })] = id;
        }
    }
    return id;
}

Validacao Cidade::validar_estrada(Celula c) const {
    if (!mapa_.dentro(c.x, c.y)) return Validacao::ForaDoMapa;
    // Floresta precisa ser derrubada e água precisará de ponte; argila e
    // turfa aceitam um caminho de terra batida por cima.
    const Terreno t = mapa_.terreno(c.x, c.y);
    if (t == Terreno::Floresta || t == Terreno::Agua) return Validacao::TerrenoErrado;
    if (ocupacao_[indice(c)] != SEM_EDIFICIO) return Validacao::Ocupado;
    if (estradas_[indice(c)]) return Validacao::JaExiste;
    return Validacao::Ok;
}

bool Cidade::construir_estrada(Celula c) {
    if (validar_estrada(c) != Validacao::Ok) return false;
    estradas_[indice(c)] = 1;
    return true;
}

bool Cidade::demolir(Celula c) {
    if (!mapa_.dentro(c.x, c.y)) return false;

    if (estradas_[indice(c)]) {
        estradas_[indice(c)] = 0;
        return true;
    }

    const IdEdificio id = ocupacao_[indice(c)];
    if (id == SEM_EDIFICIO) return false;

    Edificio& e = edificios_[id];
    e.ativo = false;
    const DefinicaoEdificio& def = conteudo_->edificio(e.tipo);
    for (int dy = 0; dy < def.altura; ++dy) {
        for (int dx = 0; dx < def.largura; ++dx) {
            ocupacao_[indice({ e.origem.x + dx, e.origem.y + dy })] = SEM_EDIFICIO;
        }
    }
    return true;
}

bool Cidade::tem_estrada(Celula c) const {
    return mapa_.dentro(c.x, c.y) && estradas_[indice(c)] != 0;
}

IdEdificio Cidade::edificio_em(Celula c) const {
    return mapa_.dentro(c.x, c.y) ? ocupacao_[indice(c)] : SEM_EDIFICIO;
}

std::vector<Celula> caminho_em_l(Celula inicio, Celula fim) {
    std::vector<Celula> caminho;
    const int passo_x = fim.x >= inicio.x ? 1 : -1;
    const int passo_y = fim.y >= inicio.y ? 1 : -1;
    for (int x = inicio.x; x != fim.x + passo_x; x += passo_x) caminho.push_back({ x, inicio.y });
    for (int y = inicio.y + passo_y; y != fim.y + passo_y; y += passo_y) caminho.push_back({ fim.x, y });
    return caminho;
}
