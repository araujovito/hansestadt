#!/usr/bin/env bash
# Baixa o cabeçalho único do toml++ numa versão fixa e confere o checksum.
# É só um .hpp: não há o que compilar.
set -euo pipefail

VERSAO="v3.4.0"
SHA256="6b5172ad4dd6519aec67b919181fa7a38a2234131e5b2afa232dfe444819783e"
URL="https://raw.githubusercontent.com/marzer/tomlplusplus/${VERSAO}/toml.hpp"

raiz="$(cd "$(dirname "$0")/.." && pwd)"
destino="$raiz/vendor/tomlplusplus"

if [[ -f "$destino/toml.hpp" ]]; then
    echo "toml++ já baixado em vendor/tomlplusplus"
    exit 0
fi

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

echo "baixando toml++ $VERSAO"
curl -fsSL "$URL" -o "$tmp/toml.hpp"
echo "$SHA256  $tmp/toml.hpp" | sha256sum -c --quiet

mkdir -p "$destino"
mv "$tmp/toml.hpp" "$destino/toml.hpp"
echo "toml++ pronto em vendor/tomlplusplus/toml.hpp"
