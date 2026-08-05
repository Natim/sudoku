#!/bin/bash
# Capture a screenshot of the application with a generated grid loaded.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

cmake --build build >/dev/null

g++ -std=c++17 -Iinclude \
  scripts/write_sample_grid.cpp src/random.cpp src/generator.cpp src/sudoku_grid.cpp \
  -o build/write_sample_grid
./build/write_sample_grid

SUDOKU_AUTO_LOAD=grille.sdk ./build/sudoku &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT

sleep 2
WIN=$(xwininfo -root -tree 2>/dev/null | grep -o '0x[0-9a-f]* "Sudoku' | head -1 | cut -d' ' -f1)
if [ -z "$WIN" ]; then
  echo "Sudoku window not found" >&2
  exit 1
fi

mkdir -p docs
import -window "$WIN" docs/screenshot.png
echo "Screenshot saved to docs/screenshot.png"
