#!/usr/bin/env python3
# GAME.BF's files, as GAME.DSC lists them: (GAME(dir[name,offset,size,flag]...)...),
# offsets from GAME.BF's start, aligned to 2048 bytes, no compression.
# `unpack` writes every file to the assets' GAME folder, which the game then
# reads first (host/disc.c); a file the folder lacks comes from GAME.BF.
#
#   bigfile.py list <assets> [pattern]
#   bigfile.py unpack <assets>
#   bigfile.py extract <assets> <out> [pattern]

import fnmatch
import sys
from pathlib import Path

USAGE = """usage: bigfile.py list <assets> [pattern]
       bigfile.py unpack <assets>
       bigfile.py extract <assets> <out> [pattern]"""

def entries(dsc: str):
    out, stack, i = [], [], 0
    while i < len(dsc):
        c = dsc[i]
        if c == '(':
            j = i + 1
            while j < len(dsc) and dsc[j] not in '([)':
                j += 1
            stack.append(dsc[i + 1:j])
            i = j
        elif c == ')':
            stack.pop()
            i += 1
        elif c == '[':
            j = dsc.index(']', i)
            name, offset, size, flag = dsc[i + 1:j].rsplit(',', 3)
            out.append(('/'.join(stack[1:] + [name]), int(offset), int(size), int(flag)))
            i = j + 1
        else:
            i += 1
    return out

def main(argv):
    if len(argv) < 3 or argv[1] not in ('list', 'extract', 'unpack'):
        print(USAGE)
        return 1
    assets = Path(argv[2])
    if argv[1] == 'unpack':
        if (assets / 'GAME').exists():
            sys.exit(f'{assets / "GAME"} is there already')
        argv = [argv[0], 'extract', argv[2], str(assets / 'GAME')]
    files = entries((assets / 'GAME.DSC').read_text(errors='replace'))
    pattern = argv[4] if argv[1] == 'extract' and len(argv) > 4 else argv[3] if argv[1] == 'list' and len(argv) > 3 else '*'
    chosen = [f for f in files if fnmatch.fnmatch(f[0].lower(), pattern.lower())]
    if argv[1] == 'list':
        for path, offset, size, flag in chosen:
            print(f'{path}\t{size}')
        print(f'{len(chosen)} of {len(files)} files', file=sys.stderr)
        return 0
    if len(argv) < 4:
        print(USAGE)
        return 1
    out = Path(argv[3])
    with open(assets / 'GAME.BF', 'rb') as bf:
        for path, offset, size, flag in chosen:
            bf.seek(offset)
            target = out / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(bf.read(size))
    print(f'{len(chosen)} files to {out}', file=sys.stderr)
    if out.resolve() == (assets / 'GAME').resolve():
        print('the game now reads GAME.BF from that folder: GAME.BF and GAME.DSC may be deleted', file=sys.stderr)
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
