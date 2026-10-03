#include "arte.hpp"

#include <algorithm>
#include <vector>

#include "producao.hpp"
#include "ruido.hpp"

namespace {

constexpr int C = 32;  // célula do jogo, em pixels de mundo (igual a TAMANHO_CELULA)
constexpr int P = 16;  // peça, desenhada sempre em 1x

// Folha da textura única: cada peça ocupa 18x18 (16 + 1 px de folga de cada
// lado), em COLUNAS_ATLAS colunas.
constexpr int CASA = 18;
constexpr int COLUNAS_ATLAS = 32;

enum class Folha { Nossas, Town, Farm };
constexpr int PECAS_NOSSAS = 70;
constexpr int PECAS_TINY = 132;  // 12 x 11 em cada pacote da Kenney
constexpr int BASE_TOWN = PECAS_NOSSAS;
constexpr int BASE_FARM = BASE_TOWN + PECAS_TINY;
constexpr int PECA_BRANCA = BASE_FARM + PECAS_TINY;  // usada para retângulos (neve)
constexpr int TOTAL_PECAS = PECA_BRANCA + 1;

// Peças próprias usadas pelo nome, na ordem de ferramentas/gerar_pecas.py.
enum : int {
    AGUA = 0, BARREIRO = 2, TURFA = 3, TIJOLO = 4, TIJOLO_JANELA = 5, TIJOLO_PORTA = 6,
    MARGEM = 8, FRANJA = 12, MONTE_ARGILA = 16, POCA_JUNCO = 17, PEDRINHAS = 18, CHAMINE = 19,
    PALHA = 20, VARAL_PEIXE = 28, RODA = 29, PORCO = 33, LAMA = 34, CEPO = 35, FORNO = 36,
    PLACA_PAO = 37, TORRE_MADEIRA = 38, TORRE_TIJOLO = 40, BANDEIRA = 42, JANELA_GOTICA = 43,
    EMPENA_ESCALONADA = 44, JANELA_ESTREITA = 46, PLACA_CANECA = 47, MONTE_CARVAO = 48,
    PILHA_TURFA = 49, TOLDO = 50, BALCAO = 52, RESPIRO = 53, CEVADA = 54, VARAL_TECIDOS = 55,
    TINA = 56, COURO = 57, TANQUE_SAL = 58, BURACO_ARGILA = 59, PILHA_TIJOLOS = 60,
    FORNO_OLARIA = 61, BARCO = 63, REDE = 65, PIER = 66, GUINDASTE = 67, SOMBRA = 69,
};

Texture2D atlas{};
bool outono = false, inverno = false;

constexpr Color NEVE{ 244, 248, 255, 170 };
constexpr Color NEVE_FORTE{ 250, 252, 255, 235 };
const int dx4[4] = { 0, 1, 0, -1 }, dy4[4] = { -1, 0, 1, 0 };

// Peças que já trazem grama embaixo (pedras do caminho, mata) precisam do
// mesmo tom da estação que o chão em volta, senão viram quadrados.
Color tom_da_estacao() { return outono ? Color{ 248, 236, 196, 255 } : WHITE; }

Rectangle origem(int global) {
    return { static_cast<float>((global % COLUNAS_ATLAS) * CASA + 1), static_cast<float>((global / COLUNAS_ATLAS) * CASA + 1),
             P, P };
}

int global_de(Folha f, int n) {
    return f == Folha::Nossas ? n : f == Folha::Town ? BASE_TOWN + n : BASE_FARM + n;
}

void peca(Folha f, int n, float x, float y, Color tinta = WHITE) {
    DrawTexturePro(atlas, origem(global_de(f, n)), { x, y, P, P }, { 0, 0 }, 0, tinta);
}
void nossa(int n, float x, float y, Color t = WHITE) { peca(Folha::Nossas, n, x, y, t); }
void town(int n, float x, float y, Color t = WHITE) { peca(Folha::Town, n, x, y, t); }
void farm(int n, float x, float y, Color t = WHITE) { peca(Folha::Farm, n, x, y, t); }

// Sombra: a peça de elipse suave esticada. Uma elipse desenhada pela raylib
// custava mais de 100 vértices e quebrava o agrupamento das peças.
void sombra(float cx, float cy, float rx, float ry) {
    DrawTexturePro(atlas, origem(SOMBRA), { cx - rx, cy - ry, 2 * rx, 2 * ry }, { 0, 0 }, 0, WHITE);
}

template <typename F> void subpecas(int x, int y, F f) {
    for (int j = 0; j < 2; ++j)
        for (int i = 0; i < 2; ++i) f(2 * x + i, 2 * y + j, static_cast<float>(x * C + i * P), static_cast<float>(y * C + j * P));
}

// ---------------------------------------------------------------- edifícios

enum class Telhado { Ardosia, Telha, Palha };
enum class Parede { Madeira, Pedra, Tijolo };
struct Estilo {
    Telhado telhado;
    Parede parede;
    bool empena;            // fachada pontuda na frente
    bool chamine;           // chaminé de tijolo com fumaça
    bool estreita = false;  // uma peça mais estreita, com quintal do lado esquerdo
};

// Peça de telhado: alto/beiral x esquerda/meio/direita, empena e mansarda.
// Palha usa as peças próprias, na mesma ordem das de telha.
void telhado(Telhado t, int qual, float x, float y) {
    static const int telha[8] = { 52, 53, 54, 64, 65, 66, 67, 55 };
    static const int ardosia[8] = { 48, 49, 50, 60, 61, 62, 63, 51 };
    if (t == Telhado::Palha) nossa(PALHA + qual, x, y);
    else town(t == Telhado::Telha ? telha[qual] : ardosia[qual], x, y);
}

// A cobertura acompanha a época da cidade: palha na vila, telha no mercado e,
// da cidade em diante, paredes de tijolo (Backsteingotik).
Estilo estilo_de(const std::string& id, Fase fase, float sorte) {
    if (id == "casa") {
        const Telhado t = fase == Fase::Vila ? Telhado::Palha : Telhado::Telha;
        const Parede p = fase >= Fase::Cidade ? Parede::Tijolo : Parede::Madeira;
        return { t, p, sorte < 0.7f, sorte > 0.45f, sorte < 0.35f };
    }
    if (id == "armazem") return { Telhado::Ardosia, Parede::Madeira, false, false };
    if (id == "padaria") return { Telhado::Telha, Parede::Pedra, false, true };
    if (id == "moinho") return { Telhado::Ardosia, Parede::Pedra, true, false };
    if (id == "pescador") return { Telhado::Palha, Parede::Madeira, false, true };
    if (fase >= Fase::Cidade) return { Telhado::Telha, Parede::Tijolo, true, false };
    return { Telhado::Palha, Parede::Madeira, false, false };
}

// O telhado tem sempre duas fileiras (alto e beiral): repetir a de cima
// empilhava linhas de cumeeira. Lotes mais altos ganham uma segunda fileira
// de parede e, se ainda sobrar, um quintal atrás do edifício.
int altura_desenhada(int h) { return h >= 3 ? 4 : 3; }

// Edifício de w x h células, encostado na base do lote, com a porta na coluna
// da porta do jogo (meio da borda de baixo).
void compor(const Estilo& e, float x0, float y0, int w, int h) {
    const int H = 2 * h;
    const int A = altura_desenhada(h);
    const int i0 = e.estreita ? 1 : 0;
    const int W = 2 * w - i0;
    x0 += i0 * P;
    const int porta = w - i0;
    y0 += (H - A) * P;
    for (int j = 0; j < 2; ++j)
        for (int i = 0; i < W; ++i) telhado(e.telhado, j * 3 + (i == 0 ? 0 : i == W - 1 ? 2 : 1), x0 + i * P, y0 + j * P);
    if (e.empena) telhado(e.telhado, 6, x0 + porta * P, y0 + P);
    if (inverno) {
        DrawRectangleV({ x0 + 1, y0 + 1 }, { W * P - 2.0f, 7 }, NEVE_FORTE);
        for (int i = 0; i < W * 4; ++i)
            if ((i * 7) % 3) DrawRectangleV({ x0 + i * 4.0f, y0 + 8 }, { 4, 2 }, NEVE_FORTE);
    }
    if (e.chamine) nossa(CHAMINE, x0 + (W - 2) * P, y0 - 8);
    auto parede = [&](int i, float y, bool com_porta) {
        const float x = x0 + i * P;
        if (e.parede == Parede::Tijolo) {
            nossa(com_porta && i == porta ? TIJOLO_PORTA : (i % 2 ? TIJOLO_JANELA : TIJOLO), x, y);
            return;
        }
        const bool madeira = e.parede == Parede::Madeira;
        int n;
        if (com_porta && i == porta) n = madeira ? 85 : 89;
        else if (i == 0) n = madeira ? 72 : 76;
        else if (i == W - 1) n = madeira ? 75 : 79;
        else n = (i % 2) ? (madeira ? 84 : 88) : (madeira ? 73 : 77);
        town(n, x, y);
    };
    for (int j = 2; j < A - 1; ++j)
        for (int i = 0; i < W; ++i) parede(i, y0 + j * P, false);
    for (int i = 0; i < W; ++i) parede(i, y0 + (A - 1) * P, true);
}

// Igreja por nível: capela de madeira e palha, pedra e ardósia, e, do nível 3
// em diante, tijolo com fachada em degraus, janelas góticas e torre com
// pináculo. A torre fica na frente da fachada, como a torre ocidental das
// igrejas de tijolo do Báltico; ocupa o lote inteiro na altura para ser
// vertical, ao contrário das casas.
void igreja(int nivel, float nx, float ny) {
    const Telhado t = nivel <= 1 ? Telhado::Palha : nivel == 2 ? Telhado::Ardosia : Telhado::Telha;
    const bool tijolo = nivel >= 3, pedra = nivel == 2;
    const int W = 4;
    for (int j = 0; j < 2; ++j)
        for (int i = 0; i < W; ++i) telhado(t, j * 3 + (i == 0 ? 0 : i == W - 1 ? 2 : 1), nx + i * P, ny + j * P);
    for (int j = 2; j < 4; ++j)
        for (int i = 0; i < W; ++i) {
            const float x = nx + i * P, y = ny + j * P;
            const bool porta = j == 3 && i == 2;
            if (tijolo) { nossa(porta ? TIJOLO_PORTA : JANELA_GOTICA, x, y); continue; }
            town(porta ? (pedra ? 89 : 85) : i == 0 ? (pedra ? 76 : 72) : i == W - 1 ? (pedra ? 79 : 75) : (pedra ? 77 : 73), x, y);
            if (!porta && (i == 1 || i == 3) && j == 2) nossa(JANELA_ESTREITA, x, y);
        }
    if (tijolo) { nossa(EMPENA_ESCALONADA, nx + P, ny + P); nossa(EMPENA_ESCALONADA + 1, nx + 2 * P, ny + P); }
    else telhado(t, 6, nx + 2 * P, ny + P);
    if (inverno) DrawRectangleV({ nx + 1, ny + 1 }, { W * P - 2.0f, 7 }, NEVE_FORTE);
    const float tx = nx + 2, base = ny + 4 * P;
    const int andares = nivel >= 4 ? 3 : 2;
    for (int k = 0; k < andares; ++k) nossa(tijolo ? TORRE_TIJOLO + 1 : TORRE_MADEIRA + 1, tx, base - P * (k + 1));
    nossa(tijolo ? TORRE_TIJOLO : TORRE_MADEIRA, tx, base - P * (andares + 1));
}

// Casa do conselho: arcada no térreo nos níveis de madeira e pedra (as
// câmaras hanseáticas tinham o mercado coberto embaixo), fachada em degraus
// no tijolo, e a bandeira de Lübeck.
void conselho(int nivel, float nx, float ny) {
    const Estilo e = nivel <= 1 ? Estilo{ Telhado::Palha, Parede::Madeira, false, false }
                   : nivel == 2 ? Estilo{ Telhado::Telha, Parede::Pedra, false, true }
                                : Estilo{ Telhado::Telha, Parede::Tijolo, false, nivel >= 4 };
    compor(e, nx, ny, 2, 2);
    if (nivel <= 2) {
        town(nivel == 1 ? 74 : 78, nx + P, ny + 3 * P);
        town(nivel == 1 ? 74 : 78, nx + 3 * P, ny + 3 * P);
    }
    if (nivel >= 3) { nossa(EMPENA_ESCALONADA, nx + P, ny + 2 * P - 6); nossa(EMPENA_ESCALONADA + 1, nx + 2 * P, ny + 2 * P - 6); }
    nossa(BANDEIRA, nx + 4 * P - 8, ny + P - 6);
    if (nivel >= 4) nossa(BANDEIRA, nx - 6, ny + P - 6);
}

void cerca_em_volta(float x0, float y0, int W, int H) {
    for (int i = 0; i < W; ++i) {
        const int n = i == 0 ? 80 : i == W - 1 ? 82 : 81;
        town(n, x0 + i * P, y0 - 6);
        town(n, x0 + i * P, y0 + H * P - 12);
    }
    for (int j = 1; j < H; ++j) { town(58, x0 - 4, y0 + j * P - 10); town(58, x0 + W * P - 12, y0 + j * P - 10); }
}

// Alpendre de palha baixo e largo (3 peças x 2): beiral e uma parede só.
void abrigo(float x, float y) {
    for (int i = 0; i < 3; ++i) telhado(Telhado::Palha, 3 + (i == 0 ? 0 : i == 2 ? 2 : 1), x + i * P, y);
    town(72, x, y + P); town(74, x + P, y + P); town(75, x + 2 * P, y + P);
}

void celeiro(float x0, float y0) {
    const int pecas[4][3] = { { 93, 94, 95 }, { 117, 118, 119 }, { 102, 103, 104 }, { 114, 126, 116 } };
    for (int j = 0; j < 4; ++j)
        for (int i = 0; i < 3; ++i)
            farm(pecas[j][i], x0 + P / 2 + i * P, y0 + j * P, inverno && j < 2 ? Color{ 214, 226, 236, 255 } : WHITE);
    if (inverno)
        for (int k = 0; k < 10; ++k) {
            DrawRectangleV({ x0 + P / 2 + 22.0f - k * 2, y0 + 3.0f + k * 2 }, { 4.0f + k, 2 }, NEVE_FORTE);
            DrawRectangleV({ x0 + P / 2 + 26.0f, y0 + 3.0f + k * 2 }, { 4.0f + k, 2 }, NEVE_FORTE);
        }
}

// Centeio: brota no outono, cresce na primavera, amadurece em julho e é
// colhido em agosto. A cevada usa a peça própria quando madura.
int estagio_do_trigo(int mes) {
    if (mes == 7) return 66;
    if (mes == 8) return 67;
    if (mes >= 3 && mes <= 6) return 65;
    return 64;
}

// Primeira direção (N, L, S, O) em que há água a até `alcance` células da
// borda do lote, e a distância. Moinho, porto e pesca se viram para ela.
bool lado_da_agua(const Cidade& cidade, const Edificio& e, const DefinicaoEdificio& def, int alcance, int& lado, int& dist) {
    const Mapa& mapa = cidade.mapa();
    const int W = def.largura_total(), H = def.altura_total();
    for (int k = 1; k <= alcance; ++k)
        for (int l = 0; l < 4; ++l) {
            const int n = (l == 0 || l == 2) ? W : H;
            for (int t = 0; t < n; ++t) {
                int x, y;
                if (l == 0) { x = e.origem.x + t; y = e.origem.y - k; }
                else if (l == 2) { x = e.origem.x + t; y = e.origem.y + H - 1 + k; }
                else if (l == 3) { x = e.origem.x - k; y = e.origem.y + t; }
                else { x = e.origem.x + W - 1 + k; y = e.origem.y + t; }
                if (mapa.dentro(x, y) && mapa.terreno(x, y) == Terreno::Agua) { lado = l; dist = k; return true; }
            }
        }
    return false;
}

void moinho(const Cidade& cidade, const Edificio& e, const DefinicaoEdificio& def, float nx, float ny) {
    compor(estilo_de("moinho", def.fase, 0), nx, ny, def.largura, def.altura);
    // Roda d'água no lado que dá para a água (só leste ou oeste: de frente
    // ou de costas ela sumiria atrás do telhado), com um canal até o rio.
    int lado = 1, dist = 1;
    int l, d;
    if (lado_da_agua(cidade, e, def, 4, l, d) && (l == 1 || l == 3)) { lado = l == 3 ? -1 : 1; dist = d; }
    const float bw = def.largura * C, bh = def.altura * C;
    const float yc = ny + bh - P;
    const float xb = lado < 0 ? nx : nx + bw;
    for (int k = 0; k < 2 * (dist - 1) + 1; ++k) {
        const float xc = lado < 0 ? xb - (k + 1) * P : xb + k * P;
        nossa(AGUA + (k % 2), xc, yc);
        nossa(MARGEM, xc, yc);
        nossa(MARGEM + 2, xc, yc);
    }
    const float rx = lado < 0 ? xb - 2 * P - 4 : xb - 12;
    for (int k = 0; k < 4; ++k) nossa(RODA + k, rx + (k % 2) * P, yc - P + (k / 2) * P);
}

// Píer da borda do lote até a água, na direção dela, com barco na ponta.
void pier_ate_agua(const Cidade& cidade, const Edificio& e, const DefinicaoEdificio& def, float nx, float ny, bool barco) {
    int lado = 2, dist = 1;
    lado_da_agua(cidade, e, def, 3, lado, dist);
    const float bw = def.largura * C, bh = def.altura * C;
    // Começo do píer: meio da borda do lote que dá para a água.
    float x = nx + bw / 2 - P, y = ny + bh / 2 - P;
    if (lado == 0) y = ny - P;
    if (lado == 2) y = ny + bh;
    if (lado == 3) x = nx - P;
    if (lado == 1) x = nx + bw;
    const int passos = 2 * dist;
    for (int k = 0; k < passos; ++k) {
        const float px = x + dx4[lado] * k * P, py = y + dy4[lado] * k * P;
        nossa(PIER, px, py);
        if (lado == 0 || lado == 2) nossa(PIER, px + P, py);
        else nossa(PIER, px, py + P);
    }
    if (barco) {
        const float bx = x + dx4[lado] * passos * P + (lado == 0 || lado == 2 ? -P / 2.0f : 0);
        const float by = y + dy4[lado] * passos * P + (lado == 1 || lado == 3 ? P / 2.0f : 0);
        nossa(BARCO, bx, by);
        nossa(BARCO + 1, bx + P, by);
    }
}

void desenhar_edificio(const Cidade& cidade, IdEdificio id, Fase fase, int mes) {
    const Edificio& e = cidade.edificio(id);
    const DefinicaoEdificio& def = cidade.definicao(id);
    const std::string& tipo = def.id;
    const float ox = e.origem.x * C, oy = e.origem.y * C;

    // Lavoura em volta (fazendas): terra arada, a planta no estágio do mês e
    // uma cerca com passagem na frente da porta.
    if (def.campos > 0) {
        const bool cevada = tipo == "fazenda_cevada";
        for (int cy = 0; cy < def.altura_total(); ++cy)
            for (int cx = 0; cx < def.largura_total(); ++cx) {
                if (!(cx < def.campos || cy < def.campos || cx >= def.largura_total() - def.campos ||
                      cy >= def.altura_total() - def.campos))
                    continue;
                for (int j = 0; j < 2; ++j)
                    for (int i = 0; i < 2; ++i) {
                        const float px = ox + cx * C + i * P, py = oy + cy * C + j * P;
                        farm(25, px, py);
                        if (cevada && (mes == 7 || mes == 8)) nossa(CEVADA, px, py);
                        else farm(estagio_do_trigo(mes), px, py, cevada ? Color{ 230, 240, 180, 255 } : WHITE);
                        if (inverno) DrawRectangleV({ px, py }, { P, P }, NEVE);
                    }
            }
        const int Wt = 2 * def.largura_total(), Ht = 2 * def.altura_total();
        const int passagem = Wt / 2;
        for (int i = 0; i < Wt; ++i) {
            const int topo = i == 0 ? 80 : i == Wt - 1 ? 82 : 81;
            town(topo, ox + i * P, oy - 6);
            if (i != passagem && i != passagem - 1)
                town(i == passagem + 1 ? 80 : i == passagem - 2 ? 82 : topo, ox + i * P, oy + Ht * P - 10);
        }
        for (int j = 0; j < Ht; ++j) { town(58, ox - 8, oy + j * P - 6); town(58, ox + Wt * P - 8, oy + j * P - 6); }
    }

    const float nx = ox + def.campos * C, ny = oy + def.campos * C;
    const float bw = def.largura * C, bh = def.altura * C;
    const bool cercado = tipo == "curral" || tipo == "ovelheiro" || tipo == "pocilga" || tipo == "poco_argila" ||
                         tipo == "salina" || tipo == "porto" || tipo == "mercado";
    if (!cercado) sombra(nx + bw / 2 + 3, ny + bh - 1, bw / 2 + 2, 5);
    const float sorte = ruido::hash(e.origem.x, e.origem.y, 5);
    const Estilo est = estilo_de(tipo, tipo == "casa" ? fase : def.fase, sorte);
    const Estilo tijolo_telha{ Telhado::Telha, Parede::Tijolo, true, false };

    if (tipo == "fazenda" || tipo == "fazenda_cevada") {
        celeiro(nx, ny);
    } else if (tipo == "igreja") {
        igreja(e.nivel, nx, ny);
    } else if (tipo == "governo") {
        conselho(e.nivel, nx, ny);
    } else if (tipo == "armazem") {
        compor(est, nx, ny, def.largura, def.altura);
        const float yp = ny + bh - P;
        town(74, nx + (def.largura - 1) * P, yp);
        town(74, nx + def.largura * P, yp);
        town(107, nx - 12, ny + bh - 14);
        farm(85, nx + bw - 4, ny + bh - 15);
        farm(96, nx + bw - 2, ny + bh - 28);
    } else if (tipo == "pocilga") {
        for (int i = 1; i < 3; ++i) nossa(PALHA + (i == 1 ? 0 : 2), nx + i * P, ny);
        for (int i = 1; i < 3; ++i) nossa(PALHA + 3 + (i == 1 ? 0 : 2), nx + i * P, ny + P);
        nossa(LAMA, nx + 4, ny + 2 * P);
        nossa(LAMA, nx + 2 * P, ny + 2 * P + 6);
        nossa(PORCO, nx + 6, ny + 2 * P - 2);
        nossa(PORCO, nx + 2 * P + 4, ny + 2 * P + 8);
        for (int i = 0; i < 4; ++i) town(i == 0 ? 80 : i == 3 ? 82 : 81, nx + i * P, ny + 4 * P - 12);
        for (int j = 1; j < 4; ++j) { town(58, nx - 4, ny + j * P - 10); town(58, nx + 4 * P - 12, ny + j * P - 10); }
    } else if (tipo == "lenhador") {
        // Galpão aberto: telhado, frente sem parede e lenha à mostra.
        const float yb = ny + P;
        for (int i = 0; i < 4; ++i) nossa(PALHA + (i == 0 ? 0 : i == 3 ? 2 : 1), nx + i * P, yb);
        for (int i = 0; i < 4; ++i) nossa(PALHA + 3 + (i == 0 ? 0 : i == 3 ? 2 : 1), nx + i * P, yb + P);
        for (int i = 0; i < 4; ++i) town(i == 0 ? 72 : i == 3 ? 75 : 74, nx + i * P, yb + 2 * P);
        for (int i = 1; i < 3; ++i) { town(106, nx + i * P, yb + 2 * P + 4); town(106, nx + i * P + 2, yb + 2 * P - 2); }
        nossa(CEPO, nx + 4 * P - 4, yb + 2 * P + 2);
    } else if (tipo == "moinho") {
        moinho(cidade, e, def, nx, ny);
    } else if (tipo == "padaria") {
        compor(est, nx, ny, def.largura, def.altura);
        sombra(nx - 6, ny + bh - 2, 9, 3);
        nossa(FORNO, nx - 14, ny + bh - 16);
        nossa(PLACA_PAO, nx + bw / 2 + P - 2, ny + bh - 2 * P + 4);
    } else if (tipo == "pescador") {
        compor(est, nx, ny, def.largura, def.altura);
        nossa(VARAL_PEIXE, nx - 14, ny + bh - 18);
        nossa(VARAL_PEIXE, nx - 14, ny + bh - 34);
        farm(85, nx + bw - 4, ny + bh - 15);
    } else if (tipo == "malteria") {
        compor({ Telhado::Telha, Parede::Pedra, false, false }, nx, ny, 2, 2);
        nossa(RESPIRO, nx + 4, ny + 4); nossa(RESPIRO, nx + 2 * P + 10, ny + 4);
        farm(97, nx - 10, ny + 4 * P - 14);
    } else if (tipo == "cervejaria") {
        compor({ Telhado::Telha, Parede::Madeira, true, true }, nx, ny, 2, 2);
        nossa(PLACA_CANECA, nx + P - 2, ny + 3 * P - 14);
        farm(85, nx - 12, ny + 4 * P - 15); farm(85, nx - 12, ny + 4 * P - 28); farm(85, nx + 4 * P - 4, ny + 4 * P - 15);
    } else if (tipo == "curral") {
        cerca_em_volta(nx, ny, 2 * def.largura, 2 * def.altura);
        abrigo(nx + 2, ny + 2);
        farm(110, nx + 3 * P, ny + P);
        farm(121, nx + 2 * P + 6, ny + 2 * P + 4); farm(121, nx + 4 * P, ny + 2 * P);
    } else if (tipo == "ovelheiro") {
        cerca_em_volta(nx, ny, 2 * def.largura, 2 * def.altura);
        abrigo(nx + 2, ny + 2);
        farm(120, nx + 2 * P + 4, ny + 2 * P - 2); farm(120, nx + 6, ny + 3 * P - 4); farm(120, nx + 2 * P + 10, ny + 3 * P - 2);
    } else if (tipo == "carvoaria") {
        abrigo(nx + P, ny + 2);
        nossa(MONTE_CARVAO, nx + 2, ny + 2 * P + 2); nossa(MONTE_CARVAO, nx + 14, ny + 2 * P + 6); nossa(MONTE_CARVAO, nx + 6, ny + 3 * P + 2);
        town(106, nx + 2 * P + 4, ny + 3 * P + 2); town(106, nx + 3 * P, ny + 3 * P + 2);
    } else if (tipo == "cortador_turfa") {
        abrigo(nx, ny + 2);
        nossa(PILHA_TURFA, nx + 2 * P + 2, ny + P); nossa(PILHA_TURFA, nx + 2 * P + 6, ny + 2 * P + 4); nossa(PILHA_TURFA, nx + 6, ny + 3 * P);
        nossa(POCA_JUNCO, nx + 3 * P, ny + 3 * P - 2);
    } else if (tipo == "mercado") {
        for (int j = 0; j < 2 * def.altura; ++j)
            for (int i = 0; i < 2 * def.largura; ++i) town(43, nx + i * P, ny + j * P, tom_da_estacao());
        for (int k = 0; k < 2; ++k) {
            const float x = nx + k * 2 * P, y = ny + k * P + 4;
            nossa(BALCAO, x + 2, y + P - 2); nossa(BALCAO, x + P, y + P - 2);
            nossa(TOLDO, x, y); nossa(TOLDO + 1, x + P, y);
        }
    } else if (tipo == "tecelagem") {
        compor({ Telhado::Ardosia, Parede::Tijolo, false, false }, nx, ny, 2, 2);
        nossa(EMPENA_ESCALONADA, nx + P, ny + P); nossa(EMPENA_ESCALONADA + 1, nx + 2 * P, ny + P);
        nossa(VARAL_TECIDOS, nx - 14, ny + 4 * P - 18);
    } else if (tipo == "curtume") {
        compor({ Telhado::Telha, Parede::Pedra, false, true }, nx, ny, 2, 2);
        nossa(TINA, nx - 12, ny + 4 * P - 14); nossa(TINA, nx - 4, ny + 4 * P - 4);
        nossa(COURO, nx + 4 * P - 4, ny + 4 * P - 18);
    } else if (tipo == "salina") {
        abrigo(nx + P / 2, ny);
        for (int j = 0; j < 2; ++j)
            for (int i = 0; i < 2; ++i) nossa(TANQUE_SAL, nx + 2 + i * (P + 12), ny + 2 * P + 2 + j * (P - 2));
        town(107, nx + 4 * P - 8, ny + 2 * P);
    } else if (tipo == "poco_argila") {
        for (int j = 0; j < 4; ++j)
            for (int i = 0; i < 4; ++i)
                if (!((i == 0 || i == 3) && (j == 0 || j == 3))) town(25, nx + i * P, ny + j * P, tom_da_estacao());
        nossa(BURACO_ARGILA, nx + P / 2, ny + P);
        nossa(BURACO_ARGILA, nx + 2 * P, ny + 2 * P);
        nossa(MONTE_ARGILA, nx + 4, ny + 3 * P); nossa(MONTE_ARGILA, nx + 3 * P, ny + 4);
        town(106, nx + 3 * P, ny + 3 * P + 2);
    } else if (tipo == "olaria") {
        compor({ Telhado::Telha, Parede::Tijolo, false, true }, nx, ny, 2, 2);
        nossa(FORNO_OLARIA, nx + 4 * P + 4, ny + 2 * P); nossa(FORNO_OLARIA + 1, nx + 4 * P + 4, ny + 3 * P);
        nossa(PILHA_TIJOLOS, nx + 5 * P, ny + 3 * P + 2); nossa(PILHA_TIJOLOS, nx + 5 * P + 6, ny + 3 * P - 6);
    } else if (tipo == "tanoeiro") {
        compor({ Telhado::Telha, Parede::Madeira, true, false }, nx, ny, 2, 2);
        for (int k = 0; k < 3; ++k) farm(85, nx - 12 + (k % 2) * 6, ny + 4 * P - 15 - (k / 2) * 12);
        town(106, nx + 4 * P - 4, ny + 4 * P - 12);
    } else if (tipo == "salgadeira") {
        compor({ Telhado::Ardosia, Parede::Tijolo, false, false }, nx, ny, 2, 2);
        farm(85, nx - 12, ny + 4 * P - 15); farm(85, nx + 4 * P - 4, ny + 4 * P - 15);
        nossa(TANQUE_SAL, nx + 4 * P - 4, ny + 4 * P - 30);
    } else if (tipo == "pescador_arenque") {
        compor({ Telhado::Ardosia, Parede::Madeira, false, true }, nx, ny, 2, 2);
        nossa(REDE, nx - 14, ny + 4 * P - 20);
        pier_ate_agua(cidade, e, def, nx, ny, true);
    } else if (tipo == "porto") {
        // Plataforma de tábuas no lote, guindaste, fardos e píer até a água.
        for (int j = 0; j < 2 * def.altura; ++j)
            for (int i = 0; i < 2 * def.largura; ++i) nossa(PIER, nx + i * P, ny + j * P);
        pier_ate_agua(cidade, e, def, nx, ny, true);
        nossa(GUINDASTE, nx + 4 * P, ny - 16 + P); nossa(GUINDASTE + 1, nx + 4 * P, ny + P + P);
        farm(96, nx + P, ny + P + 2); town(107, nx + 2 * P, ny + P + 4);
    } else {
        // Edifício sem desenho próprio (um tipo novo no conteúdo): estilo da fase.
        compor(def.fase >= Fase::Cidade ? tijolo_telha : est, nx, ny, def.largura, def.altura);
    }

    // Quintal das casas: cerquinha, canteiro, girassol ou colmeia.
    if (tipo == "casa") {
        if (est.estreita) {
            town(47, nx + 2, ny + bh - 16);
            if (!inverno) { farm(83, nx + 1, ny + bh - 30); town(2, nx, ny + bh - 12); }
        }
        const float hc = ruido::hash(e.origem.x, e.origem.y, 3);
        if (!(inverno && hc < 0.5f)) peca(hc < 0.5f ? Folha::Farm : Folha::Town, hc < 0.5f ? 83 : 94, nx + bw - 6, ny + bh - 15);
    }
    // Edifícios altos ganham um quintal com caixas e feno atrás.
    if (def.altura >= 3 && def.campos == 0 && !cercado) {
        farm(11, nx + 4, ny + 6);
        farm(96, nx + 22, ny + 4);
        farm(97, nx + bw - 22, ny + 6);
        town(107, nx + bw - 40, ny + 10);
    }
}

}  // namespace

bool Arte::carregar(const std::string& raiz, std::string& erro) {
    const std::string caminhos[3] = { raiz + "/dados/arte/pecas.png", raiz + "/vendor/kenney/tiny-town/tilemap_packed.png",
                                      raiz + "/vendor/kenney/tiny-farm/tilemap_packed.png" };
    Image folhas[3];
    for (int k = 0; k < 3; ++k) {
        folhas[k] = LoadImage(caminhos[k].c_str());
        if (folhas[k].data == nullptr) {
            erro = "não foi possível abrir " + caminhos[k] + " (rode make para baixar a arte)";
            for (int j = 0; j < k; ++j) UnloadImage(folhas[j]);
            return false;
        }
        ImageFormat(&folhas[k], PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    }
    // Monta a folha única com 1 px de folga repetida em volta de cada peça.
    const int linhas = (TOTAL_PECAS + COLUNAS_ATLAS - 1) / COLUNAS_ATLAS;
    const int largura = COLUNAS_ATLAS * CASA, altura = linhas * CASA;
    std::vector<Color> pixels(static_cast<std::size_t>(largura * altura), BLANK);
    auto copiar = [&](int global, const Image& folha, int sx, int sy, bool branca) {
        const Color* src = static_cast<const Color*>(folha.data);
        const int cx = (global % COLUNAS_ATLAS) * CASA, cy = (global / COLUNAS_ATLAS) * CASA;
        for (int y = -1; y <= P; ++y)
            for (int x = -1; x <= P; ++x) {
                const int ix = std::clamp(x, 0, P - 1), iy = std::clamp(y, 0, P - 1);
                const Color c = branca ? WHITE : src[(sy + iy) * folha.width + (sx + ix)];
                pixels[static_cast<std::size_t>((cy + y + 1) * largura + (cx + x + 1))] = c;
            }
    };
    for (int n = 0; n < PECAS_NOSSAS; ++n) copiar(n, folhas[0], n * P, 0, false);
    for (int n = 0; n < PECAS_TINY; ++n) {
        copiar(BASE_TOWN + n, folhas[1], (n % 12) * P, (n / 12) * P, false);
        copiar(BASE_FARM + n, folhas[2], (n % 12) * P, (n / 12) * P, false);
    }
    copiar(PECA_BRANCA, folhas[0], 0, 0, true);
    for (Image& f : folhas) UnloadImage(f);

    Image unica{ pixels.data(), largura, altura, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    atlas_ = LoadTextureFromImage(unica);
    if (atlas_.id == 0) {
        erro = "não foi possível criar a textura da arte";
        return false;
    }
    SetTextureFilter(atlas_, TEXTURE_FILTER_POINT);
    // Retângulos (neve, sombras de chão) passam a usar o pixel branco da
    // própria folha: sem troca de textura, a raylib não quebra o agrupamento.
    const Rectangle branca = origem(PECA_BRANCA);
    SetShapesTexture(atlas_, { branca.x + 4, branca.y + 4, 1, 1 });
    return true;
}

void Arte::descarregar() {
    if (atlas_.id != 0) UnloadTexture(atlas_);
    atlas_ = {};
}

void Arte::desenhar_terreno(const Cidade& cidade, int x0, int y0, int x1, int y1, Estacao estacao) const {
    atlas = atlas_;
    outono = estacao == Estacao::Outono;
    inverno = estacao == Estacao::Inverno;
    const Mapa& mapa = cidade.mapa();
    x0 = std::max(0, x0);
    y0 = std::max(0, y0);
    x1 = std::min(mapa.largura() - 1, x1);
    y1 = std::min(mapa.altura() - 1, y1);

    // 1. Chão: água, grama (com flores em grupos e tufos) e a base das trilhas.
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) {
            const Terreno t = mapa.terreno(x, y);
            const bool estrada = cidade.tem_estrada({ x, y });
            subpecas(x, y, [&](int sx, int sy, float px, float py) {
                const float h = ruido::hash(sx, sy, 7);
                if (t == Terreno::Agua) { nossa(AGUA + (h < 0.5f ? 0 : 1), px, py); return; }
                if (estrada) { town(0, px, py, tom_da_estacao()); return; }
                const float prado = ruido::valor(static_cast<float>(sx), static_cast<float>(sy), 7.0f, 51);
                const float chance_flor = (prado > 0.68f ? 0.35f : 0.01f) * (outono ? 0.25f : 1.0f);
                const int n = (t == Terreno::Campo && !inverno && h < chance_flor) ? 2 : h < 0.2f ? 1 : 0;
                town(n, px, py, tom_da_estacao());
            });
        }
    // 2. Margem de areia e espuma onde a água toca a terra.
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) {
            if (mapa.terreno(x, y) != Terreno::Agua) continue;
            for (int lado = 0; lado < 4; ++lado) {
                const int vx = x + dx4[lado], vy = y + dy4[lado];
                if (!mapa.dentro(vx, vy) || mapa.terreno(vx, vy) == Terreno::Agua) continue;
                for (int k = 0; k < 2; ++k) {
                    const int i = lado == 1 ? 1 : lado == 3 ? 0 : k;
                    const int j = lado == 0 ? 0 : lado == 2 ? 1 : k;
                    nossa(MARGEM + lado, static_cast<float>(x * C + i * P), static_cast<float>(y * C + j * P));
                }
            }
        }
    if (inverno)
        for (int y = y0; y <= y1; ++y)
            for (int x = x0; x <= x1; ++x)
                if (mapa.terreno(x, y) != Terreno::Agua)
                    DrawRectangleV({ static_cast<float>(x * C), static_cast<float>(y * C) }, { C, C }, NEVE);
    // 3. Caminho de pedras com uma peça de largura: uma no meio da célula e
    //    meia peça em direção a cada vizinho com estrada.
    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) {
            if (!cidade.tem_estrada({ x, y })) continue;
            const float cx = static_cast<float>(x * C + P / 2), cy = static_cast<float>(y * C + P / 2);
            auto pedra = [&](float px, float py) {
                town(43, px, py, tom_da_estacao());
                if (inverno) DrawRectangleV({ px, py }, { P, P }, NEVE);
            };
            pedra(cx, cy);
            for (int lado = 0; lado < 4; ++lado)
                if (cidade.tem_estrada({ x + dx4[lado], y + dy4[lado] })) pedra(cx + dx4[lado] * P, cy + dy4[lado] * P);
        }
    // 4. Objetos do terreno, de cima para baixo (o de baixo cobre o de cima).
    for (int sy = 2 * y0; sy <= 2 * y1 + 1; ++sy)
        for (int sx = 2 * x0; sx <= 2 * x1 + 1; ++sx) {
            const int x = sx / 2, y = sy / 2;
            if (cidade.tem_estrada({ x, y }) || cidade.edificio_em({ x, y }) != SEM_EDIFICIO) continue;
            const float h = ruido::hash(sx, sy, 11);
            const float px = static_cast<float>(sx * P), py = static_cast<float>(sy * P);
            const float grupo = ruido::valor(static_cast<float>(sx), static_cast<float>(sy), 5.0f, 41);
            switch (mapa.terreno(x, y)) {
                case Terreno::Floresta: {
                    // Mata em 9 partes: cantos e bordas onde a subpeça vizinha
                    // não é floresta livre, miolo no resto.
                    auto mata = [&](int ax, int ay) {
                        const int cx = ax >= 0 ? ax / 2 : -1, cy = ay >= 0 ? ay / 2 : -1;
                        return mapa.dentro(cx, cy) && mapa.terreno(cx, cy) == Terreno::Floresta &&
                               !cidade.tem_estrada({ cx, cy }) && cidade.edificio_em({ cx, cy }) == SEM_EDIFICIO;
                    };
                    const int linha = !mata(sx, sy - 1) ? 0 : !mata(sx, sy + 1) ? 2 : 1;
                    const int coluna = !mata(sx - 1, sy) ? 0 : !mata(sx + 1, sy) ? 2 : 1;
                    const int base = outono && ruido::valor(static_cast<float>(sx), static_cast<float>(sy), 4.0f, 61) < 0.7f ? 9 : 6;
                    town(base + linha * 12 + coluna, px, py, tom_da_estacao());
                    if (inverno) {
                        DrawRectangleV({ px, py }, { P, P }, Color{ 244, 248, 255, 70 });
                        DrawRectangleV({ px + 3, py + 2 }, { 10, 3 }, NEVE_FORTE);
                    }
                    if (linha == 1 && coluna == 1 && h < 0.3f) {
                        sombra(px + 8, py + 13, 6, 2);
                        town(outono ? 15 : 16, px + (ruido::hash(sx, sy, 15) - 0.5f) * 6, py - 5);
                    }
                    break;
                }
                case Terreno::Barreiro:
                    if (grupo > 0.55f && h < 0.28f) nossa(MONTE_ARGILA, px, py);
                    else if (h < 0.08f) nossa(PEDRINHAS, px, py);
                    break;
                case Terreno::Turfa:
                    if (grupo > 0.6f && h < 0.55f) nossa(POCA_JUNCO, px, py);
                    break;
                case Terreno::Campo: {
                    // Perto da mata as árvores escapam para o campo.
                    float perto = 0.0f;
                    for (int r = 1; r <= VIZINHANCA_DO_PEDACO && perto == 0.0f; ++r)
                        for (int k = 0; k < 4; ++k) {
                            const int vx = x + dx4[k] * r, vy = y + dy4[k] * r;
                            if (mapa.dentro(vx, vy) && mapa.terreno(vx, vy) == Terreno::Floresta) perto = 1.0f / r;
                        }
                    const float chance = 0.004f + 0.12f * perto * perto;
                    if (h < chance) {
                        sombra(px + 8, py + 14, 6, 2);
                        const float t = ruido::hash(sx, sy, 14);
                        town(outono ? (t < 0.65f ? 15 : 27) : (t < 0.65f ? 16 : 28), px, py - 4,
                             inverno ? Color{ 170, 196, 190, 255 } : WHITE);
                    } else if (h < chance + 0.004f) town(5, px, py);
                    else if (h < chance + 0.007f) nossa(PEDRINHAS, px, py);
                    break;
                }
                case Terreno::Agua: break;
            }
        }
}

void Arte::desenhar_edificios(const Cidade& cidade, Fase fase, int mes, Rectangle visivel) const {
    atlas = atlas_;
    const Estacao estacao = estacao_do_mes(mes);
    outono = estacao == Estacao::Outono;
    inverno = estacao == Estacao::Inverno;
    // Em ordem de linha da base: o edifício mais embaixo na tela é desenhado
    // por último e cobre o de cima (torres e chaminés passam do lote).
    std::vector<IdEdificio> ordem;
    for (IdEdificio id = 0; id < cidade.edificios().size(); ++id) {
        const Edificio& e = cidade.edificio(id);
        if (!e.ativo) continue;
        const DefinicaoEdificio& def = cidade.definicao(id);
        // Folga de uma célula e meia: objetos e píer passam da borda do lote.
        const Rectangle r{ (e.origem.x - 1.5f) * C, (e.origem.y - 1.5f) * C, (def.largura_total() + 3.0f) * C,
                           (def.altura_total() + 3.0f) * C };
        if (CheckCollisionRecs(r, visivel)) ordem.push_back(id);
    }
    std::stable_sort(ordem.begin(), ordem.end(), [&cidade](IdEdificio a, IdEdificio b) {
        const auto base = [&cidade](IdEdificio id) { return cidade.edificio(id).origem.y + cidade.definicao(id).altura_total(); };
        return base(a) < base(b);
    });
    for (const IdEdificio id : ordem) desenhar_edificio(cidade, id, fase, mes);
}

void Arte::desenhar_carregadores(const Cidade& cidade, const Transporte& transporte, Rectangle visivel) const {
    (void)cidade;
    atlas = atlas_;
    for (const Carregador& c : transporte.carregadores()) {
        // Parado em casa fica escondido dentro do depósito.
        if (c.livre() && !c.andando()) continue;
        Vector2 p{ static_cast<float>(c.posicao.x * C), static_cast<float>(c.posicao.y * C) };
        if (c.andando()) {
            const Celula prox = c.rota[c.passo];
            const float f = static_cast<float>(c.ticks_na_celula) / Transporte::TICKS_POR_CELULA;
            p.x += static_cast<float>(prox.x - c.posicao.x) * C * f;
            p.y += static_cast<float>(prox.y - c.posicao.y) * C * f;
        }
        if (!CheckCollisionPointRec({ p.x + C / 2.0f, p.y + C / 2.0f }, visivel)) continue;
        const float px = p.x + 8, py = p.y + 6;
        sombra(px + 8, py + 15, 5, 2);
        farm(108, px, py);
        if (c.carregando) town(107, px + 5, py - 6);
    }
}
