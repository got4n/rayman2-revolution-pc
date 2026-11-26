from __future__ import annotations

import re

KIND = {"i1": ["i32"], "i8": ["i32"], "i16": ["i32"], "i32": ["i32"], "ptr": ["i32"], "i64": ["i64"],
        "i128": ["i64", "i64"], "float": ["f32"], "double": ["f64"]}
FUNC = re.compile(r"^(define|declare)\s+(?P<pre>[^@]*?)\s*@(?P<name>\"[^\"]+\"|[\w.$]+)\((?P<params>.*)\)", re.M)
CALL = re.compile(r"\bcall\s+(?:[\w()]+\s+)*?(?P<ret>void|i\d+|ptr|float|double)\s+@(?P<name>[\w.$]+)"
                  r"\((?P<args>.*?)\)(?:\s+#\d+)?\s*(?:,|$)", re.M)

def _split(params: str) -> list[str]:
    out, depth, cur = [], 0, ""
    for ch in params:
        depth += (ch in "([{<") - (ch in ")]}>")
        if ch == "," and not depth:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out

def _kinds(ty: str) -> list[str]:
    if ty not in KIND:
        raise ValueError(f"no register class for {ty}")
    return KIND[ty]

def _type(ret: str, params: list[str]) -> tuple:
    args = [k for p in params for k in (["i32"] if p == "..." else _kinds(p.split()[0]))]
    return tuple(args), () if ret == "void" else tuple(_kinds(ret))

# Every function the IR defines or uses, with the register classes it takes and
# returns. A function without a prototype takes the types of its first call.
def functions(ir: str) -> dict[str, tuple[bool, tuple]]:
    body = "\n".join(l for l in ir.splitlines() if not l.startswith(("define", "declare")))
    used = set(re.findall(r"@([\w.$]+)", body))
    out = {}
    for m in FUNC.finditer(ir):
        pre = m["pre"].split()
        name = m["name"].strip('"')
        defined = m[1] == "define"
        if name.startswith("llvm.") or "internal" in pre or "private" in pre or not defined and name not in used:
            continue
        ret, params = pre[-1], _split(m["params"])
        if not defined and params == ["..."]:
            call = next((c for c in CALL.finditer(body) if c["name"] == name), None)
            if call:
                ret, params = call["ret"], _split(call["args"])
        out[name] = (defined, _type(ret, params))
    return out
