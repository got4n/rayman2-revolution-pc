#include "common.h"

#include "GLI/GLI.h"
#include "GAM/GAM.h"
#include "AI/AI.h"

extern s32 inv_GLI_bFirstInit;
extern s32 GLI_gsIsInit;

void func_0014BA70(f32 period);

void GLI_xInitGli(void)
{
    GliDisplayStateView* display;

    if (inv_GLI_bFirstInit != 0) {
        u8* memory = AI_fn_p_vTrueAlloc(gp_6F74, 0x20CC);
        s32 misalignment = (s32) memory & 15;

        inv_GLI_renderContext = (GLI_tdstRenderContext*) memory;
        if (misalignment != 0)
            inv_GLI_renderContext = (GLI_tdstRenderContext*) (memory - (misalignment - 16));
        inv_GLI_renderContext->fogParameters = 0;
        inv_GLI_renderContext->appliedFogParameters = 0;
        inv_GLI_renderContext->fogEnabled = 0;
        inv_GLI_renderContext->unk20B0 = 0;
        inv_GLI_renderContext->inv_bNoCameraClear = 0;
        inv_GLI_renderContext->inv_bUseSinEffectBase = 0;
        inv_GLI_renderContext->globalAlpha = 0.0f;
        GLI_xCreateMaterial(&gs_hDefaultMaterial);
        GLI_vInitWaterplaneModule();
        GLI_vInitLensFlare();
        GLI_xInitMatrixStack();
        inv_GLI_bFirstInit = 0;
    }
    display = &inv_GLI_stDisplayState;
    display->frameRate = 0.0f;
    GLI_RW_xInitRasterize(0, 1, g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, g_lVIDEO_WantedDepth);
    if (display->frameRate < 40.0f || display->frameRate >= 210.0f) {
        display->frameRate = 60.0f;
        display->flags |= 0x40;
    }
    func_0014BA70(1.0f / inv_GLI_stDisplayState.frameRate);
    GLI_gsIsInit = 1;
}

void GLI_xDesinit(void)
{
    GLI_gsIsInit = 0;
    loc_11C5D0();
}

s32 GLI_xIsGliInit(void)
{
    return GLI_gsIsInit;
}
