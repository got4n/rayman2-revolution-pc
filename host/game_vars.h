#ifndef REVO_GAME_VARS_H
#define REVO_GAME_VARS_H

#include "rt.h"

#define GAME_VARS(X) \
    X(inv_GLI_renderContext) \
    X(gs_hDefaultMaterial) \
    X(inv_GLI_cTextGradientMode) \
    X(GLI_fF2pow24) \
    X(inv_PS2_vblankCount) \
    X(inv_SNA_p_hLevelGeometricObjects) \
    X(inv_RW3_p_pLevelMaterials) \
    X(inv_RW3_p_stLevelTextures) \
    X(inv_RW3_ulLevelTextureCount) \
    X(inv_RW3_p_pLevelLights) \
    X(inv_RW3_lightmapSlots) \
    X(inv_RW3_ulLightmapSlotCount) \
    X(inv_RW3_p_stLightHalos) \
    X(inv_RW3_p_ucQueuedHalos) \
    X(inv_GLI_p_stHaloMaterial) \
    X(inv_SCT_ucDrawPass) \
    X(inv_GLI_blendModeState) \
    X(inv_GLI_zWriteState) \
    X(inv_GLI_zTestState) \
    X(inv_SCT_bSendingSectorToViewport) \
    X(inv_RW3_p_stWorldHeap) \
    X(inv_GLI_packetCursor) \
    X(inv_FMN_bCdReadRetrying) \
    X(inv_MPEG_pictureX) \
    X(inv_MPEG_pictureY) \
    X(inv_MPEG_displayBuffer) \
    X(GLI_fSinEffectBase) \
    X(IPT_g_hInputStructure) \
    X(RW3_g_pWorld) \
    X(a_stConditionTable) \
    X(a_stFunctionTable) \
    X(a_stKeyWordTable) \
    X(a_stMetaActionTable) \
    X(a_stOperatorTable) \
    X(a_stProcedureTable) \
    X(__CTOR_LIST__) \
    X(__malloc_av_) \
    X(__malloc_current_mallinfo) \
    X(__malloc_max_sbrked_mem) \
    X(__malloc_max_total_mem) \
    X(__malloc_sbrk_base) \
    X(__malloc_top_pad) \
    X(__malloc_trim_threshold) \
    X(_impure_ptr) \
    X(byte_2538AA) \
    X(inv_GLI_ucTextGradientRed) \
    X(dword_250EB0) \
    X(dword_251ED0) \
    X(dword_252570) \
    X(dword_2525E8) \
    X(dword_252618) \
    X(dword_387B4C) \
    X(dword_394940) \
    X(dword_412790) \
    X(dword_39A6F0) \
    X(errno) \
    X(func_001002A0) \
    X(a64_fSinTable) \
    X(g_AlphabetCharacterPointer) \
    X(g_BaseTimeForParticleAnimation) \
    X(g_bHasTextureAnimationFinished) \
    X(g_bOneShotAnim) \
    X(g_bTakeCareOfAnimationCoordination) \
    X(g_lVIDEO_WantedHeight) \
    X(g_p_stCurrentMatrix) \
    X(g_stEngineStructure) \
    X(gp_6F74) \
    X(gp_stDynamicWorld) \
    X(g_stAlways) \
    X(g_hCurrentSuperObjPerso) \
    X(g_p_stIntelligence) \
    X(g_ucNRule) \
    X(gs_CurrentNumberOfLightInViewport) \
    X(gs_aDEF_p_CurrentListOfLight) \
    X(gs_fCoefOfMultiplicationForAnimatedTexture) \
    X(gs_lActualRefreshNumberForScolledTextures) \
    X(initialized_10) \
    X(inv_FMN_pathCharacterFlags) \
    X(inv_GAM_engineClockState) \
    X(inv_GLI_currentCamera) \
    X(a3_xSinEffectBaseMesh) \
    X(inv_MPEG_decoder) \
    X(inv_MPEG_pictureBuffer) \
    X(inv_TXT_textTableState) \
    X(off_3887FC) \
    X(off_388808) \
    X(qword_38A3A0) \
    X(qword_38A3B0) \
    X(stSinEffectAmp) \
    X(inv_GLI_stDisplayState) \
    X(inv_MPEG_retryIconOn) \
    X(inv_MPEG_retryIconOff) \

#define GAME_VAR_DECL(name) extern const u32 game_var_##name;
GAME_VARS(GAME_VAR_DECL)
#undef GAME_VAR_DECL

#define GAME_VAR(name) (game_var_##name)

#endif
