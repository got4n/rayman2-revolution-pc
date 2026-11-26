#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "snd/snd.h"

#include "ps2/libc.h"
#include "AI/AI.h"

#define MICRO_SPEED_LIMIT 32000.0f
#define FIXED16_ONE 65536.0f

typedef struct LST2_tdstAnchorToMIC_tdxHandleToMicroDyn LST2_tdstAnchorToMIC_tdxHandleToMicroDyn;

typedef struct tdstIndexMicros_ {
    struct tdstIndexMicros_* hNextBrotherDyn;
    struct tdstIndexMicros_* hPrevBrotherDyn;
    LST2_tdstAnchorToMIC_tdxHandleToMicroDyn* hFatherDyn;
    s32 lIndex;
    tdstEngineObject_* hMicroObj;
} tdstIndexMicros_;

struct LST2_tdstAnchorToMIC_tdxHandleToMicroDyn {
    tdstIndexMicros_* hFirstElementDyn;
    tdstIndexMicros_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
};

typedef struct tdstIndexMicrosList_ {
    LST2_tdstAnchorToMIC_tdxHandleToMicroDyn hMicrosList;
} tdstIndexMicrosList_;

extern tdstIndexMicrosList_ g_stIndexMicrosList;

s32 SND_fn_lCreateMicroSound(void);
void fn_vAddMicro(tdstEngineObject_* object);
u32 fn_bMicroGetActivationFlag(tdstMicro_* micro);
s32 func_0010A5B0(s32 index);
tdstMicro_* fn_h_MicroRealAlloc(void);

static inline tdstMicro_* inv_GAM_fn_p_stMicroOf(tdstEngineObject_* object)
{
    return object->h_Micro;
}

static inline f32 inv_GAM_fn_xComponentOf(const MTH3D_tdstVector_* vector, s32 axis)
{
    switch (axis) {
    case 0:
        return vector->xX;
    case 1:
        return vector->xY;
    default:
        return vector->xZ;
    }
}

static inline s32 inv_GAM_fn_lToFixed16(f32 value, f32 scale)
{
    s32 integer = value;
    f32 integerPart = integer;

    integer <<= 16;
    return integer + (value - integerPart) * scale;
}

void fn_vMicroAlloc(tdstEngineObject_* p_stObject)
{
    MTH3D_tdstVector_ stI;
    MTH3D_tdstVector_ stJ;
    MTH3D_tdstVector_ stK;

    p_stObject->h_Micro = fn_h_MicroRealAlloc();
    MTH3D_M_vSetXYZ_inv(&stJ, -1.0f, 0.0f, 0.0f);
    MTH3D_M_vSetXYZ_inv(&stK, stJ.xY, stJ.xY, 1.0f);
    if (p_stObject->h_Cineinfo != NULL) {
        MTH3D_M_vSetXYZ_inv(&stI, stJ.xY, -1.0f, stJ.xY);
    } else {
        MTH3D_M_vSetXYZ_inv(&stI, stJ.xY, 1.0f, stJ.xY);
    }
    _POS_fn_vSetRotationMatrix(inv_GAM_fn_p_stMicroOf(p_stObject)->hMicroMatrix, &stI, &stJ, &stK);
}

tdstMicro_* fn_h_MicroRealAlloc(void)
{
    tdstMicro_* micro = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstMicro_));

    micro->hMicroMatrix = GEO_fn_hCreateMatrix();
    return micro;
}

void fn_vMicroInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    if (eObjectInit == OTI_MapLoaded && fn_bMicroGetActivationFlag(inv_GAM_fn_p_stMicroOf(p_stObject))) {
        fn_vAddMicro(p_stObject);
    }
}

void fn_vMicroDesinit(void)
{
}

void fn_vMicroCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    *inv_GAM_fn_p_stMicroOf(_p_stObjectDest) = *inv_GAM_fn_p_stMicroOf(_p_stObjectSrc);
}

u32 fn_bMicroGetActivationFlag(tdstMicro_* micro)
{
    return micro->bIsActive;
}

tdstEngineObject_* fn_hReturnHandleMicroFromIndex(s32 lIndex)
{
    tdstIndexMicros_* hMicro;

    for (hMicro = g_stIndexMicrosList.hMicrosList.hFirstElementDyn; hMicro != NULL; hMicro = hMicro->hNextBrotherDyn) {
        if (hMicro->lIndex == lIndex) {
            return hMicro->hMicroObj;
        }
    }
    return NULL;
}

static inline s32 inv_GAM_fn_lMicroPositionComponent(s32 index, s32 axis)
{
    tdstEngineObject_* object = fn_hReturnHandleMicroFromIndex(index);
    f32 scale = FIXED16_ONE;
    POS_tdstMatrix_* matrix = inv_fn_hGetOwnerSuperObject(object)->gu_hGlobalMatrix;
    MTH3D_tdstVector_ position = *inv_POS_fn_p_stPosMatrixTranslation(matrix);

    return inv_GAM_fn_lToFixed16(inv_GAM_fn_xComponentOf(&position, axis), scale);
}

s32 RetMicroSonoreX(s32 index)
{
    return inv_GAM_fn_lMicroPositionComponent(index, 0);
}

s32 RetMicroSonoreY(s32 index)
{
    return inv_GAM_fn_lMicroPositionComponent(index, 1);
}

s32 RetMicroSonoreZ(s32 index)
{
    return inv_GAM_fn_lMicroPositionComponent(index, 2);
}

static inline s32 inv_GAM_fn_lMicroVelocityComponent(s32 index, s32 axis)
{
    f32 velocity = 0.0f;
    tdstEngineObject_* object = fn_hReturnHandleMicroFromIndex(index);
    tdstEngineObject_* owner;
    tdstDynam_* dynam;
    DNM_stDynamics* dynamics;
    f32 limit;

    if (object != NULL) {
        owner = inv_fn_hGetOwnerSuperObject(object)->hLinkedObject;
        dynam = owner->h_Dynam;
        if (dynam == NULL) {
            return 0;
        }
        dynamics = dynam->p_stDynamics;
        if (dynamics->m_tdstDynamicsBaseBlock.m_lObjectType == 0) {
            f32 delta = inv_fn_xGetEngineClockDelta();

            velocity = inv_GAM_fn_xComponentOf(&dynamics->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsolutePrevSpeed.m_stLinear, axis) / delta;
        } else {
            velocity = inv_GAM_fn_xComponentOf(&dynamics->m_tdstDynamicsBaseBlock.m_stPreviousSpeed, axis);
        }
    }
    limit = MICRO_SPEED_LIMIT;
    if (limit < velocity) {
        velocity = limit;
    }
    limit = -MICRO_SPEED_LIMIT;
    if (velocity < limit) {
        velocity = limit;
    }
    return inv_GAM_fn_lToFixed16(velocity, FIXED16_ONE);
}

s32 RetMicroSonoreVX(s32 index)
{
    return inv_GAM_fn_lMicroVelocityComponent(index, 0);
}

s32 RetMicroSonoreVY(s32 index)
{
    return inv_GAM_fn_lMicroVelocityComponent(index, 1);
}

s32 RetMicroSonoreVZ(s32 index)
{
    return inv_GAM_fn_lMicroVelocityComponent(index, 2);
}

static inline s32 inv_GAM_fn_lMicroDirectionComponent(s32 index, s32 axis, s32 tangent)
{
    tdstEngineObject_* object = fn_hReturnHandleMicroFromIndex(index);
    HIE_tdstSuperObject_* owner = inv_fn_hGetOwnerSuperObject(object);
    tdstEngineObject_* linked = owner->hLinkedObject;
    void* matrix = owner->gu_hGlobalMatrix;
    void* microMatrix = inv_GAM_fn_p_stMicroOf(linked)->hMicroMatrix;
    MTH3D_tdstVector_ x0;
    MTH3D_tdstVector_ y0;
    MTH3D_tdstVector_ z0;
    MTH3D_tdstVector_ x1;
    MTH3D_tdstVector_ y1;
    MTH3D_tdstVector_ z1;
    f32 basisX;
    f32 basisY;
    f32 dot;

    POS_fn_vGetRotationVectors(matrix, &x0, &y0, &z0);
    POS_fn_vGetRotationVectors(microMatrix, &x1, &y1, &z1);
    if (tangent) {
        basisX = y1.xX;
        basisY = y1.xY;
        dot = inv_GAM_fn_xComponentOf(&x0, axis) * basisX + inv_GAM_fn_xComponentOf(&y0, axis) * basisY +
              inv_GAM_fn_xComponentOf(&z0, axis) * y1.xZ;
    } else {
        basisX = x1.xX;
        basisY = x1.xY;
        dot = inv_GAM_fn_xComponentOf(&x0, axis) * basisX + inv_GAM_fn_xComponentOf(&y0, axis) * basisY +
              inv_GAM_fn_xComponentOf(&z0, axis) * x1.xZ;
    }
    return inv_GAM_fn_lToFixed16(dot, FIXED16_ONE);
}

s32 RetMicroSonoreDNX(s32 index)
{
    return inv_GAM_fn_lMicroDirectionComponent(index, 0, 0);
}

s32 RetMicroSonoreDNY(s32 index)
{
    return inv_GAM_fn_lMicroDirectionComponent(index, 1, 0);
}

s32 RetMicroSonoreDNZ(s32 index)
{
    return inv_GAM_fn_lMicroDirectionComponent(index, 2, 0);
}

s32 RetMicroSonoreDTX(s32 index)
{
    return inv_GAM_fn_lMicroDirectionComponent(index, 0, 1);
}

s32 RetMicroSonoreDTY(s32 index)
{
    return inv_GAM_fn_lMicroDirectionComponent(index, 1, 1);
}

s32 RetMicroSonoreDTZ(s32 index)
{
    return inv_GAM_fn_lMicroDirectionComponent(index, 2, 1);
}

s32 func_0010A5B0(s32 index)
{
    return 0;
}

void fn_vAddMicro(tdstEngineObject_* hEngObj)
{
    tdstIndexMicros_* entry = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstIndexMicros_));
    LST2_tdstAnchorToMIC_tdxHandleToMicroDyn* anchor;

    entry->hNextBrotherDyn = NULL;
    entry->hPrevBrotherDyn = NULL;
    entry->hFatherDyn = NULL;
    entry->lIndex = SND_fn_lCreateMicroSound();
    entry->hMicroObj = hEngObj;
    if (entry->hNextBrotherDyn != NULL) {
        entry->hNextBrotherDyn->hPrevBrotherDyn = entry->hPrevBrotherDyn;
    }
    if (entry->hPrevBrotherDyn != NULL) {
        entry->hPrevBrotherDyn->hNextBrotherDyn = entry->hNextBrotherDyn;
    }
    anchor = entry->hFatherDyn;
    if (anchor != NULL) {
        anchor->lNumberOfElementsDyn--;
        anchor = entry->hFatherDyn;
        if (anchor->hFirstElementDyn == entry) {
            anchor->hFirstElementDyn = entry->hNextBrotherDyn;
            anchor = entry->hFatherDyn;
        }
        if (anchor->hLastElementDyn == entry) {
            anchor->hLastElementDyn = entry->hPrevBrotherDyn;
        }
    }
    entry->hPrevBrotherDyn = NULL;
    entry->hFatherDyn = NULL;
    entry->hNextBrotherDyn = NULL;
    entry->hPrevBrotherDyn = g_stIndexMicrosList.hMicrosList.hLastElementDyn;
    entry->hFatherDyn = &g_stIndexMicrosList.hMicrosList;
    if (g_stIndexMicrosList.hMicrosList.hFirstElementDyn == NULL) {
        g_stIndexMicrosList.hMicrosList.hFirstElementDyn = entry;
    } else {
        g_stIndexMicrosList.hMicrosList.hLastElementDyn->hNextBrotherDyn = entry;
    }
    g_stIndexMicrosList.hMicrosList.hLastElementDyn = entry;
    g_stIndexMicrosList.hMicrosList.lNumberOfElementsDyn++;
    inv_GAM_fn_p_stMicroOf(hEngObj)->bIsActive = 1;
}

void fn_vRemoveAllMicros(void)
{
    tdstIndexMicros_* entry;
    tdstIndexMicros_* next;

    for (entry = g_stIndexMicrosList.hMicrosList.hFirstElementDyn, next = entry ? entry->hNextBrotherDyn : NULL; entry != NULL;
         entry = next, next = entry ? entry->hNextBrotherDyn : NULL) {
        SND_fn_vDestroyMicroSound((s32*) entry->lIndex);
        if (entry->hNextBrotherDyn != NULL) {
            entry->hNextBrotherDyn->hPrevBrotherDyn = entry->hPrevBrotherDyn;
        }
        if (entry->hPrevBrotherDyn != NULL) {
            entry->hPrevBrotherDyn->hNextBrotherDyn = entry->hNextBrotherDyn;
        }
        if (entry->hFatherDyn != NULL) {
            entry->hFatherDyn->lNumberOfElementsDyn--;
            if (entry->hFatherDyn->hFirstElementDyn == entry) {
                entry->hFatherDyn->hFirstElementDyn = entry->hNextBrotherDyn;
            }
            if (entry->hFatherDyn->hLastElementDyn == entry) {
                entry->hFatherDyn->hLastElementDyn = entry->hPrevBrotherDyn;
            }
        }
        entry->hNextBrotherDyn = NULL;
        entry->hPrevBrotherDyn = NULL;
        entry->hFatherDyn = NULL;
    }
    g_stIndexMicrosList.hMicrosList.hFirstElementDyn = NULL;
    g_stIndexMicrosList.hMicrosList.hLastElementDyn = NULL;
    g_stIndexMicrosList.hMicrosList.lNumberOfElementsDyn = 0;
}

void fn_vInitMicrosFunction(void)
{
    _tdstAllRetMicroSound st_pfnRetMicroSound;

    memset(&st_pfnRetMicroSound, 0, sizeof(st_pfnRetMicroSound));
    st_pfnRetMicroSound.X = RetMicroSonoreX;
    st_pfnRetMicroSound.Y = RetMicroSonoreY;
    st_pfnRetMicroSound.Z = RetMicroSonoreZ;
    st_pfnRetMicroSound.VX = RetMicroSonoreVX;
    st_pfnRetMicroSound.VY = RetMicroSonoreVY;
    st_pfnRetMicroSound.VZ = RetMicroSonoreVZ;
    st_pfnRetMicroSound.DNX = RetMicroSonoreDNX;
    st_pfnRetMicroSound.DNY = RetMicroSonoreDNY;
    st_pfnRetMicroSound.DNZ = RetMicroSonoreDNZ;
    st_pfnRetMicroSound.DTX = RetMicroSonoreDTX;
    st_pfnRetMicroSound.DTY = RetMicroSonoreDTY;
    st_pfnRetMicroSound.DTZ = RetMicroSonoreDTZ;
    st_pfnRetMicroSound.Filtre = func_0010A5B0;
    SND_fn_vSetRetMicroSound(&st_pfnRetMicroSound);
    g_stIndexMicrosList.hMicrosList.hFirstElementDyn = NULL;
    g_stIndexMicrosList.hMicrosList.hLastElementDyn = NULL;
    g_stIndexMicrosList.hMicrosList.lNumberOfElementsDyn = 0;
}

tdstEngineObject_* fn_p_stGetFirstEngineObjectInMicroList(void)
{
    tdstIndexMicros_* entry = g_stIndexMicrosList.hMicrosList.hFirstElementDyn;

    if (entry != NULL) {
        return entry->hMicroObj;
    }
    return NULL;
}

void GAM_fn_vChangeCharacterForMicro(HIE_tdstSuperObject_* _hCharacter)
{
    g_stIndexMicrosList.hMicrosList.hFirstElementDyn->hMicroObj = _hCharacter->hLinkedObject;
}
