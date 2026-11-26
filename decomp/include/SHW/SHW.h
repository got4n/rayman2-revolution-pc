#ifndef REVO_SHW_SHW_H
#define REVO_SHW_SHW_H

#include "ps2/types.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

struct ShadowProjectionMatrixView;
extern void SHW_fn_vInitShadowCounter(void);
extern void inv_SHW_fn_vInitProjectionBuffers(void);
extern void inv_SHW_fn_vResetProjectionGlobals(void);
extern void inv_SHW_fn_vRenderQueuedProjectedShadows(void* attributes, u32 ulDisplayMode);
extern void inv_SHW_fn_vBuildProjectionMatrix(struct ShadowProjectionMatrixView* out, void* pos, MTH3D_tdstVector_* dir, f32 scaleZ, f32 scaleX, f32 scaleY);
extern s32 inv_SHW_fn_bQueueProjectedShadow(void* sectorObject, void* superObject, void* source, GliMaterialData* material, f32 weight, f32 start, f32 end, tdstFogParams_* fogParameters);

extern void func_001479A0(void);
extern void inv_SHW_fn_vRenderShadowQueue(void);
extern void inv_SHW_fn_vInitShadowQueue(void);
extern f32 inv_SHW_fn_xGetPlaneZAt(MTH3D_tdstVector_* point, MTH3D_tdstVector_* normal, f32 inverseNormalZ, f32 x, f32 y);
extern f32 inv_fn_xGetSuperObjectTransparencyByte(HIE_tdstSuperObject_* superObject);
extern void inv_SHW_fn_vQueueCharacterShadow(struct GLD_tdstViewportAttributes_* viewport, HIE_tdstSuperObject_* superObject, HIE_tdstSuperObject_* sectorObject, s32 drawMask);

extern void inv_SHW_fn_vResetShadowQueue(void);

#endif
