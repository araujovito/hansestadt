#include <filesystem>

#include "conteudo_de_teste.hpp"
#include "salvamento.hpp"
#include "verificar.hpp"

TESTE("salvamento: restaura o estado e continua deterministicamente") {
    Cidade cidade(Mapa(12, 8), conteudo_de_teste());
    const IdEdificio deposito = *cidade.construir_edificio(tipo("deposito"), { 1, 1 });
    const IdEdificio casa = *cidade.construir_edificio(tipo("casa"), { 4, 1 });
    cidade.construir_estrada(cidade.porta(deposito));
    for (const Celula c : caminho_em_l(cidade.porta(deposito), cidade.porta(casa))) cidade.construir_estrada(c);
    cidade.adicionar(deposito, item("pao"), 6);
    Simulacao original(std::move(cidade));
    for (int i = 0; i < 37; ++i) original.passo();

    const std::string caminho = "/tmp/hansestadt-teste.hanse";
    std::string erro;
    VERIFICAR(salvar_partida(original, caminho, erro));
    ResultadoSalvamento resultado = carregar_partida(caminho, conteudo_de_teste());
    VERIFICAR(resultado.simulacao.has_value());
    if (!resultado.simulacao) return;
    Simulacao restaurada = std::move(*resultado.simulacao);
    VERIFICAR(restaurada.tick == original.tick);
    VERIFICAR(restaurada.data == original.data);
    VERIFICAR(restaurada.populacao.total() == original.populacao.total());
    VERIFICAR(restaurada.cidade.estoque_total(item("pao")) == original.cidade.estoque_total(item("pao")));

    for (int i = 0; i < 100; ++i) { original.passo(); restaurada.passo(); }
    VERIFICAR(restaurada.data == original.data);
    VERIFICAR(restaurada.populacao.refeicoes_perdidas() == original.populacao.refeicoes_perdidas());
    VERIFICAR(restaurada.cidade.estoque_total(item("pao")) == original.cidade.estoque_total(item("pao")));
    std::filesystem::remove(caminho);
}
