#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "edificios.hpp"
#include "mapa.hpp"

// Índice no vetor de edifícios. Não é reaproveitado depois de uma demolição:
// quem guardar um id (um carregador indo entregar, por exemplo) encontra o
// edifício marcado como inativo em vez de outro edifício no mesmo lugar.
using IdEdificio = std::uint32_t;
inline constexpr IdEdificio SEM_EDIFICIO = std::numeric_limits<IdEdificio>::max();

struct Edificio {
    TipoEdificio tipo;
    Celula origem;  // canto superior esquerdo
    bool ativo;
};

enum class Validacao : std::uint8_t {
    Ok,
    ForaDoMapa,
    TerrenoErrado,
    Ocupado,
    JaExiste,  // estrada sobre estrada: não é erro, só não muda nada
};

const char* descrever(Validacao v);

// Tudo o que foi construído sobre o mapa. Não depende da raylib, para poder
// ser testado sem janela.
class Cidade {
public:
    explicit Cidade(Mapa mapa);

    const Mapa& mapa() const { return mapa_; }
    const std::vector<Edificio>& edificios() const { return edificios_; }

    Validacao validar_edificio(TipoEdificio tipo, Celula origem) const;
    std::optional<IdEdificio> construir_edificio(TipoEdificio tipo, Celula origem);

    Validacao validar_estrada(Celula c) const;
    bool construir_estrada(Celula c);

    // Remove o edifício ou a estrada na célula. Devolve false se não havia nada.
    bool demolir(Celula c);

    bool tem_estrada(Celula c) const;
    IdEdificio edificio_em(Celula c) const;

private:
    std::size_t indice(Celula c) const {
        return static_cast<std::size_t>(c.y) * static_cast<std::size_t>(mapa_.largura()) +
               static_cast<std::size_t>(c.x);
    }

    Mapa mapa_;
    std::vector<Edificio> edificios_;
    // Uma entrada por célula, na mesma ordem do mapa.
    std::vector<IdEdificio> ocupacao_;
    std::vector<std::uint8_t> estradas_;
};

// Caminho que a ferramenta de estrada desenha ao arrastar: primeiro na
// horizontal a partir de `inicio`, depois na vertical até `fim`.
std::vector<Celula> caminho_em_l(Celula inicio, Celula fim);
