#!/usr/bin/env python3
# The retail executable's tables of functions (AI procedures, conditions,
# mechanics, callbacks) are each called through one pointer type. A function
# that is void in the C still leaves something in v0 on the EE, and callers
# went on from it; the PC calls each entry with its C type, so a void entry in
# a table that returns values hands its caller garbage (fn_p_stTextSuperImposed
# and fn_p_SetMechanicClimbSpeedLimit made the AI call address 0).
# Lists the entries whose C type is not their table's, with what the retail
# code leaves in v0. config/table_checked.txt names the ones found right.
#
#   check_tables.py --decomp decomp --c-functions build/guest/c_functions.json

from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

WORD = re.compile(r"/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]+\s*\*/\s*\.word\s+([A-Za-z_]\w*)")

def tables(decomp: Path, names: dict[str, list]) -> list[list[tuple[int, str]]]:
    out = []
    for f in sorted((decomp / "asm" / "data").rglob("*.s")):
        run: list[tuple[int, str]] = []
        last = None
        for line in f.read_text(errors="replace").splitlines():
            m = WORD.search(line)
            if m and m.group(2) in names:
                a = int(m.group(1), 16)
                if last is not None and a != last + 4 and run:
                    out.append(run)
                    run = []
                run.append((a, m.group(2)))
                last = a
            elif run:
                out.append(run)
                run, last = [], None
        if run:
            out.append(run)
    return [t for t in out if len(t) >= 3]

def _asm(decomp: Path, _cache: dict = {}) -> dict[str, list[str]]:
    if not _cache:
        for f in sorted((decomp / "asm").rglob("*.s")):
            if "data" in f.parts or "nonmatchings" in f.parts:
                continue
            cur = None
            for line in f.read_text(errors="replace").splitlines():
                g = re.match(r"\s*glabel\s+(\w+)", line)
                if g:
                    cur = None if g.group(1) in _cache else _cache.setdefault(g.group(1), [])
                    continue
                if re.match(r"\s*endlabel", line):
                    cur = None
                elif cur is not None and "*/" in line:
                    ins = line.split("*/", 1)[1].strip()
                    if ins and not ins.startswith("."):
                        cur.append(ins)
    return _cache

def _operands(ins: str) -> tuple[str, list[str]]:
    op, _, rest = ins.partition(" ")
    return op, [x.strip() for x in re.split(r"[,()]", rest) if x.strip()]

_NO_DEST = {"sw", "sd", "sh", "sb", "swc1", "sdc1", "sq", "swl", "swr", "sdl", "sdr", "beq", "bne", "beql", "bnel",
            "blez", "bgtz", "bltz", "bgez", "blezl", "bgtzl", "bltzl", "bgezl", "jr", "jalr", "mtc1", "teq", "tne"}

def reads_before_write(decomp: Path, name: str, regs: list[str]) -> list[str]:
    ins = _asm(decomp).get(name, [])
    written, read = set(), []
    left = -1
    for s in ins:
        if left == 0:
            break
        left -= left > 0
        op, ops = _operands(s)
        srcs = ops if op in _NO_DEST else ops[1:]
        if op == "mfc1" or op == "cfc1":
            srcs = []
        if op == "mtc1":
            srcs = ops[:1]
        for r in regs:
            if r in srcs and r not in written and r not in read:
                read.append(r)
        if op not in _NO_DEST and ops:
            written.add(ops[0])
        if op in ("jal", "bal", "jalr", "jr") or op.startswith("b") and op not in ("break",):
            left = 1 if left < 0 else left
    return read

def v0_at_return(decomp: Path, name: str) -> str:
    ins = _asm(decomp).get(name)
    if not ins:
        return "no asm"
    ret = max((i for i, s in enumerate(ins) if s.startswith("jr ") and "$ra" in s), default=None)
    if ret is None:
        return "no jr $ra"
    for s in reversed(ins[:ret + 2]):
        op, ops = _operands(s)
        if op in ("jal", "jalr", "bal"):
            return f"v0 from {' '.join(s.split())}"
        if op not in _NO_DEST and ops and ops[0] == "$v0":
            return f"v0 set: {' '.join(s.split())}"
    return "v0 not set"

def column_type(entries: list, kind) -> tuple | None:
    (k, n), = Counter(kind(e)[1] for e in entries).most_common(1)
    if n * 2 <= len(entries):
        return None
    return max((kind(e)[0] for e in entries), key=len), k

def columns(t: list, kind) -> tuple[int, list]:
    for stride in range(1, 17):
        cols = [t[c::stride] for c in range(stride)]
        if any(len(c) < 2 for c in cols):
            break
        types = [column_type(c, kind) for c in cols]
        uniform = all(sum(kind(e) == max(set(map(kind, c)), key=list(map(kind, c)).count) for e in c) * 2 > len(c)
                      for c in cols)
        if uniform and None not in types:
            return stride, types
    return 0, []

def main() -> int:
    ap = argparse.ArgumentParser(description="Check the game's tables of functions against the C's types.")
    ap.add_argument("--decomp", type=Path, required=True)
    ap.add_argument("--c-functions", type=Path, required=True)
    ap.add_argument("--checked", type=Path, help="entries looked at and found right")
    ap.add_argument("--all", action="store_true", help="list the checked entries too")
    a = ap.parse_args()
    cfuncs = json.loads(a.c_functions.read_text())
    names = {v["name"]: v["type"] for v in cfuncs.values()}
    checked = set()
    if a.checked and a.checked.exists():
        checked = {l.split("#")[0].strip() for l in a.checked.read_text().splitlines()} - {""}
    bad = scanned = entries = 0
    kind = lambda e: (tuple(names[e[1]][0]), bool(names[e[1]][1]))
    runs = tables(a.decomp, names)
    by_len: dict[int, list] = {}
    for t in runs:
        by_len.setdefault(len(t), []).append(t)
    for t in runs:
        stride, types = columns(t, kind)
        same = by_len[len(t)]
        if not stride and len(same) >= 3:
            types = [column_type([r[k] for r in same], kind) for k in range(len(t))]
            if None not in types:
                stride = len(t)
        if not stride:
            continue
        scanned += 1
        entries += len(t)
        for i, (addr, n) in enumerate(t):
            params, result = types[i % stride]
            p, r = names[n]
            why = []
            if result and not r:
                why.append(f"void in a column of results ({v0_at_return(a.decomp, n)})")
            missing = [f"$a{k}" for k in range(len(p), min(len(params), 8)) if params[k] == "i32"]
            if len(p) < len(params) and missing:
                read = reads_before_write(a.decomp, n, missing)
                if read:
                    why.append(f"{len(p)} parameters, the column's {len(params)}, and it reads {', '.join(read)}")
            if not why:
                continue
            ok = n in checked
            if ok and not a.all:
                continue
            bad += not ok
            print(f"{'checked: ' if ok else ''}{n} (table at {t[0][0]:08X}, slot {i}"
                  f"{f', column {i % stride} of {stride}' if stride > 1 else ''}): {'; '.join(why)}")
    print(f"check_tables: {entries} entries of {scanned} tables of functions, {bad} not of their table's type")
    if bad:
        print(f"check_tables: {bad} table entries whose C type is not their table's: give them its type under "
              "PC_PORT, or name them in config/table_checked.txt once found right", file=sys.stderr)
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
