#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"

typedef s32 (*TextTableKindQuery)(tdstAnim3d_* p_stAnim, s32 channel, u32 frame);

void fn_vStandardGameSetCustomBitsSO(HIE_tdstSuperObject_* superObject, u32 customBits)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    object->h_StandardGame->ulCustomBits = customBits;
}

s32 fn_ucStandardGameGetTransparencyZoneMinSO(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;
    gu_tdstStandardGame* standardGame = object->h_StandardGame;

    if (standardGame != NULL) {
        return standardGame->ucTransparencyZoneMin;
    }
    return 0x46;
}

s32 fn_ucStandardGameGetTransparencyZoneMaxSO(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;
    gu_tdstStandardGame* standardGame = object->h_StandardGame;

    if (standardGame != NULL) {
        return standardGame->ucTransparencyZoneMax;
    }
    return 0x50;
}

u8 inv_fn_ucSuperObjectGetPlatFormType(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return object->h_StandardGame->ucPlatFormType;
}

s32 fn_ucStdGameCheckValidity(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return object->h_StandardGame != NULL;
}

void inv_TXT_fn_vInitTextTables(void)
{
    inv_TXT_textTableState.nbLanguages = 0;
    inv_TXT_textTableState.language = 0xFF;
    inv_TXT_textTableState.count = 0;
    inv_TXT_textTableState.languageTexts = NULL;
    inv_TXT_textTableState.highIdTexts = NULL;
    inv_TXT_textTableState.texts = NULL;
}

void func_0010CAC8(void)
{
}

s32 inv_PLA_fn_lGetMorphingAmount(tdstAnim3d_* p_stAnim, s32 channel, u16 frame)
{
    s32 count;
    s32 i;

    p_stAnim->d_stMorphDataArray = p_stAnim->p_stData->morphData;
    count = p_stAnim->p_stData->morphCount;
    for (i = 0; i < count; i++) {
        if (p_stAnim->d_stMorphDataArray[i].channel == channel && p_stAnim->d_stMorphDataArray[i].frame == frame) {
            return p_stAnim->d_stMorphDataArray[i].morphingAmount;
        }
    }
    return 0;
}

AnimationMorphEntry* inv_PLA_fn_p_stFindMorphData(tdstAnim3d_* p_stAnim, s32 channel, u16 frame)
{
    AnimationData* data = p_stAnim->p_stData;
    s32 i;

    p_stAnim->d_stMorphDataArray = data->morphData;
    for (i = 0; i < data->morphCount; i++) {
        if (p_stAnim->d_stMorphDataArray[i].channel == channel && p_stAnim->d_stMorphDataArray[i].frame == frame)
            return &p_stAnim->d_stMorphDataArray[i];
    }
    return NULL;
}

void PLA_fn_vStopCurrentMorphAnim(HIE_tdstSuperObject_* superObject)
{
    gu_tdst3dData_* data3d = ((tdstEngineObject_*) superObject->hLinkedObject)->h_3dData;
    tdstAnim3d_* p_stAnim = data3d->h_CurrentState->p_stAnim;
    u32 frame = data3d->uwCurrentFrame;
    s32 channel;

    if (p_stAnim != NULL && p_stAnim->d_stMorphDataArray != NULL && data3d->stFrame3d.p_stArrayOfElts3d != NULL) {
        for (channel = 0; channel < p_stAnim->ucMaxNumberOfElements; channel++) {
            if ((u32) ((TextTableKindQuery) inv_PLA_fn_lGetMorphingAmount)(p_stAnim, channel, frame) < 100)
                MOR_fn_xStopMorph(superObject, data3d->stFrame3d.p_stArrayOfElts3d[channel].ucChannelNumber);
        }
    }
}
