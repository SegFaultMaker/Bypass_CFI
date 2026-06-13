#!/usr/bin/env bash
set -u
cd "$(dirname "$0")"

line() { printf '\n\033[1;36m=== %s ===\033[0m\n' "$1"; }

line "0. Build (clang++ -std=c++20 with -fsanitize=cfi)"
make --no-print-directory

line "1. A real coroutine frame: first two slots are resume/destroy pointers"
echo '(handle.resume() loads frame[0] and calls it -- a compiler intrinsic,'
echo ' not a typed indirect call, so clang cfi-icall never checks it)'
printf '\x00' | ./challenge 2>&1 | sed -n '1,7p' | sed 's/^/   /'

line "2. CFOP: forge fake frames whose resume slot points at gadgets -> CFI is BLIND"
echo '(forged frame 0 -> unlock_gadget flips latch; forged frame 1 -> reveal_gadget'
echo ' prints the flag. The scheduler loop chains them via resume())'
python3 exploit.py 2>&1 | sed 's/^/   /'
