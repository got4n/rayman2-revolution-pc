#include "common.h"

#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "include_asm.h"
#include "SCT/SCT.h"
#include "GEO/GEO.h"

#define GEOMETRIC_ELEMENT_INDEXED 3
#define GLI_M_TRAP() (*(u64*) 3 = 0x365FC0)

extern POS_tdstMatrix_* g_p_stCurrentMatrix;


#ifdef PC_PORT
u32 fptoui(f32 value);
#endif
void inv_RW3_fn_vDrawSpriteList(void* object, void* element);

static inline s32 inv_GLI_fn_bIsIndexedElement(s32 enabled, s16 type)
{
    s32 valid = enabled;

    if (type == GEOMETRIC_ELEMENT_INDEXED) {
        return valid;
    }
    valid = 0;
    return valid;
}

void GLI_vSetGlobalAlpha(f32 Alpha)
{
    if (255.0f < Alpha) {
        inv_GLI_renderContext->globalAlpha = 1.0f;
    } else if (Alpha < 0.0f) {
        inv_GLI_renderContext->globalAlpha = 0.0f;
    } else {
        inv_GLI_renderContext->globalAlpha = Alpha * (1.0f / 255.0f);
    }
}

void func_0011A810(f32 alpha)
{
    if (1.0f < alpha) {
        inv_GLI_renderContext->globalAlpha = 1.0f;
    } else if (alpha < 0.0f) {
        inv_GLI_renderContext->globalAlpha = 0.0f;
    } else {
        inv_GLI_renderContext->globalAlpha = alpha;
    }
}

f32 GLI_vGetGlobalAlpha(void)
{
    return inv_GLI_renderContext->globalAlpha * 255.0f;
}

f32 func_0011A860(void)
{
    return inv_GLI_renderContext->globalAlpha;
}

void GLI_vDoMaterialSelection(void)
{
    GLI_tdstRenderContext* context;
    GliMaterialData* material;
    FloatBits alpha;
    u32 color;
    u32 rgb;

    if (!(inv_GLI_renderContext->lHierachDrawMask & 0x100) ||
        inv_GLI_renderContext->currentMaterial == NULL) {
        inv_GLI_renderContext->currentMaterial = gs_hDefaultMaterial;
    }
    inv_GLI_renderContext->lCurrentDrawMask =
        inv_GLI_renderContext->currentMaterial->type & inv_GLI_renderContext->lHierachDrawMask;
    GLI_xRefreshAnimatedTexture(inv_GLI_renderContext->currentMaterial);
    inv_GLI_renderContext->currentTexture =
        inv_GLI_renderContext->currentMaterial->layers[0].texture;
    GLI_RW_vDoMaterialSelection();

    context = inv_GLI_renderContext;
    if ((context->lHierachDrawMask & 8) || context->globalAlpha == 1.0f) {
        color = 0xFFFFFFFF;
    } else {
        material = context->currentMaterial;
        alpha.value = context->globalAlpha + 32768.0f + 16384.0f;
        color = alpha.bits;
        color <<= 24;
        if (material->layers[0].texture == NULL) {
#ifdef PC_PORT
            rgb = fptoui(material->color.xR * 255.0f) << 16;
            rgb |= fptoui(material->color.xG * 255.0f) << 8;
            rgb |= fptoui(material->color.xB * 255.0f);
#else
            rgb = (u32) (material->color.xR * 255.0f) << 16;
            rgb |= (u32) (material->color.xG * 255.0f) << 8;
            rgb |= (u32) (material->color.xB * 255.0f);
#endif
            color |= rgb;
        } else {
            color |= 0xFFFFFF;
        }
    }
    context->vertexColor.packed = color;
}

void GLI_xSendObjectElement(GLD_tdstViewportAttributes_* p_stVpt)
{
    func_00152B10();
}

void func_0011A9C8(GLD_tdstViewportAttributes_* object, s32 flags)
{
    s16 i;
    GeometricObjectView* geometricObject;

    for (i = 0; i < (*(GeometricObjectView**) ((u8*) (long) (*(s32*) &inv_GLI_renderContext) + 4))
                        ->elementCount;
         i++) {
        geometricObject =
            *(GeometricObjectView**) ((u8*) (long) (*(s32*) &inv_GLI_renderContext) + 4);
        if (geometricObject->elementTypes[i] == GEOMETRIC_ELEMENT_INDEXED) {
            inv_RW3_fn_vDrawSpriteList(*(void**) (*&inv_GLI_renderContext), geometricObject->elements[i]);
        }
    }
}

void GLI_xSendObjectToViewportWithLights(GLD_tdstViewportAttributes_* p_stVpt,
                                          GeometricObjectView* p_stObj, s32 lDrawModeMask)
{
    GLI_tdstRenderContext* context = inv_GLI_renderContext;
    GLI_tdstCamera_** slot = p_stVpt->p_vSpecificToXD;
    GLI_tdstCamera_* instance;
    POS_tdstMatrix_ saved;
    s16 mode;
    GeometricObjectView* current;
    s32 enabled;
    s32 status;

    context->object = p_stVpt;
    context->currentMatrix = g_p_stCurrentMatrix;
    context->lHierachDrawMask = lDrawModeMask;
    context->inv_lDrawFlags = 0;
    context->geometricObject = p_stObj;
    context->geometryMatrix = &(*slot)->stMatrix;
    instance = *slot;
    context->camera = instance;
    GLI_DRV_vSetZClip(instance, instance->xNear);
    GLI_DRV_vSetClipWindow(p_stVpt->values.dwClipLeftInPix, p_stVpt->values.dwClipRightInPix,
                            p_stVpt->values.dwClipTopInPix, p_stVpt->values.dwClipBottomInPix);
    if (GEO_bIsLookAt(p_stObj) != 0) {
        saved = *g_p_stCurrentMatrix;
        GEO_vTurnLookAt(p_stObj, p_stVpt, g_p_stCurrentMatrix);
        _POS_fn_vMulMatrixMatrix(&inv_GLI_renderContext->objectMatrix, &(*slot)->stMatrix,
                                 inv_GLI_renderContext->currentMatrix);
    } else {
        _POS_fn_vMulMatrixMatrix(&inv_GLI_renderContext->objectMatrix, &(*slot)->stMatrix,
                                 inv_GLI_renderContext->currentMatrix);
    }
    mode = g_cSinusEffectState;
    if ((mode == 1 && lDrawModeMask >= 0) || (mode == 2 && lDrawModeMask < 0) || mode == 3) {
        inv_RW3_fn_vSendDeformedObject(p_stObj);
    }
    context = inv_GLI_renderContext;
    current = context->geometricObject;
    enabled = 1;
    status = (s32) current->rw3Object;
    enabled = enabled - status + status;
    if (status != 0) {
        GLI_xSendObjectElement(p_stVpt);
    } else if (inv_SCT_ucDrawPass == 0) {
        mode = current->elementTypes[0];
        status = inv_GLI_fn_bIsIndexedElement(enabled, mode);
        if (status != 0) {
            func_0011A9C8(p_stVpt, lDrawModeMask);
        } else {
            GLI_M_TRAP();
        }
    }
    if (GEO_bIsLookAt(p_stObj) != 0) {
        *g_p_stCurrentMatrix = saved;
    }
}

void func_0011AE58(GLI_tdstCamera_* camera, s32 count, MTH4D_tdstVector_* source,
                   ProjectedVertex* destination)
{
    s32 farValue;
    ProjectedVertex* last = destination + count;

    *(f32*) &farValue = 8000.0f;

    while (destination < last) {
        if (*(s32*) &source->xZ < *(s32*) &camera->xNear) {
            *(s32*) &destination->z = *(s32*) &destination->z | 0x80000000;
            *(s32*) &destination->x = farValue | ((*(s32*) &source->xX & 0x80000000) ^ 0x80000000);
            *(s32*) &destination->y = farValue | ((*(s32*) &source->xY & 0x80000000) ^ 0x80000000);
        } else {
            f32 inverseZ = 1.0f / source->xZ;
            f32 quarter;
            f32 scale;
            destination->z = inverseZ;
            quarter = inverseZ * 0.25f;
            scale = camera->xScreen;
            destination->z = quarter;
            scale = scale * inverseZ;
            destination->x = (source->xX * camera->stScale.xX) * scale;
            destination->y = (source->xY * camera->stScale.xY) * scale;
        }
        source++;
        destination++;
    }
}
