#!/usr/bin/env python3
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import target

ROOT = Path(__file__).resolve().parent.parent.parent
OBJ = ROOT / "build" / "native" / "obj"
DECOMP = ROOT / "decomp"

def nm(paths: list[Path]) -> tuple[set[str], set[str]]:
    defined, undefined = set(), set()
    out = subprocess.run([target.tool("llvm-nm"), *map(str, paths)], capture_output=True, text=True).stdout
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "U":
            undefined.add(target.c_name(parts[1]))
        elif len(parts) == 3 and parts[1] not in ("U", "w"):
            defined.add(target.c_name(parts[2]))
    return defined, undefined

def main() -> None:
    objs = sorted(OBJ.glob("*.o"))
    defined, undefined = nm(objs)
    missing = sorted(undefined - defined)
    pc = set(re.findall(r"\b(\w+)\s*\(", (DECOMP / "include" / "PC" / "PC.h").read_text(errors="replace")))
    crt = set(re.findall(r"\b(\w+)\s*\(", (ROOT / "host" / "crt.h").read_text(errors="replace")))
    groups: dict[str, list[str]] = {"PC layer (PC.h)": [], "C runtime (crt.h)": [], "other": []}
    for s in missing:
        if s in pc:
            groups["PC layer (PC.h)"].append(s)
        elif s in crt:
            groups["C runtime (crt.h)"].append(s)
        else:
            groups["other"].append(s)
    print(f"{len(objs)} objects, {len(defined)} symbols defined, {len(missing)} needed from outside")
    for k, v in groups.items():
        print(f"  {k}: {len(v)}  e.g. {', '.join(v[:8])}")

if __name__ == "__main__":
    main()
