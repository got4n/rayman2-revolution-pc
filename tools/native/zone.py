#!/usr/bin/env python3
# The game's memory as the PS2 had it. The program holds a zone at the PS2's
# addresses: its offset in the program's block (host/native_mem.c) is the
# PS2 address, so the player's SLUS_201.38 is copied there as the console's
# loader copied it (host/complete.c), and the pointers it holds are right as
# they are. The game's C reaches its data there: each name of the game's data
# becomes the block's base plus its address, and a function's address taken in
# C is its PS2 address, as the data holds. A call through a pointer goes
# through revo_code (host/complete.c), which finds the native function for an
# address in the zone.
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT / "tools"))
import target
from retail import base_name, resolve

DECOMP = ROOT / "decomp"
OUT = ROOT / "build" / "native" / "zone"
LO, SIZE = 0x100000, 0x440000

CONFIG = DECOMP / "config"
ELF = DECOMP / "build" / "SLUS_201.38.elf"
ENCODED = re.compile(r"(?:D|func|unk|dword|word|byte|off|flt|dbl|jpt|jtbl|loc)_[0-9A-Fa-f]{6,8}|D_[0-9A-Fa-f]+")

_symbols: dict[str, tuple[int, bool]] | None = None
_elf: dict[str, int] | None = None
missing: dict[str, int] = {}

def symbols() -> dict[str, tuple[int, bool]]:
    """The decomp's names (config/revo.symbol_addrs.txt and its linker scripts): address, and whether a function."""
    global _symbols
    if _symbols is None:
        _symbols = {}
        for line in (CONFIG / "revo.symbol_addrs.txt").read_text(errors="replace").splitlines():
            m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)$", line)
            if m and "ignore:true" not in m.group(3):
                _symbols.setdefault(m.group(1), (int(m.group(2), 16), "type:func" in m.group(3)))
        aliases: dict[str, str | int] = {}
        for f in ("revo.undefined_syms.txt", "revo.undefined_syms_auto.txt", "revo.undefined_funcs_auto.txt"):
            path = CONFIG / f
            if not path.exists():
                continue
            for line in path.read_text(errors="replace").splitlines():
                m = re.match(r"\s*(?:PROVIDE\()?\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+|\w+)\s*\)?\s*;", line)
                if m:
                    v = m.group(2)
                    aliases.setdefault(m.group(1), int(v, 16) if v.startswith("0x") else v)
        functions = {a for a, code in _symbols.values() if code}

        def target_of(name: str, depth: int = 0):
            if name in _symbols:
                return _symbols[name]
            v = aliases.get(name)
            if v is None and ENCODED.fullmatch(name):
                v = resolve(name, {})
            if v is None or depth > 8:
                return None
            if isinstance(v, int):
                return v, v in functions or name.startswith("func_")
            return target_of(v, depth + 1)

        for name in aliases:
            if name not in _symbols and (s := target_of(name)):
                _symbols[name] = s
    return _symbols

def elf_symbols() -> dict[str, int]:
    """The decomp's PS2 build, when it is there: to check the names above against."""
    global _elf
    if _elf is None:
        _elf = {}
        if ELF.exists():
            from elf32 import Elf32
            for s in Elf32(ELF).symbols():
                if s.name and s.shndx and s.bind != 0 and s.type in (0, 1, 2) and not s.name.endswith(".NON_MATCHING"):
                    _elf.setdefault(s.name, s.value)
    return _elf

def look(name: str) -> tuple[int, bool] | None:
    n = name.strip('"')
    if n.startswith("\\01"):
        n = n[3:]
        if target.MAC and n.startswith("_"):
            n = n[1:]
    t = symbols()
    for c in (n, base_name(n)):
        if c in t:
            return t[c]
    b = base_name(n)
    if ENCODED.fullmatch(b):
        a = resolve(b, {})
        return a, b.startswith("func_")
    if b in elf_symbols():
        missing[b] = elf_symbols()[b]
    return None

def check() -> list[str]:
    """What the decomp's config says that its PS2 build does not."""
    elf = elf_symbols()
    return [f"{n} = {a:#x} in the config, {elf[n]:#x} in decomp/build" for n, (a, _) in sorted(symbols().items())
            if n in elf and elf[n] != a]

def in_zone(a: int) -> bool:
    return LO <= a < SIZE

NAME = r'(?:"[^"]+"|[-\w.$]+)'
EXTERNAL = re.compile(rf"^@({NAME}) = external\b.*\b(?:global|constant)\b")
DEFINED = re.compile(rf"^@({NAME}) = (?!external\b)((?:[\w()]+ )*?)(global|constant)\b")
FUNC = re.compile(rf"^(?:declare|define)\b[^@]*@({NAME})\(")
FP = re.compile(r"^(\s*)(%__revo_fp\d+) = (addrspacecast .*)$")

CALLEE = " to ptr addrspace(271))("

def at(a: int) -> str:
    return f"getelementptr (i8, ptr {target.IMAGE_BASE_IR}, i64 {a})"

def names_pattern(names, after: str) -> re.Pattern:
    alt = "|".join(re.escape(n) for n in sorted(names, key=len, reverse=True))
    return re.compile(r"(?<!blockaddress\()@(" + alt + r")" + after)

def skip(line: str) -> bool:
    return line.startswith(("!", "attributes", "declare", "define", "source_filename", "target "))

def rewrite(ir: str) -> tuple[str, dict]:
    """The game's data at its addresses, its functions' addresses as the PS2's."""
    data, funcs, out, info = {}, {}, [], {"taken": [], "data": [], "inits": []}
    for line in ir.split("\n"):
        m = EXTERNAL.match(line)
        if m:
            s = look(m.group(1))
            if s:
                data[m.group(1)] = s[0]
        m = DEFINED.match(line)
        if m:
            s = look(m.group(1))
            if s and in_zone(s[0]) and "internal" not in m.group(2) and "private" not in m.group(2):
                # The game's data written in the C: the C's value replaces the copy's at the start.
                init = f'"{m.group(1).strip(chr(34))}.revo_init"'
                info["inits"].append([init.strip('"'), s[0]])
                data[m.group(1)] = s[0]
                line = f"@{init} = " + line[m.end(1) + 3:]
        m = FUNC.match(line)
        if m:
            s = look(m.group(1))
            if s and in_zone(s[0]):
                funcs[m.group(1)] = s[0]
        out.append(line)
    text = "\n".join(out)
    if data:
        pat = names_pattern(data, r"(?![-\w.$])")
        lines = []
        for line in text.split("\n"):
            if not (skip(line) or EXTERNAL.match(line)):
                line = pat.sub(lambda m: at(data[m.group(1)]), line)
            lines.append(line)
        text = "\n".join(lines)
        info["data"] = sorted(n.strip('"') for n in data)
    if funcs:
        pat = names_pattern(funcs, r"(?![-\w.$(])")
        taken = set()

        def value(m: re.Match) -> str:
            # A call to a function through its address as a 4-byte pointer stays a call to it.
            if m.string.startswith(CALLEE, m.end()):
                return m.group(0)
            taken.add(m.group(1).strip('"'))
            return at(funcs[m.group(1)])

        text = "\n".join(line if skip(line) else pat.sub(value, line) for line in text.split("\n"))
        info["taken"] = sorted(taken)
    return text, info

NUMBER = re.compile(r'@"(0x[0-9A-Fa-f]+)"')

def numbers(ir: str) -> str:
    """A name that is a number (glue_native's .set "0x..."): the address itself. On Linux, where the program is
    position independent code, such a name would be reached through the GOT, which the assembler cannot read."""
    if not target.LINUX:
        return ir
    def one(m: re.Match) -> str:
        return f"inttoptr (i64 {int(m.group(1), 16)} to ptr)"
    return "\n".join(line if line.startswith("@") else NUMBER.sub(one, line) for line in ir.split("\n"))

def indirect(ir: str) -> str:
    """A call through a pointer: revo_code gives the native function of an address in the zone."""
    out = []
    for line in ir.split("\n"):
        m = FP.match(line)
        if m:
            ind, name, rest = m.groups()
            out += [f"{ind}{name}.raw = {rest}", f"{ind}{name} = call ptr @revo_code(ptr {name}.raw)"]
            continue
        out.append(line)
    text = "\n".join(out)
    if "@revo_code(" in text and not re.search(r"^declare [^\n]*@revo_code\(", text, re.M):
        text += "\ndeclare ptr @revo_code(ptr)\n"
    return text

def save(stem: str, info: dict) -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / f"{stem}.json").write_text(json.dumps(info) + "\n")

def load_all() -> dict:
    out = {"taken": set(), "data": set(), "inits": []}
    for f in sorted(OUT.glob("*.json")):
        d = json.loads(f.read_text())
        out["taken"] |= set(d["taken"])
        out["data"] |= set(d["data"])
        out["inits"] += d["inits"]
    return out

def zone_asm() -> str:
    """The zone: first in the program, so that its offsets reach the PS2's addresses. Its bytes are in the
    file, zeros, so that the program can write a copy of itself with the game in it (host/complete.c)."""
    if target.MAC:
        return f'.section __DATA_CONST,__zone\n.globl _revo_zone\n.p2align 14\n_revo_zone:\n.space {SIZE:#x}\n'
    if target.LINUX:
        # Read-only data first in the link (zone.s.o is the first object): lld puts it right after the headers.
        return f'.section .revo_zone,"a",@progbits\n.globl revo_zone\n.p2align 12\nrevo_zone:\n.zero {SIZE:#x}\n'
    return f'.section .text,"xr"\n.globl revo_zone\n.p2align 12\nrevo_zone:\n.zero {SIZE:#x}\n'

MAC_LINK = ["-Wl,-rename_section,__TEXT,__text,__CODE,__text", "-Wl,-rename_section,__TEXT,__const,__CODE,__const",
            "-Wl,-rename_section,__TEXT,__cstring,__CODE,__cstring", "-Wl,-segprot,__CODE,rx,rx"]

def add_missing() -> int:
    """Add to config/revo.symbol_addrs.txt the names of the decomp's build the PC build needs (zone.py --add)."""
    if not ELF.exists():
        sys.exit("needs the decomp's PS2 build (decomp/build/SLUS_201.38.elf) and build/native/ll (build.py build)")
    used = set()
    for f in sorted((ROOT / "build" / "native" / "ll").glob("*.ll")):
        if f.name.endswith(".fixed.ll"):
            continue
        for line in f.read_text(encoding="utf-8", errors="replace").splitlines():
            m = EXTERNAL.match(line) or FUNC.match(line)
            if m:
                look(m.group(1))
    for v in re.findall(r"^\s*X\((\w+)\)", (ROOT / "host" / "game_vars.h").read_text(), re.M):
        look(v)
    from elf32 import Elf32
    kinds = {s.name: s.type for s in Elf32(ELF).symbols() if s.name}
    path = CONFIG / "revo.symbol_addrs.txt"
    lines = path.read_text(errors="replace").splitlines()
    named = {int(m.group(1), 16) for line in lines
             if (m := re.match(r"\s*\w+\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)", line)) and "ignore:true" not in m.group(2)}
    # splat takes one name an address: a second one is a PROVIDE of the linker script, which the C's wins over.
    new, provide = [], []
    for n, a in sorted(missing.items(), key=lambda x: x[1]):
        (provide if a in named else new).append((a, n))
        named.add(a)
    out, k = [], 0
    for line in lines:
        m = re.match(r"\s*\w+\s*=\s*0x([0-9A-Fa-f]+)", line)
        while m and k < len(new) and new[k][0] < int(m.group(1), 16):
            a, n = new[k]
            out.append(f"{n} = 0x{a:08X};" + (" // type:func" if kinds.get(n) == 2 else ""))
            k += 1
        out.append(line)
    out += [f"{n} = 0x{a:08X};" + (" // type:func" if kinds.get(n) == 2 else "") for a, n in new[k:]]
    path.write_text("\n".join(out) + "\n")
    if provide:
        u = CONFIG / "revo.undefined_syms.txt"
        u.write_text(u.read_text().rstrip("\n") + "\n" + "".join(f"PROVIDE({n} = 0x{a:08X});\n" for a, n in provide))
    print(f"decomp/config: {len(new)} name(s) in revo.symbol_addrs.txt, {len(provide)} in revo.undefined_syms.txt")
    return 0

if __name__ == "__main__":
    if sys.argv[1:] != ["--add"]:
        sys.exit("zone.py --add: add the names the PC build needs to decomp/config/revo.symbol_addrs.txt")
    raise SystemExit(add_missing())
