#include "populacao.hpp"

#include <algorithm>

Populacao::Populacao(int quantidade_inicial)
    : moradores_(static_cast<std::size_t>(std::max(0, quantidade_inicial))) {}

Populacao Populacao::restaurar(std::vector<Morador> moradores, int mortos_de_fome, int refeicoes_perdidas) {
    Populacao p(0);
    p.moradores_ = std::move(moradores);
    p.mortos_de_fome_ = mortos_de_fome;
    p.refeicoes_perdidas_ = refeicoes_perdidas;
    return p;
}

int Populacao::total() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [](const Morador& m) { return m.vivo; }));
}

int Populacao::alojados() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [](const Morador& m) { return m.vivo && m.casa != SEM_EDIFICIO; }));
}

int Populacao::moradores_na_casa(IdEdificio casa) const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [casa](const Morador& m) { return m.vivo && m.casa == casa; }));
}

int Populacao::trabalhadores_em(IdEdificio edificio) const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [edificio](const Morador& m) { return m.vivo && m.trabalho == edificio; }));
}

int Populacao::desempregados() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(), [](const Morador& m) {
        return m.vivo && m.casa != SEM_EDIFICIO && m.trabalho == SEM_EDIFICIO;
    }));
}

void Populacao::alojar(Cidade& cidade) {
    for (Morador& m : moradores_) {
        if (!m.vivo) continue;
        if (m.casa != SEM_EDIFICIO && !cidade.edificio(m.casa).ativo) m.casa = SEM_EDIFICIO;
    }

    for (Morador& m : moradores_) {
        if (!m.vivo) continue;
        if (m.casa != SEM_EDIFICIO) continue;
        for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
            const Edificio& e = cidade.edificio(id);
            const DefinicaoEdificio& def = cidade.definicao(id);
            if (e.ativo && def.moradores > moradores_na_casa(id)) {
                m.casa = id;
                break;
            }
        }
    }
}

void Populacao::empregar(const Cidade& cidade) {
    // Recalcular em ordem de id torna o resultado independente do histórico
    // de construções e libera imediatamente quem perdeu casa ou trabalho.
    for (Morador& m : moradores_) m.trabalho = SEM_EDIFICIO;
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo) continue;
        const int vagas = cidade.definicao(id).trabalhadores;
        for (Morador& m : moradores_) {
            if (trabalhadores_em(id) >= vagas) break;
            if (m.vivo && m.casa != SEM_EDIFICIO && m.trabalho == SEM_EDIFICIO) m.trabalho = id;
        }
    }
}

void Populacao::consumir_comida(Cidade& cidade) {
    const Conteudo& conteudo = cidade.conteudo();
    for (Morador& m : moradores_) {
        if (!m.vivo) continue;
        bool comeu = false;
        if (m.casa != SEM_EDIFICIO) {
            for (IdItem item = 0; item < conteudo.itens.size(); ++item) {
                if (conteudo.itens[item].tipo != TipoItem::Comida) continue;
                if (cidade.retirar(m.casa, item, 1)) {
                    comeu = true;
                    break;
                }
            }
        }
        m.dias_sem_comida = comeu ? 0 : m.dias_sem_comida + 1;
        if (!comeu) ++refeicoes_perdidas_;
        if (m.dias_sem_comida >= DIAS_ATE_MORRER_DE_FOME) {
            m.vivo = false;
            m.casa = SEM_EDIFICIO;
            m.trabalho = SEM_EDIFICIO;
            ++mortos_de_fome_;
        }
    }
}
