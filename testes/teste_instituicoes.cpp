#include "campanha.hpp"
#include "cidade.hpp"
#include "conteudo.hpp"
#include "salvamento.hpp"
#include "verificar.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

constexpr const char* ITENS = R"(
[[item]]
id = "tora"
nome = "tora"
tipo = "insumo"

[[item]]
id = "tijolo"
nome = "tijolo"
tipo = "insumo"
)";

constexpr const char* EDIFICIOS = R"(
[[edificio]]
id = "deposito"
nome = "depósito"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
estoque = 100
deposito = true

[[edificio]]
id = "igreja"
nome = "igreja"
fase = "vila"
instituicao = "igreja"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
custo = { tora = 1 }

[[edificio.melhoria]]
fase = "mercado"
custo = { tora = 2 }

[[edificio.melhoria]]
fase = "hansa"
custo = { tijolo = 3 }
)";

}  // namespace

TESTE("instituições: definição carrega níveis, fases e custos") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, EDIFICIOS);
    VERIFICAR(r.conteudo.has_value());
    if (!r.conteudo) return;
    const DefinicaoEdificio& igreja = r.conteudo->edificio(*r.conteudo->buscar_edificio("igreja"));
    VERIFICAR(igreja.instituicao == std::optional<std::string>("igreja"));
    VERIFICAR(nivel_maximo_disponivel(igreja, Fase::Vila) == 1);
    VERIFICAR(nivel_maximo_disponivel(igreja, Fase::Mercado) == 2);
    VERIFICAR(nivel_maximo_disponivel(igreja, Fase::Hansa) == 3);
}

TESTE("instituições: melhorias consomem materiais e respeitam fase e nível") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, EDIFICIOS);
    VERIFICAR(r.conteudo.has_value());
    if (!r.conteudo) return;
    const Conteudo& conteudo = *r.conteudo;
    Cidade cidade(Mapa(10, 10), conteudo);
    const IdEdificio deposito = *cidade.construir_edificio(*conteudo.buscar_edificio("deposito"), { 1, 1 });
    cidade.adicionar(deposito, *conteudo.buscar_item("tora"), 5);
    cidade.adicionar(deposito, *conteudo.buscar_item("tijolo"), 3);
    const IdEdificio igreja = *cidade.construir_edificio(*conteudo.buscar_edificio("igreja"), { 4, 1 });

    VERIFICAR(cidade.edificio(igreja).nivel == 1);
    VERIFICAR(cidade.validar_melhoria(igreja, Fase::Vila) == ResultadoMelhoria::FaseBloqueada);
    VERIFICAR(cidade.validar_edificio(*conteudo.buscar_edificio("igreja"), { 7, 1 }) == Validacao::JaExiste);
    VERIFICAR(cidade.melhorar_edificio(igreja, Fase::Mercado) == ResultadoMelhoria::Ok);
    VERIFICAR(cidade.edificio(igreja).nivel == 2);
    VERIFICAR(cidade.edificio(deposito).estoque[*conteudo.buscar_item("tora")] == 2);
    VERIFICAR(cidade.melhorar_edificio(igreja, Fase::Hansa) == ResultadoMelhoria::Ok);
    VERIFICAR(cidade.edificio(igreja).nivel == 3);
    VERIFICAR(cidade.validar_melhoria(igreja, Fase::Auge) == ResultadoMelhoria::NivelMaximo);
}

TESTE("instituições: requeridas mas ainda não construídas bloqueiam progressão") {
    const ResultadoCarga r = carregar_conteudo("dados");
    VERIFICAR(r.conteudo.has_value());
    if (!r.conteudo) return;
    Cidade cidade(Mapa(8, 8), *r.conteudo);
    const auto estados = estados_das_instituicoes(cidade, Fase::Vila);
    VERIFICAR(estados.size() == 2);
    VERIFICAR(!estados.empty() && estados[0].nivel == 0);
    VERIFICAR(!tentar_avancar_fase(Fase::Vila, { 1200, 1, 1 }, estados).avancou);
}

TESTE("campanha: simulação avança no ano se as instituições da fase estiverem prontas") {
    const ResultadoCarga r = carregar_conteudo("dados");
    VERIFICAR(r.conteudo.has_value());
    if (!r.conteudo) return;
    const Conteudo& conteudo = *r.conteudo;
    Cidade cidade(Mapa(24, 20), conteudo);
    const IdEdificio deposito = *cidade.construir_edificio(*conteudo.buscar_edificio("armazem"), { 1, 1 });
    cidade.adicionar(deposito, *conteudo.buscar_item("tora"), 100);
    VERIFICAR(cidade.construir_edificio(*conteudo.buscar_edificio("igreja"), { 6, 1 }).has_value());
    VERIFICAR(cidade.construir_edificio(*conteudo.buscar_edificio("governo"), { 10, 1 }).has_value());
    Simulacao sim(std::move(cidade));
    sim.data = { 1199, 12, 31 };
    sim.tick = Simulacao::TICKS_POR_DIA - 1;
    sim.passo();
    const Data dia_esperado{ 1200, 1, 1 };
    VERIFICAR(sim.data == dia_esperado);
    VERIFICAR(sim.fase == Fase::Mercado);
}

TESTE("instituições: salvamento preserva fase e níveis") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, EDIFICIOS);
    VERIFICAR(r.conteudo.has_value());
    if (!r.conteudo) return;
    const Conteudo& conteudo = *r.conteudo;
    Cidade cidade(Mapa(10, 10), conteudo);
    const IdEdificio deposito = *cidade.construir_edificio(*conteudo.buscar_edificio("deposito"), { 1, 1 });
    cidade.adicionar(deposito, *conteudo.buscar_item("tora"), 10);
    const IdEdificio igreja = *cidade.construir_edificio(*conteudo.buscar_edificio("igreja"), { 4, 1 });
    VERIFICAR(cidade.melhorar_edificio(igreja, Fase::Mercado) == ResultadoMelhoria::Ok);
    Simulacao sim(std::move(cidade));
    sim.fase = Fase::Mercado;

    const std::string caminho = "/tmp/hansestadt-instituicao.hanse";
    std::string erro;
    VERIFICAR(salvar_partida(sim, caminho, erro));
    const ResultadoSalvamento carga = carregar_partida(caminho, conteudo);
    VERIFICAR(carga.simulacao.has_value());
    if (carga.simulacao) {
        VERIFICAR(carga.simulacao->fase == Fase::Mercado);
        VERIFICAR(carga.simulacao->cidade.edificio(igreja).nivel == 2);
    }
    std::filesystem::remove(caminho);
}

TESTE("salvamento: versões 3 a 5 migram para o formato atual") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, EDIFICIOS);
    if (!r.conteudo) { VERIFICAR(false); return; }
    Simulacao sim(Cidade(Mapa(2, 2), *r.conteudo));
    sim.fase = Fase::Hansa;
    sim.populacao = Populacao::restaurar({}, 0, 0, 123);
    const std::string caminho = "/tmp/hansestadt-teste-v3.hanse";
    const std::string caminho_legado = "/tmp/hansestadt-teste-v3-legado.hanse";
    const std::string caminho_v4 = "/tmp/hansestadt-teste-v4-legado.hanse";
    std::string erro;
    VERIFICAR(salvar_partida(sim, caminho, erro));
    const ResultadoSalvamento carga_v6 = carregar_partida(caminho, *r.conteudo);
    VERIFICAR(carga_v6.simulacao.has_value());
    if (carga_v6.simulacao)
        VERIFICAR(carga_v6.simulacao->populacao.mortos_por_eventos() == 123);
    std::ifstream entrada(caminho, std::ios::binary);
    std::string bytes{ std::istreambuf_iterator<char>(entrada), std::istreambuf_iterator<char>() };
    entrada.close();
    // V6 acrescentou, no offset 73 deste save mínimo, um vetor com um nível
    // de produtividade por tipo. Removê-lo produz um V5 legítimo.
    constexpr std::size_t OFFSET_PRODUTIVIDADE = 73;
    const std::size_t celulas = 4;
    const std::size_t bytes_novos = sizeof(std::uint64_t) + r.conteudo->edificios.size() * sizeof(int) +
                                    sizeof(std::uint64_t) + celulas * sizeof(std::uint8_t) +
                                    sizeof(std::uint64_t) + celulas * sizeof(std::uint16_t) +
                                    sizeof(std::uint64_t) + celulas * sizeof(std::uint8_t);
    bytes.erase(OFFSET_PRODUTIVIDADE, bytes_novos);
    bytes[10] = '\5';
    const std::string caminho_v5 = "/tmp/hansestadt-teste-v5-legado.hanse";
    std::ofstream saida_v5(caminho_v5, std::ios::binary | std::ios::trunc);
    saida_v5.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    saida_v5.close();
    const ResultadoSalvamento carga_v5 = carregar_partida(caminho_v5, *r.conteudo);
    VERIFICAR(carga_v5.simulacao.has_value());
    if (carga_v5.simulacao)
        VERIFICAR(carga_v5.simulacao->cidade.nivel_produtividade(0) == 0);

    // O save mínimo tem dois contadores de população antes do contador de
    // mortes por evento, cujo offset é 89 bytes (sem edifícios/moradores).
    constexpr std::size_t OFFSET_MORTES_EVENTOS = 89;
    std::string bytes_v4 = bytes;
    bytes_v4.erase(OFFSET_MORTES_EVENTOS, sizeof(int));
    bytes_v4[10] = '\4';
    std::ofstream saida_v4(caminho_v4, std::ios::binary | std::ios::trunc);
    saida_v4.write(bytes_v4.data(), static_cast<std::streamsize>(bytes_v4.size()));
    saida_v4.close();
    const ResultadoSalvamento carga_v4 = carregar_partida(caminho_v4, *r.conteudo);
    VERIFICAR(carga_v4.simulacao.has_value());
    if (carga_v4.simulacao) {
        VERIFICAR(carga_v4.simulacao->fase == Fase::Hansa);
        VERIFICAR(carga_v4.simulacao->populacao.mortos_por_eventos() == 0);
    }

    bytes.erase(OFFSET_MORTES_EVENTOS, sizeof(int));
    bytes.erase(32, 1);  // v3 ainda não tinha a fase persistida.
    bytes[10] = '\3';
    std::ofstream saida(caminho_legado, std::ios::binary | std::ios::trunc);
    saida.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    saida.close();
    const ResultadoSalvamento carga = carregar_partida(caminho_legado, *r.conteudo);
    VERIFICAR(carga.simulacao.has_value());
    if (carga.simulacao) VERIFICAR(carga.simulacao->fase == Fase::Vila);
    if (carga.simulacao) VERIFICAR(carga.simulacao->populacao.mortos_por_eventos() == 0);
    std::filesystem::remove(caminho);
    std::filesystem::remove(caminho_legado);
    std::filesystem::remove(caminho_v4);
    std::filesystem::remove(caminho_v5);
}
