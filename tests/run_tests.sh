#!/usr/bin/env bash
# run_tests.sh — compila o interpretador (lexer.l + parser.y) e roda os testes
# em tests/cases/*.py, comparando a saída com tests/cases/*.expected.
#
# Uso:
#   ./tests/run_tests.sh
#
# Requer: flex, bison, gcc instalados (sudo apt install flex bison build-essential)

set -u

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
CASES_DIR="$ROOT_DIR/tests/cases"
BIN="$BUILD_DIR/interpretador"

mkdir -p "$BUILD_DIR"

echo "==> Compilando..."
bison -d -o "$BUILD_DIR/y.tab.c" "$ROOT_DIR/parser/parser.y" || exit 1
flex  -o "$BUILD_DIR/lex.yy.c" "$ROOT_DIR/lexer/lexer.l" || exit 1
gcc -I "$BUILD_DIR" -o "$BIN" "$BUILD_DIR/y.tab.c" "$BUILD_DIR/lex.yy.c" -lm || exit 1
echo "==> Build OK"
echo

PASS=0
FAIL=0
FAILED_NAMES=()

for py_file in "$CASES_DIR"/*.py; do
    name="$(basename "$py_file" .py)"
    expected_file="$CASES_DIR/$name.expected"

    if [[ ! -f "$expected_file" ]]; then
        echo "SKIP  $name (sem arquivo .expected)"
        continue
    fi

    actual_output="$("$BIN" "$py_file" 2>/dev/null)"
    expected_output="$(cat "$expected_file")"

    if [[ "$actual_output" == "$expected_output" ]]; then
        echo "PASS  $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL  $name"
        echo "  --- esperado ---"
        echo "$expected_output" | sed 's/^/  /'
        echo "  --- obtido ---"
        echo "$actual_output" | sed 's/^/  /'
        FAIL=$((FAIL + 1))
        FAILED_NAMES+=("$name")
    fi
done

echo
echo "======================================"
echo "Resultado: $PASS passou(aram), $FAIL falhou(aram)"
if [[ $FAIL -gt 0 ]]; then
    echo "Testes que falharam: ${FAILED_NAMES[*]}"
    exit 1
fi
exit 0
