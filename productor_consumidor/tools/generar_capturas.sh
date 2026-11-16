#!/usr/bin/env bash
# Regenera las 6 capturas a partir de los textos reales guardados en capturas/texto/.
# Uso: ./tools/generar_capturas.sh
set -euo pipefail
cd "$(dirname "$0")/.."

gen() { tools/captura.sh "$1" "capturas/texto/$2" "capturas/$3"; }

gen "Antes - productores00 (solo productores)"       01_antes_productores00.txt    01_antes_productores00.png
gen "Antes - productores01 (solo productores)"       02_antes_productores01.txt    02_antes_productores01.png
gen "Antes - productores02 (solo productores)"       03_antes_productores02.txt    03_antes_productores02.png
gen "Despues - productor y consumidor con buffer"    04_despues_intercalado.txt    04_despues_intercalado.png
gen "Despues - verificacion de 40000 items"          05_despues_verificacion.txt   05_despues_verificacion.png
gen "Calidad - sin advertencias y TSan limpio"       06_calidad_tsan.txt           06_calidad_tsan.png

echo "Capturas generadas en capturas/"
