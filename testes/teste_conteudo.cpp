#include <string>
#include <algorithm>

#include "conteudo.hpp"
#include "verificar.hpp"

namespace {

constexpr const char* ITENS = R"(
[[item]]
id = "centeio"
nome = "centeio"
tipo = "insumo"

[[item]]
id = "pao"
nome = "pão"
tipo = "comida"
)";

std::string edificio(const std::string& extra) {
    return R"(
[[edificio]]
id = "padaria"
nome = "padaria"
fase = "vila"
tamanho = [2, 2]
terreno = "campo"
cor = "#c88c50"
trabalhadores = 1
)" + extra;
}

bool tem_erro_com(const ResultadoCarga& r, const std::string& trecho) {
    for (const auto& e : r.erros)
        if (e.find(trecho) != std::string::npos) return true;
    return false;
}

}  // namespace

TESTE("conteúdo: os arquivos reais do jogo carregam sem erros") {
    const ResultadoCarga r = carregar_conteudo("dados");
    for (const auto& e : r.erros) std::printf("    %s\n", e.c_str());
    VERIFICAR(r.conteudo.has_value());
    VERIFICAR(r.conteudo && r.conteudo->buscar_edificio("fazenda").has_value());
    if (r.conteudo) {
        const Conteudo& c = *r.conteudo;
        VERIFICAR(c.buscar_item("cerveja").has_value());
        const auto fazenda = c.buscar_edificio("fazenda_cevada");
        const auto malteria = c.buscar_edificio("malteria");
        const auto cervejaria = c.buscar_edificio("cervejaria");
        const auto curral = c.buscar_edificio("curral");
        const auto ovelheiro = c.buscar_edificio("ovelheiro");
        const auto tecelagem = c.buscar_edificio("tecelagem");
        const auto curtume = c.buscar_edificio("curtume");
        const auto salina = c.buscar_edificio("salina");
        const auto tanoeiro = c.buscar_edificio("tanoeiro");
        const auto pescador = c.buscar_edificio("pescador_arenque");
        const auto salgadeira = c.buscar_edificio("salgadeira");
        VERIFICAR(fazenda && malteria && cervejaria);
        VERIFICAR(curral && ovelheiro && tecelagem && curtume);
        VERIFICAR(salina && tanoeiro && pescador && salgadeira);
        if (fazenda && malteria && cervejaria) {
            VERIFICAR(c.edificio(*fazenda).receitas[0].saidas[0].item == *c.buscar_item("cevada"));
            VERIFICAR(c.edificio(*malteria).receitas[0].entradas[0].item == *c.buscar_item("cevada"));
            VERIFICAR(c.edificio(*malteria).receitas[0].saidas[0].item == *c.buscar_item("malte"));
            VERIFICAR(c.edificio(*cervejaria).receitas[0].entradas[0].item == *c.buscar_item("malte"));
            VERIFICAR(c.edificio(*cervejaria).receitas[0].saidas[0].item == *c.buscar_item("cerveja"));
        }
        if (curral && ovelheiro && tecelagem && curtume) {
            const auto produz = [](const Receita& receita, IdItem id) {
                return std::any_of(receita.saidas.begin(), receita.saidas.end(),
                    [id](const Quantidade& q) { return q.item == id; });
            };
            const auto quantidade = [](const Receita& receita, IdItem id) {
                for (const Quantidade& q : receita.saidas)
                    if (q.item == id) return q.quantidade;
                return 0;
            };
            VERIFICAR(c.edificio(*curral).receitas[0].saidas.size() == 2);
            VERIFICAR(quantidade(c.edificio(*curral).receitas[0], *c.buscar_item("carne")) == 12);
            VERIFICAR(produz(c.edificio(*ovelheiro).receitas[0], *c.buscar_item("la")));
            VERIFICAR(quantidade(c.edificio(*ovelheiro).receitas[0], *c.buscar_item("carne")) == 12);
            VERIFICAR(c.edificio(*tecelagem).receitas[0].entradas[0].item == *c.buscar_item("la"));
            VERIFICAR(c.edificio(*tecelagem).receitas[0].saidas[0].item == *c.buscar_item("tecido"));
            VERIFICAR(c.edificio(*curtume).receitas[0].entradas[0].item == *c.buscar_item("pele"));
            VERIFICAR(c.edificio(*curtume).receitas[0].saidas[0].item == *c.buscar_item("couro"));
        }
        if (salina && tanoeiro && pescador && salgadeira) {
            const Receita& salga = c.edificio(*salgadeira).receitas[0];
            const auto tem_entrada = [&salga, &c](const char* id) {
                return std::any_of(salga.entradas.begin(), salga.entradas.end(),
                    [&c, id](const Quantidade& q) { return q.item == *c.buscar_item(id); });
            };
            VERIFICAR(c.edificio(*salina).receitas[0].saidas[0].item == *c.buscar_item("sal"));
            VERIFICAR(c.edificio(*tanoeiro).receitas[0].saidas[0].item == *c.buscar_item("barril"));
            VERIFICAR(c.edificio(*pescador).receitas[0].saidas[0].item == *c.buscar_item("arenque"));
            VERIFICAR(c.edificio(*pescador).receitas[0].saidas[0].quantidade == 16);
            VERIFICAR(tem_entrada("arenque") && tem_entrada("sal") && tem_entrada("barril"));
            VERIFICAR(salga.saidas[0].item == *c.buscar_item("arenque_salgado"));
            VERIFICAR(c.itens[*c.buscar_item("arenque_salgado")].tipo == TipoItem::Comida);
        }
    }
}

TESTE("conteúdo: lê receita, sazonalidade e requisito") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio(R"(
requer = { terreno = "agua", distancia = 2 }
[[edificio.receita]]
dias = 3
entradas = { centeio = 1 }
saidas = { pao = 2 }
sazonalidade = [0, 0, 0, 0, 0, 0, 1, 1, 0.5, 0, 0, 0]
)"));
    VERIFICAR(r.erros.empty());
    if (!r.conteudo) return;
    const DefinicaoEdificio& d = r.conteudo->edificios[0];
    VERIFICAR(d.largura == 2 && d.altura == 2);
    VERIFICAR(d.cor == 0xC88C50FFU);
    VERIFICAR(d.requer && d.requer->terreno == Terreno::Agua && d.requer->distancia == 2);
    VERIFICAR(d.receitas.size() == 1);
    VERIFICAR(d.receitas[0].dias == 3);
    VERIFICAR(d.receitas[0].entradas.size() == 1 && d.receitas[0].entradas[0].quantidade == 1);
    VERIFICAR(d.receitas[0].saidas[0].item == *r.conteudo->buscar_item("pao"));
    VERIFICAR(d.receitas[0].sazonalidade[0] == 0.0f && d.receitas[0].sazonalidade[8] == 0.5f);
}

TESTE("conteúdo: sazonalidade omitida vale 1 o ano todo") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio(R"(
[[edificio.receita]]
dias = 3
saidas = { pao = 1 }
)"));
    VERIFICAR(r.conteudo && r.conteudo->edificios[0].receitas[0].sazonalidade[5] == 1.0f);
}

TESTE("conteúdo: recusa item desconhecido na receita") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio(R"(
[[edificio.receita]]
dias = 3
saidas = { bolo = 1 }
)"));
    VERIFICAR(!r.conteudo);
    VERIFICAR(tem_erro_com(r, "item desconhecido 'bolo'"));
}

TESTE("conteúdo: recusa chave escrita errada") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio("trabalhadore = 2\n"));
    VERIFICAR(tem_erro_com(r, "chave desconhecida 'trabalhadore'"));
}

TESTE("conteúdo: sazonalidade precisa de 12 meses") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio(R"(
[[edificio.receita]]
dias = 3
saidas = { pao = 1 }
sazonalidade = [1, 1, 1]
)"));
    VERIFICAR(tem_erro_com(r, "12 valores"));
}

TESTE("conteúdo: depósito não pode ter receita") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio(R"(
estoque = 10
deposito = true
[[edificio.receita]]
dias = 3
saidas = { pao = 1 }
)"));
    VERIFICAR(tem_erro_com(r, "depósito não pode ter receita"));
}

TESTE("conteúdo: só depósito tem carregadores") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, edificio("carregadores = 3\n"));
    VERIFICAR(tem_erro_com(r, "só depósito tem carregadores"));
}

TESTE("conteúdo: erro de sintaxe informa linha e coluna") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, "[[edificio]]\nid = \n");
    VERIFICAR(tem_erro_com(r, "linha 2"));
}

TESTE("conteúdo: junta todos os erros em vez de parar no primeiro") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, R"(
[[edificio]]
id = "x"
nome = "x"
fase = "medieval"
tamanho = [0, 2]
terreno = "pantano"
cor = "vermelho"
)");
    VERIFICAR(r.erros.size() == 4);
}

namespace {

// Padaria com receita, para as técnicas terem o que acelerar.
std::string padaria_e(const std::string& tecnicas) {
    return edificio("[[edificio.receita]]\ndias = 3\nsaidas = { pao = 1 }\n") + tecnicas;
}

}  // namespace

TESTE("conteúdo: lê técnicas com fase, edifícios, requisito, custo e bônus") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, padaria_e(R"(
[[tecnica]]
id = "forno"
nome = "forno de tijolo"
descricao = "assa mais pão por fornada"
fase = "vila"
edificios = ["padaria"]
custo = { centeio = 2 }
bonus = 0.75

[[tecnica]]
id = "guilda"
nome = "guilda"
fase = "cidade"
edificios = ["padaria"]
requer = "forno"
custo = { centeio = 4 }
bonus = 0.75
)"));
    for (const auto& e : r.erros) std::printf("    %s\n", e.c_str());
    VERIFICAR(r.conteudo.has_value());
    if (!r.conteudo) return;
    const Conteudo& c = *r.conteudo;
    VERIFICAR(c.tecnicas.size() == 2);
    const Tecnica& forno = c.tecnicas[0];
    VERIFICAR(forno.nome == "forno de tijolo" && forno.descricao == "assa mais pão por fornada");
    VERIFICAR(forno.fase == Fase::Vila && forno.bonus == 750 && !forno.requer);
    VERIFICAR(forno.edificios.size() == 1 && forno.edificios[0] == *c.buscar_edificio("padaria"));
    VERIFICAR(forno.custo.size() == 1 && forno.custo[0].quantidade == 2);
    VERIFICAR(c.tecnicas[1].requer == c.buscar_tecnica("forno"));
}

TESTE("conteúdo: sem técnicas declaradas a lista fica vazia") {
    const ResultadoCarga r = carregar_conteudo_de_texto(ITENS, padaria_e(""));
    VERIFICAR(r.conteudo && r.conteudo->tecnicas.empty());
}

TESTE("conteúdo: recusa técnica malformada") {
    const auto erro = [](const std::string& tecnica, const std::string& trecho) {
        return tem_erro_com(carregar_conteudo_de_texto(ITENS, padaria_e(tecnica)), trecho);
    };
    const std::string base = "[[tecnica]]\nid = \"t\"\nnome = \"t\"\nfase = \"vila\"\ncusto = { centeio = 1 }\n";
    VERIFICAR(erro(base + "edificios = [\"moinho\"]\nbonus = 0.5\n", "edifício desconhecido 'moinho'"));
    VERIFICAR(erro(base + "bonus = 0.5\n", "precisa de 'edificios'"));
    VERIFICAR(erro(base + "edificios = [\"padaria\"]\nbonus = 0\n", "'bonus'"));
    VERIFICAR(erro(base + "edificios = [\"padaria\"]\nbonus = 0.5\nrequer = \"depois\"\n",
                   "declarada antes"));
    VERIFICAR(erro(base + "edificios = [\"padaria\"]\nbonus = 0.5\nbonsu = 1\n", "chave desconhecida 'bonsu'"));
    // Técnica de fase posterior não pode ser requisito de uma anterior.
    VERIFICAR(erro(R"(
[[tecnica]]
id = "a"
nome = "a"
fase = "cidade"
edificios = ["padaria"]
custo = { centeio = 1 }
bonus = 0.5

[[tecnica]]
id = "b"
nome = "b"
fase = "vila"
edificios = ["padaria"]
requer = "a"
custo = { centeio = 1 }
bonus = 0.5
)", "fase não pode ser anterior"));
}

TESTE("conteúdo: técnica não vale para edifício que não produz") {
    const std::string casa = R"(
[[edificio]]
id = "casa"
nome = "casa"
fase = "vila"
tamanho = [1, 1]
terreno = "campo"
cor = "#808080"
moradores = 2

[[tecnica]]
id = "t"
nome = "t"
fase = "vila"
edificios = ["casa"]
custo = { centeio = 1 }
bonus = 0.5
)";
    VERIFICAR(tem_erro_com(carregar_conteudo_de_texto(ITENS, casa), "não produz nada"));
}
