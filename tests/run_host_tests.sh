#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SERIAL="${EDP_SERIAL_SOURCE_DIR:-$ROOT/../EDP-Serial}"
SYSTEM="${EDP_SYSTEM_SOURCE_DIR:-$ROOT/../EDP-System}"
OUTPUT="$(mktemp)"
trap 'rm -f "$OUTPUT"' EXIT
test -d "$SERIAL/src" || { echo "Missing EDP-Serial" >&2; exit 2; }
test -d "$SYSTEM/src" || { echo "Missing EDP-System" >&2; exit 2; }
"${CXX:-c++}" -std=c++20 -Wall -Wextra -Wpedantic -Werror \
    -I"$ROOT/tests/fake_idf" -I"$ROOT/src" \
    -I"$SERIAL/src" -I"$SYSTEM/src" \
    "$ROOT/tests/UartFifoByteStreamWriterTests.cpp" -o "$OUTPUT"
"$OUTPUT"
echo "EDP-Serial-ESP-IDF UART FIFO abstract byte provider: PASS"
