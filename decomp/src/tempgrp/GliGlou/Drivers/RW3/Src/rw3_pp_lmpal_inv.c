#include "common.h"

#include "ps2/libc.h"
#include "renderware.h"
#include "GLI/GLI.h"

extern u8 D_003936E8;
extern u8 off_362A00[];

s32 RwRasterGetStride(RwRaster* raster);
void RpSkyTextureCacheSetState(s32 state);
void skyTexCacheFlush(void);
void skyTexCacheRasterLock(RwRaster* raster, s32 lock);
u32 inv_RW3_fn_lGetRasterTbp0(RwRaster* raster);
void inv_RW3_vFillWhiteAlphaRampPalette(RwRaster* raster);
void func_001517F0(void);

void* func_00151338(void)
{
    return off_362A00;
}

void inv_RW3_vFillWhiteAlphaRampPalette(RwRaster* raster)
{
    u32 alpha = 0;
    u32* palette = RwRasterLockPalette(raster, 3);
    u8 i;

    do {
        for (i = 0; i < 8; i++) {
            *palette++ = (alpha++ << 24) | 0xFFFFFF;
        }
        palette += 8;
        for (i = 0; i < 8; i++) {
            *palette++ = (alpha++ << 24) | 0xFFFFFF;
        }
        palette -= 16;
        for (i = 0; i < 8; i++) {
            *palette++ = (alpha++ << 24) | 0xFFFFFF;
        }
        palette += 8;
        for (i = 0; i < 8; i++) {
            *palette++ = (alpha++ << 24) | 0xFFFFFF;
        }
    } while (alpha < 0x100);

    RwRasterUnlockPalette(raster);
}

void inv_RW3_fn_vCreateLightmapPaletteRaster(RwRaster** raster)
{
    u8* pixels;
    s32 height;
    s32 stride;
    s32 x;
    s32 y;
    u8 value;

    *raster = RwRasterCreate(0x20, 0x20, 0, 0x2504);
    if (*raster == NULL) {
        printf("RwRasterCreate texture Pal8 failed\n");
        if (*raster == NULL) {
            return;
        }
    }

    pixels = RwRasterLock(*raster, 0, 3);
    if (pixels == NULL) {
        printf("RwRasterLock Pal8 failed\n");
    } else {
        height = RwRasterGetHeight(*raster);
        stride = RwRasterGetStride(*raster);
        for (y = 0; y < height; y++) {
            value = ((s64) y << 8) / height;
            for (x = 0; x < stride; x++) {
                *pixels++ = value;
            }
        }
    }

    RwRasterUnlock(*raster);
    inv_RW3_vFillWhiteAlphaRampPalette(*raster);
}

void func_001515C0(void)
{
    if (D_003936E8 == 1) {
        return;
    }
    if (inv_RW3_lightmapRasterR == NULL) {
        inv_RW3_lightmapRasterR = RwRasterCreate(0x80, 0x80, 8, 0x2504);
        memset(RwRasterLock(inv_RW3_lightmapRasterR, 0, 1), 0x60, 0x4000);
        RwRasterUnlock(inv_RW3_lightmapRasterR);
        inv_RW3_vFillWhiteAlphaRampPalette(inv_RW3_lightmapRasterR);
    }
    if (inv_RW3_lightmapRasterG == NULL) {
        inv_RW3_lightmapRasterG = RwRasterCreate(0x80, 0x80, 8, 0x2504);
        memset(RwRasterLock(inv_RW3_lightmapRasterG, 0, 1), 0x60, 0x4000);
        RwRasterUnlock(inv_RW3_lightmapRasterG);
        inv_RW3_vFillWhiteAlphaRampPalette(inv_RW3_lightmapRasterG);
    }
    if (inv_RW3_lightmapRasterB == NULL) {
        inv_RW3_lightmapRasterB = RwRasterCreate(0x80, 0x80, 8, 0x2504);
        memset(RwRasterLock(inv_RW3_lightmapRasterB, 0, 1), 0x60, 0x4000);
        RwRasterUnlock(inv_RW3_lightmapRasterB);
        inv_RW3_vFillWhiteAlphaRampPalette(inv_RW3_lightmapRasterB);
    }
    if (inv_RW3_lightmapPaletteRaster == NULL) {
        inv_RW3_fn_vCreateLightmapPaletteRaster(&inv_RW3_lightmapPaletteRaster);
    }
}

void func_001516F8(void)
{
    char* format;
    u32 address;

    if (D_003936E8 == 1) {
        return;
    }
    RpSkyTextureCacheSetState(1);
    skyTexCacheFlush();
    skyTexCacheRasterLock(inv_RW3_lightmapRasterR, 1);
    address = inv_RW3_fn_lGetRasterTbp0(inv_RW3_lightmapRasterR);
    format = "Raster locked at 0x%X in VRAM\n";
    inv_RW3_lightmapVramR = address;
    printf(format, address);
    skyTexCacheRasterLock(inv_RW3_lightmapRasterG, 1);
    address = inv_RW3_fn_lGetRasterTbp0(inv_RW3_lightmapRasterG);
    inv_RW3_lightmapVramG = address;
    printf(format, address);
    skyTexCacheRasterLock(inv_RW3_lightmapRasterB, 1);
    address = inv_RW3_fn_lGetRasterTbp0(inv_RW3_lightmapRasterB);
    inv_RW3_lightmapVramB = address;
    printf(format, address);
    skyTexCacheRasterLock(inv_RW3_lightmapPaletteRaster, 1);
    printf(format, inv_RW3_fn_lGetRasterTbp0(inv_RW3_lightmapPaletteRaster));
    inv_RW3_fn_vCopyRasterPalette(inv_RW3_lightmapRasterR, inv_RW3_lightmapPaletteRaster);
    inv_RW3_fn_vCopyRasterPalette(inv_RW3_lightmapRasterG, inv_RW3_lightmapPaletteRaster);
    inv_RW3_fn_vCopyRasterPalette(inv_RW3_lightmapRasterB, inv_RW3_lightmapPaletteRaster);
    D_003936E8 = 1;
}

void func_001517F0(void)
{
    if (D_003936E8 != 1) {
        return;
    }
    skyTexCacheFlush();
    skyTexCacheRasterLock(inv_RW3_lightmapRasterR, 0);
    inv_RW3_lightmapVramR = 0;
    RwRasterDestroy(inv_RW3_lightmapRasterR);
    inv_RW3_lightmapRasterR = NULL;
    skyTexCacheRasterLock(inv_RW3_lightmapRasterG, 0);
    inv_RW3_lightmapVramG = 0;
    RwRasterDestroy(inv_RW3_lightmapRasterG);
    inv_RW3_lightmapRasterG = NULL;
    skyTexCacheRasterLock(inv_RW3_lightmapRasterB, 0);
    inv_RW3_lightmapVramB = 0;
    RwRasterDestroy(inv_RW3_lightmapRasterB);
    inv_RW3_lightmapRasterB = NULL;
    skyTexCacheRasterLock(inv_RW3_lightmapPaletteRaster, 0);
    RwRasterDestroy(inv_RW3_lightmapPaletteRaster);
    inv_RW3_lightmapPaletteRaster = NULL;
    printf("     --> DESinitialization for Lightmaps DONE.\n");
    D_003936E8 = 0;
}

void inv_RW3_fn_vDestroyLightmapTemplates(void)
{
    if (inv_RW3_lightmapTemplateRasterR != NULL) {
        RwRasterDestroy(inv_RW3_lightmapTemplateRasterR);
        inv_RW3_lightmapTemplateRasterR = NULL;
    }
    if (inv_RW3_lightmapTemplateRasterG != NULL) {
        RwRasterDestroy(inv_RW3_lightmapTemplateRasterG);
        inv_RW3_lightmapTemplateRasterG = NULL;
    }
    if (inv_RW3_lightmapTemplateRasterB != NULL) {
        RwRasterDestroy(inv_RW3_lightmapTemplateRasterB);
        inv_RW3_lightmapTemplateRasterB = NULL;
    }
    func_001517F0();
}
