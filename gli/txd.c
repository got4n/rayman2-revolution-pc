#include "game_vars.h"
#include <stdlib.h>
#include <string.h>

#include "rwstream.h"
#include "txd.h"

#define PLATFORM_PS2 0x00325350u

static bool raster(Reader *r, TxdTexture *t) {
    Reader ext;
    if (!chunk(r, CHUNK_STRUCT, &ext) || ext.end - ext.at < 0x40) return false;
    const u8 *p = r->bytes + ext.at;
    t->width = (int)read_u32_at(p + 0x00);
    t->height = (int)read_u32_at(p + 0x04);
    t->depth = (int)read_u32_at(p + 0x08);
    t->raster_format = read_u32_at(p + 0x0C) & 0xFFFF;
    t->tex0 = read_u64_at(p + 0x10);
    t->tex1 = read_u32_at(p + 0x1C);
    t->miptbp1 = read_u64_at(p + 0x20);
    t->miptbp2 = read_u64_at(p + 0x28);
    t->pixels_size = read_u32_at(p + 0x30);
    t->palette_size = read_u32_at(p + 0x34);
    t->mipmap_kl = read_u32_at(p + 0x3C);
    Reader data;
    if (!chunk(r, CHUNK_STRUCT, &data)) return false;
    if (t->pixels_size + t->palette_size > data.end - data.at) return false;
    t->pixels = r->bytes + data.at;
    t->palette = t->palette_size ? t->pixels + t->pixels_size : NULL;
    return t->width > 0 && t->height > 0 && t->depth > 0;
}

static bool texture(Reader *r, TxdTexture *t) {
    Reader body, platform, ras;
    if (!chunk(r, CHUNK_TEXTURE_NATIVE, &body)) return false;
    if (!chunk(&body, CHUNK_STRUCT, &platform) || platform.end - platform.at < 8) return false;
    if (read_u32_at(r->bytes + platform.at) != PLATFORM_PS2) return false;
    memset(t, 0, sizeof *t);
    t->filter_addressing = read_u32_at(r->bytes + platform.at + 4);
    if (!string_chunk(&body, t->name) || !string_chunk(&body, t->mask)) return false;
    return chunk(&body, CHUNK_STRUCT, &ras) && raster(&ras, t);
}

int txd_read(const u8 *bytes, u32 length, TxdTexture **textures) {
    *textures = NULL;
    Reader file = {bytes, 0, length}, dict, head;
    if (!find_chunk(&file, CHUNK_TEXTURE_DICTIONARY, &dict)) return TXD_NONE;
    if (!chunk(&dict, CHUNK_STRUCT, &head) || head.end - head.at < 4) return TXD_DAMAGED;
    u32 count = read_u32_at(bytes + head.at);
    if (count > length / 12) return TXD_DAMAGED;
    TxdTexture *t = calloc(count ? count : 1, sizeof *t);
    for (u32 i = 0; i < count; i++)
        if (!texture(&dict, &t[i])) { free(t); return TXD_DAMAGED; }
    *textures = t;
    return (int)count;
}
