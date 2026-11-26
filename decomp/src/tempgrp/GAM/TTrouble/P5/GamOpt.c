#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "IPT/IPT.h"
#include "MTH/MTH.h"
#include "SVG/SVG.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"
#include "AI/AI.h"
#include "SAI/SAI.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "renderware.h"
#include "GEO/GEO.h"
#include "AGO/AGO.h"
#include "snd/snd.h"

#define MUSIC_VOLUME_MAX 0x7E
#define VOLUME_MAX 0x7F
#define PLAYER_NAME_LENGTH 6
#define SAVE_COMMENT_LENGTH 11
#define NO_LEVEL 0xFF

extern UnalignedU64x2 qword_3892A8;
extern char aRayman[];
extern f32 inv_GAM_xFlashAlpha;
extern s32 inv_GAM_demoTextPulse;
extern s32 inv_GAM_demoTextPulseStep;

s32 strcasecmp(const char* left, const char* right);
void GLI_vDisplayFrame(f32 alpha, f32 z, MTH3D_tdstVector_* topLeft,
                                    MTH3D_tdstVector_* bottomRight,
                                    GLD_tdstViewportAttributes_* viewport);
void inv_SAV_fn_vSetCurrentPlayerName(char* name);

s32 inv_fn_vDisplayPauseOrOptionsOverlay(void)
{
    UnalignedU64x2 color;
    MTH3D_tdstVector_ position;
    s32 result;

    GLI_vFogOff();
    inv_GLI_RW_vClearCameraIfEnabled(
        g_stEngineStructure.stFixViewportAttr.values.dwLeftInPixForClip,
        g_stEngineStructure.stFixViewportAttr.values.dwRightInPixForClip,
        g_stEngineStructure.stFixViewportAttr.values.dwTopInPixForClip,
        g_stEngineStructure.stFixViewportAttr.values.dwBottomInPixForClip);
    GLD_bRequestWriteToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport,
                                &g_stEngineStructure.stFixViewportAttr, &result);
    GEO_fn_vComputeCameraParametersForCulling(&g_stEngineStructure.stFixViewportAttr, 0.0f);

    color = qword_3892A8;
    inv_RW3_fn_vSetSingleLightColor(&color);

    if (inv_GLI_blendModeState != 0) {
        inv_GLI_blendModeState = 0;
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*) 1);
        inv_GLI_zTestState = 1;
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*) 1);
        inv_GLI_zWriteState = 1;
        inv_GLI_blendModeState = 0;
    }

    gu_GLI_RW_vInitTextureModule(&g_stEngineStructure.stFixViewportAttr,
                                 g_stEngineStructure.ulDisplayMode);

    inv_GAM_demoTextPulse += inv_GAM_demoTextPulseStep;
    if (inv_GAM_demoTextPulse < 0 || inv_GAM_demoTextPulse > 0xFF) {
        inv_GAM_demoTextPulseStep = -inv_GAM_demoTextPulseStep;
        inv_GAM_demoTextPulse += inv_GAM_demoTextPulseStep * 2;
    }

    if (inv_DEMO_bPlaying != 0) {
        MTH3D_M_vSetXYZ_inv(&position, 100.0f, 700.0f, 18.0f);
        inv_TXT_fn_vDrawStringWithTransparency(&position, "DEMO", inv_GAM_demoTextPulse);
    }

    if (fn_bIsEngineInPause() == 0) {
        inv_AGO_vDisplayGaugesAndIcons();
    } else {
        char text[0x80];
        MTH3D_tdstVector_ topLeft;
        MTH3D_tdstVector_ bottomRight;
        char* label;
        s32 joystick = IPT_fn_ucGetFirstValidJoystick();

        MTH3D_M_vZeroXYZ_inv(&topLeft);
        MTH3D_M_vSetXYZ_inv(&bottomRight, 100.0f, 100.0f, 0.0f);
        GLI_vDisplayFrame(128.0f, 0.0f, &topLeft, &bottomRight,
                                       &g_stEngineStructure.stFixViewportAttr);

        if (joystick == 0) {
            if (inv_IPT_fn_ulGetInputFrameCounter() >= 2) {
                MTH3D_M_vSetXYZ_inv(&topLeft, 420.0f, 300.0f, 15.0f);
                label = fn_p_GetTextById(inv_GAM_textIdBase + 1);
                sprintf(text, "%s %d", label, 1);
                inv_TXT_fn_vDrawStringWithTransparency(&topLeft, text, inv_GAM_demoTextPulse);
            }
            MTH3D_M_vSetXYZ_inv(&topLeft, 300.0f, 400.0f, 15.0f);
            inv_TXT_fn_vDrawStringWithTransparency(&topLeft, fn_p_GetTextById(inv_GAM_textIdBase + 2),
                          inv_GAM_demoTextPulse);
            MTH3D_M_vSetXYZ_inv(&topLeft, 70.0f, 500.0f, 13.0f);
            inv_TXT_fn_vDrawStringWithTransparency(&topLeft, fn_p_GetTextById(inv_GAM_textIdBase + 3),
                          inv_GAM_demoTextPulse);
        } else if (joystick > 0) {
            MTH3D_M_vSetXYZ_inv(&topLeft, 420.0f, 300.0f, 15.0f);
            label = fn_p_GetTextById(inv_GAM_textIdBase + 1);
            sprintf(text, "%s %d", label, joystick + 1);
            inv_TXT_fn_vDrawStringWithTransparency(&topLeft, text, inv_GAM_demoTextPulse);
            MTH3D_M_vSetXYZ_inv(&topLeft, 300.0f, 400.0f, 15.0f);
            inv_TXT_fn_vDrawStringWithTransparency(&topLeft, fn_p_GetTextById(inv_GAM_textIdBase + 2),
                          inv_GAM_demoTextPulse);
            MTH3D_M_vSetXYZ_inv(&topLeft, 70.0f, 500.0f, 13.0f);
            inv_TXT_fn_vDrawStringWithTransparency(&topLeft, fn_p_GetTextById(inv_GAM_textIdBase + 3),
                          inv_GAM_demoTextPulse);
            MTH3D_M_vSetXYZ_inv(&topLeft, 100.0f, 600.0f, 13.0f);
            inv_TXT_fn_vDrawStringWithTransparency(&topLeft, fn_p_GetTextById(inv_GAM_textIdBase + 4),
                          inv_GAM_demoTextPulse);
        }
    }

    if (inv_VID_bMovieJustEnded == 1) {
        inv_VID_bMovieJustEnded = 0;
        inv_GAM_xFlashAlpha = 255.0f;
    }
    if (inv_GAM_xFlashAlpha > 0.0f) {
        MTH3D_tdstVector_ bottomRight;

        MTH3D_M_vZeroXYZ_inv(&position);
        MTH3D_M_vSetXYZ_inv(&bottomRight, 100.0f, 100.0f, 0.0f);
        GLI_vDisplayFrame(inv_GAM_xFlashAlpha, 0.0f, &position, &bottomRight,
                                       &g_stEngineStructure.stFixViewportAttr);
#ifdef PC_PORT
        inv_GAM_xFlashAlpha -= (f32) pc_mod_fade_in_step(6);
#else
        inv_GAM_xFlashAlpha -= 6.0f;
#endif
    }

    inv_GLI_fn_vRenderPrepared2DRectangle();
    inv_RW3_fn_vFlushAllQuadBatches(&g_stEngineStructure.stFixViewportAttr);
    inv_RW3_fn_vDisableSingleLight();
    return GLD_bWriteToViewportFinished(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport);
}

void fn_vValidateAllOption(void)
{
    fn_vChangeMusicVolumeValue(inv_SAV_stGameData.volumes[1]);
    fn_vChangeFxVolumeValue(inv_SAV_stGameData.volumes[0]);
    fn_vChangeVoiceVolumeValue(inv_SAV_stGameData.volumes[2]);
}

void fn_vChangeMusicVolumeValue(u32 volume)
{
    if (volume > MUSIC_VOLUME_MAX) {
        volume = MUSIC_VOLUME_MAX;
    }
    inv_SAV_stGameData.volumes[1] = volume;
    SND_fn_vSetVolumeLineSound((s32*) g_lSoundVolumeLineMusic, volume);
}

void fn_vChangeFxVolumeValue(u32 volume)
{
    if (volume > VOLUME_MAX) {
        volume = VOLUME_MAX;
    }
    inv_SAV_stGameData.volumes[0] = volume;
    SND_fn_vSetVolumeLineSound((s32*) g_lSoundVolumeLineSound, volume);
}

void fn_vChangeVoiceVolumeValue(u32 volume)
{
    if (volume > VOLUME_MAX) {
        volume = VOLUME_MAX;
    }
    inv_SAV_stGameData.volumes[2] = volume;
    SND_fn_vSetVolumeLineSound((s32*) g_lSoundVolumeLineVoices, volume);
}

void inv_SND_fn_vInitDataLoadSnd(void)
{
    inv_SAV_stCurrentSlot[0].next = NULL;
    inv_SAV_stCurrentSlot[0].id = -1;
    inv_SAV_stCurrentSlot[0].info.valid = 0;
    inv_SAV_stCurrentSlot[0].info.modificationTime = 0;
    inv_SAV_stCurrentSlot[0].info.name[0] = '\0';
    memcpy(inv_SAV_stCurrentSlot[0].info.playerName, aRayman,
           sizeof(inv_SAV_stCurrentSlot[0].info.playerName));
}

void inv_SAV_fn_vSetCurrentPlayerName(char* name)
{
    if (strlen(name) <= PLAYER_NAME_LENGTH) {
        strcpy(inv_SAV_stCurrentSlot[0].info.playerName, name);
    } else {
        strncpy(inv_SAV_stCurrentSlot[0].info.playerName, name, PLAYER_NAME_LENGTH);
        inv_SAV_stCurrentSlot[0].info.playerName[PLAYER_NAME_LENGTH] = '\0';
    }
}

char* inv_SAV_fn_szGetCurrentPlayerName(void)
{
    return inv_SAV_stCurrentSlot[0].info.playerName;
}

void inv_SAV_fn_vSetCurrentSlotIndex(s16 index)
{
    inv_SAV_stCurrentSlot[0].info.slotIndex = index;
}

u16 inv_SAV_fn_wGetCurrentSlotIndex(void)
{
    return inv_SAV_stCurrentSlot[0].info.slotIndex;
}

void inv_SAV_fn_vSetCurrentSaveComment(char* comment)
{
    if (strlen(comment) <= SAVE_COMMENT_LENGTH) {
        strcpy(inv_SAV_stCurrentSlot[0].info.comment, comment);
    } else {
        strncpy(inv_SAV_stCurrentSlot[0].info.comment, comment, SAVE_COMMENT_LENGTH);
        inv_SAV_stCurrentSlot[0].info.comment[SAVE_COMMENT_LENGTH] = '\0';
    }
}

char* inv_SAV_fn_szGetCurrentSaveComment(void)
{
    return inv_SAV_stCurrentSlot[0].info.comment;
}

void inv_SAV_fn_vSetCurrentGameDuration(u32 duration)
{
    inv_SAV_stCurrentSlot[0].info.gameDuration = duration;
}

u32 inv_SAV_fn_ulGetCurrentGameDuration(void)
{
    return inv_SAV_stCurrentSlot[0].info.gameDuration;
}

s32 inv_SAV_fn_bSlotMatchesPlayerAndTimestamp(SaveSlot* slot)
{
    if (slot != NULL) {
        s32 matches = 0;

        if (strcasecmp(slot->info.playerName, inv_SAV_stCurrentSlot[0].info.playerName) == 0) {
            matches = slot->info.modificationTime == inv_SAV_stCurrentSlot[0].info.modificationTime;
        }
        return matches;
    }
    return 0;
}

s32 inv_SAV_fn_bProcessCurrentSaveRequest(void)
{
    if (inv_SAV_fn_bIsSaveDeviceReady() == 0) {
        return 0;
    }
    if (inv_SAV_fn_bIsSaveDeviceFormatted() != 0) {
        if (inv_SAV_stCurrentSlot[0].info.valid != 0) {
            if (inv_fn_bIsSaveSlotValid(&inv_SAV_stCurrentSlot[0].info) == 0) {
                return 0;
            }
            return inv_SAV_fn_bSaveSelectedSlot(inv_SAV_stCurrentSlot);
        }
        if (inv_fn_bCanSaveGame(SAI_fn_ucPlayerSaveValues()) == 0) {
            return 0;
        }
        return inv_SAV_fn_bSaveCurrentGame(inv_SAV_stCurrentSlot);
    }
    return 0;
}

s32 inv_SAV_fn_bInitCurrentSaveForPlayer(char* name)
{
    inv_SND_fn_vInitDataLoadSnd();
    inv_SAV_fn_vSetCurrentPlayerName(name);
    inv_SIF_fn_vInitSoundInfos();
    return 1;
}

s32 inv_SAV_fn_bSelectOrReplaceSlot(s16 index)
{
    SaveSlot* slot = inv_SAV_fn_p_stGetSlotByIndex(index);

    if (inv_SAV_fn_bSlotMatchesPlayerAndTimestamp(slot) == 0) {
        s32 result;

        if (inv_SAV_fn_bDeleteSlot(slot) == 0) {
            return 0;
        }
        inv_SAV_stCurrentSlot[0].info.modificationTime = 0;
        inv_SAV_stCurrentSlot[0].info.valid = 0;
        result = inv_SAV_fn_bSaveCurrentGame(inv_SAV_stCurrentSlot);
        slot->info = inv_SAV_stCurrentSlot[0].info;
        return result;
    }
    return inv_SAV_fn_bSaveSelectedSlot(inv_SAV_stCurrentSlot);
}

u32 inv_SAV_fn_bApplyLoadedGameOptionsTree(HIE_tdstSuperObject_* superObject)
{
    HIE_tdstSuperObject_* child;

    for (child = superObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        inv_SAV_fn_bApplyLoadedGameOptionsTree(child);
    }
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        func_001DDFF8(superObject->hLinkedObject);
    }
    return 1;
}

s32 fn_bLoadCurrentGameOptions(void)
{
    fn_vValidateAllOption();
    return inv_SAV_fn_bApplyLoadedGameOptionsTree(gp_stDynamicWorld);
}

s32 inv_SAV_fn_bLoadGameFromSlot(s16 index)
{
    SaveSlot* slot = inv_SAV_fn_p_stGetSlotByIndex(index);

    if (slot == NULL) {
        return 0;
    }
    if (inv_SAV_fn_bLoadSlotData(slot) != 0) {
        inv_SAV_stCurrentSlot[0] = *slot;
        return fn_bLoadCurrentGameOptions();
    }
    return 0;
}

s32 inv_SAV_fn_bCreateNewGameSave(void)
{
    inv_SAV_stCurrentSlot[0].id = -1;
    inv_SAV_stCurrentSlot[0].info.valid = 0;
    inv_SAV_stCurrentSlot[0].info.modificationTime = 0;
    return inv_SAV_fn_bSaveCurrentGame(inv_SAV_stCurrentSlot);
}

void inv_fn_vFreeLevelDynamicLists(void)
{
    SaveSlot* slot = inv_SAV_p_stDirectorySlots;

    while (slot != NULL) {
        SaveSlot* next = slot->next;

        inv_MEM_fn_vFreeInHeap(gp_6F74, slot);
        slot = next;
    }
    inv_SAV_p_stDirectorySlots = NULL;
    inv_SVG_bDirectoryValid = 0;
}

void inv_SAV_fn_vEnsureLevelDynamicListsLoaded(void)
{
    if (inv_SVG_bDirectoryValid == 0) {
        inv_fn_vFreeLevelDynamicLists();
        inv_SAV_p_stDirectorySlots = inv_SVG_fn_p_stScanAndLoadSaveDirectory();
    }
}

void* func_001114D0(void)
{
    return NULL;
}

s32 inv_fn_lGetLevelDynamicListCount(void)
{
    SaveSlot* slot;
    s32 count = 0;

    for (slot = inv_SAV_p_stDirectorySlots; slot != NULL; slot = slot->next) {
        count++;
    }
    return count;
}

u32 inv_SAV_fn_bCollectCurrentGameOptionsTree(HIE_tdstSuperObject_* superObject)
{
    HIE_tdstSuperObject_* child;

    for (child = superObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        inv_SAV_fn_bCollectCurrentGameOptionsTree(child);
    }
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        func_001DDDD0(superObject->hLinkedObject);
    }
    return 1;
}

s32 fn_bSaveCurrentGameOptions(void)
{
    return inv_SAV_fn_bCollectCurrentGameOptionsTree(gp_stDynamicWorld);
}

u8 inv_fn_ucFindLevelNameIndex(const char* name)
{
    u32 i;

    for (i = 0; i < g_stEngineStructure.ucNumberOfLevels; i++) {
        if (strcasecmp(name, g_stEngineStructure.a_szLevelName[i]) == 0) {
            return i;
        }
    }
    return NO_LEVEL;
}
