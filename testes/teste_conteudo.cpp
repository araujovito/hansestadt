#include <string>

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
