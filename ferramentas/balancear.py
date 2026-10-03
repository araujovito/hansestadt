#!/usr/bin/env python3
"""Simula a economia da vila sem janela e gera série mensal e gráfico SVG."""

from __future__ import annotations

import argparse
import calendar
import csv
from pathlib import Path
import tomllib


RAIZ = Path(__file__).resolve().parents[1]
CONFIGURACAO = {
    "fazenda": 1,
    "moinho": 1,
    "padaria": 1,
    "pescador": 1,
}
POPULACAO = 6
COMIDA_INICIAL = 600


def carregar() -> tuple[dict[str, dict], set[str]]:
    with (RAIZ / "dados/edificios.toml").open("rb") as arquivo:
        edificios = tomllib.load(arquivo)["edificio"]
    with (RAIZ / "dados/itens.toml").open("rb") as arquivo:
        itens = tomllib.load(arquivo)["item"]
    return {e["id"]: e for e in edificios}, {i["id"] for i in itens if i["tipo"] == "comida"}


def rendimento_comida_por_trabalhador() -> list[tuple[str, float]]:
    """Capacidade bruta das receitas autônomas, sem insumos/transporte/estoque."""
    edificios, comidas = carregar()
    resultados: list[tuple[str, float]] = []
    ano = 1300  # calendário do jogo: ano juliano bissexto, inclusive nos séculos.
    for identificador, edificio in edificios.items():
        trabalhadores = edificio.get("trabalhadores", 0)
        if trabalhadores <= 0:
            continue
        for receita in edificio.get("receita", []):
            if receita.get("entradas"):
                continue
            saidas_comida = sum(q for item, q in receita.get("saidas", {}).items() if item in comidas)
            if saidas_comida == 0:
                continue
            sazonalidade = receita.get("sazonalidade", [1] * 12)
            dias_ativos = sum(
                (28 if mes == 2 else 30 if mes in (4, 6, 9, 11) else 31) * fator
                for mes, fator in enumerate(sazonalidade, start=1)
            )
            dias_ativos += sazonalidade[1] if ano % 4 == 0 else 0
            dias_ano = 366 if ano % 4 == 0 else 365
            por_dia_trabalhador = saidas_comida * (dias_ativos / receita["dias"]) / trabalhadores / dias_ano
            resultados.append((identificador, por_dia_trabalhador))
    return resultados


def simular(anos: int) -> list[dict[str, int]]:
    edificios, comidas = carregar()
    estoque: dict[str, int] = {"pao": COMIDA_INICIAL}
    progresso: dict[tuple[str, int], float] = {}
    produzido = 0
    consumido = 0
    faltou = 0
    linhas: list[dict[str, int]] = []

    for ano in range(1150, 1150 + anos):
        for mes in range(3 if ano == 1150 else 1, 13):
            for _dia in range(1, calendar.monthrange(ano, mes)[1] + 1):
                for edificio_id, quantidade in CONFIGURACAO.items():
                    edificio = edificios[edificio_id]
                    for unidade in range(quantidade):
                        for indice, receita in enumerate(edificio.get("receita", [])):
                            fator = receita.get("sazonalidade", [1] * 12)[mes - 1]
                            chave = (edificio_id, unidade * 100 + indice)
                            progresso[chave] = progresso.get(chave, 0.0) + fator / receita["dias"]
                            while progresso[chave] >= 1:
                                entradas = receita.get("entradas", {})
                                if any(estoque.get(item, 0) < q for item, q in entradas.items()):
                                    break
                                for item, q in entradas.items():
                                    estoque[item] = estoque.get(item, 0) - q
                                for item, q in receita.get("saidas", {}).items():
                                    estoque[item] = estoque.get(item, 0) + q
                                    if item in comidas:
                                        produzido += q
                                progresso[chave] -= 1
                            break  # receitas são alternativas, não paralelas

                for _ in range(POPULACAO):
                    disponivel = next((item for item in comidas if estoque.get(item, 0) > 0), None)
                    if disponivel:
                        estoque[disponivel] -= 1
                        consumido += 1
                    else:
                        faltou += 1

            linhas.append({
                "ano": ano,
                "mes": mes,
                "produzido": produzido,
                "consumido": consumido,
                "falta": faltou,
                "estoque_comida": sum(estoque.get(item, 0) for item in comidas),
            })
    return linhas


def salvar_csv(linhas: list[dict[str, int]], destino: Path) -> None:
    destino.parent.mkdir(parents=True, exist_ok=True)
    with destino.open("w", newline="", encoding="utf-8") as arquivo:
        escritor = csv.DictWriter(arquivo, fieldnames=linhas[0].keys())
        escritor.writeheader()
        escritor.writerows(linhas)


def salvar_svg(linhas: list[dict[str, int]], destino: Path) -> None:
    largura, altura, margem = 960, 480, 55
    maximo = max(max(l["produzido"], l["consumido"]) for l in linhas) or 1
    def pontos(campo: str) -> str:
        return " ".join(
            f"{margem + i * (largura - 2*margem) / max(1, len(linhas)-1):.1f},"
            f"{altura-margem-l[campo] * (altura-2*margem) / maximo:.1f}"
            for i, l in enumerate(linhas)
        )
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" width="{largura}" height="{altura}">
<rect width="100%" height="100%" fill="#fffaf0"/><line x1="{margem}" y1="{altura-margem}" x2="{largura-margem}" y2="{altura-margem}" stroke="#594a3a"/>
<line x1="{margem}" y1="{margem}" x2="{margem}" y2="{altura-margem}" stroke="#594a3a"/>
<polyline fill="none" stroke="#43834f" stroke-width="3" points="{pontos('produzido')}"/>
<polyline fill="none" stroke="#b4513e" stroke-width="3" points="{pontos('consumido')}"/>
<text x="{margem}" y="30" font-family="sans-serif" font-size="18">Comida acumulada — verde: produzida; vermelho: consumida</text>
<text x="{largura-240}" y="{altura-15}" font-family="sans-serif" font-size="13">meses desde março de 1150</text></svg>'''
    destino.parent.mkdir(parents=True, exist_ok=True)
    destino.write_text(svg, encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--anos", type=int, default=1)
    parser.add_argument("--saida", type=Path, default=RAIZ / "build/balanceamento")
    args = parser.parse_args()
    linhas = simular(args.anos)
    salvar_csv(linhas, args.saida / "economia.csv")
    salvar_svg(linhas, args.saida / "economia.svg")
    fim = linhas[-1]
    print(f"{args.anos} ano(s): produzido={fim['produzido']}, consumido={fim['consumido']}, "
          f"falta={fim['falta']}, estoque={fim['estoque_comida']}")
    print("Capacidade bruta de comida por trabalhador/dia (sem limitar insumos, transporte ou estoque):")
    for edificio, taxa in rendimento_comida_por_trabalhador():
        print(f"  {edificio}: {taxa:.2f}")
    print(args.saida / "economia.csv")
    print(args.saida / "economia.svg")


if __name__ == "__main__":
    main()
