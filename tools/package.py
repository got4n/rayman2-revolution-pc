#!/usr/bin/env python3
# The Windows release from a finished build (python3 build.py build first):
# dist/RaymanRe-Revolution-<version>-windows-x64.zip and SHA256SUMS.txt. The zip
# holds revolution.exe, its DLLs, README.md, the licenses and files.txt (SHA-256 of each
# file, for the updater). It stops on any game file, on build_info.json, on
# a path of this machine in the program and on any bytes of the game's
# executable in it.
#
#   package.py [--version 0.2.0]

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build"
DIST = ROOT / "dist"
NAME = "RaymanRe-Revolution"

GAME_DATA = re.compile(r"(^|/)(SLUS_201\.38|GAME\.BF|GAME\.DSC|SYSTEM\.CNF|memcard0|SOUND|VIDEO|GAME)(/|$)"
                       r"|\.(iso|bin|cue|mdf|irx|elf)$", re.I)

def version(exe: Path) -> str:
    m = re.search(rb"REVO_VERSION=([A-Za-z0-9._+-]{1,64})\0", exe.read_bytes())
    if not m:
        sys.exit(f"{exe.name} has no version: rebuild it with `python3 build.py build`")
    return m.group(1).decode()

def toolchain_bin() -> Path | None:
    cc = shutil.which(os.environ.get("CC", "clang"))
    return Path(cc).resolve().parent if cc else None

def strip(exe: Path, bindir: Path | None) -> None:
    tool = None
    for name in ("llvm-strip", "strip"):
        tool = shutil.which(name, path=str(bindir)) if bindir else None
        tool = tool or shutil.which(name)
        if tool:
            break
    if not tool:
        print("warning: no llvm-strip: the program keeps its debug information")
        return
    subprocess.run([tool, "--strip-debug", str(exe)], check=True)

def check_no_local_paths(exe: Path) -> None:
    data = exe.read_bytes()
    home = Path.home()
    needles = {str(home), home.as_posix(), str(ROOT), ROOT.as_posix()}
    found = sorted(n for n in needles if n.encode() in data or n.lower().encode() in data)
    if found:
        sys.exit(f"{exe.name} names folders of this machine ({', '.join(found)}): rebuild it with "
                 "`python3 build.py build` (it maps the tree's paths away), then package again")

def dlls(stage_exe: Path, bindir: Path | None) -> list[Path]:
    sys.path.insert(0, str(ROOT))
    import build
    if bindir:
        build.copy_dlls(stage_exe)
    else:
        print("warning: no clang on the PATH: the DLLs are taken from build/native/ as they are")
        for d in (BUILD / "native").glob("*.dll"):
            shutil.copy2(d, stage_exe.parent / d.name)
    return sorted(stage_exe.parent.glob("*.dll"))

def pacman() -> str | None:
    for p in (shutil.which("pacman"), r"C:\msys64\usr\bin\pacman.exe"):
        if p and Path(p).exists():
            return p
    return None

def msys_path(p: str) -> Path:
    pm = pacman()
    root = Path(pm).resolve().parent.parent.parent if pm else Path(r"C:\msys64")
    return root / p.lstrip("/")

def notices(stage: Path, shipped: list[Path], bindir: Path | None) -> None:
    lic = stage / "licenses"
    lic.mkdir()
    shutil.copy2(ROOT / "decomp" / "LICENSE", lic / "decomp-LICENSE.txt")
    imgui_lic = ROOT / "third_party" / "imgui" / "LICENSE.txt"
    if imgui_lic.exists():
        shutil.copy2(imgui_lic, lic / "imgui-LICENSE.txt")
    lines = [
        "Third-party software in this package",
        "====================================",
        "",
        "revolution.exe: the PC port (MIT, LICENSE.txt), the game's decompilation",
        "(MIT, licenses/decomp-LICENSE.txt), Dear ImGui (MIT, licenses/imgui-LICENSE.txt).",
        "",
        "The DLLs are MSYS2's builds (https://www.msys2.org/) of the libraries below,",
        "unchanged. Their sources, and the scripts that built them, are at",
        "https://github.com/msys2/MINGW-packages (one folder per package, named",
        "mingw-w64-<package>) and https://repo.msys2.org/mingw/sources/.",
        "",
        "FFmpeg, as MSYS2 builds it, is under the GNU GPL version 3 or later (it",
        "includes x264 and x265): this package as a whole is distributed under its",
        "terms. The complete source code of the PC port is at",
        "https://github.com/got4n/rayman2-revolution-pc; ask there for the source of",
        "any library in this package if the links above stop working.",
        "",
        f"{'Package':34s} {'Version':28s} License",
    ]
    pm = pacman()
    if not pm or not bindir:
        lines.append("(pacman was not found: list the DLLs' licenses by hand)")
        for d in shipped:
            lines.append(f"  {d.name}")
    else:
        pkgs: dict[str, list[str]] = {}
        for d in shipped:
            r = subprocess.run([pm, "-Qqo", str(bindir / d.name)], capture_output=True, text=True)
            pkgs.setdefault(r.stdout.strip() or "?", []).append(d.name)
        for pkg in sorted(pkgs):
            if pkg == "?":
                lines.append(f"{'(unknown)':34s} {'':28s} " + ", ".join(pkgs[pkg]))
                continue
            info = subprocess.run([pm, "-Qi", pkg], capture_output=True, text=True).stdout
            field = lambda k: (re.search(rf"^{k}\s*:\s*(.*)$", info, re.M) or [None, ""])[1].strip()
            short = pkg.removeprefix("mingw-w64-clang-x86_64-")
            lines.append(f"{short:34s} {field('Version'):28s} {field('Licenses')}")
            lines.append(f"{'':34s} {field('URL')}  ({', '.join(pkgs[pkg])})")
            files = subprocess.run([pm, "-Qlq", pkg], capture_output=True, text=True).stdout.split()
            for f in files:
                if "/share/licenses/" in f and not f.endswith("/"):
                    src = msys_path(f)
                    if src.is_file():
                        shutil.copy2(src, lic / f"{short}-{src.name}")
    gpl = msys_path("/clang64/share/licenses/libiconv/COPYING")
    if gpl.is_file() and b"Version 3" in gpl.read_bytes():
        shutil.copy2(gpl, lic / "GPL-3.0.txt")
    (stage / "THIRD_PARTY_NOTICES.txt").write_text("\r\n".join(lines) + "\r\n", encoding="utf-8")

def check_no_game_data(stage: Path) -> None:
    bad = [p for p in stage.rglob("*") if GAME_DATA.search(p.relative_to(stage).as_posix())
           or p.name == "build_info.json"]
    if bad:
        sys.exit("game data or machine-specific files in the package, stopping: "
                 + ", ".join(str(p.relative_to(stage)) for p in bad))

# The program must hold none of the game's executable: it copies it from the
# player's own copy as it starts (host/complete.c). Any 64-byte block of
# SLUS_201.38's segments that is not trivial stops the packaging. A build made
# without the game (no --assets) read none of it, and has nothing to check.
def check_no_game_bytes(exe: Path) -> None:
    info = BUILD / "build_info.json"
    if not info.exists():
        print("built without the game: nothing of it to look for in the program")
        return
    game = Path(json.loads(info.read_text())["assets"]) / "SLUS_201.38"
    if not game.exists():
        sys.exit(f"{game} not found (build_info.json): the program cannot be checked for the game's bytes")
    elf = game.read_bytes()
    phoff, entry, count = (int.from_bytes(elf[0x1C:0x20], "little"), int.from_bytes(elf[0x2A:0x2C], "little"),
                           int.from_bytes(elf[0x2C:0x2E], "little"))
    blocks = set()
    for k in range(count):
        ph = elf[phoff + entry * k:phoff + entry * (k + 1)]
        if int.from_bytes(ph[0:4], "little") != 1:
            continue
        off, va, size = (int.from_bytes(ph[i:i + 4], "little") for i in (4, 8, 16))
        for a in range(-va % 64, size - 63, 64):
            b = elf[off + a:off + a + 64]
            if len(set(b)) >= 16:
                blocks.add(b)
    data = exe.read_bytes()
    found = sum(data[i:i + 64] in blocks for i in range(0, len(data) - 63, 64))
    if found:
        sys.exit(f"{exe.name} holds {found} blocks of the game's executable, stopping")

def sha256(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()

MANIFEST = "files.txt"

def manifest(stage: Path) -> None:
    lines = [f"{sha256(p)}  {p.relative_to(stage).as_posix()}"
             for p in sorted(stage.rglob("*")) if p.is_file() and p.name != MANIFEST]
    (stage / MANIFEST).write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")

def main() -> None:
    ap = argparse.ArgumentParser(description="Make the Windows release from a finished build.")
    ap.add_argument("--version", default=None,
                    help="checked against the program's own (build.py: REVO_VERSION=0.1.0, else git describe)")
    a = ap.parse_args()
    if sys.platform != "win32":
        sys.exit("tools/package.py makes the Windows package: run it on Windows after build.py build")
    exe = BUILD / "native" / "revolution.exe"
    if not exe.exists():
        sys.exit(f"{exe} not found: python3 build.py build first")
    ver = version(exe)
    if a.version and a.version.removeprefix("v") != ver:
        sys.exit(f"{exe.name} is version {ver}, not {a.version}: it would offer itself the update to its own "
                 f"release. Rebuild it: REVO_VERSION={a.version.removeprefix('v')} python3 build.py build")
    if ver.endswith("-dirty") or re.search(r"-g[0-9a-f]+$", ver):
        print(f"warning: version {ver} is not a release's: tag the commit (git tag v0.1.0) and rebuild")
    bindir = toolchain_bin()

    stage = DIST / "stage"
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)
    shutil.copy2(exe, stage / exe.name)
    strip(stage / exe.name, bindir)
    check_no_local_paths(stage / exe.name)
    check_no_game_bytes(stage / exe.name)
    shipped = dlls(stage / exe.name, bindir)
    shutil.copy2(ROOT / "README.md", stage / "README.md")
    shutil.copy2(ROOT / "LICENSE", stage / "LICENSE.txt")
    notices(stage, shipped, bindir)
    check_no_game_data(stage)
    manifest(stage)

    base = f"{NAME}-{ver}-windows-x64"
    zpath = DIST / f"{base}.zip"
    with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for p in sorted(stage.rglob("*")):
            if p.is_file():
                z.write(p, f"{base}/{p.relative_to(stage).as_posix()}")
    print(f"wrote {zpath} ({zpath.stat().st_size / 1e6:.0f} MB, {len(shipped)} DLLs)")

    (DIST / "SHA256SUMS.txt").write_text(f"{sha256(zpath)}  {zpath.name}\n")
    print(f"wrote {DIST / 'SHA256SUMS.txt'}")

if __name__ == "__main__":
    main()
