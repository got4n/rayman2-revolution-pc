#ifndef REVO_GLI_GLD_H
#define REVO_GLI_GLD_H

#include "ps2/types.h"

typedef struct GLDCameraAttributesPrefix {
    s32 dwInitialHeight;
    s32 dwInitialWidth;
    u32
        dwHeight;
    u32
        dwWidth;
    s32 dwTopInPix;
    s32 dwBottomInPix;
    s32 dwLeftInPix;
    s32 dwRightInPix;
    s32 dwTopInPixForClip;
    s32 dwBottomInPixForClip;
    s32 dwLeftInPixForClip;
    s32 dwRightInPixForClip;
    s32 dwWidthInPercent;
    s32 dwHeightInPercent;
    u32 dwClipTopInPix;
    u32 dwClipBottomInPix;
    u32 dwClipLeftInPix;
    u32 dwClipRightInPix;
    s32 dwClipTopInPerMille;
    s32 dwClipBottomInPerMille;
    s32 dwClipLeftInPerMille;
    s32 dwClipRightInPerMille;
    s32 iOffsetPosX;
    s32 iOffsetPosY;
} GLDCameraAttributesPrefix;

typedef struct GLD_tdstViewportAttributes_ {
    GLDCameraAttributesPrefix values;
    s32 p_cVirtualScreen;
    s32 lPitch;
    void* p_vSpecificToXD;
    s16 hDevice;
    s16 hViewport;
} GLD_tdstViewportAttributes_;

typedef struct GLI_tdstSpecificAttributesFor3D_ {
    struct GLI_tdstCamera_* p_stCam;
    f32 xNear;
} GLI_tdstSpecificAttributesFor3D_;

#endif
