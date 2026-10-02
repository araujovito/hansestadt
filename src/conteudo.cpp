#include "conteudo.hpp"

#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <sstream>

#include "toml.hpp"

const char* nome_da_fase(Fase f) {
    switch (f) {
        case Fase::Vila: return "vila";
        case Fase::Mercado: return "mercado";
        case Fase::Cidade: return "cidade";
        case Fase::Hansa: return "hansa";
        case Fase::Auge: return "auge";
    }
    return "?";
}

bool DefinicaoEdificio::usa_item(IdItem item) const {
    for (const Receita& r : receitas) {
        for (const Quantidade& q : r.entradas)
            if (q.item == item) return true;
        for (const Quantidade& q : r.saidas)
            if (q.item == item) return true;
    }
    return false;
}

std::optional<IdTipoEdificio> Conteudo::buscar_edificio(std::string_view id) const {
    for (std::size_t i = 0; i < edificios.size(); ++i) {
        if (edificios[i].id == id) return static_cast<IdTipoEdificio>(i);
    }
    return std::nullopt;
}

std::optional<IdItem> Conteudo::buscar_item(std::string_view id) const {
    for (std::size_t i = 0; i < itens.size(); ++i) {
        if (itens[i].id == id) return static_cast<IdItem>(i);
    }
    return std::nullopt;
}

namespace {

// Acumula erros com o contexto de onde estão ("edificios.toml, 'moinho'").
class Erros {
public:
    void adicionar(const std::string& contexto, const std::string& mensagem) {
        lista.push_back(contexto + ": " + mensagem);
    }
    std::vector<std::string> lista;
};

std::optional<Terreno> terreno_por_id(std::string_view id) {
    if (id == "campo") return Terreno::Campo;
    if (id == "floresta") return Terreno::Floresta;
    if (id == "barreiro") return Terreno::Barreiro;
    if (id == "agua") return Terreno::Agua;
    if (id == "turfa") return Terreno::Turfa;
    return std::nullopt;
}

std::optional<Fase> fase_por_id(std::string_view id) {
    for (Fase f : { Fase::Vila, Fase::Mercado, Fase::Cidade, Fase::Hansa, Fase::Auge }) {
        if (id == nome_da_fase(f)) return f;
    }
    return std::nullopt;
}

std::optional<std::uint32_t> cor_por_texto(std::string_view texto) {
    if (texto.size() != 7 || texto[0] != '#') return std::nullopt;
    std::uint32_t rgb = 0;
    for (char c : texto.substr(1)) {
        rgb <<= 4;
        if (c >= '0' && c <= '9') rgb |= static_cast<std::uint32_t>(c - '0');
        else if (c >= 'a' && c <= 'f') rgb |= static_cast<std::uint32_t>(c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') rgb |= static_cast<std::uint32_t>(c - 'A' + 10);
        else return std::nullopt;
    }
    return (rgb << 8) | 0xFFU;
}

// Chave escrita errada (`trabalhadore`) seria ignorada em silêncio e o
// edifício ficaria com o valor padrão; é melhor recusar.
void conferir_chaves(const toml::table& tabela, std::initializer_list<std::string_view> permitidas,
                     const std::string& contexto, Erros& erros) {
    for (const auto& [chave, valor] : tabela) {
        (void)valor;
        if (std::find(permitidas.begin(), permitidas.end(), chave.str()) == permitidas.end()) {
            erros.adicionar(contexto, "chave desconhecida '" + std::string(chave.str()) + "'");
        }
    }
}

std::optional<std::string> texto_obrigatorio(const toml::table& t, std::string_view chave,
                                             const std::string& contexto, Erros& erros) {
    if (auto v = t[chave].value<std::string>()) return v;
    erros.adicionar(contexto, "falta '" + std::string(chave) + "' (texto)");
    return std::nullopt;
}

int inteiro(const toml::table& t, std::string_view chave, int padrao, int minimo, const std::string& contexto,
            Erros& erros) {
    if (!t.contains(chave)) return padrao;
    auto v = t[chave].value<std::int64_t>();
    if (!v || *v < minimo) {
        erros.adicionar(contexto, "'" + std::string(chave) + "' precisa ser inteiro >= " + std::to_string(minimo));
        return padrao;
    }
    return static_cast<int>(*v);
}

void ler_itens(const toml::table& raiz, Conteudo& c, Erros& erros) {
    const toml::array* itens = raiz["item"].as_array();
    if (!itens) {
        erros.adicionar("itens.toml", "nenhum [[item]]");
        return;
    }
    for (const toml::node& no : *itens) {
        const toml::table* t = no.as_table();
        if (!t) continue;
        const std::string ctx = "itens.toml, '" + (*t)["id"].value_or(std::string("?")) + "'";
        conferir_chaves(*t, { "id", "nome", "tipo" }, ctx, erros);

        auto id = texto_obrigatorio(*t, "id", ctx, erros);
        auto nome = texto_obrigatorio(*t, "nome", ctx, erros);
        auto tipo = texto_obrigatorio(*t, "tipo", ctx, erros);
        if (!id || !nome || !tipo) continue;
        if (*tipo != "insumo" && *tipo != "comida") {
            erros.adicionar(ctx, "tipo '" + *tipo + "' inválido (insumo ou comida)");
            continue;
        }
        if (c.buscar_item(*id)) {
            erros.adicionar(ctx, "id repetido");
            continue;
        }
        c.itens.push_back({ *id, *nome, *tipo == "comida" ? TipoItem::Comida : TipoItem::Insumo });
    }
}

std::vector<Quantidade> ler_quantidades(const toml::table& receita, std::string_view chave, const Conteudo& c,
                                        const std::string& ctx, Erros& erros) {
    std::vector<Quantidade> lista;
    const toml::table* t = receita[chave].as_table();
    if (!t) return lista;
    for (const auto& [item, qtd] : *t) {
        auto id = c.buscar_item(item.str());
        auto n = qtd.value<std::int64_t>();
        if (!id) {
            erros.adicionar(ctx, "item desconhecido '" + std::string(item.str()) + "'");
        } else if (!n || *n <= 0) {
            erros.adicionar(ctx, "quantidade de '" + std::string(item.str()) + "' precisa ser inteiro > 0");
        } else {
            lista.push_back({ *id, static_cast<int>(*n) });
        }
    }
    // A ordem das chaves numa tabela TOML não é garantida; ordenar mantém a
    // simulação determinística.
    std::sort(lista.begin(), lista.end(), [](Quantidade a, Quantidade b) { return a.item < b.item; });
    return lista;
}

std::optional<Receita> ler_receita(const toml::table& t, const Conteudo& c, const std::string& ctx, Erros& erros) {
    conferir_chaves(t, { "dias", "entradas", "saidas", "sazonalidade" }, ctx, erros);
    Receita r{};
    r.dias = inteiro(t, "dias", 1, 1, ctx, erros);
    r.entradas = ler_quantidades(t, "entradas", c, ctx, erros);
    r.saidas = ler_quantidades(t, "saidas", c, ctx, erros);
    if (r.saidas.empty()) erros.adicionar(ctx, "receita sem saídas");

    r.sazonalidade.fill(1.0f);
    if (t.contains("sazonalidade")) {
        const toml::array* meses = t["sazonalidade"].as_array();
        if (!meses || meses->size() != 12) {
            erros.adicionar(ctx, "sazonalidade precisa ter 12 valores, de janeiro a dezembro");
        } else {
            for (std::size_t m = 0; m < 12; ++m) {
                auto v = (*meses)[m].value<double>();
                if (!v || *v < 0.0) {
                    erros.adicionar(ctx, "sazonalidade do mês " + std::to_string(m + 1) + " precisa ser número >= 0");
                } else {
                    r.sazonalidade[m] = static_cast<float>(*v);
                }
            }
        }
    }
    return r;
}

void ler_edificios(const toml::table& raiz, Conteudo& c, Erros& erros) {
    const toml::array* edificios = raiz["edificio"].as_array();
    if (!edificios) {
        erros.adicionar("edificios.toml", "nenhum [[edificio]]");
        return;
    }
    for (const toml::node& no : *edificios) {
        const toml::table* t = no.as_table();
        if (!t) continue;
        const std::string ctx = "edificios.toml, '" + (*t)["id"].value_or(std::string("?")) + "'";
        conferir_chaves(*t,
                        { "id", "nome", "fase", "tamanho", "terreno", "cor", "trabalhadores", "estoque", "moradores",
                          "campos", "deposito", "carregadores", "requer", "receita" },
                        ctx, erros);

        DefinicaoEdificio d{};
        const std::size_t erros_antes = erros.lista.size();

        auto id = texto_obrigatorio(*t, "id", ctx, erros);
        auto nome = texto_obrigatorio(*t, "nome", ctx, erros);
        if (id) d.id = *id;
        if (nome) d.nome = *nome;
        if (id && c.buscar_edificio(*id)) erros.adicionar(ctx, "id repetido");

        auto fase = fase_por_id((*t)["fase"].value_or(std::string()));
        if (!fase) erros.adicionar(ctx, "fase inválida (vila, mercado, cidade, hansa ou auge)");
        else d.fase = *fase;

        const toml::array* tamanho = (*t)["tamanho"].as_array();
        auto largura = tamanho && tamanho->size() == 2 ? (*tamanho)[0].value<std::int64_t>() : std::nullopt;
        auto altura = tamanho && tamanho->size() == 2 ? (*tamanho)[1].value<std::int64_t>() : std::nullopt;
        if (!largura || !altura || *largura < 1 || *altura < 1) {
            erros.adicionar(ctx, "tamanho precisa ser [largura, altura] com inteiros >= 1");
        } else {
            d.largura = static_cast<int>(*largura);
            d.altura = static_cast<int>(*altura);
        }

        auto terreno = terreno_por_id((*t)["terreno"].value_or(std::string()));
        if (!terreno) erros.adicionar(ctx, "terreno inválido (campo, floresta, barreiro, agua ou turfa)");
        else d.terreno = *terreno;

        auto cor = cor_por_texto((*t)["cor"].value_or(std::string()));
        if (!cor) erros.adicionar(ctx, "cor precisa ser \"#rrggbb\"");
        else d.cor = *cor;

        d.trabalhadores = inteiro(*t, "trabalhadores", 0, 0, ctx, erros);
        d.estoque = inteiro(*t, "estoque", 0, 0, ctx, erros);
        d.moradores = inteiro(*t, "moradores", 0, 0, ctx, erros);
        d.campos = inteiro(*t, "campos", 0, 0, ctx, erros);
        if (t->contains("deposito")) {
            auto deposito = (*t)["deposito"].value<bool>();
            if (!deposito) erros.adicionar(ctx, "'deposito' precisa ser true ou false");
            d.deposito = deposito.value_or(false);
        }

        if (const toml::table* requer = (*t)["requer"].as_table()) {
            conferir_chaves(*requer, { "terreno", "distancia" }, ctx + ", requer", erros);
            auto terreno_req = terreno_por_id((*requer)["terreno"].value_or(std::string()));
            if (!terreno_req) erros.adicionar(ctx, "requer.terreno inválido");
            d.requer = Requisito{ terreno_req.value_or(Terreno::Campo),
                                  inteiro(*requer, "distancia", 1, 1, ctx + ", requer", erros) };
        }

        if (const toml::array* receitas = (*t)["receita"].as_array()) {
            for (std::size_t i = 0; i < receitas->size(); ++i) {
                if (const toml::table* r = (*receitas)[i].as_table()) {
                    const std::string ctx_r = ctx + ", receita " + std::to_string(i + 1);
                    if (auto receita = ler_receita(*r, c, ctx_r, erros)) d.receitas.push_back(std::move(*receita));
                }
            }
        }

        d.carregadores = inteiro(*t, "carregadores", 0, 0, ctx, erros);
        if (d.carregadores > 0 && !d.deposito) erros.adicionar(ctx, "só depósito tem carregadores");
        if (d.deposito && !d.receitas.empty()) erros.adicionar(ctx, "depósito não pode ter receita");
        if (d.deposito && d.estoque == 0) erros.adicionar(ctx, "depósito precisa de estoque > 0");
        if (!d.receitas.empty() && d.trabalhadores == 0) {
            erros.adicionar(ctx, "edifício com receita precisa de trabalhadores");
        }

        if (erros.lista.size() == erros_antes) c.edificios.push_back(std::move(d));
    }
}

std::optional<toml::table> analisar(std::string_view texto, const std::string& arquivo, Erros& erros) {
    try {
        return toml::parse(texto, arquivo);
    } catch (const toml::parse_error& e) {
        std::ostringstream msg;
        msg << "linha " << e.source().begin.line << ", coluna " << e.source().begin.column << ": " << e.description();
        erros.adicionar(arquivo, msg.str());
        return std::nullopt;
    }
}

std::optional<std::string> ler_arquivo(const std::string& caminho, Erros& erros) {
    std::ifstream arquivo(caminho, std::ios::binary);
    if (!arquivo) {
        erros.adicionar(caminho, "não foi possível abrir");
        return std::nullopt;
    }
    std::ostringstream conteudo;
    conteudo << arquivo.rdbuf();
    return conteudo.str();
}

}  // namespace

ResultadoCarga carregar_conteudo_de_texto(std::string_view itens_toml, std::string_view edificios_toml) {
    Erros erros;
    Conteudo c;
    // Itens primeiro: as receitas dos edifícios referenciam os ids deles.
    if (auto raiz = analisar(itens_toml, "itens.toml", erros)) ler_itens(*raiz, c, erros);
    if (auto raiz = analisar(edificios_toml, "edificios.toml", erros)) ler_edificios(*raiz, c, erros);

    if (!erros.lista.empty()) return { std::nullopt, std::move(erros.lista) };
    return { std::move(c), {} };
}

ResultadoCarga carregar_conteudo(const std::string& pasta) {
    Erros erros;
    auto itens = ler_arquivo(pasta + "/itens.toml", erros);
    auto edificios = ler_arquivo(pasta + "/edificios.toml", erros);
    if (!itens || !edificios) return { std::nullopt, std::move(erros.lista) };
    return carregar_conteudo_de_texto(*itens, *edificios);
}
