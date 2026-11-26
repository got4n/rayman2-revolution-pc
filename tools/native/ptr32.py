#!/usr/bin/env python3
from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import target

import clang.cindex as ci

ARGS = [f"--target={target.TRIPLE}", "-fms-extensions", "-ffreestanding", "-w",
        "-Wno-everything"]
K = ci.CursorKind
DECLS = {K.VAR_DECL, K.FIELD_DECL, K.PARM_DECL, K.FUNCTION_DECL, K.TYPEDEF_DECL}
TYPE_CONTEXTS = DECLS | {K.CSTYLE_CAST_EXPR, K.CXX_UNARY_EXPR, K.COMPOUND_LITERAL_EXPR, K.TYPE_REF,
                         K.STRUCT_DECL, K.UNION_DECL}
EXPR_KINDS = {K.BINARY_OPERATOR, K.UNARY_OPERATOR, K.COMPOUND_ASSIGNMENT_OPERATOR}
TYPE_WORDS = {"void", "char", "short", "int", "long", "float", "double", "signed", "unsigned", "const",
              "volatile", "struct", "union", "enum", "_Bool", "__restrict", "restrict"}
ATTR = " __ptr32 __uptr "

def parse(path: Path, cxx: bool = False) -> ci.TranslationUnit:
    lang = ["-x", "c++", "-fno-exceptions", "-fno-rtti"] if cxx else ["-x", "c"]
    return ci.Index.create().parse(str(path), args=lang + ARGS, options=0)

def type_stars(tu: ci.TranslationUnit, text: bytes) -> list[tuple[int, ci.Cursor]]:
    toks = list(tu.get_tokens(extent=tu.cursor.extent))
    out = []
    for i, t in enumerate(toks):
        if t.spelling != "*" or t.kind != ci.TokenKind.PUNCTUATION:
            continue
        cur = ci.Cursor.from_location(tu, t.location)
        if cur is None or cur.kind in EXPR_KINDS:
            continue
        if cur.kind not in TYPE_CONTEXTS:
            continue
        if cur.kind in (K.CXX_UNARY_EXPR, K.CSTYLE_CAST_EXPR, K.COMPOUND_LITERAL_EXPR):
            prev = toks[i - 1]
            nxt = toks[i + 1].spelling if i + 1 < len(toks) else ""
            if not (prev.spelling in TYPE_WORDS or prev.spelling == "*" or prev.kind == ci.TokenKind.IDENTIFIER
                    or (prev.spelling == "(" and nxt in (")", "*"))):
                continue
            if nxt not in (")", "*", "const", "volatile", "(", "__restrict", "restrict"):
                continue
        out.append((t.extent.end.offset, cur))
    return out

def outermost(stars: list[tuple[int, ci.Cursor]]) -> set[int]:
    by_decl: dict[tuple, list[int]] = {}
    decl_of: dict[tuple, ci.Cursor] = {}
    for off, cur in stars:
        if cur.kind not in (K.PARM_DECL, K.FUNCTION_DECL):
            continue
        key = (cur.kind, cur.extent.start.offset, cur.extent.end.offset)
        by_decl.setdefault(key, []).append(off)
        decl_of[key] = cur
    keep = set()
    for key, offs in by_decl.items():
        cur = decl_of[key]
        name_at = cur.location.offset if cur.spelling else cur.extent.end.offset
        before = [o for o in offs if o <= name_at]
        if cur.kind == K.PARM_DECL and cur.type.kind == ci.TypeKind.INCOMPLETEARRAY:
            continue
        if before:
            keep.add(max(before))
    return keep

def address_taken_params(tu: ci.TranslationUnit) -> list[tuple[ci.Cursor, ci.Cursor]]:
    out = []
    for fn in tu.cursor.walk_preorder():
        if fn.kind != K.FUNCTION_DECL or not fn.is_definition():
            continue
        params = [p for p in fn.get_children() if p.kind == K.PARM_DECL and p.spelling
                  and p.type.get_canonical().kind == ci.TypeKind.POINTER]
        body = next((c for c in fn.get_children() if c.kind == K.COMPOUND_STMT), None)
        if not params or body is None:
            continue
        taken = set()
        for c in body.walk_preorder():
            if c.kind != K.UNARY_OPERATOR:
                continue
            toks = list(c.get_tokens())
            if not toks or toks[0].spelling != "&":
                continue
            ref = next(c.get_children(), None)
            while ref is not None and ref.kind == K.UNEXPOSED_EXPR:
                ref = next(ref.get_children(), None)
            if ref is not None and ref.kind == K.DECL_REF_EXPR and ref.referenced is not None \
                    and ref.referenced.kind == K.PARM_DECL:
                taken.add(ref.referenced.spelling)
        out += [(p, body) for p in params if p.spelling in taken]
    return out

UNSIGNED = {ci.TypeKind.UCHAR, ci.TypeKind.CHAR_U, ci.TypeKind.USHORT, ci.TypeKind.BOOL}
SIGNED = {ci.TypeKind.SCHAR, ci.TypeKind.CHAR_S, ci.TypeKind.SHORT}

def narrow(t) -> str:
    k = t.get_canonical().kind
    return "zeroext" if k in UNSIGNED else "signext" if k in SIGNED else ""

def extensions(tu: ci.TranslationUnit) -> dict[str, list[str]]:
    out: dict[str, list[str]] = {}
    for c in tu.cursor.get_children():
        if c.kind != K.FUNCTION_DECL:
            continue
        ext = [narrow(c.result_type)] + [narrow(a.type) for a in c.get_arguments()]
        if any(ext):
            out[target.c_name(c.mangled_name) if c.mangled_name else c.spelling] = ext
    return out

LONG_LITERAL = re.compile(rb"^((?:0[xX][0-9A-Fa-f]+|0[bB][01]+|\d+))([uU]?)[lL]([uU]?)$")

def ilp32_longs(tu: ci.TranslationUnit) -> list[tuple[int, int, bytes]]:
    toks = list(tu.get_tokens(extent=tu.cursor.extent))
    edits = []
    for i, t in enumerate(toks):
        a, b = t.extent.start.offset, t.extent.end.offset
        if t.kind == ci.TokenKind.LITERAL:
            m = LONG_LITERAL.match(t.spelling.encode())
            if m:
                edits.append((a, b, m.group(1) + m.group(2) + m.group(3)))
            continue
        if t.kind != ci.TokenKind.KEYWORD or t.spelling != "long":
            continue
        prev = toks[i - 1].spelling if i else ""
        nxt = toks[i + 1].spelling if i + 1 < len(toks) else ""
        if prev == "long" or nxt in ("long", "double"):
            continue
        near = set()
        for step in (-1, 1):
            j = i + step
            while 0 <= j < len(toks) and toks[j].spelling in SPECIFIERS:
                near.add(toks[j].spelling)
                j += step
        edits.append((a, b, b"" if "int" in near else b"int"))
    return edits

ELF_SECTION = re.compile(rb'__attribute__\s*\(\(\s*section\s*\(\s*"[^",]*"\s*\)\s*\)\)')
ASM_LABEL = re.compile(rb'((?<=[\w)\]])\s*(?:__asm__|__asm|asm)\s*\(\s*)"([A-Za-z0-9_$][\w$.]*)"\s*\)')
SPECIFIERS = {"unsigned", "signed", "int", "const", "volatile", "static", "extern", "register", "inline",
              "typedef", "__restrict", "restrict", "__inline", "__inline__", "_Thread_local", "__extension__"}

def ilp32_text(decl: bytes) -> bytes:
    if not target.LP64:
        return decl
    out = re.sub(rb"\blong\s+long\b", b"\0LL\0", decl)
    out = re.sub(rb"\blong(\s+(?:unsigned\s+|signed\s+)?int)\b", rb"\1", out)
    out = re.sub(rb"\blong\b(?!\s+double\b)", b"int", out)
    return out.replace(b"\0LL\0", b"long long")

def rewrite(src: Path, dst: Path, cxx: bool = False) -> dict:
    text = src.read_bytes()
    tu = parse(src, cxx)
    stars = type_stars(tu, text)
    keep = outermost(stars)
    edits: list[tuple[int, int, bytes]] = []
    for off, _ in stars:
        if off not in keep:
            edits.append((off, off, ATTR.encode()))
    if target.LP64:
        edits += ilp32_longs(tu)
    copies = address_taken_params(tu)
    for p, body in copies:
        name = p.spelling.encode()
        start, end = p.extent.start.offset, p.extent.end.offset
        at = p.location.offset
        edits.append((at, at + len(name), b"__revo_arg_" + name))
        decl = text[start:end]
        rel = at - start
        decl = decl[:rel] + name + decl[rel + len(name):]
        decl = ilp32_text(decl).replace(b"*", b"*" + ATTR.encode())
        brace = body.extent.start.offset + 1
        edits.append((brace, brace, b" " + decl + b" = __revo_arg_" + name + b";"))
    edits.sort(key=lambda e: (e[0], e[1]))
    pieces, last = [], 0
    for start, end, rep in edits:
        pieces.append(text[last:start])
        pieces.append(rep)
        last = end
    pieces.append(text[last:])
    out = b"".join(pieces)
    if target.MAC:
        out = ELF_SECTION.sub(b"", out)
        out = ASM_LABEL.sub(lambda m: m.group(1) + b'"' + target.PREFIX.encode() + m.group(2) + b'")', out)
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_bytes(out)
    errors = [d.spelling for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
    return {"stars": len(stars), "ptr32": len(stars) - len(keep), "kept_8_bytes": len(keep),
            "param_copies": len(copies), "parse_errors": errors[:5], "extensions": extensions(tu)}

if __name__ == "__main__":
    print(rewrite(Path(sys.argv[1]), Path(sys.argv[2])))
