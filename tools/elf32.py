from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path

@dataclass
class Section:
    name: str
    type: int
    flags: int
    addr: int
    offset: int
    size: int

@dataclass
class Segment:
    type: int
    offset: int
    vaddr: int
    filesz: int
    memsz: int

@dataclass
class Symbol:
    name: str
    value: int
    size: int
    bind: int
    type: int
    shndx: int

class Elf32:
    def __init__(self, path: Path | str):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        d = self.data
        if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 1:
            raise ValueError(f"{path}: not a little-endian ELF32 file")
        (self.e_type, self.e_machine, _, self.entry, phoff, shoff, _, _, phentsize,
         phnum, shentsize, shnum, shstrndx) = struct.unpack_from("<HHIIIIIHHHHHH", d, 16)
        self.segments = []
        for i in range(phnum):
            p_type, p_off, p_vaddr, _, p_filesz, p_memsz, _, _ = struct.unpack_from(
                "<IIIIIIII", d, phoff + i * phentsize)
            self.segments.append(Segment(p_type, p_off, p_vaddr, p_filesz, p_memsz))
        raw = []
        for i in range(shnum):
            raw.append(struct.unpack_from("<IIIIIIIIII", d, shoff + i * shentsize))
        names_off = raw[shstrndx][4] if shnum else 0
        self.sections = []
        self._raw_sections = raw
        for r in raw:
            self.sections.append(Section(self._cstr(names_off + r[0]), r[1], r[2], r[3], r[4], r[5]))

    def _cstr(self, off: int) -> str:
        end = self.data.index(b"\0", off)
        return self.data[off:end].decode("latin-1")

    def section(self, name: str) -> Section | None:
        for s in self.sections:
            if s.name == name:
                return s
        return None

    def symbols(self) -> list[Symbol]:
        out = []
        for idx, r in enumerate(self._raw_sections):
            if r[1] != 2:
                continue
            off, size, link, entsize = r[4], r[5], r[6], r[9] or 16
            stroff = self._raw_sections[link][4]
            for i in range(1, size // entsize):
                st_name, st_value, st_size, st_info, _, st_shndx = struct.unpack_from(
                    "<IIIBBH", self.data, off + i * entsize)
                out.append(Symbol(self._cstr(stroff + st_name), st_value, st_size,
                                  st_info >> 4, st_info & 15, st_shndx))
        return out

    def load_image(self) -> list[tuple[int, bytes, int]]:
        return [(s.vaddr, self.data[s.offset:s.offset + s.filesz], s.memsz)
                for s in self.segments if s.type == 1]

    def read(self, addr: int, size: int) -> bytes:
        for vaddr, blob, _ in self.load_image():
            if vaddr <= addr and addr + size <= vaddr + len(blob):
                return blob[addr - vaddr: addr - vaddr + size]
        raise KeyError(f"address 0x{addr:08X} not in file image")
