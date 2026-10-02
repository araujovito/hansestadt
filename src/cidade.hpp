#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "conteudo.hpp"
#include "mapa.hpp"

// Índice no vetor de edifícios. Não é reaproveitado depois de uma demolição:
// quem guardar um id (um carregador indo entregar, por exemplo) encontra o
// edifício marcado como inativo em vez de outro edifício no mesmo lugar.
using IdEdificio = std::uint32_t;
inline constexpr IdEdificio SEM_EDIFICIO = std::numeric_limits<IdEdificio>::max();

struct Edificio {
    IdTipoEdificio tipo;
    Celula origem;  // canto superior esquerdo
    bool ativo;
    // Quantidade guardada de cada item, indexada por IdItem.
    std::vector<int> estoque;
    // Receita em andamento (índice em DefinicaoEdificio::receitas) ou -1
    // parado, e quanto do ciclo já correu (ver producao.hpp).
    int receita_ativa = -1;
    std::int64_t progresso = 0;
};

enum class Validacao : std::uint8_t {
    Ok,
    ForaDoMapa,
    TerrenoErrado,
    Ocupado,
    LongeDoRecurso,  // faltou o terreno exigido em `requer` por perto
    JaExiste,  // estrada sobre estrada: não é erro, só não muda nada
};

const char* descrever(Validacao v);

// Tudo o que foi construído sobre o mapa. Não depende da raylib, para poder
// ser testado sem janela.
class Cidade {
public:
    // O conteúdo não é copiado: precisa existir enquanto a cidade existir.
    Cidade(Mapa mapa, const Conteudo& conteudo);

    const Mapa& mapa() const { return mapa_; }
    const Conteudo& conteudo() const { return *conteudo_; }
    const std::vector<Edificio>& edificios() const { return edificios_; }

    Validacao validar_edificio(IdTipoEdificio tipo, Celula origem) const;
    std::optional<IdEdificio> construir_edificio(IdTipoEdificio tipo, Celula origem);

    Validacao validar_estrada(Celula c) const;
    bool construir_estrada(Celula c);

    // Remove o edifício ou a estrada na célula. Devolve false se não havia nada.
    bool demolir(Celula c);

    bool tem_estrada(Celula c) const;
    IdEdificio edificio_em(Celula c) const;

    Edificio& edificio(IdEdificio id) { return edificios_[id]; }
    const Edificio& edificio(IdEdificio id) const { return edificios_[id]; }
    const DefinicaoEdificio& definicao(IdEdificio id) const { return conteudo_->edificio(edificios_[id].tipo); }

    // Quanto do item ainda cabe no edifício (0 se ele não guarda esse item).
    int espaco_para(IdEdificio id, IdItem item) const;
    // Falham sem mudar nada se não houver espaço ou quantidade suficiente.
    bool adicionar(IdEdificio id, IdItem item, int quantidade);
    bool retirar(IdEdificio id, IdItem item, int quantidade);

private:
    bool atende_requisito(const Requisito& r, Celula origem, int largura, int altura) const;

    std::size_t indice(Celula c) const {
        return static_cast<std::size_t>(c.y) * static_cast<std::size_t>(mapa_.largura()) +
               static_cast<std::size_t>(c.x);
    }

    Mapa mapa_;
    const Conteudo* conteudo_;
    std::vector<Edificio> edificios_;
    // Uma entrada por célula, na mesma ordem do mapa.
    std::vector<IdEdificio> ocupacao_;
    std::vector<std::uint8_t> estradas_;
};

// Caminho que a ferramenta de estrada desenha ao arrastar: primeiro na
// horizontal a partir de `inicio`, depois na vertical até `fim`.
std::vector<Celula> caminho_em_l(Celula inicio, Celula fim);
