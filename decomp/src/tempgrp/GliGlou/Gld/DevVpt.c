#include "common.h"

#include "GLI/GLD.h"
#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

#define GLD_MAX_DEVICES 16
#define GLD_MAX_VIEWPORTS 16
#define GLD_FREED_FILL 0xCC

typedef struct PackedDeviceAttributes {
    u64 words[3];
    u32 last;
} __attribute__((packed, aligned(4))) PackedDeviceAttributes;

typedef struct GLD_stViewport {
    GLD_tdstViewportAttributes_ stViewAttrib;
    s32 bIsLocked;
} GLD_stViewport;

typedef struct GLD_stDevice {
    GLD_tdstDeviceAttributes_ stDevAttrib;
    GLD_stViewport* pstViewports[GLD_MAX_VIEWPORTS];
    s32 bIsLocked;
    u32 dwStyleBeforeSwap;
    u32 dwExStyleBeforeSwap;
    s32 rcWindowBeforeSwap[4];
    u32 dwBackgroundColor;
} GLD_stDevice;

extern GLD_stDevice* g_pstGLDDevices[GLD_MAX_DEVICES];
extern s32 D_0039335C;
extern s32 GLI_gs_lBackGroundColor;

void* RwCameraEndUpdate(void* camera);
s32 GLD_bClearDeviceWithChoice(s32 unused, s16 flags);
s32 GLD_bDeleteDevice(s16 index);
s32 GLD_bReleaseViewportAndDevice(s16 device, s16 viewport);
void fn_vUpdateViewportSizesInPixels(GLD_stDevice* device, GLD_stViewport* viewport);

static inline u32 inv_GLD_fn_ulDivideUnsigned(u32 value, u32 divisor)
{
    return value / divisor;
}

static inline void inv_GLD_fn_vCopyDeviceAttributes(void* destination, const void* source)
{
    PackedDeviceAttributes* to = destination;
    const PackedDeviceAttributes* from = source;

    *to = *from;
}

void func_00116AC0(void)
{
    GLD_bReleaseViewportAndDevice(0, 0);
}

s32 inv_GLD_fn_bInitDeviceTable(void)
{
    memset(g_pstGLDDevices, 0, sizeof(g_pstGLDDevices));
    return 1;
}

void fn_vUpdateViewportSizesInPixels(GLD_stDevice* p_stDev, GLD_stViewport* p_stVP)
{
    GLDCameraAttributesPrefix* values = &p_stVP->stViewAttrib.values;
    u32 percent0;
    u32 percent1;
    u32 perMille0;
    u32 perMille1;
    u32 perMille2;
    u32 perMille3;
    s32 outside;
    s32 originX;
    s32 originY;
    s32 right;
    s32 bottom;
    s32 height;
    s32 width;
    s32 visibleRight;
    s32 visibleBottom;
    s32 clipTop;
    s32 clipBottom;
    s32 clipRight;
    s32 clipLeft;

    percent0 = 100;
    right = inv_GLD_fn_ulDivideUnsigned(values->dwWidthInPercent * (p_stDev->stDevAttrib.dwWidth - 1), percent0);
    percent1 = 100;
    bottom = inv_GLD_fn_ulDivideUnsigned(values->dwHeightInPercent * (p_stDev->stDevAttrib.dwHeight - 1), percent1);
    values->dwTopInPixForClip = 0;
    values->dwLeftInPixForClip = 0;
    originX = values->iOffsetPosX;
    originY = values->iOffsetPosY;
    width = right + 1;
    height = bottom + 1;
    values->dwWidth = width;
    values->dwHeight = height;
    values->dwBottomInPixForClip = bottom;
    values->dwRightInPixForClip = right;
    visibleRight = right + originX;
    visibleBottom = bottom + originY;
    if (visibleRight < 0 || p_stDev->stDevAttrib.dwWidth - 1 < originX) {
        values->dwRightInPixForClip = 0;
        values->dwLeftInPixForClip = 0;
    } else if (visibleBottom < 0 || p_stDev->stDevAttrib.dwHeight - 1 < originY) {
        values->dwBottomInPixForClip = 0;
        values->dwTopInPixForClip = 0;
    } else {
        if (originX < 0) {
            values->dwRightInPixForClip = visibleRight;
            values->dwLeftInPixForClip = 0;
        } else if (p_stDev->stDevAttrib.dwWidth - 1 < visibleRight) {
            values->dwLeftInPixForClip = originX;
            values->dwRightInPixForClip = p_stDev->stDevAttrib.dwWidth - 1;
        } else {
            values->dwRightInPixForClip = visibleRight;
            values->dwLeftInPixForClip = originX;
        }
        if (originY < 0) {
            values->dwBottomInPixForClip = visibleBottom;
            values->dwTopInPixForClip = 0;
        } else if (p_stDev->stDevAttrib.dwHeight - 1 < visibleBottom) {
            values->dwTopInPixForClip = originY;
            values->dwBottomInPixForClip = p_stDev->stDevAttrib.dwHeight - 1;
        } else {
            values->dwBottomInPixForClip = visibleBottom;
            values->dwTopInPixForClip = originY;
        }
    }
    perMille0 = 1000;
    perMille1 = 1000;
    perMille2 = 1000;
    perMille3 = 1000;
    {
        s32 deviceWidth;
        s32 deviceHeight;

        outside = -1;
        clipTop = inv_GLD_fn_ulDivideUnsigned(height * values->dwClipTopInPerMille, perMille0) + originY;
        clipBottom = inv_GLD_fn_ulDivideUnsigned(height * values->dwClipBottomInPerMille, perMille1) + originY;
        clipLeft = inv_GLD_fn_ulDivideUnsigned(width * values->dwClipLeftInPerMille, perMille3) + originX;
        clipRight = inv_GLD_fn_ulDivideUnsigned(width * values->dwClipRightInPerMille, perMille2) + originX;
        deviceWidth = p_stDev->stDevAttrib.dwWidth;
        deviceHeight = p_stDev->stDevAttrib.dwHeight;
        values->dwRightInPix = values->dwWidth;
        p_stVP->bIsLocked = 0;
        values->dwTopInPix = 0;
        values->dwLeftInPix = 0;
        values->dwBottomInPix = values->dwHeight;
        if (clipTop <= outside) {
            clipTop = 0;
        }
        values->dwClipTopInPix = clipTop;
        if (clipBottom > deviceHeight) {
            clipBottom = deviceHeight;
        }
        values->dwClipBottomInPix = clipBottom;
        if (clipRight > deviceWidth) {
            clipRight = deviceWidth;
        }
        values->dwClipRightInPix = clipRight;
        if (clipLeft <= outside) {
            clipLeft = 0;
        }
        values->dwClipLeftInPix = clipLeft;
    }
    inv_GLI_RW_vReset2DVertexBuffer();
    if (values->dwClipTopInPix != 0) {
        inv_GLI_RW_vAdd2DRectangle(0, 0, values->dwWidth, values->dwClipTopInPix);
    }
    if (values->dwClipBottomInPix < values->dwHeight) {
        inv_GLI_RW_vAdd2DRectangle(0, values->dwClipBottomInPix, values->dwWidth, values->dwHeight);
    }
    if (values->dwClipLeftInPix != 0) {
        inv_GLI_RW_vAdd2DRectangle(0, 0, values->dwClipLeftInPix, values->dwHeight);
    }
    if (values->dwClipRightInPix < values->dwWidth) {
        inv_GLI_RW_vAdd2DRectangle(values->dwClipRightInPix, 0, values->dwWidth, values->dwHeight);
    }
}

s32 func_00116D88(s16* index)
{
    s16 i;

    for (i = 0; i < GLD_MAX_DEVICES; i++) {
        if (g_pstGLDDevices[i] == NULL) {
            *index = i;
            return 1;
        }
    }
    *index = -1;
    return 0;
}

GLD_stDevice* GLD_pGetDevice(s16 index);

s32 func_00116DE0(s16 device, s16* index)
{
    GLD_stViewport** viewports = GLD_pGetDevice(device)->pstViewports;
    s16 i;

    for (i = 0; i < GLD_MAX_VIEWPORTS; i++) {
        if (viewports[i] == NULL) {
            *index = i;
            return 1;
        }
    }
    *index = -1;
    return 0;
}

GLD_stViewport* GLD_pGetViewport(s16 device, s16 viewport)
{
    return g_pstGLDDevices[device]->pstViewports[viewport];
}

GLD_stDevice* GLD_pGetDevice(s16 index)
{
    return g_pstGLDDevices[index];
}

s32 GLD_bClearDevice(s16 device)
{
    return GLD_bClearDeviceWithChoice(device, 3);
}

s32 GLD_bClearDeviceWithChoice(s32 unused, s16 flags)
{
    inv_GLI_RW_vClearCamera(flags & 2, flags & 1, 0);
    return 1;
}

s32 GLD_bClearViewportWithChoice(s32 hDev, s32 hVP, s16 sChoice)
{
    inv_GLI_RW_vClearCamera(sChoice & 2, sChoice & 1, GLI_gs_lBackGroundColor);
    return 1;
}

s32 GLD_bCreateDevice(GLD_tdstDeviceAttributes_* pstDevAttrib, s16* phDev)
{
    GLD_stDevice* device;

    if (D_0039335C != 0) {
        if (inv_GLD_fn_bInitDeviceTable() == 0) {
            return 0;
        }
        D_0039335C = 0;
    }
    device = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(GLD_stDevice));
    memset(device, 0, sizeof(GLD_stDevice));
    pstDevAttrib->dwWidth = g_lVIDEO_WantedWidth;
    pstDevAttrib->dwHeight = g_lVIDEO_WantedHeight;
    func_00116D88(phDev);
    g_pstGLDDevices[*phDev] = device;
    pstDevAttrib->xPixelDimensionX = 0.0003125f;
    pstDevAttrib->xPixelDimensionY = 0.0003125f;
    pstDevAttrib->lPitch = -1;
    pstDevAttrib->p_cVirtualScreen = 0;
    inv_GLD_fn_vCopyDeviceAttributes(&device->stDevAttrib, pstDevAttrib);
    device->bIsLocked = 0;
    device->dwBackgroundColor = 0;
    return 1;
}

s32 GLD_bGetDeviceAttributes(s16 hDev, GLD_tdstDeviceAttributes_* pstDevAttrib)
{
    GLD_stDevice* pstDev = GLD_pGetDevice(hDev);

    if (pstDev == NULL) {
        return 0;
    }
    inv_GLD_fn_vCopyDeviceAttributes(pstDevAttrib, &pstDev->stDevAttrib);
    return 1;
}

s32 GLD_bCreateViewport(s16 hDev, GLD_tdstViewportAttributes_* pstViewAttrib, s16* phVP)
{
    GLD_stDevice* device = GLD_pGetDevice(hDev);
    GLD_stViewport* viewport;
    s16 slot;

    if (func_00116DE0(hDev, &slot) != 0) {
        viewport = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(GLD_stViewport));
        if (viewport == NULL) {
            *phVP = -1;
            memset(pstViewAttrib, GLD_FREED_FILL, sizeof(GLD_tdstViewportAttributes_));
            return 0;
        }
    } else {
        *phVP = -1;
        memset(pstViewAttrib, GLD_FREED_FILL, sizeof(GLD_tdstViewportAttributes_));
        return 0;
    }
    memset(viewport, 0, sizeof(GLD_stViewport));
    memcpy(viewport, pstViewAttrib, sizeof(GLD_tdstViewportAttributes_));
    fn_vUpdateViewportSizesInPixels(device, viewport);
    viewport->stViewAttrib.values.dwInitialHeight = viewport->stViewAttrib.values.dwHeight;
    viewport->stViewAttrib.values.dwInitialWidth = viewport->stViewAttrib.values.dwWidth;
    viewport->stViewAttrib.lPitch = -1;
    viewport->stViewAttrib.p_vSpecificToXD = NULL;
    viewport->stViewAttrib.p_cVirtualScreen = 0;
    viewport->bIsLocked = 0;
    memcpy(pstViewAttrib, viewport, sizeof(GLD_tdstViewportAttributes_));
    g_pstGLDDevices[hDev]->pstViewports[slot] = viewport;
    *phVP = hDev * GLD_MAX_VIEWPORTS + slot;
    return 1;
}

s32 GLD_bGetViewportAttributes(s16 _unused, s16 hVP, GLD_tdstViewportAttributes_* pstViewAttrib)
{
    s32 index = hVP;
    s32 state;
    s32 device = index / GLD_MAX_VIEWPORTS;
    GLD_stViewport* viewport;

    state = -1;
    GLD_pGetDevice(device);
    viewport = GLD_pGetViewport(device, index - (device << 4));
    viewport->stViewAttrib.lPitch = state;
    viewport->stViewAttrib.p_cVirtualScreen = 0;
    *pstViewAttrib = viewport->stViewAttrib;
    state++;
    return 1;
}

s32 GLD_bBeginWriteToDevice(s16 hDevice)
{
    return inv_GLI_RW_bRender2DVertexBuffer();
}

s32 GLD_bEndWriteToDevice(s16 device, void* attributes)
{
    return func_0011BC98();
}

s32 GLD_bRequestWriteToViewport(s32 _unused, s16 hVP, GLD_tdstViewportAttributes_* pstViewAttrib,
                                s32* pbCanWrite)
{
    GLD_stDevice* device = GLD_pGetDevice(hVP / GLD_MAX_VIEWPORTS);
    GLD_stViewport* viewport =
        GLD_pGetViewport(hVP / GLD_MAX_VIEWPORTS, hVP % GLD_MAX_VIEWPORTS);

    *pbCanWrite = 1;
    *pstViewAttrib = viewport->stViewAttrib;
    device->bIsLocked = 1;
    inv_RW3_fn_vSelectViewportCamera(pstViewAttrib);
    RwCameraBeginUpdate(inv_GLI_currentCamera);
    return 1;
}

s32 GLD_bWriteToViewportFinished(s32 _unused, s16 hVP)
{
    GLD_stDevice* device = GLD_pGetDevice(hVP / GLD_MAX_VIEWPORTS);
    void* camera = inv_GLI_currentCamera;

    device->bIsLocked = 0;
    RwCameraEndUpdate(camera);
    return 1;
}

s32 GLD_bFlipDeviceWithSynchro(s16 device, s32 value)
{
    inv_GLI_RW_vShowCameraRaster();
    return 1;
}

s32 GLD_bSetViewportPosition(s32 _unused, s16 hVP, GLD_tdstViewportAttributes_* pstViewAttrib)
{
    GLD_stDevice* device = GLD_pGetDevice(hVP / GLD_MAX_VIEWPORTS);
    GLD_stViewport* viewport =
        GLD_pGetViewport(hVP / GLD_MAX_VIEWPORTS, hVP % GLD_MAX_VIEWPORTS);

    memset(viewport, 0, sizeof(GLD_stViewport));
    memcpy(&viewport->stViewAttrib, pstViewAttrib, sizeof(GLD_tdstViewportAttributes_));
    fn_vUpdateViewportSizesInPixels(device, viewport);
    viewport->stViewAttrib.lPitch = -1;
    viewport->stViewAttrib.p_cVirtualScreen = 0;
    memcpy(pstViewAttrib, &viewport->stViewAttrib, sizeof(GLD_tdstViewportAttributes_));
    return 1;
}

s32 GLD_bDeleteViewport(s32 _unused, s16 hVP)
{
    s32 index = hVP / GLD_MAX_VIEWPORTS;
    GLD_stDevice* device = GLD_pGetDevice(index);
    s16 slot = hVP % GLD_MAX_VIEWPORTS;
    GLD_stViewport* viewport = GLD_pGetViewport(index, slot);

    memset(viewport, GLD_FREED_FILL, sizeof(GLD_stViewport));
    inv_MEM_fn_vFreeInHeap(gp_6F74, viewport);
    device->pstViewports[slot] = NULL;
    return 1;
}

s32 GLD_bDeleteDevice(s16 hDev)
{
    GLD_stDevice* device = GLD_pGetDevice(hDev);
    GLD_stViewport** viewports = device->pstViewports;
    s16 i;

    for (i = 0; i < GLD_MAX_VIEWPORTS; i++) {
        if (viewports[i] != NULL) {
            memset(viewports[i], GLD_FREED_FILL, sizeof(GLD_stViewport));
            viewports[i] = NULL;
        }
    }
    memset(device, GLD_FREED_FILL, sizeof(GLD_stDevice));
    inv_MEM_fn_vFreeInHeap(gp_6F74, device);
    g_pstGLDDevices[hDev] = NULL;
    return 1;
}

s32 GLD_bGiveSpecificAttributesToViewport(s32 hDev, s16 hVP, void* p_vSpecAttrib)
{
    GLD_pGetDevice(hVP / GLD_MAX_VIEWPORTS);
    GLD_pGetViewport(hVP / GLD_MAX_VIEWPORTS, hVP % GLD_MAX_VIEWPORTS)
        ->stViewAttrib.p_vSpecificToXD = p_vSpecAttrib;
    return 1;
}

s32 GLD_bReleaseViewportAndDevice(s16 device, s16 viewport)
{
    GLD_bDeleteViewport(device, viewport);
    GLD_bDeleteDevice(device);
    return 1;
}
