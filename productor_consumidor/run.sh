#!/usr/bin/env bash
# Actividad 09 - Productor / consumidor con buffer compartido
# Compila y ejecuta los programas originales y el mejorado, y regenera las evidencias.
#
# Uso: ./run.sh
# Requiere: g++ con C++17, -pthread y el comando timeout.
set -uo pipefail
cd "$(dirname "$0")"

CXX=(g++ -std=c++17 -O2 -Wall -Wextra -pthread)
mkdir -p bin evidencias
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

echo "[1/4] Compilando programas originales..."
{
  echo "== Compilacion de originales con -Wall -Wextra =="
  for n in 00 01 02; do
    echo "--- originales/productores$n.cpp"
    "${CXX[@]}" "originales/productores$n.cpp" -o "bin/productores$n" 2>&1 |
      grep -E "warning|error" || true
  done
} > evidencias/compilacion_originales.txt

echo "[2/4] Ejecutando originales (se cortan a los 2 s: son infinitos)..."
{
  echo "######## EVIDENCIAS: PROGRAMAS ORIGINALES"
  echo
  echo "Los tres programas tienen bucles while(true): no terminan nunca."
  echo "Por eso se ejecutan con 'timeout 2'; codigo 124 = matado por timeout."
  echo
  for n in 00 01 02; do
    # Se guardan solo 40 lineas y las 5 ultimas: el programa produce millones
    # de lineas por segundo y no debe llenar el disco.
    timeout 2 "bin/productores$n" 2>&1 |
      awk -v m="$tmp/p$n.txt" '
        NR <= 40 { print > m }
        { ult[NR % 5] = $0 }
        END {
          print NR > (m ".total")
          for (i = NR - 4; i <= NR; i++) print ult[i % 5] > (m ".tail")
        }'
    ec=${PIPESTATUS[0]}
    lineas=$(cat "$tmp/p$n.txt.total")
    inicio=$(grep -n "Productor-1" "$tmp/p$n.txt" | head -1 | cut -d: -f1)
    echo "=================================================================="
    echo "== originales/productores$n.cpp =="
    echo "Comando: timeout 2 ./bin/productores$n"
    echo "Codigo de salida: $ec (124 = el programa no termina solo)"
    echo "Lineas producidas en 2 s: $lineas"
    echo "--- primeras 6 lineas ---"
    head -6 "$tmp/p$n.txt"
    echo "   [...]   (el bucle no termina)"
    echo
    echo "--- muestra donde aparece el segundo productor ---"
    if [ -n "$inicio" ]; then
      sed -n "${inicio},$((inicio + 3))p" "$tmp/p$n.txt"
    else
      echo "(no se alcanzo a ver al Productor-1 en la muestra)"
    fi
    echo "   [...]"
    echo "--- ultimas lineas completas ---"
    grep "estoy produciendo" "$tmp/p$n.txt.tail" | tail -3
    echo
  done
} > evidencias/antes.txt

echo "[3/4] Compilando y ejecutando la version mejorada..."
{
  echo "######## EVIDENCIAS: VERSION MEJORADA"
  echo
  echo "== Compilacion con -Wall -Wextra (sin advertencias) =="
  "${CXX[@]}" mejorado/productor_consumidor_mejorado.cpp -o bin/prodcons 2>&1
  echo
  echo "== 2 productores, 2 consumidores, capacidad 5, 8 items c/u =="
  bin/prodcons 2 2 5 8 1
  echo
  echo "== 4 productores, 3 consumidores, capacidad 2, 10000 items c/u =="
  bin/prodcons 4 3 2 10000 0
} > evidencias/despues.txt 2>&1

echo "[4/4] Ejecutando la version mejorada con ThreadSanitizer..."
{
  echo "== Compilacion con -fsanitize=thread =="
  g++ -std=c++17 -O1 -g -fsanitize=thread -pthread \
    mejorado/productor_consumidor_mejorado.cpp -o bin/prodcons_tsan 2>&1
  echo "== 3 productores, 3 consumidores, capacidad 2, 500 items c/u =="
  bin/prodcons_tsan 3 3 2 500 0 2>&1 |
    grep -E "WARNING|SUMMARY|Producidos"
  echo "(sin WARNING = ThreadSanitizer no detecta data races)"
} > evidencias/tsan_mejorado.txt 2>&1

echo
echo "Listo. Evidencias en evidencias/:"
echo "  - compilacion_originales.txt"
echo "  - antes.txt"
echo "  - despues.txt"
echo "  - tsan_mejorado.txt"
echo
echo "Para regenerar las capturas: ./tools/generar_capturas.sh"
