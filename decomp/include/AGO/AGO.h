#ifndef REVO_AGO_AGO_H
#define REVO_AGO_AGO_H

#include "ps2/types.h"
#include "MTH/MTH.h"

extern s32 inv_TXT_fn_lGetFreeScriptStringNumber(void);
extern int inv_strlen(u8* string);
extern void inv_strcpy(u8* dst, u8* src);
extern void inv_strcpy2(u8* dst, s8* src);
extern void inv_TXT_fn_vHideScriptStrings(void);
extern void inv_TXT_fn_vShowScriptStrings(void);
extern void JFFTXT_vModifyCaracter(int index, int offset, int value);
extern void AGO_vInit(void);
extern void inv_AGO_vDisplayGaugesAndIcons(void);
extern void AGO_vProcessAllParts(void);
extern void AGO_vDisplayAllParts(void);
extern void inv_TXT_fn_vDrawStringWithTransparency(MTH3D_tdstVector_* src, const char* text, f32 scale);
extern void AGO_vAddParticle(u32 PartType, MTH3D_tdstVector_* p_stPosition, MTH3D_tdstVector_* p_stDirection, void* hVisualMaterial, f32 parameter);
extern s32 inv_atoi(const u8* string);
extern void JFFTXT_vAddText(s32 index, MTH3D_tdstVector_* src, u8* string, f32 scale);
extern void JFFTXT_vExtendText(s32 index, s32 command, f32 argument);

extern f32 g_AGO_AnimTimeScale;

#endif
