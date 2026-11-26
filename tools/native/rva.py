#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import target

SYM_LONG = target.SYM_LONG

def rva(text: str) -> str:
    return target.rva(text)

if __name__ == "__main__":
    for f in sys.argv[1:]:
        p = Path(f)
        p.write_text(rva(p.read_text(encoding="utf-8")), encoding="utf-8")
