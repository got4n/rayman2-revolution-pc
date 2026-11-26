#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLD.h"
#include "HIE/HIE.h"
#include "IPT/IPT.h"
#include "SCT/SCT.h"

#include "ps2/libc.h"
#include "sdk.h"

#include "include_asm.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "GLI/GLI.h"
#include "TMR/TMR.h"
#include "INO/INO.h"
#include "VIG/VIG.h"

extern GLI_tdstSpecificAttributesFor3D_ inv_GAM_stMainViewportAttributes3D;
extern s32 dword_39A6F0[] __attribute__((section(".sdata")));
extern u8 inv_GAM_a_ucMainThreadStack[];

void fn_vReinitEngineClock(void);
void func_001000C0(void);
void func_001000C8(void);
void func_001000D0(void);

#ifdef PC_PORT
void func_00100188(void)
{
    extern u8 off_252380[];
    extern u8 unk_394100[];
    void inv___register_frame_info_table(void* table, void* object);

    inv___register_frame_info_table(off_252380, unk_394100);
}

void func_001001C0(void)
{
    extern u8 off_252380[];
    void* inv___deregister_frame_info(void* begin);

    inv___deregister_frame_info(off_252380);
}

#endif
s32 inv_fn_bCreateDefaultViewportAndCamera(void)
{
    u8 viewportPacket[0x20];
    GLD_tdstViewportAttributes_ cameraAttributes;

    if (GLD_bCreateDevice((GLD_tdstDeviceAttributes_*) viewportPacket, &g_stEngineStructure.hGLDDevice)) {
        cameraAttributes.values.dwClipLeftInPerMille = 0;
        cameraAttributes.values.dwClipTopInPerMille = 0;
        cameraAttributes.values.dwClipRightInPerMille = 1000;
        cameraAttributes.values.dwWidthInPercent = 100;
        cameraAttributes.values.dwClipBottomInPerMille = 1000;
        cameraAttributes.values.dwHeightInPercent = 100;
        cameraAttributes.values.iOffsetPosX = 0;
        cameraAttributes.values.iOffsetPosY = 0;

        if (GLD_bCreateViewport(g_stEngineStructure.hGLDDevice, &cameraAttributes,
                                  &g_stEngineStructure.hGLDViewport)) {
            GLD_bGiveSpecificAttributesToViewport(g_stEngineStructure.hGLDDevice,
                                                  g_stEngineStructure.hGLDViewport,
                                                  &inv_GAM_stMainViewportAttributes3D);
        }
    }
    return 1;
}

void func_00100288(void)
{
    while (1) {
#ifdef PC_PORT
        pc_thread_sleep();
#else
        SleepThread();
#endif
    }
}

#ifdef PC_PORT
s32 func_001002A0(void)
{
    extern u8 inv_GAM_a_ucMainThreadStack[];
    s32 PS2_fn_lInit(void);
    void func_00102548(void);

    __main();
    pc_thread_set_priority(pc_thread_create((PcAddress) func_00100288, (PcAddress) inv_GAM_a_ucMainThreadStack, 0x800, 0x3F), 0x7F);
    pc_thread_set_priority(pc_thread_self(), 0x3F);
    PS2_fn_lInit();
    func_00102548();
    if (inv_fn_bCreateDefaultViewportAndCamera()) {
        inv_fn_vMainEngineLoop();
    }
    return 0;
}
#else
s32 func_001002A0(void)
{
    ThreadParam param;
    s32 thread;

    __main();
    memset(&param, 0x30, 0);
    param.entry = func_00100288;
    param.stack = inv_GAM_a_ucMainThreadStack;
    param.stackSize = 0x800;
    param.initPriority = 0x3F;
    param.gpReg = (void*) dword_39A6F0[0];
    thread = CreateThread(&param);
    ChangeThreadPriority(thread, 0x7F);
    ChangeThreadPriority(GetThreadId(), 0x3F);
    func_001000C0();
    func_001000C8();
    if (inv_fn_bCreateDefaultViewportAndCamera() == 0) {
        return 0;
    }
    func_001000D0();
    inv_fn_vMainEngineLoop();
    return 0;
}
#endif

s32 IPT_fn_bIsKeyboardControlAvailable(void)
{
    return g_stEngineStructure.eEngineMode == 2;
}

s32 inv_fn_bIsEngineModeChangeRequested(void)
{
    u8 mode = g_stEngineStructure.eEngineMode;

    return mode == 2 || mode == 6 || mode == 8;
}

void func_001003B0(void)
{
    void* node;
    HIE_tdstSuperObject_* sector;

    inv_fn_vIsolateCurrentCharacterList();

    for (node = fn_hGetFirstAlwaysActiveCharNode(); node != NULL;
         node = fn_hGetNextAlwaysActiveCharNode(node)) {
        HIE_tdstSuperObject_* character = fn_hGetCharacterInAlwaysActiveCharacterNode(node);
        tdstEngineObject_* object = character->hLinkedObject;

        inv_fn_vQueueEngineObjectByPriority(object->h_StandardGame);
    }

    for (sector = SECT_hFatherSector->hFirstElementSta; sector != NULL;
         sector = sector->hNextBrotherSta) {
        SECT_tdstSector_* sectorData = sector->hLinkedObject;
        SECT_tdstListOfSOCharacters_* characterNode;

        for (characterNode = sectorData->stListOfCharacters.hFirstElementDyn; characterNode != NULL;
             characterNode = characterNode->hNextBrotherDyn) {
            tdstEngineObject_* object = characterNode->hPointerOfCharacter->hLinkedObject;

            inv_fn_vQueueEngineObjectByPriority(object->h_StandardGame);
        }
    }

    inv_fn_vInitComportOfCurrentCharacters();
    inv_fn_vIsolateCurrentCharacterList();
}

void fn_vChooseTheGoodInit(void)
{
    switch (g_stEngineStructure.eEngineMode) {
    case 1:
        fn_vFirstInitEngine();
        fn_vFreeNextLevelName();
        fn_vChangeEngineMode(3);
    case 3: {
        char padImage[16];
        char noPadImage[16];
        s32 i;
        s32 padConnected;
        s32 idle;

        inv_IPT_fn_vEnableAllPads();
        for (i = 0; i < 16; i++) {
            fn_vTimerWait(15);
            INO_fn_wReadAllDevices();
        }

        padConnected = inv_IPT_fn_bIsPadConnected(0);
        strcpy(padImage, "rayman_0");
        strcpy(noPadImage, "rm_nopad_0");
        inv_VIG_fn_vDisplayStartupImage(padConnected ? padImage : noPadImage, g_lVIDEO_WantedWidth,
                                        g_lVIDEO_WantedHeight, 1);
        inv_VIG_fn_vDisplayStartupImage(padConnected ? noPadImage : padImage, g_lVIDEO_WantedWidth,
                                        g_lVIDEO_WantedHeight, 0);
        fn_vReinitEngineClock();
        g_stEngineStructure.xDemoMode = 0;

        do {
            u32 now = func_0014BA50();
            u32 deadline = now + func_0014B9F8((f32) inv_DEMO_fn_lGetPendingState());

            do {
                if (inv_IPT_fn_bIsPadConnected(0) != padConnected) {
                    func_00153550();
                    padConnected = (padConnected ^ 1) & 0xFF;
                }
                fn_vTimerWait(15);
                IPT_fn_vReadInput();
                idle = inv_IPT_fn_lGetPadWithButton8Pressed();
            } while (func_0014BA50() < deadline && idle);

            if (idle) {
                inv_DEMO_fn_vStartAutoPlay();
                g_stEngineStructure.xDemoMode = inv_DEMO_bPlaying;
            } else {
                inv_IPT_fn_bSwapJoystickDevices(0, 0);
                fn_vFreeNextLevelName();
            }
            if (g_stEngineStructure.xDemoMode) {
                break;
            }
        } while (idle);

        fn_vInitGameLoop();
        fn_vChangeEngineMode(9);
        fn_vValidateAllOption();
        func_001003B0();
        fn_vFreeNextLevelName();
        inv_SIF_fn_vInitSoundInfos();
        fn_vReinitEngineClock();
        break;
    }
    case 5:
        inv_IPT_fn_vInitInputFrameState();
        GAM_fn_vActualizeAllGameViewportSize();
        fn_vInitLevelLoop();
        fn_vChangeEngineMode(9);
        fn_vValidateAllOption();
        func_001003B0();
        fn_vReinitEngineClock();
#ifdef PC_PORT
        pc_mod_load_mark(PC_LOAD_READY);
#endif
        break;
    case 7:
        fn_vInitDeadLoop();
        fn_vValidateAllOption();
        fn_vChangeEngineMode(9);
        break;
    }
}

void fn_vChooseTheGoodDesInit(void)
{
    switch (g_stEngineStructure.eEngineMode) {
    case 2:
        fn_vDesInitLevelLoop();
        fn_vLastDesinitEngine();
        break;
    case 6:
        if (g_stEngineStructure.xDemoMode) {
            g_stEngineStructure.xDemoMode = 0;
            fn_vDesInitLevelLoop();
            fn_vChangeEngineMode(3);
        } else {
            fn_vDesInitLevelLoop();
            fn_vSetLevelName(fn_p_szGetNextLevelName());
            fn_vChangeEngineMode(5);
        }
        break;
    case 8:
        fn_vDesInitDeadLoop();
        fn_vChangeEngineMode(7);
        break;
    }
}

void fn_vOneTrame(void)
{
    if (IPT_fn_bIsKeyboardControlAvailable()) {
        return;
    }
    fn_vEngine();
    if (g_stEngineStructure.eEngineMode == 2) {
        return;
    }
    fn_vDisplayAll();
    fn_vEngineReadInput();
}
