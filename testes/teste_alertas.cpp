#include "alertas.hpp"
#include "conteudo_de_teste.hpp"
#include "verificar.hpp"

namespace {

Simulacao vila_vazia(int moradores) {
    Simulacao sim(Cidade(Mapa(12, 8), conteudo_de_teste()));
    sim.populacao = Populacao(moradores);
    return sim;
}

const Alerta* achar(const std::vector<Alerta>& alertas, Alerta::Tipo tipo) {
    for (const Alerta& a : alertas)
        if (a.tipo == tipo) return &a;
    return nullptr;
}

}  // namespace

TESTE("alertas: pouca comida é grave e vem primeiro; sem comida conta zero dias") {
    Simulacao sim = vila_vazia(6);
    const std::vector<Alerta> a = levantar_alertas(sim);
    VERIFICAR(!a.empty() && a.front().tipo == Alerta::Tipo::Fome);
    VERIFICAR(!a.empty() && a.front().grave && a.front().texto == "comida para 0 dias");
    const Alerta* sem_casa = achar(a, Alerta::Tipo::SemCasa);
    VERIFICAR(sem_casa && sem_casa->texto == "6 moradores sem casa" && !sem_casa->grave);
}

TESTE("alertas: a comida usa os mesmos limites da barra de cima") {
    Simulacao sim = vila_vazia(1);
    // Nove depósitos de 10: 90 porções para 1 morador, exatamente o limite.
    std::vector<IdEdificio> depositos;
    for (int x = 1; x <= 9; ++x) depositos.push_back(*sim.cidade.construir_edificio(tipo("deposito"), { x, 1 }));
    sim.cidade.adicionar(depositos[0], item("pao"), 10);
    const std::vector<Alerta> antes = levantar_alertas(sim);
    const Alerta* fome = achar(antes, Alerta::Tipo::Fome);
    VERIFICAR(fome && fome->grave && fome->texto == "comida para 10 dias");

    for (const IdEdificio d : depositos) sim.cidade.adicionar(d, item("pao"), 10 - sim.cidade.edificio(d).estoque[item("pao")]);
    const std::vector<Alerta> depois = levantar_alertas(sim);
    VERIFICAR(achar(depois, Alerta::Tipo::Fome) == nullptr);
}

TESTE("alertas: edifício sem estrada só aparece nesse alerta, com o id para o clique") {
    Simulacao sim = vila_vazia(0);
    const IdEdificio serraria = *sim.cidade.construir_edificio(tipo("serraria"), { 3, 3 });
    const std::vector<Alerta> a = levantar_alertas(sim);
    const Alerta* sem_estrada = achar(a, Alerta::Tipo::SemEstrada);
    VERIFICAR(sem_estrada && sem_estrada->grave);
    VERIFICAR(sem_estrada && sem_estrada->edificios == std::vector<IdEdificio>{ serraria });
    VERIFICAR(sem_estrada && sem_estrada->texto == "1 edifício sem estrada na entrada");
    VERIFICAR(achar(a, Alerta::Tipo::SemTrabalhadores) == nullptr);
}

TESTE("alertas: com estrada, aponta falta de trabalhador e depois falta de insumo") {
    Simulacao sim = vila_vazia(1);
    const IdEdificio serraria = *sim.cidade.construir_edificio(tipo("serraria"), { 3, 3 });
    sim.cidade.construir_estrada(sim.cidade.porta(serraria));
    const std::vector<Alerta> sem_ninguem = levantar_alertas(sim);
    const Alerta* sem_trabalho = achar(sem_ninguem, Alerta::Tipo::SemTrabalhadores);
    VERIFICAR(sem_trabalho && sem_trabalho->edificios == std::vector<IdEdificio>{ serraria });

    sim.cidade.construir_edificio(tipo("casa"), { 6, 1 });
    sim.populacao.alojar(sim.cidade);
    sim.populacao.empregar(sim.cidade);
    const std::vector<Alerta> a = levantar_alertas(sim);
    VERIFICAR(achar(a, Alerta::Tipo::SemTrabalhadores) == nullptr);
    const Alerta* falta = achar(a, Alerta::Tipo::FaltaEntrada);
    VERIFICAR(falta && falta->texto == "1 edifício parado por falta de insumo");
}

TESTE("alertas: graves antes dos avisos, sem trocar a ordem dentro de cada grupo") {
    Simulacao sim = vila_vazia(2);
    sim.cidade.construir_edificio(tipo("serraria"), { 3, 3 });  // sem estrada: grave
    const std::vector<Alerta> a = levantar_alertas(sim);
    bool viu_aviso = false, grave_depois_de_aviso = false;
    for (const Alerta& x : a) {
        if (!x.grave) viu_aviso = true;
        else if (viu_aviso) grave_depois_de_aviso = true;
    }
    VERIFICAR(!grave_depois_de_aviso);
    VERIFICAR(a.size() >= 2 && a[0].tipo == Alerta::Tipo::Fome && a[1].tipo == Alerta::Tipo::SemEstrada);
}
