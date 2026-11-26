#include <stdlib.h>
#include <string.h>

#include "texels.h"

#define TA0 0x00u
#define TA1 0x80u

enum { PSMCT32 = 0x00, PSMCT24 = 0x01, PSMCT16 = 0x02, PSMCT16S = 0x0A,
       PSMT8 = 0x13, PSMT4 = 0x14, PSMT8H = 0x1B, PSMT4HL = 0x24, PSMT4HH = 0x2C };

static int unit_bits(int psm) {
    switch (psm) {
    case PSMCT32: return 32;
    case PSMCT24: return 24;
    case PSMCT16: case PSMCT16S: return 16;
    case PSMT8: case PSMT8H: return 8;
    default: return 4;
    }
}

static u32 colour16(u32 c) {
    u32 r = (c & 31) << 3, g = (c >> 5 & 31) << 3, b = (c >> 10 & 31) << 3;
    return r | g << 8 | b << 16 | (c & 0x8000 ? TA1 : TA0) << 24;
}

typedef struct Upload {
    const Ps2Raster *r;
    u32 row_bytes;
    u32 rrw;
    int bits;
} Upload;

static u32 clut(const Ps2Raster *r, u32 index, bool four_bits) {
    u64 tex0 = r->tex0;
    int cpsm = (int)(tex0 >> 51) & 15, csa = (int)(tex0 >> 56) & 31;
    u32 width = four_bits ? 8 : 16, height = four_bits ? 2 : 16;
    u32 i = four_bits ? index + (u32)csa * 16 : index;
    u32 s = (i & ~0x18u) | (i & 0x08) << 1 | (i & 0x10) >> 1;
    u32 x = s & 15, y = s >> 4;
    if (!r->palette || x >= width || y >= height) return 0;
    u32 n = y * width + x, size = cpsm == PSMCT32 ? 4 : 2;
    if ((n + 1) * size > r->palette_size) return 0;
    if (size == 4) { u32 c; memcpy(&c, r->palette + n * 4, 4); return c; }
    u16 c;
    memcpy(&c, r->palette + n * 2, 2);
    return colour16(c);
}

void texels_decode(const Ps2Raster *r, u32 *out) {
    int psm = (int)(r->tex0 >> 20) & 0x3F, tw = tex0_width(r->tex0), th = tex0_height(r->tex0);
    Upload u = {r, 0, 0, unit_bits(psm)};
    u32 qwords = (u32)(((r->width * r->depth >> 3) + 15) >> 4);
    u.row_bytes = qwords * 16;
    u.rrw = qwords * 128 / (u32)r->depth;
    const u32 xs = u.rrw < (u32)tw ? u.rrw : (u32)tw;
    const int ys = r->height < th ? r->height : th;
    memset(out, 0, sizeof *out * (size_t)tw * (size_t)th);
    if (xs == 0 || ys <= 0) return;

    const bool indexed = psm == PSMT8 || psm == PSMT8H || psm == PSMT4 || psm == PSMT4HL || psm == PSMT4HH;
    const bool four = psm == PSMT4 || psm == PSMT4HL || psm == PSMT4HH;
    u32 lut[256];
    if (indexed)
        for (u32 i = 0; i < (four ? 16u : 256u); i++) lut[i] = clut(r, i, four);

    const u64 last_bit = ((u64)(ys - 1) * u.rrw + (xs - 1)) * (u64)u.bits;
    const u64 n = (last_bit >> 3) + 5;
    u8 *stream = malloc((size_t)n + 8);
    if (!stream) return;
    for (u64 k = 0; k < n;) {
        const u64 row = k / u.row_bytes, in = k % u.row_bytes;
        u64 len = u.row_bytes - in;
        if (len > n - k) len = n - k;
        const u64 at = row * r->stride + in;
        u64 have = at < r->size ? r->size - at : 0;
        if (have > len) have = len;
        if (have) memcpy(stream + k, r->pixels + at, (size_t)have);
        memset(stream + k + have, 0, (size_t)(len - have));
        k += len;
    }
    memset(stream + n, 0, 8);

    const u64 mask = u.bits == 32 ? 0xFFFFFFFFull : (1ull << u.bits) - 1;
    for (int y = 0; y < ys; y++) {
        u32 *line = out + (size_t)y * (size_t)tw;
        u64 bit = (u64)y * u.rrw * (u64)u.bits;
        for (u32 x = 0; x < xs; x++, bit += (u64)u.bits) {
            u64 v;
            memcpy(&v, stream + (bit >> 3), 8);
            const u32 t = (u32)((v >> (bit & 7)) & mask);
            u32 c = 0;
            switch (psm) {
            case PSMCT32: c = t; break;
            case PSMCT24: c = (t & 0xFFFFFF) | TA0 << 24; break;
            case PSMCT16: case PSMCT16S: c = colour16(t); break;
            default: if (indexed) c = lut[t]; break;
            }
            line[x] = c;
        }
    }
    free(stream);
}
