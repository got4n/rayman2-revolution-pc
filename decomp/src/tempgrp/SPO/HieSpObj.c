#include "common.h"

#include "GEO/GEO.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "GAM/GAM.h"
#include "AI/AI.h"

#define SUPER_OBJECT_FLAGS_SUPERIMPOSED                                                            \
    ((u32) (HIE_FLAG_SUPERIMPOSED_inv | HIE_FLAG_NOT_PICKABLE_inv))
#define SUPER_OBJECT_FRAME_ALWAYS -1
#define SUPER_OBJECT_FRAME_INVALID -2
#define SUPER_OBJECT_OPAQUE 255.0f

#define SUPERIMPOSED_CAPACITY 32

extern s16 g_xFixIndexSave;
extern u8 g_ucContext;
extern HIE_tdstSuperObject_* gd_hFixSPOSave[];
extern u8 gd_ucContextOfSPOSave[];

s32 HIE_fn_bIsSuperObjectValid(HIE_tdstSuperObject_* _hSprObj)
{
    return _hSprObj != NULL;
}

void HIE_fn_vSetSuperObjectObjectAndType(HIE_tdstSuperObject_* _hSprObj, void* linkedObject,
                                         s32 type)
{
    _hSprObj->hLinkedObject = linkedObject;
    if (linkedObject == NULL) {
        type = HIE_TYPE_EMPTY_inv;
    }
    _hSprObj->lTypeOfLinkedObject = type;
}

void SPO_ReinitSuperimposedArray(void)
{
    s16 xIndex;

    for (xIndex = 0; xIndex < g_xFixIndex; xIndex++) {
        gd_hFixSPO[xIndex]->flags &= ~SUPER_OBJECT_FLAGS_SUPERIMPOSED;
    }
    g_xFixIndex = 0;
    g_ucContext = 0;
    g_xFixIndexSave = 0;
}

HIE_tdstSuperObject_* HIE_fn_hGetElementFormFixViewport(s16 xIndex)
{
    if (xIndex < g_xFixIndex) {
        return gd_hFixSPO[xIndex];
    }
    return NULL;
}

void HIE_fn_SO_vSetSuperimposedFlag(HIE_tdstSuperObject_* _hSO)
{
    if ((_hSO->flags & HIE_FLAG_SUPERIMPOSED_inv) == 0) {
        _hSO->flags |= SUPER_OBJECT_FLAGS_SUPERIMPOSED;
        if (g_xFixIndex < SUPERIMPOSED_CAPACITY) {
            gd_hFixSPO[g_xFixIndex++] = _hSO;
        }
    }
}

void HIE_fn_SO_vClearSuperimposedFlag(HIE_tdstSuperObject_* _hSO)
{
    s16 i;

    if (_hSO->flags & HIE_FLAG_SUPERIMPOSED_inv) {
        _hSO->flags &= ~SUPER_OBJECT_FLAGS_SUPERIMPOSED;
        if (g_xFixIndex != 0) {
            for (i = 0; i < SUPERIMPOSED_CAPACITY; i++) {
                if (gd_hFixSPO[i] == _hSO) {
                    g_xFixIndex--;
                    if (g_xFixIndex != 0) {
                        gd_hFixSPO[i] = gd_hFixSPO[g_xFixIndex];
                    }
                    gd_hFixSPO[g_xFixIndex] = NULL;
                    break;
                }
            }
        }
    }
}

void HIE_fn_SO_SwitchSuperImposedTab(s32 lParam)
{
    s32 i;

    if (lParam) {
        if (g_xFixIndex != 0) {
            for (i = 0; i < g_xFixIndex; i++) {
                gd_hFixSPOSave[g_xFixIndexSave++] = gd_hFixSPO[i];
                gd_ucContextOfSPOSave[i] = g_ucContext;
            }
            g_xFixIndex = 0;
        }
        g_ucContext++;
    } else {
        g_ucContext--;
        if (g_xFixIndexSave != 0) {
            g_xFixIndex = 0;
            while (g_xFixIndexSave > 0 &&
                   gd_ucContextOfSPOSave[g_xFixIndexSave - 1] == g_ucContext) {
                if (fn_ucStdGameCheckValidity(gd_hFixSPOSave[g_xFixIndexSave - 1])) {
                    gd_hFixSPO[g_xFixIndex++] = gd_hFixSPOSave[g_xFixIndexSave - 1];
                }
                g_xFixIndexSave--;
            }
        }
    }
}

void HIE_fn_vSetSuperObjectExcluLight(HIE_tdstSuperObject_* _hSprObj, u8 excluded)
{
    if (excluded) {
        _hSprObj->flags |= HIE_FLAG_EXCLU_LIGHT_inv;
    } else {
        _hSprObj->flags &= ~HIE_FLAG_EXCLU_LIGHT_inv;
    }
}

static inline void inv_HIE_fn_vUnlinkFromBrothers(HIE_tdstSuperObject_* _hSprObj)
{
    if (_hSprObj->hNextBrotherSta != NULL) {
        _hSprObj->hNextBrotherSta->hPrevBrotherSta = _hSprObj->hPrevBrotherSta;
    }
    if (_hSprObj->hPrevBrotherSta != NULL) {
        _hSprObj->hPrevBrotherSta->hNextBrotherSta = _hSprObj->hNextBrotherSta;
    }
}

static inline void inv_HIE_fn_vUnlinkFromFather(HIE_tdstSuperObject_* _hSprObj)
{
    if (_hSprObj->hFatherSta != NULL) {
        _hSprObj->hFatherSta->lNumberOfElementsSta--;
        if (_hSprObj->hFatherSta->hFirstElementSta == _hSprObj) {
            _hSprObj->hFatherSta->hFirstElementSta = _hSprObj->hNextBrotherSta;
        }
        if (_hSprObj->hFatherSta->hLastElementSta == _hSprObj) {
            _hSprObj->hFatherSta->hLastElementSta = _hSprObj->hPrevBrotherSta;
        }
    }
}

void HIE_fn_vIsolate(HIE_tdstSuperObject_* _hSprObj)
{
    inv_HIE_fn_vUnlinkFromBrothers(_hSprObj);
    inv_HIE_fn_vUnlinkFromFather(_hSprObj);
    _hSprObj->hNextBrotherSta = NULL;
    _hSprObj->hPrevBrotherSta = NULL;
    _hSprObj->hFatherSta = NULL;
}

void HIE_fn_vPseudoIsolate(HIE_tdstSuperObject_* _hSprObj)
{
    HIE_tdstSuperObject_* nextBrother = _hSprObj->hNextBrotherSta;

    inv_HIE_fn_vUnlinkFromBrothers(_hSprObj);
    inv_HIE_fn_vUnlinkFromFather(_hSprObj);
    _hSprObj->hNextBrotherSta = nextBrother;
    _hSprObj->hFatherSta = NULL;
    _hSprObj->hPrevBrotherSta = NULL;
}

HIE_tdstSuperObject_* inv_HIE_fn_hCreateSuperObject(void)
{
    HIE_tdstSuperObject_* object = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(HIE_tdstSuperObject_));

    object->hFirstElementSta = NULL;
    object->hLastElementSta = NULL;
    object->lNumberOfElementsSta = 0;
    object->hNextBrotherSta = NULL;
    object->hPrevBrotherSta = NULL;
    object->hFatherSta = NULL;
    HIE_fn_vSetSuperObjectObjectAndType(object, NULL, HIE_TYPE_EMPTY_inv);
    object->hVisualBoundingVolume = NULL;
    object->hCollideBoundingVolume = NULL;
    object->gu_lDrawModeMask = -1;
    object->gu_hGlobalMatrix = GEO_fn_hCreateMatrix();
    gu_MTH4D_M_vSetIdentityMatrix(object->gu_hGlobalMatrix);
    HIE_fn_vInvalidateOneGlobalMatrix(object);
    object->gu_fTransparenceLevel = SUPER_OBJECT_OPAQUE;
    HIE_fn_vSetSuperObjectExcluLight(object, 0);
    return object;
}

HIE_tdstSuperObject_* HIE_fn_p_stCreateSuperObjectTab(s32 lSize)
{
    HIE_tdstSuperObject_* hNewSuperObjectTab =
        AI_fn_p_vTrueAlloc(gp_6F74, lSize * sizeof(HIE_tdstSuperObject_));
    HIE_tdstSuperObject_* hNewSuperObject = hNewSuperObjectTab;
    s32 lIndex;

    for (lIndex = 0; lIndex < lSize; lIndex++, hNewSuperObject++) {
        hNewSuperObject->hFirstElementSta = NULL;
        hNewSuperObject->hLastElementSta = NULL;
        hNewSuperObject->lNumberOfElementsSta = 0;
        hNewSuperObject->hNextBrotherSta = NULL;
        hNewSuperObject->hPrevBrotherSta = NULL;
        hNewSuperObject->hFatherSta = NULL;
        HIE_fn_vSetSuperObjectObjectAndType(hNewSuperObject, NULL, 0);
        hNewSuperObject->hVisualBoundingVolume = NULL;
        hNewSuperObject->hCollideBoundingVolume = NULL;
        hNewSuperObject->gu_lDrawModeMask = -1;
        hNewSuperObject->gu_hGlobalMatrix = GEO_fn_hCreateMatrix();
        gu_MTH4D_M_vSetIdentityMatrix(hNewSuperObject->gu_hGlobalMatrix);
        HIE_fn_vInvalidateOneGlobalMatrix(hNewSuperObject);
        hNewSuperObject->gu_fTransparenceLevel = SUPER_OBJECT_OPAQUE;
        HIE_fn_vSetSuperObjectExcluLight(hNewSuperObject, 0);
    }
    return hNewSuperObjectTab;
}

HIE_tdstSuperObject_* HIE_fn_hForceGetSuperObjectNextBrother(HIE_tdstSuperObject_* _hSprObj)
{
    if (HIE_fn_bIsSuperObjectValid(_hSprObj)) {
        return _hSprObj->hNextBrotherSta;
    }
    return _hSprObj;
}

HIE_tdstSuperObject_* HIE_fn_hGetChildNumber(HIE_tdstSuperObject_* _hParent, s32 _lNumber)
{
    HIE_tdstSuperObject_* child = _hParent->hFirstElementSta;
    s16 i;

    for (i = 0; i < _lNumber; i++) {
        child = child->hNextBrotherSta;
    }
    return child;
}

void HIE_fn_vInvalidateAllGlobalMatrices(void)
{
    HIE_gs_lCurrentFrame++;
    if (HIE_gs_lCurrentFrame == SUPER_OBJECT_FRAME_ALWAYS) {
        HIE_gs_lCurrentFrame++;
    }
}

void HIE_fn_vInvalidateOneGlobalMatrix(HIE_tdstSuperObject_* _hSprObj)
{
    if (_hSprObj->gu_lLastComputeFrame != SUPER_OBJECT_FRAME_ALWAYS) {
        if (HIE_gs_lCurrentFrame != 0) {
            _hSprObj->gu_lLastComputeFrame = HIE_gs_lCurrentFrame - 1;
        } else {
            _hSprObj->gu_lLastComputeFrame = SUPER_OBJECT_FRAME_INVALID;
        }
    }
}

void HIE_fn_vInitGlobalMatrixFrameCounter(s32 lValue)
{
    if (lValue != SUPER_OBJECT_FRAME_ALWAYS) {
        HIE_gs_lCurrentFrame = lValue;
    } else {
        HIE_gs_lCurrentFrame = 0;
    }
}

static inline u8 inv_HIE_fn_bIsGlobalMatrixUpToDate(HIE_tdstSuperObject_* _hSprObj)
{
    return _hSprObj->gu_lLastComputeFrame == HIE_gs_lCurrentFrame ||
           _hSprObj->gu_lLastComputeFrame == SUPER_OBJECT_FRAME_ALWAYS;
}

void HIE_fn_vPushSuperObjectMatrix(HIE_tdstSuperObject_* _hSprObj)
{
    if (inv_HIE_fn_bIsGlobalMatrixUpToDate(_hSprObj)) {
        HIE_fn_vLoadMatrix(_hSprObj->gu_hGlobalMatrix);
    } else {
        _POS_fn_vMulMatrixMatrix(_hSprObj->gu_hGlobalMatrix,
                                 HIE_g_a_hMatrixStack[HIE_g_lNbMatrixInStack - 1],
                                 _hSprObj->gu_hLocalMatrix);
        HIE_fn_vLoadMatrix(_hSprObj->gu_hGlobalMatrix);
        _hSprObj->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
    }
}

void HIE_fn_vComputeNewRelativeMatrix(HIE_tdstSuperObject_* _hSprObj)
{
    POS_tdstMatrix_ inverseFatherMatrix;

    POS_fn_vInvertMatrix(&inverseFatherMatrix, _hSprObj->hFatherSta->gu_hGlobalMatrix);
    _POS_fn_vMulMatrixMatrix(_hSprObj->gu_hLocalMatrix, &inverseFatherMatrix,
                             _hSprObj->gu_hGlobalMatrix);
}

void HIE_fn_vChangeFather(HIE_tdstSuperObject_* _hSprObj, HIE_tdstSuperObject_* _hNewFather,
                          u8 _bAtHead)
{
    inv_HIE_fn_vUnlinkFromBrothers(_hSprObj);
    inv_HIE_fn_vUnlinkFromFather(_hSprObj);
    _hSprObj->hNextBrotherSta = NULL;
    _hSprObj->hPrevBrotherSta = NULL;
    _hSprObj->hFatherSta = NULL;
    if (_bAtHead) {
        HIE_fn_vSuperObjectAddHead(_hNewFather, _hSprObj);
    } else {
        HIE_fn_vSuperObjectAddTail(_hNewFather, _hSprObj);
    }
    HIE_fn_vComputeNewRelativeMatrix(_hSprObj);
}

void HIE_fn_vRefreshHierarchy(HIE_tdstSuperObject_* _hSprObj)
{
    HIE_tdstSuperObject_* hChild;

    for (hChild = _hSprObj->hFirstElementSta; hChild != NULL; hChild = hChild->hNextBrotherSta) {
        _POS_fn_vMulMatrixMatrix(hChild->gu_hGlobalMatrix, _hSprObj->gu_hGlobalMatrix,
                                 hChild->gu_hLocalMatrix);
        hChild->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
        HIE_fn_vRefreshHierarchy(hChild);
    }
}

void inv_HIE_fn_vUpdateGlobalMatrixFromFather(HIE_tdstSuperObject_* _hSprObj)
{
    HIE_tdstSuperObject_* child;

    if (!inv_HIE_fn_bIsGlobalMatrixUpToDate(_hSprObj)) {
        _POS_fn_vMulMatrixMatrix(_hSprObj->gu_hGlobalMatrix, _hSprObj->hFatherSta->gu_hGlobalMatrix,
                                 _hSprObj->gu_hLocalMatrix);
        _hSprObj->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
    }
    for (child = _hSprObj->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        inv_HIE_fn_vUpdateGlobalMatrixFromFather(child);
    }
}

void inv_HIE_fn_vPushHierarchyMatrices(HIE_tdstSuperObject_* _hSprObj)
{
    HIE_tdstSuperObject_* child;

    HIE_fn_vPushSuperObjectMatrix(_hSprObj);
    for (child = _hSprObj->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        inv_HIE_fn_vPushHierarchyMatrices(child);
    }
    HIE_fn_vPopMatrix();
}

void HIE_fn_vInvalidateHierarchyFrame(HIE_tdstSuperObject_* _hSprObj)
{
    HIE_tdstSuperObject_* child;

    HIE_fn_vInvalidateOneGlobalMatrix(_hSprObj);
    for (child = _hSprObj->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        HIE_fn_vInvalidateHierarchyFrame(child);
    }
}

u32 inv_GLI_fn_ulGetActiveLightMask(void)
{
    GLI_tdstLight_** light = gs_aDEF_p_CurrentListOfLight;
    u8* affected = g_ucTableOfActiveLightsForChar;
    u32 mask = 0;
    s32 i;

    for (i = 0; i < gs_CurrentNumberOfLightInViewport; i++, light++, affected++) {
        if ((*light)->b_lOnOff && (*light)->bValid && *affected) {
            mask |= 1 << i;
        }
    }
    return mask;
}

void inv_GLI_vResetLightCommands(void)
{
    inv_GLI_lightCommandCursor = inv_GLI_lightCommandBuffer;
}

void inv_GLI_vPushLightCommand06(void)
{
    *inv_GLI_lightCommandCursor++ = LIGHT_COMMAND_STATE_06;
    inv_GLI_openLightCommand = NULL;
}

void inv_GLI_vPushLightCommandSkip(void)
{
    *inv_GLI_lightCommandCursor++ = LIGHT_COMMAND_STATE_SKIP;
    inv_GLI_openLightCommand = NULL;
}

void inv_GLI_vPushLightCommand05(void)
{
    *inv_GLI_lightCommandCursor++ = LIGHT_COMMAND_STATE_05;
    inv_GLI_openLightCommand = NULL;
}

void inv_GLI_vPushTransparentSuperObject(HIE_tdstSuperObject_* _hSprObj, s32 drawMask)
{
    LightCommandValue level;
    LightCommandValue module;

    level.level = GEO_xGetZFarTransparencyLevel();
    module.superObject = _hSprObj;
    *inv_GLI_lightCommandCursor++ = LIGHT_COMMAND_MODULE;
    *inv_GLI_lightCommandCursor++ = module.word;
    *inv_GLI_lightCommandCursor++ = drawMask;
    *inv_GLI_lightCommandCursor++ = level.word;
    *inv_GLI_lightCommandCursor++ = inv_GLI_fn_ulGetActiveLightMask();
    inv_GLI_openLightCommand = NULL;
}
