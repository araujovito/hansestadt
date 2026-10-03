#include "populacao.hpp"

#include <algorithm>

Populacao::Populacao(int quantidade_inicial)
    : moradores_(static_cast<std::size_t>(std::max(0, quantidade_inicial))) {
    for (std::size_t i = 0; i < moradores_.size(); ++i) moradores_[i].mulher = i % 2 != 0;
}

Populacao Populacao::restaurar(std::vector<Morador> moradores, int mortos_de_fome, int refeicoes_perdidas) {
    Populacao p(0);
    p.moradores_ = std::move(moradores);
    p.mortos_de_fome_ = mortos_de_fome;
    p.refeicoes_perdidas_ = refeicoes_perdidas;
    // A contagem por edifício não vai para o arquivo: sai dos próprios
    // moradores, que guardam onde trabalham.
    for (const Morador& m : p.moradores_) {
        if (!m.vivo || m.trabalho == SEM_EDIFICIO) continue;
        if (m.trabalho >= p.trabalhadores_.size()) p.trabalhadores_.resize(m.trabalho + 1, 0);
        ++p.trabalhadores_[m.trabalho];
    }
    return p;
}

int Populacao::total() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [](const Morador& m) { return m.vivo; }));
}

int Populacao::criancas() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(),
                                          [](const Morador& m) { return m.vivo && !m.adulto(); }));
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
    return edificio < trabalhadores_.size() ? trabalhadores_[edificio] : 0;
}

int Populacao::desempregados() const {
    return static_cast<int>(std::count_if(moradores_.begin(), moradores_.end(), [](const Morador& m) {
        return m.vivo && m.adulto() && m.casa != SEM_EDIFICIO && m.trabalho == SEM_EDIFICIO;
    }));
}

void Populacao::alojar(Cidade& cidade) {
    for (Morador& m : moradores_) {
        if (!m.vivo) continue;
        if (m.casa != SEM_EDIFICIO && !cidade.edificio(m.casa).ativo) m.casa = SEM_EDIFICIO;
    }

    // Quantos moram em cada edifício, contados uma vez. Perguntar
    // moradores_na_casa a cada casa examinada percorria a população inteira
    // por pergunta: sem-teto × casas × moradores por tick.
    std::vector<int> ocupacao(cidade.edificios().size(), 0);
    bool falta_casa = false;
    for (const Morador& m : moradores_) {
        if (!m.vivo) continue;
        if (m.casa == SEM_EDIFICIO) falta_casa = true;
        else ++ocupacao[m.casa];
    }
    if (!falta_casa) return;

    // As casas são ocupadas em ordem de id, e uma casa cheia continua cheia
    // até o fim desta chamada: cada sem-teto procura a partir de onde o
    // anterior parou.
    IdEdificio casa = 0;
    for (Morador& m : moradores_) {
        if (!m.vivo || m.casa != SEM_EDIFICIO) continue;
        while (casa < cidade.edificios().size() &&
               !(cidade.edificio(casa).ativo && cidade.definicao(casa).moradores > ocupacao[casa])) {
            ++casa;
        }
        if (casa == cidade.edificios().size()) break;
        m.casa = casa;
        ++ocupacao[casa];
    }
}

void Populacao::empregar(const Cidade& cidade) {
    // Recalcular em ordem de id torna o resultado independente do histórico
    // de construções e libera imediatamente quem perdeu casa ou trabalho.
    for (Morador& m : moradores_) m.trabalho = SEM_EDIFICIO;
    trabalhadores_.assign(cidade.edificios().size(), 0);
    // Os empregos são distribuídos em ordem de morador, então quem ficou
    // para trás do cursor já tem trabalho ou não pode trabalhar: o próximo
    // edifício continua de onde o anterior parou, sem voltar ao começo.
    std::size_t proximo = 0;
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.edificio(id).ativo) continue;
        const int vagas = cidade.definicao(id).trabalhadores;
        int ocupadas = 0;
        for (; ocupadas < vagas && proximo < moradores_.size(); ++proximo) {
            Morador& m = moradores_[proximo];
            if (m.vivo && m.adulto() && m.casa != SEM_EDIFICIO) {
                m.trabalho = id;
                ++ocupadas;
            }
        }
        trabalhadores_[id] = ocupadas;
    }
}

void Populacao::avancar_dia(Cidade& cidade, Data data) {
    for (Morador& m : moradores_) if (m.vivo) ++m.idade_dias;
    if (data.mes != 1 || data.dia != 1) return;

    std::vector<IdEdificio> nascimentos;
    for (IdEdificio casa = 0; casa < cidade.edificios().size(); ++casa) {
        if (!cidade.edificio(casa).ativo) continue;
        const int capacidade = cidade.definicao(casa).moradores;
        if (capacidade == 0 || moradores_na_casa(casa) >= capacidade) continue;
        bool homem = false, mulher = false;
        for (const Morador& m : moradores_) {
            if (!m.vivo || !m.adulto() || m.casa != casa) continue;
            mulher = mulher || m.mulher;
            homem = homem || !m.mulher;
        }
        bool tem_comida = false;
        for (IdItem item = 0; item < cidade.conteudo().itens.size(); ++item)
            tem_comida = tem_comida || (cidade.conteudo().itens[item].tipo == TipoItem::Comida &&
                                        cidade.edificio(casa).estoque[item] > 0);
        if (homem && mulher && tem_comida) nascimentos.push_back(casa);
    }
    for (IdEdificio casa : nascimentos) {
        Morador bebe;
        bebe.casa = casa;
        bebe.idade_dias = 0;
        bebe.mulher = moradores_.size() % 2 != 0;
        moradores_.push_back(bebe);
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
            if (m.trabalho != SEM_EDIFICIO) --trabalhadores_[m.trabalho];
            m.trabalho = SEM_EDIFICIO;
            ++mortos_de_fome_;
        }
    }
}
