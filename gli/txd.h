#ifndef GLI_TXD_H
#define GLI_TXD_H

#include "guest.h"

typedef struct TxdTexture {
    char name[32], mask[32];
    u32 filter_addressing;
    int width, height;
    int depth;
    u32 raster_format;
    u64 tex0;
    u32 tex1;
    u64 miptbp1, miptbp2;
    u32 mipmap_kl;
    const u8 *pixels;
    u32 pixels_size;
    const u8 *palette;
    u32 palette_size;
} TxdTexture;

enum { TXD_NONE = -1, TXD_DAMAGED = -2 };
int txd_read(const u8 *bytes, u32 length, TxdTexture **textures);

#endif
