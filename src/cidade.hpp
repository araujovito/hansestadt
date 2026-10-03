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
    // Reservas do transporte, também por IdItem: itens do estoque já
    // prometidos a um carregador que vem buscar, e itens que um carregador
    // está trazendo e ainda não chegaram.
    std::vector<int> reservado;
    std::vector<int> a_caminho;
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
    PortaBloqueada,  // a entrada cairia em água ou floresta
    JaExiste,  // estrada sobre estrada: não é erro, só não muda nada
};

const char* descrever(Validacao v);

// Entrada do edifício: a célula logo abaixo do meio da borda inferior da área
// total. Fica fora do edifício, para uma estrada poder passar por ela; é por
// onde os carregadores entram e saem.
Celula porta_de(const DefinicaoEdificio& def, Celula origem);

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
    // Muda a cada estrada construída ou demolida; quem guarda rotas compara
    // com o valor da época para saber se elas ainda valem.
    std::uint64_t versao_estradas() const { return versao_estradas_; }

    Celula porta(IdEdificio id) const;
    // Se há estrada na porta. Sem isso nenhum carregador chega ao edifício.
    bool tem_acesso(IdEdificio id) const;

    Edificio& edificio(IdEdificio id) { return edificios_[id]; }
    const Edificio& edificio(IdEdificio id) const { return edificios_[id]; }
    const DefinicaoEdificio& definicao(IdEdificio id) const { return conteudo_->edificio(edificios_[id].tipo); }

    // Quanto do item ainda cabe no edifício (0 se ele não guarda esse item).
    int espaco_para(IdEdificio id, IdItem item) const;
    // Para planejar transporte: o que pode ser prometido a um carregador
    // (estoque menos o já reservado) e quanto ainda cabe descontando o que
    // está a caminho.
    int disponivel(IdEdificio id, IdItem item) const;
    int espaco_livre(IdEdificio id, IdItem item) const;
    void reservar_saida(IdEdificio id, IdItem item) { ++edificios_[id].reservado[item]; }
    void liberar_saida(IdEdificio id, IdItem item) { --edificios_[id].reservado[item]; }
    void reservar_chegada(IdEdificio id, IdItem item) { ++edificios_[id].a_caminho[item]; }
    void liberar_chegada(IdEdificio id, IdItem item) { --edificios_[id].a_caminho[item]; }
    // Falham sem mudar nada se não houver espaço ou quantidade suficiente.
    bool adicionar(IdEdificio id, IdItem item, int quantidade);
    bool retirar(IdEdificio id, IdItem item, int quantidade);
    int estoque_total(IdItem item) const;
    int estoque_total(TipoItem tipo) const;

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
    // Células que são a porta de algum edifício. Basta uma marca: dois
    // edifícios nunca dividem a porta, porque a célula logo acima dela é
    // sempre do próprio edifício.
    std::vector<std::uint8_t> portas_;
    std::uint64_t versao_estradas_ = 0;
};

// Caminho que a ferramenta de estrada desenha ao arrastar: primeiro na
// horizontal a partir de `inicio`, depois na vertical até `fim`.
std::vector<Celula> caminho_em_l(Celula inicio, Celula fim);
