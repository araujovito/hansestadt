#pragma once

#include <string>
#include <vector>

#include "conteudo.hpp"
#include "simulacao.hpp"

// Vagas de partida salva que o menu oferece. Não depende da raylib, para o
// que aparece em cada vaga poder ser testado sem janela.
struct VagaDePartida {
    std::string nome;     // como aparece no menu
    std::string arquivo;  // caminho relativo à pasta onde o jogo roda
};

// O salvamento rápido (F5/F9) vem primeiro e usa o mesmo arquivo de antes do
// menu existir, para partidas já salvas continuarem abrindo.
const std::vector<VagaDePartida>& vagas_de_partida();

// "12 de março de 1151, 14 moradores": o que identifica uma partida na lista.
std::string resumir_partida(const Simulacao& sim);

struct SituacaoDaVaga {
    enum class Estado { Vazia, Valida, Invalida };
    Estado estado = Estado::Vazia;
    // Resumo da partida (válida) ou o motivo de não abrir (inválida).
    std::string texto;
};

// Abre o arquivo da vaga para dizer o que há nele. Uma partida de versão
// antiga ou corrompida aparece como inválida, com o erro do carregamento.
SituacaoDaVaga examinar_vaga(const std::string& arquivo, const Conteudo& conteudo);
