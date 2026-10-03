"""Peças próprias do Hansestadt em pixel art 16x16, na paleta e no contorno
dos pacotes Tiny Town e Tiny Farm da Kenney.

Gera dados/arte/pecas.png, uma linha de peças de 16x16 na ordem abaixo. O
jogo junta essa folha com as da Kenney (vendor/kenney) numa textura só.
Determinístico: rodar de novo gera a mesma imagem byte a byte, então a
imagem fica no repositório e quem só compila o jogo não precisa de Python
nem de Pillow. Rodar de novo só quando uma peça mudar:

    python3 ferramentas/gerar_pecas.py dados/arte/pecas.png vendor/kenney/tiny-town/tilemap_packed.png

As peças de palha (20-27) são as telhas do Tiny Town recoloridas; por isso
o script lê a folha da Kenney.

Ordem das peças:
  0-1 água (duas fases de ondulação)    2 barreiro (argila)
  3 turfa (brejo)                        4 parede de tijolo
  5 parede de tijolo com janela          6 parede de tijolo com porta
  7 estrada calçada de tijolo
  8-11  margem de água (por cima da água, no lado que toca a terra): N, L, S, O
  12-15 franja de grama (por cima de terra, barreiro ou turfa, no lado que
        toca a grama): N, L, S, O
  16 monte de argila (objeto sobre a grama, no barreiro)
  17 poça de brejo com junco (objeto sobre a grama, na turfa)
  18 pedrinhas soltas
  19 chaminé de tijolo com fumaça (por cima do telhado)
  20-27 telhado de palha: as peças de telha do Tiny Town (52, 53, 54, 64,
        65, 66, 67, 55) recoloridas em tons de palha. As casas de uma vila de
        1150 eram cobertas de palha; telha e tijolo vêm com as fases.
  28 varal de peixe secando           29-32 roda d'água 2x2 (NO, NE, SO, SE)
  33 porco                            34 poça de lama
  35 cepo com machado
  36 forno de barro (padaria)         37 placa com pão
  38-39 torre de madeira com cruz (alto, base)
  40-41 torre de tijolo com pináculo de telha (alto, base)
  42 bandeira da Hansa (vermelho e branco) no mastro
  43 parede de tijolo com janela gótica (arco em ponta)
  44-45 empena escalonada de tijolo (Treppengiebel), metades esquerda e direita
  46 janela estreita em arco (por cima de parede de madeira ou pedra do Tiny Town)
  47 placa com caneca (cervejaria)    48 monte de carvão fumegando (carvoaria)
  49 pilha de blocos de turfa         50-51 toldo listrado de barraca (esq., dir.)
  52 balcão com mercadorias           53 respiro de secagem no telhado (malteria)
  54 cevada madura (espigas caídas)   55 varal de tecidos coloridos (tecelagem)
  56 tina de curtume                  57 couro esticado na moldura (curtume)
  58 tanque de sal (salina)           59 buraco de argila (poço de argila)
  60 pilha de tijolos                 61-62 forno de olaria (alto, base)
  63-64 barco (proa, popa)            65 rede de pesca estendida
  66 píer de tábuas (chão)            67-68 guindaste de madeira (alto, base)
  69 sombra (elipse suave, esticada conforme o tamanho de quem a projeta)
"""
import random
import sys

from PIL import Image

T = 16
CONTORNO = (61, 37, 59, 255)       # o roxo-escuro do contorno dos Tiny

def nova():
    return Image.new("RGBA", (T, T), (0, 0, 0, 0))

def preencher(im, cor):
    for y in range(T):
        for x in range(T):
            im.putpixel((x, y), cor)

def salpicar(im, rnd, cores, n):
    for _ in range(n):
        im.putpixel((rnd.randrange(T), rnd.randrange(T)), rnd.choice(cores))

def agua(fase):
    rnd = random.Random(10 + fase)
    im = nova()
    preencher(im, (79, 164, 229, 255))
    # Ondas: traços horizontais curtos mais claros, deslocados por fase.
    for y in range(2, T, 5):
        for x0 in range((y * 3 + fase * 4) % 7, T, 8):
            for dx in range(3):
                if x0 + dx < T:
                    im.putpixel((x0 + dx, y), (148, 208, 248, 255))
    salpicar(im, rnd, [(64, 140, 210, 255)], 10)
    return im

def barreiro():
    rnd = random.Random(2)
    im = nova()
    preencher(im, (190, 112, 74, 255))
    salpicar(im, rnd, [(165, 92, 60, 255), (214, 140, 96, 255)], 40)
    # Seixos de argila.
    for (x, y) in [(3, 4), (10, 9), (6, 12)]:
        im.putpixel((x, y), (120, 66, 48, 255))
        im.putpixel((x + 1, y), (226, 160, 116, 255))
    return im

def turfa():
    rnd = random.Random(3)
    im = nova()
    preencher(im, (94, 74, 56, 255))
    salpicar(im, rnd, [(78, 60, 46, 255), (112, 90, 66, 255)], 50)
    # Tufos de capim do brejo.
    for (x, y) in [(2, 6), (9, 3), (12, 11), (5, 13)]:
        for dy in range(3):
            im.putpixel((x, y - dy), (108, 150, 72, 255))
        im.putpixel((x - 1, y - 1), (90, 130, 60, 255))
        im.putpixel((x + 1, y - 1), (90, 130, 60, 255))
    return im

def tijolos(im, y0=0, y1=T):
    """Aparelho de tijolos em amarração: fiadas de 4 px, juntas desencontradas."""
    argamassa = (214, 186, 160, 255)
    for y in range(y0, y1):
        for x in range(T):
            fiada = (y - y0) // 4
            junta_h = (y - y0) % 4 == 3
            junta_v = (x + (4 if fiada % 2 else 0)) % 8 == 7
            if junta_h or junta_v:
                im.putpixel((x, y), argamassa)
            else:
                # Variação leve entre tijolos, como em parede de verdade.
                tom = ((x + (4 if fiada % 2 else 0)) // 8 + fiada) % 3
                im.putpixel((x, y), [(170, 64, 48, 255), (184, 74, 54, 255), (156, 58, 46, 255)][tom])

def parede_tijolo(com=None):
    im = nova()
    tijolos(im)
    # Rodapé e contorno inferior, como as paredes dos Tiny.
    for x in range(T):
        im.putpixel((x, T - 1), CONTORNO)
        im.putpixel((x, T - 2), (120, 50, 44, 255))
    if com == "janela":
        for y in range(4, 11):
            for x in range(5, 11):
                borda = y in (4, 10) or x in (5, 10)
                im.putpixel((x, y), CONTORNO if borda else (160, 200, 226, 255))
        for y in range(5, 10):
            im.putpixel((7, y), CONTORNO)
            im.putpixel((8, y), CONTORNO)
    if com == "porta":
        for y in range(3, T - 1):
            for x in range(4, 12):
                borda = y == 3 or x in (4, 11)
                im.putpixel((x, y), CONTORNO if borda else (126, 74, 52, 255))
        im.putpixel((9, 10), (230, 200, 120, 255))
    return im

def calcamento():
    im = nova()
    tijolos(im)
    # Mais escuro e sem rodapé: chão, não parede.
    for y in range(T):
        for x in range(T):
            r, g, b, a = im.getpixel((x, y))
            im.putpixel((x, y), (int(r * 0.85), int(g * 0.8), int(b * 0.8), a))
    return im

def girar(im, lado):
    """As peças de borda são desenhadas para o norte e giradas para os outros
    lados: 0=N, 1=L, 2=S, 3=O."""
    return im.rotate(-90 * lado) if lado else im

def margem_norte():
    rnd = random.Random(8)
    im = nova()
    areia = (236, 214, 158, 255)
    espuma = (214, 238, 252, 255)
    for x in range(T):
        fundo = 2 + (1 if rnd.random() < 0.35 else 0)  # borda irregular
        for y in range(fundo):
            im.putpixel((x, y), areia)
        im.putpixel((x, fundo), espuma)
        if rnd.random() < 0.3:
            im.putpixel((x, fundo + 1), espuma)
    return im

def franja_norte():
    rnd = random.Random(12)
    im = nova()
    grama = (130, 196, 102, 255)
    sombra = (96, 158, 78, 255)
    x = 0
    while x < T:
        largura = rnd.choice([2, 3, 4])
        fundo = rnd.choice([1, 2, 2, 3])
        for dx in range(largura):
            if x + dx >= T:
                break
            for y in range(fundo):
                im.putpixel((x + dx, y), grama)
            im.putpixel((x + dx, fundo), sombra)
        x += largura
    return im

def contorno_em_volta(im):
    """Contorno de 1 px em volta do que não é transparente, como nos Tiny."""
    alvo = []
    for y in range(T):
        for x in range(T):
            if im.getpixel((x, y))[3] != 0:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                vx, vy = x + dx, y + dy
                if 0 <= vx < T and 0 <= vy < T and im.getpixel((vx, vy))[3] == 255 and im.getpixel((vx, vy)) != CONTORNO:
                    alvo.append((x, y))
                    break
    for p in alvo:
        im.putpixel(p, CONTORNO)
    return im

def monte_argila():
    """Um monte só, redondo e baixo, com brilho de barro molhado em cima e
    uma poça de argila escura em volta da base."""
    im = nova()
    for y in range(T):
        for x in range(T):
            base = ((x - 8) ** 2) / 42 + ((y - 12) ** 2) / 6
            if base <= 1.0:
                im.putpixel((x, y), (150, 84, 58, 255))
    for y in range(T):
        for x in range(T):
            d = ((x - 8) ** 2) / 25 + ((y - 9) ** 2) / 12
            if d <= 1.0:
                cor = (222, 140, 92, 255) if d < 0.35 and y < 9 else (196, 112, 74, 255) if y < 11 else (170, 94, 64, 255)
                im.putpixel((x, y), cor)
    for (x, y) in [(6, 7), (7, 7), (7, 6)]:
        im.putpixel((x, y), (246, 190, 140, 255))  # brilho
    return contorno_em_volta(im)

def poca_junco():
    im = nova()
    for y in range(T):
        for x in range(T):
            d = ((x - 8) ** 2) / 36 + ((y - 11) ** 2) / 9
            if d <= 1.0:
                im.putpixel((x, y), (92, 140, 176, 255) if d > 0.5 else (112, 166, 204, 255))
    contorno_em_volta(im)
    # Juncos saindo da beira.
    for (x, h) in [(3, 6), (4, 4), (12, 5), (13, 7)]:
        for dy in range(h):
            im.putpixel((x, 11 - dy), (84, 120, 52, 255))
        im.putpixel((x, 11 - h), (150, 110, 60, 255))  # pendão
    return im

def pedrinhas():
    im = nova()
    for (x, y) in [(3, 10), (9, 6), (12, 12)]:
        for dx, dy in ((0, 0), (1, 0), (0, 1), (1, 1)):
            im.putpixel((x + dx, y + dy), (176, 176, 184, 255) if dy == 0 else (136, 136, 148, 255))
    return contorno_em_volta(im)

def chamine():
    im = nova()
    for y in range(6, 15):
        for x in range(9, 14):
            borda = x in (9, 13) or y == 14
            if borda:
                im.putpixel((x, y), CONTORNO)
            else:
                im.putpixel((x, y), (170, 64, 48, 255) if (y // 2 + x) % 3 else (214, 186, 160, 255))
    for x in range(8, 15):
        im.putpixel((x, 6), CONTORNO)
        im.putpixel((x, 7), (120, 50, 44, 255))
    # Fumaça subindo em três bolinhas.
    for (cx, cy, r) in [(11, 4, 1.6), (9, 2, 1.3), (12, 0, 1.0)]:
        for y in range(T):
            for x in range(T):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    im.putpixel((x, y), (232, 232, 236, 220))
    return im

def palha(folha_town, n):
    """Recolore uma peça de telha (vermelho) em palha (amarelo-palha), pela
    luminosidade: o desenho das telhas vira o das camadas de palha."""
    col = 12
    peca = folha_town.crop(((n % col) * T, (n // col) * T, (n % col) * T + T, (n // col) * T + T)).convert("RGBA")
    tons = [(118, 82, 44, 255), (164, 120, 60, 255), (204, 160, 84, 255), (232, 196, 118, 255)]
    for y in range(T):
        for x in range(T):
            r, g, b, a = peca.getpixel((x, y))
            if a == 0 or (r, g, b, a) == CONTORNO:
                continue
            avermelhado = r > g + 40 and r > b + 40
            if not avermelhado:
                continue  # mantém contorno, beiral cinza e madeira
            lum = (r * 3 + g * 6 + b) / 10
            i = 0 if lum < 110 else 1 if lum < 140 else 2 if lum < 170 else 3
            peca.putpixel((x, y), tons[i])
    return peca

def varal_peixe():
    """Varal de peixe defumado: moldura grossa de madeira e três peixes
    grandes, dourados, pendurados pelo rabo."""
    im = nova()
    madeira, escura = (168, 108, 62, 255), (118, 74, 44, 255)
    for y in range(2, 16):
        for x in (1, 2, 13, 14):
            im.putpixel((x, y), madeira if x in (1, 13) else escura)
    for x in range(1, 15):
        im.putpixel((x, 2), madeira)
        im.putpixel((x, 3), escura)
    corpo, sombra, barriga = (214, 160, 74, 255), (172, 118, 52, 255), (238, 204, 132, 255)
    for x0 in (4, 7, 10):
        im.putpixel((x0, 4), sombra); im.putpixel((x0 + 1, 4), sombra)        # rabo
        for y in range(5, 12):
            larg = 1 if y in (5, 11) else 2
            for dx in range(larg):
                im.putpixel((x0 + dx, y), barriga if dx == 1 else corpo)
        im.putpixel((x0, 12), sombra)                                          # cabeça
        im.putpixel((x0 + 1, 10), CONTORNO)                                    # olho
    return contorno_em_volta(im)

def roda_dagua():
    """Roda de 32x32 (quatro peças): aro, raios e pás de madeira."""
    import math
    G = 32
    im = Image.new("RGBA", (G, G), (0, 0, 0, 0))
    c = (G - 1) / 2
    for y in range(G):
        for x in range(G):
            d = math.hypot(x - c, y - c)
            ang = math.atan2(y - c, x - c)
            if 13.5 <= d <= 15.5:
                im.putpixel((x, y), (196, 140, 86, 255))          # pás na borda
            elif 11 <= d <= 12.5:
                im.putpixel((x, y), (150, 100, 60, 255))          # aro
            elif d < 11 and min(abs(math.sin(ang * 4)), 1) < 0.18:
                im.putpixel((x, y), (170, 116, 70, 255))          # raios
            elif d < 2.5:
                im.putpixel((x, y), (110, 74, 46, 255))           # eixo
    # Divisões das pás.
    for k in range(16):
        a = k * math.pi / 8
        for r in (13.5, 14.5, 15.5):
            x, y = int(round(c + r * math.cos(a))), int(round(c + r * math.sin(a)))
            if 0 <= x < G and 0 <= y < G:
                im.putpixel((x, y), CONTORNO)
    # Contorno externo.
    alvo = []
    for y in range(G):
        for x in range(G):
            if im.getpixel((x, y))[3] == 0 and any(0 <= x + dx < G and 0 <= y + dy < G and im.getpixel((x + dx, y + dy))[3] == 255
                                                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                alvo.append((x, y))
    for q in alvo:
        im.putpixel(q, CONTORNO)
    return [im.crop((0, 0, 16, 16)), im.crop((16, 0, 32, 16)), im.crop((0, 16, 16, 32)), im.crop((16, 16, 32, 32))]

def porco():
    im = nova()
    rosa, sombra = (238, 160, 170, 255), (206, 124, 140, 255)
    for y in range(6, 13):
        for x in range(3, 13):
            if ((x - 7.5) ** 2) / 25 + ((y - 9) ** 2) / 10 <= 1:
                im.putpixel((x, y), rosa if y < 10 else sombra)
    for (x, y) in [(12, 8), (13, 8), (12, 9), (13, 9)]:
        im.putpixel((x, y), (226, 140, 152, 255))           # focinho
    im.putpixel((11, 7), CONTORNO)                            # olho
    im.putpixel((10, 5), rosa); im.putpixel((11, 5), rosa)    # orelha
    for x in (5, 7, 9, 11):
        im.putpixel((x, 13), sombra)                          # patas
    im.putpixel((2, 8), sombra)                               # rabinho
    return contorno_em_volta(im)

def lama():
    im = nova()
    for y in range(T):
        for x in range(T):
            d = ((x - 8) ** 2) / 49 + ((y - 9) ** 2) / 20
            if d <= 1:
                im.putpixel((x, y), (116, 82, 56, 255) if d > 0.4 else (98, 68, 46, 255))
    im.putpixel((6, 8), (140, 104, 74, 255)); im.putpixel((10, 10), (140, 104, 74, 255))
    return im

def cepo():
    im = nova()
    for y in range(9, 15):
        for x in range(4, 12):
            im.putpixel((x, y), (164, 108, 64, 255) if y > 9 else (214, 170, 110, 255))
    for x in range(5, 11):
        im.putpixel((x, 10), (190, 140, 88, 255))
    # Machado cravado: cabo e lâmina.
    for k in range(6):
        im.putpixel((9 + k // 2, 8 - k), (132, 90, 54, 255))
    for (x, y) in [(10, 3), (11, 3), (12, 3), (11, 4), (12, 4), (12, 2)]:
        im.putpixel((x, y), (186, 192, 204, 255))
    return contorno_em_volta(im)

def forno():
    im = nova()
    barro, sombra, claro = (198, 120, 76, 255), (156, 88, 58, 255), (226, 156, 104, 255)
    for y in range(T):
        for x in range(T):
            d = ((x - 8) ** 2) / 42 + ((y - 11) ** 2) / 36
            if d <= 1 and y <= 14:
                im.putpixel((x, y), claro if (y < 8 and x < 8) else barro if y < 12 else sombra)
    # Boca do forno com brasa.
    for y in range(10, 15):
        for x in range(6, 11):
            borda = y == 10 or x in (6, 10)
            im.putpixel((x, y), CONTORNO if borda else (236, 120, 40, 255) if y > 12 else (120, 40, 30, 255))
    im = contorno_em_volta(im)
    for (cx, cy, r) in [(12, 3, 1.4), (13, 1, 1.0)]:
        for y in range(T):
            for x in range(T):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    im.putpixel((x, y), (232, 232, 236, 220))
    return im

def placa_pao():
    im = nova()
    for x in range(3, 13):
        im.putpixel((x, 3), (118, 74, 44, 255))                 # haste
    im.putpixel((4, 4), (118, 74, 44, 255)); im.putpixel((11, 4), (118, 74, 44, 255))
    for y in range(5, 13):
        for x in range(3, 13):
            im.putpixel((x, y), (214, 176, 120, 255))           # tábua
    for y in range(7, 11):
        for x in range(5, 11):
            if ((x - 7.5) ** 2) / 9 + ((y - 8.5) ** 2) / 4 <= 1:
                im.putpixel((x, y), (176, 110, 52, 255) if y > 7 else (214, 150, 80, 255))  # pão
    im.putpixel((6, 8), (240, 200, 140, 255)); im.putpixel((9, 8), (240, 200, 140, 255))
    return contorno_em_volta(im)

def torre(material):
    """Torre de igreja de 16x32 (duas peças): corpo estreito, sineira com o
    sino e, no alto, cruz (madeira) ou pináculo de telha (tijolo)."""
    G = Image.new("RGBA", (T, 2 * T), (0, 0, 0, 0))
    if material == "madeira":
        corpo, sombra = (176, 118, 70, 255), (138, 90, 54, 255)
    else:
        corpo, sombra = (178, 70, 52, 255), (140, 54, 42, 255)
    # Corpo da torre (x 4..11), da linha 12 até o fim.
    for y in range(12, 2 * T):
        for x in range(4, 12):
            if material == "madeira":
                c = corpo if x % 3 else sombra           # tábuas verticais
            else:
                fiada = (y - 12) // 3
                junta = (y - 12) % 3 == 2 or (x + (2 if fiada % 2 else 0)) % 4 == 3
                c = (214, 186, 160, 255) if junta else corpo
            G.putpixel((x, y), c)
    # Sineira: abertura escura com o sino dourado.
    for y in range(14, 19):
        for x in range(6, 10):
            G.putpixel((x, y), (60, 40, 50, 255))
    for (x, y) in [(7, 16), (8, 16), (7, 17), (8, 17)]:
        G.putpixel((x, y), (230, 190, 80, 255))
    if material == "madeira":
        # Telhadinho de palha e cruz.
        for y in range(8, 12):
            for x in range(3 + (11 - y) // 1 - 3, 13 - ((11 - y) - 0)):
                if 0 <= x < T:
                    G.putpixel((x, y), (204, 160, 84, 255))
        for y in range(2, 8):
            G.putpixel((8, y), (120, 78, 48, 255))
        for x in range(6, 11):
            G.putpixel((x, 4), (120, 78, 48, 255))
    else:
        # Pináculo pontudo de telha vermelha.
        for y in range(1, 12):
            meia = (y * 5) // 11
            for x in range(8 - meia, 9 + meia):
                if 0 <= x < T:
                    G.putpixel((x, y), (214, 92, 70, 255) if x < 8 else (182, 70, 56, 255))
        G.putpixel((8, 0), (230, 190, 80, 255))
    # Contorno.
    alvo = []
    for y in range(2 * T):
        for x in range(T):
            if G.getpixel((x, y))[3] == 0 and any(0 <= x + dx < T and 0 <= y + dy < 2 * T and G.getpixel((x + dx, y + dy))[3] == 255
                                                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                alvo.append((x, y))
    for q in alvo:
        G.putpixel(q, CONTORNO)
    return [G.crop((0, 0, T, T)), G.crop((0, T, T, 2 * T))]

def bandeira():
    im = nova()
    for y in range(1, 16):
        im.putpixel((3, y), (120, 78, 48, 255))       # mastro
    im.putpixel((3, 0), (230, 190, 80, 255))
    # Flâmula em duas faixas, vermelha em cima e branca embaixo (cores de Lübeck).
    for y in range(2, 9):
        for x in range(4, 14 - (abs(y - 5) // 2)):
            im.putpixel((x, y), (214, 64, 56, 255) if y < 5 else (244, 240, 232, 255))
    return contorno_em_volta(im)

def janela_gotica():
    im = nova()
    tijolos(im)
    for x in range(T):
        im.putpixel((x, T - 1), CONTORNO)
        im.putpixel((x, T - 2), (120, 50, 44, 255))
    # Arco em ponta: largura 6, ponta no alto.
    for y in range(2, 14):
        meia = 3 if y >= 5 else (y - 2)
        for x in range(8 - meia, 8 + meia):
            borda = x in (8 - meia, 8 + meia - 1) or y == 13
            im.putpixel((x, y), CONTORNO if borda else (150, 196, 230, 255) if (x + y) % 5 else (210, 230, 246, 255))
    for y in range(5, 13):
        im.putpixel((7, y), CONTORNO)                       # mainel
    return im

def empena_escalonada():
    """Fachada em degraus, 32x16: cada degrau tem 4 px de largura e 3 de
    altura, com a cobertura clara de pedra em cima de cada degrau."""
    G = Image.new("RGBA", (2 * T, T), (0, 0, 0, 0))
    degraus = 5
    for k in range(degraus):
        topo = T - 1 - (k + 1) * 3
        x0, x1 = k * 3, 2 * T - k * 3
        for y in range(topo, T):
            for x in range(x0, x1):
                fiada = (y - topo) // 3
                junta = (y % 3 == 2) or ((x + (2 if (y // 3) % 2 else 0)) % 5 == 4)
                G.putpixel((x, y), (214, 186, 160, 255) if junta else (176, 68, 50, 255))
        for x in range(x0, x1):
            G.putpixel((x, topo), (232, 220, 200, 255))   # cobertura do degrau
    # Rosácea no meio.
    for y in range(6, 11):
        for x in range(14, 18):
            G.putpixel((x, y), (150, 196, 230, 255))
    alvo = []
    for y in range(T):
        for x in range(2 * T):
            if G.getpixel((x, y))[3] == 0 and any(0 <= x + dx < 2 * T and 0 <= y + dy < T and G.getpixel((x + dx, y + dy))[3] == 255
                                                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                alvo.append((x, y))
    for q in alvo:
        G.putpixel(q, CONTORNO)
    return [G.crop((0, 0, T, T)), G.crop((T, 0, 2 * T, T))]

def janela_estreita():
    """Só a janela, transparente em volta, para pôr sobre uma parede lisa."""
    im = nova()
    for y in range(3, 14):
        meia = 2 if y >= 5 else 1
        for x in range(8 - meia, 8 + meia):
            borda = x in (8 - meia, 8 + meia - 1) or y in (3, 13)
            im.putpixel((x, y), CONTORNO if borda else (150, 196, 230, 255))
    return im

def placa_caneca():
    im = nova()
    for x in range(3, 13):
        im.putpixel((x, 3), (118, 74, 44, 255))
    for y in range(5, 13):
        for x in range(3, 13):
            im.putpixel((x, y), (214, 176, 120, 255))
    for y in range(7, 12):
        for x in range(6, 10):
            im.putpixel((x, y), (230, 180, 60, 255) if y > 7 else (250, 246, 236, 255))  # cerveja e espuma
    for y in range(8, 11):
        im.putpixel((10, y), (150, 110, 70, 255))                                        # alça
    return contorno_em_volta(im)

def monte_carvao():
    im = nova()
    for y in range(T):
        for x in range(T):
            d = ((x - 8) ** 2) / 42 + ((y - 12) ** 2) / 25
            if d <= 1 and y <= 14:
                im.putpixel((x, y), (70, 60, 56, 255) if y > 9 else (96, 84, 76, 255))
    for (x, y) in [(6, 11), (10, 12), (8, 13)]:
        im.putpixel((x, y), (236, 120, 40, 255))  # brasa aparecendo
    im = contorno_em_volta(im)
    for (cx, cy, r) in [(8, 4, 1.8), (10, 2, 1.4), (7, 0, 1.0)]:
        for y in range(T):
            for x in range(T):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    im.putpixel((x, y), (190, 190, 196, 220))
    return im

def pilha_turfa():
    im = nova()
    for fileira, (y0, n) in enumerate([(11, 3), (7, 2)]):
        for k in range(n):
            x0 = 2 + k * 4 + fileira * 2
            for y in range(y0, y0 + 4):
                for x in range(x0, x0 + 4):
                    im.putpixel((x, y), (92, 66, 46, 255) if y > y0 else (124, 92, 64, 255))
    return contorno_em_volta(im)

def toldo():
    G = Image.new("RGBA", (2 * T, T), (0, 0, 0, 0))
    for y in range(2, 9):
        for x in range(1, 2 * T - 1):
            G.putpixel((x, y), (214, 64, 56, 255) if (x // 4) % 2 == 0 else (244, 240, 232, 255))
    for x in range(1, 2 * T - 1):          # franja ondulada
        if x % 4 in (0, 1):
            G.putpixel((x, 9), (214, 64, 56, 255) if (x // 4) % 2 == 0 else (244, 240, 232, 255))
    for y in range(9, T):                  # postes
        G.putpixel((2, y), (118, 74, 44, 255)); G.putpixel((2 * T - 3, y), (118, 74, 44, 255))
    alvo = []
    for y in range(T):
        for x in range(2 * T):
            if G.getpixel((x, y))[3] == 0 and any(0 <= x + dx < 2 * T and 0 <= y + dy < T and G.getpixel((x + dx, y + dy))[3] == 255
                                                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                alvo.append((x, y))
    for q in alvo:
        G.putpixel(q, CONTORNO)
    return [G.crop((0, 0, T, T)), G.crop((T, 0, 2 * T, T))]

def balcao():
    im = nova()
    for y in range(9, 14):
        for x in range(1, 15):
            im.putpixel((x, y), (168, 108, 62, 255) if y > 9 else (196, 140, 86, 255))
    # Mercadorias: pão, peixe e um pote.
    for (x, y, c) in [(3, 7, (214, 150, 80, 255)), (4, 7, (214, 150, 80, 255)), (7, 8, (190, 200, 210, 255)),
                      (8, 8, (190, 200, 210, 255)), (11, 6, (120, 84, 60, 255)), (11, 7, (120, 84, 60, 255)),
                      (12, 7, (120, 84, 60, 255)), (12, 6, (120, 84, 60, 255))]:
        im.putpixel((x, y), c)
    return contorno_em_volta(im)

def respiro():
    im = nova()
    for y in range(4, 14):
        for x in range(4, 12):
            borda = x in (4, 11) or y == 13
            im.putpixel((x, y), CONTORNO if borda else (150, 110, 70, 255) if (y % 3) else (100, 70, 46, 255))  # venezianas
    for x in range(3, 13):
        im.putpixel((x, 3), CONTORNO); im.putpixel((x, 4), (204, 160, 84, 255))
    for (cx, cy, r) in [(8, 1, 1.2)]:
        for y in range(T):
            for x in range(T):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    im.putpixel((x, y), (236, 236, 240, 200))
    return im

def contornar_grande(G):
    w, h = G.size
    alvo = []
    for y in range(h):
        for x in range(w):
            if G.getpixel((x, y))[3] == 0 and any(0 <= x + dx < w and 0 <= y + dy < h and G.getpixel((x + dx, y + dy))[3] == 255
                                                   for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                alvo.append((x, y))
    for q in alvo:
        G.putpixel(q, CONTORNO)
    return G

def cevada():
    im = nova()
    for x0 in (1, 5, 8, 11):
        for y in range(6, 15):
            im.putpixel((x0, y), (150, 170, 80, 255))                 # caule
        for k in range(4):                                             # espiga caída para o lado
            im.putpixel((x0 + 1 + k // 2, 4 + k), (226, 210, 120, 255))
            im.putpixel((x0 + 2 + k // 2, 4 + k), (200, 182, 96, 255))
        im.putpixel((x0 + 3, 3), (240, 230, 160, 255))                # pragana
    return im

def varal_tecidos():
    im = nova()
    for y in range(2, 15):
        im.putpixel((1, y), (118, 74, 44, 255)); im.putpixel((14, y), (118, 74, 44, 255))
    for x in range(1, 15):
        im.putpixel((x, 3), (90, 60, 40, 255))
    for x0, cor in ((3, (70, 110, 180, 255)), (7, (200, 70, 60, 255)), (11, (220, 190, 80, 255))):
        for y in range(4, 12):
            for x in range(x0, x0 + 3):
                if x < 14:
                    im.putpixel((x, y), cor)
    return contorno_em_volta(im)

def tina(cor_liquido):
    im = nova()
    for y in range(6, 15):
        for x in range(2, 14):
            if ((x - 7.5) ** 2) / 36 + ((y - 10) ** 2) / 20 <= 1:
                im.putpixel((x, y), (150, 100, 60, 255))
    for y in range(7, 10):
        for x in range(4, 12):
            im.putpixel((x, y), cor_liquido)
    for x in range(2, 14):
        im.putpixel((x, 11), (110, 74, 46, 255))                       # aro
    return contorno_em_volta(im)

def couro():
    im = nova()
    for y in range(2, 15):
        im.putpixel((2, y), (118, 74, 44, 255)); im.putpixel((13, y), (118, 74, 44, 255))
    for x in range(2, 14):
        im.putpixel((x, 2), (118, 74, 44, 255)); im.putpixel((x, 14), (118, 74, 44, 255))
    for y in range(4, 13):
        for x in range(4, 12):
            if not ((y in (4, 12)) and x in (4, 11)):
                im.putpixel((x, y), (190, 140, 96, 255) if (x + y) % 4 else (170, 120, 80, 255))
    return contorno_em_volta(im)

def tanque_sal():
    im = nova()
    for y in range(4, 15):
        for x in range(1, 15):
            borda = y in (4, 14) or x in (1, 14)
            im.putpixel((x, y), (150, 120, 90, 255) if borda else (236, 240, 244, 255) if (x * 3 + y) % 5 else (200, 214, 226, 255))
    return contorno_em_volta(im)

def buraco_argila():
    im = nova()
    for y in range(T):
        for x in range(T):
            d = ((x - 8) ** 2) / 42 + ((y - 10) ** 2) / 22
            if d <= 1:
                im.putpixel((x, y), (120, 66, 48, 255) if d < 0.45 else (180, 104, 70, 255))
    for y in range(4, 12):                                               # escada
        im.putpixel((11, y), (118, 74, 44, 255)); im.putpixel((13, y), (118, 74, 44, 255))
        if y % 2 == 0: im.putpixel((12, y), (118, 74, 44, 255))
    return contorno_em_volta(im)

def pilha_tijolos():
    im = nova()
    for fileira in range(4):
        y0 = 13 - fileira * 3
        for k in range(4 - fileira // 2):
            x0 = 2 + k * 3 + (fileira % 2)
            for y in range(y0, y0 + 2):
                for x in range(x0, x0 + 3):
                    im.putpixel((x, y), (184, 74, 54, 255) if y == y0 else (150, 58, 44, 255))
    return contorno_em_volta(im)

def forno_olaria():
    """Forno de tijolos em garrafa, 16x32: base larga com boca acesa, gargalo e
    fumaça. Duas peças (alto, base)."""
    G = Image.new("RGBA", (T, 2 * T), (0, 0, 0, 0))
    for y in range(6, 2 * T):
        meia = 3 if y < 14 else min(7, 3 + (y - 14) // 2)
        for x in range(8 - meia, 8 + meia):
            fiada = y // 3
            junta = y % 3 == 2 or (x + (2 if fiada % 2 else 0)) % 4 == 3
            G.putpixel((x, y), (214, 186, 160, 255) if junta else (178, 70, 52, 255))
    for y in range(24, 31):
        for x in range(5, 11):
            G.putpixel((x, y), (236, 120, 40, 255) if y > 27 else (90, 30, 26, 255))
    for (cx, cy, r) in [(8, 4, 1.8), (10, 1, 1.3)]:
        for y in range(2 * T):
            for x in range(T):
                if (x - cx) ** 2 + (y - cy) ** 2 <= r * r:
                    G.putpixel((x, y), (200, 200, 206, 220))
    contornar_grande(G)
    return [G.crop((0, 0, T, T)), G.crop((0, T, T, 2 * T))]

def barco():
    G = Image.new("RGBA", (2 * T, T), (0, 0, 0, 0))
    for y in range(7, 13):
        recuo = (y - 7) // 2
        for x in range(2 + recuo + (2 if y < 9 else 0), 30 - recuo):
            G.putpixel((x, y), (156, 102, 60, 255) if y > 8 else (196, 140, 86, 255))
    for y in range(1, 8):
        G.putpixel((16, y), (118, 74, 44, 255))                       # mastro
    for y in range(2, 7):
        for x in range(17, 17 + (7 - y) + 3):
            G.putpixel((x, y), (240, 236, 220, 255))                   # vela
    contornar_grande(G)
    return [G.crop((0, 0, T, T)), G.crop((T, 0, 2 * T, T))]

def rede():
    im = nova()
    for y in range(3, 14):
        for x in range(2, 14):
            if (x + y) % 3 == 0 or (x - y) % 3 == 0:
                im.putpixel((x, y), (120, 110, 90, 255))
    for x in range(2, 14):
        im.putpixel((x, 3), (180, 140, 90, 255))                       # boias de cortiça
    return im

def pier():
    im = nova()
    for y in range(T):
        for x in range(T):
            junta = y % 4 == 3
            im.putpixel((x, y), (120, 80, 50, 255) if junta else (176, 122, 74, 255) if (x // 5 + y // 4) % 2 else (162, 112, 68, 255))
    return im

def guindaste():
    G = Image.new("RGBA", (T, 2 * T), (0, 0, 0, 0))
    for y in range(4, 2 * T):
        G.putpixel((6, y), (150, 100, 60, 255)); G.putpixel((7, y), (118, 74, 44, 255))
    for x in range(2, 15):
        G.putpixel((x, 4), (150, 100, 60, 255)); G.putpixel((x, 5), (118, 74, 44, 255))
    for k in range(6):
        G.putpixel((7 + k, 6 + k), (118, 74, 44, 255))                 # mão-francesa
    for y in range(6, 16):
        G.putpixel((13, y), (90, 80, 70, 255))                          # corda
    for y in range(16, 20):
        for x in range(11, 16):
            G.putpixel((x, y), (184, 130, 80, 255))                     # carga (fardo)
    contornar_grande(G)
    return [G.crop((0, 0, T, T)), G.crop((0, T, T, 2 * T))]

def sombra():
    im = nova()
    for y in range(T):
        for x in range(T):
            d = ((x - 7.5) ** 2) / 56 + ((y - 7.5) ** 2) / 56
            if d <= 1:
                im.putpixel((x, y), (0, 0, 0, int(70 * (1 - d * d))))
    return im

def main(saida):
    pecas = [agua(0), agua(1), barreiro(), turfa(), parede_tijolo(), parede_tijolo("janela"),
             parede_tijolo("porta"), calcamento()]
    pecas += [girar(margem_norte(), lado) for lado in range(4)]
    pecas += [girar(franja_norte(), lado) for lado in range(4)]
    pecas += [monte_argila(), poca_junco(), pedrinhas(), chamine()]
    town = Image.open(sys.argv[2]).convert("RGBA")
    pecas += [palha(town, n) for n in (52, 53, 54, 64, 65, 66, 67, 55)]
    pecas += [varal_peixe()] + roda_dagua() + [porco(), lama(), cepo(), forno(), placa_pao()]
    pecas += torre("madeira") + torre("tijolo") + [bandeira()]
    pecas += [janela_gotica()] + empena_escalonada()
    pecas += [janela_estreita(), placa_caneca(), monte_carvao(), pilha_turfa()] + toldo() + [balcao(), respiro()]
    pecas += [cevada(), varal_tecidos(), tina((120, 84, 50, 255)), couro(), tanque_sal(), buraco_argila(), pilha_tijolos()]
    pecas += forno_olaria() + barco() + [rede(), pier()] + guindaste()
    pecas += [sombra()]
    folha = Image.new("RGBA", (T * len(pecas), T), (0, 0, 0, 0))
    for i, p in enumerate(pecas):
        folha.paste(p, (i * T, 0))
    folha.save(saida)

if __name__ == "__main__":
    main(sys.argv[1])
