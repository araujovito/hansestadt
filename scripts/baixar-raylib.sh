#!/usr/bin/env bash
# Baixa a raylib numa versão fixa, confere o checksum e compila a biblioteca
# estática em vendor/raylib. Não instala nada no sistema.
set -euo pipefail

VERSAO="5.5"
SHA256="aea98ecf5bc5c5e0b789a76de0083a21a70457050ea4cc2aec7566935f5e258e"
URL="https://github.com/raysan5/raylib/archive/refs/tags/${VERSAO}.tar.gz"

raiz="$(cd "$(dirname "$0")/.." && pwd)"
destino="$raiz/vendor/raylib"

if [[ -f "$destino/src/libraylib.a" ]]; then
    echo "raylib já compilada em vendor/raylib"
    exit 0
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

echo "baixando raylib $VERSAO"
curl -fsSL "$URL" -o "$tmp/raylib.tar.gz"
echo "$SHA256  $tmp/raylib.tar.gz" | sha256sum -c --quiet

tar xzf "$tmp/raylib.tar.gz" -C "$tmp"
rm -rf "$destino"
mkdir -p "$raiz/vendor"
mv "$tmp/raylib-$VERSAO" "$destino"

echo "compilando raylib"
make -C "$destino/src" PLATFORM=PLATFORM_DESKTOP -j"$(nproc)" >/dev/null
echo "raylib pronta em vendor/raylib/src/libraylib.a"
