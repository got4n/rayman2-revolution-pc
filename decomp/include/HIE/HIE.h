#ifndef REVO_HIE_HIE_H
#define REVO_HIE_HIE_H

#include "ps2/types.h"

struct MTH3D_tdstVector_;
struct POS_tdstMatrix_;

typedef struct HIE_tdstSuperObject_ {
    s32 lTypeOfLinkedObject;
    void* hLinkedObject;
    struct HIE_tdstSuperObject_* hFirstElementSta;
    struct HIE_tdstSuperObject_* hLastElementSta;
    s32 lNumberOfElementsSta;
    struct HIE_tdstSuperObject_* hNextBrotherSta;
    struct HIE_tdstSuperObject_* hPrevBrotherSta;
    struct HIE_tdstSuperObject_* hFatherSta;
    struct POS_tdstMatrix_* gu_hLocalMatrix;
    struct POS_tdstMatrix_* gu_hGlobalMatrix;
    s32 gu_lLastComputeFrame;
    s32 gu_lDrawModeMask;
    s32 flags;
    void* hVisualBoundingVolume;
    void* hCollideBoundingVolume;
    f32 gu_fTransparenceLevel;
} HIE_tdstSuperObject_;

#define HIE_TYPE_EMPTY_inv 1
#define HIE_TYPE_ACTOR_inv 2
#define HIE_TYPE_SECTOR_inv 4
#define HIE_TYPE_PHYSICAL_OBJECT_inv 8
#define HIE_TYPE_GEOMETRIC_OBJECT_inv 0x400

#define HIE_FLAG_NOT_PICKABLE_inv 0x1
#define HIE_FLAG_HIDDEN_inv 0x2
#define HIE_FLAG_TRANSLATION_ONLY_inv 0x4
#define HIE_FLAG_BOX_BOUNDS_inv 0x10
#define HIE_FLAG_SUPERIMPOSED_inv 0x20
#define HIE_FLAG_ALWAYS_VISIT_CHILDREN_inv 0x200
#define HIE_FLAG_TRANSPARENT_inv 0x10000
#define HIE_FLAG_EXCLU_LIGHT_inv 0x20000

#define HIE_MATRIX_STACK_SIZE_inv 50

static inline void HIE_fn_vSuperObjectAddHead(HIE_tdstSuperObject_* father,
                                              HIE_tdstSuperObject_* object)
{
    if (object->hNextBrotherSta != 0)
        object->hNextBrotherSta->hPrevBrotherSta = object->hPrevBrotherSta;
    if (object->hPrevBrotherSta != 0)
        object->hPrevBrotherSta->hNextBrotherSta = object->hNextBrotherSta;
    if (object->hFatherSta != 0) {
        object->hFatherSta->lNumberOfElementsSta--;
        if (object->hFatherSta->hFirstElementSta == object)
            object->hFatherSta->hFirstElementSta = object->hNextBrotherSta;
        if (object->hFatherSta->hLastElementSta == object)
            object->hFatherSta->hLastElementSta = object->hPrevBrotherSta;
    }
    object->hNextBrotherSta = 0;
    object->hFatherSta = 0;
    object->hPrevBrotherSta = 0;

    object->hNextBrotherSta = father->hFirstElementSta;
    object->hFatherSta = father;
    if (father->hLastElementSta == 0)
        father->hLastElementSta = object;
    else
        father->hFirstElementSta->hPrevBrotherSta = object;
    father->hFirstElementSta = object;
    father->lNumberOfElementsSta++;
}

static inline void HIE_fn_vSuperObjectAddTail(HIE_tdstSuperObject_* father,
                                              HIE_tdstSuperObject_* object)
{
    if (object->hNextBrotherSta != 0)
        object->hNextBrotherSta->hPrevBrotherSta = object->hPrevBrotherSta;
    if (object->hPrevBrotherSta != 0)
        object->hPrevBrotherSta->hNextBrotherSta = object->hNextBrotherSta;
    if (object->hFatherSta != 0) {
        object->hFatherSta->lNumberOfElementsSta--;
        if (object->hFatherSta->hFirstElementSta == object)
            object->hFatherSta->hFirstElementSta = object->hNextBrotherSta;
        if (object->hFatherSta->hLastElementSta == object)
            object->hFatherSta->hLastElementSta = object->hPrevBrotherSta;
    }
    object->hPrevBrotherSta = 0;
    object->hFatherSta = 0;
    object->hNextBrotherSta = 0;

    object->hPrevBrotherSta = father->hLastElementSta;
    object->hFatherSta = father;
    if (father->hFirstElementSta == 0)
        father->hFirstElementSta = object;
    else
        father->hLastElementSta->hNextBrotherSta = object;
    father->hLastElementSta = object;
    father->lNumberOfElementsSta++;
}

extern struct POS_tdstMatrix_* GEO_fn_hCreateMatrix(void);
extern void HIE_fn_vInitMatrixStack(void);
extern void HIE_fn_vCreateMatrixStack(void);

extern void HIE_fn_vPopMatrix(void);
extern void HIE_fn_vLoadMatrix(struct POS_tdstMatrix_* matrix);
extern void HIE_fn_vPushMatrix(void);

extern void HIE_fn_vInvalidateOneGlobalMatrix(struct HIE_tdstSuperObject_* object);
extern void HIE_fn_vInvalidateAllGlobalMatrices(void);
extern HIE_tdstSuperObject_* HIE_fn_hForceGetSuperObjectNextBrother(HIE_tdstSuperObject_* _hSprObj);
extern HIE_tdstSuperObject_* HIE_fn_hGetChildNumber(HIE_tdstSuperObject_* _hParent, s32 _lNumber);
extern void HIE_fn_vInitGlobalMatrixFrameCounter(s32 lValue);
extern void HIE_fn_vInvalidateHierarchyFrame(HIE_tdstSuperObject_* _hSprObj);
extern void HIE_fn_vIsolate(HIE_tdstSuperObject_* _hSprObj);
extern void HIE_fn_vPseudoIsolate(HIE_tdstSuperObject_* _hSprObj);
extern void SPO_ReinitSuperimposedArray(void);
extern void inv_HIE_fn_vUpdateGlobalMatrixFromFather(HIE_tdstSuperObject_* _hSprObj);
extern void inv_GLI_vResetLightCommands(void);
extern void inv_GLI_vPushLightCommand06(void);
extern void inv_GLI_vPushLightCommandSkip(void);
extern void inv_GLI_vPushLightCommand05(void);
extern void inv_GLI_vPushTransparentSuperObject(HIE_tdstSuperObject_* _hSprObj, s32 drawMask);
extern HIE_tdstSuperObject_* HIE_fn_hGetElementFormFixViewport(s16 xIndex);
extern void HIE_fn_SO_SwitchSuperImposedTab(s32 lParam);
extern void HIE_fn_SO_vClearSuperimposedFlag(HIE_tdstSuperObject_* _hSO);
extern void HIE_fn_SO_vSetSuperimposedFlag(HIE_tdstSuperObject_* _hSO);
extern void HIE_fn_vChangeFather(HIE_tdstSuperObject_* _hSprObj, HIE_tdstSuperObject_* _hNewFather,
                                 u8 _bAtHead);
extern void HIE_fn_vComputeNewRelativeMatrix(HIE_tdstSuperObject_* _hSprObj);
extern void HIE_fn_vRefreshHierarchy(HIE_tdstSuperObject_* _hSprObj);
extern void HIE_fn_vPushSuperObjectMatrix(HIE_tdstSuperObject_* _hSprObj);
extern HIE_tdstSuperObject_* HIE_fn_p_stCreateSuperObjectTab(s32 lSize);

extern s32 HIE_bIntersectSegmentWithFirstSuperObject(struct MTH3D_tdstVector_* p_stVertexA,
                                                     struct MTH3D_tdstVector_* p_stVectAB,
                                                     HIE_tdstSuperObject_* _hSprObj,
                                                     struct MTH3D_tdstVector_* _p_stHit,
                                                     struct MTH3D_tdstVector_* _p_stNormal,
                                                     HIE_tdstSuperObject_** _p_hSprObjHit);
extern s32 HIE_bDetectIntersectSegmentWithSuperObject(struct MTH3D_tdstVector_* p_stVertexA, struct MTH3D_tdstVector_* p_stVectAB, HIE_tdstSuperObject_* _hSprObj, void* camera);

static inline void* HIE_fn_hGetSuperObjectObject(HIE_tdstSuperObject_* hSupObj)
{
    return hSupObj->hLinkedObject;
}

struct COL_tdstParallelBox_;
struct MTH4D_tdstMatrix_;
extern s32 func_00148C10(HIE_tdstSuperObject_* superObject, struct COL_tdstParallelBox_* total);
extern void func_001488E0(struct COL_tdstParallelBox_* bounds, struct MTH4D_tdstMatrix_* matrix, struct COL_tdstParallelBox_* source);
struct GLD_tdstViewportAttributes_;
extern void HIE_fn_vSendCharacterToViewport(void* _hVpt, HIE_tdstSuperObject_* _hSprObj,
                                            s32 _lDrawMask, s32 _lCullingResult);
extern s32 gu_GLI_RW_vInitTextureModule(void* _hVpt, s32 _lDrawMask);
extern void HIE_fn_vSendStaticWorldToViewport(struct GLD_tdstViewportAttributes_* _hVpt,
                                              HIE_tdstSuperObject_* _hSprObj, s32 _DrawMask,
                                              s32 _lCullingResult);

extern void fn_vSendOneModuleToViewPort(void* _hVpt, HIE_tdstSuperObject_* _hSprObj,
                                        s32 _lDrawMask);

extern s32 HIE_g_lNbMatrixInStack;
extern struct POS_tdstMatrix_* HIE_g_a_hMatrixStack[HIE_MATRIX_STACK_SIZE_inv];
extern HIE_tdstSuperObject_* gd_hFixSPO[];
extern s16 g_xFixIndex;
extern s32 HIE_gs_lCurrentFrame;

#endif
