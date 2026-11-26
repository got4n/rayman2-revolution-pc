#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import os
import shutil
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "tools" / "native"))
import zone  # noqa: E402

ROOT = Path(__file__).resolve().parent
TOOLS = ROOT / "tools"
CONFIG = ROOT / "config"
DECOMP = ROOT / "decomp"
UBISOUND = ROOT / "ubisound"
HOST = ROOT / "host"
GLI = ROOT / "gli"
PLATFORM = ROOT / "platform"
IMGUI = ROOT / "third_party" / "imgui"
BUILD = ROOT / "build"
NATIVE = BUILD / "native"

def host_arch() -> str:
    if sys.platform == "darwin":
        r = subprocess.run(["sysctl", "-n", "hw.optional.arm64"], capture_output=True, text=True)
        if r.stdout.strip() == "1":
            return "arm64"
    return subprocess.run(["uname", "-m"], capture_output=True, text=True).stdout.strip()

def prefer_native_tools() -> None:
    if sys.platform == "darwin" and Path("/opt/homebrew/bin").is_dir() and host_arch() == "arm64":
        os.environ["PATH"] = "/opt/homebrew/bin:" + os.environ.get("PATH", "")
    if zone.target.LINUX:
        # One LLVM for every step, the one tools/native/target.py takes (apt's newest: clang 18 is too old).
        os.environ.setdefault("REVO_CLANG", zone.target.CLANG)
        os.environ.setdefault("CC", zone.target.CLANG)
        os.environ.setdefault("CXX", zone.target.tool("clang++"))

def find_tool(name: str, env_var: str, candidates: list[Path]) -> str:
    if os.environ.get(env_var):
        return os.environ[env_var]
    for c in candidates:
        if c.exists():
            return str(c)
    found = shutil.which(name)
    if found:
        return found
    sys.exit(f"{name} not found: install it or set {env_var}")

def run(cmd: list, quiet: bool = False, **kw) -> None:
    if not quiet:
        print("+", " ".join(str(c) for c in cmd[:6]), "..." if len(cmd) > 6 else "")
    r = subprocess.run([str(c) for c in cmd], **kw)
    if r.returncode:
        sys.exit(f"step failed ({r.returncode}): {cmd[0]}")

def sdl2_flags() -> tuple[str, str]:
    if sys.platform == "win32" and not os.environ.get("REVO_SDL2_CONFIG"):
        cflags, libs = pkg_config("sdl2")
        libs = " ".join(w for w in libs.split() if w not in ("-lmingw32", "-lSDL2main", "-mwindows"))
        return cflags + " -DSDL_MAIN_HANDLED", libs
    sdl = sdl2_config()
    return (subprocess.run([sdl, "--cflags"], capture_output=True, text=True).stdout.strip(),
            subprocess.run([sdl, "--libs"], capture_output=True, text=True).stdout.strip())

def ninja_path(p) -> str:
    return str(p).replace("$", "$$").replace(":", "$:").replace(" ", "$ ")

def sdl2_config() -> str:
    if os.environ.get("REVO_SDL2_CONFIG"):
        return os.environ["REVO_SDL2_CONFIG"]
    if sys.platform == "darwin" and host_arch() == "arm64" and Path("/opt/homebrew/bin/sdl2-config").exists():
        return "/opt/homebrew/bin/sdl2-config"
    return shutil.which("sdl2-config") or "sdl2-config"

def pkg_config(*packages: str) -> tuple[str, str]:
    tool = os.environ.get("REVO_PKG_CONFIG") or shutil.which("pkg-config") or "/opt/homebrew/bin/pkg-config"

    def ask(flag: str) -> str:
        p = subprocess.run([tool, flag, *packages], capture_output=True, text=True)
        if p.returncode:
            sys.exit(f"pkg-config: {' '.join(packages)} not found ({p.stderr.strip()})")
        return p.stdout.strip()
    return ask("--cflags"), ask("--libs")

def vulkan_tools() -> tuple:
    found = None
    dirs = [os.environ.get("REVO_VULKAN_INCLUDE"), "/opt/homebrew/opt/molten-vk/libexec/include",
            "/usr/local/opt/molten-vk/libexec/include", "/opt/homebrew/include", "/usr/local/include",
            "/usr/include"]
    if sys.platform == "win32":
        cc = shutil.which(os.environ.get("CC", "clang"))
        dirs[1:1] = [os.environ.get("VULKAN_SDK") and str(Path(os.environ["VULKAN_SDK"]) / "Include"),
                     cc and str(Path(cc).resolve().parent.parent / "include")]
    for d in dirs:
        if d and (Path(d) / "vulkan" / "vulkan.h").exists():
            found = Path(d)
            break
    glslang = os.environ.get("REVO_GLSLANG") or shutil.which("glslangValidator")
    if not glslang and Path("/opt/homebrew/bin/glslangValidator").exists():
        glslang = "/opt/homebrew/bin/glslangValidator"
    if not found or not glslang:
        sys.exit("build.py: the Vulkan headers and glslangValidator are needed (the Vulkan SDK, MSYS2's "
                 "vulkan-headers and glslang, or apt's libvulkan-dev and glslang-tools; REVO_VULKAN_INCLUDE and "
                 "REVO_GLSLANG point to them)")
    return found, glslang

SHADERS = ("native.vert", "native.frag", "rich.vert", "rich.frag", "present.vert", "present.frag", "overlay.vert",
           "overlay.frag")

def vulkan_rules(lines: list, vk_include, glslang) -> tuple[str, str]:
    out = BUILD / "gen" / "shaders"
    common = ninja_path(GLI / "shaders" / "state.glsl")
    quiet = (f"cmd /c {glslang} -V $in --vn $var -o $out > nul" if sys.platform == "win32"
             else f"{glslang} -V $in --vn $var -o $out > /dev/null")
    lines.append(f"rule spirv\n  command = {quiet}\n  description = SPIRV $in")
    headers = []
    for name in SHADERS:
        var = name.replace(".", "_")
        h = out / (var + ".h")
        lines.append(f"build {ninja_path(h)}: spirv {ninja_path(GLI / 'shaders' / name)} | "
                     f"{common}\n  var = {var}")
        headers.append(ninja_path(h))
    return f"-I{vk_include} -I{out}", " | " + " ".join(headers)

def system_libs() -> str:
    if sys.platform == "win32":
        return "-lpthread -lshell32 -lole32 -lcomdlg32 -lwinhttp -lbcrypt"
    return "-lpthread -ldl"

def port_version() -> str:
    v = os.environ.get("REVO_VERSION", "").strip()
    if not v:
        try:
            r = subprocess.run(["git", "describe", "--tags", "--always", "--dirty"], cwd=ROOT,
                               capture_output=True, text=True)
            v = r.stdout.strip() if r.returncode == 0 and r.stdout.strip() else "dev"
        except FileNotFoundError:
            v = "dev"
    v = v[1:] if re.match(r"v\d", v) else v
    if not re.fullmatch(r"[A-Za-z0-9._+-]{1,64}", v):
        sys.exit(f"REVO_VERSION {v!r}: letters, digits and . _ + - only")
    return v

def write_version(gen: Path) -> None:
    h = gen / "revo_version.h"
    text = f'#define REVO_VERSION "{port_version()}"\n'
    if not h.exists() or h.read_text() != text:
        gen.mkdir(parents=True, exist_ok=True)
        h.write_text(text)

WINDOWS_IMAGE_BASE = 0x200000000

def write_ninja(gen: Path, out: Path, game_objs: list[Path]) -> Path:
    cc = os.environ.get("CC", "clang")
    cxx = os.environ.get("CXX", "clang++")
    sdl_cflags, sdl_libs = sdl2_flags()
    av_cflags, av_libs = pkg_config("libavcodec", "libavutil")
    inc = f"-I{HOST} -I{gen}"
    extra = f"-ffile-prefix-map={ROOT.as_posix()}=. -ffile-prefix-map={ROOT}=. " + os.environ.get("REVO_EXTRA_CFLAGS", "")
    common = f"-ffp-contract=off -fno-delete-null-pointer-checks {extra} {inc}"
    if sys.platform == "win32":
        link = (f"{cxx} -Wl,--error-limit=0 @$out.rsp -o $out -Wl,--image-base={WINDOWS_IMAGE_BASE:#x} "
                f"-Wl,--disable-high-entropy-va -Wl,--disable-dynamicbase {sdl_libs} {av_libs} {system_libs()}")
    elif sys.platform.startswith("linux"):
        # Not position independent, at a 4 GB aligned address: the 4-byte pointers are offsets from there. The
        # C runtime's start files are its position independent ones: the others reach their data with 32-bit
        # addresses, which cannot be above 4 GB.
        def crt(name: str) -> str:
            return subprocess.run([cxx, f"-print-file-name={name}"], capture_output=True, text=True).stdout.strip()
        start = " ".join(crt(n) for n in ("Scrt1.o", "crti.o", "crtbeginS.o"))
        end = " ".join(crt(n) for n in ("crtendS.o", "crtn.o"))
        link = (f"{cxx} -fuse-ld=lld -no-pie -nostartfiles -Wl,--image-base={zone.target.IMAGE_BASE:#x} {start} "
                f"@$out.rsp -o $out {sdl_libs} {av_libs} {system_libs()} {end}")
    else:
        link = f"{cxx} @$out.rsp -o $out {' '.join(zone.MAC_LINK)} {sdl_libs} {av_libs} {system_libs()}"
    lines = [
        f"cflags = -O1 -g -w -fno-strict-aliasing {common} {sdl_cflags}",
        f"cxxflags = -O1 -g -w -std=c++17 {common} -I{IMGUI} -I{IMGUI / 'backends'} {sdl_cflags}",
        f"rule cc\n  command = {cc} $cflags -MMD -MF $out.d -c $in -o $out\n  depfile = $out.d\n  deps = gcc\n  description = CC $in",
        f"rule cxx\n  command = {cxx} $cxxflags -MMD -MF $out.d -c $in -o $out\n  depfile = $out.d\n  deps = gcc\n  description = CXX $in",
        f"rule link\n  command = {link}\n  rspfile = $out.rsp\n  rspfile_content = $in\n  description = LINK $out",
    ]
    vk_flags, vk_inputs = vulkan_rules(lines, *vulkan_tools())
    objs = []

    def add(rule: str, src: Path, flags: str = "") -> None:
        base = ROOT if src.is_relative_to(ROOT) else Path(src.anchor)
        o = NATIVE / "hostobj" / (src.relative_to(base).as_posix().replace("/", "_") + ".o")
        head = f"build {ninja_path(o)}: {rule} {ninja_path(src)}"
        var = "cxxflags" if rule == "cxx" else "cflags"
        if src.name == "render_vk.c":
            lines.append(f"{head}{vk_inputs}\n  cflags = $cflags {vk_flags}")
        elif flags:
            lines.append(f"{head}\n  {var} = ${var} {flags}")
        else:
            lines.append(head)
        objs.append(o)

    ui_flags = vk_flags.split(" ")[0] + " -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES"
    for part in (HOST, GLI):
        for f in sorted(part.rglob("*.c")) + sorted(part.rglob("*.cpp")):
            add("cxx" if f.suffix == ".cpp" else "cc", f,
                (ui_flags + " " if f.name == "ui.cpp" else "") + f"-I{DECOMP / 'include' / 'PC'}")
    for name in ("imgui.cpp", "imgui_draw.cpp", "imgui_tables.cpp", "imgui_widgets.cpp",
                 "backends/imgui_impl_sdl2.cpp"):
        add("cxx", IMGUI / name)
    add("cxx", IMGUI / "backends" / "imgui_impl_vulkan.cpp", ui_flags)
    for f in sorted(PLATFORM.glob("*.c")):
        add("cc", f, f"-I{DECOMP / 'include' / 'PC'} {av_cflags}")
    lines.append("build host: phony " + " ".join(ninja_path(o) for o in objs))
    # The zone first: its offsets in the program are the PS2's addresses (tools/native/zone.py).
    zs = NATIVE / "zone" / "zone.s"
    zs.parent.mkdir(parents=True, exist_ok=True)
    if not zs.exists() or zs.read_text() != zone.zone_asm():
        zs.write_text(zone.zone_asm())
    zo = NATIVE / "hostobj" / "zone.s.o"
    lines.append(f"build {ninja_path(zo)}: cc {ninja_path(zs)}")
    objs.insert(0, zo)
    glue = NATIVE / "glue"
    for src in (glue / "glue.c", glue / "glue.s"):
        o = NATIVE / "hostobj" / (src.name + ".o")
        lines.append(f"build {ninja_path(o)}: cc {ninja_path(src)}")
        objs.append(o)
    lines.append(f"build {ninja_path(out)}: link " + " ".join(ninja_path(o) for o in objs + game_objs))
    lines.append(f"default {ninja_path(out)}")
    nf = NATIVE / "build.ninja"
    nf.write_text("\n".join(lines) + "\n")
    return nf

def rename_game_objects() -> list[Path]:
    sys.path.insert(0, str(TOOLS / "native"))
    import link_plan
    import target
    link = NATIVE / "link"
    link.mkdir(parents=True, exist_ok=True)
    names = NATIVE / "glue" / "redefine.txt"
    # An object is renamed again only when it, or the names, changed since the last build.
    stamp = link / ".names"
    same_names = stamp.exists() and stamp.read_bytes() == names.read_bytes()
    objs = [o for o in sorted((NATIVE / "obj").glob("*.o")) if not o.name.startswith(link_plan.RW3)]
    out = []
    for o in objs:
        dst = link / (o.parent.name + "__" + o.name)
        if not (same_names and dst.exists() and dst.stat().st_mtime >= o.stat().st_mtime):
            run([target.tool("llvm-objcopy"), f"--redefine-syms={names}", o, dst], quiet=True)
        out.append(dst)
    for old in set(link.glob("*.o")) - set(out):
        old.unlink()
    shutil.copyfile(names, stamp)
    return out + sorted((NATIVE / "snd" / "final").glob("*.o"))

def assets_dir(a, required: bool = True) -> Path | None:
    given = a.assets or os.environ.get("REVO_ASSETS")
    if not given and (BUILD / "build_info.json").exists():
        given = json.loads((BUILD / "build_info.json").read_text()).get("assets")
    if not given and required:
        sys.exit("where are the game's files? --assets DIR (or REVO_ASSETS)")
    return Path(given).resolve() if given else None

def cmd_build(a) -> None:
    assets = assets_dir(a, required=False)
    retail = assets / "SLUS_201.38" if assets else None
    if retail and not retail.exists():
        sys.exit(f"the retail executable not found: {retail} (see README.md)")
    # The decomp's PS2 build is only needed to check the tables of functions against the game.
    ps2_build = (DECOMP / "build" / "SLUS_201.38.elf").exists() and (DECOMP / "asm" / "data").is_dir()
    NATIVE.mkdir(parents=True, exist_ok=True)
    gen = BUILD / "gen"
    py = sys.executable
    native = TOOLS / "native"

    run([py, TOOLS / "guest_build.py", "--decomp", DECOMP, "--out", BUILD / "guest",
         *(["--retail", retail] if retail else []),
         "--define", "PC_PORT", "--driver-api", GLI / "driver.h"])
    if retail and ps2_build:
        run([py, TOOLS / "check_tables.py", "--decomp", DECOMP, "--c-functions", BUILD / "guest" / "c_functions.json",
             "--checked", CONFIG / "table_checked.txt"])
    else:
        print("without the game and the decomp's PS2 build, the tables of functions are not checked (check_tables.py)")
    run([py, native / "build_native.py"])
    run([py, native / "snd_native.py"])
    exe = executable()
    write_version(gen)
    run(["ninja", "-f", write_ninja(gen, exe, []), "host"], cwd=NATIVE)
    run([py, native / "glue_native.py"])
    game = rename_game_objects()
    run(["ninja", "-f", write_ninja(gen, exe, game)], cwd=NATIVE)
    if sys.platform == "win32":
        copy_dlls(exe)
    if assets:
        (BUILD / "build_info.json").write_text(json.dumps({"assets": str(assets)}, indent=1) + "\n")
    print(f"built {exe}")

def copy_dlls(exe: Path) -> None:
    cc = shutil.which(os.environ.get("CC", "clang"))
    if not cc:
        return
    bindir = Path(cc).resolve().parent
    objdump = shutil.which("objdump", path=str(bindir)) or shutil.which("llvm-objdump", path=str(bindir))
    if not objdump:
        print("warning: no objdump next to clang, the DLLs were not copied next to the program")
        return
    todo, seen, copied = [exe], set(), 0
    while todo:
        dep = todo.pop()
        p = subprocess.run([objdump, "-p", str(dep)], capture_output=True, text=True)
        names = [line.split("DLL Name:")[1].strip() for line in p.stdout.splitlines() if "DLL Name:" in line]
        if dep.name.lower() == "sdl2.dll" and b"SDL3.dll" in dep.read_bytes():
            names.append("SDL3.dll")
        for name in names:
            if name.lower() in seen:
                continue
            seen.add(name.lower())
            src = bindir / name
            if src.exists():
                dst = exe.parent / name
                if not dst.exists() or dst.stat().st_mtime < src.stat().st_mtime:
                    shutil.copy2(src, dst)
                copied += 1
                todo.append(src)
    print(f"{copied} DLLs next to {exe.name}")

def executable() -> Path:
    return NATIVE / ("revolution.exe" if sys.platform == "win32" else "revolution")

def cmd_run(a) -> None:
    exe = executable()
    if sys.platform == "win32":
        code = subprocess.run([str(exe), str(assets_dir(a))]).returncode
        if code:
            print(f"{exe.name} stopped with code {code & 0xFFFFFFFF:#010x}"
                  + (" (a DLL is missing: run build.py build again)" if code & 0xFFFFFFFF == 0xC0000135 else ""))
        sys.exit(code)
    os.execv(str(exe), [str(exe), str(assets_dir(a))])

def main() -> None:
    prefer_native_tools()
    ap = argparse.ArgumentParser(description="Build and run the PC port.")
    sub = ap.add_subparsers(dest="cmd", required=True)
    b = sub.add_parser("build", help="build build/revolution")
    b.add_argument("--assets", help="the game's files (also REVO_ASSETS)")
    b.set_defaults(fn=cmd_build)
    r = sub.add_parser("run", help="run the game")
    r.add_argument("--assets", help="default: the folder of the last build")
    r.set_defaults(fn=cmd_run)
    a = ap.parse_args()
    a.fn(a)

if __name__ == "__main__":
    main()
