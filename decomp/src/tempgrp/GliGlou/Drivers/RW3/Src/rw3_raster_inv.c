#include "common.h"

#include "GLI/GLI.h"

#include "rwps2.h"

#include "include_asm.h"

RwFrame* RwFrameCreate(void);

#define ATOMIC_REQUEST_COUNT 300
#define TEXTURED_REQUEST_COUNT 150

extern AtomicDrawRequest inv_GLI_atomicRequests[ATOMIC_REQUEST_COUNT];
extern TexturedDrawRequest inv_GLI_texturedRequests[TEXTURED_REQUEST_COUNT];

RwFrame* RwFrameGetParent(RwFrame* frame);

RwRGBA* RpMaterialGetColor(RpMaterial* material);

typedef void (*SkyFreeFunc)(void* memory);

extern SkyFreeFunc inv_RW_pfnRwFree[];

void inv_RW3_fn_vSetRasterPalette(RwRaster* raster, void* palette)
{
    _SkyRasterExt* ext;
    u32 tex0;
    u32 new0;
    u32 new1;
    s32 value;
    void** mip;
    s32 i;
    RwGlobals_inv* globals;

    if (raster == NULL || palette == NULL) {
        return;
    }
    if (raster->depth != 0x20) {
        return;
    }

    ext = (_SkyRasterExt*) ((u8*) raster + skyRasterExt);
    if ((ext->nTexCacheSize & 0x1F) != 0) {
        return;
    }
    if (raster->cFormat != 5) {
        return;
    }
    if (ext->dmaRefCount != 0) {
        return;
    }

    raster->depth = 8;
    raster->width <<= 2;

    ext->tex0.words[0] += 0x08000000;
    tex0 = ext->tex0.words[0] & 0xFFF03FFF;
    ext->tex0.words[0] = tex0 | ((raster->width >= 0x81) ? ((raster->width >> 6) << 14) : 0x8000);

    raster->cFormat = 0x25;
    new0 = (ext->tex0.words[0] & 0xFC0FFFFF) | 0x01300000;
    new1 = (ext->tex0.words[1] & 0xDF87FFFB) | 0x20000004;
    ext->tex0.words[0] = new0;
    ext->tex0.words[1] = new1;
    raster->palette = palette;
    ext->sysMemPalSize = 0x400;

    if (raster->width < 0x80) {
        value = 0x10;
    } else if (raster->height < 0x40) {
        value = 8;
    } else {
        value = ext->nTexCacheSize >> 6;
        ext->palOffset = value;
        ext->nTexCacheSize += 0x800;
        goto valueDone;
    }
    ext->palOffset = value;
valueDone:

    if (ext->palUploadPkt != NULL) {
        inv_RW_pfnRwFree[0](ext->palUploadPkt);
        ext->palUploadPkt = NULL;
    }

    globals = &ourGlobals;
    mip = &ext->mipUploadPkts[0];
    i = 6;
    do {
        if (*mip != NULL) {
            globals->memoryFunctions_inv.rwfree(*mip);
            *mip = NULL;
        }
        i--;
        mip++;
    } while (i >= 0);

    skyPrepareUploadRaster(raster);
}

void inv_RW3_fn_vSetAtomicAlpha(f32 intensity, RpAtomic* object)
{
    RpMaterial* material;
    RwRGBA color;

    material = RpGeometryGetMaterial(object->geometry, 0);
    color = *RpMaterialGetColor(material);
    color.alpha = intensity * 255.0f;
    RpMaterialSetColor(material, &color);
}

void inv_RW3_fn_vInitAtomicRequests(void)
{
    AtomicDrawRequest* request;
    AtomicDrawRequest* end;
    AtomicDrawRequest* requests;
    u32 i;

    requests = inv_GLI_atomicRequests;
    request = requests;
    end = request + ATOMIC_REQUEST_COUNT;
    do {
        request->atomic = NULL;
        request->comparisonFlags = 0;
        request->mode = 0;
        request->alpha = 0;
        request->flags = 0;
        request->fogParameters = NULL;
        request->frame = RwFrameCreate();
        inv_RW3_fn_vClearLightList(&request->lights);
        request++;
    } while (request < end);
    request = requests;
    for (i = 0; i < TEXTURED_REQUEST_COUNT; i++) {
        inv_GLI_texturedRequests[i].quad = inv_GLI_singleQuad;
        inv_GLI_texturedRequests[i].texture = NULL;
        inv_GLI_texturedRequests[i].fogParameters = NULL;
    }
}

void inv_RW3_fn_vAddAtomicRequest(f32 alpha, RpAtomic* atomic, s32 comparisonFlags, u8 mode, s32 unused, u8 flags)
{
    AtomicDrawRequest* request;
    RwFrame* parent;

    if (inv_GLI_atomicRequestCount < ATOMIC_REQUEST_COUNT) {
        request = &inv_GLI_atomicRequests[inv_GLI_atomicRequestCount];
        inv_GLI_drawQueue[inv_GLI_atomicRequestCount + inv_GLI_texturedRequestCount].object.atomic =
            request;
        inv_GLI_drawQueue[inv_GLI_atomicRequestCount + inv_GLI_texturedRequestCount].kind =
            DRAW_KIND_ATOMIC;
        request->mode = mode;
        request->flags = flags;
        request->comparisonFlags = comparisonFlags;
        request->alpha = alpha;
        request->atomic = atomic;
        if (inv_GLI_renderContext->fogEnabled != 0)
            request->fogParameters = inv_GLI_renderContext->fogParameters;
        else
            request->fogParameters = NULL;
        if (!(flags & DRAW_REQUEST_KEEP_FRAME)) {
            parent = RwFrameGetParent(atomic->object.object.parent);
            if (parent != NULL)
                RW3_Utils_DuplicateFrame(request->frame, parent);
        }
        inv_RW3_fn_vCopyViewportLights(&request->lights);
        inv_GLI_atomicRequestCount++;
    }
}

s32 inv_RW3_fn_bReserveTexturedRequest(TexturedDrawRequest** result, GliTexture* texture)
{
    if (inv_GLI_texturedRequestCount < TEXTURED_REQUEST_COUNT) {
        *result = &inv_GLI_texturedRequests[inv_GLI_texturedRequestCount];
        inv_GLI_texturedRequests[inv_GLI_texturedRequestCount].texture = texture;
        if (inv_GLI_renderContext->fogEnabled)
            inv_GLI_texturedRequests[inv_GLI_texturedRequestCount].fogParameters =
                inv_GLI_renderContext->fogParameters;
        else
            inv_GLI_texturedRequests[inv_GLI_texturedRequestCount].fogParameters = NULL;
        inv_GLI_drawQueue[inv_GLI_atomicRequestCount + inv_GLI_texturedRequestCount]
            .object.textured = &inv_GLI_texturedRequests[inv_GLI_texturedRequestCount];
        inv_GLI_drawQueue[inv_GLI_atomicRequestCount + inv_GLI_texturedRequestCount].kind =
            DRAW_KIND_TEXTURED;
        inv_GLI_texturedRequestCount++;
        return 1;
    }
    return 0;
}

void inv_RW3_fn_vResetAtomicRequests(void)
{
    inv_GLI_atomicRequestCount = 0;
}
