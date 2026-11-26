#!/usr/bin/env python3
from __future__ import annotations

import re
from pathlib import Path

ASM_KW = re.compile(r"\b(__asm__|__asm|asm)\b")
IDENT = re.compile(r"^[A-Za-z0-9_.$][\w.$]*$")
HOST_DIR = Path(__file__).resolve().parents[1] / "host"

STMT_MNEMONICS = {"sync", "sync.p", "nop", "ei", "di", "syscall", "vnop", "vwaitq"}

class Untranslatable(Exception):
    pass

def match_paren(s: str, i: int) -> int:
    depth = 0
    n = len(s)
    while i < n:
        c = s[i]
        if c in "\"'":
            j = i + 1
            while s[j] != c:
                j += 2 if s[j] == "\\" else 1
            i = j + 1
            continue
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise Untranslatable("unbalanced parentheses")

def split_top(s: str, sep: str) -> list[str]:
    parts, depth, cur, i = [], 0, [], 0
    while i < len(s):
        c = s[i]
        if c in "\"'":
            j = i + 1
            while s[j] != c:
                j += 2 if s[j] == "\\" else 1
            cur.append(s[i:j + 1])
            i = j + 1
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == sep and depth == 0:
            parts.append("".join(cur))
            cur = []
        else:
            cur.append(c)
        i += 1
    parts.append("".join(cur))
    return parts

def string_value(s: str) -> str:
    out = []
    for m in re.finditer(r'"((?:[^"\\]|\\.)*)"', s):
        out.append(m.group(1).replace("\\n", "\n").replace("\\t", "\t").replace('\\"', '"'))
    return "".join(out)

def parse_operands(text: str) -> list[tuple[str, str]]:
    text = text.strip()
    if not text:
        return []
    ops = []
    for part in split_top(text, ","):
        part = part.strip()
        m = re.match(r'^(?:\[\w+\]\s*)?"([^"]*)"\s*\((.*)\)\s*$', part, re.S)
        if not m:
            raise Untranslatable(f"operand {part!r}")
        expr, inner_problems = transform(m.group(2).strip())
        if inner_problems:
            raise Untranslatable(inner_problems[0])
        ops.append((m.group(1), expr))
    return ops

VU_BASE = ("opmula", "opmsub", "madd", "msub", "mul", "add", "sub", "max", "mini", "move",
           "mr32", "abs", "ftoi0", "ftoi4", "ftoi12", "ftoi15", "itof0", "itof4", "itof12", "itof15")
VU_RE = re.compile(r"^v(" + "|".join(VU_BASE) + r")(a?)([xyzwqi]?)(?:\.([xyzw]+))?$")
VU_OPNAME = {"add": "ADD", "sub": "SUB", "mul": "MUL", "madd": "MADD", "msub": "MSUB", "max": "MAX",
             "mini": "MINI", "opmula": "OPMULA", "opmsub": "OPMSUB", "move": "MOVE", "mr32": "MR32",
             "abs": "ABS"}

def dest_mask(s: str | None) -> int:
    if not s:
        return 15
    return sum({"x": 8, "y": 4, "z": 2, "w": 1}[c] for c in s)

class Block:
    def __init__(self, outs, ins):
        self.outs, self.ins = outs, ins
        self.operands = [e for _, e in outs] + [e for _, e in ins]
        self.cons = [c for c, _ in outs] + [c for c, _ in ins]
        self.temps: dict[str, str] = {}
        self.stmts: list[str] = []
        self.uses_cpu = False
        for c, e in ins:
            if c.isdigit():
                self.stmts.append(f"({self.operands[int(c)]}) = ({e});")

    def op(self, tok: str) -> str:
        tok = tok.strip()
        m = re.fullmatch(r"%(\d+)", tok)
        if m:
            return f"({self.operands[int(m.group(1))]})"
        m = re.fullmatch(r"\$f(\d+)", tok)
        if m:
            self.temps[f"_hf{m.group(1)}"] = "float"
            return f"_hf{m.group(1)}"
        if tok in ("$0", "$zero"):
            return "0"
        m = re.fullmatch(r"\$(\d+|[a-z]\w*)", tok)
        if m:
            self.temps[f"_hr_{m.group(1)}"] = "unsigned long long"
            return f"_hr_{m.group(1)}"
        if re.fullmatch(r"-?(0x[0-9A-Fa-f]+|\d+)", tok):
            return tok
        raise Untranslatable(f"operand token {tok!r}")

    def vf(self, tok: str) -> tuple[int, int | None]:
        tok = tok.strip()
        m = re.fullmatch(r"\$vf(\d+)([xyzw]?)", tok)
        if not m:
            raise Untranslatable(f"vf token {tok!r}")
        self.uses_cpu = True
        bc = "xyzw".index(m.group(2)) if m.group(2) else None
        return int(m.group(1)), bc

    def vi(self, tok: str) -> int:
        m = re.fullmatch(r"\$vi(\d+)", tok.strip())
        if not m:
            raise Untranslatable(f"vi token {tok!r}")
        self.uses_cpu = True
        return int(m.group(1))

    def addr(self, tok: str) -> str:
        tok = tok.strip()
        m = re.fullmatch(r"%(\d+)", tok)
        if m and "m" in self.cons[int(m.group(1))]:
            return f"((char *)&({self.operands[int(m.group(1))]}))"
        m = re.fullmatch(r"(.*)\((.+)\)", tok)
        if not m:
            raise Untranslatable(f"address {tok!r}")
        off = m.group(1).strip() or "0"
        if off.startswith("%"):
            off = self.op(off)
        return f"((char *)(__revo_uptr)({self.op(m.group(2))}) + ({off}))"

    def q(self, tok: str) -> str:
        return f"__revo_toq({self.op(tok)})"

    def insn(self, line: str) -> None:
        parts = line.split(None, 1)
        mn = parts[0]
        A = [a.strip() for a in split_top(parts[1], ",")] if len(parts) > 1 else []
        S = self.stmts.append
        base_mn = mn.split(".")[0] if mn.endswith((".ni", ".i")) else mn
        if mn in ("nop", "vnop", "sync", "sync.p", "vwaitq"):
            return
        if mn in ("ei", "di"):
            S(f"__revo_cop0_{mn}();")
            return
        fbin = {"add.s": "+", "sub.s": "-", "mul.s": "*", "div.s": "/"}
        if mn in fbin:
            S(f"{self.op(A[0])} = {self.op(A[1])} {fbin[mn]} {self.op(A[2])};")
        elif mn == "abs.s":
            S(f"{self.op(A[0])} = __builtin_fabsf({self.op(A[1])});")
        elif mn == "neg.s":
            S(f"{self.op(A[0])} = -{self.op(A[1])};")
        elif mn == "mov.s":
            S(f"{self.op(A[0])} = {self.op(A[1])};")
        elif mn == "sqrt.s":
            S(f"{self.op(A[0])} = __builtin_sqrtf(__builtin_fabsf({self.op(A[1])}));")
        elif mn == "rsqrt.s":
            S(f"{self.op(A[0])} = {self.op(A[1])} / __builtin_sqrtf(__builtin_fabsf({self.op(A[2])}));")
        elif mn == "max.s":
            S(f"{self.op(A[0])} = __revo_fmax({self.op(A[1])}, {self.op(A[2])});")
        elif mn == "min.s":
            S(f"{self.op(A[0])} = __revo_fmin({self.op(A[1])}, {self.op(A[2])});")
        elif mn in ("mula.s", "adda.s", "suba.s", "madda.s", "msuba.s"):
            self.temps["_hacc"] = "float"
            o = {"mula.s": "= {a} * {b}", "adda.s": "= {a} + {b}", "suba.s": "= {a} - {b}",
                 "madda.s": "+= {a} * {b}", "msuba.s": "-= {a} * {b}"}[mn]
            S("_hacc " + o.format(a=self.op(A[0]), b=self.op(A[1])) + ";")
        elif mn in ("madd.s", "msub.s"):
            self.temps["_hacc"] = "float"
            sign = "+" if mn == "madd.s" else "-"
            S(f"{self.op(A[0])} = _hacc {sign} {self.op(A[1])} * {self.op(A[2])};")
        elif mn == "cvt.w.s":
            S(f"{self.op(A[0])} = __revo_i2f_bits(__revo_trunc({self.op(A[1])}));")
        elif mn == "cvt.s.w":
            S(f"{self.op(A[0])} = (float)(int)__revo_f2i_bits({self.op(A[1])});")
        elif mn == "mfc1":
            S(f"{self.op(A[0])} = (int)__revo_f2i_bits({self.op(A[1])});")
        elif mn == "mtc1":
            S(f"{self.op(A[1])} = __revo_i2f_bits((unsigned)({self.op(A[0])}));")
        elif mn == "lwc1":
            S(f"{self.op(A[0])} = *(float *){self.addr(A[1])};")
        elif mn == "swc1":
            S(f"*(float *){self.addr(A[1])} = {self.op(A[0])};")
        elif mn in ("lw", "sw", "ld", "sd", "lq", "sq"):
            ty = {"lw": "int", "sw": "int", "ld": "long long", "sd": "long long",
                  "lq": "unsigned __int128", "sq": "unsigned __int128"}[mn]
            if mn.startswith("l"):
                S(f"{self.op(A[0])} = *({ty} *){self.addr(A[1])};")
            else:
                S(f"*({ty} *){self.addr(A[1])} = {self.op(A[0])};")
        elif mn in ("move",) or (mn in ("or", "daddu", "addu") and len(A) == 3 and A[2] in ("$0", "$zero")):
            S(f"{self.op(A[0])} = {self.op(A[1])};")
        elif mn in ("andi", "ori", "xori", "and", "or", "xor"):
            o = {"and": "&", "or": "|", "xor": "^"}[mn.rstrip("i")]
            S(f"{self.op(A[0])} = {self.op(A[1])} {o} {self.op(A[2])};")
        elif mn in ("addiu", "addu"):
            S(f"{self.op(A[0])} = (int)((unsigned)({self.op(A[1])}) + (unsigned)({self.op(A[2])}));")
        elif mn in ("subu",):
            S(f"{self.op(A[0])} = (int)((unsigned)({self.op(A[1])}) - (unsigned)({self.op(A[2])}));")
        elif mn == "lui":
            S(f"{self.op(A[0])} = (int)((unsigned)({self.op(A[1])}) << 16);")
        elif mn == "li":
            S(f"{self.op(A[0])} = {self.op(A[1])};")
        elif mn in ("dsll", "dsrl", "dsra", "dsll32", "dsrl32", "dsra32"):
            extra = " + 32" if mn.endswith("32") else ""
            if mn.startswith("dsra"):
                S(f"{self.op(A[0])} = (long long)({self.op(A[1])}) >> (({self.op(A[2])}){extra});")
            else:
                o = "<<" if mn.startswith("dsll") else ">>"
                S(f"{self.op(A[0])} = (unsigned long long)({self.op(A[1])}) {o} (({self.op(A[2])}){extra});")
        elif mn in ("mult", "multu") and len(A) == 3:
            ty = "int" if mn == "mult" else "unsigned"
            S(f"{self.op(A[0])} = (int)(({ty})({self.op(A[1])}) * ({ty})({self.op(A[2])}));")
        elif mn == "mfc0":
            m0 = re.fullmatch(r"\$(\d+)", A[1].strip())
            if not m0:
                raise Untranslatable(f"mfc0 operand {A[1]!r}")
            S(f"{self.op(A[0])} = (int)__revo_cop0_read({m0.group(1)});")
        elif mn in ("sll", "srl", "sra"):
            if mn == "sra":
                S(f"{self.op(A[0])} = (int)({self.op(A[1])}) >> ({self.op(A[2])});")
            else:
                o = "<<" if mn == "sll" else ">>"
                S(f"{self.op(A[0])} = (int)((unsigned)({self.op(A[1])}) {o} ({self.op(A[2])}));")
        elif mn == "mtsab":
            self.uses_cpu = True
            S(f"__REVO_C->sa = (((unsigned)({self.op(A[0])}) & 15) ^ (({self.op(A[1])}) & 15)) * 8;")
        elif mn == "qfsrv":
            self.uses_cpu = True
            S(f"{{ RevoQ _o; revo_mmi(\"qfsrv\", {self.q(A[1])}, {self.q(A[2])}, __REVO_C->sa, &_o); "
              f"{self.op(A[0])} = __revo_fromq(_o); }}")
        elif mn.startswith("p") and len(A) in (2, 3):
            shift = mn in ("psllh", "psrlh", "psrah", "psllw", "psrlw", "psraw")
            if shift:
                s, t, sa = "__revo_toq(0)", self.q(A[1]), self.op(A[2])
            elif len(A) == 2:
                s, t, sa = "__revo_toq(0)", self.q(A[1]), "0"
            else:
                s, t, sa = self.q(A[1]), self.q(A[2]), "0"
            S(f"{{ RevoQ _o; if (!revo_mmi(\"{mn}\", {s}, {t}, {sa}, &_o)) __builtin_trap(); "
              f"{self.op(A[0])} = __revo_fromq(_o); }}")
        elif mn == "lqc2":
            n, _ = self.vf(A[0])
            S(f"__builtin_memcpy(__REVO_C->vf[{n}], {self.addr(A[1])}, 16);" if n else ";")
        elif mn == "sqc2":
            n, _ = self.vf(A[0])
            S(f"__builtin_memcpy({self.addr(A[1])}, __REVO_C->vf[{n}], 16);")
        elif base_mn == "qmfc2":
            n, _ = self.vf(A[1])
            S(f"{{ RevoQ _o; __builtin_memcpy(&_o, __REVO_C->vf[{n}], 16); {self.op(A[0])} = __revo_fromq(_o); }}")
        elif base_mn == "qmtc2":
            n, _ = self.vf(A[1])
            S(f"{{ RevoQ _o = {self.q(A[0])}; __builtin_memcpy(__REVO_C->vf[{n}], &_o, 16); }}" if n else ";")
        elif base_mn == "cfc2":
            S(f"{self.op(A[0])} = (int)__REVO_C->vi[{self.vi(A[1])}];")
        elif base_mn == "ctc2":
            S(f"__REVO_C->vi[{self.vi(A[1])}] = (unsigned)({self.op(A[0])});")
        elif mn == "vcallms":
            S(f"__revo_vu0_callms((unsigned)({self.op(A[0])}));")
        elif mn == "vcallmsr":
            S(f"__revo_vu0_callms(__REVO_C->vi[{self.vi(A[0])}] << 3);")
        elif mn in ("vdiv", "vsqrt", "vrsqrt"):
            kind = {"vdiv": 0, "vsqrt": 1, "vrsqrt": 2}[mn]
            if kind == 1:
                ft, ftf = self.vf(A[1])
                fs, fsf = 0, 0
            else:
                fs, fsf = self.vf(A[1])
                ft, ftf = self.vf(A[2])
            S(f"revo_vdiv(__REVO_C, {kind}, {fs}, {fsf or 0}, {ft}, {ftf or 0});")
        elif mn.startswith(("vlqi", "vsqi")):
            n, _ = self.vf(A[0] if mn.startswith("vlqi") else A[0])
            m = re.fullmatch(r"\(\$vi(\d+)\+\+\)", A[1].replace(" ", ""))
            if not m:
                raise Untranslatable(f"vlqi/vsqi operand {A[1]!r}")
            vi = int(m.group(1))
            fn = "__revo_vu0_lqi" if mn.startswith("vlqi") else "__revo_vu0_sqi"
            S(f"{fn}({n}, {vi}, {dest_mask(mn.split('.')[1] if '.' in mn else None)});")
        else:
            m = VU_RE.match(mn)
            if not m:
                raise Untranslatable(f"instruction {line!r}")
            base, acc, bcs, dest = m.groups()
            self.uses_cpu = True
            dm = dest_mask(dest)
            if base.startswith(("ftoi", "itof")):
                fd, _ = self.vf(A[0])
                fs, _ = self.vf(A[1])
                S(f"revo_vop(__REVO_C, REVO_VOP_{base.upper()}, {fd}, {fs}, 0, REVO_VBC_NONE, {dm});")
                return
            if base in ("move", "mr32", "abs"):
                fd, _ = self.vf(A[0])
                fs, _ = self.vf(A[1])
                S(f"revo_vop(__REVO_C, REVO_VOP_{VU_OPNAME[base]}, {fd}, {fs}, 0, REVO_VBC_NONE, {dm});")
                return
            is_acc = bool(acc) or base == "opmula"
            if A[0].strip().upper().lstrip("$") == "ACC":
                is_acc = True
                fd = "REVO_VACC"
            else:
                fd = "REVO_VACC" if is_acc else str(self.vf(A[0])[0])
            fs, _ = self.vf(A[1])
            if bcs in ("q", "i"):
                ft, bc = 0, "REVO_VBC_Q" if bcs == "q" else "REVO_VBC_I"
            else:
                ft, tbc = self.vf(A[2])
                bc = str("xyzw".index(bcs)) if bcs else "REVO_VBC_NONE"
            S(f"revo_vop(__REVO_C, REVO_VOP_{VU_OPNAME[base]}, {fd}, {fs}, {ft}, {bc}, {dm});")

    def render(self) -> str:
        decls = " ".join(f"{t} {n} = 0;" for n, t in self.temps.items())
        return "do { " + decls + " " + " ".join(self.stmts) + " } while (0)"

def translate_block(tmpl: str, outs, ins) -> str:
    b = Block(outs, ins)
    for line in re.split(r"[\n;]", tmpl):
        line = line.split("#")[0].strip()
        if not line or line.startswith("."):
            continue
        b.insn(line)
    return b.render()

def _find_asm(text: str, i: int):
    n = len(text)
    while i < n:
        c = text[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if c == "_" or c == "a":
            m = ASM_KW.match(text, i)
            if m and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
                return m
        i += 1
    return None

def transform(text: str) -> tuple[str, list[str]]:
    out, problems = [], []
    i, n = 0, len(text)
    while True:
        m = _find_asm(text, i)
        if not m:
            out.append(text[i:])
            break
        line_start = text.rfind("\n", 0, m.start()) + 1
        if text[line_start:m.start()].lstrip().startswith("#"):
            out.append(text[i:m.end()])
            i = m.end()
            continue
        j = m.end()
        while True:
            while j < n and text[j] in " \t\n":
                j += 1
            q = re.match(r"(volatile|__volatile__|goto|inline)\b", text[j:j + 16])
            if not q:
                break
            j += q.end()
        if j >= n or text[j] != "(":
            out.append(text[i:m.end()])
            i = m.end()
            continue
        end = match_paren(text, j)
        inner = text[j + 1:end - 1]
        sections = split_top(inner, ":")
        tmpl = string_value(sections[0])
        original = text[m.start():end]
        lineno = text.count("\n", 0, m.start()) + 1
        prev = text[:m.start()].rstrip()[-1:] if m.start() else ""
        is_stmt = bool(re.match(r"\s*(volatile|__volatile__)", text[m.end():m.end() + 20])) \
            or prev in (";", "{", "}", ":") or tmpl.strip().split(" ")[0] in STMT_MNEMONICS
        if len(sections) == 1 and tmpl.strip().startswith("$") and not is_stmt:
            repl = ""
        elif len(sections) == 1 and IDENT.match(tmpl.strip()) and not is_stmt:
            repl = original
        elif len(sections) == 1 and all(not l.strip() or l.strip().startswith(".")
                                        for l in re.split(r"[\n;]", tmpl)):
            repl = ""
        else:
            try:
                outs = parse_operands(sections[1]) if len(sections) > 1 else []
                ins = parse_operands(sections[2]) if len(sections) > 2 else []
                repl = translate_block(tmpl, outs, ins)
            except (Untranslatable, IndexError, ValueError) as exc:
                problems.append(f"line {lineno}: {exc}")
                repl = original
        repl += "\n" * original.count("\n")
        out.append(text[i:m.start()])
        out.append(repl)
        i = end
    return "".join(out), problems

def _inline_header(name: str) -> str:
    return "".join(l for l in (HOST_DIR / name).read_text().splitlines(True)
                   if not l.startswith("#include"))

def prelude() -> str:
    return (
        "/* hostify prelude */\n"
        "typedef unsigned long __revo_uptr;\n"
        + _inline_header("mmi_ops.h")
        + """
static inline unsigned __revo_f2i_bits(float f) { union { float f; unsigned u; } x; x.f = f; return x.u; }
static inline float __revo_i2f_bits(unsigned u) { union { float f; unsigned u; } x; x.u = u; return x.f; }
static inline float __revo_fmax(float a, float b) { return a > b ? a : b; }
static inline float __revo_fmin(float a, float b) { return a < b ? a : b; }
static inline int __revo_trunc(float f) { if (f >= 2147483648.0f) return 0x7FFFFFFF; if (f <= -2147483648.0f) return (int)0x80000000; return (int)f; }
static inline RevoQ __revo_toq(unsigned __int128 v) { RevoQ q; q.lo = (unsigned long long)v; q.hi = (unsigned long long)(v >> 64); return q; }
static inline unsigned __int128 __revo_fromq(RevoQ q) { return ((unsigned __int128)q.hi << 64) | q.lo; }
# 1 "<hostified>"
""")

def hostify_text(text: str) -> tuple[str, list[str]]:
    new, problems = transform(text)
    return prelude() + new, problems
