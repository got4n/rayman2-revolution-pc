#include "common.h"

#include "GLI/GLI.h"

#include "renderware.h"
#include "FMN/FMN.h"
#include "TMR/TMR.h"

void* memset(void* s, s32 c, s32 n);
char* strcpy(char* dst, const char* src);
char* strupr(char* s);
void* RwRasterPushContext(RwRaster* raster);
void RwRasterRenderScaled(RwRaster* raster, ScreenRect* rect);
void RwRasterPopContext(void);

void inv_VIG_fn_vDisplayStartupImage(const char* name, s32 width, s32 height, u8 flip)
{
    RwRGBA clearColor;
    char path[0x40];
    ScreenRect rect;
    RwRaster* raster;

    memset(&clearColor, 0, sizeof(clearColor));
    clearColor.alpha = 0xFF;
    strcpy(path, name);
    strupr(path);
    FMN_ConvertPathUnix2Dos(path);
    raster = RwRasterCreate(0x200, 0x100, 0x20, 0x504);
    inv_RW3_fn_vLoadVignetteRaster(raster, path);
    if (flip == 1)
        RwCameraClear(inv_GLI_currentCamera, &clearColor, 3);
    if (RwRasterPushContext(RwCameraGetRaster(inv_GLI_currentCamera)) != NULL) {
        rect.x = 0;
        rect.y = 0;
        rect.width = width;
        rect.height = height;
        RwRasterRenderScaled(raster, &rect);
        RwRasterPopContext();
    }
    if (flip == 1)
        RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);
    fn_vTimerWait(50);
    RwRasterDestroy(raster);
}

void func_00153550(void)
{
    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);
}
