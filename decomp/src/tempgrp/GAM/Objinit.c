#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "snd/snd.h"
#include "GLI/GLI.h"

#define CHANNEL_COUNT (sizeof(tdstEngineObject_) / sizeof(void*))

void inv_fn_vDesinitAllChannelsOfObject(tdstEngineObject_* object, u8 mode);
void fn_vInitAllObjectTree(u8 mode, HIE_tdstSuperObject_* node);

static inline POS_tdstMatrix_* inv_GAM_fn_p_stGlobalMatrixOf(HIE_tdstSuperObject_* superObject)
{
    return superObject->gu_hGlobalMatrix;
}

static inline u8 inv_GAM_fn_bIsActive(u8 flags)
{
    return flags >> 1 & 1;
}

static inline u8 inv_GAM_fn_bIsInFixedList(gu_tdstStandardGame* standardGame)
{
    u8 fixed = 0;

    if (standardGame->listAnchor == inv_GAM_currentCharacterList ||
        standardGame->listAnchor == inv_GAM_previousCharacterList) {
        fixed = 1;
    }
    return fixed;
}

void fn_vDestroyAllHierarchy(HIE_tdstSuperObject_* p_stSuperObject)
{
    HIE_tdstSuperObject_* node = p_stSuperObject->hFirstElementSta;
    HIE_tdstSuperObject_* next = HIE_fn_hForceGetSuperObjectNextBrother(node);

    while (node != NULL) {
        fn_vDestroyAllHierarchy(node);
        node = next;
        next = HIE_fn_hForceGetSuperObjectNextBrother(node);
    }
    HIE_fn_vIsolate(p_stSuperObject);
}

void fn_vInitOneObject(tdstEngineObject_* p_stObject, u8 eObjIniType)
{
    u32 i;

    for (i = 0; i < CHANNEL_COUNT; i++) {
        if (inv_GAM_fn_pp_vChannelsOf(p_stObject)[i] != NULL) {
            off_2523D0[i].init(p_stObject, eObjIniType);
        }
    }
    if (eObjIniType == OTI_MapLoaded) {
        p_stObject->h_StandardGame->ucMiscFlags |= 8;
    }
}

void inv_fn_vDesinitAllChannelsOfObject(tdstEngineObject_* object, u8 mode)
{
    u32 i;

    for (i = 0; i < CHANNEL_COUNT; i++) {
        if (inv_GAM_fn_pp_vChannelsOf(object)[i] != NULL) {
            off_2523D0[i].desinit(object, mode);
        }
    }
}

void fn_vForceActiveAnObject(tdstEngineObject_* p_stObject)
{
    gu_tdstStandardGame* standardGame = p_stObject->h_StandardGame;
    HIE_tdstSuperObject_* owner;
    void* fatherMatrix;
    POS_tdstMatrix_* local;
    gu_tdst3dData_* data;
    tdstFrame3d_* elements;
    tdstState_* state;
    tdstAnim3d_* animation;
    MTH3D_tdstVector_ position;

    if (!inv_GAM_fn_bIsInFixedList(standardGame)) {
        inv_fn_vQueueEngineObjectByPriority(standardGame);
    }
    owner = p_stObject->h_StandardGame->h_SuperObject;
    fatherMatrix = owner->hFatherSta->gu_hGlobalMatrix;
    local = owner->gu_hLocalMatrix;
    position = *inv_POS_fn_p_stPosMatrixTranslation(local);
    POS_fn_vMulMatrixVertex(&position, fatherMatrix, &position);
    fn_vSectInfoInitForSingleCharacter(p_stObject->h_StandardGame->h_SuperObject, &position);
    p_stObject->h_StandardGame->ucMiscFlags |= 2;
    p_stObject->h_StandardGame->ucMiscFlags |= 4;
    p_stObject->h_3dData->ulStartTime = inv_fn_ulGetEngineClock();
    p_stObject->h_3dData->ulTimePreviousFrame = inv_fn_ulGetEngineClock();
    data = p_stObject->h_3dData;
    elements = &data->stFrame3d;
    if (elements->p_stArrayOfElts3d == NULL) {
        state = data->h_CurrentState;
        if (state != NULL) {
            animation = state->p_stAnim;
            if (animation != NULL) {
                elements->p_stAnim = 0;
                fn_vAllocArrayOfElts3d(p_stObject->h_3dData, animation->ucMaxNumberOfElements);
            }
        }
        p_stObject->h_3dData->stFrame3d.p_stAnim = 0;
    }
}

void fn_vDesactiveAnObject(tdstEngineObject_* p_stObject)
{
    gu_tdstStandardGame* standardGame;
    u8 flags;

    PLA_fn_vDesInitAllChildOfCharacter(p_stObject->h_StandardGame->h_SuperObject, p_stObject);
    SND_fn_vKillObjectSound2((s32) p_stObject->h_StandardGame->h_SuperObject,
                              g_lSoundObjectTypeAnim);
    standardGame = p_stObject->h_StandardGame;
    flags = standardGame->ucMiscFlags;
    if (flags & 1) {
        return;
    }
    if (inv_GAM_fn_bIsActive(flags) == 1) {
        standardGame->ucMiscFlags = flags & ~2;
        p_stObject->h_StandardGame->ucMiscFlags &= ~1;
        p_stObject->h_StandardGame->ucMiscFlags &= ~4;
    }
}

void fn_vDesinitOneObject(tdstEngineObject_* object)
{
    gu_tdstStandardGame* standardGame;
    u8 flags;

    PLA_fn_vDesInitAllChildOfCharacter(object->h_StandardGame->h_SuperObject, object);
    standardGame = object->h_StandardGame;
    flags = standardGame->ucMiscFlags;
    if (!(flags & 1)) {
        standardGame->ucMiscFlags = flags | 1;
        standardGame = object->h_StandardGame;
        if (standardGame->eInitFlagWhenDeadOrTaken == 5) {
            standardGame->ucMiscFlags &= ~2;
        } else {
            standardGame->ucMiscFlags |= 2;
        }
        standardGame = object->h_StandardGame;
        if (standardGame->eInitFlagWhenDeadOrTaken == 1) {
            fn_vInitOneObject(object, 4);
            inv_fn_vQueueEngineObjectByPriority(object->h_StandardGame);
        } else {
            standardGame->ucMiscFlags &= ~4;
            if (object->h_StandardGame->ucMiscFlags >> 1 & 1) {
                inv_fn_vMoveToInactiveCharacterList(object->h_StandardGame);
            }
        }
    }
    SND_fn_vKillObjectSound2((s32) object->h_StandardGame->h_SuperObject,
                              g_lSoundObjectTypeAnim);
}

void fn_vKillEngineObjectOrAlwaysByPointer(tdstEngineObject_* p_stObject)
{
    gu_tdstStandardGame* standardGame = p_stObject->h_StandardGame;
    u16 personalType;

    if (standardGame->listNext != NULL)
        standardGame->listNext->listPrev = standardGame->listPrev;
    if (standardGame->listPrev != NULL)
        standardGame->listPrev->listNext = standardGame->listNext;
    if (standardGame->listAnchor != NULL) {
        standardGame->listAnchor->count--;
        if (standardGame->listAnchor->first == standardGame)
            standardGame->listAnchor->first = standardGame->listNext;
        if (standardGame->listAnchor->last == standardGame)
            standardGame->listAnchor->last = standardGame->listPrev;
    }
    standardGame->listNext = NULL;
    standardGame->listPrev = NULL;
    standardGame->listAnchor = NULL;
    fn_vDesinitOneObject(p_stObject);
    standardGame = p_stObject->h_StandardGame;
    personalType = standardGame->otObjectPersonalType;
    if ((s16) standardGame->otObjectPersonalType < 0)
        fn_vKillAlwaysByPersonalType(personalType);
}

void fn_vInitAllObjectsWhenNewGameStart(void)
{
}

void fn_vInitAllObjectsWhenMapJustLoaded(void)
{
    HIE_fn_vInitMatrixStack();
    fn_vInitAllObjectTree(OTI_MapLoaded, gp_stDynamicWorld);
    fn_vInitAlwaysModel(OTI_MapLoaded);
}

void inv_fn_vReinitWorldHierarchy(u8 mode)
{
    fn_vInitAllObjectTree(mode, gp_stDynamicWorld);
}

void fn_vInitAllObjectsWhenSavedGameJustLoaded(void)
{
    HIE_fn_vInitMatrixStack();
    inv_fn_vReinitWorldHierarchy(OTI_LoadSavedGame);
    fn_vInitAlwaysModel(OTI_LoadSavedGame);
}

void fn_vForceReinitAllObjects(void)
{
    HIE_fn_vInitMatrixStack();
    inv_fn_vReinitWorldHierarchy(OTI_ReinitTheMap);
    fn_vInitAlwaysModel(OTI_ReinitTheMap);
    CAM_fn_vReInitCameras();
    func_001003B0();
}

void fn_vSnifThePlayerIsDead(void)
{
    gu_MTH4D_M_vSetIdentityMatrix(&g_stEngineStructure.stMainCharacterPosition);
    gu_MTH4D_M_vSetIdentityMatrix(&g_stEngineStructure.stMainCameraPosition);
    g_stEngineStructure.bResurection = 0;
    inv_SND_fn_vStopThemeIfPlaying();
    SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeAnim);
    SND_fn_vKillAllObjectTypeSound(inv_SND_moduleObjectSoundType);
    SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeVoice);
    SND_fn_vKillAllObjectTypeSound(inv_SND_moduleVoiceObjectSoundType);
    fn_vChangeEngineMode(8);
}

void fn_vSnifThePlayerIsDeadWithResurrectionAt(HIE_tdstSuperObject_* _hCharacterPositionOne,
                                                HIE_tdstSuperObject_* _hCharacterPositionTwo)
{
    HIE_tdstSuperObject_* active = g_stEngineStructure.activeSuperObject;

    g_stEngineStructure.bResurection = 0;
    if (_hCharacterPositionOne != NULL) {
        if (active != NULL) {
            g_stEngineStructure.stMainCharacterPosition = *inv_GAM_fn_p_stSavedMatrixOf(active);
            *inv_GAM_fn_p_stSavedMatrixOf(active) = *inv_GAM_fn_p_stGlobalMatrixOf(_hCharacterPositionOne);
            g_stEngineStructure.bResurection = 1;
        }
    } else if (active != NULL) {
        g_stEngineStructure.stMainCharacterPosition = *inv_GAM_fn_p_stSavedMatrixOf(active);
        g_stEngineStructure.bResurection = 1;
    }
    if (_hCharacterPositionTwo != NULL) {
        if (g_stEngineStructure.h_StdCamCaracter != NULL) {
            g_stEngineStructure.stMainCameraPosition = *inv_GAM_fn_p_stSavedMatrixOf(g_stEngineStructure.h_StdCamCaracter);
            *inv_GAM_fn_p_stSavedMatrixOf(g_stEngineStructure.h_StdCamCaracter) = *inv_GAM_fn_p_stGlobalMatrixOf(_hCharacterPositionTwo);
            g_stEngineStructure.bResurection = 1;
        }
    } else if (g_stEngineStructure.h_StdCamCaracter != NULL) {
        g_stEngineStructure.stMainCameraPosition = *inv_GAM_fn_p_stSavedMatrixOf(g_stEngineStructure.h_StdCamCaracter);
        g_stEngineStructure.bResurection = 1;
    }
    SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeAnim);
    SND_fn_vKillAllObjectTypeSound(inv_SND_moduleObjectSoundType);
    SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeVoice);
    SND_fn_vKillAllObjectTypeSound(inv_SND_moduleVoiceObjectSoundType);
    fn_vChangeEngineMode(8);
}

void fn_vInitAllObjectsWhenPlayerDead(void)
{
    HIE_fn_vInitMatrixStack();
    inv_fn_vReinitWorldHierarchy(OTI_PlayerDead);
    fn_vInitAlwaysModel(OTI_PlayerDead);
}

void fn_vInitAllObjectTree(u8 eObjIniType, HIE_tdstSuperObject_* p_stSupObject)
{
    HIE_tdstSuperObject_* child;
    tdstEngineObject_* object;
    gu_tdstStandardGame* standardGame;
    POS_tdstMatrix_* global;
    tdstFamilyListView* family;

    HIE_fn_vPushSuperObjectMatrix(p_stSupObject);
    global = p_stSupObject->gu_hGlobalMatrix;
    *global = *g_p_stCurrentMatrix;
    HIE_fn_vInvalidateOneGlobalMatrix(p_stSupObject);
    for (child = p_stSupObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        fn_vInitAllObjectTree(eObjIniType, child);
    }
    HIE_fn_vPopMatrix();

    object = inv_HIE_fn_p_stGetEngineObject(p_stSupObject);
    if (p_stSupObject->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
        return;
    }
    switch (eObjIniType) {
    case OTI_MapLoaded:
    case OTI_LoadSavedGame:
        standardGame = object->h_StandardGame;
        if (standardGame->eInitFlagWhenDeadOrTaken == 0   ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 1 ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 2 ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 3 ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 4
                ? 1
                : standardGame->eInitFlagWhenDeadOrTaken == 5 && !(standardGame->ucMiscFlags & 1)) {
            if (eObjIniType == OTI_LoadSavedGame && object->h_StandardGame->eInitFlagWhenDeadOrTaken == 4) {
                fn_vInitOneObject(object, OTI_ReinitTheMap);
            } else {
                fn_vInitOneObject(object, eObjIniType);
            }
        } else {
            inv_fn_vDesinitAllChannelsOfObject(object, eObjIniType);
        }
        family = inv_HIE_fn_p_stGetEngineObject(p_stSupObject)->h_3dData->h_Family;
        p_stSupObject->hVisualBoundingVolume = family->hBoundingSphere;
        p_stSupObject->hCollideBoundingVolume = family->hBoundingSphere;
        break;
    case OTI_ReinitTheMap:
        fn_vInitOneObject(object, eObjIniType);
        break;
    case OTI_PlayerDead:
    case OTI_AlwaysCreated:
        standardGame = object->h_StandardGame;
        if (standardGame->eInitFlagWhenDeadOrTaken == 0   ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 1 ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 2 ? 1
            : standardGame->eInitFlagWhenDeadOrTaken == 5 && !(standardGame->ucMiscFlags & 1)
                ? 1
                : standardGame->eInitFlagWhenDeadOrTaken == 3 && !(standardGame->ucMiscFlags & 1)) {
            fn_vInitOneObject(object, eObjIniType);
        } else {
            inv_fn_vDesinitAllChannelsOfObject(object, eObjIniType);
        }
        break;
    }
    HIE_fn_vPushSuperObjectMatrix(p_stSupObject);
    HIE_fn_vPopMatrix();
    if (inv_HIE_fn_p_stGetEngineObject(p_stSupObject)->h_Dynam == NULL) {
        return;
    }
    fn_vDynamInit(object, eObjIniType);
}
