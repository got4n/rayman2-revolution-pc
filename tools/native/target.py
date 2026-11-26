#!/usr/bin/env python3
from __future__ import annotations

import os
import re
import shutil
import sys
from pathlib import Path

def _default() -> str:
    if sys.platform == "darwin":
        return "mac"
    return "linux" if sys.platform.startswith("linux") else "windows"

NAME = os.environ.get("REVO_NATIVE_TARGET", _default())
if NAME not in ("windows", "mac", "linux"):
    raise SystemExit(f"REVO_NATIVE_TARGET={NAME}: windows, mac or linux")
MAC = NAME == "mac"
WINDOWS = NAME == "windows"
LINUX = NAME == "linux"
# long is 8 bytes on macOS and Linux, 4 on Windows and the PS2.
LP64 = not WINDOWS
# Where the program is linked: 4 GB aligned, so that a 4-byte pointer is the low half of the address. ELF has
# no relocation for an offset from the image, so on Linux the program is not position independent and the
# offsets are from this address (build.py links it there).
IMAGE_BASE = 0x200000000

def _llvm_dir() -> Path | None:
    if os.environ.get("REVO_LLVM"):
        return Path(os.environ["REVO_LLVM"])
    if MAC:
        for d in ("/opt/homebrew/opt/llvm/bin", "/usr/local/opt/llvm/bin"):
            if Path(d, "clang").exists():
                return Path(d)
    if LINUX:
        # apt's newest LLVM (clang-20 and up: clang 18 casts ptr32 pointers wrongly).
        found = []
        for d in Path("/usr/lib").glob("llvm-*/bin"):
            v = d.parent.name[5:]
            if v.isdigit() and (d / "clang").exists():
                found.append((int(v), d))
        if found:
            return max(found)[1]
    return None

LLVM = _llvm_dir()

def _libclang() -> None:
    if LLVM is None:
        return
    lib = LLVM.parent / "lib"
    site = lib / f"python{sys.version_info.major}.{sys.version_info.minor}" / "site-packages"
    if site.is_dir() and str(site) not in sys.path:
        sys.path.insert(0, str(site))
    try:
        import clang.cindex as ci
    except ImportError:
        return
    names = ["libclang.dylib"] if MAC else ["libclang.so", *sorted(lib.glob("libclang-*.so.1"))]
    for dylib in (lib / n for n in names):
        if dylib.exists() and not ci.Config.loaded:
            try:
                ci.Config.set_library_file(str(dylib))
            except Exception:
                pass
            break

if MAC or LINUX:
    _libclang()

def tool(name: str) -> str:
    if LLVM is not None and (LLVM / name).exists():
        return str(LLVM / name)
    return shutil.which(name) or name

CLANG = os.environ.get("REVO_CLANG") or tool("clang")
TRIPLE = {"mac": "arm64-apple-macos11.0", "linux": "x86_64-pc-linux-gnu"}.get(NAME, "x86_64-w64-windows-gnu")
TARGET = [f"--target={TRIPLE}", "-fms-extensions", "-mno-ms-bitfields"] + (["-fPIE"] if LINUX else [])
IMAGE_BASE_IR = {"mac": "@_mh_execute_header", "linux": "@__executable_start"}.get(NAME, "@__ImageBase")
IMAGE_BASE_ASM = {"mac": "__mh_execute_header", "linux": "__executable_start"}.get(NAME, "__ImageBase")
PREFIX = "_" if MAC else ""

def c_name(sym: str) -> str:
    return sym[1:] if MAC and sym.startswith("_") else sym

def sym_name(name: str) -> str:
    return PREFIX + name

SYM_LONG = re.compile(r'^(\s*)\.long\s+((?:"[^"\n]+"|[A-Za-z_.$][\w.$]*)(?:\s*[+-]\s*(?:0x[0-9A-Fa-f]+|\d+))?)\s*$',
                      re.M)

def rva(text: str) -> str:
    if MAC:
        return SYM_LONG.sub(lambda m: f"{m.group(1)}.long {m.group(2)} - {IMAGE_BASE_ASM}", text)
    if LINUX:
        return SYM_LONG.sub(lambda m: f"{m.group(1)}.long {m.group(2)} - {IMAGE_BASE:#x}", text)
    return SYM_LONG.sub(r"\1.rva \2", text)

_IDENT = re.compile(r'"([^"\n]+)"|(?<![\w.$])([A-Za-z_$][\w.$]*)')
_NAMED = re.compile(r"^(\s*)(\.globl|\.global|\.set|\.long|\.quad|\.short|\.byte)\b(.*)$")
_LABEL = re.compile(r'^(\s*)("[^"\n]+"|[A-Za-z_$][\w.$]*):(.*)$')

def _names(expr: str) -> str:
    def one(m: re.Match) -> str:
        if m.group(1) is not None:
            return f'"{PREFIX}{m.group(1)}"'
        return PREFIX + m.group(2)
    return _IDENT.sub(one, expr)

def mangle_asm(text: str) -> str:
    if not MAC:
        return text
    out = []
    for line in text.split("\n"):
        s = line.lstrip()
        if not s or s.startswith("#"):
            out.append(line)
            continue
        m = _NAMED.match(line)
        if m:
            out.append(m.group(1) + m.group(2) + _names(m.group(3)))
            continue
        m = _LABEL.match(line)
        if m and not s.startswith("."):
            out.append(m.group(1) + _names(m.group(2)) + ":" + m.group(3))
            continue
        out.append(line)
    return "\n".join(out)
