#!/usr/bin/env bash
# Baixa os pacotes de arte Tiny Town e Tiny Farm da Kenney (licença CC0),
# confere o checksum de cada um e guarda em vendor/kenney só o que o jogo usa:
# a folha de peças de 16x16 (tilemap_packed.png) e a licença.
set -euo pipefail

raiz="$(cd "$(dirname "$0")/.." && pwd)"
destino="$raiz/vendor/kenney"

# nome  sha256  url
PACOTES=(
    "tiny-town 9768692dccff1d706408a5aedd6ca4f6cd1409506cbc84cb2f862919764be977 https://kenney.nl/media/pages/assets/tiny-town/a415fbeb49-1735736916/kenney_tiny-town.zip"
    "tiny-farm a06f75f312c27eff15a2288475612e6f6699411be7259d408323cd15a790decc https://kenney.nl/media/pages/assets/tiny-farm/dfded1ae3e-1782913588/kenney_tiny-farm.zip"
)

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

for pacote in "${PACOTES[@]}"; do
    read -r nome sha url <<<"$pacote"
    if [[ -f "$destino/$nome/tilemap_packed.png" ]]; then
        echo "kenney $nome já está em vendor/kenney"
        continue
    fi
    echo "baixando kenney $nome"
    curl -fsSL "$url" -o "$tmp/$nome.zip"
    echo "$sha  $tmp/$nome.zip" | sha256sum -c --quiet
    unzip -q -o "$tmp/$nome.zip" "Tilemap/tilemap_packed.png" "License.txt" -d "$tmp/$nome"
    mkdir -p "$destino/$nome"
    mv "$tmp/$nome/Tilemap/tilemap_packed.png" "$tmp/$nome/License.txt" "$destino/$nome/"
done
echo "arte da Kenney pronta em vendor/kenney"
