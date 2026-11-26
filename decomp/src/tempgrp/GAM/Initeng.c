#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "MTH/MTH.h"
#include "snd/snd.h"

#include "ps2/libc.h"
#include "sdk.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "HIE/HIE.h"
#include "IPT/IPT.h"
#include "MEC/MEC.h"
#include "SAI/SAI.h"
#include "AGO/AGO.h"
#include "SHW/SHW.h"
#include "ps2/PS2.h"
#include "VIG/VIG.h"

extern s32 D_00393264;
extern s32 inv_GAM_lSoundInitResult;

s32 GetThreadId(void);
void RwCameraEndUpdate(void*);
void RwCameraShowRaster(void*, void*, s32);
void fn_vReinitEngineClock(void);
void GLI_vDisplayFrame(MTH3D_tdstVector_* topLeft, MTH3D_tdstVector_* bottomRight,
                                    void* a2, f32 a3, f32 a4);
void inv_fn_vInitLevel(void);
void* RwCameraBeginUpdate(void*);
void fn_vReinitTheStaticHierarchie(HIE_tdstSuperObject_* superObject);
s32 sceScfGetLanguage(void);

static inline void inv_GAM_fn_vInitEngineList(EngineList* list)
{
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
}

void fn_vLastDesinitEngine(void)
{
    GAM_fn_vSetEngineInFixMode();
    fn_vDestroyFixViewPort();
    GLI_xDesinit();
    SND_fn_iDesInitSound();
    fn_vLastDesinitFamilyStructure();
}

void fn_vInitGameLoop(void)
{
    u8 previousMode = g_stEngineStructure.eEngineMode;

    GAM_fn_vSetEngineInLevelMode();
    fn_vReinitEngineClock();
    HIE_fn_vInitGlobalMatrixFrameCounter(0);
    if (fn_bIsNextLevelNameValid() != 0) {
        fn_vSetLevelName(fn_p_szGetNextLevelName());
        fn_vFreeNextLevelName();
    }
    fn_vInitAllObjectsWhenNewGameStart();
    SAI_fn_bInitAllWhen(1);
    g_stEngineStructure.activeSuperObject = g_stEngineStructure.initialSuperObject;
    fn_vChangeEngineMode(5);
    fn_vInitLevelLoop();
    fn_vChangeEngineMode(previousMode);
    TFMT_vInitTextAfficheStructure();
}

void inv_fn_vInitLevel(void)
{
    char* levelName;
    char* levelNumber;

    GAM_fn_bIsSlotValid(0, 1);
    inv_GLI_fn_bSetBackGroundColor(0, 0);
    levelName = fn_p_szGetLevelName();
    levelNumber = inv_fn_p_szGetLevelNumber();
    inv_fn_vLoadLevelDataFiles(levelName, levelNumber, fn_p_szGetFirstLevelName());
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_FILES);
#endif
    inv_fn_vAllocateLevelMemory(inv_fn_ulGetBiggestFreeBlockSize() - 0x400);
    fn_vInitAllObjectsWhenMapJustLoaded();
    CAM_fn_vInitCameras();
    SAI_fn_bInitAllWhen(2);
    g_stEngineStructure.activeSuperObject = g_stEngineStructure.initialSuperObject;
    fn_vInitDeadLoop();
}

void fn_vInitDeadLoop(void)
{
    HIE_tdstSuperObject_* active;
    HIE_tdstSuperObject_* other;

    g_stEngineStructure.bEngineFrozen = 0;
    g_stEngineStructure.cCameraMode = 0;
    fn_vReinitEngineClock();
    SHW_fn_vInitShadowCounter();
    fn_vReinitTheStaticHierarchie(SECT_hFatherSector);
    CAM_fn_vActualizeAViewport(1);

    if (g_stEngineStructure.eEngineMode != 5 && g_stEngineStructure.eEngineMode != 3) {
        active = g_stEngineStructure.activeSuperObject;
        if (inv_HIE_fn_p_stGetEngineObject(active)->h_StandardGame->ucHitPoints != 0) {
            SAI_fn_bInitAllWhen(0x20);
            g_stEngineStructure.activeSuperObject = g_stEngineStructure.initialSuperObject;
            fn_vInitAllObjectsWhenPlayerDead();
            IPT_fn_vActivateAllEntryElements();
        } else {
            fn_vReinitTheMap();
        }

        if (g_stEngineStructure.bResurection != 0) {
            if (active != NULL) {
                *inv_GAM_fn_p_stSavedMatrixOf(active) = g_stEngineStructure.stMainCharacterPosition;
            }
            other = g_stEngineStructure.h_StdCamCaracter;
            if (other != NULL) {
                *inv_GAM_fn_p_stSavedMatrixOf(other) = g_stEngineStructure.stMainCameraPosition;
            }
        }

        inv_GAM_currentSector = 0;
        g_stEngineStructure.bResurection = 0;
        GLI_cGlobalFogIsOn = 0;
        GLI_vInitWaterplaneModule();
    }

    fn_vReputCharacterSuperObjectAtTheWorld(g_stEngineStructure.activeSuperObject);

    if (g_stEngineStructure.eEngineMode == 7 && g_bBeginMapSoundEventFlag != 0) {
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMusic, GAM_p_stGetBlockEventSlotMemory(0), 0, 0);
        SND_fn_vSynchroSound();
    }
}

void fn_vDesInitDeadLoop(void)
{
    fn_vDestructAllFatherSonLinks();
    fn_vKillAllAlways();
    AGO_vInit();
}

void fn_vReinitTheMap(void)
{
    inv_IPT_fn_vInitInputFrameState();
    SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeAnim);
    SND_fn_vKillAllObjectTypeSound(inv_SND_moduleObjectSoundType);
    SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeVoice);
    SND_fn_vKillAllObjectTypeSound(inv_SND_moduleVoiceObjectSoundType);
    gu_MTH4D_M_vSetIdentityMatrix(&g_stEngineStructure.stMainCharacterPosition);
    gu_MTH4D_M_vSetIdentityMatrix(&g_stEngineStructure.stMainCameraPosition);
    g_stEngineStructure.bResurection = 0;

    fn_vKillAllAlways();
    fn_vDestructAllFatherSonLinks();
    SAI_fn_bInitAllWhen(0x10);
    g_stEngineStructure.activeSuperObject = g_stEngineStructure.initialSuperObject;
    fn_vForceReinitAllObjects();
    fn_vReinitTheStaticHierarchie(SECT_hFatherSector);
    CAM_fn_vActualizeAViewport(1);
    GLI_vInitWaterplaneModule();

    if (g_bBeginMapSoundEventFlag != 0) {
        GAM_fn_vSaveBlockEventSlotMemory(g_stBeginMapSoundEvent[0], 0);
    } else {
        GAM_fn_vSaveBlockEventSlotMemory(0, 0);
    }

    inv_MOR_fn_vStopAllMorphsInHierarchy(gp_stDynamicWorld);
    GLI_cGlobalFogIsOn = 0;
    SPO_ReinitSuperimposedArray();
    inv_GAM_currentSector = 0;
    iNumLoop = 0;
    IPT_fn_vActivateAllEntryElements();
    TFMT_vInitTextAfficheStructure();
    inv_AI_fn_vClearTextLabels(1);
    AGO_vInit();
    GLD_bClearViewportWithChoice(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport, 1);
    fn_vReinitEngineClock();
    inv_SHW_stShadowQueue.count = 0;
    inv_SHW_stShadowQueue.firstCursor = 0;
    inv_SHW_stShadowQueue.secondCursor = 0;
}

void fn_vReinitTheStaticHierarchie(HIE_tdstSuperObject_* hSuperObject)
{
    HIE_tdstSuperObject_* hChild = hSuperObject->hFirstElementSta;

    hSuperObject->gu_lDrawModeMask |= 0xEFFFFFFF;
    if (hChild == NULL) {
        return;
    }
    do {
        fn_vReinitTheStaticHierarchie(hChild);
        hChild = hChild->hNextBrotherSta;
    } while (hChild != NULL);
}

void fn_vFirstInitEngine(void)
{
    s32 soundParameters[4];

    inv_GAM_fn_vInitEngineList((EngineList*) &g_stEngineStructure.hCameraList);
    g_stEngineStructure.activeSuperObject = NULL;
    g_stEngineStructure.pendingSuperObject = NULL;
    g_stEngineStructure.initialSuperObject = NULL;
    inv_GAM_fn_vInitEngineList((EngineList*) &g_stEngineStructure.hAlwaysActiveCharactersList);
    g_stEngineStructure.ucPreviousLevel = 0;
    g_stEngineStructure.ucExitIdToQuitPrevLevel = 0;
    g_stEngineStructure.ucLevelGlobalCounter = 0;
    g_stEngineStructure.ucCurrentLanguage = 0xFF;
    inv_fn_vInitEngineObjectLists();
    GAM_fn_vCreateGameDeviceAndViewPorts();
    inv_TXT_fn_vInitTextTables();
    inv_j_fn_v_InitSinTab();
    fn_vInitAI(0);
    D_00393264 = 1;
    inv_GAM_lSoundInitResult = SND_fn_iInitSound(soundParameters);
    if (inv_GAM_lSoundInitResult == 1) {
        do {
            inv_GAM_lSoundInitResult = SND_fn_iTestInitSound();
        } while (inv_GAM_lSoundInitResult == 1);
    }
    if (inv_GAM_lSoundInitResult == 3) {
        printf("Init sound succeeds\n");
    } else if (inv_GAM_lSoundInitResult == -1) {
        printf("Init sound fails\n");
    } else {
        printf("Unexpected Sound status\n");
    }
    SND_fn_vSetDopplerFactorSound(0);
    inv_SIF_fn_vInitSoundInfos();
    inv_SND_fn_vInitDataLoadSnd();
    fn_lInitSoundForAnim();
    fn_vInitMicrosFunction();
    POS_fn_vInitPos();
    PLA_fn_vCreateAndInitHeaps();
    gu_MTH4D_M_vSetIdentityMatrix(&g_stEngineStructure.stMainCharacterPosition);
    gu_MTH4D_M_vSetIdentityMatrix(&g_stEngineStructure.stMainCameraPosition);
    g_stEngineStructure.bResurection = 0;
    HIE_fn_vCreateMatrixStack();
    TFMT_vSetDefaultValuesCharacter(1.0f, 2.0f, 0.0f);
    GAM_fn_vReadGameDescriptionBinaryFile();
    inv_VIG_fn_vDisplayStartupImage("rw", g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, 1);
#ifdef PC_PORT
    switch (pc_language()) {
#else
    switch (sceScfGetLanguage()) {
#endif
    default:
        g_stEngineStructure.ucCurrentLanguage = 3;
        inv_TXT_textTableState.language = 0;
        break;
    case 1:
        inv_TXT_textTableState.language = 0;
        g_stEngineStructure.ucCurrentLanguage = 0;
        break;
    case 2:
        inv_TXT_textTableState.language = 1;
        g_stEngineStructure.ucCurrentLanguage = 1;
        break;
    case 4:
        inv_TXT_textTableState.language = 2;
        g_stEngineStructure.ucCurrentLanguage = 2;
        break;
    case 3:
        inv_TXT_textTableState.language = 3;
        g_stEngineStructure.ucCurrentLanguage = 3;
        break;
    case 5:
        inv_TXT_textTableState.language = 4;
        g_stEngineStructure.ucCurrentLanguage = 3;
        break;
    }
    IPT_fn_vInitInput(0);
}

void func_00102548(void)
{
    func_00112E60();
    g_stEngineStructure.eEngineMode = 1;
    inv_SHW_fn_vResetProjectionGlobals();
}

void fn_vInitLevelLoop(void)
{
    ThreadParam status;
    char savedLevelName[0x38];
    u8 savedFlag;

#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_BEGIN);
#endif
#ifdef PC_PORT
    status.currentPriority = pc_thread_priority(pc_thread_self());
#else
    ReferThreadStatus(GetThreadId(), &status);
#endif
    inv_VID_fn_vStartMovie(status.currentPriority);
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_MOVIE_ON);
#endif
    GLI_g_bSaveFog = 0;
    g_AlphabetCharacterPointer = NULL;
    inv_GAM_bMenuCharactersOnly = 0;
    inv_fn_vInitLevel();
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_OBJECTS);
#endif
    RW3_fn_vCreateAllRWObjectLEVEL();
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_RW);
#endif
    inv_SHW_fn_vInitProjectionBuffers();

    if (g_stEngineStructure.eEngineMode == 5 || g_stEngineStructure.eEngineMode == 3) {
        if (g_bBeginMapSoundEventFlag != 0) {
            GAM_fn_vSaveBlockEventSlotMemory(g_stBeginMapSoundEvent[0], 0);
            fnSetLastEventForSoundObjectType(g_lSoundObjectTypeMusic, 0);
        }
    }

    InitArrayOfDynamicFunction(g_a_pfnDynamicFunction);
    IPT_fn_vResetInputEntry();
    PLA_fn_vInitHeaps();
    GLI_cGlobalFogIsOn = 0;
    GLI_vInitWaterplaneModule();
    IPT_fn_vActivateAllEntryElements();

    savedFlag = inv_DEMO_bPlaying;
    if (savedFlag != 0) {
        strcpy(savedLevelName, g_stEngineStructure.szLevelName);
    }
    IPT_fn_vReadInput();
    IPT_fn_vReadInput();
    inv_DEMO_bPlaying = savedFlag;
    if (savedFlag != 0) {
        strcpy(g_stEngineStructure.szLevelName, savedLevelName);
    }

    inv_GAM_currentSector = 0;
    inv_GAM_previousSector = 0;
    AGO_vInit();
    inv_AI_fn_vClearTextLabels(0);
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_MOVIE_OFF);
#endif
    inv_VID_fn_vStopMovie(status.currentPriority);
#ifdef PC_PORT
    printf("PLD_fn_vStopMpeg() :threadid = %d , _u8CallerThreadPri = %d\n", pc_thread_self(), status.currentPriority);
#else
    printf("PLD_fn_vStopMpeg() :threadid = %d , _u8CallerThreadPri = %d\n", GetThreadId(), status.currentPriority);
#endif
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_MOVIE_DONE);
#endif
    func_001516F8();
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_SKY_CACHE);
#endif
    inv_SHW_fn_vInitShadowQueue();
    IPT_fn_vDesactiveJoystickControl(0);
    inv_IPT_fn_bActivateJoystick(0);
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_END);
#endif
}

void fn_vDesInitLevelLoop(void)
{
    u32 divisor;
    u32 scale;
    s32 volume0;
    s32 volume1;
    s32 volume2;
    s32 count;
    MTH3D_tdstVector_ topLeft;
    MTH3D_tdstVector_ bottomRight;

#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_LEAVE);
#endif
    volume0 = SND_fn_lGetVolumeLineSound((s32*) g_lSoundVolumeLineMusic);
    volume1 = SND_fn_lGetVolumeLineSound((s32*) g_lSoundVolumeLineSound);
    volume2 = SND_fn_lGetVolumeLineSound((s32*) g_lSoundVolumeLineVoices);
    inv_GLI_fn_vPrepareLevelFadeOut();
    divisor = 1200;
    scale = 1000;
    count = 44;
#ifdef PC_PORT
    count = pc_mod_fade_frames(count + 1) - 1;
    if (count < 44) {
        scale = 100;
    }
#endif
    do {
        MTH3D_M_vZeroXYZ_inv(&topLeft);
        MTH3D_M_vSetXYZ_inv(&bottomRight, 100.0f, 100.0f, 0.0f);
        RwCameraBeginUpdate(inv_GLI_currentCamera);
        GLI_vDisplayFrame(&topLeft, &bottomRight, &g_stEngineStructure.stFixViewportAttr, 37.0f, 0.0f);
        inv_GLI_fn_vRenderPrepared2DRectangle();
        RwCameraEndUpdate(inv_GLI_currentCamera);
        RwCameraShowRaster(inv_GLI_currentCamera, 0, 0);
        SND_fn_vSetVolumeLineSound((s32*) g_lSoundVolumeLineMusic, volume0);
        SND_fn_vSetVolumeLineSound((s32*) g_lSoundVolumeLineVoices, volume2);
        SND_fn_vSetVolumeLineSound((s32*) g_lSoundVolumeLineSound, volume1);
        volume0 = (u32) volume0 * scale / divisor;
        volume1 = (u32) volume1 * scale / divisor;
        volume2 = (u32) volume2 * scale / divisor;
    } while (--count >= 0);
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_FADED);
#endif
    SND_fn_vKillAllObjectSound();
    SND_fn_vSetDopplerFactorSound(0);
    SND_fn_vRestoreVolumeLines();
    SND_fn_bUnLoadBinaryBankAndData(inv_SND_ulLevelBank);
    inv_fn_vClearEngineObjectLists();
    fn_vDesInitDeadLoop();
    SPO_ReinitSuperimposedArray();
    inv_SHW_fn_vDesInitShadow();
    RW3_fn_vDestroyAllRWObjectLEVEL();
    fn_vKillAllAlwaysModel();
    fn_vDestroyAllHierarchy(gp_stActualWorld);
    inv_fn_vFreeLevelDynamicLists();
    fn_vRemoveAllMicros();
    fn_vFreeLevelAllowedMemory();
    inv_SNA_fn_vDelocateMemoryBlocks();
    inv_GAM_fn_vInitEngineList((EngineList*) &g_stEngineStructure.hCameraList);
    g_stEngineStructure.activeSuperObject = NULL;
    g_stEngineStructure.pendingSuperObject = NULL;
    g_stEngineStructure.initialSuperObject = NULL;
    inv_GAM_fn_vInitEngineList((EngineList*) &g_stEngineStructure.hAlwaysActiveCharactersList);
    fn_vReputGameViewportCamera();
    inv_fn_vClearLevelRuntimeList();
    inv_SHW_fn_vResetShadowQueue();
#ifdef PC_PORT
    pc_mod_load_mark(PC_LOAD_LEFT);
#endif
}
