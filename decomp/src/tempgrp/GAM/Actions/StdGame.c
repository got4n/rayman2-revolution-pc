#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "AI/AI.h"

gu_tdstStandardGame* fn_h_StdGameRealAlloc(tdstEngineObject_* object);
void fn_vStdGameInitValueSAI(gu_tdstStandardGame* standardGame);
void func_0010C928(gu_tdstStandardGame* standardGame);
void func_0010C930(gu_tdstStandardGame* standardGame);

static inline s64 inv_GAM_fn_lWiden(s32 value)
{
    return value;
}

static inline u8 inv_GAM_fn_bIsClear(u8 flags, u8 mask)
{
    return (flags & mask) == 0;
}

static inline void inv_GAM_fn_vInsertInSectorList(tdstEngineObject_* object)
{
    void* node = object->h_StandardGame->h_SuperObject;
    MTH3D_tdstVector_ position;

    node = ((HIE_tdstSuperObject_*) node)->gu_hGlobalMatrix;
    position = *inv_POS_fn_p_stPosMatrixTranslation(node);
    fn_vSectInfoInitForSingleCharacter(object->h_StandardGame->h_SuperObject, &position);
}

void fn_vStdGameAlloc(tdstEngineObject_* p_stObject)
{
    gu_tdstStandardGame* standardGame = fn_h_StdGameRealAlloc(p_stObject);

    p_stObject->h_StandardGame = standardGame;
    fn_vStdGameInitValueSAI(standardGame);
}

gu_tdstStandardGame* fn_h_StdGameRealAlloc(tdstEngineObject_* object)
{
    gu_tdstStandardGame* standardGame = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(gu_tdstStandardGame));
    s32 queue;

    standardGame->ucTransparencyZoneMax = 0x50;
    standardGame->queueKey0C = 0x40;
    standardGame->ucTransparencyZoneMin = 0x46;
    standardGame->ucMiscFlags = (standardGame->ucMiscFlags & ~1) | 2;
    standardGame->ucTooFarLimit = 0x46;
    queue = 0;
    standardGame->ucHitPoints = 0;
    standardGame->ucHitPointsMax = 0;
    standardGame->ulCustomBits = 0x200000;
    standardGame->ulAICustomBits = 0;
    standardGame->listNext = NULL;
    standardGame->listPrev = NULL;
    standardGame->listAnchor = NULL;
    if (inv_GAM_currentCharacterList == NULL || inv_GAM_previousCharacterList == NULL) {
        queue = 1;
    }
    if (inv_GAM_fn_lWiden(queue)) {
        inv_fn_vQueueEngineObjectByPriority(standardGame);
    }
    return standardGame;
}

void fn_vStdGameInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    gu_tdstStandardGame* standardGame;
    s32 type = eObjectInit;

    p_stObject->h_StandardGame->ulLastTrame = 0;
    standardGame = p_stObject->h_StandardGame;
    if (type == 2 || type == 4) {
        standardGame->ulCustomBits = standardGame->ulSaveCustomBits;
        p_stObject->h_StandardGame->ulAICustomBits = p_stObject->h_StandardGame->ulSaveAICustomBits;
    }

    switch (type) {
    case 2:
        if (!fn_ucIsAnAlwaysObject(p_stObject)) {
            p_stObject->h_StandardGame->ucMiscFlags &= ~1;
            p_stObject->h_StandardGame->ucMiscFlags |= 2;
            inv_GAM_fn_vInsertInSectorList(p_stObject);
        }
        break;
    case 5:
        if (!fn_ucIsAnAlwaysObject(p_stObject)) {
            p_stObject->h_StandardGame->ucMiscFlags |= 2;
            inv_GAM_fn_vInsertInSectorList(p_stObject);
        }
        break;
    case 4:
        if (!fn_ucIsAnAlwaysObject(p_stObject)) {
            if ((p_stObject->h_StandardGame->ucMiscFlags & 1) &&
                !((p_stObject->h_StandardGame->ucMiscFlags >> 1) & 1)) {
                break;
            }
            p_stObject->h_StandardGame->ucMiscFlags &= ~1;
            p_stObject->h_StandardGame->ucMiscFlags |= 2;
            inv_GAM_fn_vInsertInSectorList(p_stObject);
        }
        break;
    case 3:
        if (!fn_ucIsAnAlwaysObject(p_stObject)) {
            if (p_stObject->h_StandardGame->eInitFlagWhenDeadOrTaken == 5) {
                if (inv_GAM_fn_bIsClear(p_stObject->h_StandardGame->ucMiscFlags, 1)) {
                    p_stObject->h_StandardGame->ucMiscFlags |= 2;
                } else {
                    p_stObject->h_StandardGame->ucMiscFlags &= ~2;
                }
            }
            inv_GAM_fn_vInsertInSectorList(p_stObject);
        }
        break;
    case 1:
        func_0010C928(p_stObject->h_StandardGame);
        p_stObject->h_StandardGame->ucMiscFlags |= 2;
        if (p_stObject->h_StandardGame->ulCustomBits & 0x80000000) {
            func_0010C930(p_stObject->h_StandardGame);
        }
    case 0:
        if (!((p_stObject->h_StandardGame->ucMiscFlags >> 3) & 1)) {
            fn_vStdGameInitValueSAI(p_stObject->h_StandardGame);
        }
        break;
    }

    standardGame = p_stObject->h_StandardGame;
    if (!(standardGame->ulCustomBits & 0x80000000) && (type == 2 || type == 4)) {
        standardGame->ucHitPoints = standardGame->ucSaveHitPoints;
    } else if (type == 0) {
        standardGame->ucHitPoints = standardGame->ucSaveHitPoints;
    }
    if (type == 1 || type == 2) {
        standardGame->ucMiscFlags =
            (standardGame->ucMiscFlags & ~7) | standardGame->ucSaveMiscFlags;
    }
}

void fn_vStdGameDesinit(tdstEngineObject_* p_stObject, s32 eObjectInit)
{
    eObjectInit &= 0xFF;
    if (eObjectInit < 6 && eObjectInit != 0) {
        p_stObject->h_StandardGame->ucMiscFlags &= ~2;
    }
}

void fn_vStdGameInitValueSAI(gu_tdstStandardGame* standardGame)
{
    standardGame->ucSaveHitPoints = standardGame->ucHitPoints;
    standardGame->ucSaveHitPointsMax = standardGame->ucHitPointsMax;
    standardGame->ucSaveMiscFlags = standardGame->ucMiscFlags & 7;
    standardGame->ulSaveCustomBits = standardGame->ulCustomBits;
    standardGame->ulSaveAICustomBits = standardGame->ulAICustomBits;
}

void func_0010C928(gu_tdstStandardGame* standardGame)
{
}

void func_0010C930(gu_tdstStandardGame* standardGame)
{
}

void func_0010C938(void)
{
}

void func_0010C940(void)
{
}

void fn_vSetHitPointsToInitValue(tdstEngineObject_* _p_stObject)
{
    _p_stObject->h_StandardGame->ucHitPoints = _p_stObject->h_StandardGame->ucSaveHitPoints;
}

void fn_vSetHitPointsMaxToInitValue(tdstEngineObject_* _p_stObject)
{
    _p_stObject->h_StandardGame->ucHitPointsMax = _p_stObject->h_StandardGame->ucSaveHitPointsMax;
}

void fn_vStdGameCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    *_p_stObjectDest->h_StandardGame = *_p_stObjectSrc->h_StandardGame;
}

u32 fn_ulStandardGameGetCustomBitsSO(HIE_tdstSuperObject_* hSupObj)
{
    tdstEngineObject_* object = hSupObj->hLinkedObject;

    return object->h_StandardGame->ulCustomBits;
}
