#include "cidade.hpp"

#include <algorithm>
#include <limits>
#include <utility>

const char* descrever(Validacao v) {
    switch (v) {
        case Validacao::Ok: return "ok";
        case Validacao::ForaDoMapa: return "fora do mapa";
        case Validacao::TerrenoErrado: return "terreno inadequado";
        case Validacao::Ocupado: return "lugar ocupado";
        case Validacao::LongeDoRecurso: return "longe do recurso";
        case Validacao::PortaBloqueada: return "entrada bloqueada";
        case Validacao::SemRecursos: return "faltam materiais";
        case Validacao::JaExiste: return "já existe";
    }
    return "?";
}

const char* descrever(ResultadoMelhoria r) {
    switch (r) {
        case ResultadoMelhoria::Ok: return "ok";
        case ResultadoMelhoria::EdificioInativo: return "instituição inativa";
        case ResultadoMelhoria::NaoInstituicao: return "não é uma instituição";
        case ResultadoMelhoria::NivelMaximo: return "já está no nível máximo";
        case ResultadoMelhoria::FaseBloqueada: return "nível ainda não disponível nesta fase";
        case ResultadoMelhoria::SemRecursos: return "faltam materiais";
    }
    return "?";
}

const char* descrever(ResultadoProdutividade r) {
    switch (r) {
        case ResultadoProdutividade::Ok: return "ok";
        case ResultadoProdutividade::TipoSemMelhoria: return "este edifício não tem melhoria de produtividade";
        case ResultadoProdutividade::NivelMaximo: return "produtividade no nível máximo";
        case ResultadoProdutividade::FaseBloqueada: return "melhoria ainda não disponível nesta fase";
        case ResultadoProdutividade::SemRecursos: return "faltam materiais";
    }
    return "?";
}

Cidade::Cidade(Mapa mapa, const Conteudo& conteudo)
    : mapa_(std::move(mapa)),
      conteudo_(&conteudo),
      ocupacao_(static_cast<std::size_t>(mapa_.largura()) * static_cast<std::size_t>(mapa_.altura()),
                SEM_EDIFICIO),
      estradas_(ocupacao_.size(), 0),
      trilhas_(ocupacao_.size(), 0),
      uso_trilhas_(ocupacao_.size(), 0),
      dias_sem_uso_trilhas_(ocupacao_.size(), 0),
      usada_hoje_(ocupacao_.size(), 0),
      portas_(ocupacao_.size(), 0),
      produtividade_(conteudo.edificios.size(), 0) {}

Cidade Cidade::restaurar(Mapa mapa, const Conteudo& conteudo, std::vector<Edificio> edificios,
                         std::vector<std::uint8_t> estradas, std::uint64_t versao_estradas,
                         std::vector<int> produtividade, std::vector<std::uint8_t> trilhas,
                         std::vector<std::uint16_t> uso_trilhas,
                         std::vector<std::uint8_t> dias_sem_uso_trilhas) {
    Cidade cidade(std::move(mapa), conteudo);
    if (estradas.size() != cidade.estradas_.size()) return cidade;
    cidade.edificios_ = std::move(edificios);
    cidade.estradas_ = std::move(estradas);
    if (trilhas.size() == cidade.trilhas_.size()) cidade.trilhas_ = std::move(trilhas);
    if (uso_trilhas.size() == cidade.uso_trilhas_.size()) cidade.uso_trilhas_ = std::move(uso_trilhas);
    if (dias_sem_uso_trilhas.size() == cidade.dias_sem_uso_trilhas_.size())
        cidade.dias_sem_uso_trilhas_ = std::move(dias_sem_uso_trilhas);
    cidade.versao_estradas_ = versao_estradas;
    if (!produtividade.empty()) cidade.produtividade_ = std::move(produtividade);
    for (IdEdificio id = 0; id < cidade.edificios_.size(); ++id) {
        const Edificio& e = cidade.edificios_[id];
        if (!e.ativo) continue;
        const DefinicaoEdificio& def = cidade.definicao(id);
        for (int dy = 0; dy < def.altura_total(); ++dy)
            for (int dx = 0; dx < def.largura_total(); ++dx)
                cidade.ocupacao_[cidade.indice({e.origem.x + dx, e.origem.y + dy})] = id;
        cidade.portas_[cidade.indice(porta_de(def, e.origem))] = 1;
    }
    return cidade;
}

ResultadoProdutividade Cidade::validar_produtividade(IdTipoEdificio tipo, Fase fase_atual) const {
    if (tipo >= conteudo_->edificios.size()) return ResultadoProdutividade::TipoSemMelhoria;
    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    if (def.produtividade.empty()) return ResultadoProdutividade::TipoSemMelhoria;
    const int nivel = produtividade_[tipo];
    if (nivel < 0 || static_cast<std::size_t>(nivel) >= def.produtividade.size())
        return ResultadoProdutividade::NivelMaximo;
    const MelhoriaProdutividade& melhoria = def.produtividade[static_cast<std::size_t>(nivel)];
    if (melhoria.fase > fase_atual) return ResultadoProdutividade::FaseBloqueada;
    for (const Quantidade& q : melhoria.custo) {
        int total = 0;
        for (IdEdificio id = 0; id < edificios_.size(); ++id) total += disponivel(id, q.item);
        if (total < q.quantidade) return ResultadoProdutividade::SemRecursos;
    }
    return ResultadoProdutividade::Ok;
}

ResultadoProdutividade Cidade::melhorar_produtividade(IdTipoEdificio tipo, Fase fase_atual) {
    const ResultadoProdutividade resultado = validar_produtividade(tipo, fase_atual);
    if (resultado != ResultadoProdutividade::Ok) return resultado;
    const MelhoriaProdutividade& melhoria = conteudo_->edificio(tipo).produtividade[produtividade_[tipo]];
    for (const Quantidade& q : melhoria.custo) {
        int restante = q.quantidade;
        for (IdEdificio origem = 0; origem < edificios_.size() && restante > 0; ++origem) {
            Edificio& e = edificios_[origem];
            if (!e.ativo) continue;
            const int retirado = std::min(restante, disponivel(origem, q.item));
            e.estoque[q.item] -= retirado;
            restante -= retirado;
        }
    }
    ++produtividade_[tipo];
    return ResultadoProdutividade::Ok;
}

int Cidade::bonus_produtividade(IdTipoEdificio tipo) const {
    int bonus = 0;
    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    for (int i = 0; i < produtividade_[tipo]; ++i)
        bonus += def.produtividade[static_cast<std::size_t>(i)].percentual;
    return bonus;
}

Celula porta_de(const DefinicaoEdificio& def, Celula origem) {
    return { origem.x + (def.largura_total() - 1) / 2, origem.y + def.altura_total() };
}

Validacao Cidade::validar_edificio(IdTipoEdificio tipo, Celula origem) const {
    const DefinicaoEdificio& def = conteudo_->edificio(tipo);
    if (def.instituicao) {
        for (const Edificio& existente : edificios_)
            if (existente.ativo && conteudo_->edificio(existente.tipo).instituicao == def.instituicao)
                return Validacao::JaExiste;
    }
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
    if (!tem_recursos_para(tipo)) return Validacao::SemRecursos;
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
    consumir_custo(def);
    return id;
}

ResultadoMelhoria Cidade::validar_melhoria(IdEdificio id, Fase fase_atual) const {
    if (id >= edificios_.size() || !edificios_[id].ativo) return ResultadoMelhoria::EdificioInativo;
    const DefinicaoEdificio& def = definicao(id);
    if (!def.instituicao) return ResultadoMelhoria::NaoInstituicao;
    const Edificio& e = edificios_[id];
    if (e.nivel < 1 || static_cast<std::size_t>(e.nivel) > def.melhorias.size())
        return ResultadoMelhoria::NivelMaximo;
    const MelhoriaEdificio& melhoria = def.melhorias[static_cast<std::size_t>(e.nivel - 1)];
    if (melhoria.fase > fase_atual) return ResultadoMelhoria::FaseBloqueada;
    for (const Quantidade& q : melhoria.custo) {
        int disponivel_total = 0;
        for (IdEdificio origem = 0; origem < edificios_.size(); ++origem)
            disponivel_total += disponivel(origem, q.item);
        if (disponivel_total < q.quantidade) return ResultadoMelhoria::SemRecursos;
    }
    return ResultadoMelhoria::Ok;
}

ResultadoMelhoria Cidade::melhorar_edificio(IdEdificio id, Fase fase_atual) {
    const ResultadoMelhoria resultado = validar_melhoria(id, fase_atual);
    if (resultado != ResultadoMelhoria::Ok) return resultado;
    const MelhoriaEdificio& melhoria = definicao(id).melhorias[static_cast<std::size_t>(edificios_[id].nivel - 1)];
    for (const Quantidade& q : melhoria.custo) {
        int restante = q.quantidade;
        for (IdEdificio origem = 0; origem < edificios_.size() && restante > 0; ++origem) {
            Edificio& e = edificios_[origem];
            if (!e.ativo) continue;
            const int retirado = std::min(restante, disponivel(origem, q.item));
            e.estoque[q.item] -= retirado;
            restante -= retirado;
        }
    }
    ++edificios_[id].nivel;
    return ResultadoMelhoria::Ok;
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
    trilhas_[indice(c)] = 0;
    uso_trilhas_[indice(c)] = 200;
    ++versao_estradas_;
    return true;
}

bool Cidade::tem_trilha(Celula c) const {
    return mapa_.dentro(c.x, c.y) && trilhas_[indice(c)] != 0;
}

bool Cidade::transitavel(Celula c) const {
    if (!mapa_.dentro(c.x, c.y) || mapa_.terreno(c.x, c.y) == Terreno::Agua) return false;
    return ocupacao_[indice(c)] == SEM_EDIFICIO;
}

int Cidade::custo_movimento(Celula c) const {
    if (!transitavel(c)) return std::numeric_limits<int>::max();
    if (tem_estrada(c)) return 100;
    if (tem_trilha(c)) return 133;
    return mapa_.terreno(c.x, c.y) == Terreno::Floresta ? 286 : 200;
}

void Cidade::registrar_passagem(Celula c) {
    if (!transitavel(c) || tem_estrada(c)) return;
    const std::size_t i = indice(c);
    usada_hoje_[i] = 1;
    dias_sem_uso_trilhas_[i] = 0;
    if (uso_trilhas_[i] < 200) ++uso_trilhas_[i];
    if (!trilhas_[i] && uso_trilhas_[i] >= 30) {
        trilhas_[i] = 1;
        ++versao_estradas_;
    }
    if (uso_trilhas_[i] >= 200) {
        trilhas_[i] = 0;
        estradas_[i] = 1;
        ++versao_estradas_;
    }
}

void Cidade::avancar_desgaste_trilhas() {
    for (std::size_t i = 0; i < trilhas_.size(); ++i) {
        if (estradas_[i] || !trilhas_[i]) { usada_hoje_[i] = 0; continue; }
        if (usada_hoje_[i]) {
            dias_sem_uso_trilhas_[i] = 0;
        } else if (++dias_sem_uso_trilhas_[i] >= 7) {
            dias_sem_uso_trilhas_[i] = 0;
            if (uso_trilhas_[i] > 0) --uso_trilhas_[i];
            if (uso_trilhas_[i] == 0) {
                trilhas_[i] = 0;
                ++versao_estradas_;
            }
        }
        usada_hoje_[i] = 0;
    }
}

bool Cidade::demolir(Celula c) {
    if (!mapa_.dentro(c.x, c.y)) return false;

    if (estradas_[indice(c)]) {
        estradas_[indice(c)] = 0;
        uso_trilhas_[indice(c)] = 0;
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
    return edificios_[id].ativo && transitavel(porta(id));
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
    if ((def.moradores > 0 || def.alimenta_sem_teto) && conteudo_->itens[item].tipo == TipoItem::Comida) {
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

int Cidade::estoque_total(IdItem item) const {
    int total = 0;
    for (const Edificio& e : edificios_) {
        if (e.ativo) total += e.estoque[item];
    }
    return total;
}

int Cidade::estoque_total(TipoItem tipo) const {
    int total = 0;
    for (IdItem item = 0; item < conteudo_->itens.size(); ++item) {
        if (conteudo_->itens[item].tipo == tipo) total += estoque_total(item);
    }
    return total;
}

bool Cidade::tem_recursos_para(IdTipoEdificio tipo) const {
    for (const Quantidade& q : conteudo_->edificio(tipo).custo) {
        int total = 0;
        for (IdEdificio id = 0; id < edificios_.size(); ++id) total += disponivel(id, q.item);
        if (total < q.quantidade) return false;
    }
    return true;
}

void Cidade::consumir_custo(const DefinicaoEdificio& def) {
    for (const Quantidade& q : def.custo) {
        int restante = q.quantidade;
        for (IdEdificio id = 0; id < edificios_.size() && restante > 0; ++id) {
            Edificio& e = edificios_[id];
            if (!e.ativo) continue;
            const int retirado = std::min(restante, disponivel(id, q.item));
            e.estoque[q.item] -= retirado;
            restante -= retirado;
        }
    }
}

bool Cidade::tem_recurso_proximo(IdEdificio id) const {
    const DefinicaoEdificio& def = definicao(id);
    return def.extrai && def.requer && atende_requisito(*def.requer, edificios_[id].origem,
                                                        def.largura_total(), def.altura_total());
}

bool Cidade::extrair_recurso_proximo(IdEdificio id) {
    if (!tem_recurso_proximo(id)) return false;
    const Edificio& e = edificios_[id];
    const DefinicaoEdificio& def = definicao(id);
    const Requisito& r = *def.requer;
    for (int y = e.origem.y - r.distancia; y < e.origem.y + def.altura_total() + r.distancia; ++y) {
        for (int x = e.origem.x - r.distancia; x < e.origem.x + def.largura_total() + r.distancia; ++x) {
            if (mapa_.dentro(x, y) && mapa_.terreno(x, y) == *def.extrai) {
                mapa_.definir_terreno(x, y, Terreno::Campo);
                return true;
            }
        }
    }
    return false;
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
