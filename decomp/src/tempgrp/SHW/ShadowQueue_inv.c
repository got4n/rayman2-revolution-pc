#include "common.h"
#include "GAM/GAM.h"
#include "ps2/libc.h"
#include "GEO/GEO.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "AI/AI.h"
#include "COL/COL.h"
#include "SCT/SCT.h"
#include "ps2/PS2.h"
#include "SHW/SHW.h"

void inv_SHW_fn_vResetShadowQueue(void)
{
    ShadowQueue_inv* list = &inv_SHW_stShadowQueue;

    memset(list, 0, sizeof(ShadowQueue_inv));
    list->firstCapacity = 4;
    list->secondCapacity = 2;
}

#define CUSTOM_BIT_ZFAR_TRANSPARENCY 0x200000
#define SUPER_OBJECT_TRANSPARENCE_LEVEL 0x10000

s32 inv_SHW_fn_bComputeShadowBounds(ShadowEntry_inv* entry);
void inv_SHW_fn_vComputeShadowProjection(ShadowEntry_inv* entry);
s32 inv_SHW_fn_bAddProjectedShadow(HIE_tdstSuperObject_* superObject, Data3dShadow_inv* shadow);
f32 inv_fn_xGetSuperObjectTransparencyFactor(HIE_tdstSuperObject_* superObject, ShadowEntry_inv* entry);

void inv_SHW_fn_vInitShadowQueue(void)
{
    ShadowQueue_inv* queue = &inv_SHW_stShadowQueue;

    RW3_SHW_fn_bInit(queue->secondCapacity, queue->firstCapacity);
    queue->count = 0;
    queue->firstCursor = 0;
    queue->secondCursor = 0;
}

void func_001479A0(void)
{
}

void inv_SHW_fn_vQueueCharacterShadow(GLD_tdstViewportAttributes_* viewport, HIE_tdstSuperObject_* superObject,
                   HIE_tdstSuperObject_* sectorObject, s32 drawMask)
{
    Data3dShadow_inv* shadow;
    u32 mode;

    if ((superObject->flags >> 5) & 1) {
        return;
    }
    if (!((inv_HIE_fn_p_stGetEngineObject(superObject)->h_StandardGame->ucMiscFlags >> 2) & 1)) {
        return;
    }
    if ((fn_ulStandardGameGetCustomBitsSO(superObject) & CUSTOM_BIT_ZFAR_TRANSPARENCY) &&
        GEO_xGetZFarTransparencyLevel() == 0.0f) {
        return;
    }
    if ((superObject->flags >> 1) & 1) {
        return;
    }

    shadow = &inv_HIE_fn_p_stGetEngineObject(superObject)->h_3dData->shadow;
    if (!inv_SHW_fn_bIsShadowOn(shadow)) {
        return;
    }
    mode = shadow->control.mode;
    switch (mode) {
    case 0:
        break;
    case 1:
        fn_vComputeCollideResult(superObject, sectorObject);
        break;
    case 2:
        inv_SHW_fn_bAddProjectedShadow(superObject, shadow);
        break;
    default:
        break;
    }
}

f32 inv_fn_xGetSuperObjectTransparencyFactor(HIE_tdstSuperObject_* superObject, ShadowEntry_inv* entry)
{
    if (fn_ulStandardGameGetCustomBitsSO(superObject) & CUSTOM_BIT_ZFAR_TRANSPARENCY) {
        return GEO_xGetZFarTransparencyLevel();
    }
    if (!(superObject->gu_lDrawModeMask & 8)) {
        return superObject->gu_fTransparenceLevel * (1.0f / 255.0f);
    }
    return 1.0f;
}

s32 inv_SHW_fn_bAddProjectedShadow(HIE_tdstSuperObject_* superObject, Data3dShadow_inv* shadow)
{
    ShadowQueue_inv* queue;
    ShadowEntry_inv* entry;
    GLI_tdstRenderContext* state;
    GliMaterialData* material;
    u32 cursor;
    s32 kind;
    s32 queued = 0;

    if (inv_SHW_stShadowQueue.count < SHADOW_QUEUE_SIZE) {
        queue = &inv_SHW_stShadowQueue;
        material = NULL;
        kind = shadow->source.listKind;
        if (kind == 0) {
            cursor = queue->firstCursor;
            if (cursor < queue->firstCapacity) {
                material = queue->firstMaterials[cursor];
                queue->firstCursor = cursor + 1;
            }
        } else if (kind != 1) {
            cursor = queue->firstCursor;
            if (cursor < queue->firstCapacity) {
                material = queue->firstMaterials[cursor];
                queue->firstCursor = cursor + 1;
            }
        } else {
            cursor = queue->secondCursor;
            if (cursor < queue->secondCapacity) {
                material = queue->secondMaterials[cursor];
                queue->secondCursor = cursor + 1;
            }
        }
        if (material != NULL) {
            state = inv_GLI_renderContext;
            entry = &inv_SHW_stShadowQueue.entries[inv_SHW_stShadowQueue.count];
            entry->superObject = superObject;
            entry->sectorObject = inv_HIE_fn_p_stGetEngineObject(superObject)->h_SectInfo->hCurrentSector;
            entry->material = material;
            entry->shadow = *shadow;
            if (state->fogEnabled != 0) {
                entry->fogParameters = state->fogParameters;
            } else {
                entry->fogParameters = NULL;
            }
            inv_SHW_fn_vComputeShadowProjection(entry);
            entry->scale =
                entry->scale * inv_fn_xGetSuperObjectTransparencyFactor(superObject, entry);
            inv_SHW_stShadowQueue.count++;
            queued = 1;
        }
    }
    return queued;
}

void inv_SHW_fn_vRenderShadowQueue(void)
{
    ShadowEntry_inv* entry;
    u32 i = 0;

    if (inv_SHW_stShadowQueue.count != 0) {
        do {
            entry = &inv_SHW_stShadowQueue.entries[i];
            if (inv_SHW_fn_bComputeShadowBounds(entry)) {
                RW3_SHW_fn_vRenderCharacterInShadowView(entry->superObject, &entry->matrix,
                                                        entry->material);
                inv_SHW_fn_bQueueProjectedShadow(
                    entry->sectorObject, entry->superObject, &entry->projection, entry->material,
                    entry->scale, entry->shadow.scaleX, entry->shadow.scaleY, entry->fogParameters);
            }
            i++;
        } while (i < inv_SHW_stShadowQueue.count);
    }
    inv_SHW_stShadowQueue.count = 0;
    inv_SHW_stShadowQueue.firstCursor = 0;
    inv_SHW_stShadowQueue.secondCursor = 0;
}

void inv_SHW_fn_vComputeShadowProjection(ShadowEntry_inv* entry)
{
    POS_tdstMatrix_* global = entry->superObject->gu_hGlobalMatrix;
    MTH3D_tdstVector_ projected;
    MTH3D_tdstVector_ axes[3];
    f32 forwardZ;
    f32 forwardY;
    f32 forwardX;
    f32 absZ;
    f32 absY;
    f32 crossX;
    f32 crossY;
    f32 crossZ;
    f32 upY;
    f32 upZ;
    f32 length;
    f32 zero;
    f32 inverse;
    f32 one;
    f32 sideZ;
    f32 sideY;
    f32 sideX;
    f32 x;
    f32 z;
    f32 y;

    inv_GLI_fn_vGetLightDirectionAt(inv_POS_fn_p_stPosMatrixTranslation(global), &projected, &entry->scale);
    axes[2] = projected;
    forwardZ = axes[2].xZ;
    forwardY = axes[2].xY;
    absZ = __builtin_fabsf(forwardZ);
    absY = __builtin_fabsf(forwardY);
    if (absY < absZ) {
        MTH3D_M_vSetXYZ_inv(&axes[1], 0.0f, 1.0f, 0.0f);
    } else {
        f32 axisZero = 0.0f;

        MTH3D_M_vSetXYZ_inv(&axes[1], axisZero, axisZero, 1.0f);
    }
    upY = axes[1].xY;
    upZ = axes[1].xZ;
    forwardX = axes[2].xX;
    crossX = inv_MTH_fn_xMulSub(upY, forwardZ, upZ, forwardY);
    crossY = inv_MTH_fn_xMulSub(upZ, forwardX, axes[1].xX, forwardZ);
    crossZ = inv_MTH_fn_xMulSub(axes[1].xX, forwardY, upY, forwardX);
    length = MTH_fn_xSqrAddSqrAddSqrOpt2(crossX, crossY, crossZ);
    zero = 0.0f;
    axes[0].xX = crossX;
    length = MTH_fn_xSqrtOpt2(length);
    axes[0].xY = crossY;
    axes[0].xZ = crossZ;
    if (length != zero) {
        one = 1.0f;
        inverse = one / length;
        inv_MTH3D_fn_vKTimesXYZ(&axes[0], inverse, &axes[0]);
    }
    sideZ = axes[0].xZ;
    sideY = axes[0].xY;
    sideX = axes[0].xX;
    x = inv_MTH_fn_xMulSub(forwardY, sideZ, forwardZ, sideY);
    y = inv_MTH_fn_xMulSub(forwardZ, sideX, forwardX, sideZ);
    axes[1].xX = x;
    z = inv_MTH_fn_xMulSub(forwardX, sideY, forwardY, sideX);
    axes[1].xY = y;
    axes[1].xZ = z;
    inv_POS_fn_vInitMatrixFromVectors(&entry->matrix, 5, &axes[0], &axes[1], &axes[2], NULL);
}

extern f32 flt_3935B8[];
extern f32 flt_3935BC[];

void inv_POS_fn_vScale(POS_tdstMatrix_* matrix, f32* scale);

s32 inv_SHW_fn_bComputeShadowBounds(ShadowEntry_inv* entry)
{
    MTH3D_tdstVector_ center;
    COL_tdstParallelBox_ bounds;
    COL_tdstParallelBox_ extent;
    POS_tdstMatrix_ scaling;
    f32 scale[3];
    POS_tdstMatrix_* view;
    POS_tdstMatrix_* projection;
    DNM_stDynamics* dynamics;
    u8 identity;

    if (entry->superObject->hFatherSta == NULL)
        return 0;
    bounds.stMinPoint.xX = flt_3935B8[0];
    bounds.stMinPoint.xY = flt_3935B8[0];
    bounds.stMinPoint.xZ = flt_3935B8[0];
    bounds.stMaxPoint.xX = flt_3935BC[0];
    bounds.stMaxPoint.xY = flt_3935BC[0];
    bounds.stMaxPoint.xZ = flt_3935BC[0];
    inv_HIE_fn_vUpdateGlobalMatrixFromFather(entry->superObject);
    if (func_00148C10(entry->superObject, &bounds) == 0)
        return 0;
    if (bounds.stMinPoint.xX > bounds.stMaxPoint.xX)
        return 0;
    view = &entry->matrix;
    identity = 1;
    MTH3D_M_vSetXYZ_inv(&center, (bounds.stMinPoint.xX + bounds.stMaxPoint.xX) * 0.5f, (bounds.stMinPoint.xY + bounds.stMaxPoint.xY) * 0.5f,
                        (bounds.stMinPoint.xZ + bounds.stMaxPoint.xZ) * 0.5f);
    asm("");
    if (view->ucType == identity)
        view->ucType = MAT_eTransform_Translation;
    projection = &entry->projection;
    *inv_POS_fn_p_stPosMatrixTranslation(&entry->matrix) = center;
    entry->matrix.stMatrix.stCol_3.xW = 1.0f;
    POS_fn_vInvertMatrix(projection, view);
    func_001488E0(&extent, &projection->stMatrix, &bounds);
    extent.stMaxPoint.xX = inv_MTH_fn_xMaximum(inv_MTH_fn_xAbsoluteTied(extent.stMaxPoint.xX),
                                               inv_MTH_fn_xAbsoluteTied(extent.stMinPoint.xX));
    extent.stMaxPoint.xY = inv_MTH_fn_xMaximum(inv_MTH_fn_xAbsoluteTied(extent.stMaxPoint.xY),
                                               inv_MTH_fn_xAbsoluteTied(extent.stMinPoint.xY));
    entry->viewRadius = inv_MTH_fn_xMaximum(extent.stMaxPoint.xX, extent.stMaxPoint.xY);
    dynamics = fn_p_stDynamGetDynamicsSO(entry->superObject);
    if (dynamics != NULL && (dynamics->m_tdstDynamicsBaseBlock.ulFlags & 0x20))
        entry->shadow.scaleX = entry->shadow.scaleY = extent.stMaxPoint.xZ + 1.5f;
    gu_MTH4D_M_vSetIdentityMatrix(&scaling);
    scale[0] = 0.5f / extent.stMaxPoint.xX;
    scale[1] = 0.5f / extent.stMaxPoint.xY;
    scale[2] = 1.0f / entry->shadow.scaleY;
    inv_POS_fn_vScale(&scaling, scale);
    _POS_fn_vMulMatrixMatrix(projection, &scaling, projection);
    scale[0] = extent.stMaxPoint.xX + extent.stMaxPoint.xX;
    scale[1] = extent.stMaxPoint.xY + extent.stMaxPoint.xY;
    scale[2] = entry->shadow.scaleY;
    inv_POS_fn_vScale(view, scale);
    entry->viewMinScale = inv_MTH_fn_xMinimum(scale[0], scale[1]);
    return 1;
}

f32 inv_fn_xGetSuperObjectTransparencyByte(HIE_tdstSuperObject_* superObject)
{
    f32 value;

    if (fn_ulStandardGameGetCustomBitsSO(superObject) & CUSTOM_BIT_ZFAR_TRANSPARENCY) {
        return GEO_xGetZFarTransparencyLevel() * 255.0f;
    }
    value = 255.0f;
    if (superObject->flags & SUPER_OBJECT_TRANSPARENCE_LEVEL) {
        value = superObject->gu_fTransparenceLevel;
    }
    return value;
}

f32 inv_SHW_fn_xGetPlaneZAt(MTH3D_tdstVector_* point, MTH3D_tdstVector_* normal, f32 inverseNormalZ, f32 x,
                  f32 y)
{
    f32 dx = x - point->xX;
    f32 dy = y - point->xY;
    f32 dot = normal->xX * dx + normal->xY * dy;

    return point->xZ - inverseNormalZ * dot;
}
