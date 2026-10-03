#include "eventos.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "toml.hpp"

namespace {

struct Erros {
    std::vector<std::string> lista;
    void adicionar(const std::string& contexto, const std::string& mensagem) {
        lista.push_back(contexto + ": " + mensagem);
    }
};

bool data_valida(Data data) {
    return data.ano >= 1 && data.mes >= 1 && data.mes <= 12 && data.dia >= 1 &&
           data.dia <= dias_no_mes(data.ano, data.mes);
}

std::optional<Data> ler_data(const toml::table& tabela, const std::string& chave, const std::string& contexto,
                             Erros& erros) {
    const toml::array* valores = tabela[chave].as_array();
    if (!valores || valores->size() != 3) {
        erros.adicionar(contexto, "'" + chave + "' precisa ser [ano, mês, dia]");
        return std::nullopt;
    }
    const auto ano = (*valores)[0].value<std::int64_t>();
    const auto mes = (*valores)[1].value<std::int64_t>();
    const auto dia = (*valores)[2].value<std::int64_t>();
    if (!ano || !mes || !dia) {
        erros.adicionar(contexto, "'" + chave + "' precisa conter três inteiros");
        return std::nullopt;
    }
    const Data data{static_cast<int>(*ano), static_cast<int>(*mes), static_cast<int>(*dia)};
    if (!data_valida(data)) {
        erros.adicionar(contexto, "'" + chave + "' não é uma data juliana válida");
        return std::nullopt;
    }
    return data;
}

std::optional<float> ler_multiplicador(const toml::node& no, const std::string& contexto, Erros& erros) {
    const auto fator = no.value<double>();
    if (!fator || *fator < 0.0 || *fator > 5.0) {
        erros.adicionar(contexto, "multiplicador precisa ser número entre 0 e 5");
        return std::nullopt;
    }
    return static_cast<float>(*fator);
}

}  // namespace

ResultadoEventos carregar_eventos(const std::string& pasta, const Conteudo& conteudo) {
    const std::string caminho = pasta + "/eventos.toml";
    std::ifstream arquivo(caminho, std::ios::binary);
    if (!arquivo) return {std::nullopt, {caminho + ": não foi possível abrir"}};
    std::ostringstream texto;
    texto << arquivo.rdbuf();

    toml::table raiz;
    try {
        raiz = toml::parse(texto.str(), caminho);
    } catch (const toml::parse_error& e) {
        std::ostringstream mensagem;
        mensagem << caminho << ": linha " << e.source().begin.line << ", coluna " << e.source().begin.column
                 << ": " << e.description();
        return {std::nullopt, {mensagem.str()}};
    }

    Erros erros;
    const toml::array* lista = raiz["evento"].as_array();
    if (!lista) return {std::nullopt, {caminho + ": nenhum [[evento]]"}};
    std::vector<EventoHistorico> eventos;
    for (const toml::node& no : *lista) {
        const toml::table* tabela = no.as_table();
        if (!tabela) continue;
        const std::string id = (*tabela)["id"].value_or(std::string("?"));
        const std::string ctx = "eventos.toml, '" + id + "'";
        EventoHistorico evento;
        evento.id = (*tabela)["id"].value_or(std::string{});
        evento.nome = (*tabela)["nome"].value_or(std::string{});
        evento.descricao = (*tabela)["descricao"].value_or(std::string{});
        if (evento.id.empty()) erros.adicionar(ctx, "falta 'id'");
        if (evento.nome.empty()) erros.adicionar(ctx, "falta 'nome'");
        if (evento.descricao.empty()) erros.adicionar(ctx, "falta 'descricao'");
        for (const auto& existente : eventos)
            if (existente.id == evento.id) erros.adicionar(ctx, "id repetido");

        const auto inicio = ler_data(*tabela, "inicio", ctx, erros);
        const auto fim = ler_data(*tabela, "fim", ctx, erros);
        if (inicio) evento.inicio = *inicio;
        if (fim) evento.fim = *fim;
        if (inicio && fim && *fim < *inicio) erros.adicionar(ctx, "'fim' precisa ser igual ou posterior a 'inicio'");

        if (tabela->contains("multiplicador_geral")) {
            if (auto fator = ler_multiplicador(*(*tabela)["multiplicador_geral"].node(), ctx, erros))
                evento.multiplicador_geral = *fator;
        }
        if (const toml::table* fatores = (*tabela)["producao"].as_table()) {
            for (const auto& [id_edificio, no_fator] : *fatores) {
                const auto tipo = conteudo.buscar_edificio(id_edificio.str());
                if (!tipo) {
                    erros.adicionar(ctx, "edifício desconhecido em producao: '" + std::string(id_edificio.str()) + "'");
                    continue;
                }
                if (auto fator = ler_multiplicador(no_fator, ctx + ", producao." + std::string(id_edificio.str()), erros))
                    evento.producao_por_edificio.emplace_back(*tipo, *fator);
            }
        } else if (tabela->contains("producao")) {
            erros.adicionar(ctx, "'producao' precisa ser tabela de multiplicadores por edifício");
        }
        eventos.push_back(std::move(evento));
    }

    if (!erros.lista.empty()) return {std::nullopt, std::move(erros.lista)};
    std::stable_sort(eventos.begin(), eventos.end(), [](const auto& a, const auto& b) { return a.inicio < b.inicio; });
    return {std::move(eventos), {}};
}

std::vector<const EventoHistorico*> Eventos::ativos(Data data) const {
    std::vector<const EventoHistorico*> resultado;
    for (const auto& evento : definicoes_)
        if (evento.inicio <= data && data <= evento.fim) resultado.push_back(&evento);
    return resultado;
}

std::vector<const EventoHistorico*> Eventos::ocorridos(Data data) const {
    std::vector<const EventoHistorico*> resultado;
    for (const auto& evento : definicoes_)
        if (evento.inicio <= data) resultado.push_back(&evento);
    return resultado;
}

float Eventos::multiplicador_producao(Data data, IdTipoEdificio edificio) const {
    float total = 1.0f;
    for (const EventoHistorico* evento : ativos(data)) {
        total *= evento->multiplicador_geral;
        for (const auto& [tipo, fator] : evento->producao_por_edificio)
            if (tipo == edificio) total *= fator;
    }
    return total;
}
