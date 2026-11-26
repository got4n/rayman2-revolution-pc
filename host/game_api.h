#ifndef REVO_GAME_API_H
#define REVO_GAME_API_H

#include "rt.h"

typedef u32 GuestPtr;

#define GAME_API

GAME_API GuestPtr AI_fn_p_vTrueAlloc(GuestPtr heap, u32 size);
GAME_API void inv_MEM_fn_vFreeInHeap(GuestPtr heap, GuestPtr block);
GAME_API GuestPtr inv_MEM_fn_p_vAllocAligned(GuestPtr memory, u32 size, u32 align, u32 flags);
GAME_API void inv_MEM_fn_vFree(GuestPtr where);
GAME_API u32 inv_fn_ulGetBiggestFreeBlockSize(void);
GAME_API void inv_MEM_fn_vInitDefaultHeap(u32 size);
GAME_API void inv_MEM_fn_vCreateLevelSlotHeap(u32 size, u32 slot);
GAME_API void inv_MEM_fn_vFreeLevelSlotHeap(u32 slot);

GAME_API u32 inv_FMN_fn_ulOpenVirtualOrFallback(GuestPtr path);
GAME_API s32 FMN_Length(u32 file);
GAME_API s32 FMN_Read(u32 file, GuestPtr buffer, u64 size, u64 count);
GAME_API s32 FMN_Close(u32 file);
GAME_API GuestPtr FMN_ConvertPathUnix2Dos(GuestPtr path);

GAME_API u32 GLI_xIsGliInit(void);
GAME_API u32 inv_fn_ulGetEngineTrameNumber(void);
GAME_API void GLI_vFogOn(void);
GAME_API void GLI_vFogOff(void);
GAME_API void GLI_vDoMaterialSelection(void);
GAME_API void GLI_xGetMaterialTexture(GuestPtr material, GuestPtr texture_out);
GAME_API void GLI_xRefreshAnimatedTexture(GuestPtr material);
GAME_API void GLI_vSetLightSwitch(GuestPtr light, u32 on);
GAME_API GuestPtr GLI_pGetVisualMaterialForLetter(u32 letter);

GAME_API void POS_fn_vMulMatrixVertex(GuestPtr result, GuestPtr matrix, GuestPtr point);
GAME_API void POS_fn_vApplyMatrixToVector(GuestPtr result, GuestPtr matrix, GuestPtr vector);
GAME_API void POS_fn_vMulMatrixVertexArray(GuestPtr out, GuestPtr matrix, GuestPtr in, u32 count);
GAME_API void _POS_fn_vMulMatrixMatrix(GuestPtr out, GuestPtr a, GuestPtr b);
GAME_API void POS_fn_vInvertMatrix(GuestPtr out, GuestPtr in);
GAME_API void inv_POS_fn_vRemoveScaleFromMatrix(GuestPtr out, GuestPtr matrix);
GAME_API float POS_fn_xGetXScale(GuestPtr matrix);

GAME_API GuestPtr CAM_fn_hGetActiveCamera(s32 index);
GAME_API u32 inv_COL_fn_bIntersectSegmentWithLevel(GuestPtr segment);
GAME_API s32 fn_vTimerWait(s32 ticks);

GAME_API GuestPtr fn_hIsThisZoneActive(GuestPtr character, s32 type, s32 number);
GAME_API s32 fn_bIsCharacterInZdd(GuestPtr geometry, GuestPtr matrix, GuestPtr character);
GAME_API s32 fn_eGetPrivilegedActivationZoneOfCharacter(u32 type, GuestPtr character, s32 index);
GAME_API GuestPtr fn_p_stGetCurrentZoneSetOfType2(u32 type, GuestPtr character);

void game_fn_void(GuestPtr fn);
void game_fn_void_u32(GuestPtr fn, u32 a);
u32 game_fn_u32(GuestPtr fn);
s32 game_fn_compare(GuestPtr fn, GuestPtr a, GuestPtr b);
s32 game_fn_main(GuestPtr fn, s32 argc, GuestPtr argv);

GuestPtr game_stack_push(u32 size);
void game_stack_pop(u32 size);

#endif
