#!/usr/bin/env python3
from __future__ import annotations

import argparse
import concurrent.futures as cf
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hostify
from build_cache import Cache, compiler, digest
from hostify import match_paren

CLANG = os.environ.get("REVO_CLANG", "clang")
TARGET = ["--target=mipsel-unknown-linux-gnu", "-fforce-enable-int128", "-fno-math-errno"]
CFLAGS = ["-O1", "-g0", "-fno-strict-aliasing", "-ffreestanding", "-fno-builtin",
          "-fno-common", "-fdata-sections", "-ffunction-sections", "-w",
          "-Wno-error=implicit-function-declaration", "-Wno-error=int-conversion",
          "-Wno-error=incompatible-pointer-types", "-Wno-error=return-type",
          "-Wno-error=implicit-int", "-fno-stack-protector"]

DRIVER_PROVIDED: list[str] = []
DEFINES: list[str] = []

CAST_CALL = re.compile(r"\(\s*\(\s*([A-Za-z_][\w\s\*]*?)\s*\(\s*\*\s*\)\s*\(([^()]*)\)\s*\)\s*"
                       r"&?\s*([A-Za-z_]\w*)\s*\)\s*\(")

def rewrite_casted_calls(text: str) -> str:
    base = text.find('# 1 "<hostified>"')
    base = text.index("\n", base) + 1 if base >= 0 else 0
    if not CAST_CALL.search(text, base):
        return text

    def is_function(name: str) -> bool:
        return re.search(r"[\w\*\)]\s*\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*[;{]", text) is not None

    def repl(m: re.Match) -> str:
        ret, args, name = m.group(1).strip(), m.group(2).strip(), m.group(3)
        if not is_function(name):
            return m.group(0)
        label = re.search(r"\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*(?:__asm__|__asm|asm)\s*\(\s*\"([\w.$]+)\"", text)
        target = label.group(1) if label else name
        tag = hashlib.sha1(f"{ret}|{args}".encode()).hexdigest()[:6]
        view = f"__revo_cast{tag}__{target}"
        return f"(({{ extern {ret} {view}({args or 'void'}); {view}; }}))("

    return text[:base] + CAST_CALL.sub(repl, text[base:])

EQU = re.compile(r"\.equ\s+(\w+)\s*,\s*(\w+)\s*(?:\+\s*(0x[0-9A-Fa-f]+|\d+))?")

def equ_aliases(text: str) -> tuple[str, list]:
    pieces = []
    renames = {}
    for m in EQU.finditer(text):
        name, body, off = m.group(1), m.group(2), m.group(3)
        if not re.search(r"\bstatic\b[^;{}()]*\b" + re.escape(body) + r"\s*\(", text):
            continue
        if off is None or int(off, 0) == 0:
            renames[body] = name
        else:
            pieces.append({"name": name, "body": body, "offset": int(off, 0)})
    for body, name in renames.items():
        text = re.sub(r"\bstatic(\s[^;{}()]*\b)" + re.escape(body) + r"(\s*\()", r"\1" + body + r"\2", text)
        text = re.sub(r"\b" + re.escape(body) + r"\b", name, text)
    for pc in pieces:
        pc["body"] = renames.get(pc["body"], pc["body"])
    return text, [pc for pc in pieces if pc["body"] in renames.values()]

def compile_tu(decomp: Path, src: Path, out_dir: Path, defines: list[str] | None = None) -> dict:
    rel = src.relative_to(decomp)
    stem = rel.with_suffix("").as_posix().replace("/", "__")
    lang = (["-x", "c++", "-fno-exceptions", "-fno-rtti", "-fno-strict-return"] if src.suffix == ".cpp"
            else ["-x", "c"])
    pre = out_dir / "pre" / (stem + ".i")
    host = out_dir / "host" / (stem + ".i")
    ir = out_dir / "ir" / (stem + ".ll")
    for p in (pre, host, ir):
        p.parent.mkdir(parents=True, exist_ok=True)
    res = {"tu": rel.as_posix(), "ir": str(ir), "ok": False}
    # No system headers: clang's own <stdint.h> on every host, not glibc's (Linux) or the macOS SDK's
    # (Homebrew's LLVM adds it for every target).
    cmd = [CLANG, *TARGET, *lang, "-nostdlibinc", "-ffreestanding", "-E", "-P", "-DM2CTX", *DEFINES, *(defines or []),
           f"-I{decomp}/include",
           f"-I{decomp}/src", str(src), "-o", str(pre)]
    p = subprocess.run(cmd, capture_output=True, text=True, cwd=decomp)
    if p.returncode:
        res["stage"] = "preprocess"
        res["error"] = first_error(p.stderr)
        return res
    text = pre.read_text(encoding="utf-8", errors="surrogateescape")
    # The rest is kept by the preprocessed text: a unit whose text, defines and tools are the same is not compiled
    # again. A unit has one entry for each set of defines (signature views give a second pass).
    cache = Cache(out_dir / "cache")
    entry = f"{stem}.{digest(*(defines or []))[:8]}"
    key = digest(text, *DEFINES, *(defines or []), *DRIVER_PROVIDED, tools_key())
    kept = {"host.i": host, "ll": ir}
    hit = cache.get(entry, key, kept=kept)
    if hit is not None:
        return hit
    res = compile_after_preprocess(text, defines, host, ir, decomp, src, lang, res)
    cache.put(entry, key, res, kept=kept)
    return res

def tools_key() -> str:
    """What compiles the units: this file, hostify and the headers it puts in each unit, and the compiler."""
    global _tools_key
    if _tools_key is None:
        here = Path(__file__).resolve().parent
        headers = re.findall(r'_inline_header\("([^"]+)"\)', (here / "hostify.py").read_text())
        _tools_key = digest(Path(__file__).resolve(), here / "hostify.py", *(hostify.HOST_DIR / h for h in headers),
                            compiler(CLANG))
    return _tools_key

_tools_key: str | None = None

def compile_after_preprocess(text: str, defines, host: Path, ir: Path, decomp: Path, src: Path, lang: list,
                             res: dict) -> dict:
    text, pieces = equ_aliases(text)
    if pieces:
        res["equ_pieces"] = pieces
    new, problems = hostify.hostify_text(text)
    new = rewrite_casted_calls(new)
    for d in defines or []:
        old, _, repl = d[2:].partition("=")
        new = re.sub(r'(__asm__|__asm|asm)(\s*)\(\s*"' + re.escape(old) + r'"\s*\)',
                     lambda mm: f'{mm.group(1)}{mm.group(2)}("{repl}")', new)
    host.write_text(new, encoding="utf-8", errors="surrogateescape")
    if problems:
        res["stage"] = "hostify"
        res["error"] = problems[0]
        res["asm_problems"] = problems
        return res
    cmd = [CLANG, *TARGET, *CFLAGS, "-S", "-emit-llvm", *lang, str(host), "-o", str(ir)]
    views = []
    dropped = []
    for name in DRIVER_PROVIDED:
        drop_named(host, name, dropped, "served by the native driver (gli/driver.h)")
    for _ in range(20):
        t = host.read_text(encoding="utf-8", errors="surrogateescape")
        k = t.find("__builtin_next_arg", _hostified_offset(t))
        if k < 0:
            break
        line = t.count("\n", _hostified_offset(t), k) + 1
        col = k - t.rfind("\n", 0, k)
        if not drop_function(host, f"<hostified>:{line}:{col}: error: EE varargs (__builtin_next_arg)", dropped):
            break
    for _ in range(80):
        p = subprocess.run(cmd, capture_output=True, text=True, cwd=decomp)
        if not p.returncode:
            break
        fixed = fix_view(host, p.stderr, views) or drop_function(host, p.stderr, dropped)
        if not fixed:
            break
    res["abi_views"] = views
    res["fallback_functions"] = dropped
    if p.returncode:
        res["stage"] = "compile"
        res["error"] = first_error(p.stderr)
        res["errors"] = [l for l in p.stderr.splitlines() if "error:" in l][:20]
        return res
    res["ok"] = True
    res["sha1"] = hashlib.sha1(src.read_bytes()).hexdigest()
    return res

DIAG = re.compile(r"<hostified>:(\d+):(\d+): (error|note): (.*)")

def _hostified_offset(text: str) -> int:
    k = text.index('# 1 "<hostified>"')
    return text.index("\n", k) + 1

def _pos(text: str, base: int, line: int, col: int) -> int:
    p = base
    for _ in range(line - 1):
        p = text.index("\n", p) + 1
    return p + col - 1

def _scope_end(text: str, pos: int) -> int:
    stack = []
    i, n = 0, len(text)
    while i < pos:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while text[j] != c:
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if c == "{":
            stack.append(i)
        elif c == "}":
            stack.pop()
        i += 1
    if not stack:
        return n
    depth = len(stack)
    while i < n:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while text[j] != c:
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth < len(stack):
                return i
        i += 1
    return n

def fix_view(host: Path, stderr: str, views: list) -> bool:
    text = host.read_text(encoding="utf-8", errors="surrogateescape")
    base = _hostified_offset(text)
    lines = stderr.splitlines()
    for k, l in enumerate(lines):
        m = DIAG.search(l)
        if not m or m.group(3) != "error":
            continue
        msg = m.group(4)
        nm = re.match(r"(?:redeclaration of|conflicting types for) '([\w$]+)'", msg)
        if not nm:
            continue
        name = nm.group(1)
        line, col = int(m.group(1)), int(m.group(2))
        pos = _pos(text, base, line, col)
        if not text.startswith(name, pos):
            idx = text.find(name, pos - 200 if pos > 200 else 0)
            if idx < 0:
                return False
            pos = idx
        after = text[pos + len(name):pos + len(name) + 400].lstrip()
        is_func = after.startswith("(")
        if is_func:
            close = match_close(text, text.index("(", pos + len(name)))
            if text[close:close + 50].lstrip().startswith("{"):
                prev = None
                for l2 in lines[k + 1:k + 4]:
                    m2 = DIAG.search(l2)
                    if m2 and m2.group(3) == "note":
                        prev = _pos(text, base, int(m2.group(1)), int(m2.group(2)))
                        break
                if prev is None:
                    return False
                return _rename(text, host, name, prev, pos, True, views)
        end = _scope_end(text, pos)
        return _rename(text, host, name, pos, end, is_func, views)
    return False

def drop_named(host: Path, name: str, dropped: list,
               why: str) -> bool:
    text = host.read_text(encoding="utf-8", errors="surrogateescape")
    base = _hostified_offset(text)
    for m in re.finditer(r"(?<![\w$])" + re.escape(name) + r"\s*\(", text[base:]):
        k = base + m.end() - 1
        e = match_close(text, k)
        j = e
        while j < len(text) and text[j] in " \t\n":
            j += 1
        if j >= len(text) or text[j] != "{":
            continue
        pos = j + 1
        line = text.count("\n", base, pos) + 1
        col = pos - text.rfind("\n", 0, pos)
        return drop_function(host, f"<hostified>:{line}:{col}: error: {why}", dropped)
    return False

def drop_function(host: Path, stderr: str, dropped: list) -> bool:
    text = host.read_text(encoding="utf-8", errors="surrogateescape")
    base = _hostified_offset(text)
    for l in stderr.splitlines():
        m = DIAG.search(l)
        if not m or m.group(3) != "error":
            continue
        pos = _pos(text, base, int(m.group(1)), int(m.group(2)))
        depth, i, start = 0, base, None
        while i < pos:
            c = text[i]
            if c in "\"'":
                j = i + 1
                while text[j] != c:
                    j += 2 if text[j] == "\\" else 1
                i = j + 1
                continue
            if c == "{":
                if depth == 0:
                    start = i
                depth += 1
            elif c == "}":
                depth -= 1
            i += 1
        if depth == 0 or start is None:
            return False
        head = text[:start].rstrip()
        if not head.endswith(")"):
            return False
        d, k = 0, len(head) - 1
        while k >= 0:
            if head[k] == ")":
                d += 1
            elif head[k] == "(":
                d -= 1
                if d == 0:
                    break
            k -= 1
        nm = re.search(r"([A-Za-z_]\w*)\s*$", head[:k])
        if not nm:
            return False
        name = nm.group(1)
        end = _scope_end(text, start + 1) + 1
        line_start = head.rfind("\n", 0, nm.start()) + 1
        decl = head[line_start:]
        decl = re.sub(r"\b(static|inline|__inline__|__inline)\b", "", decl)
        new = text[:line_start] + decl + ";" + "\n" * text[start:end].count("\n") + text[end:]
        host.write_text(new, encoding="utf-8", errors="surrogateescape")
        dropped.append({"name": name, "reason": m.group(4)[:200]})
        return True
    return False

def match_close(text: str, i: int) -> int:
    depth = 0
    while i < len(text):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return i

def _rename(text, host, name, start, end, is_func, views) -> bool:
    new = f"__revo_view{len(views)}__{name}"
    seg = text[start:end]
    seg = re.sub(r"(?<![\w$.>])" + re.escape(name) + r"(?![\w$])", new, seg)
    if not is_func:
        m = re.match(re.escape(new) + r"((?:\s*\[[^\]]*\])*)", seg)
        if m:
            seg = seg[:m.end()] + f' __asm__("{name}")' + seg[m.end():]
    views.append({"name": name, "view": new, "kind": "function" if is_func else "data"})
    host.write_text(text[:start] + seg + text[end:], encoding="utf-8", errors="surrogateescape")
    return True

def first_error(stderr: str) -> str:
    for l in stderr.splitlines():
        if "error:" in l:
            return l.strip()
    return stderr.strip()[:300]

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--decomp", type=Path, default=os.environ.get("REVO_DECOMP_ROOT"))
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--jobs", type=int, default=os.cpu_count())
    ap.add_argument("--only", nargs="*")
    ap.add_argument("--define", action="append", default=[], help="a macro defined in every TU (repeatable)")
    ap.add_argument("--driver-api", type=Path, help="gli/driver.h: functions the native driver serves")
    ap.add_argument("--retail", type=Path, help="the retail SLUS_201.38: write c_functions.json (check_tables.py)")
    args = ap.parse_args()
    DEFINES.extend(f"-D{d}" for d in args.define)
    if args.driver_api:
        from retail import driver_api
        sym_file = args.decomp / "config" / "revo.symbol_addrs.txt"
        names_at: dict[int, list[str]] = {}
        for line in sym_file.read_text(errors="replace").splitlines():
            m = re.match(r"\s*(\w+)\s*=\s*0x([0-9A-Fa-f]+)\s*;", line)
            if m:
                names_at.setdefault(int(m.group(2), 16), []).append(m.group(1))
        for a in driver_api(args.driver_api):
            DRIVER_PROVIDED.extend(names_at.get(a, []) + [f"func_{a:08X}"])
    decomp = args.decomp.resolve()
    args.out = args.out.resolve()
    srcs = sorted([*(decomp / "src").rglob("*.c"), *(decomp / "src").rglob("*.cpp")])
    if args.only:
        srcs = [s for s in srcs if any(o in str(s) for o in args.only)]
    with cf.ThreadPoolExecutor(args.jobs) as ex:
        results = list(ex.map(lambda s: compile_tu(decomp, s, args.out), srcs))
    from signatures import functions
    renames: dict[str, list[str]] = {}
    for _ in range(4):
        sig_defs: dict[str, tuple] = {}
        sig_refs: dict[str, dict[tuple, list[str]]] = {}
        for r in results:
            if not r["ok"]:
                continue
            for name, (defined, t) in functions(Path(r["ir"]).read_text()).items():
                if defined:
                    sig_defs[name] = t
                else:
                    sig_refs.setdefault(name, {}).setdefault(t, []).append(r["tu"])
        changed: dict[str, list[str]] = {}
        for fn, by_type in sig_refs.items():
            canon = sig_defs.get(fn) or max(by_type, key=lambda t: len(by_type[t]))
            for t, tus in by_type.items():
                if t == canon:
                    continue
                for tu in tus:
                    d = f"-D{fn}=__revo_sig{hashlib.sha1(repr(t).encode()).hexdigest()[:6]}__{fn}"
                    if d not in renames.get(tu, []):
                        changed.setdefault(tu, []).append(d)
        if not changed:
            break
        for tu, ds in changed.items():
            renames.setdefault(tu, []).extend(ds)
        idx = {r["tu"]: k for k, r in enumerate(results)}
        for tu in changed:
            results[idx[tu]] = compile_tu(decomp, decomp / tu, args.out, renames[tu])
            results[idx[tu]]["signature_views"] = renames[tu]
    ok = [r for r in results if r["ok"]]
    (args.out / "guest_status.json").write_text(json.dumps(results, indent=1))
    # With the game and the decomp's PS2 build: which C functions are the retail ones (check_tables.py).
    if args.retail and not args.only and (decomp / "build" / "SLUS_201.38.elf").exists():
        import retail
        defined = {}
        for r in ok:
            for name, (is_defined, t) in functions(Path(r["ir"]).read_text()).items():
                if is_defined:
                    defined.setdefault(name, t)
        cfuncs, rejected = retail.c_functions(defined, decomp / "build" / "SLUS_201.38.elf", args.retail)
        (args.out / "c_functions.json").write_text(json.dumps(cfuncs, indent=0))
        if rejected:
            print(f"guest C: {len(rejected)} decomp C function(s) differ from retail: "
                  + ", ".join(f"{x['name']}@{x['addr']}" for x in rejected[:20])
                  + (" ..." if len(rejected) > 20 else ""), file=sys.stderr)
    print(f"guest C: {len(ok)}/{len(results)} TUs compile (ILP32, MIPS)")
    # A function the guest compile drops for an error would be missing from the program, and the game would stop
    # when it calls it: only the functions the native driver serves may be left out.
    broken = [(r["tu"], f) for r in results for f in r.get("fallback_functions", [])
              if not f["reason"].startswith("served by the native driver")]
    for tu, f in broken:
        print(f"  {tu}: {f['name']} does not compile: {f['reason']}")
    # A unit that does not compile is left out of the native build too, with all its functions.
    by = {}
    for r in results:
        if not r["ok"]:
            by.setdefault(r["stage"], []).append(r)
    for stage, rs in by.items():
        print(f"  {stage}: {len(rs)}")
        for r in rs[:6]:
            print(f"    {r['tu']}: {r['error'][:160]}")
    return 1 if broken or by else 0

if __name__ == "__main__":
    raise SystemExit(main())
