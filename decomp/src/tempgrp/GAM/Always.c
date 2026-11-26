#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "SCT/SCT.h"

#define CHANNEL_COUNT (sizeof(tdstEngineObject_) / sizeof(void*))

extern LST2_tdstAnchorToALW_tdxHandleToModelListDyn inv_ALW_h_LstAlwaysModel;

static inline s32 inv_GAM_fn_bIsSuperimposed(HIE_tdstSuperObject_* superObject)
{
    return (superObject->flags >> 5) & 1;
}

static inline s32 inv_GAM_fn_lTestBit(u32 value, s32 bit)
{
    return (value >> bit) & 1;
}

void fn_vKillAllAlwaysModel(void)
{
    fn_vKillAllAlways();
    if (g_stAlways.ulMaxNumberOfAlways != 0) {
        g_stAlways.ulMaxNumberOfAlways = 0;
        g_stAlways.d_stAlwaysEngineObjectInit = NULL;
        g_stAlways.d_stAlwaysSuperObject = NULL;
        g_stAlways.d_hAlwaysGenerator = NULL;
        g_stAlways.used = NULL;
    }
}

void fn_vInitAlwaysModel(u8 eObjIniType)
{
    tdstAlwaysModelList_* h_AlwaysModelChild;

    for (h_AlwaysModelChild = inv_ALW_h_LstAlwaysModel.hFirstElementDyn; h_AlwaysModelChild != NULL;
         h_AlwaysModelChild = h_AlwaysModelChild->hNextBrotherDyn) {
        fn_vInitOneObject(h_AlwaysModelChild->p_stAlwaysObject, eObjIniType);
    }
}

static inline u16 inv_fn_uwGetPersonalType(tdstEngineObject_* p_stEngineObject)
{
    return p_stEngineObject->h_StandardGame->otObjectPersonalType;
}

u8 fn_ucIsAnAlwaysObject(tdstEngineObject_* p_stEngineObject)
{
    tdstAlwaysModelList_* h_AlwaysModelChild;

    if ((s16)inv_fn_uwGetPersonalType(p_stEngineObject) < 0)
        return 1;
    for (h_AlwaysModelChild = inv_ALW_h_LstAlwaysModel.hFirstElementDyn; h_AlwaysModelChild != NULL;
         h_AlwaysModelChild = h_AlwaysModelChild->hNextBrotherDyn) {
        if (inv_fn_uwGetPersonalType(p_stEngineObject) ==
            inv_fn_uwGetPersonalType(h_AlwaysModelChild->p_stAlwaysObject))
            return 1;
    }
    return 0;
}

HIE_tdstSuperObject_* fn_p_stAllocateAlways(u16 otObjectModelType,
                                            HIE_tdstSuperObject_* p_stFatherSuperObject,
                                            HIE_tdstSuperObject_* _hGenerator, s32 uwAction,
                                            POS_tdstMatrix_* p_stMatrix)
{
    tdstAlwaysModelList_* h_AlwaysModelChild;

    for (h_AlwaysModelChild = inv_ALW_h_LstAlwaysModel.hFirstElementDyn; h_AlwaysModelChild != NULL;
         h_AlwaysModelChild = h_AlwaysModelChild->hNextBrotherDyn) {
        HIE_tdstSuperObject_* h_stSuperObject;
        tdstEngineObject_* p_stEngineObject;
        HIE_tdstSuperObject_* h_SOSector;
        MTH3D_tdstVector_* position;
        u32 InstNb;
        u32 MsNb;

        if (h_AlwaysModelChild->otObjectModelType != otObjectModelType) {
            continue;
        }

        InstNb = 0;
        h_stSuperObject = g_stAlways.d_stAlwaysSuperObject;
        while (InstNb < g_stAlways.ulMaxNumberOfAlways) {
            if (g_stAlways.d_hAlwaysGenerator[InstNb] == NULL &&
                h_stSuperObject->hNextBrotherSta == NULL)
                break;
            InstNb++;
            h_stSuperObject++;
        }
        if (InstNb >= g_stAlways.ulMaxNumberOfAlways)
            return NULL;

        g_stAlways.d_hAlwaysGenerator[InstNb] = _hGenerator;
        g_stAlways.used[InstNb] = 1;
        h_stSuperObject->gu_lDrawModeMask = -1;
        p_stEngineObject = h_stSuperObject->hLinkedObject;
        h_stSuperObject->flags = 0;

        for (MsNb = 0; MsNb < CHANNEL_COUNT; MsNb++) {
            if (inv_GAM_fn_pp_vChannelsOf(h_AlwaysModelChild->p_stAlwaysObject)[MsNb] != NULL) {
                if (inv_GAM_fn_pp_vChannelsOf(&g_stAlways.d_stAlwaysEngineObjectInit[InstNb])[MsNb] == NULL)
                    off_2523D0[MsNb].create(
                        inv_GAM_fn_pp_vChannelsOf(&g_stAlways.d_stAlwaysEngineObjectInit[InstNb]));
                inv_GAM_fn_pp_vChannelsOf(p_stEngineObject)[MsNb] =
                    inv_GAM_fn_pp_vChannelsOf(&g_stAlways.d_stAlwaysEngineObjectInit[InstNb])[MsNb];
                off_2523D0[MsNb].copy(p_stEngineObject, h_AlwaysModelChild->p_stAlwaysObject);
            }
        }

        p_stEngineObject->h_StandardGame->otObjectPersonalType = InstNb - 0x8000;
        p_stEngineObject->h_StandardGame->h_SuperObject = h_stSuperObject;

        if (p_stEngineObject->h_3dData != NULL)
            p_stEngineObject->h_3dData->stGLIObjectMatrix = *p_stMatrix;

        _POS_fn_vMulMatrixMatrix(h_stSuperObject->gu_hGlobalMatrix,
                                 p_stFatherSuperObject->gu_hGlobalMatrix,
                                 h_stSuperObject->gu_hLocalMatrix);
        h_stSuperObject->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;

        position = (MTH3D_tdstVector_*) &h_stSuperObject->gu_hGlobalMatrix->stMatrix.stCol_3;
        h_SOSector = GAM_fn_hGetCurrentSector(_hGenerator);
        if (!SECT_fn_bIsInBorderBox(position, h_SOSector->hLinkedObject))
            h_SOSector = SECT_fn_hResearchInWhatSectorIAm(SECT_hFatherSector, position);
        if (h_SOSector == NULL) {
            g_stAlways.d_hAlwaysGenerator[InstNb] = NULL;
            return NULL;
        }

        p_stEngineObject->h_SectInfo->hCurrentSector = h_SOSector;

        for (MsNb = 0; MsNb < CHANNEL_COUNT; MsNb++) {
            if (inv_GAM_fn_pp_vChannelsOf(p_stEngineObject)[MsNb] != NULL)
                off_2523D0[MsNb].init(p_stEngineObject, 5);
        }

        if (inv_fn_ucSuperObjectGetPlatFormType(h_stSuperObject))
            HIE_fn_vSuperObjectAddHead(p_stFatherSuperObject, h_stSuperObject);
        else
            HIE_fn_vSuperObjectAddTail(p_stFatherSuperObject, h_stSuperObject);

        p_stEngineObject->h_StandardGame->ucMiscFlags |= 2;
        inv_fn_vQueueEngineObjectByPriority(
            inv_HIE_fn_p_stGetEngineObject(h_stSuperObject)->h_StandardGame);
        h_stSuperObject->hVisualBoundingVolume =
            fn_p_stGetAnimInState(p_stEngineObject->h_3dData->h_Family);
        h_stSuperObject->hCollideBoundingVolume =
            fn_p_stGetAnimInState(p_stEngineObject->h_3dData->h_Family);
        p_stEngineObject->h_StandardGame->ucMiscFlags |= 4;
        p_stEngineObject->h_3dData->ulStartTime = inv_fn_ulGetEngineClock();
        p_stEngineObject->h_3dData->ulTimePreviousFrame = inv_fn_ulGetEngineClock();

        if (p_stEngineObject->h_3dData->stFrame3d.p_stArrayOfElts3d == NULL) {
            tdstState_* h_CurrentState = p_stEngineObject->h_3dData->h_CurrentState;

            if (h_CurrentState != NULL && h_CurrentState->p_stAnim != NULL) {
                fn_vAllocArrayOfElts3d(p_stEngineObject->h_3dData,
                                        h_CurrentState->p_stAnim->ucMaxNumberOfElements);
                p_stEngineObject->h_3dData->stFrame3d.p_stAnim = 0;
            }
            p_stEngineObject->h_3dData->stFrame3d.p_stAnim = 0;
        }
        return h_stSuperObject;
    }
    return NULL;
}

void fn_vKillAlwaysByPersonalType(u16 otPersonalType)
{
    s32 ulRank = otPersonalType - 0x8000;
    HIE_tdstSuperObject_* h_stSuperObject = &g_stAlways.d_stAlwaysSuperObject[ulRank];
    tdstEngineObject_* p_stEngineObject;
    HIE_tdstSuperObject_* h_Child;
    HIE_tdstSuperObject_* h_NextChild;

    TFMT_ucClearNumberOfCurrentTextAffiche(h_stSuperObject);
    p_stEngineObject = h_stSuperObject->hLinkedObject;
    g_stAlways.d_hAlwaysGenerator[ulRank] = NULL;
    g_stAlways.used[ulRank] = 0;
    fn_vRemoveObjectInSectorList(h_stSuperObject);
    p_stEngineObject->h_StandardGame->otObjectFamilyType = 0xFFFF;
    p_stEngineObject->h_StandardGame->otObjectModelType = 0xFFFF;
    p_stEngineObject->h_StandardGame->otObjectPersonalType = 0x8000;
    PLA_fn_vDesInitAllChildOfCharacter(h_stSuperObject, h_stSuperObject->hLinkedObject);
    fn_vFreeMorphList(&p_stEngineObject->h_3dData->hMorphChannelList);
    if (p_stEngineObject->h_Dynam != NULL)
        fn_v_DynamicFree(p_stEngineObject->h_Dynam);
    HIE_fn_vPseudoIsolate(h_stSuperObject);

    if (inv_fn_ucSuperObjectGetPlatFormType(h_stSuperObject)) {
        for (h_Child = h_stSuperObject->hFirstElementSta,
            h_NextChild = HIE_fn_hForceGetSuperObjectNextBrother(h_Child);
             h_Child != NULL;
             h_Child = h_NextChild, h_NextChild = HIE_fn_hForceGetSuperObjectNextBrother(h_Child)) {
            if (h_Child->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv)
                fn_vReputCharacterSuperObjectAtTheWorld(h_Child);
        }
    }

    if (inv_GAM_fn_bIsSuperimposed(h_stSuperObject))
        HIE_fn_SO_vClearSuperimposedFlag(h_stSuperObject);
    memset(p_stEngineObject, 0, sizeof(tdstEngineObject_));
}

void fn_vKillAllAlways(void)
{
    u32 i;

    for (i = 0; i < g_stAlways.ulMaxNumberOfAlways; i++) {
        HIE_tdstSuperObject_* p_stAlwaysSuperObject = &g_stAlways.d_stAlwaysSuperObject[i];

        if (inv_GAM_fn_bIsSuperimposed(p_stAlwaysSuperObject)) {
            tdstSectInfo_* p_stSectInfo = inv_HIE_fn_p_stGetEngineObject(p_stAlwaysSuperObject)->h_SectInfo;
            HIE_tdstSuperObject_* p_stAlwaysModel;

            HIE_fn_SO_vClearSuperimposedFlag(p_stAlwaysSuperObject);
            p_stAlwaysModel = inv_HIE_fn_p_stGetEngineObject(g_stEngineStructure.activeSuperObject)
                             ->h_SectInfo->hCurrentSector;
            p_stSectInfo->hCurrentSector = p_stAlwaysModel;
            fn_vInsertObjectInSectorList(p_stAlwaysModel, p_stAlwaysSuperObject);
        }

        if (inv_HIE_fn_p_stGetEngineObject(p_stAlwaysSuperObject)->h_StandardGame != NULL)
            fn_vKillEngineObjectOrAlwaysByPointer(p_stAlwaysSuperObject->hLinkedObject);
    }
}

u32 fn_ulGetNumberOfAlwaysBelongingToMe(HIE_tdstSuperObject_* _hGenerator)
{
    u32 ulReturn = 0;
    u32 i;

    for (i = 0; i < g_stAlways.ulMaxNumberOfAlways; i++) {
        tdstEngineObject_* hAlwaysObject =
            inv_HIE_fn_p_stGetEngineObject(&g_stAlways.d_stAlwaysSuperObject[i]);

        if (hAlwaysObject != NULL && hAlwaysObject->h_3dData != NULL &&
            g_stAlways.d_hAlwaysGenerator[i] == _hGenerator)
            ulReturn++;
    }
    return ulReturn;
}

void fn_vKillAllAlwaysInNonActiveSectors(void)
{
    HIE_tdstSuperObject_* hAlwaysSuperObject = g_stAlways.d_stAlwaysSuperObject;
    HIE_tdstSuperObject_** p_hGenerator = g_stAlways.d_hAlwaysGenerator;
    HIE_tdstSuperObject_** p_hLastGenerator = p_hGenerator + g_stAlways.ulMaxNumberOfAlways;

    for (; p_hGenerator < p_hLastGenerator; p_hGenerator++, hAlwaysSuperObject++) {
        tdstEngineObject_* p_stAlwaysEngObj = hAlwaysSuperObject->hLinkedObject;
        gu_tdstStandardGame* p_stStandardGame = p_stAlwaysEngObj->h_StandardGame;

        if (p_stStandardGame != NULL) {
            if ((p_stStandardGame->eInitFlagWhenOutOfZone == 7 &&
                 inv_GAM_fn_lTestBit(inv_HIE_fn_p_stGetEngineObject(*p_hGenerator)->h_StandardGame->ucMiscFlags,
                         2) != 1) ||
                ((p_stStandardGame->ulCustomBits & 0x20000) &&
                 PLA_fn_bTestIfEndOfAction(hAlwaysSuperObject))) {
                fn_vKillEngineObjectOrAlwaysByPointer(p_stAlwaysEngObj);
            }
        }
        if (*p_hGenerator == NULL)
            hAlwaysSuperObject->hNextBrotherSta = NULL;
    }
}

s32 fn_ucIsAlwaysGeneratedByMe(HIE_tdstSuperObject_* _hAlways, HIE_tdstSuperObject_* _hGenerator)
{
    if (_hGenerator == NULL)
        return 0;
    return inv_ALWAYS_fn_hGetGeneratorOfAlways(_hAlways) == _hGenerator;
}

HIE_tdstSuperObject_* inv_ALWAYS_fn_hGetGeneratorOfAlways(HIE_tdstSuperObject_* _hAlways)
{
    s32 ulRank;

    if (_hAlways == NULL || inv_HIE_fn_p_stGetEngineObject(_hAlways)->h_StandardGame == NULL ||
        inv_HIE_fn_p_stGetEngineObject(_hAlways) == NULL)
        return NULL;

    ulRank = inv_HIE_fn_p_stGetEngineObject(_hAlways)->h_StandardGame->otObjectPersonalType - 0x8000;
    return g_stAlways.d_hAlwaysGenerator[ulRank];
}
