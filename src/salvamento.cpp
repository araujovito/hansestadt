#include "salvamento.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <type_traits>

namespace {

constexpr char MAGIA[] = "HANSESTADT\1";
constexpr std::uint64_t MAXIMO_VETOR = 10'000'000;

template <class T>
void escrever(std::ostream& out, T valor) {
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);
    out.write(reinterpret_cast<const char*>(&valor), sizeof(valor));
}

template <class T>
T ler(std::istream& in) {
    static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>);
    T valor{};
    if (!in.read(reinterpret_cast<char*>(&valor), sizeof(valor))) throw std::runtime_error("arquivo incompleto");
    return valor;
}

template <class T>
void escrever_vetor(std::ostream& out, const std::vector<T>& valores) {
    escrever(out, static_cast<std::uint64_t>(valores.size()));
    for (T valor : valores) escrever(out, valor);
}

template <class T>
std::vector<T> ler_vetor(std::istream& in) {
    const std::uint64_t tamanho = ler<std::uint64_t>(in);
    if (tamanho > MAXIMO_VETOR) throw std::runtime_error("vetor grande demais");
    std::vector<T> valores;
    valores.reserve(static_cast<std::size_t>(tamanho));
    for (std::uint64_t i = 0; i < tamanho; ++i) valores.push_back(ler<T>(in));
    return valores;
}

void escrever_celula(std::ostream& out, Celula c) { escrever(out, c.x); escrever(out, c.y); }
Celula ler_celula(std::istream& in) { return {ler<int>(in), ler<int>(in)}; }

void escrever_estado(std::ostream& out, const Simulacao& sim) {
    out.write(MAGIA, sizeof(MAGIA));
    escrever(out, sim.tick);
    escrever(out, sim.data.ano); escrever(out, sim.data.mes); escrever(out, sim.data.dia);

    const Mapa& mapa = sim.cidade.mapa();
    escrever(out, mapa.largura()); escrever(out, mapa.altura());
    for (int y = 0; y < mapa.altura(); ++y)
        for (int x = 0; x < mapa.largura(); ++x) escrever(out, mapa.terreno(x, y));

    escrever(out, static_cast<std::uint64_t>(sim.cidade.edificios().size()));
    for (const Edificio& e : sim.cidade.edificios()) {
        escrever(out, e.tipo); escrever_celula(out, e.origem); escrever(out, e.ativo);
        escrever_vetor(out, e.estoque); escrever_vetor(out, e.reservado); escrever_vetor(out, e.a_caminho);
        escrever(out, e.receita_ativa); escrever(out, e.progresso);
    }
    escrever_vetor(out, sim.cidade.estradas());
    escrever(out, sim.cidade.versao_estradas());

    escrever(out, static_cast<std::uint64_t>(sim.populacao.moradores().size()));
    for (const Morador& m : sim.populacao.moradores()) {
        escrever(out, m.casa); escrever(out, m.trabalho); escrever(out, m.dias_sem_comida); escrever(out, m.vivo);
    }
    escrever(out, sim.populacao.mortos_de_fome()); escrever(out, sim.populacao.refeicoes_perdidas());

    escrever(out, static_cast<std::uint64_t>(sim.transporte.carregadores().size()));
    for (const Carregador& c : sim.transporte.carregadores()) {
        escrever(out, c.casa); escrever_celula(out, c.posicao);
        escrever(out, static_cast<std::uint64_t>(c.rota.size()));
        for (Celula p : c.rota) escrever_celula(out, p);
        escrever(out, static_cast<std::uint64_t>(c.passo)); escrever(out, c.ticks_na_celula);
        escrever(out, c.tarefa); escrever(out, c.origem); escrever(out, c.destino); escrever(out, c.item);
        escrever(out, c.carregando);
    }
    escrever_vetor(out, sim.transporte.depositos_criados());
    escrever(out, sim.transporte.tick()); escrever(out, sim.transporte.entregas()); escrever(out, sim.transporte.perdidos());
}

Simulacao ler_estado(std::istream& in, const Conteudo& conteudo) {
    char magia[sizeof(MAGIA)]{};
    if (!in.read(magia, sizeof(magia)) || std::string(magia, sizeof(magia)) != std::string(MAGIA, sizeof(MAGIA)))
        throw std::runtime_error("formato ou versão incompatível");
    const auto tick = ler<std::uint64_t>(in);
    const Data data{ler<int>(in), ler<int>(in), ler<int>(in)};
    const int largura = ler<int>(in), altura = ler<int>(in);
    if (largura <= 0 || altura <= 0 || largura > 4096 || altura > 4096) throw std::runtime_error("mapa inválido");
    Mapa mapa(largura, altura);
    for (int y = 0; y < altura; ++y)
        for (int x = 0; x < largura; ++x) mapa.definir_terreno(x, y, ler<Terreno>(in));

    const std::uint64_t n_edificios = ler<std::uint64_t>(in);
    if (n_edificios > MAXIMO_VETOR) throw std::runtime_error("edifícios demais");
    std::vector<Edificio> edificios;
    for (std::uint64_t i = 0; i < n_edificios; ++i) {
        Edificio e;
        e.tipo = ler<IdTipoEdificio>(in); e.origem = ler_celula(in); e.ativo = ler<bool>(in);
        e.estoque = ler_vetor<int>(in); e.reservado = ler_vetor<int>(in); e.a_caminho = ler_vetor<int>(in);
        e.receita_ativa = ler<int>(in); e.progresso = ler<std::int64_t>(in);
        if (e.tipo >= conteudo.edificios.size() || e.estoque.size() != conteudo.itens.size())
            throw std::runtime_error("conteúdo incompatível");
        edificios.push_back(std::move(e));
    }
    auto estradas = ler_vetor<std::uint8_t>(in);
    const auto versao = ler<std::uint64_t>(in);
    Cidade cidade = Cidade::restaurar(std::move(mapa), conteudo, std::move(edificios), std::move(estradas), versao);
    Simulacao sim(std::move(cidade)); sim.tick = tick; sim.data = data;

    const std::uint64_t n_moradores = ler<std::uint64_t>(in);
    if (n_moradores > MAXIMO_VETOR) throw std::runtime_error("moradores demais");
    std::vector<Morador> moradores;
    for (std::uint64_t i = 0; i < n_moradores; ++i) {
        Morador m; m.casa = ler<IdEdificio>(in); m.trabalho = ler<IdEdificio>(in);
        m.dias_sem_comida = ler<int>(in); m.vivo = ler<bool>(in); moradores.push_back(m);
    }
    const int mortos = ler<int>(in);
    const int refeicoes_perdidas = ler<int>(in);
    sim.populacao = Populacao::restaurar(std::move(moradores), mortos, refeicoes_perdidas);

    const std::uint64_t n_carregadores = ler<std::uint64_t>(in);
    if (n_carregadores > MAXIMO_VETOR) throw std::runtime_error("carregadores demais");
    std::vector<Carregador> carregadores;
    for (std::uint64_t i = 0; i < n_carregadores; ++i) {
        Carregador c; c.casa = ler<IdEdificio>(in); c.posicao = ler_celula(in);
        const std::uint64_t n_rota = ler<std::uint64_t>(in);
        if (n_rota > MAXIMO_VETOR) throw std::runtime_error("rota grande demais");
        for (std::uint64_t j = 0; j < n_rota; ++j) c.rota.push_back(ler_celula(in));
        c.passo = static_cast<std::size_t>(ler<std::uint64_t>(in)); c.ticks_na_celula = ler<int>(in);
        c.tarefa = ler<Tarefa>(in); c.origem = ler<IdEdificio>(in); c.destino = ler<IdEdificio>(in);
        c.item = ler<IdItem>(in); c.carregando = ler<bool>(in); carregadores.push_back(std::move(c));
    }
    auto criados = ler_vetor<std::uint8_t>(in);
    const auto tick_transporte = ler<std::uint64_t>(in), entregas = ler<std::uint64_t>(in), perdidos = ler<std::uint64_t>(in);
    sim.transporte = Transporte::restaurar(std::move(carregadores), std::move(criados), tick_transporte, entregas, perdidos);
    return sim;
}

}  // namespace

bool salvar_partida(const Simulacao& simulacao, const std::string& caminho, std::string& erro) {
    try {
        const std::string temporario = caminho + ".tmp";
        std::ofstream out(temporario, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("não foi possível abrir o arquivo");
        escrever_estado(out, simulacao); out.close();
        if (!out) throw std::runtime_error("falha ao gravar o arquivo");
        std::filesystem::rename(temporario, caminho);
        erro.clear(); return true;
    } catch (const std::exception& e) { erro = e.what(); return false; }
}

ResultadoSalvamento carregar_partida(const std::string& caminho, const Conteudo& conteudo) {
    try {
        std::ifstream in(caminho, std::ios::binary);
        if (!in) throw std::runtime_error("partida não encontrada");
        return {ler_estado(in, conteudo), {}};
    } catch (const std::exception& e) { return {std::nullopt, e.what()}; }
}
