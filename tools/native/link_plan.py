#!/usr/bin/env python3
from __future__ import annotations

import argparse
import collections
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(HERE))
from retail import base_name, resolve
import undefined
import target
import zone

DECOMP = ROOT / "decomp"
RW3 = "src__tempgrp__GliGlou__Drivers__RW3__"

def names_in(header: Path, pattern: str) -> set[str]:
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", header.read_text(errors="replace"), flags=re.S)
    return set(re.findall(pattern, text))

def driver(header: Path) -> dict[int, str]:
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", header.read_text(errors="replace"), flags=re.S)
    text = re.sub(r"#\s*define\s+PC_DRIVER[^\n]*", "", text)
    return {int(a, 16): n for a, n in re.findall(r"PC_DRIVER\(\s*(0x[0-9A-Fa-f]+)\s*\)\s*\w+\s+(\w+)\s*\(", text)}

def plan() -> dict[str, list[tuple[str, str]]]:
    objs = [o for o in sorted((ROOT / "build" / "native" / "obj").glob("*.o")) if not o.name.startswith(RW3)]
    defined, used = undefined.nm(objs)
    from_code = set(used)
    # A function whose address the C takes (zone.py) is one the program must have.
    taken = zone.load_all()["taken"]
    used |= taken
    from_code |= taken
    missing = sorted(used - defined)
    retail = {n: a for n, (a, _) in zone.symbols().items()}
    by_addr = collections.defaultdict(list)
    for n, a in retail.items():
        by_addr[a].append(n)
    drv = driver(ROOT / "gli" / "driver.h")
    pc = names_in(DECOMP / "include" / "PC" / "PC.h", r"PC_API\s+[^;(]+?\b(\w+)\s*\(")
    crt = names_in(ROOT / "host" / "crt.h", r"\bcrt_(\w+)\s*\(")
    out = collections.defaultdict(list)
    for s in missing:
        b = base_name(s)
        a = resolve(s, retail)
        if a is not None and a in drv:
            out["driver (gli/driver.h)"].append((s, drv[a]))
        elif b in pc:
            out["PC layer (PC.h)"].append((s, b))
        elif b in crt:
            out["C runtime (crt.h)"].append((s, b))
        elif a is not None and (c := [n for n in by_addr.get(a, []) if n in defined and n != s]):
            out["decomp C under another name"].append((s, c[0]))
        elif b != s and b in defined:
            out["view of a defined name"].append((s, b))
        elif s not in from_code:
            out["only the data points at it"].append((s, ""))
        elif a is None:
            out["no retail address"].append((s, ""))
        else:
            out["nothing served"].append((s, f"{a:#010x}"))
    return out

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--list")
    args = ap.parse_args()
    p = plan()
    print(f"{sum(map(len, p.values()))} symbols needed")
    for k, v in sorted(p.items(), key=lambda kv: -len(kv[1])):
        print(f"  {k}: {len(v)}  e.g. {', '.join(f'{s}->{t}' if t else s for s, t in v[:5])}")
        if args.list and args.list in k:
            for s, t in v:
                print(f"      {s} {t}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
