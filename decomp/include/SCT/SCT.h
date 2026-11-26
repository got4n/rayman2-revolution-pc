#ifndef REVO_SCT_SCT_H
#define REVO_SCT_SCT_H

#include "ps2/types.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

typedef struct SECT_tdstListOfStaticLights_ {
    struct GLI_tdstLight_* hStaticLights;
} SECT_tdstListOfStaticLights_;

typedef struct SECT_tdstListOfSOSectorsInGraphicInteraction_ {
    s16 wLevelOfDetail;
    s8 cDisplayMode;
    void* hPointerOfSectorSO;
} SECT_tdstListOfSOSectorsInGraphicInteraction_;

typedef struct SECT_tdstListOfSOSectorsInCollisionInteraction_ {
    HIE_tdstSuperObject_* hPointerOfSectorSO;
} SECT_tdstListOfSOSectorsInCollisionInteraction_;

typedef struct SECT_tdstListOfSOSectorsInActivityInteraction_ {
    HIE_tdstSuperObject_* hPointerOfSectorSO;
} SECT_tdstListOfSOSectorsInActivityInteraction_;

typedef struct SECT_tdstListOfSOCharacters_ SECT_tdstListOfSOCharacters_;
typedef struct SECT_tdstListOfDNMLights_ SECT_tdstListOfDNMLights_;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstCharacterDyn {
    SECT_tdstListOfSOCharacters_* hFirstElementDyn;
    SECT_tdstListOfSOCharacters_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstCharacterDyn;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstDNMLightsDyn {
    SECT_tdstListOfDNMLights_* hFirstElementDyn;
    SECT_tdstListOfDNMLights_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstDNMLightsDyn;

struct SECT_tdstListOfDNMLights_ {
    void* hDNMLights;
    SECT_tdstListOfDNMLights_* hNextBrotherDyn;
    SECT_tdstListOfDNMLights_* hPrevBrotherDyn;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstDNMLightsDyn* hFatherDyn;
};

struct SECT_tdstListOfSOCharacters_ {
    HIE_tdstSuperObject_* hPointerOfCharacter;
    SECT_tdstListOfSOCharacters_* hNextBrotherDyn;
    SECT_tdstListOfSOCharacters_* hPrevBrotherDyn;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstCharacterDyn* hFatherDyn;
};

typedef struct SCT_tdstListOfSoundEvent_ {
    HIE_tdstSuperObject_* hPointerOfSectorSO;
    s32* event;
} SCT_tdstListOfSoundEvent_;

typedef struct SECT_tdstListOfSOSectorsInSoundInteraction_ {
    HIE_tdstSuperObject_* hPointerOfSectorSO;
    s32 lVolume;
} SECT_tdstListOfSOSectorsInSoundInteraction_;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstStaticLightsOpt {
    SECT_tdstListOfStaticLights_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstStaticLightsOpt;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstGraphicInteractionOpt {
    SECT_tdstListOfSOSectorsInGraphicInteraction_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstGraphicInteractionOpt;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstCollisionInteractionOpt {
    SECT_tdstListOfSOSectorsInCollisionInteraction_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstCollisionInteractionOpt;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstActivityInteractionOpt {
    SECT_tdstListOfSOSectorsInActivityInteraction_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstActivityInteractionOpt;

typedef struct LST2_tdstAnchorToSECT_tdxHandleOfElementLstSoundInteractionOpt {
    SECT_tdstListOfSOSectorsInSoundInteraction_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToSECT_tdxHandleOfElementLstSoundInteractionOpt;

typedef struct LST2_tdstAnchorToSCT_tdxHandleOfLstSoundEventOpt {
    SCT_tdstListOfSoundEvent_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToSCT_tdxHandleOfLstSoundEventOpt;

typedef struct SECT_tdstSector_ {
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstCharacterDyn stListOfCharacters;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstStaticLightsOpt stListOfStaticLights;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstDNMLightsDyn stListOfDNMLights;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstGraphicInteractionOpt stListOfSectorsInGraphicInteraction;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstCollisionInteractionOpt stListOfSectorsInCollisionInteraction;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstActivityInteractionOpt stListOfSectorsInActivityInteraction;
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstSoundInteractionOpt stListOfSectorsInSoundInteraction;
    LST2_tdstAnchorToSCT_tdxHandleOfLstSoundEventOpt stListOfSoundEvent;
    MTH3D_tdstVector_ stMinMaxPoints[2];
    f32 xZFar;
    u8 xVirtual;
    s8 cCameraType;
    s8 cPriority;
} SECT_tdstSector_;

static inline SECT_tdstSector_* inv_HIE_fn_p_stGetSector(HIE_tdstSuperObject_* hSupObj)
{
    return hSupObj->hLinkedObject;
}

typedef struct tdstSectInfo_ {
    HIE_tdstSuperObject_* hCurrentSector;
    void* hNodeInSector;
    MTH3D_tdstVector_ stPreviousPosition;
} tdstSectInfo_;

extern void SECT_fn_vSendFogToViewport(void* viewport, HIE_tdstSuperObject_* superObject);
extern void SCT_fn_vSendSectorWhereIAmToViewport(MTH3D_tdstVector_* _p_stAbsolutePositionOfCamera,
                                                 struct GLD_tdstViewportAttributes_* _p_stVpt,
                                                 HIE_tdstSuperObject_* _hSprObjSector,
                                                 s32 _lDrawMask);

extern MTH3D_tdstVector_* SECT_fn_p_stGetMaxPointInBorder(SECT_tdstSector_* _hSector);
extern MTH3D_tdstVector_* SECT_fn_p_stGetMinPointInBorder(SECT_tdstSector_* _hSector);
extern SECT_tdstListOfSOCharacters_* SECT_fn_hCreateElementLstCharacter(void);
extern s8 fn_cGetSectorCameraType(HIE_tdstSuperObject_* _hSprObjSector);
extern u8 SECT_fn_bIsThisSectorVirtual(HIE_tdstSuperObject_* _hSprObjSector);
extern void SECT_fn_vAddHeadCharListNode(HIE_tdstSuperObject_* hSector,
                                         SECT_tdstListOfSOCharacters_* hNode);
extern void SECT_fn_vAddTailCharListNode(HIE_tdstSuperObject_* hSector,
                                         SECT_tdstListOfSOCharacters_* hNode);
extern void SECT_fn_vIsolateCharListNode(SECT_tdstListOfSOCharacters_* hNode);
extern void SECT_fn_vSetCharacterPointerInCharacterList(
    SECT_tdstListOfSOCharacters_* hCharacterElement, HIE_tdstSuperObject_* hSectorSuperObject);
extern f32 fn_xGetSectorZFar(HIE_tdstSuperObject_* _hSprObjSector);
extern void SECT_fn_vGetMaxPointInBorder(const SECT_tdstSector_* _hSector,
                                         MTH3D_tdstVector_* _p_stMaxPoint);
extern void SECT_fn_vGetMinPointInBorder(const SECT_tdstSector_* _hSector,
                                         MTH3D_tdstVector_* _p_stMinPoint);
extern void SECT_fn_vSetHandleOfLightsInDNMLightsList(SECT_tdstListOfDNMLights_* hDNMLightsElement,
                                                      void* hDNMLights);
extern void inv_SpatialTree_vFindChildAtPoint(void* unused, MTH3D_tdstVector_* point,
                                              HIE_tdstSuperObject_** found);
extern SECT_tdstListOfDNMLights_* SECT_fn_hCreateElementLstDNMLights(void);
extern void SECT_fn_vAddTailDNMLightsListNode(HIE_tdstSuperObject_* hSector,
                                              SECT_tdstListOfDNMLights_* hNode);
extern void SECT_fn_vIsolateDNMLightsListNode(SECT_tdstListOfDNMLights_* hNode);
extern HIE_tdstSuperObject_* SECT_fn_hResearchInWhatSectorIAm(HIE_tdstSuperObject_* SECT_hFatherSector, MTH3D_tdstVector_* p_stAbsolutePosition);
extern s32 SECT_fn_bIsInBorderBox(MTH3D_tdstVector_* _p_stPosition, SECT_tdstSector_* _hSector);

extern s8 inv_SCT_bSendingSectorToViewport;
extern u8 inv_SCT_ucDrawPass;

struct Data3dShadow_inv;
extern f32 inv_SHW_fn_xGetShadowScaleX(struct Data3dShadow_inv* flags);
extern f32 inv_SHW_fn_xGetShadowScaleY(struct Data3dShadow_inv* flags);
extern void inv_SHW_fn_vSetShadowIncluded(struct Data3dShadow_inv* flags, s32 enabled);
extern void inv_SHW_fn_vSetShadowScaleX(struct Data3dShadow_inv* flags, f32 value);
extern void inv_SHW_fn_vSetShadowScaleY(struct Data3dShadow_inv* flags, f32 value);
extern void inv_SHW_fn_vSetShadowOn(struct Data3dShadow_inv* flags, s32 enabled);
extern s32 inv_SHW_fn_bIsShadowOn(struct Data3dShadow_inv* flags);

extern void SCT_fn_vSendSectorWhereIAmToViewportWithMirror(MTH3D_tdstVector_* _p_stAbsolutePositionOfCamera, s16 _hDevice, s16 _hViewport, struct GLD_tdstViewportAttributes_* _p_stVpt, HIE_tdstSuperObject_* _hSprObjSector, s32 _lDrawMask);

extern s32 dword_355570[];

#endif
