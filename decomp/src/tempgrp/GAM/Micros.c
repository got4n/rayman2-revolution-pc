#include "common.h"

#include "GAM/GAM.h"
#include "GMT/GMT.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"
#include "snd/snd.h"

#include "ps2/libc.h"

#define GHOST_SUPER_OBJECT -1
#define FIXED_ONE 0x10000
#define PITCH_MAX 0xC80000
#define PITCH_MIN 0x1999
#define SAVED_SOUND_EVENT_COUNT 10

void RetObjetSonoreExtraCoefMusic(tdstAChannel_* sound, _SND_tduRefEvt event, s32* pitch,
                                  s32* pan, s32* volume);

typedef void (*SoundInfoCallback)(HIE_tdstSuperObject_* superObject, char* text, s32 size);
typedef s32 (*SoundRollOffCallback)(HIE_tdstSuperObject_* superObject, u32* distances);

extern s32 ga_pstBlockEventSlotMemory[SAVED_SOUND_EVENT_COUNT];

s32 RetObjetSonoreSwitchAnim(HIE_tdstSuperObject_* superObject, s32 type);
s32 SND_fn_lAddVolumeLineSound(void);

static inline s32 inv_GAM_fn_lFloatToFixed(f32 value)
{
    s32 integer;
    f32 integerPart;
    f32 fixedPoint;

    integer = (s32) value;
    integerPart = (f32) integer;
    integer <<= 16;
    fixedPoint = (f32) integer;
    fixedPoint = fixedPoint + (value - integerPart) * 65536.0f;
    return (s32) fixedPoint;
}

static inline MTH3D_tdstVector_ inv_GAM_fn_stSuperObjectPosition(HIE_tdstSuperObject_* superObject)
{
    POS_tdstMatrix_* matrix = superObject->gu_hGlobalMatrix;

    return *inv_POS_fn_p_stPosMatrixTranslation(matrix);
}

s32 RetObjetSonoreXAnim(HIE_tdstSuperObject_* superObject)
{
    MTH3D_tdstVector_ position = inv_GAM_fn_stSuperObjectPosition(superObject);

    return inv_GAM_fn_lFloatToFixed(position.xX);
}

s32 RetObjetSonoreYAnim(HIE_tdstSuperObject_* superObject)
{
    MTH3D_tdstVector_ position = inv_GAM_fn_stSuperObjectPosition(superObject);

    return inv_GAM_fn_lFloatToFixed(position.xY);
}

s32 RetObjetSonoreZAnim(HIE_tdstSuperObject_* superObject)
{
    MTH3D_tdstVector_ position = inv_GAM_fn_stSuperObjectPosition(superObject);

    return inv_GAM_fn_lFloatToFixed(position.xZ);
}

static inline tdstDynam_* inv_GAM_fn_p_stSuperObjectDynamics(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return object->h_Dynam;
}

s32 RetObjetSonoreVXAnim(HIE_tdstSuperObject_* superObject)
{
    tdstDynam_* dynamics = inv_GAM_fn_p_stSuperObjectDynamics(superObject);
    MTH3D_tdstVector_ speed;

    if (dynamics == NULL) {
        return 0;
    }
    speed = dynamics->p_stDynamics->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
    return inv_GAM_fn_lFloatToFixed(speed.xX);
}

s32 RetObjetSonoreVYAnim(HIE_tdstSuperObject_* superObject)
{
    tdstDynam_* dynamics = inv_GAM_fn_p_stSuperObjectDynamics(superObject);
    MTH3D_tdstVector_ speed;

    if (dynamics == NULL) {
        return 0;
    }
    speed = dynamics->p_stDynamics->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
    return inv_GAM_fn_lFloatToFixed(speed.xY);
}

s32 RetObjetSonoreVZAnim(HIE_tdstSuperObject_* superObject)
{
    tdstDynam_* dynamics = inv_GAM_fn_p_stSuperObjectDynamics(superObject);
    MTH3D_tdstVector_ speed;

    if (dynamics == NULL) {
        return 0;
    }
    speed = dynamics->p_stDynamics->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
    return inv_GAM_fn_lFloatToFixed(speed.xZ);
}

#ifdef PC_PORT
s32 RetObjetSonoreREVERBAnim(void* unused)
#else
s32 RetObjetSonoreREVERBAnim(void)
#endif
{
    return 0;
}

void RetObjetSonoreExtraCoefAnim(s32 object, _SND_tduRefEvt event, s32* pitch, s32* pan, s32* volume)
{
    tdstDynam_* dynamics;
    DNM_stDynamics* base;
    MTH3D_tdstVector_ speed;
    f32 length;
    s32 value;
    s32 coefficient;

    if (pitch != NULL) {
        dynamics = inv_GAM_fn_p_stSuperObjectDynamics((HIE_tdstSuperObject_*) object);
        if (dynamics == NULL) {
            *pitch = 0;
        } else {
            base = dynamics->p_stDynamics;
            do {
                speed = base->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
                length = MTH_fn_xSqrAddSqrAddSqrOpt2(speed.xX, speed.xY, speed.xZ);
                length = MTH_fn_xSqrtOpt2(length);
                value = inv_GAM_fn_lFloatToFixed(length);
                if (value > PITCH_MAX) {
                    value = PITCH_MAX;
                } else if (value < PITCH_MIN) {
                    value = PITCH_MIN;
                }
            } while (value == 0);
            *pitch = value >> 7;
        }
    }
    if (volume != NULL) {
        coefficient = 0;
        do {
            if (coefficient != 0) {
                *volume = coefficient;
                break;
            }
            coefficient = FIXED_ONE;
        } while (1);
    }
    if (pan != NULL) {
        coefficient = 0;
        do {
            if (coefficient != 0) {
                *pan = coefficient;
                break;
            }
            coefficient = FIXED_ONE;
        } while (1);
    }
}

s32 RetObjetSonoreSwitchAnim(HIE_tdstSuperObject_* superObject, s32 type)
{
    tdstDynam_* dynamics;
    DNM_stDynamics* base;
    DNM_stReport_* report;
    u32 options;
    u32 flags;
    GMT_tdstGameMaterial_* material;

    switch (type) {
    case 1:
        dynamics = inv_GAM_fn_p_stSuperObjectDynamics(superObject);
        if (dynamics != NULL) {
            base = dynamics->p_stDynamics;
            report = base->m_tdstDynamicsBaseBlock.m_pstReport;
            options = base->m_tdstDynamicsBaseBlock.ulFlags;
            flags = report->m_ulCurrSurfaceState;
            if ((options & 0x10) && (flags & 0x10)) {
                material = report->m_stCeil.hCollidedMaterial;
                if (material != NULL)
                    return GMT_fn_hGetSoundMaterial(material);
                return 0;
            }
            if ((options & 0x40) && (flags & 0x04)) {
                material = report->m_stWall.hCollidedMaterial;
                if (material != NULL)
                    return GMT_fn_hGetSoundMaterial(material);
                return 0;
            }
            if (flags & 0x01) {
                material = report->m_stGround.hCollidedMaterial;
                if (material != NULL)
                    return GMT_fn_hGetSoundMaterial(material);
            }
            return 0;
        }
        break;
    case 2:
        return 0;
    default:
        return 0;
    }
    return 0;
}

s32 RetObjetSonoreLinkAnim(HIE_tdstSuperObject_* root, s32 index)
{
    tdstEngineObject_* object;
    tdstEngineObject_* micro;
    HIE_tdstSuperObject_* microSuperObject;
    HIE_tdstSuperObject_* sector;
    HIE_tdstSuperObject_* microSector;
    SECT_tdstSector_* sectorData;
    SECT_tdstListOfSOSectorsInSoundInteraction_* link;
    s32 count;

    object = root->hLinkedObject;
    sector = object->h_SectInfo->hCurrentSector;
    microSuperObject = fn_hReturnHandleMicroFromIndex(index)->h_StandardGame->h_SuperObject;
    if (root == g_stEngineStructure.activeSuperObject) {
        return 1;
    }
    micro = microSuperObject->hLinkedObject;
    microSector = micro->h_SectInfo->hCurrentSector;
    if (sector == microSector) {
        return 1;
    }
    sectorData = microSector->hLinkedObject;
    count = sectorData->stListOfSectorsInSoundInteraction.lNumberOfElementsOpt;
    link = sectorData->stListOfSectorsInSoundInteraction.hFirstElementOpt;
    for (; count != 0; count--, link++) {
        if (link->hPointerOfSectorSO == sector) {
            return link->lVolume != 0;
        }
    }
    return 1;
}

void RetObjetInfoAnim(HIE_tdstSuperObject_* superObject, char* text, s32 size)
{
    tdstEngineObject_* object;

    if ((s32) superObject == GHOST_SUPER_OBJECT) {
        strncpy(text, "Ghost Object", size);
        return;
    }
    object = superObject->hLinkedObject;
    strncpy(text, "Perso:", size);
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        if (!((object->h_StandardGame->ucMiscFlags >> 4) & 1)) {
            if (object->h_Brain != NULL) {
                strncat(text, "UNAVAILABLE (D_ActorIdentificationEnabled is not defined)", size - strlen(text));
            } else {
                strncat(text, "no intelligence", size - strlen(text));
            }
        } else {
            strncat(text, "Always", size - strlen(text));
        }
    }
}

s32 inv_RetObjetSonoreXMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreYMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreZMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreVXMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreVYMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreVZMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreREVERBMenu(void)
{
    return 0;
}

void inv_RetObjetSonoreExtraCoefMenu(s32 object, _SND_tduRefEvt event, s32* pitch, s32* pan,
                                     s32* volume)
{
    tdstEngineObject_* first;

    if (volume == NULL) {
        return;
    }
    first = fn_p_stGetFirstEngineObjectInMicroList();
    if (first != NULL) {
        *(gu_tdst3dData_* volatile*) &first->h_3dData;
    }
    *volume = FIXED_ONE;
}

s32 inv_RetObjetSonoreSwitchMenu(void)
{
    return 0;
}

s32 inv_RetObjetSonoreLinkMenu(void)
{
    return 1;
}

s32 RetObjetSonoreXMusic(tdstAChannel_* sound)
{
    MTH3D_tdstVector_ position;

    if (sound->bActiveChannel != 0) {
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        return inv_GAM_fn_lFloatToFixed(position.xX);
    }
    return inv_GAM_fn_lFloatToFixed(sound->inv_stLastPosition.xX);
}

s32 RetObjetSonoreYMusic(tdstAChannel_* sound)
{
    MTH3D_tdstVector_ position;

    if (sound->bActiveChannel != 0) {
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        return inv_GAM_fn_lFloatToFixed(position.xY);
    }
    return inv_GAM_fn_lFloatToFixed(sound->inv_stLastPosition.xY);
}

s32 RetObjetSonoreZMusic(tdstAChannel_* sound)
{
    MTH3D_tdstVector_ position;

    if (sound->bActiveChannel != 0) {
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        return inv_GAM_fn_lFloatToFixed(position.xZ);
    }
    return inv_GAM_fn_lFloatToFixed(sound->inv_stLastPosition.xZ);
}

s32 RetObjetSonoreVXMusic(tdstAChannel_* sound)
{
    MTH3D_tdstVector_ position;
    f32 delta;
    f32 last;

    if (sound->bActiveChannel != 0) {
        delta = inv_fn_xGetEngineClockDelta();
        last = sound->inv_stLastPosition.xX;
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        return inv_GAM_fn_lFloatToFixed((position.xX - last) / delta);
    }
    return 0;
}

s32 RetObjetSonoreVYMusic(tdstAChannel_* sound)
{
    MTH3D_tdstVector_ position;
    f32 delta;
    f32 last;

    if (sound->bActiveChannel != 0) {
        delta = inv_fn_xGetEngineClockDelta();
        last = sound->inv_stLastPosition.xY;
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        return inv_GAM_fn_lFloatToFixed((position.xY - last) / delta);
    }
    return 0;
}

s32 RetObjetSonoreVZMusic(tdstAChannel_* sound)
{
    MTH3D_tdstVector_ position;
    f32 delta;
    f32 last;

    if (sound->bActiveChannel != 0) {
        delta = inv_fn_xGetEngineClockDelta();
        last = sound->inv_stLastPosition.xZ;
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        return inv_GAM_fn_lFloatToFixed((position.xZ - last) / delta);
    }
    return 0;
}

s32 RetObjetSonoreREVERBMusic(void)
{
    return 0;
}

void RetObjetSonoreExtraCoefMusic(tdstAChannel_* sound, _SND_tduRefEvt event, s32* pitch,
                                  s32* pan, s32* volume)
{
    MTH3D_tdstVector_ delta;
    f32 delta_t;
    f32 length;
    s32 speed;
    s32 value;
    s32 coefficient;

    if (sound->bActiveChannel != 0) {
        delta_t = inv_fn_xGetEngineClockDelta();
        delta = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
        MTH3D_M_vSetXYZ_inv(&delta, (delta.xX - sound->inv_stLastPosition.xX) / delta_t,
                            (delta.xY - sound->inv_stLastPosition.xY) / delta_t,
                            (delta.xZ - sound->inv_stLastPosition.xZ) / delta_t);
        length = MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ);
        length = MTH_fn_xSqrtOpt2(length);
        speed = inv_GAM_fn_lFloatToFixed(length);
    } else {
        speed = 0;
    }
    if (pitch != NULL) {
        do {
            value = speed;
            if (value > PITCH_MAX) {
                value = PITCH_MAX;
            } else if (value < PITCH_MIN) {
                value = PITCH_MIN;
            }
            *pitch = value >> 7;
        } while (value == 0);
    }
    coefficient = 0;
    for (;;) {
        if (volume == NULL)
            goto pan;
        if ((value = coefficient) != 0)
            goto pan;
        coefficient = FIXED_ONE;
        *volume = coefficient;
    }
pan:
    coefficient = 0;
    for (;;) {
        if (pan == NULL)
            return;
        if (coefficient != 0)
            return;
        coefficient = FIXED_ONE;
        *pan = coefficient;
    }
}

s32 RetObjetSonoreSwitchMusic(void)
{
    return 0;
}

s32 RetObjetSonoreLinkMusic(tdstAChannel_* sound, s32 index)
{
    MTH3D_tdstVector_ position;
    HIE_tdstSuperObject_* microSuperObject;
    tdstEngineObject_* micro;
    HIE_tdstSuperObject_* sector;
    SECT_tdstSector_* sectorData;
    SECT_tdstListOfSOSectorsInSoundInteraction_* link;
    s32 count;

    if (sound->bActiveChannel != 0) {
        position = inv_GAM_fn_stSuperObjectPosition(sound->hSupObject);
    } else {
        position = sound->inv_stLastPosition;
    }
    microSuperObject = fn_hReturnHandleMicroFromIndex(index)->h_StandardGame->h_SuperObject;
    micro = microSuperObject->hLinkedObject;
    sector = micro->h_SectInfo->hCurrentSector;
    if (SECT_fn_bIsInBorderBox(&position, sector->hLinkedObject)) {
        return 1;
    }
    sectorData = sector->hLinkedObject;
    count = sectorData->stListOfSectorsInSoundInteraction.lNumberOfElementsOpt;
    link = sectorData->stListOfSectorsInSoundInteraction.hFirstElementOpt;
    for (; count != 0; count--, link++) {
        if (SECT_fn_bIsInBorderBox(&position, link->hPointerOfSectorSO->hLinkedObject)) {
            return link->lVolume != 0;
        }
    }
    return 1;
}

void inv_RetObjetInfoModule(HIE_tdstSuperObject_* superObject, char* text, s32 size)
{
    strncpy(text, "Module:", size);
}

void fn_lInitSoundForAnim(void)
{
    SND_tdstTypeObjectSoundInfos info;

    g_lSoundVolumeLineVoices = SND_fn_lAddVolumeLineSound();

    memset(&info, 0, sizeof(info));
    info.callback00 = RetObjetSonoreXAnim;
    info.callback04 = RetObjetSonoreYAnim;
    info.callback08 = RetObjetSonoreZAnim;
    info.callback0C = RetObjetSonoreVXAnim;
    info.callback10 = RetObjetSonoreVYAnim;
    info.callback14 = RetObjetSonoreVZAnim;
    info.retReverb = RetObjetSonoreREVERBAnim;
    info.retExtraCoef = RetObjetSonoreExtraCoefAnim;
    info.retSwitch = RetObjetSonoreSwitchAnim;
    info.retMicroLink = RetObjetSonoreLinkAnim;
    g_lSoundVolumeLineSound = SND_fn_lAddVolumeLineSound();
    g_lSoundObjectTypeAnim = SND_fn_lAddObjectTypeSound2(&info, (struct VLNode*) g_lSoundVolumeLineSound);
    printf("object type anim %d assoc g_lSoundVolumeLineSound\n", g_lSoundObjectTypeAnim);
    SND_fn_vSetInfoForTypeSound(g_lSoundObjectTypeAnim, RetObjetInfoAnim);

    g_lSoundObjectTypeVoice = SND_fn_lAddObjectTypeSound2(&info, (struct VLNode*) g_lSoundVolumeLineVoices);
    printf("object type voice %d assoc g_lSoundVolumeLineVoice\n", g_lSoundObjectTypeVoice);
    SND_fn_vSetInfoForTypeSound(g_lSoundObjectTypeVoice, RetObjetInfoAnim);

    memset(&info, 0, sizeof(info));
    info.callback00 = RetObjetSonoreXMusic;
    info.callback04 = RetObjetSonoreYMusic;
    info.callback08 = RetObjetSonoreZMusic;
    info.callback0C = RetObjetSonoreVXMusic;
    info.callback10 = RetObjetSonoreVYMusic;
    info.callback14 = RetObjetSonoreVZMusic;
    info.retReverb = RetObjetSonoreREVERBMusic;
    info.retExtraCoef = (SND_tfnRetObjectSoundExtraCoef) RetObjetSonoreExtraCoefMusic;
    info.retSwitch = RetObjetSonoreSwitchMusic;
    info.retMicroLink = RetObjetSonoreLinkMusic;
    inv_SND_moduleObjectSoundType = SND_fn_lAddObjectTypeSound2(&info, (struct VLNode*) g_lSoundVolumeLineSound);
    printf("object type module %d assoc g_lSoundVolumeLineSound\n", inv_SND_moduleObjectSoundType);
    SND_fn_vSetInfoForTypeSound(inv_SND_moduleObjectSoundType, inv_RetObjetInfoModule);

    inv_SND_moduleVoiceObjectSoundType =
        SND_fn_lAddObjectTypeSound2(&info, (struct VLNode*) g_lSoundVolumeLineVoices);
    printf("object type voice module %d assoc g_lSoundVolumeLineVoice\n",
           inv_SND_moduleVoiceObjectSoundType);
    SND_fn_vSetInfoForTypeSound(inv_SND_moduleVoiceObjectSoundType, inv_RetObjetInfoModule);

    memset(&info, 0, sizeof(info));
    info.callback00 = inv_RetObjetSonoreXMenu;
    info.callback04 = inv_RetObjetSonoreYMenu;
    info.callback08 = inv_RetObjetSonoreZMenu;
    info.callback0C = inv_RetObjetSonoreVXMenu;
    info.callback10 = inv_RetObjetSonoreVYMenu;
    info.callback14 = inv_RetObjetSonoreVZMenu;
    info.retReverb = inv_RetObjetSonoreREVERBMenu;
    info.retExtraCoef = inv_RetObjetSonoreExtraCoefMenu;
    info.retSwitch = inv_RetObjetSonoreSwitchMenu;
    info.retMicroLink = inv_RetObjetSonoreLinkMenu;
    g_lSoundVolumeLineMusic = SND_fn_lAddVolumeLineSound();
    g_lSoundObjectTypeMusic = SND_fn_lAddObjectTypeSound2(&info, (struct VLNode*) g_lSoundVolumeLineMusic);
    printf("object type Dialog %d assoc g_lSoundVolumeLineMusic\n", g_lSoundObjectTypeMusic);

    memset(&info, 0, sizeof(info));
    info.callback00 = inv_RetObjetSonoreXMenu;
    info.callback04 = inv_RetObjetSonoreYMenu;
    info.callback08 = inv_RetObjetSonoreZMenu;
    info.callback0C = inv_RetObjetSonoreVXMenu;
    info.callback10 = inv_RetObjetSonoreVYMenu;
    info.callback14 = inv_RetObjetSonoreVZMenu;
    info.retReverb = inv_RetObjetSonoreREVERBMenu;
    info.retExtraCoef = inv_RetObjetSonoreExtraCoefMenu;
    info.retSwitch = inv_RetObjetSonoreSwitchMenu;
    info.retMicroLink = inv_RetObjetSonoreLinkMenu;
    g_lSoundObjectTypeMenu = SND_fn_lAddObjectTypeSound2(&info, (struct VLNode*) g_lSoundVolumeLineSound);
    printf("object type Menu %d assoc g_lSoundVolumeLineSound\n", g_lSoundObjectTypeMenu);

    SND_fn_vSetRollOffFunctionForTypeSound(g_lSoundObjectTypeAnim, fn_bRetRollOffFactor);
    memset(ga_pstBlockEventSlotMemory, 0, sizeof(ga_pstBlockEventSlotMemory));
}

void GAM_fn_vSaveBlockEventSlotMemory(s32 event, s32 slot)
{
    ga_pstBlockEventSlotMemory[slot] = event;
}

s32 GAM_p_stGetBlockEventSlotMemory(s32 s32_Index)
{
    return ga_pstBlockEventSlotMemory[s32_Index];
}
