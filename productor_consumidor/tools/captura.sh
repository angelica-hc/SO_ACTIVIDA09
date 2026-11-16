#!/usr/bin/env bash
# Genera una imagen estilo captura de terminal a partir de un archivo de texto
# con la salida real de un programa.
# Uso: tools/captura.sh "Titulo de la ventana" entrada.txt salida.png
set -euo pipefail

titulo="${1:?Falta el titulo}"
texto="${2:?Falta el archivo de texto}"
salida="${3:?Falta el archivo de salida}"

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

pango-view -q \
  --font="JetBrainsMono NF 13" \
  --background="#1e1e2e" \
  --foreground="#cdd6f4" \
  --margin=22 \
  --pixels \
  --text="$(cat "$texto")" \
  -o "$tmp/cuerpo.png"

ancho="$(magick identify -format %w "$tmp/cuerpo.png")"

magick -size "${ancho}x38" xc:"#181825" \
  -fill "#f38ba8" -draw "circle 20,19 20,26" \
  -fill "#f9e2af" -draw "circle 42,19 42,26" \
  -fill "#a6e3a1" -draw "circle 64,19 64,26" \
  -fill "#a6adc8" -font "DejaVu-Sans" -pointsize 13 \
  -annotate +86+24 "$titulo" \
  "$tmp/barra.png"

magick "$tmp/barra.png" "$tmp/cuerpo.png" -append \
  -bordercolor "#11111b" -border 2 \
  "$salida"
