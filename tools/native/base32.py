#!/usr/bin/env python3
from __future__ import annotations

import os
import re
import sys

from irfix import P271, closing, split_args
import target

BASE = target.IMAGE_BASE_IR
LOCAL_BASE = "%b32.base"
BASE_GLOBAL = "@revo_base"
SSP = "@revo_ssp"
VAL = r'%(?:[-\w$.]+|"[^"]*")'
NAME_END = r"(?![-\w$.])"
LABEL = re.compile(r'^("[^"]+"|[-\w.$]+):')
DEFINE = re.compile(r"^define\b.*\{\s*$")
CAST_IN = re.compile(rf"^(\s*)({VAL}) = addrspacecast {re.escape(P271)} (.+) to ptr\s*$")
INTTOPTR = re.compile(rf"^(\s*)({VAL}) = inttoptr (i\d+) (.+) to ptr\s*$")
LOAD = re.compile(rf"^(\s*(?:{VAL} = )?load )((?:atomic )?(?:volatile )?)(.*)$")
STORE = re.compile(r"^(\s*store )((?:atomic )?(?:volatile )?)(.*)$")
ALLOCA = re.compile(rf"^(\s*)({VAL}) = alloca (.*)$")
RET = re.compile(r"^\s*ret\b")
CE_CAST = "addrspacecast (" + P271 + " "
NULL_SELECT = False
NULL_TEST = re.compile(rf"^(\s*)({VAL}) = icmp (eq|ne) ptr (?!addrspace)(.+), null\s*$")
PTR_TO_I64 = re.compile(rf"^(\s*)(%(?!b32\.)(?:[-\w$.]+|\"[^\"]*\")) = ptrtoint ptr (?!addrspace)(.+) to i64\s*$")

def uses(name: str) -> re.Pattern:
    return re.compile(r"(?<![-\w$.])" + re.escape(name) + NAME_END)

class Fn:

    def __init__(self, k: int):
        self.k, self.n = k, 0

    def name(self, tag: str) -> str:
        self.n += 1
        return f"%b32.{tag}{self.n}"

    def real(self, ind: str, value: str, null: bool, dst: str | None = None) -> tuple[list[str], str]:
        null = null and NULL_SELECT
        o, z = self.name("o"), self.name("z")
        g = dst if dst and not null else self.name("g")
        out = [f"{ind}{o} = ptrtoint {P271} {value} to i32", f"{ind}{z} = zext i32 {o} to i64",
               f"{ind}{g} = getelementptr i8, ptr {LOCAL_BASE}, i64 {z}"]
        if not null:
            return out, g
        nz, r = self.name("nz"), dst or self.name("r")
        out += [f"{ind}{nz} = icmp ne i32 {o}, 0", f"{ind}{r} = select i1 {nz}, ptr {g}, ptr null"]
        return out, r

    def offset_real(self, ind: str, off32: str, dst: str) -> list[str]:
        z, g, nz = self.name("z"), self.name("g"), self.name("nz")
        if not NULL_SELECT:
            return [f"{ind}{z} = zext i32 {off32} to i64", f"{ind}{dst} = getelementptr i8, ptr {LOCAL_BASE}, i64 {z}"]
        return [f"{ind}{z} = zext i32 {off32} to i64", f"{ind}{g} = getelementptr i8, ptr {LOCAL_BASE}, i64 {z}",
                f"{ind}{nz} = icmp ne i32 {off32}, 0", f"{ind}{dst} = select i1 {nz}, ptr {g}, ptr null"]

def indent(line: str) -> str:
    return re.match(r"^\s*", line).group(0) or "  "

def non_escaping(line: str, name: str, pat: re.Pattern) -> bool:
    if "@llvm.lifetime." in line:
        return True
    if len(pat.findall(line)) != 1:
        return False
    for rx in (LOAD, STORE):
        m = rx.match(line)
        if m:
            args = split_args(m.group(3))
            return len(args) > 1 and args[1].strip() == f"ptr {name}"
    return False

SLACK = 64
ZERO_LOCALS = False
LOCAL_FILL = int(os.environ.get("REVO_LOCAL_FILL", "0"), 0)

TYPES: dict[str, str] = {}

def float_only(ty: str, depth: int = 0) -> bool:
    ty = ty.strip()
    if depth > 20:
        return False
    if ty in ("float", "double"):
        return True
    m = re.fullmatch(r"[\[<]\d+ x (.+)[\]>]", ty)
    if m:
        return float_only(m.group(1), depth + 1)
    if ty in TYPES:
        return float_only(TYPES[ty], depth + 1)
    m = re.fullmatch(r"<?\{(.*)\}>?", ty)
    if m:
        fields = split_args(m.group(1))
        return bool(fields) and all(float_only(f, depth + 1) for f in fields)
    return False

def shadow_stack(fn: Fn, body: list[str], stats: dict) -> list[str]:
    allocas = [(i, m) for i, line in enumerate(body) if (m := ALLOCA.match(line))]
    moved: dict[int, re.Match] = {}
    for i, m in allocas:
        name = m.group(2)
        pat = uses(name)
        if any(pat.search(line) and not non_escaping(line, name, pat) for j, line in enumerate(body) if j != i):
            moved[i] = m
    stats["allocas"] += len(allocas)
    if not moved:
        return body
    stats["moved"] += len(moved)
    stats["fns_shadow"] += 1
    names = {m.group(2) for m in moved.values()}
    lifetimes = [uses(n) for n in names]
    out: list[str] = []
    for i, line in enumerate(body):
        if i in moved:
            ind, name, rest = moved[i].groups()
            args = [a.strip() for a in split_args(rest)]
            ty, count, align = args[0], None, 1
            for a in args[1:]:
                if a.startswith("align "):
                    align = int(a.split()[1])
                elif re.match(r"i(32|64) ", a):
                    count = a
            s, neg, b = fn.name("s"), fn.name("neg"), fn.name("b")
            out.append(f"{ind}{s} = load ptr, ptr {SSP}, align 8")
            if count is None or re.fullmatch(r"i(32|64) \d+", count):
                n = count.split()[1] if count else "1"
                size = f"ptrtoint (ptr getelementptr ({ty}, ptr null, i64 {n}) to i64)"
                out.append(f"{ind}{neg} = sub i64 -{SLACK}, {size}")
            else:
                cty, cval = count.split(None, 1)
                c64, sz = fn.name("cnt"), fn.name("size")
                out.append(f"{ind}{c64} = zext {cty} {cval} to i64" if cty == "i32" else f"{ind}{c64} = add i64 {cval}, 0")
                out.append(f"{ind}{sz} = mul i64 {c64}, ptrtoint (ptr getelementptr ({ty}, ptr null, i64 1) to i64)")
                out.append(f"{ind}{neg} = sub i64 -{SLACK}, {sz}")
                stats["dynamic"] += 1
            out.append(f"{ind}{b} = getelementptr i8, ptr {s}, i64 {neg}")
            out.append(f"{ind}{name} = call ptr @llvm.ptrmask.p0.i64(ptr {b}, i64 {-max(align, 16)})")
            out.append(f"{ind}store ptr {name}, ptr {SSP}, align 8")
            if ZERO_LOCALS:
                z = fn.name("len")
                fill = LOCAL_FILL if LOCAL_FILL and float_only(ty) else 0
                out.append(f"{ind}{z} = sub i64 0, {neg}")
                out.append(f"{ind}call void @llvm.memset.p0.i64(ptr {name}, i8 {fill}, i64 {z}, i1 false)")
            continue
        if "@llvm.lifetime." in line and any(p.search(line) for p in lifetimes):
            continue
        if RET.match(line):
            out.append(f"{indent(line)}store ptr %b32.sp0, ptr {SSP}, align 8")
        out.append(line)
    at = 1 if out and LABEL.match(out[0]) else 0
    out.insert(at, f"  %b32.sp0 = load ptr, ptr {SSP}, align 8")
    return out

KEEP_ATTR = re.compile(r"\bnoundef\b|\bsret\((?:[^()]|\([^()]*\))*\)")

def parameters(fn: Fn, head: str, body: list[str], stats: dict) -> tuple[str, list[str]]:
    at = head.find("(", head.index("@"))
    end = closing(head, at)
    params = split_args(head[at + 1:end - 1])
    new, entry = [], []
    for p in params:
        q = p.strip()
        m = re.match(rf"^ptr\b(?! addrspace)(.*?)\s({VAL})$", q)
        if not m or re.search(r"\b(byval|inalloca|preallocated)\b", q):
            new.append(q)
            continue
        attrs, name = m.group(1), m.group(2)
        kept = " ".join(KEEP_ATTR.findall(attrs))
        new.append(f"ptr {kept + ' ' if kept else ''}{name}")
        r = fn.name("p")
        pat = uses(name)
        body[:] = [pat.sub(r, line) for line in body]
        o = fn.name("o")
        entry += [f"  {o} = ptrtoint ptr {name} to i32"] + fn.offset_real("  ", o, r)
        stats["params"] += 1
    if entry:
        k = 1 if body and LABEL.match(body[0]) else 0
        body[k:k] = entry
    return head[:at + 1] + ", ".join(new) + head[end - 1:], body

TO271 = " to " + P271 + ")"
NUMBER = re.compile(r"^inttoptr \(i32 (-?\d+) to " + re.escape(P271) + r"\)$")

def constant_real(value: str) -> str | None:
    value = value.strip()
    if value == "null":
        return "null"
    m = NUMBER.match(value)
    if m:
        n = int(m.group(1)) & 0xFFFFFFFF
        return f"inttoptr (i64 add (i64 ptrtoint (ptr {BASE} to i64), i64 {n}) to ptr)" if n else "null"
    if "%" in value or "inttoptr" in value or "ptrtoint" in value:
        return None
    out, pos = [], 0
    while True:
        i = value.find("addrspacecast (ptr ", pos)
        if i < 0:
            break
        e = closing(value, i + len("addrspacecast "))
        expr = value[i:e]
        if not expr.endswith(TO271):
            return None
        out += [value[pos:i], expr[len("addrspacecast (ptr "):-len(TO271)]]
        pos = e
    out.append(value[pos:])
    v = "".join(out)
    if v == value and not v.startswith("@"):
        return None
    return v.replace(P271, "ptr")

def hoist_ce_casts(fn: Fn, line: str, stats: dict) -> tuple[list[str], str]:
    pre = []
    while True:
        i = line.find(CE_CAST)
        if i < 0:
            return pre, line
        e = closing(line, i + len("addrspacecast "))
        expr = line[i:e]
        if not expr.endswith(" to ptr)"):
            raise SystemExit(f"base32: unexpected cast {expr[:120]}")
        value = expr[len(CE_CAST):-len(" to ptr)")]
        c = constant_real(value)
        if c is not None:
            line = line[:i] + c + line[e:]
            stats["const_casts"] += 1
            continue
        if re.match(r"^\s*" + VAL + r" = phi ", line):
            raise SystemExit(f"base32: a 4-byte pointer cast in a phi: {line.strip()[:160]}")
        lines, r = fn.real(indent(line), value, null=True)
        pre += lines
        line = line[:i] + r + line[e:]
        stats["hoisted_casts"] += 1

def access(fn: Fn, line: str, stats: dict) -> list[str]:
    pre, line = hoist_ce_casts(fn, line, stats)
    ind = indent(line)
    m = CAST_IN.match(line)
    if m:
        lines, _ = fn.real(m.group(1), m.group(3), null=True, dst=m.group(2))
        stats["casts"] += 1
        return pre + lines
    m = PTR_TO_I64.match(line)
    if m:
        ind, dst, value = m.groups()
        t = fn.name("t")
        stats["ptr_ints"] += 1
        return pre + [f"{ind}{t} = ptrtoint ptr {value} to i32", f"{ind}{dst} = zext i32 {t} to i64"]
    m = NULL_TEST.match(line) if not NULL_SELECT else None
    if m:
        ind, dst, op, value = m.groups()
        t = fn.name("t")
        stats["null_tests"] += 1
        return pre + [f"{ind}{t} = ptrtoint ptr {value} to i32", f"{ind}{dst} = icmp {op} i32 {t}, 0"]
    m = INTTOPTR.match(line)
    if m:
        ind, dst, ty, value = m.groups()
        off = value
        if ty != "i32":
            off = fn.name("t")
            pre.append(f"{ind}{off} = trunc {ty} {value} to i32")
        stats["inttoptr"] += 1
        return pre + fn.offset_real(ind, off, dst)
    for rx in (LOAD, STORE):
        m = rx.match(line)
        if not m:
            continue
        args = split_args(m.group(3))
        if len(args) < 2 or not args[1].strip().startswith(P271):
            break
        lines, r = fn.real(ind, args[1].strip()[len(P271):].strip(), null=False)
        args[1] = f" ptr {r}"
        stats["accesses"] += 1
        return pre + lines + [m.group(1) + m.group(2) + ",".join(args)]
    if re.search(r"\b(atomicrmw|cmpxchg)\b", line) and P271 in line:
        raise SystemExit(f"base32: an atomic access through a 4-byte pointer: {line.strip()[:160]}")
    return pre + [line]

CALL = re.compile(rf"^(\s*)({VAL}) = (?:(?:tail|musttail|notail) )?call (.*)$")
RET_ATTR = re.compile(r"^(?:(?:fastcc|ccc|win64cc|x86_64_sysvcc|noundef|nonnull|noalias|zeroext|signext|inreg|"
                      r"align \d+|dereferenceable(?:_or_null)?\(\d+\)|range\([^)]*\)|nofpclass\([^)]*\))\s+)*")

def returns(fn: Fn, body: list[str], stats: dict) -> list[str]:
    out, keep, renames = [], set(), []
    for line in body:
        out.append(line)
        m = CALL.match(line)
        if not m:
            continue
        rest = m.group(3)
        rest = rest[RET_ATTR.match(rest).end():]
        if not re.match(r"ptr (?!addrspace)", rest) or re.search(r"@llvm\.", rest.split("(", 1)[0] + " "):
            continue
        callee = re.match(r"ptr (?:\([^)]*\) )?(\S+?)\(", rest)
        if callee and callee.group(1).startswith("@llvm."):
            continue
        ind, name = m.group(1), m.group(2)
        r, o = fn.name("ret"), fn.name("o")
        keep.add(len(out) - 1)
        for x in [f"{ind}{o} = ptrtoint ptr {name} to i32"] + fn.offset_real(ind, o, r):
            keep.add(len(out))
            out.append(x)
        renames.append((uses(name), r))
        stats["returns"] += 1
    for pat, r in renames:
        for i, line in enumerate(out):
            if i not in keep and pat.search(line):
                out[i] = pat.sub(r, line)
    return out

NANTRACE = os.environ.get("REVO_NANTRACE") == "1"
STORE_FLOAT = re.compile(rf"^(\s*)store (?:volatile )?float ({VAL}), ptr ")
LOAD_FLOAT = re.compile(rf"^(\s*)({VAL}) = load (?:volatile )?float, ptr ")

def nan_trace(fn: Fn, body: list[str], where: str) -> list[str]:
    out = []
    for line in body:
        m = STORE_FLOAT.match(line)
        if m:
            ind, v = m.group(1), m.group(2)
            ptr = split_args(STORE.match(line).group(3))[1].strip()[len("ptr "):]
            a = fn.name("nanat")
            out += [f"{ind}{a} = ptrtoint ptr {ptr} to i64",
                    f"{ind}call void @revo_nan_check(float {v}, ptr {where}, i64 {a})"]
        out.append(line)
        m = LOAD_FLOAT.match(line)
        if m:
            ind, v = m.group(1), m.group(2)
            ptr = split_args(LOAD.match(line).group(3))[1].strip()[len("ptr "):]
            a = fn.name("nanat")
            out += [f"{ind}{a} = ptrtoint ptr {ptr} to i64",
                    f"{ind}call void @revo_nan_read(float {v}, ptr {where}, i64 {a})"]
    return out

def transform(text: str) -> tuple[str, dict]:
    stats = dict.fromkeys(("functions", "allocas", "moved", "dynamic", "fns_shadow", "params", "casts",
                           "const_casts", "hoisted_casts", "inttoptr", "accesses", "returns", "null_tests", "ptr_ints"), 0)
    lines, out, k, strings = text.split("\n"), [], 0, []
    TYPES.clear()
    for m in re.finditer(r'^(%[-\w.$"]+) = type (.*)$', text, re.M):
        TYPES[m.group(1)] = m.group(2)
    while k < len(lines):
        line = lines[k]
        if not DEFINE.match(line):
            out.append(line)
            k += 1
            continue
        end = lines.index("}", k)
        fn = Fn(k)
        body = returns(fn, lines[k + 1:end], stats)
        body = shadow_stack(fn, body, stats)
        head, body = parameters(fn, line, body, stats)
        new = []
        for b in body:
            new += access(fn, b, stats) if b.strip() and not b.lstrip().startswith(";") and not LABEL.match(b) else [b]
        if NANTRACE:
            fname = re.search(r'@("[^"]+"|[\w.$]+)\(', head).group(1).strip('"')
            where = f"@b32.where.{stats['functions']}"
            strings.append(f'{where} = private unnamed_addr constant [{len(fname) + 1} x i8] c"{fname}\\00"')
            new = nan_trace(fn, new, where)
        if any(LOCAL_BASE in x for x in new):
            at = 1 if new and LABEL.match(new[0]) else 0
            new.insert(at, f"  {LOCAL_BASE} = load ptr, ptr {BASE_GLOBAL}, align 8")
        out += [head, *new, "}"]
        stats["functions"] += 1
        k = end + 1
    text = "\n".join(out + strings)
    if NANTRACE:
        text += "\ndeclare void @revo_nan_check(float, ptr, i64)\ndeclare void @revo_nan_read(float, ptr, i64)"
    if BASE in text and not re.search(r"^" + re.escape(BASE) + r" = ", text, re.M):
        text += f"\n{BASE} = external dso_local global i8"
    if BASE_GLOBAL in text and not re.search(r"^" + re.escape(BASE_GLOBAL) + r" = ", text, re.M):
        text += f"\n{BASE_GLOBAL} = external constant ptr, align 8"
    if SSP in text and not re.search(r"^" + re.escape(SSP) + r" = ", text, re.M):
        text += f"\n{SSP} = external thread_local global ptr, align 8"
    if "@llvm.memset.p0.i64" in text and not re.search(r"^declare [^\n]*@llvm\.memset\.p0\.i64\(", text, re.M):
        text += "\ndeclare void @llvm.memset.p0.i64(ptr, i8, i64, i1 immarg)"
    if "@llvm.ptrmask.p0.i64" in text and not re.search(r"^declare [^\n]*@llvm\.ptrmask\.p0\.i64\(", text, re.M):
        text += "\ndeclare ptr @llvm.ptrmask.p0.i64(ptr, i64)"
    return text + "\n", stats

if __name__ == "__main__":
    src = open(sys.argv[1], encoding="utf-8").read()
    res, st = transform(src)
    open(sys.argv[2], "w", encoding="utf-8").write(res)
    print("base32:", st)
