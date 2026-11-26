#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from elf32 import Elf32

VIEW = re.compile(r"^__revo_(?:view\d+|sig[0-9a-f]+|cast[0-9a-f]+)__(.+)$")

def retail_symbols(elf_path: Path) -> tuple[dict[str, int], set[str]]:
    e = Elf32(elf_path)
    addrs: dict[str, set[int]] = {}
    for s in e.symbols():
        if not s.name or s.type in (3, 4) or s.name.endswith(".NON_MATCHING"):
            continue
        if s.shndx == 0:
            continue
        addrs.setdefault(s.name, set()).add(s.value)
    unique = {n: next(iter(v)) for n, v in addrs.items() if len(v) == 1}
    ambiguous = {n for n, v in addrs.items() if len(v) > 1}
    return unique, ambiguous

def base_name(n: str) -> str:
    while True:
        m = VIEW.match(n)
        if not m:
            return n
        n = m.group(1)

def resolve(n: str, table: dict[str, int]) -> int | None:
    b = base_name(n)
    if b in table:
        return table[b]
    if re.fullmatch(r"0x[0-9A-Fa-f]+", b):
        return int(b, 16)
    m = re.fullmatch(r"(?:D|func|unk|dword|word|byte|off|flt|dbl|jpt|jtbl|loc)_([0-9A-Fa-f]{6,8})", b)
    if m:
        return int(m.group(1), 16)
    return None

DRV_TYPES = {"void": None, "u32": ("u32", "i"), "s32": ("s32", "i"), "u8": ("u8", "i"),
             "GuestPtr": ("u32", "i"), "float": ("f32", "f")}

def driver_api(header: Path) -> dict[int, tuple[str, str, list[str]]]:
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", header.read_text(), flags=re.S)
    text = re.sub(r"#\s*define\s+PC_DRIVER[^\n]*", "", text)
    api = {}
    for addr, ret, name, params in re.findall(
            r"PC_DRIVER\(\s*(0x[0-9A-Fa-f]+)\s*\)\s*(\w+)\s+(\w+)\s*\(([^)]*)\)\s*;", text):
        types = []
        for prm in (q.strip() for q in params.split(",")):
            if prm and prm != "void":
                t = prm.split()[0]
                if t not in DRV_TYPES or t == "void":
                    sys.exit(f"driver.h: {name}: parameter type {t!r} (gli/driver.h takes {', '.join(DRV_TYPES)})")
                types.append(t)
        if ret not in DRV_TYPES:
            sys.exit(f"driver.h: {name}: return type {ret!r}")
        a = int(addr, 16)
        if a in api:
            sys.exit(f"driver.h: {name}: {addr} is also {api[a][0]}'s")
        api[a] = (name, ret, types)
    return api

def c_functions(defined: dict[str, tuple], decomp_elf: Path, retail: Path) -> tuple[dict, list]:
    table, ambiguous = retail_symbols(decomp_elf)
    cf: dict[int, dict] = {}
    for name in sorted(defined):
        if base_name(name) in ambiguous:
            continue
        a = resolve(name, table)
        if a is not None:
            cf.setdefault(a, {"name": name, "type": defined[name]})
    d, r = Elf32(decomp_elf), Elf32(retail)
    size = {}
    for s in d.symbols():
        if s.type == 2 and s.size and s.shndx:
            size.setdefault(s.value, s.size)
    kept, rejected = {}, []
    for slot, info in cf.items():
        n = size.get(slot)
        try:
            same = bool(n) and d.read(slot, n) == r.read(slot, n)
        except KeyError:
            same = False
        if same:
            kept[slot] = info
        else:
            rejected.append({"addr": f"0x{slot:08X}", "name": info["name"], "size": n or 0})
    return kept, rejected
