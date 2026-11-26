#ifndef GLI_TEXELS_H
#define GLI_TEXELS_H

#include "guest.h"

typedef struct Ps2Raster {
    const u8 *pixels;
    u32 size;
    u32 stride;
    int width, height, depth;
    const u8 *palette;
    u32 palette_size;
    u64 tex0;
} Ps2Raster;

static inline int tex0_width(u64 tex0) { return 1 << ((tex0 >> 26) & 15); }
static inline int tex0_height(u64 tex0) { return 1 << ((tex0 >> 30) & 15); }

void texels_decode(const Ps2Raster *r, u32 *out);

#endif
