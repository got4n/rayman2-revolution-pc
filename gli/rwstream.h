#ifndef GLI_RWSTREAM_H
#define GLI_RWSTREAM_H

#include <string.h>

#include "guest.h"

enum {
    CHUNK_STRUCT = 0x01,
    CHUNK_STRING = 0x02,
    CHUNK_EXTENSION = 0x03,
    CHUNK_TEXTURE = 0x06,
    CHUNK_MATERIAL = 0x07,
    CHUNK_MATERIAL_LIST = 0x08,
    CHUNK_FRAME_LIST = 0x0E,
    CHUNK_GEOMETRY = 0x0F,
    CHUNK_CLUMP = 0x10,
    CHUNK_ATOMIC = 0x14,
    CHUNK_TEXTURE_NATIVE = 0x15,
    CHUNK_TEXTURE_DICTIONARY = 0x16,
    CHUNK_GEOMETRY_LIST = 0x1A,
    CHUNK_BIN_MESH = 0x50E,
};

typedef struct Reader {
    const u8 *bytes;
    u32 at, end;
} Reader;

static inline u32 read_u32_at(const u8 *p) { u32 v; memcpy(&v, p, 4); return v; }
static inline u64 read_u64_at(const u8 *p) { u64 v; memcpy(&v, p, 8); return v; }

static inline bool chunk(Reader *r, u32 type, Reader *body) {
    if (r->end - r->at < 12) return false;
    u32 t = read_u32_at(r->bytes + r->at), size = read_u32_at(r->bytes + r->at + 4);
    if (t != type || size > r->end - r->at - 12) return false;
    *body = (Reader){r->bytes, r->at + 12, r->at + 12 + size};
    r->at = body->end;
    return true;
}

static inline bool find_chunk(Reader *r, u32 type, Reader *body) {
    while (!chunk(r, type, body)) {
        if (r->end - r->at < 12) return false;
        u32 size = read_u32_at(r->bytes + r->at + 4);
        if (size > r->end - r->at - 12) return false;
        r->at += 12 + size;
    }
    return true;
}

static inline const u8 *take(Reader *r, u32 n) {
    if (r->end - r->at < n) return NULL;
    const u8 *p = r->bytes + r->at;
    r->at += n;
    return p;
}

static inline bool take_u32(Reader *r, u32 *v) {
    const u8 *p = take(r, 4);
    if (p) *v = read_u32_at(p);
    return p != NULL;
}

static inline bool string_chunk(Reader *r, char out[32]) {
    Reader s;
    if (!chunk(r, CHUNK_STRING, &s)) return false;
    u32 n = s.end - s.at < 31 ? s.end - s.at : 31;
    memcpy(out, s.bytes + s.at, n);
    out[n] = 0;
    return true;
}

#endif
