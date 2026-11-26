#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MANIFEST = ROOT / "config" / "ps2_originals.sha256"
SUFFIXES = (".c", ".cpp", ".h", ".inc")

_PC_IF = re.compile(rb"^\s*#\s*if(n?)def\s+PC_PORT\b")
_IF = re.compile(rb"^\s*#\s*if")
_ELSE = re.compile(rb"^\s*#\s*else\b")
_ELIF = re.compile(rb"^\s*#\s*elif\b")
_ENDIF = re.compile(rb"^\s*#\s*endif\b")

def _unifdef_pc(data: bytes, path: Path) -> bytes:
    out = []
    stack = []
    for line in data.splitlines(keepends=True):
        keeping = all(e is None or e[0] for e in stack)
        m = _PC_IF.match(line)
        if m:
            stack.append([m.group(1) == b"n"])
        elif _IF.match(line):
            if keeping:
                out.append(line)
            stack.append(None)
        elif _ELIF.match(line):
            if stack and stack[-1] is not None:
                sys.exit(f"check_ps2: #elif in a PC_PORT block: {path}")
            if keeping:
                out.append(line)
        elif _ELSE.match(line):
            if stack and stack[-1] is not None:
                stack[-1][0] = not stack[-1][0]
            elif keeping:
                out.append(line)
        elif _ENDIF.match(line):
            if not stack:
                sys.exit(f"check_ps2: #endif without #if: {path}")
            if stack.pop() is None and all(e is None or e[0] for e in stack):
                out.append(line)
        elif keeping:
            out.append(line)
    if stack:
        sys.exit(f"check_ps2: unbalanced conditionals: {path}")
    return b"".join(out)

def without_pc(path: Path) -> bytes:
    data = path.read_bytes()
    if b"PC_PORT" not in data:
        return data
    if shutil.which("unifdef") is None:
        return _unifdef_pc(data, path)
    r = subprocess.run(["unifdef", "-UPC_PORT", str(path)], capture_output=True)
    if r.returncode not in (0, 1):
        sys.exit(f"check_ps2: unifdef failed on {path}: {r.stderr.decode().strip()}")
    return r.stdout

def main() -> int:
    expected = {}
    for line in MANIFEST.read_text().splitlines():
        digest, rel = line.split(maxsplit=1)
        expected[rel] = digest
    bad = 0
    seen = set()
    for top in ("decomp", "ubisound"):
        for d in ("src", "include"):
            for path in sorted((ROOT / top / d).rglob("*")):
                if not path.is_file() or path.suffix not in SUFFIXES:
                    continue
                rel = path.relative_to(ROOT).as_posix()
                if "/include/PC/" in rel:
                    continue
                if rel not in expected:
                    print(f"not in {MANIFEST.name}: {rel}")
                    bad += 1
                    continue
                seen.add(rel)
                if hashlib.sha256(without_pc(path)).hexdigest() != expected[rel]:
                    print(f"PS2 code changed: {rel}")
                    bad += 1
    for rel in sorted(set(expected) - seen):
        print(f"missing: {rel}")
        bad += 1
    print("PS2 code: the original's" if not bad else f"PS2 code: {bad} problem(s)")
    return 1 if bad else 0

if __name__ == "__main__":
    raise SystemExit(main())
