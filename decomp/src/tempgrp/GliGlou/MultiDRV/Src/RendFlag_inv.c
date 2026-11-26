#include "common.h"

#include "GLI/GLI.h"

extern s32 inv_GLI_savedRenderFlags;

void inv_GLI_vSaveRenderFlags(void)
{
    GLI_tdstRenderContext* driver = inv_GLI_renderContext;
    s32 flags = driver->lHierachDrawMask;

    inv_GLI_savedRenderFlags = flags;
    driver->lHierachDrawMask = flags & ~0x408;
    driver->lCurrentDrawMask = flags & ~0x408;
}

void inv_GLI_vRestoreRenderFlags(void)
{
    inv_GLI_renderContext->lHierachDrawMask = inv_GLI_savedRenderFlags;
}
