#include "transporte.hpp"

#include <limits>

Transporte Transporte::restaurar(std::vector<Carregador> carregadores,
                                  std::vector<std::uint8_t> depositos_criados, std::uint64_t tick,
                                  std::uint64_t entregas, std::uint64_t perdidos) {
    Transporte t;
    t.carregadores_ = std::move(carregadores);
    t.ja_criou_ = std::move(depositos_criados);
    t.tick_ = tick;
    t.entregas_ = entregas;
    t.perdidos_ = perdidos;
    return t;
}

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

IdEdificio Transporte::origem_mais_proxima(const Cidade& cidade, IdItem item, IdEdificio destino) {
    IdEdificio melhor = SEM_EDIFICIO;
    std::size_t menor = std::numeric_limits<std::size_t>::max();
    // A busca só é feita quando aparece o primeiro candidato: a maior parte
    // das vagas pede um item que ninguém tem, e aí não há o que medir.
    bool mediu = false;
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (id == destino || !cidade.tem_acesso(id) || cidade.disponivel(id, item) <= 0) continue;
        const DefinicaoEdificio& def = cidade.definicao(id);
        // Do produtor só sai o que ele produz; a entrada dele é dele.
        bool oferece = def.deposito;
        for (const Receita& r : def.receitas)
            for (const Quantidade& q : r.saidas) oferece = oferece || q.item == item;
        if (!oferece) continue;
        if (!mediu) {
            perto_do_destino_.calcular(cidade, cidade.porta(destino));
            mediu = true;
        }
        const int d = perto_do_destino_.ate(cidade.porta(id));
        if (d != DistanciasPelaEstrada::SEM_LIGACAO && static_cast<std::size_t>(d) < menor) {
            menor = static_cast<std::size_t>(d);
            melhor = id;
        }
    }
    return melhor;
}

IdEdificio Transporte::deposito_mais_proximo(const Cidade& cidade, IdItem item, Celula de) {
    IdEdificio melhor = SEM_EDIFICIO;
    std::size_t menor = std::numeric_limits<std::size_t>::max();
    bool mediu = false;
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        if (!cidade.definicao(id).deposito || !cidade.tem_acesso(id) || cidade.espaco_livre(id, item) <= 0) continue;
        if (!mediu) {
            perto_da_origem_.calcular(cidade, de);
            mediu = true;
        }
        const int d = perto_da_origem_.ate(cidade.porta(id));
        if (d != DistanciasPelaEstrada::SEM_LIGACAO && static_cast<std::size_t>(d) < menor) {
            menor = static_cast<std::size_t>(d);
            melhor = id;
        }
    }
    return melhor;
}

Carregador* Transporte::carregador_mais_proximo(const Cidade& cidade, Celula alvo) {
    Carregador* melhor = nullptr;
    std::size_t menor = std::numeric_limits<std::size_t>::max();
    bool mediu = false;
    for (Carregador& c : carregadores_) {
        if (!c.livre()) continue;
        if (!mediu) {
            perto_da_origem_.calcular(cidade, alvo);
            mediu = true;
        }
        const int d = perto_da_origem_.ate(c.posicao);
        if (d != DistanciasPelaEstrada::SEM_LIGACAO && static_cast<std::size_t>(d) < menor) {
            menor = static_cast<std::size_t>(d);
            melhor = &c;
        }
    }
    return melhor;
}

void Transporte::atribuir(Cidade& cidade, Carregador& c, IdEdificio origem, IdEdificio destino, IdItem item) {
    cidade.reservar_saida(origem, item);
    cidade.reservar_chegada(destino, item);
    c.tarefa = Tarefa::Buscar;
    c.origem = origem;
    c.destino = destino;
    c.item = item;
    enviar(cidade, c, cidade.porta(origem));
    // Já estava na porta da origem (um carregador parado em casa buscando
    // no próprio depósito): pega o item agora.
    if (!c.andando()) chegou(cidade, c);
}

// Devolve false quando não há carregador livre que alcance a origem: não
// adianta continuar procurando pedidos nesta rodada.
bool Transporte::atender_demandas(Cidade& cidade) {
    const Conteudo& conteudo = cidade.conteudo();
    for (IdEdificio destino = 0; destino < cidade.edificios().size(); ++destino) {
        if (!cidade.tem_acesso(destino)) continue;
        const DefinicaoEdificio& def = cidade.definicao(destino);
        for (IdItem item = 0; item < conteudo.itens.size(); ++item) {
            bool consome = false;
            if (def.moradores > 0 && conteudo.itens[item].tipo == TipoItem::Comida) consome = true;
            for (const Receita& r : def.receitas)
                for (const Quantidade& q : r.entradas) consome = consome || q.item == item;
            if (!consome) continue;
            while (cidade.espaco_livre(destino, item) > 0) {
                const IdEdificio origem = origem_mais_proxima(cidade, item, destino);
                if (origem == SEM_EDIFICIO) break;
                Carregador* c = carregador_mais_proximo(cidade, cidade.porta(origem));
                if (!c) return false;
                atribuir(cidade, *c, origem, destino, item);
            }
        }
    }
    return true;
}

bool Transporte::escoar_sobras(Cidade& cidade) {
    for (IdEdificio origem = 0; origem < cidade.edificios().size(); ++origem) {
        if (!cidade.tem_acesso(origem)) continue;
        const DefinicaoEdificio& def = cidade.definicao(origem);
        for (const Receita& r : def.receitas) {
            for (const Quantidade& q : r.saidas) {
                while (cidade.disponivel(origem, q.item) > 0) {
                    const IdEdificio destino = deposito_mais_proximo(cidade, q.item, cidade.porta(origem));
                    if (destino == SEM_EDIFICIO) break;
                    Carregador* c = carregador_mais_proximo(cidade, cidade.porta(origem));
                    if (!c) return false;
                    atribuir(cidade, *c, origem, destino, q.item);
                }
            }
        }
    }
    return true;
}

void Transporte::planejar(Cidade& cidade) {
    // Demandas antes das sobras: a farinha vai direto do moinho para a
    // padaria quando ela tem vaga, sem passar pelo armazém.
    if (atender_demandas(cidade)) escoar_sobras(cidade);
}

void Transporte::cancelar(Cidade& cidade, Carregador& c) {
    switch (c.tarefa) {
        case Tarefa::Nenhuma:
            return;
        case Tarefa::Buscar:
            cidade.liberar_saida(c.origem, c.item);
            cidade.liberar_chegada(c.destino, c.item);
            break;
        case Tarefa::Entregar:
            cidade.liberar_chegada(c.destino, c.item);
            ++perdidos_;
            break;
    }
    c.carregando = false;
    c.tarefa = Tarefa::Nenhuma;
}

void Transporte::redirecionar(Cidade& cidade, Carregador& c) {
    cidade.liberar_chegada(c.destino, c.item);
    const IdEdificio deposito = deposito_mais_proximo(cidade, c.item, c.posicao);
    if (deposito == SEM_EDIFICIO || deposito == c.destino) {
        ++perdidos_;
        c.carregando = false;
        c.tarefa = Tarefa::Nenhuma;
        return;
    }
    cidade.reservar_chegada(deposito, c.item);
    c.destino = deposito;
    enviar(cidade, c, cidade.porta(deposito));
    if (!c.andando()) chegou(cidade, c);
}

void Transporte::conferir_tarefa(Cidade& cidade, Carregador& c) {
    if (c.tarefa == Tarefa::Buscar && !cidade.edificio(c.origem).ativo) {
        cancelar(cidade, c);  // ainda não pegou nada: não há o que perder
        c.rota.clear();
        c.passo = 0;
    } else if (c.tarefa == Tarefa::Entregar && !cidade.edificio(c.destino).ativo) {
        redirecionar(cidade, c);
    }
}

void Transporte::chegou(Cidade& cidade, Carregador& c) {
    c.rota.clear();
    c.passo = 0;

    switch (c.tarefa) {
        case Tarefa::Nenhuma:
            break;
        case Tarefa::Buscar:
            cidade.liberar_saida(c.origem, c.item);
            if (cidade.retirar(c.origem, c.item, 1)) {
                c.carregando = true;
                c.tarefa = Tarefa::Entregar;
                if (!cidade.edificio(c.destino).ativo || !enviar(cidade, c, cidade.porta(c.destino))) {
                    redirecionar(cidade, c);
                } else if (!c.andando()) {
                    chegou(cidade, c);
                }
            } else {
                cidade.liberar_chegada(c.destino, c.item);
                c.tarefa = Tarefa::Nenhuma;
            }
            break;
        case Tarefa::Entregar:
            if (!cidade.adicionar(c.destino, c.item, 1)) {
                // Lotou por outro caminho (produção, entrega manual):
                // procura vaga num depósito.
                redirecionar(cidade, c);
                break;
            }
            cidade.liberar_chegada(c.destino, c.item);
            ++entregas_;
            c.carregando = false;
            c.tarefa = Tarefa::Nenhuma;
            break;
    }
}

void Transporte::mover(Cidade& cidade, Carregador& c) {
    if (!c.andando()) return;
    if (++c.ticks_na_celula < TICKS_POR_CELULA) return;
    c.ticks_na_celula = 0;

    const Celula proxima = c.rota[c.passo];
    if (!cidade.tem_estrada(proxima)) {
        // Demoliram a estrada à frente: recalcula até o mesmo alvo. Se não
        // houver outro caminho, para onde está e desiste da tarefa.
        const Celula alvo = c.rota.back();
        if (!enviar(cidade, c, alvo)) {
            c.rota.clear();
            c.passo = 0;
            if (c.tarefa == Tarefa::Entregar) redirecionar(cidade, c);
            else cancelar(cidade, c);
        }
        return;
    }
    c.posicao = proxima;
    if (++c.passo == c.rota.size()) chegou(cidade, c);
}

void Transporte::passo(Cidade& cidade) {
    criar_carregadores(cidade);
    if (tick_++ % PLANEJAR_A_CADA == 0) planejar(cidade);
    for (Carregador& c : carregadores_) {
        conferir_tarefa(cidade, c);
        if (!cidade.edificio(c.casa).ativo) mudar_de_casa(cidade, c);
        // Livre e longe de casa: volta.
        if (c.livre() && !c.andando() && !(c.posicao == cidade.porta(c.casa))) {
            enviar(cidade, c, cidade.porta(c.casa));
        }
        mover(cidade, c);
    }
}
