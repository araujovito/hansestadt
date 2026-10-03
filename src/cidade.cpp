#include "cidade.hpp"

#include <utility>

const char* descrever(Validacao v) {
    switch (v) {
        case Validacao::Ok: return "ok";
        case Validacao::ForaDoMapa: return "fora do mapa";
        case Validacao::TerrenoErrado: return "terreno inadequado";
        case Validacao::Ocupado: return "lugar ocupado";
        case Validacao::LongeDoRecurso: return "longe do recurso";
        case Validacao::PortaBloqueada: return "entrada bloqueada";
        case Validacao::JaExiste: return "já existe";
    }
    return "?";
}

Cidade::Cidade(Mapa mapa, const Conteudo& conteudo)
    : mapa_(std::move(mapa)),
      conteudo_(&conteudo),
      ocupacao_(static_cast<std::size_t>(mapa_.largura()) * static_cast<std::size_t>(mapa_.altura()),
                SEM_EDIFICIO),
      estradas_(ocupacao_.size(), 0),
      portas_(ocupacao_.size(), 0) {}

Celula porta_de(const DefinicaoEdificio& def, Celula origem) {
    return { origem.x + (def.largura_total() - 1) / 2, origem.y + def.altura_total() };
}

Validacao Cidade::validar_edificio(IdTipoEdificio tipo, Celula origem) const {
    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    for (int dy = 0; dy < def.altura_total(); ++dy) {
        for (int dx = 0; dx < def.largura_total(); ++dx) {
            const Celula c{ origem.x + dx, origem.y + dy };
            if (!mapa_.dentro(c.x, c.y)) return Validacao::ForaDoMapa;
            // O anel de lavoura precisa de campo aberto; o núcleo, do terreno
            // do edifício (que também é campo, no caso da fazenda).
            const Terreno exigido = eh_lavoura(def, dx, dy) ? Terreno::Campo : def.terreno;
            if (mapa_.terreno(c.x, c.y) != exigido) return Validacao::TerrenoErrado;
            if (ocupacao_[indice(c)] != SEM_EDIFICIO || estradas_[indice(c)]) return Validacao::Ocupado;
            // Construir sobre a entrada de outro edifício o deixaria isolado.
            if (portas_[indice(c)]) return Validacao::Ocupado;
        }
    }

    const Celula p = porta_de(def, origem);
    if (!mapa_.dentro(p.x, p.y)) return Validacao::ForaDoMapa;
    if (ocupacao_[indice(p)] != SEM_EDIFICIO) return Validacao::Ocupado;
    const Terreno tp = mapa_.terreno(p.x, p.y);
    if (tp == Terreno::Floresta || tp == Terreno::Agua) return Validacao::PortaBloqueada;

    if (def.requer && !atende_requisito(*def.requer, origem, def.largura_total(), def.altura_total())) {
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
    const std::vector<int> zeros(conteudo_->itens.size(), 0);
    edificios_.push_back({ tipo, origem, true, zeros, zeros, zeros, -1, 0 });

    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    for (int dy = 0; dy < def.altura_total(); ++dy) {
        for (int dx = 0; dx < def.largura_total(); ++dx) {
            ocupacao_[indice({ origem.x + dx, origem.y + dy })] = id;
        }
    }
    portas_[indice(porta_de(def, origem))] = 1;
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
    ++versao_estradas_;
    return true;
}

bool Cidade::demolir(Celula c) {
    if (!mapa_.dentro(c.x, c.y)) return false;

    if (estradas_[indice(c)]) {
        estradas_[indice(c)] = 0;
        ++versao_estradas_;
        return true;
    }

    const IdEdificio id = ocupacao_[indice(c)];
    if (id == SEM_EDIFICIO) return false;

    Edificio& e = edificios_[id];
    e.ativo = false;
    const DefinicaoEdificio& def = conteudo_->edificio(e.tipo);
    for (int dy = 0; dy < def.altura_total(); ++dy) {
        for (int dx = 0; dx < def.largura_total(); ++dx) {
            ocupacao_[indice({ e.origem.x + dx, e.origem.y + dy })] = SEM_EDIFICIO;
        }
    }
    portas_[indice(porta_de(def, e.origem))] = 0;
    return true;
}

Celula Cidade::porta(IdEdificio id) const {
    return porta_de(definicao(id), edificios_[id].origem);
}

bool Cidade::tem_acesso(IdEdificio id) const {
    return edificios_[id].ativo && tem_estrada(porta(id));
}

int Cidade::espaco_para(IdEdificio id, IdItem item) const {
    const Edificio& e = edificios_[id];
    if (!e.ativo) return 0;
    const DefinicaoEdificio& def = conteudo_->edificio(e.tipo);
    if (def.deposito) {
        int total = 0;
        for (int q : e.estoque) total += q;
        return def.estoque - total;
    }
    if (def.moradores > 0 && conteudo_->itens[item].tipo == TipoItem::Comida) {
        int comida = 0;
        for (IdItem i = 0; i < conteudo_->itens.size(); ++i) {
            if (conteudo_->itens[i].tipo == TipoItem::Comida) comida += e.estoque[i];
        }
        return def.estoque - comida;
    }
    return def.usa_item(item) ? def.estoque - e.estoque[item] : 0;
}

int Cidade::disponivel(IdEdificio id, IdItem item) const {
    const Edificio& e = edificios_[id];
    return e.ativo ? e.estoque[item] - e.reservado[item] : 0;
}

int Cidade::espaco_livre(IdEdificio id, IdItem item) const {
    const Edificio& e = edificios_[id];
    if (!e.ativo) return 0;
    if (conteudo_->edificio(e.tipo).deposito) {
        int chegando = 0;
        for (int q : e.a_caminho) chegando += q;
        return espaco_para(id, item) - chegando;
    }
    return espaco_para(id, item) - e.a_caminho[item];
}

bool Cidade::adicionar(IdEdificio id, IdItem item, int quantidade) {
    if (espaco_para(id, item) < quantidade) return false;
    edificios_[id].estoque[item] += quantidade;
    return true;
}

bool Cidade::retirar(IdEdificio id, IdItem item, int quantidade) {
    Edificio& e = edificios_[id];
    if (!e.ativo || e.estoque[item] < quantidade) return false;
    e.estoque[item] -= quantidade;
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
