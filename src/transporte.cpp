#include "transporte.hpp"

#include <limits>

void Transporte::criar_carregadores(const Cidade& cidade) {
    ja_criou_.resize(cidade.edificios().size(), 0);
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (ja_criou_[id] || !cidade.edificio(id).ativo) continue;
        ja_criou_[id] = 1;
        const DefinicaoEdificio& def = cidade.definicao(id);
        for (int i = 0; i < def.carregadores; ++i) {
            Carregador c;
            c.casa = id;
            c.posicao = cidade.porta(id);
            carregadores_.push_back(c);
        }
    }
}

// O depósito foi demolido: o carregador adota o depósito ativo de rota mais
// curta. Sem nenhum alcançável, fica onde está com a casa antiga, e tenta
// de novo no próximo tick (a busca sem rota fica no cache e custa pouco).
void Transporte::mudar_de_casa(const Cidade& cidade, Carregador& c) {
    std::size_t melhor = std::numeric_limits<std::size_t>::max();
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo || !cidade.definicao(id).deposito) continue;
        const auto& r = rotas_.rota(cidade, c.posicao, cidade.porta(id));
        if (r && r->size() < melhor) {
            melhor = r->size();
            c.casa = id;
        }
    }
}

bool Transporte::enviar(const Cidade& cidade, Carregador& c, Celula alvo) {
    const auto& r = rotas_.rota(cidade, c.posicao, alvo);
    if (!r) return false;
    c.rota = *r;
    c.passo = 1;  // rota[0] é a célula onde ele já está
    c.ticks_na_celula = 0;
    return true;
}

void Transporte::chegou(Cidade& cidade, Carregador& c) {
    (void)cidade;
    c.rota.clear();
    c.passo = 0;
}

void Transporte::mover(Cidade& cidade, Carregador& c) {
    if (!c.andando()) return;
    if (++c.ticks_na_celula < TICKS_POR_CELULA) return;
    c.ticks_na_celula = 0;

    const Celula proxima = c.rota[c.passo];
    if (!cidade.tem_estrada(proxima)) {
        // Demoliram a estrada à frente: recalcula até o mesmo alvo. Se não
        // houver outro caminho, para onde está.
        const Celula alvo = c.rota.back();
        if (!enviar(cidade, c, alvo)) chegou(cidade, c);
        return;
    }
    c.posicao = proxima;
    if (++c.passo == c.rota.size()) chegou(cidade, c);
}

void Transporte::passo(Cidade& cidade) {
    criar_carregadores(cidade);
    for (Carregador& c : carregadores_) {
        if (!cidade.edificio(c.casa).ativo) mudar_de_casa(cidade, c);
        // Livre e longe de casa: volta.
        if (c.livre() && !c.andando() && !(c.posicao == cidade.porta(c.casa))) {
            enviar(cidade, c, cidade.porta(c.casa));
        }
        mover(cidade, c);
    }
}
