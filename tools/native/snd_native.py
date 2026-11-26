#!/usr/bin/env python3
from __future__ import annotations

import argparse
import concurrent.futures as cf
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(HERE))
import target as platform
import build_native
import base32
import irfix
import ptr32
import rva
from build_cache import Cache, digest

UBI = ROOT / "ubisound"
OUT = ROOT / "build" / "native" / "snd"
CLANG = build_native.CLANG
DEFS = ["-DM2CTX", "-DPC_PORT", f"-I{UBI / 'include'}", f"-I{UBI / 'src'}"]
CFLAGS = [f for f in build_native.CFLAGS if not f.startswith("-ftrivial-auto-var-init")] + \
         ["-ftrivial-auto-var-init=pattern"]
PREFIX = "ubi_"
CACHE = Cache(OUT / "cache")

def build(src: Path) -> dict:
    stem = src.stem
    pre, rew, ll, llf, obj = (OUT / d / f"{stem}{ext}" for d, ext in
                              (("pre", ".i"), ("src", ".i"), ("ll", ".ll"), ("ll", ".fixed.ll"), ("obj", ".o")))
    for p in (pre, rew, ll, obj):
        p.parent.mkdir(parents=True, exist_ok=True)
    res = {"src": src.name, "ok": False}
    p = subprocess.run([CLANG, *build_native.TARGET, "-E", "-P", *DEFS, "-x", "c", str(src), "-o", str(pre)],
                       capture_output=True, text=True)
    if p.returncode:
        res.update(stage="preprocess", error=build_native.first_error(p.stderr))
        return res
    # Kept by the preprocessed text: the same text built by the same tools gives the same object.
    key = digest(pre, build_native.TOOLS_KEY, Path(__file__).resolve(), *CFLAGS)
    hit = CACHE.get(stem, key, [obj])
    if hit is not None:
        return hit
    res = build_preprocessed(pre, rew, ll, llf, obj, res)
    CACHE.put(stem, key, res)
    return res

def build_preprocessed(pre: Path, rew: Path, ll: Path, llf: Path, obj: Path, res: dict) -> dict:
    try:
        info = ptr32.rewrite(pre, rew, False)
    except Exception as e:
        res.update(stage="ptr32", error=repr(e)[:300])
        return res
    p = subprocess.run([CLANG, *build_native.TARGET, *CFLAGS, "-x", "c", "-S", "-emit-llvm", str(rew), "-o", str(ll)],
                       capture_output=True, text=True)
    if p.returncode:
        res.update(stage="clang", error=build_native.first_error(p.stderr))
        return res
    fixed, _, _ = irfix.fix(ll.read_text(encoding="utf-8"), info.pop("extensions", {}), preempt=False)
    try:
        fixed, _ = base32.transform(fixed)
    except SystemExit as e:
        res.update(stage="base32", error=str(e)[:300])
        return res
    llf.write_text(fixed, encoding="utf-8")
    asm = obj.with_suffix(".s")
    p = subprocess.run([CLANG, *build_native.TARGET, build_native.OPT, "-fno-addrsig", "-S", "-x", "ir", str(llf),
                        "-o", str(asm)], capture_output=True, text=True)
    if p.returncode:
        res.update(stage="codegen", error=build_native.first_error(p.stderr))
        return res
    asm.write_text(rva.rva(asm.read_text(encoding="utf-8")), encoding="utf-8")
    p = subprocess.run([CLANG, *build_native.TARGET, "-c", "-x", "assembler", str(asm), "-o", str(obj)],
                       capture_output=True, text=True)
    if p.returncode:
        res.update(stage="assemble", error=build_native.first_error(p.stderr))
        return res
    res.update(ok=True, obj=str(obj))
    return res

def symbols(obj: Path) -> tuple[set[str], set[str]]:
    out = subprocess.run([platform.tool("llvm-nm"), str(obj)], capture_output=True, text=True, check=True).stdout
    defined, undefined = set(), set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "U":
            undefined.add(platform.c_name(parts[1]))
        elif len(parts) == 3 and parts[1] in "TDBRCS":
            defined.add(platform.c_name(parts[2]))
    return defined, undefined

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    a = ap.parse_args()
    srcs = sorted(UBI.glob("src/*.c"))
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        results = list(ex.map(build, srcs))
    bad = [r for r in results if not r["ok"]]
    print(f"sound driver natively: {len(results) - len(bad)}/{len(results)} sources build")
    for r in bad:
        print(f"  {r['src']} ({r['stage']}): {r['error']}")
    if bad:
        return 1
    objs = [Path(r["obj"]) for r in results]
    defined, undefined = set(), set()
    for o in objs:
        d, u = symbols(o)
        defined |= d
        undefined |= u
    names = {s: PREFIX + s for s in defined}
    host = sorted(u for u in undefined - defined if u.startswith("pc_"))
    names.update({s: "snd_" + s for s in host})
    other = sorted(undefined - defined - set(host))
    renames = OUT / "renames.txt"
    text = "".join(f"{platform.sym_name(k)} {platform.sym_name(v)}\n" for k, v in sorted(names.items()))
    if not renames.exists() or renames.read_text(encoding="utf-8") != text:
        renames.write_text(text, encoding="utf-8")
    final = OUT / "final"
    final.mkdir(exist_ok=True)
    for o in objs:
        dst = final / o.name
        if dst.exists() and dst.stat().st_mtime >= max(o.stat().st_mtime, renames.stat().st_mtime):
            continue
        subprocess.run([platform.tool("llvm-objcopy"), f"--redefine-syms={renames}", str(o), str(dst)], check=True)
    print(f"  {len(defined)} symbols prefixed {PREFIX}, {len(host)} PC layer calls to snd_pc_*; "
          f"other undefined: {', '.join(other) or 'none'}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
