#include <filesystem>
#include <fstream>
#include <set>

#include "conteudo_de_teste.hpp"
#include "partidas.hpp"
#include "salvamento.hpp"
#include "verificar.hpp"

namespace {

// Arquivo temporário próprio de cada teste, apagado ao sair.
struct ArquivoTemporario {
    std::string caminho;
    explicit ArquivoTemporario(const char* nome)
        : caminho((std::filesystem::temp_directory_path() / nome).string()) {
        std::filesystem::remove(caminho);
    }
    ~ArquivoTemporario() { std::filesystem::remove(caminho); }
};

}  // namespace

TESTE("partidas: o salvamento rápido vem primeiro e usa o arquivo de antes do menu") {
    const auto& vagas = vagas_de_partida();
    VERIFICAR(!vagas.empty() && vagas.front().arquivo == "partida.hanse");
    std::set<std::string> arquivos;
    for (const VagaDePartida& v : vagas) arquivos.insert(v.arquivo);
    VERIFICAR(arquivos.size() == vagas.size());  // duas vagas nunca dividem arquivo
}

TESTE("partidas: o resumo traz data e moradores") {
    Simulacao sim(Cidade(Mapa(8, 8), conteudo_de_teste()));
    sim.data = { 1151, 3, 12 };
    VERIFICAR(resumir_partida(sim) == "12 de março de 1151, 6 moradores");
    sim.populacao = Populacao(1);
    VERIFICAR(resumir_partida(sim) == "12 de março de 1151, 1 morador");
}

TESTE("partidas: vaga sem arquivo aparece vazia") {
    const ArquivoTemporario a("hansestadt-vaga-vazia.hanse");
    VERIFICAR(examinar_vaga(a.caminho, conteudo_de_teste()).estado == SituacaoDaVaga::Estado::Vazia);
}

TESTE("partidas: vaga com partida salva mostra o resumo") {
    const ArquivoTemporario a("hansestadt-vaga-valida.hanse");
    Simulacao sim(Cidade(Mapa(8, 8), conteudo_de_teste()));
    std::string erro;
    VERIFICAR(salvar_partida(sim, a.caminho, erro));
    const SituacaoDaVaga s = examinar_vaga(a.caminho, conteudo_de_teste());
    VERIFICAR(s.estado == SituacaoDaVaga::Estado::Valida);
    VERIFICAR(s.texto == resumir_partida(sim));
}

TESTE("partidas: arquivo que não é partida aparece inválido, com o motivo") {
    const ArquivoTemporario a("hansestadt-vaga-invalida.hanse");
    std::ofstream(a.caminho) << "isto não é uma partida";
    const SituacaoDaVaga s = examinar_vaga(a.caminho, conteudo_de_teste());
    VERIFICAR(s.estado == SituacaoDaVaga::Estado::Invalida);
    VERIFICAR(!s.texto.empty());
}
