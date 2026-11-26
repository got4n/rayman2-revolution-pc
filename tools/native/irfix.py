#!/usr/bin/env python3
from __future__ import annotations

import re
import sys

P271 = "ptr addrspace(271)"
CAST = "addrspacecast (ptr "
TO271 = " to " + P271 + ")"

def closing(text: str, i: int) -> int:
    depth = 0
    while i < len(text):
        c = text[i]
        if c == '"':
            i = text.index('"', i + 1)
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise ValueError("unbalanced parentheses")

def fix_statics(line: str) -> tuple[str, int]:
    out, pos, n = [], 0, 0
    while True:
        i = line.find(CAST, pos)
        if i < 0:
            break
        end = closing(line, i + len("addrspacecast "))
        expr = line[i:end]
        if not expr.endswith(TO271):
            out.append(line[pos:end])
            pos = end
            continue
        const = expr[len(CAST):-len(TO271)]
        out.append(line[pos:i])
        out.append(f"inttoptr (i32 ptrtoint (ptr {const} to i32) to {P271})")
        pos = end
        n += 1
    out.append(line[pos:])
    return "".join(out), n

def ordinary(value: str) -> str:
    if value.startswith(CAST) and value.endswith(TO271) and closing(value, len("addrspacecast ")) == len(value):
        return value[len(CAST):-len(TO271)]
    return f"addrspacecast ({P271} {value} to ptr)"

def split_args(s: str) -> list[str]:
    args, depth, cur = [], 0, []
    for c in s:
        if c in "([{<":
            depth += 1
        elif c in ")]}>":
            depth -= 1
        if c == "," and depth == 0:
            args.append("".join(cur))
            cur = []
        else:
            cur.append(c)
    if "".join(cur).strip():
        args.append("".join(cur))
    return args

CALL_HEAD = r"^(\s*(?:%[\w.]+ = )?(?:tail |musttail |notail )?call(?: (?!addrspace)\w+(?:\(\d+\))?)*) addrspace\(271\) "
MARKED = re.compile(CALL_HEAD)
CALL271 = re.compile(CALL_HEAD[:-1] + r"( [^%@\n]*?)(%[\w.]+)\(")
MEMI = re.compile(r"^(\s*)((?:tail |notail )?call void )@llvm\.(memcpy|memmove|memset)((?:\.inline)?)"
                  r"\.([\w.]+)\((.*)\)(.*)$")
DECLARE_MEM = re.compile(r"^declare [^\n]*@llvm\.(memcpy|memmove|memset)(?:\.inline)?\.[\w.]*p271")
ATTRS = re.compile(r"^(?:(?:noundef|nonnull|noalias|nocapture|readonly|writeonly|align \d+|"
                   r"dereferenceable(?:_or_null)?\(\d+\)|captures\([^)]*\))\s+)*")

def fix_lines(text: str) -> tuple[str, int, int]:
    out, calls, mems, n = [], 0, 0, 0
    declared = set(re.findall(r"^declare [^\n]*?(@llvm\.mem\w+(?:\.inline)?\.[\w.]+)\(", text, re.M))
    need: dict[str, str] = {}
    for line in text.split("\n"):
        if DECLARE_MEM.match(line):
            continue
        head = MARKED.match(line)
        if head and not CALL271.match(line):
            k = head.end(1)
            a = line.find(CAST, k)
            if a > 0:
                e = closing(line, a + len("addrspacecast "))
                line = line[:k] + " " + line[k + len(" addrspace(271) "):a] + ordinary(line[a:e]) + line[e:]
                calls += 1
        c = CALL271.match(line)
        if c:
            n += 1
            ind = re.match(r"^\s*", line).group(0)
            tmp = f"%__revo_fp{n}"
            out.append(f"{ind}{tmp} = addrspacecast {P271} {c.group(3)} to ptr")
            line = c.group(1) + c.group(2) + tmp + "(" + line[c.end():]
            calls += 1
        m = MEMI.match(line)
        if m and P271 in m.group(6):
            ind, call, kind, inline, _, args, rest = m.groups()
            new_args = []
            for arg in split_args(args):
                arg = arg.strip()
                if arg.startswith(P271):
                    tail = arg[len(P271):].strip()
                    attrs = ATTRS.match(tail).group(0)
                    val = tail[len(attrs):].strip()
                    if val.startswith("%"):
                        n += 1
                        tmp = f"%__revo_mp{n}"
                        out.append(f"{ind}{tmp} = addrspacecast {P271} {val} to ptr")
                        val = tmp
                    else:
                        val = ordinary(val)
                    arg = "ptr " + (attrs.strip() + " " if attrs.strip() else "") + val
                new_args.append(arg)
            size_ty = new_args[2].split()[0]
            if kind == "memset":
                name = f"@llvm.memset{inline}.p0.{size_ty}"
                need[name] = f"declare void {name}(ptr, i8, {size_ty}, i1)"
            else:
                name = f"@llvm.{kind}{inline}.p0.p0.{size_ty}"
                need[name] = f"declare void {name}(ptr, ptr, {size_ty}, i1)"
            line = f"{ind}{call}{name}({', '.join(new_args)}){rest}"
            mems += 1
        out.append(line)
    for name, decl in need.items():
        if name not in declared:
            out.append(decl)
    return "\n".join(out), calls, mems

CONSTANT = re.compile(r"^(@\S+ = (?:[\w()\".]+ )*?)constant ")

DIVISION = re.compile(r"^(\s*%[\w.]+ = )(fdiv|sdiv|srem|udiv|urem)((?: [a-z]+)*) (float|i16|i32|i64) ([^,]+), "
                      r"([^,]+?)((?:, ![\w.]+ ![\w.]+)*)$")
CONSTANT_OPERAND = re.compile(r"-?\d+|-?\d+\.\d+e[+-]\d+|0x[0-9A-Fa-f]+")

def ee_arithmetic(text: str) -> tuple[str, int]:
    out, n, used = [], 0, set()
    for line in text.split("\n"):
        m = DIVISION.match(line)
        if m:
            head, op, _, ty, a, b, meta = m.groups()
            if CONSTANT_OPERAND.fullmatch(b.strip()) and b.strip() not in ("0", "-1", "0.000000e+00"):
                out.append(line)
                continue
            fn = "revo_fdiv" if op == "fdiv" else f"revo_{op}_{ty}"
            used.add((fn, ty))
            out.append(f"{head}call {ty} @{fn}({ty} {a}, {ty} {b}){meta}")
            n += 1
            continue
        out.append(line)
    out += [f"declare {ty} @{fn}({ty}, {ty})" for fn, ty in sorted(used)]
    return "\n".join(out), n

LABEL = re.compile(r'^("[^"]+"|[\w.$-]+):')
BRANCH = re.compile(r"^(\s+)br (?:i1 [^,]+, )?label %")
TARGETS = re.compile(r'label %("[^"]+"|[\w.$-]+)')

def preempt_loops(text: str) -> tuple[str, int]:
    lines, out, n = text.split("\n"), [], 0
    k = 0
    while k < len(lines):
        if not lines[k].startswith("define "):
            out.append(lines[k])
            k += 1
            continue
        end = lines.index("}", k)
        body, n = function_preempt(lines[k + 1:end], n)
        out += [lines[k], *body, "}"]
        k = end + 1
    if n:
        out += ["@revo_budget = external global i32, align 4", "declare void @revo_preempt()"]
    return "\n".join(out), n

def function_preempt(body: list[str], n: int) -> tuple[list[str], int]:
    blocks: list[list] = [[None, []]]
    for line in body:
        m = LABEL.match(line)
        if m:
            blocks.append([m.group(1), [line]])
        else:
            blocks[-1][1].append(line)
    order = {name: i for i, (name, _) in enumerate(blocks) if name}
    renames: dict[str, list[tuple[str, str]]] = {}
    for i, blk in enumerate(blocks):
        name, ls = blk
        if name is None:
            continue
        j = max((x for x in range(len(ls)) if ls[x].strip()), default=-1)
        b = BRANCH.match(ls[j]) if j >= 0 else None
        if not b:
            continue
        targets = TARGETS.findall(ls[j])
        if not any(order.get(t, len(blocks)) <= i for t in targets):
            continue
        ind, pre, cont = b.group(1), f"revo.preempt{n}", f"revo.cont{n}"
        blk[1] = ls[:j] + [f"{ind}%revo.b{n} = load i32, ptr @revo_budget, align 4",
                           f"{ind}%revo.d{n} = add i32 %revo.b{n}, -1",
                           f"{ind}store i32 %revo.d{n}, ptr @revo_budget, align 4",
                           f"{ind}%revo.z{n} = icmp slt i32 %revo.d{n}, 1",
                           f"{ind}br i1 %revo.z{n}, label %{pre}, label %{cont}",
                           "", f"{pre}:", f"{ind}call void @revo_preempt()", f"{ind}br label %{cont}",
                           "", f"{cont}:", ls[j]] + ls[j + 1:]
        for t in set(targets):
            renames.setdefault(t, []).append((name, cont))
        n += 1
    for name, ls in blocks:
        for old, new in renames.get(name, []):
            pat = re.compile(r"(\[ [^\[\]]*?, )%" + re.escape(old) + r"( \])")
            for x, line in enumerate(ls):
                if " = phi " in line:
                    ls[x] = pat.sub(lambda m: m.group(1) + "%" + new + m.group(2), line)
    return [line for _, ls in blocks for line in ls], n

FUNCTION_HEAD = re.compile(r'^(define|declare)\b(.*?)\b(i1|i8|i16|i32|i64|float|double|void|ptr(?: addrspace\(\d+\))?|'
                           r'\{[^}]*\}|%[\w.]+)\s+@("[^"]+"|[\w.$]+)\((.*)\)(.*)$')
NARROW = ("i1", "i8", "i16")

def split_params(params: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in params:
        depth += ch in "({[<"
        depth -= ch in ")}]>"
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur)
    return out

def extend(line: str, ext: dict[str, list[str]]) -> str:
    m = FUNCTION_HEAD.match(line)
    if not m:
        return line
    kind, attrs, ret, name, params, tail = m.groups()
    e = ext.get(name.strip('"'))
    if not e:
        return line
    if e[0] and ret in NARROW and kind == "define":
        attrs = re.sub(r"\b(zeroext|signext)\b|\brange\([^)]*\)", "", attrs)
        attrs = re.sub(r"\s+", " ", attrs).rstrip() + " "
        ret = "i32"
    elif e[0] and ret in NARROW and not re.search(r"\b(zeroext|signext)\b", attrs):
        attrs = f"{attrs.rstrip()} {e[0]} " if attrs.strip() else f" {e[0]} "
    ps = split_params(params)
    for k, p in enumerate(ps):
        if k + 1 < len(e) and e[k + 1]:
            pm = re.match(r"(\s*)(i1|i8|i16)\b(.*)$", p)
            if pm and not re.search(r"\b(zeroext|signext)\b", pm.group(3)):
                ps[k] = f"{pm.group(1)}{pm.group(2)} {e[k + 1]}{pm.group(3)}"
    return f"{kind}{attrs}{ret} @{name}({','.join(ps)}){tail}"

PTR_TYPE = r"ptr(?: addrspace\(\d+\))?"
CAST_INSN = re.compile(rf"^(\s*%[-\w.$]+ = )(addrspacecast|bitcast) ({PTR_TYPE}) (%[-\w.$]+) to ({PTR_TYPE})\s*$")

def cast_opcode(line: str) -> str:
    """Clang 18 to 20 for x86-64 ELF (apt's) can print a cast of ptr32 pointers with the wrong opcode: an
    addrspacecast between pointers of one address space, or a bitcast between two. The IR reader refuses both."""
    m = CAST_INSN.match(line)
    if not m:
        return line
    same = m.group(3) == m.group(5)
    if same == (m.group(2) == "bitcast"):
        return line
    return f"{m.group(1)}{'bitcast' if same else 'addrspacecast'} {m.group(3)} {m.group(4)} to {m.group(5)}"

def fix(text: str, ext: dict[str, list[str]] | None = None, preempt: bool = True) -> tuple[str, int, int]:
    lines, statics = [cast_opcode(line) for line in text.split("\n")], 0
    widened, n = None, 0
    for k, line in enumerate(lines):
        if ext and (line.startswith("define ") or line.startswith("declare ")):
            old = FUNCTION_HEAD.match(line)
            lines[k] = line = extend(line, ext)
            new = FUNCTION_HEAD.match(line)
            widened = None
            if line.startswith("define ") and old and new and old.group(3) in NARROW and new.group(3) == "i32":
                widened = (old.group(3), "zext" if ext[old.group(4).strip('"')][0] == "zeroext" else "sext")
        elif widened and line == "}":
            widened = None
        elif widened:
            r = re.match(r"^(\s*)ret (i1|i8|i16) (.+)$", line)
            if r:
                lines[k] = (f"{r.group(1)}%revo.ret{n} = {widened[1]} {r.group(2)} {r.group(3)} to i32\n"
                            f"{r.group(1)}ret i32 %revo.ret{n}")
                n += 1
        if line.startswith("@"):
            lines[k] = line = CONSTANT.sub(r"\1global ", line, count=1)
        if line.startswith("@") and CAST in line:
            lines[k], s = fix_statics(line)
            statics += s
    text, calls, mems = fix_lines("\n".join(lines))
    if preempt:
        text, loops = preempt_loops(text)
    text, divisions = ee_arithmetic(text)
    return text, statics, calls + mems

if __name__ == "__main__":
    src = open(sys.argv[1], encoding="utf-8").read()
    fixed, s, c = fix(src)
    open(sys.argv[2], "w", encoding="utf-8").write(fixed)
    print(f"irfix: {s} static pointers, {c} calls")
