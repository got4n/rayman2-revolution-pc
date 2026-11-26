"""The build's steps keep what each unit gave, by content: a unit is built again only when its input or the
tools that build it change, so that a change to the PC side does not rebuild the game's C."""
from __future__ import annotations

import functools
import hashlib
import json
import shutil
import subprocess
from pathlib import Path


def digest(*parts) -> str:
    """The SHA-1 of the parts: bytes, str, or a Path's contents (its name alone when it is missing)."""
    h = hashlib.sha1()
    for p in parts:
        if isinstance(p, Path):
            h.update(str(p.name).encode())
            h.update(p.read_bytes() if p.exists() else b"(missing)")
        elif isinstance(p, bytes):
            h.update(p)
        else:
            h.update(str(p).encode("utf-8", "surrogateescape"))
        h.update(b"\0")
    return h.hexdigest()


@functools.lru_cache(maxsize=None)
def compiler(cmd: str) -> str:
    """The compiler's --version: another LLVM builds everything again."""
    try:
        return subprocess.run([cmd, "--version"], capture_output=True, text=True).stdout
    except OSError:
        return cmd


class Cache:
    """One JSON per entry in dir: the key it was made with, the step's result, and copies of files that later
    builds of other entries overwrite."""

    def __init__(self, dir: Path):
        self.dir = dir
        dir.mkdir(parents=True, exist_ok=True)

    def get(self, name: str, key: str, outputs: list[Path] = (), kept: dict[str, Path] = {}) -> dict | None:
        """The result kept for name when it was made with key, its outputs are all there and its kept files are
        copied back in place; else None."""
        meta = self.dir / f"{name}.json"
        try:
            entry = json.loads(meta.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            return None
        if entry.get("key") != key or not all(p.exists() for p in outputs):
            return None
        for tag, dst in kept.items():
            src = self.dir / f"{name}.{tag}"
            if not src.exists():
                return None
            shutil.copyfile(src, dst)
        return entry["result"]

    def put(self, name: str, key: str, result: dict, kept: dict[str, Path] = {}) -> None:
        for tag, src in kept.items():
            if src.exists():
                shutil.copyfile(src, self.dir / f"{name}.{tag}")
        (self.dir / f"{name}.json").write_text(json.dumps({"key": key, "result": result}), encoding="utf-8")
