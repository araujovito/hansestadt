#pragma once

#include <string>
#include <vector>

#include "calendario.hpp"
#include "conteudo.hpp"
#include "eventos.hpp"

// Acontecimentos históricos para a tela do jogador. Só lê os eventos e não
// depende da raylib, para ser testado sem janela; o desenho fica em
// relatorio_tela.
//
// Só entram eventos que já começaram: os futuros ficam escondidos, para a
// história não ser anunciada antes da hora.

struct EntradaDoHistorico {
    std::string nome;
    std::string descricao;
    std::string periodo;  // "1 de janeiro de 1315 a 31 de dezembro de 1317"
    bool em_andamento = false;
    int dias_restantes = 0;  // só com em_andamento; conta o dia de hoje
    // Uma linha por efeito, em palavras: "produção geral a 65%",
    // "fazenda e fazenda de cevada a 25%", "10% de mortes ao ano".
    std::vector<std::string> efeitos;
};

// Mais recente primeiro: os em andamento no topo, depois os encerrados.
std::vector<EntradaDoHistorico> montar_historico(const Eventos& eventos, Data hoje, const Conteudo& conteudo);

// Eventos cujo início caiu depois de `antes` e até `hoje`, inclusive. Em
// velocidade alta vários dias passam num quadro, e um evento que começou no
// meio deles não pode passar sem aviso.
std::vector<const EventoHistorico*> comecaram_entre(const Eventos& eventos, Data antes, Data hoje);

// "30 de novembro de 1322".
std::string escrever_data(Data d);
