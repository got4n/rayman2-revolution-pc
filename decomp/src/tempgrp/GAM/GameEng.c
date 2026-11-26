#include "common.h"

#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"
#include "snd/snd.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#include "ps2/dma.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "COL/COL.h"
#include "TMR/TMR.h"
#include "AI/AI.h"
#include "AGO/AGO.h"
#include "SHW/SHW.h"
#include "ps2/PS2.h"

#define LIGHT_TYPE_BACKGROUND 6
#define ENGINE_MODE_PLAYING 9
#define COLOR_SCALE 255.0f

typedef union ClearColor {
    s32 value;
    u8 bytes[4];
} ClearColor;

extern s8 D_00393218;
extern u8 D_0039335A;
extern volatile ClearColor GLI_gs_lBackGroundColor;

f32 fabsf(f32 value);
void fn_vInsertActorInDynamicHierarchy(HIE_tdstSuperObject_* superObject);
void fn_vFillDynamicHierarchy(HIE_tdstSuperObject_* sector);
void fn_vUpdateDynamicHierarchy(void);
s32 GAM_fn_lGetNbPersoActivableInSector(HIE_tdstSuperObject_* superObject);

static inline s32 inv_GAM_fn_lSelectIf(s32 condition, s32 value)
{
    return condition ? value : 0;
}

void fn_vTreatCharactersInSector(HIE_tdstSuperObject_* hCharacter)
{
    gu_tdstStandardGame* standardGame;
    gu_tdst3dData_* data3d;
    void* animation;
    tdstState_* animationState;
    u32 flags;
    s32 skipAnimation;
    s32 updateSector = 1;
    s32 animationGate;
    u64 animationBlocked;
    tdstDynam_* dynamicsComponent;
    DNM_stDynamics* dynamics;
    MTH3D_tdstVector_ position;

    standardGame = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_StandardGame;
    standardGame->ulLastTrame = inv_fn_ulGetEngineTrameNumber();
    position = *inv_HIE_fn_p_stGetGlobalTranslation(hCharacter);
    data3d = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_3dData;
    if (data3d != NULL) {
        animation = NULL;
        animationState = data3d->h_CurrentState;
        flags = standardGame->ulCustomBits;
        if (animationState != NULL) {
            animation = *(void**) animationState;
        }
        skipAnimation = 0;
        if (animation == NULL || ((flags & 0x2000) != 0 && hCharacter->lNumberOfElementsSta > 0) ||
            ((flags & 0x800) != 0 && (flags & 0x1001) != 0) ||
            ((flags & 0x40000) != 0 && (standardGame->ucMiscFlags >> 7) != 0)) {
            skipAnimation = 1;
        }
        if (skipAnimation == 0 || (flags & 0x800000) != 0) {
            animationGate = PLA_fn_bSkipCurrentFrame(hCharacter);
            animationBlocked = data3d->bAnimMatrixChanged;
            if (animationBlocked != 0) {
                animationBlocked = 0;
            } else {
                animationBlocked = (u64) animationGate & 1;
            }
#ifdef PC_PORT
            /* In noclip, Rayman keeps the pose he had. */
            if (pc_mod_noclip_active((PcAddress) hCharacter)) {
                animationBlocked = 1;
            }
#endif
            if ((data3d->ucFlagEndState | animationBlocked) == 0) {
                PLA_fn_bDoAnimPlayerForCharacter(hCharacter, skipAnimation, 0);
            }
        }
        fn_vMakeCharacterReact(hCharacter);
        if (inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_StandardGame == NULL) {
            return;
        }
    }
    fn_vUpdateLight(hCharacter);
    if (((hCharacter->flags >> 5) & 1) != 0) {
        return;
    }
    if (CAM_fn_bSuperObjectIsACamera(hCharacter) != 0 && g_stEngineStructure.cCameraMode == 1 &&
        (inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_Cineinfo->hWork->uwDNMFlags & 0x40) != 0) {
        return;
    }
    dynamicsComponent = inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_Dynam;
    if (dynamicsComponent != NULL) {
        dynamics = dynamicsComponent->p_stDynamics;
        updateSector = 1;
        if ((dynamics->m_tdstDynamicsBaseBlock.ulEndFlags & 2) != 0 &&
            (dynamics->m_tdstDynamicsBaseBlock.ulFlags & 2) != 0) {
            updateSector = dynamics->m_tdstDynamicsAdvancedBlock.m_cCollideCounter == 0;
        }
    }
#ifdef PC_PORT
    /* In noclip his mechanics, which count down to the next test, are left out: test each frame. */
    if (pc_mod_noclip_active((PcAddress) hCharacter)) {
        updateSector = 1;
    }
#endif
    if (updateSector != 0) {
        GAM_fn_vSectInfoTestChangeSectorForCharacter(hCharacter);
    }
}

void fn_vTreatDynamicHierarchy(HIE_tdstSuperObject_* superObject, s32 treatChildren)
{
    HIE_tdstSuperObject_* child;
    gu_tdstStandardGame* standardGame;
    tdstEngineStructure_* engine;
    POS_tdstMatrix_* referenceMatrix;
    POS_tdstMatrix_* objectMatrix;
    f32* objectPosition;
    f32* referencePosition;
    s32 childMode = 0;
    s32 active;
    s32 processingStamp;
    f32 positionDelta[3];
    f32 distance;

    treatChildren &= 0xFF;
    HIE_fn_vPushSuperObjectMatrix(superObject);
    if (superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        standardGame = inv_HIE_fn_p_stGetEngineObject(superObject)->h_StandardGame;
        engine = &g_stEngineStructure;
        if (engine->bEngineFrozen == 0 || (standardGame->ulCustomBits & 0x100000) != 0) {
            active = 1;
            childMode = inv_GAM_fn_lSelectIf(treatChildren, 1);
            if (((standardGame->ucMiscFlags >> 1) & 1) == active) {
                processingStamp = standardGame->ulLastTrame;
                if (processingStamp != inv_fn_ulGetEngineTrameNumber()) {
                    referenceMatrix = engine->activeSuperObject->gu_hGlobalMatrix;
                    referencePosition = &referenceMatrix->stMatrix.stCol_3.xX;
                    if (standardGame != NULL && (standardGame->ulCustomBits & 0x4C0000) != 0) {
                        objectMatrix = superObject->gu_hGlobalMatrix;
                        objectPosition = &objectMatrix->stMatrix.stCol_3.xX;
                        positionDelta[0] = objectPosition[0] - referenceMatrix->stMatrix.stCol_3.xX;
                        positionDelta[1] = objectPosition[1] - referencePosition[1];
                        positionDelta[2] = objectPosition[2] - referencePosition[2];
                        distance = inv_MTH_fn_xAbsolute(positionDelta[0]) +
                                   inv_MTH_fn_xAbsolute(positionDelta[1]) +
                                   inv_MTH_fn_xAbsolute(positionDelta[2]);
                        if (distance < (f32) (u32) standardGame->ucTooFarLimit) {
                            standardGame->ucMiscFlags &= 0x7F;
                        } else {
                            standardGame->ucMiscFlags |= 0x80;
                        }
                    }
                    fn_vTreatCharactersInSector(superObject);
                    childMode = 1;
                }
            }
            HIE_fn_vPopMatrix();
            HIE_fn_vPushSuperObjectMatrix(superObject);
        }
    }
    for (child = superObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        fn_vTreatDynamicHierarchy(child, childMode);
    }
    HIE_fn_vPopMatrix();
}

void func_00100BD0(HIE_tdstSuperObject_* sectorObject)
{
    SECT_tdstSector_* sector = sectorObject->hLinkedObject;
    SECT_tdstListOfStaticLights_* light = sector->stListOfStaticLights.hFirstElementOpt;
    MTH4D_tdstVector_ color;
    s32 red;
    s32 green;
    s32 blue;
    s16 i;

    for (i = 0; i < sector->stListOfStaticLights.lNumberOfElementsOpt; i++, light++) {
        if (GLI_lGetLightType(light->hStaticLights) == LIGHT_TYPE_BACKGROUND) {
            GLI_xGetLightBackgroundColor(light->hStaticLights, &color);
            green = color.xY * COLOR_SCALE;
            red = color.xX * COLOR_SCALE;
            blue = color.xZ * COLOR_SCALE;
            inv_GLI_fn_bSetBackGroundColor(0, (red & 0xFF) | ((green & 0xFF) << 8) | ((blue & 0xFF) << 16));
            return;
        }
    }
}

#ifdef PC_PORT
static void inv_GAM_fn_vSendWorldToViewport(s32 milliseconds)
{
    s16 device = g_stEngineStructure.hGLDDevice;
    s16 viewport = g_stEngineStructure.hGLDViewport;
    s32 ready;
    HIE_tdstSuperObject_* camera;
    tdstEngineObject_* cameraObject;
    HIE_tdstSuperObject_* sector;
    MTH3D_tdstVector_ position;

    func_001479A0();
    RW3_SHW_fn_vBeginRenderCharacterInShadowView();
    inv_PS2_fn_vSendVif0Dma(dword_251ED0);
    inv_SHW_fn_vRenderShadowQueue();
    if (D_00393218 != 0) {
        GLD_bClearViewportWithChoice(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, 3);
    } else {
        GLD_bClearViewportWithChoice(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, 2);
    }
    GLD_bBeginWriteToDevice(device);
    GLI_vUpdateEffectData(milliseconds);
    GLD_bRequestWriteToViewport(device, viewport, &g_stEngineStructure.stViewportAttr, &ready);
    cameraObject = (tdstEngineObject_*) ready;
    if (cameraObject != NULL) {
        camera = CAM_fn_hGetActiveCamera(1);
        position = *inv_HIE_fn_p_stGetGlobalTranslation(camera);
        cameraObject = camera->hLinkedObject;
        sector = cameraObject->h_SectInfo->hCurrentSector;
        CAM_fn_vApplyTransparencyOnSuperObject(cameraObject->h_Cineinfo);
        func_00100BD0(sector);
        SCT_fn_vSendSectorWhereIAmToViewportWithMirror(
            &position, g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
            &g_stEngineStructure.stViewportAttr, sector, g_stEngineStructure.ulDisplayMode);
        inv_SHW_fn_vRenderQueuedProjectedShadows(&g_stEngineStructure.stViewportAttr,
                                                 g_stEngineStructure.ulDisplayMode);
        inv_MEC_fn_vUpdateAllEffectSystems(&g_stEngineStructure.stViewportAttr, g_stEngineStructure.ulDisplayMode);
        CAM_fn_vRestoreTransparencyOnSuperObject(inv_HIE_fn_p_stGetEngineObject(camera)->h_Cineinfo);
        AGO_vDisplayAllParts();
    }
}

static void inv_GAM_fn_vDrawViewports(void)
{
    u8 color[4];

    color[2] = D_0039335A;
    color[1] = (GLI_gs_lBackGroundColor.value & 0xFF00) >> 8;
    color[0] = GLI_gs_lBackGroundColor.bytes[0];
    color[3] = 0xFF;
    func_00153800(color);
    func_00152978(&g_stEngineStructure.stViewportAttr);
    inv_RW3_fn_vFlushAllQuadBatches(&g_stEngineStructure.stViewportAttr);
    inv_RW3_fn_vDrawQueuedHalos(&g_stEngineStructure.stFixViewportAttr);
    inv_GLI_fn_vRenderPrepared2DRectangle();
    GLD_bWriteToViewportFinished(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport);
    inv_fn_vDisplayPauseOrOptionsOverlay();
    RW3_SHW_fn_vEndRenderCharacterInShadowView();
    GLD_bEndWriteToDevice(g_stEngineStructure.hGLDDevice, &g_stEngineStructure.stViewportAttr);
    GLD_bFlipDeviceWithSynchro(g_stEngineStructure.hGLDDevice, 0xFE);
}

s32 inv_GAM_bDrawingInBetween;

extern s32 HIE_gs_lCurrentFrame;
void CAM_fn_vActualizeAViewport(s32 viewport);

#define INBETWEEN_SUPERIMPOSED 0x20

static void inv_GAM_fn_vKeepMatrices(HIE_tdstSuperObject_* superObject)
{
    HIE_tdstSuperObject_* child;

    if (superObject->flags & INBETWEEN_SUPERIMPOSED) {
        return;
    }
    pc_inbetween_keep((PcAddress) superObject, superObject->gu_lLastComputeFrame == HIE_gs_lCurrentFrame ||
                                                    superObject->gu_lLastComputeFrame == -1);
    for (child = superObject->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        inv_GAM_fn_vKeepMatrices(child);
    }
}

static s32 inv_GAM_fn_bDrawFrameInBetween(s32 milliseconds)
{
    HIE_tdstSuperObject_* camera;

    if (!pc_inbetween_wanted() || gp_stDynamicWorld == NULL) {
        return 0;
    }
    camera = CAM_fn_hGetActiveCamera(1);
    pc_inbetween_keep_begin();
    inv_GAM_fn_vKeepMatrices(gp_stDynamicWorld);
    if (camera != NULL) {
        pc_inbetween_keep((PcAddress) camera, 1);
    }
    if (!pc_inbetween_move()) {
        return 0;
    }
    if (camera != NULL) {
        CAM_fn_vActualizeAViewport(1);
    }
    inv_GAM_bDrawingInBetween = 1;
    inv_GAM_fn_vSendWorldToViewport(milliseconds);
    inv_GAM_fn_vDrawViewports();
    inv_GAM_bDrawingInBetween = 0;
    pc_inbetween_restore();
    if (camera != NULL) {
        CAM_fn_vActualizeAViewport(1);
    }
    pc_frame_wait_half();
    return 1;
}

void fn_vEngine(void)
{
    s32 milliseconds;

    inv_COL_RAY_fn_vInitHitBuffer();
    if (inv_GAM_ulFramesInMode == 5 && g_bBeginMapSoundEventFlag != 0) {
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMusic, GAM_p_stGetBlockEventSlotMemory(0), 0, 0);
    }
    SND_fn_vSynchroSound();
    if (fn_bIsEngineInPause() == 0) {
        HIE_fn_vInvalidateAllGlobalMatrices();
        if (g_stEngineStructure.pendingSuperObject != NULL) {
            g_stEngineStructure.activeSuperObject = g_stEngineStructure.pendingSuperObject;
            g_stEngineStructure.pendingSuperObject = NULL;
        }
        fn_vUpdateDynamicHierarchy();
        fn_vKillAllAlwaysInNonActiveSectors();
        HIE_fn_vInitMatrixStack();
        inv_fn_vTreatQueuedCharactersBySector(g_stEngineStructure.activeSuperObject,
                                              inv_HIE_fn_p_stGetGlobalTranslation(g_stEngineStructure.activeSuperObject));
        CAM_fn_vCameraManagement();
        GLI_vRefreshAllCyclingTextures(inv_fn_xGetEngineClockDelta() /
                                       inv_fn_xGetEngineClockScale());
    }
    if ((inv_GAM_ulFramesInMode >= 6 && g_stEngineStructure.eEngineMode == ENGINE_MODE_PLAYING) ||
        g_stEngineStructure.bEngineFrozen != 0) {
        milliseconds = inv_MTH_fn_lToIntegerOrdered(inv_fn_xGetEngineClockDelta() * 1000.0f);
        if (fn_bIsEngineInPause() == 0 && g_stEngineStructure.bEngineFrozen == 0 &&
            inv_GAM_fn_bDrawFrameInBetween(milliseconds / 2)) {
            milliseconds -= milliseconds / 2;
        }
        inv_GAM_fn_vSendWorldToViewport(milliseconds);
    }
}
#else
void fn_vEngine(void)
{
    s16 device = g_stEngineStructure.hGLDDevice;
    s16 viewport = g_stEngineStructure.hGLDViewport;
    s32 milliseconds;
    s32 ready;
    HIE_tdstSuperObject_* camera;
    tdstEngineObject_* cameraObject;
    HIE_tdstSuperObject_* sector;
    MTH3D_tdstVector_ position;

    inv_COL_RAY_fn_vInitHitBuffer();
    if (inv_GAM_ulFramesInMode == 5 && g_bBeginMapSoundEventFlag != 0) {
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMusic, GAM_p_stGetBlockEventSlotMemory(0), 0, 0);
    }
    SND_fn_vSynchroSound();
    if (fn_bIsEngineInPause() == 0) {
        HIE_fn_vInvalidateAllGlobalMatrices();
        if (g_stEngineStructure.pendingSuperObject != NULL) {
            g_stEngineStructure.activeSuperObject = g_stEngineStructure.pendingSuperObject;
            g_stEngineStructure.pendingSuperObject = NULL;
        }
        fn_vUpdateDynamicHierarchy();
        fn_vKillAllAlwaysInNonActiveSectors();
        HIE_fn_vInitMatrixStack();
        inv_fn_vTreatQueuedCharactersBySector(g_stEngineStructure.activeSuperObject,
                                              inv_HIE_fn_p_stGetGlobalTranslation(g_stEngineStructure.activeSuperObject));
        CAM_fn_vCameraManagement();
        GLI_vRefreshAllCyclingTextures(inv_fn_xGetEngineClockDelta() /
                                       inv_fn_xGetEngineClockScale());
    }
    if ((inv_GAM_ulFramesInMode >= 6 && g_stEngineStructure.eEngineMode == ENGINE_MODE_PLAYING) ||
        g_stEngineStructure.bEngineFrozen != 0) {
        func_001479A0();
        RW3_SHW_fn_vBeginRenderCharacterInShadowView();
        inv_PS2_fn_vSendVif0Dma(dword_251ED0);
        inv_SHW_fn_vRenderShadowQueue();
        if (D_00393218 != 0) {
            GLD_bClearViewportWithChoice(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, 3);
        } else {
            GLD_bClearViewportWithChoice(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDFixViewport, 2);
        }
        GLD_bBeginWriteToDevice(device);
        milliseconds = inv_MTH_fn_lToIntegerOrdered(inv_fn_xGetEngineClockDelta() * 1000.0f);
        GLI_vUpdateEffectData(milliseconds);
        GLD_bRequestWriteToViewport(device, viewport, &g_stEngineStructure.stViewportAttr,
                                    &ready);
        cameraObject = (tdstEngineObject_*) ready;
        if (cameraObject != NULL) {
            camera = CAM_fn_hGetActiveCamera(1);
            position = *inv_HIE_fn_p_stGetGlobalTranslation(camera);
            cameraObject = camera->hLinkedObject;
            sector = cameraObject->h_SectInfo->hCurrentSector;
            CAM_fn_vApplyTransparencyOnSuperObject(cameraObject->h_Cineinfo);
            func_00100BD0(sector);
            SCT_fn_vSendSectorWhereIAmToViewportWithMirror(
                &position, g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                &g_stEngineStructure.stViewportAttr, sector, g_stEngineStructure.ulDisplayMode);
            inv_SHW_fn_vRenderQueuedProjectedShadows(&g_stEngineStructure.stViewportAttr,
                                                     g_stEngineStructure.ulDisplayMode);
            inv_MEC_fn_vUpdateAllEffectSystems(&g_stEngineStructure.stViewportAttr, g_stEngineStructure.ulDisplayMode);
            CAM_fn_vRestoreTransparencyOnSuperObject(inv_HIE_fn_p_stGetEngineObject(camera)->h_Cineinfo);
            AGO_vDisplayAllParts();
        }
    }
}
#endif

void fn_vReputCharacterSuperObjectAtTheWorld(HIE_tdstSuperObject_* h_SupObject)
{
    HIE_fn_vChangeFather(h_SupObject, gp_stDynamicWorld, inv_fn_ucSuperObjectGetPlatFormType(h_SupObject));
}

void func_00100F88(HIE_tdstSuperObject_* world)
{
    HIE_tdstSuperObject_* node = world->hFirstElementSta;
    HIE_tdstSuperObject_* nextNode = HIE_fn_hForceGetSuperObjectNextBrother(node);
    HIE_tdstSuperObject_* child;
    HIE_tdstSuperObject_* nextChild;

    while (node != NULL) {
        if (node->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            if (inv_fn_ucSuperObjectGetPlatFormType(node) == 0) {
                return;
            }
            child = node->hFirstElementSta;
            nextChild = HIE_fn_hForceGetSuperObjectNextBrother(child);
            while (child != NULL) {
                if (child->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
                    HIE_fn_vChangeFather(child, world, 0);
                }
                child = nextChild;
                nextChild = HIE_fn_hForceGetSuperObjectNextBrother(child);
            }
        }
        node = nextNode;
        nextNode = HIE_fn_hForceGetSuperObjectNextBrother(node);
    }
}

void fn_vDestructAllFatherSonLinks(void)
{
    HIE_tdstSuperObject_* hMainCharacter = g_stEngineStructure.activeSuperObject->hFirstElementSta;
    HIE_tdstSuperObject_* hNextChild = HIE_fn_hForceGetSuperObjectNextBrother(hMainCharacter);

    while (hMainCharacter != NULL) {
        if (hMainCharacter->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            HIE_fn_vChangeFather(hMainCharacter, gp_stDynamicWorld, inv_fn_ucSuperObjectGetPlatFormType(hMainCharacter));
        }
        hMainCharacter = hNextChild;
        hNextChild = HIE_fn_hForceGetSuperObjectNextBrother(hMainCharacter);
    }
    func_00100F88(gp_stDynamicWorld);
}

#ifdef PC_PORT
void fn_vDisplayAll(void)
{
    u32 limit;

    if ((inv_GAM_ulFramesInMode >= 6 && g_stEngineStructure.eEngineMode == ENGINE_MODE_PLAYING) ||
        g_stEngineStructure.bEngineFrozen != 0) {
        inv_GAM_fn_vDrawViewports();
    }
    if (inv_DEMO_bPlaying != 0) {
        limit = fn_lGetTemporalCurveType() * pc_frame_units_per_tick() - 1;
    } else if (pc_frame_vblanks() == 0) {
        limit = 1;
    } else {
        limit = pc_frame_vblanks();
    }
    pc_frame_wait(limit);
    if (g_stEngineStructure.bEngineFrozen == 0) {
        inv_GAM_ulFramesInMode++;
    }
    pc_mod_load_frame(inv_GAM_ulFramesInMode);
    fn_vActualizeEngineClock();
    pc_frame_start();
}
#else
void fn_vDisplayAll(void)
{
    u8 color[4];
    u32 limit;

    if ((inv_GAM_ulFramesInMode >= 6 && g_stEngineStructure.eEngineMode == ENGINE_MODE_PLAYING) ||
        g_stEngineStructure.bEngineFrozen != 0) {
        color[2] = D_0039335A;
        color[1] = (GLI_gs_lBackGroundColor.value & 0xFF00) >> 8;
        color[0] = GLI_gs_lBackGroundColor.bytes[0];
        color[3] = 0xFF;
        func_00153800(color);
        func_00152978(&g_stEngineStructure.stViewportAttr);
        inv_RW3_fn_vFlushAllQuadBatches(&g_stEngineStructure.stViewportAttr);
        inv_RW3_fn_vDrawQueuedHalos(&g_stEngineStructure.stFixViewportAttr);
        inv_GLI_fn_vRenderPrepared2DRectangle();
        GLD_bWriteToViewportFinished(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport);
        inv_fn_vDisplayPauseOrOptionsOverlay();
        RW3_SHW_fn_vEndRenderCharacterInShadowView();
        GLD_bEndWriteToDevice(g_stEngineStructure.hGLDDevice, &g_stEngineStructure.stViewportAttr);
        GLD_bFlipDeviceWithSynchro(g_stEngineStructure.hGLDDevice, 0xFE);
    }
    if (inv_DEMO_bPlaying != 0) {
        limit = fn_lGetTemporalCurveType() - 1;
    } else if (inv_PS2_vblankCount == 0) {
        limit = 1;
    } else {
        limit = inv_PS2_vblankCount;
    }
    while (limit >= inv_PS2_vblankCount) {
    }
    if (g_stEngineStructure.bEngineFrozen == 0) {
        inv_GAM_ulFramesInMode++;
    }
    fn_vActualizeEngineClock();
    inv_PS2_vblankCount = 0;
}
#endif

s32 GAM_fn_lGetNbPersoActivableInSector(HIE_tdstSuperObject_* superObject)
{
    SECT_tdstSector_* sector = superObject->hLinkedObject;
    SECT_tdstListOfSOCharacters_* node = sector->stListOfCharacters.hFirstElementDyn;
    s32 count = 0;

    while (node != NULL) {
        HIE_tdstSuperObject_* character = node->hPointerOfCharacter;
        SECT_tdstListOfSOCharacters_* next = node->hNextBrotherDyn;

        count += (inv_HIE_fn_p_stGetEngineObject(character)->h_StandardGame->ucMiscFlags >> 1) & 1;
        node = next;
    }
    return count;
}

s32 GAM_fn_lGetNbPersoActive(void)
{
    HIE_tdstSuperObject_* sectorObject =
        inv_HIE_fn_p_stGetEngineObject(g_stEngineStructure.activeSuperObject)->h_SectInfo->hCurrentSector;
    s32 count = GAM_fn_lGetNbPersoActivableInSector(sectorObject);
    SECT_tdstSector_* sector = sectorObject->hLinkedObject;
    s32 i = sector->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt;
    SECT_tdstListOfSOSectorsInActivityInteraction_* border = sector->stListOfSectorsInActivityInteraction.hFirstElementOpt;

    while (i > 0) {
        HIE_tdstSuperObject_* borderSector = border->hPointerOfSectorSO;

        i--;
        border++;
        count += GAM_fn_lGetNbPersoActivableInSector(borderSector);
    }
    return count;
}

static inline tdstEngineObject_* inv_GAM_fn_p_stCharacterOf(HIE_tdstSuperObject_* superObject)
{
    return superObject->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv
               ? superObject->hLinkedObject
               : NULL;
}

static inline u8 inv_GAM_fn_bIsDynamicWorld(StandardGameList* world)
{
    return world == inv_GAM_currentCharacterList || world == inv_GAM_previousCharacterList;
}

s32 inv_fn_bIsSuperObjectInDynamicWorld(HIE_tdstSuperObject_* superObject)
{
    s32 result = 0;

    if (inv_GAM_fn_bIsDynamicWorld(inv_HIE_fn_p_stGetEngineObject(superObject)->h_StandardGame->listAnchor)) {
        result = (inv_GAM_fn_p_stCharacterOf(superObject)->h_StandardGame->ucMiscFlags >> 2) & 1;
    }
    return result;
}

void fn_vDesinitInactivePerso(void)
{
    gu_tdstStandardGame* node;
    HIE_tdstSuperObject_* character;
    tdstEngineObject_* object;
    gu_tdstStandardGame* standardGame;
    s32 active;

    node = inv_GAM_previousCharacterList->first;
    if (node == NULL) {
        return;
    }
    character = node->h_SuperObject;
    active = 1;
    for (; node != NULL; node = node->listNext) {
        character = node->h_SuperObject;
        object = character->hLinkedObject;
        standardGame = object->h_StandardGame;
        if (standardGame == NULL || ((standardGame->ucMiscFlags >> 2) & 1) != active) {
            continue;
        }
        if ((node->ucMiscFlags & 1) == 0) {
            func_001DEE58(character, object);
        }
        PLA_fn_vDesInitAllChildOfCharacter(character, character->hLinkedObject);
        inv_HIE_fn_p_stGetEngineObject(character)->h_StandardGame->ucMiscFlags &= 0xFB;
        SND_fn_vKillObjectSound2((s32) character, g_lSoundObjectTypeAnim);
    }
}

void func_00101438(HIE_tdstSuperObject_* superObject)
{
    if (inv_fn_ucSuperObjectGetPlatFormType(superObject) != 0) {
        HIE_fn_vSuperObjectAddHead(gp_stDynamicWorld, superObject);
    } else {
        HIE_fn_vSuperObjectAddTail(gp_stDynamicWorld, superObject);
    }
}

void fn_vInsertActorInDynamicHierarchy(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* engineObject = superObject->hLinkedObject;
    gu_tdstStandardGame* standardGame = engineObject->h_StandardGame;
    s32 inactive = standardGame->listAnchor != inv_GAM_previousCharacterList;
    u8 flags;
    s32 loaded;
    s64 wide;

    if (CAM_fn_bSuperObjectIsACamera(superObject) != 0)
        return;
    if (superObject->hFatherSta == NULL)
        return;
    flags = engineObject->h_StandardGame->ucMiscFlags;
    switch ((flags >> 1) & 1) {
    case 1:
        break;
    default:
        return;
    }
    loaded = flags & 1;
    if (inactive) {
        if ((s16) loaded == 0 || engineObject->h_StandardGame->eInitFlagWhenDeadOrTaken < 2)
            goto queue;
        wide = loaded;
    } else {
        wide = (s16) loaded;
    }
    if (wide != 0)
        return;
queue:
    if (standardGame->listAnchor == inv_GAM_currentCharacterList)
        return;
    inv_fn_vQueueEngineObjectByPriority(standardGame);
    engineObject->h_StandardGame->ucMiscFlags |= 4;
    if (inactive)
        standardGame->ulCustomBits |= 0x800;
}

void fn_vFillDynamicHierarchy(HIE_tdstSuperObject_* sectorObject)
{
    SECT_tdstSector_* sector;
    SECT_tdstListOfSOCharacters_* characterNode;
    SECT_tdstListOfSOSectorsInActivityInteraction_* border;
    s32 borderCount;
    void* alwaysNode;
    HIE_tdstSuperObject_** generator;
    HIE_tdstSuperObject_** generatorEnd;
    HIE_tdstSuperObject_* object;
    s16 superimposedCount;
    s16 index;

    sector = sectorObject->hLinkedObject;
    for (characterNode = sector->stListOfCharacters.hFirstElementDyn; characterNode != NULL;
         characterNode = characterNode->hNextBrotherDyn) {
        fn_vInsertActorInDynamicHierarchy(characterNode->hPointerOfCharacter);
    }
    sector = sectorObject->hLinkedObject;
    borderCount = sector->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt;
    border = sector->stListOfSectorsInActivityInteraction.hFirstElementOpt;
    for (; borderCount > 0; borderCount--, border++) {
        HIE_tdstSuperObject_* borderSector = border->hPointerOfSectorSO;
        SECT_tdstSector_* borderInfo = borderSector->hLinkedObject;

        characterNode = borderInfo->stListOfCharacters.hFirstElementDyn;
        while (characterNode != NULL) {
            fn_vInsertActorInDynamicHierarchy(characterNode->hPointerOfCharacter);
            characterNode = characterNode->hNextBrotherDyn;
        }
    }
    for (alwaysNode = fn_hGetFirstAlwaysActiveCharNode(); alwaysNode != NULL;
         alwaysNode = fn_hGetNextAlwaysActiveCharNode(alwaysNode)) {
        fn_vInsertActorInDynamicHierarchy(fn_hGetCharacterInAlwaysActiveCharacterNode(alwaysNode));
    }
    generator = g_stAlways.d_hAlwaysGenerator;
    object = g_stAlways.d_stAlwaysSuperObject;
    generatorEnd = generator + g_stAlways.ulMaxNumberOfAlways;
    while (generator < generatorEnd) {
        if (inv_HIE_fn_p_stGetEngineObject(object)->h_StandardGame != NULL) {
            fn_vInsertActorInDynamicHierarchy(object);
        }
        generator++;
        object++;
    }
    superimposedCount = g_xFixIndex;
    for (index = 0; index < superimposedCount; index++) {
        fn_vInsertActorInDynamicHierarchy(HIE_fn_hGetElementFormFixViewport(index));
    }
}

void fn_vUpdateDynamicHierarchy(void)
{
    HIE_tdstSuperObject_* currentSector =
        inv_HIE_fn_p_stGetEngineObject(g_stEngineStructure.activeSuperObject)->h_SectInfo->hCurrentSector;
    StandardGameList* previousWorld;
    StandardGameList* nextWorld;
    HIE_tdstSuperObject_* previousSector;

    if (currentSector != inv_GAM_currentSector) {
        inv_fn_vIsolatePreviousCharacterList();
        inv_fn_vDeactivateAllEngineObjectNodes();
        previousWorld = inv_GAM_currentCharacterList;
        nextWorld = inv_GAM_previousCharacterList;
        previousSector = inv_GAM_currentSector;
        inv_GAM_currentSector = currentSector;
        inv_GAM_currentCharacterList = nextWorld;
        inv_GAM_previousCharacterList = previousWorld;
        inv_GAM_previousSector = previousSector;
        fn_vFillDynamicHierarchy(currentSector);
        fn_vDesinitInactivePerso();
        inv_fn_vRecycleAllActiveEngineObjectNodes();
        inv_fn_vIsolatePreviousCharacterList();
    }
}
