#!/usr/bin/env python3
from __future__ import annotations

import argparse
import concurrent.futures as cf
import json
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent))
import target
import base32
import irfix
import ptr32
import rva
import zone
from build_cache import Cache, compiler, digest

GUEST = ROOT / "build" / "guest"
OUT = ROOT / "build" / "native"
CLANG = target.CLANG
TARGET = target.TARGET
OPT = os.environ.get("REVO_NATIVE_OPT", "-O1")
CFLAGS = [OPT, "-g0", "-fno-strict-aliasing", "-ffreestanding", "-fno-builtin", "-fno-common", "-w",
          "-Wno-error=implicit-function-declaration", "-Wno-error=int-conversion",
          "-Wno-error=incompatible-pointer-types", "-Wno-error=return-type", "-Wno-error=implicit-int",
          "-fno-stack-protector", "-ffp-contract=off", *os.environ.get("REVO_NATIVE_CFLAGS", "").split()]

def first_error(stderr: str) -> str:
    for l in stderr.splitlines():
        if "error" in l:
            return l.strip()[:300]
    return stderr.strip()[:300]

def tools_key() -> str:
    """What builds a unit: these tools, the decomp's names that zone.py places, the flags and the compiler."""
    config = zone.CONFIG
    return digest(*(HERE / f"{m}.py" for m in ("build_native", "ptr32", "irfix", "base32", "rva", "zone", "target")),
                  HERE.parent / "retail.py", config / "revo.symbol_addrs.txt", config / "revo.undefined_syms.txt",
                  config / "revo.undefined_syms_auto.txt", config / "revo.undefined_funcs_auto.txt",
                  *TARGET, *CFLAGS, OPT, compiler(CLANG))

TOOLS_KEY = tools_key()
CACHE = Cache(OUT / "cache")

def build(tu: str) -> dict:
    """The unit's object, from the cache when its hostified C and the tools are the same as last time."""
    stem = tu.rsplit(".", 1)[0].replace("/", "__")
    src = GUEST / "host" / f"{stem}.i"
    if src.exists():
        key = digest(src, TOOLS_KEY)
        outputs = [OUT / "obj" / f"{stem}.o", OUT / "ll" / f"{stem}.fixed.ll", zone.OUT / f"{stem}.json"]
        hit = CACHE.get(stem, key, outputs)
        if hit is not None:
            return hit
        res = build_unit(tu, stem, src)
        CACHE.put(stem, key, res)
        return res
    return build_unit(tu, stem, src)

def build_unit(tu: str, stem: str, src: Path) -> dict:
    res = {"tu": tu, "ok": False}
    cxx = tu.endswith(".cpp")
    lang = ["-x", "c++", "-fno-exceptions", "-fno-rtti", "-fno-strict-return"] if cxx else ["-x", "c"]
    if not src.exists():
        res.update(stage="input", error=f"{src.name} missing: run build.py build first")
        return res
    rew, ll, llf, obj = (OUT / d / f"{stem}{ext}" for d, ext in
                         (("src", ".i"), ("ll", ".ll"), ("ll", ".fixed.ll"), ("obj", ".o")))
    for p in (rew, ll, obj):
        p.parent.mkdir(parents=True, exist_ok=True)
    try:
        res["ptr32"] = ptr32.rewrite(src, rew, cxx)
    except Exception as e:
        res.update(stage="ptr32", error=repr(e)[:300])
        return res
    p = subprocess.run([CLANG, *TARGET, *CFLAGS, *lang, "-S", "-emit-llvm", str(rew), "-o", str(ll)],
                       capture_output=True, text=True)
    if p.returncode:
        res.update(stage="clang", error=first_error(p.stderr),
                   errors=[l for l in p.stderr.splitlines() if "error" in l][:10])
        return res
    ext = res["ptr32"].pop("extensions", {})
    ir, placed = zone.rewrite(ll.read_text(encoding="utf-8"))
    ir = zone.numbers(ir)
    zone.save(stem, placed)
    fixed, statics, calls = irfix.fix(ir, ext)
    fixed = zone.indirect(fixed)
    res["ptr32"]["extended"] = len(ext)
    try:
        fixed, res["base32"] = base32.transform(fixed)
    except SystemExit as e:
        res.update(stage="base32", error=str(e)[:300])
        return res
    llf.write_text(fixed, encoding="utf-8")
    res["irfix"] = {"static": statics, "calls": calls}
    asm = obj.with_suffix(".s")
    p = subprocess.run([CLANG, *TARGET, OPT, "-fno-addrsig", "-S", "-x", "ir", str(llf), "-o", str(asm)],
                       capture_output=True, text=True)
    if p.returncode:
        res.update(stage="codegen", error=first_error(p.stderr))
        return res
    asm.write_text(rva.rva(asm.read_text(encoding="utf-8")), encoding="utf-8")
    p = subprocess.run([CLANG, *TARGET, "-c", "-x", "assembler", str(asm), "-o", str(obj)],
                       capture_output=True, text=True)
    if p.returncode:
        res.update(stage="assemble", error=first_error(p.stderr))
        return res
    res["ok"] = True
    return res

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", nargs="*")
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    a = ap.parse_args()
    status = json.loads((GUEST / "guest_status.json").read_text())
    tus = [r["tu"] for r in status if r["ok"]]
    if a.only:
        tus = [t for t in tus if any(o in t for o in a.only)]
    zone.symbols()
    if not a.only and zone.OUT.exists():
        stems = {t.rsplit(".", 1)[0].replace("/", "__") for t in tus}
        for f in zone.OUT.glob("*.json"):
            if f.stem not in stems:
                f.unlink()
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        results = list(ex.map(build, tus))
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "status.json").write_text(json.dumps(results, indent=1))
    wrong = zone.check()
    if wrong or zone.missing:
        # What is missing is found as units are built: build them all again next time, to find it again.
        for f in CACHE.dir.glob("*.json"):
            f.unlink()
        for w in wrong[:20]:
            print(f"  {w}")
        if zone.missing:
            print(f"  decomp/config/revo.symbol_addrs.txt lacks {len(zone.missing)} name(s) of the decomp's build: "
                  + ", ".join(sorted(zone.missing)[:10]) + " (python3 tools/native/zone.py --add)")
        return 1
    ok = [r for r in results if r["ok"]]
    print(f"native {target.TRIPLE} with 4-byte pointers: {len(ok)}/{len(results)} TUs build")
    by: dict[str, list] = {}
    for r in results:
        if not r["ok"]:
            by.setdefault(r["stage"], []).append(r)
    for stage, rs in by.items():
        print(f"  {stage}: {len(rs)}")
        for r in rs[:8]:
            print(f"    {r['tu']}: {r['error'][:200]}")
    return 0 if len(ok) == len(results) else 1

if __name__ == "__main__":
    raise SystemExit(main())
