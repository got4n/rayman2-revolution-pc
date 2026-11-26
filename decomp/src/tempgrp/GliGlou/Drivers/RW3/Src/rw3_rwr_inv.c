#include "common.h"

#include "ps2/libc.h"

#include "GLI/GLI.h"

#include "renderware.h"
#include "FMN/FMN.h"
#include "clib/clib.h"

void* memset(void* s, s32 c, s32 n);
char* strcpy(char* dst, const char* src);
char* strupr(char* s);
s32 sprintf(char* dst, const char* fmt, ...);

s32 FMN_Read(u32 file, void* data, s32 count, s32 size);

void* RwRasterPushContext(RwRaster* raster);
void RwRasterRenderScaled(RwRaster* raster, ScreenRect* rect);
void RwRasterPopContext(void);
s32 RwRasterGetDepth(RwRaster* raster);
s32 RwRasterGetWidth(RwRaster* raster);
s32 RwRasterGetStride(RwRaster* raster);

void inv_RW3_fn_vDisplayBmpFile(char* name, s32 width, s32 height)
{
    RwRGBA background;
    char path[0x40];
    ScreenRect rect;
    char* extension;
    RwTexture* texture;
    RwRaster* raster;

    memset(&background, 0, 4);
    background.alpha = 0xFF;
    strcpy(path, name);
    strupr(path);
    FMN_ConvertPathUnix2Dos(path);
    extension = inv_My_strchr(path, '.');
    if (extension != NULL) {
        *extension = '\0';
    }
    texture = RwTextureRead(path, NULL);
    if (texture == NULL) {
        printf("ERROR --> Cannot open BMP file : %s\n", path);
        return;
    }
    RwCameraClear(inv_GLI_currentCamera, &background, 3);
    if (RwRasterPushContext(RwCameraGetRaster(inv_GLI_currentCamera)) != NULL) {
        raster = texture->raster;
        rect.x = 0;
        rect.y = 0;
        rect.width = width;
        rect.height = height;
        RwRasterRenderScaled(raster, &rect);
        RwRasterPopContext();
    }
    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);
    RwTextureDestroy(texture);
}

void inv_RW3_fn_vLoadVignetteRaster(RwRaster* raster, char* name)
{
    char path[0x80];
    u8* pixels;
    u32 file;
    s32 height;
    s32 stride;

    pixels = RwRasterLock(raster, 0, 1);
    RwRasterGetDepth(raster);
    height = RwRasterGetHeight(raster);
    RwRasterGetWidth(raster);
    stride = RwRasterGetStride(raster);
    sprintf(path, "vignette/%s.rwr", name);
    file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
    if (file >= 0xF) {
        file = inv_FMN_fn_ulOpenVirtualOrFallback("vignette/missing.rwr");
    }
    if (file < 0xF) {
        FMN_Read(file, pixels, 1, height * stride);
        FMN_Close(file);
    }
    RwRasterUnlock(raster);
}
