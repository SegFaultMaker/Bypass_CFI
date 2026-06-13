#!/usr/bin/env bash
set -u
cd "$(dirname "$0")"

line() { printf '\n\033[1;36m=== %s ===\033[0m\n' "$1"; }

line "0. Build (clang CFI build + plain comparison build)"
make --no-print-directory

line "1. FORWARD edge: hijack an indirect call -> clang CFI BLOCKS it"
echo '(feeding mode 1; expect a CFI runtime error + abort)'
printf '1\nq\n' | ./challenge
echo "exit code: $?"

line "2. BACKWARD edge: overflow the saved return address -> ROP WINS"
echo '(running exploit.py against the SAME CFI-enabled binary)'
python3 exploit.py 2>&1 | grep -E "Captured|FLAG" | sed 's/^/   /'
