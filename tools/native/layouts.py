#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import target

import clang.cindex as ci

ROOT = Path(__file__).resolve().parent.parent.parent
GUEST = ROOT / "build" / "guest"
NATIVE = ROOT / "build" / "native"
K = ci.CursorKind

def records(path: Path, target: list[str], cxx: bool) -> dict:
    lang = ["-x", "c++", "-fno-exceptions", "-fno-rtti"] if cxx else ["-x", "c"]
    tu = ci.Index.create().parse(str(path), args=lang + target + ["-ffreestanding", "-w"], options=0)
    out = {}
    for c in tu.cursor.walk_preorder():
        if c.kind not in (K.STRUCT_DECL, K.UNION_DECL) or not c.is_definition():
            continue
        t = c.type
        size = t.get_size()
        if size < 0:
            continue
        fields = {}
        for f in c.get_children():
            if f.kind == K.FIELD_DECL and f.spelling:
                fields[f.spelling] = t.get_offset(f.spelling)
        name = c.spelling if c.spelling and "unnamed" not in c.spelling else "<anonymous>"
        out[(c.location.line, name)] = (size, t.get_align(), fields)
    return out

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", nargs="*")
    a = ap.parse_args()
    status = json.loads((NATIVE / "status.json").read_text())
    tus = [r["tu"] for r in status if r["ok"]]
    if a.only:
        tus = [t for t in tus if any(o in t for o in a.only)]
    checked, diffs, missing = 0, [], 0
    seen_types: set = set()
    for tu in tus:
        stem = tu.rsplit(".", 1)[0].replace("/", "__")
        cxx = tu.endswith(".cpp")
        ps2 = records(GUEST / "host" / f"{stem}.i", ["--target=mipsel-unknown-linux-gnu", "-fforce-enable-int128"], cxx)
        nat = records(NATIVE / "src" / f"{stem}.i", target.TARGET, cxx)
        for key, (size, align, fields) in ps2.items():
            other = nat.get(key)
            if other is None:
                missing += 1
                continue
            checked += 1
            seen_types.add(key[1])
            s2, a2, f2 = other
            bad = []
            if size != s2:
                bad.append(f"size {size} -> {s2}")
            if align != a2:
                bad.append(f"align {align} -> {a2}")
            for name, off in fields.items():
                if f2.get(name) != off:
                    bad.append(f"{name} @{off // 8 if off >= 0 else off} -> @{f2.get(name, -8) // 8}")
            if bad:
                diffs.append((tu, key, bad[:4]))
    print(f"{checked} record definitions compared ({len(seen_types)} distinct names) in {len(tus)} TUs; "
          f"{missing} not found again; {len(diffs)} differ")
    shown = set()
    for tu, key, bad in diffs:
        if key[1] in shown:
            continue
        shown.add(key[1])
        print(f"  {key[1]} ({tu}:{key[0]}): {'; '.join(bad)}")
        if len(shown) >= 30:
            break
    return 0 if not diffs else 1

if __name__ == "__main__":
    raise SystemExit(main())
