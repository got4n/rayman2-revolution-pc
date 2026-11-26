#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"
#include "snd/snd.h"
#include "AI/AI.h"

#define ONLY_LOCAL_LIGHT 0x20000
#define SECTOR_CHANGE_STEP 0.2f

tdstSectInfo_* fn_h_SectInfoRealAlloc(tdstEngineObject_* object);
void fn_vInsertLightInSectorList(HIE_tdstSuperObject_* sector, HIE_tdstSuperObject_* superObject);
void func_0010B550(HIE_tdstSuperObject_* previousSector, HIE_tdstSuperObject_* sector);

void fn_vSectInfoAlloc(tdstEngineObject_* p_stObject)
{
    p_stObject->h_SectInfo = fn_h_SectInfoRealAlloc(p_stObject);
}

tdstSectInfo_* fn_h_SectInfoRealAlloc(tdstEngineObject_* object)
{
    tdstSectInfo_* sectorInfo =
        AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstSectInfo_));

    sectorInfo->hNodeInSector = SECT_fn_hCreateElementLstCharacter();
    return sectorInfo;
}

void fn_vSectInfoInit(tdstEngineObject_* p_stObject, s32 eObjectInit)
{
    s32 type = eObjectInit & 0xFF;
    HIE_tdstSuperObject_* character;
    HIE_tdstSuperObject_* owner;
    gu_tdst3dData_* data;
    POS_tdstMatrix_* global;
    void* light;
    void* lightNode;

    switch (type) {
    case 1:
        p_stObject->h_SectInfo->hCurrentSector = NULL;
    case 2:
    case 3:
    case 4:
        owner = p_stObject->h_StandardGame->h_SuperObject;
        if (owner == NULL) {
            return;
        }
        data = p_stObject->h_3dData;
        fn_vSectInfoInitForSingleCharacter(owner, inv_POS_fn_p_stPosMatrixTranslation(&data->stGLIObjectMatrix));
        p_stObject->h_SectInfo->stPreviousPosition = *inv_POS_fn_p_stPosMatrixTranslation(&data->stGLIObjectMatrix);
        return;
    case 5:
        break;
    default:
        return;
    }

    character = p_stObject->h_StandardGame->h_SuperObject;
    if (character == NULL) {
        return;
    }
    global = character->gu_hGlobalMatrix;
    fn_vInsertObjectInSectorList(p_stObject->h_SectInfo->hCurrentSector, character);
    SECT_fn_vSetCharacterPointerInCharacterList(p_stObject->h_SectInfo->hNodeInSector, character);
    p_stObject->h_SectInfo->stPreviousPosition = *inv_POS_fn_p_stPosMatrixTranslation(global);
    light = inv_HIE_fn_p_stGetEngineObject(character)->h_MSLight;
    if (light == NULL) {
        return;
    }
    lightNode = fn_hMSLightGetLightNodeInSector(light);
    light = fn_p_stMSLightGetLight(light);
    SECT_fn_vSetHandleOfLightsInDNMLightsList(lightNode, light);
}

void fn_vSectInfoDesinit(tdstEngineObject_* p_stObject, s32 eObjectInit)
{
    s32 type = eObjectInit & 0xFF;
    HIE_tdstSuperObject_* superObject;

    if (type < 6 && type != 0) {
        superObject = p_stObject->h_StandardGame->h_SuperObject;
        if (superObject != NULL) {
            fn_vRemoveObjectInSectorList(superObject);
        }
    }
}

#ifdef PC_PORT
void fn_vSectInfoCopyClone(void* unused0, void* unused1)
#else
void fn_vSectInfoCopyClone(void)
#endif
{
}

void fn_vSectInfoInitForSingleCharacter(HIE_tdstSuperObject_* hCharacter, MTH3D_tdstVector_* hPosition)
{
    tdstSectInfo_* hSectInfo = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_SectInfo;
    HIE_tdstSuperObject_* testSO;
    void* light;
    void* lightNode;

    testSO = SECT_fn_hResearchInWhatSectorIAm(SECT_hFatherSector, hPosition);
    if (testSO == NULL) {
        testSO = HIE_fn_hGetChildNumber(SECT_hFatherSector,
                                        SECT_hFatherSector->lNumberOfElementsSta - 1);
    }
    hSectInfo->hCurrentSector = testSO;
    SECT_fn_vSetCharacterPointerInCharacterList(hSectInfo->hNodeInSector, hCharacter);
    light = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_MSLight;
    if (light != NULL) {
        lightNode = fn_hMSLightGetLightNodeInSector(light);
        SECT_fn_vSetHandleOfLightsInDNMLightsList(lightNode, fn_p_stMSLightGetLight(light));
    }
    fn_vInsertObjectInSectorList(hSectInfo->hCurrentSector, hCharacter);
}

void fn_vInsertObjectInSectorList(HIE_tdstSuperObject_* hSector, HIE_tdstSuperObject_* hCharacter)
{
    tdstSectInfo_* p_stSectInfo = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_SectInfo;

    SECT_fn_vIsolateCharListNode(p_stSectInfo->hNodeInSector);
    if (inv_fn_ucSuperObjectGetPlatFormType(hCharacter)) {
        SECT_fn_vAddHeadCharListNode(hSector, p_stSectInfo->hNodeInSector);
    } else {
        SECT_fn_vAddTailCharListNode(hSector, p_stSectInfo->hNodeInSector);
    }
    if (inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_MSLight != NULL) {
        fn_vInsertLightInSectorList(hSector, hCharacter);
    }
}

void fn_vInsertLightInSectorList(HIE_tdstSuperObject_* hSector, HIE_tdstSuperObject_* hCharacter)
{
    void* hMSLight = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_MSLight;

    if (fn_bMSLightGetLocalLight(hMSLight)) {
        if (fn_bMSLightGetOnlyLocalLight(hMSLight)) {
            hCharacter->flags |= ONLY_LOCAL_LIGHT;
        } else {
            hCharacter->flags &= ~ONLY_LOCAL_LIGHT;
        }
        return;
    }
    SECT_fn_vIsolateDNMLightsListNode(fn_hMSLightGetLightNodeInSector(hMSLight));
    SECT_fn_vAddTailDNMLightsListNode(hSector, fn_hMSLightGetLightNodeInSector(hMSLight));
}

void fn_vRemoveObjectInSectorList(HIE_tdstSuperObject_* hCharacter)
{
    SECT_fn_vIsolateCharListNode(inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_SectInfo->hNodeInSector);
    if (inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_MSLight == NULL) {
        return;
    }
    SECT_fn_vIsolateDNMLightsListNode(
        fn_hMSLightGetLightNodeInSector(inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_MSLight));
}

HIE_tdstSuperObject_* GAM_fn_hGetCurrentSector(HIE_tdstSuperObject_* _hCharacter)
{
    tdstSectInfo_* sectorInfo;

    if (_hCharacter->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
        return NULL;
    }
    sectorInfo = inv_HIE_fn_p_stGetEngineObject(_hCharacter)->h_SectInfo;
    if (sectorInfo == NULL) {
        return NULL;
    }
    return sectorInfo->hCurrentSector;
}

void func_0010B550(HIE_tdstSuperObject_* previousSector, HIE_tdstSuperObject_* sector)
{
    SECT_tdstSector_* data = sector->hLinkedObject;
    s32 remaining = data->stListOfSoundEvent.lNumberOfElementsOpt;
    SCT_tdstListOfSoundEvent_* sound = data->stListOfSoundEvent.hFirstElementOpt;

    while (remaining != 0) {
        if (sound->hPointerOfSectorSO == previousSector || sound->hPointerOfSectorSO == NULL) {
            if (sound->event != NULL) {
                SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMusic, *sound->event, 0, 0);
            }
        }
        remaining--;
        sound++;
    }
}

void GAM_fn_vSectInfoTestChangeSectorForCharacter(HIE_tdstSuperObject_* _hCharacter)
{
    MTH3D_tdstVector_ a2_stColons;
    MTH3D_tdstVector_ stAdding;
    MTH3D_tdstVector_ stPreviousRaisedPosition;
    MTH3D_tdstVector_ stCurrentRaisedPosition;
    MTH3D_tdstVector_ rotation[2];
    HIE_tdstSuperObject_* hPreviousSector;
    tdstSectInfo_* p_stSectInfo;
    SECT_tdstSector_* p_stSector;
    SECT_tdstListOfSOSectorsInActivityInteraction_* borders;
    POS_tdstMatrix_* global = _hCharacter->gu_hGlobalMatrix;
    s32 lNumberOfSoundEvent;
    s32 outside;

    POS_fn_vGetRotationVectors(global, &rotation[0], &rotation[1], &stCurrentRaisedPosition);
    MTH3D_M_vScaleXYZ_inv(&stCurrentRaisedPosition, &stCurrentRaisedPosition, SECTOR_CHANGE_STEP);

    p_stSectInfo = inv_HIE_fn_p_stGetEngineObject(_hCharacter)->h_SectInfo;
    global = _hCharacter->gu_hGlobalMatrix;
    a2_stColons = *inv_POS_fn_p_stPosMatrixTranslation(global);
    hPreviousSector = p_stSectInfo->hCurrentSector;
    stAdding = a2_stColons;
    stPreviousRaisedPosition = p_stSectInfo->stPreviousPosition;
    MTH3D_M_vAddXYZ_inv(&stAdding, &stAdding, &stCurrentRaisedPosition);
    MTH3D_M_vAddXYZ_inv(&stPreviousRaisedPosition, &stPreviousRaisedPosition,
                        &stCurrentRaisedPosition);
    inv_SpatialTree_vFindChildAtPoint(&stPreviousRaisedPosition, &stAdding, &hPreviousSector);

    if (hPreviousSector != p_stSectInfo->hCurrentSector) {
        if (_hCharacter == g_hCharacterLauchingSoundEvents) {
            func_0010B550(p_stSectInfo->hCurrentSector, hPreviousSector);
        }
        p_stSectInfo->hCurrentSector = hPreviousSector;

        if (!((inv_HIE_fn_p_stGetEngineObject(_hCharacter)->h_StandardGame->ucMiscFlags >> 4) & 1) &&
            !fn_bGetIfCharacterIsInAlwaysActiveCharacterList(_hCharacter)) {
            outside = p_stSectInfo->hCurrentSector != inv_GAM_currentSector;
            if (outside) {
                p_stSector = inv_GAM_currentSector->hLinkedObject;
                lNumberOfSoundEvent = p_stSector->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt;
                borders = p_stSector->stListOfSectorsInActivityInteraction.hFirstElementOpt;
                for (; lNumberOfSoundEvent > 0; lNumberOfSoundEvent--, borders++) {
                    if (borders->hPointerOfSectorSO == p_stSectInfo->hCurrentSector) {
                        outside = 0;
                        break;
                    }
                }
            }
            if (outside) {
                inv_fn_vIsolateStandardGame(inv_HIE_fn_p_stGetEngineObject(_hCharacter)->h_StandardGame);
                inv_HIE_fn_p_stGetEngineObject(_hCharacter)->h_StandardGame->ucMiscFlags &= ~4;
            }
        }
        fn_vInsertObjectInSectorList(p_stSectInfo->hCurrentSector, _hCharacter);
    }
    p_stSectInfo->stPreviousPosition = a2_stColons;
}
