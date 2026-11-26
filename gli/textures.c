#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "gli.h"
#include "handles.h"
#include "render.h"
#include "texels.h"
#include "txd.h"

static const CallerFrame FRAME = {0x150, 0x10};

#define LEVEL_HEAP_MARGIN 0xCCCCCu

static u32 g_level_heap;

#define DICTIONARIES 6

typedef struct Dictionary {
    u8 *bytes;
    TxdTexture *files;
    Texture *textures;
    int count;
} Dictionary;

static Dictionary g_dictionaries[DICTIONARIES];

static void dictionary_free(Dictionary *d) {
    for (int i = 0; i < d->count; i++) {
        handle_free(d->textures[i].handle);
        handle_free(d->textures[i].raster);
        render_texture_free(d->textures[i].image);
        render_texture_free(d->textures[i].lightmap_image);
    }
    free(d->bytes);
    free(d->files);
    free(d->textures);
    memset(d, 0, sizeof *d);
}

void textures_texels(const TxdTexture *t, u32 *out) {
    Ps2Raster r = {.pixels = t->pixels, .size = t->pixels_size, .stride = (u32)(t->width * t->depth / 8),
                   .width = t->width, .height = t->height, .depth = t->depth,
                   .palette = t->palette, .palette_size = t->palette_size, .tex0 = t->tex0};
    texels_decode(&r, out);
}

static u32 texture_image(const TxdTexture *t) {
    int width = tex0_width(t->tex0), height = tex0_height(t->tex0);
    u32 *texels = malloc(sizeof *texels * (size_t)width * (size_t)height);
    textures_texels(t, texels);
    u32 image = render_texture_new(width, height, texels);
    free(texels);
    return image;
}

static Texture texture_from(const TxdTexture *file) {
    u32 fa = file->filter_addressing;
    Texture t = {.file = file, .image = texture_image(file), .filter = (u8)(fa & 0xFF),
                 .address_u = (u8)(fa >> 8 & 15), .address_v = (u8)(fa >> 12 & 15)};
    if (!t.address_v) t.address_v = t.address_u;
    return t;
}

static int dictionary_read(Dictionary *d, u8 *bytes, u32 length) {
    memset(d, 0, sizeof *d);
    TxdTexture *files;
    int count = txd_read(bytes, length, &files);
    if (count < 0) {
        free(bytes);
        return count;
    }
    d->bytes = bytes;
    d->files = files;
    d->count = count;
    d->textures = calloc((size_t)count + 1, sizeof *d->textures);
    for (int i = 0; i < count; i++) {
        d->textures[i] = texture_from(&files[i]);
        d->textures[i].handle = handle_new(OBJECT_TEXTURE, &d->textures[i]);
    }
    return count;
}

int textures_count(void) {
    int n = 0;
    for (int k = 0; k < DICTIONARIES; k++)
        for (int i = 0; i < g_dictionaries[k].count; i++) n += g_dictionaries[k].textures[i].handle != 0;
    return n;
}

Texture *textures_find(const char *name) {
    for (int k = 0; k < DICTIONARIES; k++)
        for (int i = 0; i < g_dictionaries[k].count; i++)
            if (g_dictionaries[k].textures[i].handle && !strcmp(g_dictionaries[k].files[i].name, name))
                return &g_dictionaries[k].textures[i];
    return NULL;
}

static u32 lightmap_image(const TxdTexture *t) {
    TxdTexture through = *t;
    if (t->depth == 32) {
        u64 tex0 = t->tex0;
        u32 tbw = (u32)(tex0 >> 14) & 0x3F, tw = (u32)(tex0 >> 26) & 15;
        tex0 &= ~(0x3Full << 14 | 0x3Full << 20 | 15ull << 26);
        through.tex0 = tex0 | (u64)(tbw * 2) << 14 | (u64)0x13 << 20 | (u64)(tw + 2) << 26;
        through.width = t->width * 4;
        through.depth = 8;
    } else if (t->depth != 8) {
        revo_log("[gli] lightmap %s: %d bits per texel, read through its own palette", t->name, t->depth);
        return 0;
    }
    u32 ramp[256], *p = ramp, alpha = 0;
    do {
        for (int i = 0; i < 8; i++) *p++ = alpha++ << 24 | 0xFFFFFFu;
        p += 8;
        for (int i = 0; i < 8; i++) *p++ = alpha++ << 24 | 0xFFFFFFu;
        p -= 16;
        for (int i = 0; i < 8; i++) *p++ = alpha++ << 24 | 0xFFFFFFu;
        p += 8;
        for (int i = 0; i < 8; i++) *p++ = alpha++ << 24 | 0xFFFFFFu;
    } while (alpha < 0x100);
    through.palette = (const u8 *)ramp;
    through.palette_size = sizeof ramp;
    int width = tex0_width(through.tex0), height = tex0_height(through.tex0);
    u32 *texels = malloc(sizeof *texels * (size_t)width * (size_t)height);
    textures_texels(&through, texels);
    u32 image = render_texture_new(width, height, texels);
    free(texels);
    return image;
}

Texture *textures_take(const char *name) {
    Texture *t = textures_find(name);
    if (!t) return NULL;
    handle_free(t->handle);
    t->handle = 0;
    t->raster = handle_new(OBJECT_RASTER, t);
    if (!t->lightmap_image) t->lightmap_image = lightmap_image(t->file);
    return t;
}

s32 gli_load_level_textures(GuestPtr level) {
    lower_in_place(level);
    const char *name = GUEST(char, level);
    char path[0x80];

    snprintf(path, sizeof path, "%s/%s.%s", name, name, "lv2");
    u32 level_file = file_open(path, FRAME);
    u32 level_length = file_length(level_file);
    file_close(level_file);
    g_level_heap = inv_MEM_fn_p_vAllocAligned(0, inv_fn_ulGetBiggestFreeBlockSize() - level_length - LEVEL_HEAP_MARGIN,
                                              0x10, 5);

    for (int k = 0; k < DICTIONARIES; k++) dictionary_free(&g_dictionaries[k]);

    int slot = -1;
    static const char *const kinds[2] = {"rw3", "lm3"};
    for (int kind = 0; kind < 2; kind++) {
        bool opened;
        int part = -1;
        do {
            part++;
            snprintf(path, sizeof path, "%s/%s.%s.%d", name, name, kinds[kind], part);
            u32 file = file_open(path, FRAME);
            opened = file_opened(file);
            if (!opened) continue;
            u32 length;
            u8 *bytes = file_read(file, &length);
            file_read_done();
            Dictionary d = {0};
            if (bytes && dictionary_read(&d, bytes, length) != TXD_NONE) slot++;
            if (slot >= 0 && slot < DICTIONARIES) {
                dictionary_free(&g_dictionaries[slot]);
                g_dictionaries[slot] = d;
            } else {
                dictionary_free(&d);
            }
        } while (opened || part == 0);
    }
    return 1;
}

void textures_purge(void) {
    for (int k = 0; k < DICTIONARIES; k++) dictionary_free(&g_dictionaries[k]);
    render_show_frame();
    render_show_frame();
    GuestPtr slot = game_stack_push(16);
    *GUEST(u32, slot) = g_level_heap;
    inv_MEM_fn_vFree(slot);
    game_stack_pop(16);
    g_level_heap = 0;
}
