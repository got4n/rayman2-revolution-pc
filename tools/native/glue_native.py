#!/usr/bin/env python3
from __future__ import annotations

import collections
import shutil
import subprocess
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT / "tools"))
import link_plan
import zone
import rva
import target as platform
from retail import base_name

OUT = ROOT / "build" / "native" / "glue"
LL = ROOT / "build" / "native" / "ll"
DRV_INT = {"u32": "u32", "s32": "s32", "u8": "u8", "GuestPtr": "u32"}
CRT_C = {"uint32_t": "u32", "int32_t": "s32", "float": "float", "double": "double", "void": "void"}
GAME_C = {"u32": "u32", "s32": "s32", "GuestPtr": "u32", "u64": "u64", "float": "float", "void": "void"}

class Proto(tuple):
    widths: tuple[str, list[str]] = ("u64", [])

def width(t: str) -> str:
    t = t.strip()
    for ty, c in (("i1", "u8"), ("i8", "u8"), ("i16", "u16"), ("i32", "u32"), ("i64", "u64"),
                  ("ptr addrspace(271)", "u32"), ("ptr", "u64"), ("float", "float"), ("double", "double"),
                  ("void", "void")):
        if t == ty or t.startswith(ty + " "):
            return c
    return "u64"

def prototypes() -> dict[str, tuple[str, list[str], bool]]:
    def cls(t: str) -> str:
        t = t.strip()
        if t.startswith("float"):
            return "float"
        if t.startswith("double"):
            return "double"
        if t.startswith("void"):
            return "void"
        return "i"
    out: dict[str, tuple[str, list[str], bool]] = {}
    head = re.compile(r'^(?:declare|define)\b[^@]*?\s(void|float|double|i\d+|ptr(?: addrspace\(\d+\))?|\{[^}]*\}|%[\w.]+)'
                      r'(?:\s+\w+)*\s+@("[^"]+"|[\w.$]+)\((.*)\)', re.M)
    for f in sorted(LL.glob("*.fixed.ll")):
        if f.name.startswith(link_plan.RW3):
            continue
        for ret, name, params in head.findall(f.read_text(errors="replace")):
            name = name.strip('"')
            depth, cur, ps = 0, "", []
            for ch in params:
                if ch in "({[<":
                    depth += 1
                elif ch in ")}]>":
                    depth -= 1
                if ch == "," and depth == 0:
                    ps.append(cur)
                    cur = ""
                else:
                    cur += ch
            if cur.strip():
                ps.append(cur)
            variadic = bool(ps) and ps[-1].strip() == "..."
            ps = [p for p in ps if p.strip() != "..."]
            if name not in out:
                p = Proto((cls(ret), [cls(p) for p in ps], variadic))
                p.widths = (width(ret), [width(x) for x in ps])
                out[name] = p
    return out

def c_of(c: str) -> str:
    return "u64" if c == "i" else c

def sig(name: str, proto) -> str:
    ret, params, variadic = proto
    ws = getattr(proto, "widths", None) if platform.LP64 else None
    if ws and len(ws[1]) == len(params):
        ps = [f"{w if c == 'i' else c_of(c)} p{k}" for k, (c, w) in enumerate(zip(params, ws[1]))]
    else:
        ps = [f"{c_of(c)} p{k}" for k, c in enumerate(params)]
    ps += ["..."] if variadic else []
    return f"{c_of(ret)} {name}({', '.join(ps) or 'void'})"

def header_api(path: Path, pattern: str):
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(errors="replace"), flags=re.S)
    text = re.sub(r"#\s*define[^\n]*", "", text)
    out = []
    for m in re.finditer(pattern, text):
        params = [q.strip() for q in m.group("params").split(",") if q.strip() and q.strip() != "void"]
        out.append((m, [re.sub(r"\s*\b\w+\s*$", "", q) if len(q.split()) > 1 else q for q in params]))
    return out

def driver_api() -> dict[str, tuple[str, list[str]]]:
    return {m.group("name"): (m.group("ret"), types) for m, types in header_api(
        ROOT / "gli" / "driver.h",
        r"PC_DRIVER\(\s*0x[0-9A-Fa-f]+\s*\)\s*(?P<ret>\w+)\s+(?P<name>\w+)\s*\((?P<params>[^)]*)\)\s*;")}

def crt_api() -> dict[str, tuple[str, list[str]]]:
    return {m.group("name"): (m.group("ret"), types) for m, types in header_api(
        ROOT / "host" / "crt.h", r"CRT_API\s+(?P<ret>\w+)\s+crt_(?P<name>\w+)\s*\((?P<params>[^)]*)\)\s*;")}

def game_api() -> list[tuple[str, str, list[str]]]:
    return [(m.group("name"), m.group("ret"), types) for m, types in header_api(
        ROOT / "host" / "game_api.h", r"GAME_API\s+(?P<ret>\w+)\s+(?P<name>\w+)\s*\((?P<params>[^)]*)\)\s*;")]

def driver_adapter(sym: str, proto, drv_name: str, ret: str, types: list[str]) -> list[str]:
    g_ret, params, _ = proto
    ints = [f"p{k}" for k, c in enumerate(params) if c == "i"]
    floats = [f"p{k}" for k, c in enumerate(params) if c in ("float", "double")]
    args, ki, kf = [], 0, 0
    for t in types:
        if t == "float":
            args.append(floats[kf] if kf < len(floats) else "0.0f")
            kf += 1
        else:
            args.append(f"({DRV_INT[t]}){ints[ki]}" if ki < len(ints) else "0")
            ki += 1
    if ki > len(ints) or kf > len(floats):
        sys.exit(f"glue_native: {sym}: {drv_name} takes more arguments than the game passes")
    call = f"{drv_name}({', '.join(args)})"
    if g_ret == "void":
        body = f"{call};"
    elif ret == "void":
        body = f"{call}; return 0;"
    elif (ret == "float") != (g_ret in ("float", "double")):
        sys.exit(f"glue_native: {sym}: {drv_name} returns {ret}, the game's call takes {g_ret}")
    else:
        body = f"return ({c_of(g_ret)}){call};"
    return [f"{sig(sym, proto)} {{ {body} }}"]

def directly_called() -> set[str]:
    out: set[str] = set()
    pat = re.compile(r'\bcall\b[^@\n]*@("[^"]+"|[\w.$]+)\(')
    for f in sorted(LL.glob("*.fixed.ll")):
        if not f.name.startswith(link_plan.RW3):
            out.update(n.strip('"') for n in pat.findall(f.read_text(errors="replace")))
    return out

def kinds(classes: list[str]) -> list[str]:
    return ["i" if c == "i" else "f" for c in classes]

def reorder_adapter(sym: str, view, target: str, proto) -> list[str]:
    ints = [f"p{k}" for k, c in enumerate(view[1]) if c == "i"]
    floats = [f"p{k}" for k, c in enumerate(view[1]) if c != "i"]
    args, ki, kf = [], 0, 0
    for c in proto[1]:
        if c == "i":
            args.append(ints[ki] if ki < len(ints) else "0")
            ki += 1
        else:
            args.append(floats[kf] if kf < len(floats) else "0")
            kf += 1
    call = f"{target}({', '.join(args)})"
    if view[0] == "void":
        body = f"{call};"
    elif (view[0] == "i") != (proto[0] == "i") or proto[0] == "void":
        sys.exit(f"glue_native: {sym} returns {view[0]}, {target} {proto[0]}")
    else:
        body = f"return {call};"
    return [sig(target, proto) + ";", f"{sig(sym, view)} {{ {body} }}"]

def crt_adapter(sym: str, name: str, ret: str, types: list[str]) -> list[str]:
    ps = [f"{'u64' if CRT_C[t] in ('u32', 's32') else CRT_C[t]} p{k}" for k, t in enumerate(types)]
    args = ", ".join(f"({CRT_C[t]})p{k}" for k, t in enumerate(types))
    rc = {"u32": "u64", "s32": "s64"}.get(CRT_C[ret], CRT_C[ret])
    call = f"crt_{name}({args})"
    ints = all(CRT_C[t] in ("u32", "s32") for t in types) and CRT_C[ret] in ("u32", "s32", "void")
    if not ints:
        body = f"{call};" if rc == "void" else f"return ({rc}){call};"
        return [f"{rc} {sym}({', '.join(ps) or 'void'}) {{ {body} }}"]
    a = [f"(u32)p{k}" for k in range(len(types))] + ["0"] * (3 - len(types))
    if rc == "void":
        body = f"{call}; if (revo_crt_trace) crt_trace(\"{name}\", {len(types)}, {', '.join(a[:3])}, 0);"
    else:
        body = (f"{rc} r = ({rc}){call}; if (revo_crt_trace) crt_trace(\"{name}\", {len(types)}, {', '.join(a[:3])}, "
                f"(u32)r); return r;")
    return [f"{rc} {sym}({', '.join(ps) or 'void'}) {{ {body} }}"]

def game_object(name: str, rename: dict[str, str]) -> str:
    global _defined
    if "_defined" not in globals():
        objs = [o for o in sorted((ROOT / "build" / "native" / "obj").glob("*.o")) if not o.name.startswith(link_plan.RW3)]
        _defined = link_plan.undefined.nm(objs)[0]
    if name in _defined:
        return f'"{rename.get(name, name)}"'
    s = zone.look(name)
    if s is None:
        sys.exit(f"glue_native: {name} (host/game_vars.h): no address")
    if s[1]:
        sys.exit(f"glue_native: {name} (host/game_vars.h) is a function the native program does not have")
    return f"{s[0]:#x}"

def object_sizes(objs: list[Path], names: set[str]) -> dict[str, int]:
    """The size of each object the C writes, as compiled."""
    out = subprocess.run([platform.tool("llvm-nm"), "--defined-only", "-S", *map(str, objs)], capture_output=True,
                         text=True).stdout
    sizes = {}
    for line in out.splitlines():
        f = line.split()
        if len(f) == 4 and platform.c_name(f[3]) in names:
            sizes[platform.c_name(f[3])] = int(f[1], 16)
    lost = names - set(sizes)
    if lost:
        sys.exit(f"glue_native: no size for {', '.join(sorted(lost))}")
    return sizes

def code_table(rename: dict[str, str], provided: set[str]) -> list[str]:
    """revo_code's table (host/complete.c): the native function at each PS2 address of a function."""
    objs = [o for o in sorted((ROOT / "build" / "native" / "obj").glob("*.o")) if not o.name.startswith(link_plan.RW3)]
    game = {rename.get(n, n) for n in link_plan.undefined.nm(objs)[0]}
    host = [o for o in sorted((ROOT / "build" / "native" / "hostobj").glob("*.o")) if not o.name.startswith("glue.")]
    there = game | provided | (link_plan.undefined.nm(host)[0] if host else set())
    at: dict[int, str] = {}
    known = dict(zone.symbols())
    for name in there:
        if name not in known and zone.ENCODED.fullmatch(name) and (s := zone.look(name)):
            known[name] = s
    for name, (a, code) in sorted(known.items()):
        if not code or not zone.in_zone(a) or a in at:
            continue
        final = rename.get(name, name)
        if final in there and re.fullmatch(r"[A-Za-z_.$][\w.$]*", final):
            at[a] = final
    s = ["# revo_code's table: a PS2 address, the native function there", ".globl revo_code_count", ".p2align 2",
         "revo_code_count:", f"    .long {len(at)}", ".globl revo_code_table", "revo_code_table:"]
    for a in sorted(at):
        s += [f"    .long {a:#x}", f'    .long "{at[a]}"']
    inits = zone.load_all()["inits"]
    sizes = object_sizes(objs, {i for i, _ in inits})
    s += ["# the game's data the C writes: its address, the C's value, its size", ".globl revo_c_data_count",
          "revo_c_data_count:", f"    .long {len(inits)}", ".globl revo_c_data", "revo_c_data:"]
    for init, a in inits:
        s += [f"    .long {a:#x}", f'    .long "{init}"', f"    .long {sizes[init]:#x}"]
    print(f"glue: {len(at)} functions by PS2 address, {len(inits)} objects of the game's data from the C")
    return s

LIBS = ["libmingwex.a", "libmingw32.a", "libucrt.a", "libmsvcrt.a", "libkernel32.a", "libpthread.a",
        "libc++.a", "libunwind.a"]

def clashes() -> set[str]:
    objs = [o for o in sorted((ROOT / "build" / "native" / "obj").glob("*.o")) if not o.name.startswith(link_plan.RW3)]
    game = link_plan.undefined.nm(objs)[0]
    host = [o for o in sorted((ROOT / "build" / "native" / "hostobj").glob("*.o")) if not o.name.startswith("glue.")]
    others = link_plan.undefined.nm(host)[1] if host else set()
    lib = Path(shutil.which("clang") or "clang").resolve().parent.parent / "lib"
    archives = [] if not platform.WINDOWS else ([lib / a for a in LIBS if (lib / a).exists()]
                                      + sorted(lib.glob("clang/*/lib/windows/libclang_rt.builtins-x86_64.a")))
    if archives:
        others |= link_plan.undefined.nm(archives)[0]
    return game & (others | LINKER_NAMES)

LINKER_NAMES = {"__CTOR_LIST__", "__DTOR_LIST__", "__ImageBase", "_end", "end", "_etext", "etext", "_edata",
                "edata", "__bss_start__", "__bss_end__", "__data_start__", "__data_end__",
                "__RUNTIME_PSEUDO_RELOC_LIST__", "__RUNTIME_PSEUDO_RELOC_LIST_END__", "__tls_start", "__tls_end",
                "_tls_used", "__dll__", "__major_os_version__", "__image_base__"}
if platform.MAC:
    LINKER_NAMES = {"_mh_execute_header", "__dso_handle", "_tlv_bootstrap", "environ", "main"}
if platform.LINUX:
    LINKER_NAMES = {"__executable_start", "__ehdr_start", "__dso_handle", "_end", "end", "_etext", "etext",
                    "__etext", "_edata", "edata", "__bss_start", "__data_start", "data_start", "_init", "_fini",
                    "_start", "_IO_stdin_used", "_DYNAMIC", "_GLOBAL_OFFSET_TABLE_", "__init_array_start",
                    "__init_array_end", "__fini_array_start", "__fini_array_end", "environ", "__environ", "main"}

def main() -> int:
    plan = link_plan.plan()
    protos = prototypes()
    drv = driver_api()
    crt = crt_api()
    gapi = game_api()
    OUT.mkdir(parents=True, exist_ok=True)
    rename: dict[str, str] = {}
    for s, b in plan["C runtime (crt.h)"]:
        if s == b:
            rename[s] = f"revo_game_{s}"
    for name, _, _ in gapi:
        rename[name] = f"revo_game_{name}"
    for name in clashes():
        rename.setdefault(name, f"revo_game_{name}")
    c = ['/* Generated by tools/native/glue_native.py. Do not edit. */',
         '#include <stdarg.h>', '#include <stdint.h>', '#include <stdio.h>', '#include <stdlib.h>',
         '#include "rt.h"', '#include "crt.h"',
         'void revo_fatal(const char *fmt, ...) __attribute__((noreturn));',
         'void revo_shadow_stack_ensure(void);',
         'typedef u32 GuestPtr;', '']
    c.append("/* gli/driver.h */")
    for name, (ret, types) in sorted(drv.items()):
        rc = "void" if ret == "void" else ("float" if ret == "float" else DRV_INT[ret])
        c.append(f"{rc} {name}({', '.join('float' if t == 'float' else DRV_INT[t] for t in types) or 'void'});")
    c.append("")
    c.append("/* the driver functions the game calls */")
    for s, d in plan["driver (gli/driver.h)"]:
        proto = protos.get(s) or protos.get(base_name(s))
        if proto is None:
            sys.exit(f"glue_native: {s}: no prototype in the game's objects")
        c += driver_adapter(s, proto, d, *drv[d])
    c.append("")
    c.append("/* the C runtime the game calls (host/crt.h, on game addresses) */")
    c.append("#ifdef _WIN32\n#include <windows.h>\n#else\n#define IsBadReadPtr(p, n) 0\n#endif")
    c.append(CRT_TRACE)
    s32_sprintf = "s32 crt_vsprintf_native(u32 dst, u32 fmt, va_list *va);"
    c.append(s32_sprintf)
    for s, b in plan["C runtime (crt.h)"]:
        sym = rename.get(s, s)
        if b == "sprintf":
            c.append(f"s64 {sym}(u64 dst, u64 fmt, ...) {{ va_list va; va_start(va, fmt); "
                     f"s32 n = crt_vsprintf_native((u32)dst, (u32)fmt, &va); va_end(va); "
                     f"if (revo_crt_trace) crt_trace(\"sprintf\", 2, (u32)dst, (u32)fmt, 0, (u32)n); return n; }}")
            continue
        c += crt_adapter(sym, b, *crt[b])
    c.append("")
    c.append("/* the game's functions the host calls (host/game_api.h) */")
    for name, ret, types in gapi:
        proto = protos.get(name)
        if proto is None:
            sys.exit(f"glue_native: {name} (host/game_api.h) is not a function of the game's C")
        g_ret, params, _ = proto
        if len(params) != len(types):
            sys.exit(f"glue_native: {name}: host/game_api.h takes {len(types)} argument(s), the game's C {len(params)}")
        c.append(sig(f"revo_game_{name}", proto) + ";")
        args = ", ".join(f"p{k}" for k in range(len(types)))
        hs = ", ".join(f"{GAME_C[t]} p{k}" for k, t in enumerate(types)) or "void"
        rc = GAME_C[ret]
        call = f"revo_game_{name}({args})"
        c.append(f"{rc} {name}({hs}) {{ revo_shadow_stack_ensure(); {call if rc == 'void' else f'return ({rc}){call}'}; }}")
    jumps = []
    c.append("/* calls whose integers and floats come in another order than the function's */")
    called = directly_called()
    for sym, target in plan["decomp C under another name"] + plan["view of a defined name"]:
        v, t = protos.get(sym), protos.get(target)
        if sym in called and (v and t and kinds(v[1]) != kinds(t[1])[:len(v[1])] or (v and t and len(v[1]) > len(t[1]))):
            c += reorder_adapter(sym, v, rename.get(target, target), t)
        else:
            jumps.append((sym, target))
    for sym, target in jumps:
        rename[sym] = rename.get(target, target)
    c.append("/* functions only the data points at (never called), and names nothing serves */")
    c.append("void revo_native_unresolved(const char *name);")
    stubs = [sym for sym, _ in plan["only the data points at it"]]
    numbers = [sym for sym, _ in plan["nothing served"] if re.fullmatch(r"0x[0-9A-Fa-f]+", sym)]
    stubs += [sym for sym, _ in plan["nothing served"] if sym not in numbers]
    for k, sym in enumerate(stubs):
        if not re.fullmatch(r"[A-Za-z_]\w*", sym):
            raise SystemExit(f"glue: {sym} is not a C name")
        c.append(f'void revo_unresolved_{k}(void) __asm__("{platform.sym_name(sym)}");')
        c.append(f'void revo_unresolved_{k}(void) {{ revo_native_unresolved("{sym}"); }}')
    c.append(NATIVE_RT)
    (OUT / "glue.c").write_text("\n".join(c) + "\n", encoding="utf-8")

    s = ["# Generated by tools/native/glue_native.py. Do not edit.", ".data"]
    for sym in numbers:
        s += [f'.globl "{sym}"', f'.set "{sym}", {sym}']
    s.append("# host/game_vars.h: the game's objects the host names")
    for v in re.findall(r"^\s*X\((\w+)\)", (ROOT / "host" / "game_vars.h").read_text(), re.M):
        s += [f".globl game_var_{v}", ".p2align 2", f"game_var_{v}:", f"    .long {game_object(v, rename)}"]
    provided = ({s for s, _ in plan["driver (gli/driver.h)"]} | {rename.get(s, s) for s, _ in plan["C runtime (crt.h)"]}
                | {sym for sym, _ in plan["decomp C under another name"] + plan["view of a defined name"]} | set(stubs))
    s += code_table(rename, provided)
    s.append("# runtime.c's table of the game's functions by address: empty for now (names come from the PE)")
    s += [".globl revo_entry_count", ".p2align 2", "revo_entry_count:", "    .long 0",
          ".globl revo_entry_addrs", "revo_entry_addrs:", ".globl revo_entry_kinds", "revo_entry_kinds:",
          ".globl revo_entry_names", ".p2align 3", "revo_entry_names:", "    .quad 0"]
    (OUT / "glue.s").write_text(rva.rva(platform.mangle_asm("\n".join(s) + "\n")), encoding="utf-8")
    (OUT / "redefine.txt").write_text("".join(f"{platform.sym_name(a)} {platform.sym_name(b)}\n"
                                              for a, b in sorted(rename.items())), encoding="utf-8")
    counts = {k: len(v) for k, v in plan.items()}
    print(f"glue: {counts}; {len(rename)} names renamed in the game's objects")
    return 0

CRT_TRACE = r"""
int revo_crt_trace = -1;
static void crt_show(FILE *o, u32 v) {
    const unsigned char *s = RAM + v;
    if (v >= REVO_GAME_LO && v < REVO_GAME_HI && !IsBadReadPtr(s, 1) && s[0] >= 32 && s[0] < 127) {
        int k = 0;
        while (k < 48 && !IsBadReadPtr(s + k, 1) && s[k] >= 32 && s[k] < 127) k++;
        if (!IsBadReadPtr(s + k, 1) && s[k] == 0) { fprintf(o, " %08X \"%.*s\"", v, k, s); return; }
    }
    fprintf(o, " %08X", v);
}
static void crt_trace(const char *name, int n, u32 a, u32 b, u32 c, u32 r) {
    fprintf(stderr, "[crt] %s", name);
    if (n > 0) crt_show(stderr, a);
    if (n > 1) crt_show(stderr, b);
    if (n > 2) crt_show(stderr, c);
    fprintf(stderr, " ->");
    crt_show(stderr, r);
    fprintf(stderr, "\n");
}
__attribute__((constructor)) static void crt_trace_init(void) { revo_crt_trace = getenv("REVO_CRT_TRACE") != NULL; }
"""

NATIVE_RT = r"""
/* the game's functions through the pointers it holds: a pointer is the function's offset in the game's
   block (host/native_mem.c). A pointer argument is passed as its offset: the game's functions make their
   8-byte pointer parameters real addresses themselves (tools/native/base32.py). */
void *revo_code(void *p);
#define GAME_FN(T, fn) ((T)revo_code(RAM + (fn)))
void game_fn_void(u32 fn) { revo_shadow_stack_ensure(); GAME_FN(void (*)(void), fn)(); }
void game_fn_void_u32(u32 fn, u32 a) { revo_shadow_stack_ensure(); GAME_FN(void (*)(u64), fn)(a); }
s32 game_fn_main(u32 fn, s32 argc, u32 argv) { revo_shadow_stack_ensure(); return (s32)GAME_FN(u64 (*)(u64, u64), fn)((u64)(s64)argc, argv); }
u32 game_fn_u32(u32 fn) { revo_shadow_stack_ensure(); return (u32)GAME_FN(u64 (*)(void), fn)(); }
s32 game_fn_compare(u32 fn, u32 a, u32 b) { revo_shadow_stack_ensure(); return (s32)GAME_FN(u64 (*)(u64, u64), fn)(a, b); }

/* a stack in the game's block for what the host hands the game (the program's own memory is) */
static _Alignas(16) u8 revo_game_stack[1u << 20];
static u32 revo_game_sp = 0;
u32 game_stack_push(u32 size) {
    if (!revo_game_sp) revo_game_sp = (u32)(uintptr_t)(revo_game_stack + sizeof revo_game_stack);
    revo_game_sp -= (size + 15u) & ~15u;
    if (revo_game_sp < (u32)(uintptr_t)revo_game_stack) revo_fatal("game_stack_push: out of stack");
    return revo_game_sp;
}
void game_stack_pop(u32 size) { revo_game_sp += (size + 15u) & ~15u; }

/* The EE's division rules (tools/native/irfix.py item 5): the
   FPU makes no infinity or NaN, x / 0 (0 / 0 too) is +-FLT_MAX with the operands' xor as sign; an
   integer division by zero gives the R5900's result, INT_MIN / -1 gives INT_MIN. */
float revo_fdiv(float a, float b) {
    if (__builtin_expect(b == 0.0f, 0)) {
        u32 x, y, r;
        memcpy(&x, &a, 4);
        memcpy(&y, &b, 4);
        r = ((x ^ y) & 0x80000000u) | 0x7F7FFFFFu;
        memcpy(&a, &r, 4);
        return a;
    }
    return a / b;
}
void revo_div0(const char *where);
#define REVO_SDIV(T, MIN)                                                                              \
    T revo_sdiv_##T(T x, T y) {                                                                        \
        if (__builtin_expect(y == 0, 0)) { revo_div0("native"); return x < 0 ? 1 : -1; }              \
        if (__builtin_expect(x == MIN && y == -1, 0)) return MIN;                                      \
        return x / y;                                                                                  \
    }                                                                                                  \
    T revo_srem_##T(T x, T y) {                                                                        \
        if (__builtin_expect(y == 0, 0)) { revo_div0("native"); return x; }                           \
        if (__builtin_expect(x == MIN && y == -1, 0)) return 0;                                        \
        return x % y;                                                                                  \
    }
#define REVO_UDIV(T, U)                                                                                \
    T revo_udiv_##T(T x, T y) {                                                                        \
        if (__builtin_expect(y == 0, 0)) { revo_div0("native"); return (T)-1; }                       \
        return (T)((U)x / (U)y);                                                                       \
    }                                                                                                  \
    T revo_urem_##T(T x, T y) {                                                                        \
        if (__builtin_expect(y == 0, 0)) { revo_div0("native"); return x; }                           \
        return (T)((U)x % (U)y);                                                                       \
    }
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
REVO_SDIV(i16, INT16_MIN)
REVO_SDIV(i32, INT32_MIN)
REVO_SDIV(i64, INT64_MIN)
REVO_UDIV(i16, uint16_t)
REVO_UDIV(i32, uint32_t)
REVO_UDIV(i64, uint64_t)

/* REVO_NANTRACE builds (tools/native/base32.py): a float the game stores, checked */
void revo_nan_store(const char *where, u64 addr);
void revo_nan_check(float v, const char *where, u64 addr) {
    if (__builtin_expect(v != v, 0)) revo_nan_store(where, addr);
}
/* ... and a float the game reads (with REVO_LOCAL_FILL=255: a local read before it is written) */
void revo_nan_read(float v, const char *where, u64 addr) {
    static const char *seen[64];
    static unsigned n;
    if (__builtin_expect(v == v, 1)) return;
    for (unsigned i = 0; i < n; i++)
        if (seen[i] == where) return;
    if (n >= 64) return;
    seen[n++] = where;
    fprintf(stderr, "[nan] %s reads a NaN at %08X\n", where, (u32)addr);
}

void revo_native_unresolved(const char *name) {
    revo_fatal("the game called %s, which nothing serves in the PC port", name);
}
"""

if __name__ == "__main__":
    raise SystemExit(main())
