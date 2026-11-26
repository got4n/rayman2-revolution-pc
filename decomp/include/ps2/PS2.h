#ifndef REVO_PS2_PS2_H
#define REVO_PS2_PS2_H

#include "ps2/types.h"
#include "FMN/FMN.h"
#include "GLI/GLD.h"
#include "IPT/IPT.h"
#include "MTH/MTH.h"
#include "renderware_fwd.h"

extern u32 inv_PS2_fn_bIsVu0Busy(void);
extern s32 inv_PS2_fn_lIsVif0DmaBusy(void);
extern void inv_PS2_fn_vInitVif0Dma(void);
extern void inv_PS2_fn_vCallVu0Microprogram(u32 address);

extern void inv_VID_fn_vStartMovie(u8 priority);
extern void inv_VID_fn_vStopMovie(u8 priority);

extern void inv_SNA_fn_vRestoreSavedFixups(s32 slot);
extern s32 inv_VID_fn_vBuildMoviePath(char* filename, void** out, s32* size);
extern u32 INO_fn_ulJoyPreviousButtons(stDevice* _hDev);
extern u16 INO_fn_uwJoyPreviousDirections(stDevice* _hDev);
extern void inv_GLI_fn_vGetLightDirectionAt(MTH3D_tdstVector_* position, MTH3D_tdstVector_* direction, f32* intensity);
extern void func_00214F98(GLD_tdstViewportAttributes_* viewport, void* geometry, void* bounds, s32 drawMask);
extern void GLI_lSendVisualSetToViewport2(GLD_tdstViewportAttributes_* viewport, void* geometry, s32 drawMask);

extern s32 inv_SVG_fn_lEnumerateSaveDirectory(void* port, void* slot, void* name);

struct HIE_tdstSuperObject_;
extern void inv_VID_fn_vFreeMpegVideo(void);
extern void inv_VID_fn_vDrawVideoFrame(void);
extern void inv_VID_fn_vRestoreDisplay(void);
extern void inv_VID_fn_vClearScreen(RwCamera* camera);
extern Bigfile* inv_FMN_fn_bMountBigfile(void* path, void* name);
extern void inv_VID_fn_vBuildImagePacket(u8* destination, u8* source, s32 x, s32 y, s32 width, s32 height);
extern void inv_VID_fn_vAddStaticLightsFromRoot(struct HIE_tdstSuperObject_* node);

extern void func_00215668(void);
extern s32 func_00215840(void);
extern s32 inv_VID_fn_bPlayMovie(char* path);

extern volatile s32 inv_PS2_vblankCount;

extern s32 inv_MPEG_decoder[];

extern u8 D_0039378C;

extern RwRaster* inv_MPEG_videoRaster;
extern void* inv_MPEG_bitstreamBuffer;
extern u8 inv_MPEG_gsDoubleBuffer[];

#endif
