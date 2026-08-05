#!/bin/sh
# Start a built sudoku on a headless display and check that it stays up with
# every image it needs, wherever it was installed or unpacked. Requires xvfb.
#
#   scripts/smoke_test.sh build/sudoku
set -eu

program="${1:-build/sudoku}"
errors="$(mktemp)"
trap 'rm -f "$errors"' EXIT

timeout 5 xvfb-run -a "$program" >/dev/null 2>"$errors" && status=0 || status=$?

if grep -q 'Cannot open file' "$errors"; then
  echo "$program does not find its images:" >&2
  cat "$errors" >&2
  exit 1
fi

# 124 is the timeout striking a program still running, which is what a game
# waiting for a click does. Anything else means it gave up on its own.
if [ "$status" -ne 124 ]; then
  echo "$program stopped by itself, status $status" >&2
  cat "$errors" >&2
  exit 1
fi

echo "$program runs and reads its images"
