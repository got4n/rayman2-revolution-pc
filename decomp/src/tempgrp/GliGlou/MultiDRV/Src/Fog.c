#include "common.h"

#include "GLI/GLI.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"

extern s32 inv_GLI_fogRecordIndex;
extern tdstFogParams_ inv_GLI_fogRecordRing[];
extern tdstFogParams_* inv_GLI_currentFogRecord;

void GLI_vSetFog2(GEO_tdstColor_* color, f32 xBlendStart, f32 xDepthStart, f32 xBlendEnd,
                   f32 xDepthEnd, f32 xInfinite)
{
    s32 index;
    s32 next;
    tdstFogParams_* fog;
    tdstFogParams_* current;

    index = inv_GLI_fogRecordIndex;
    next = index + 1;
    if (index == 39) {
        next = 0;
    }
    fog = &inv_GLI_fogRecordRing[next];
    if (xDepthEnd < xDepthStart) {
        fog->xDepthEnd = xDepthStart;
        fog->xDepthStart = xDepthEnd;
    } else {
        fog->xDepthStart = xDepthStart;
        fog->xDepthEnd = xDepthEnd;
    }
    if (xInfinite < xDepthEnd) {
        fog->xInfinite = xDepthEnd;
    } else {
        fog->xInfinite = xInfinite;
    }
    if (xBlendStart < 0.0f) {
        xBlendStart = 0.0f;
    } else if (255.0f < xBlendStart) {
        xBlendStart = 255.0f;
    }
    if (xBlendEnd < 0.0f) {
        xBlendEnd = 0.0f;
    } else if (255.0f < xBlendEnd) {
        xBlendEnd = 255.0f;
    }
    fog->xBlendStart = xBlendStart;
    fog->xBlendEnd = xBlendEnd;
    fog->uColor.stColor = *color;

    current = inv_GLI_currentFogRecord;
    if (fog->xDepthStart != current->xDepthStart || fog->xDepthEnd != current->xDepthEnd ||
        fog->xBlendStart != current->xBlendStart ||
        fog->xBlendEnd != current->xBlendEnd ||
        memcmp(&fog->uColor.stWords, &current->uColor.stWords, sizeof(fog->uColor.stWords)) != 0) {
        inv_GLI_fogRecordIndex = next;
        inv_GLI_currentFogRecord = fog;
    }
    inv_GLI_renderContext->fogParameters = inv_GLI_currentFogRecord;
}

void GLI_vFogOn(void)
{
    inv_GLI_renderContext->fogEnabled = 1;
}

void GLI_vFogOff(void)
{
    inv_GLI_renderContext->fogEnabled = 0;
}
