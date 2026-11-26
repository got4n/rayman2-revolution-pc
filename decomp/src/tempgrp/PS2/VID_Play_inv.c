#include "common.h"

#include "GLI/GLI.h"

#include "renderware.h"
#include "rwps2.h"
#include "sdk.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "FMN/FMN.h"
#include "ps2/PS2.h"

extern u8 inv_MPEG_decoderWorkBuffer[];
extern u8 inv_MPEG_pictureBuffer[];
extern s32 inv_MPEG_bitstreamSize;
extern f32 inv_MPEG_pictureX;
extern volatile f32 inv_MPEG_pictureY;
extern s32 inv_MPEG_frameCounter;
extern s32 inv_MPEG_displayBuffer;
extern u8 inv_MPEG_bDecoderCreated;
extern u8 inv_MPEG_picturePackets[];
extern u8 inv_MPEG_retryIconOn[];
extern u8 inv_MPEG_retryIconOff[];
extern u8 inv_MPEG_retryIconPackets[];

void inv_VID_fn_vStartPacketTransfer(u8* packet);
void sceMpegCreate(s32* decoder, u8* work, s32 size);
void sceMpegAddBs(s32* decoder, void* buffer, s32 size);
s32 gu_sceMpegIsEnd(s32* decoder);
s32 sceMpegGetPicture(s32* decoder, u8* picture, u32 count);
void sceMpegReset(s32* decoder);
void sceGsPutDispEnv(u8* environment);

#ifdef PC_PORT

static void inv_VID_fn_vMoviePicture(void)
{
    if (pc_movie_picture(inv_MPEG_pictureBuffer, inv_MPEG_decoder[0], inv_MPEG_decoder[1])) {
        inv_MPEG_decoder[2]++;
    }
}

static void inv_VID_fn_vMovieOpen(void)
{
    u32 width;
    u32 height;

    pc_movie_open(inv_MPEG_bitstreamBuffer, inv_MPEG_bitstreamSize);
    if (pc_movie_size(&width, &height)) {
        inv_MPEG_decoder[0] = width;
        inv_MPEG_decoder[1] = height;
    }
}

s32 func_00215840(void)
{
    s32 result = 0;

    if (inv_MPEG_frameCounter & 1) {
        result = pc_movie_ended() ? -1 : 0;
        inv_VID_fn_vMoviePicture();
        if (pc_movie_ended()) {
            inv_VID_fn_vMovieOpen();
        }
    }
    inv_MPEG_frameCounter++;
    return result;
}

s32 inv_VID_fn_bPlayMovie(char* path)
{
    RwRaster* raster;

    inv_MPEG_pictureX = (g_lVIDEO_WantedWidth - 0x80) >> 1;
    inv_MPEG_pictureY = (f32) g_lVIDEO_WantedHeight - 148.0f;
    if (inv_VID_fn_vBuildMoviePath(path, &inv_MPEG_bitstreamBuffer, &inv_MPEG_bitstreamSize) < 0) {
        return -1;
    }

    raster = RwRasterCreate(0x200, 0x100, 0x20, 4);
    inv_MPEG_videoRaster = raster;
    inv_RW3_fn_vLoadVignetteRaster(raster, "loading");
    if (inv_MPEG_bDecoderCreated == 0) {
        inv_MPEG_bDecoderCreated = 1;
        inv_MPEG_decoder[0] = 0;
        inv_MPEG_decoder[1] = 0;
        inv_MPEG_decoder[2] = 0;
    }

    inv_VID_fn_vMovieOpen();
    inv_VID_fn_vMoviePicture();
    inv_VID_fn_vMovieOpen();
    inv_VID_fn_vDrawVideoFrame();
    inv_GLI_fn_vPrepareLevelFadeOut();
    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);

    inv_MPEG_displayBuffer = D_00393C10 == 0;
    memset(inv_MPEG_retryIconOn, 0, 0x400);
    memset(inv_MPEG_retryIconOn, 0xFF, 4);
    memset(inv_MPEG_retryIconOff, 0, 0x400);
    return 0;
}
#else
void func_00215668(void)
{
    if (inv_FMN_bCdReadRetrying != 0) {
        inv_VID_fn_vBuildImagePacket(inv_MPEG_retryIconPackets, inv_MPEG_retryIconOn, 0x32,
                      g_lVIDEO_WantedHeight - 0x42, 0x10, 0x10);
        inv_VID_fn_vBuildImagePacket(inv_MPEG_retryIconPackets + 0x100, inv_MPEG_retryIconOn, 0x32,
                      g_lVIDEO_WantedHeight - 0x42 + g_lVIDEO_WantedHeight, 0x10, 0x10);
    } else {
        inv_VID_fn_vBuildImagePacket(inv_MPEG_retryIconPackets, inv_MPEG_retryIconOff, 0x32,
                      g_lVIDEO_WantedHeight - 0x42, 0x10, 0x10);
        inv_VID_fn_vBuildImagePacket(inv_MPEG_retryIconPackets + 0x100, inv_MPEG_retryIconOff, 0x32,
                      g_lVIDEO_WantedHeight - 0x42 + g_lVIDEO_WantedHeight, 0x10, 0x10);
    }

    inv_VID_fn_vBuildImagePacket(inv_MPEG_picturePackets, inv_MPEG_pictureBuffer, inv_MPEG_pictureX,
                  inv_MPEG_pictureY, inv_MPEG_decoder[0], inv_MPEG_decoder[1]);
    inv_VID_fn_vBuildImagePacket(inv_MPEG_picturePackets + 0x1900, inv_MPEG_pictureBuffer, inv_MPEG_pictureX,
                  (s32) inv_MPEG_pictureY + g_lVIDEO_WantedHeight, inv_MPEG_decoder[0],
                  inv_MPEG_decoder[1]);

    sceGsPutDispEnv(inv_MPEG_gsDoubleBuffer + inv_MPEG_displayBuffer * 0x28);
    sceGsSyncPath(0, 0);
    inv_VID_fn_vStartPacketTransfer(inv_MPEG_retryIconPackets + inv_MPEG_displayBuffer * 0x100);
    sceGsSyncPath(0, 0);
    inv_VID_fn_vStartPacketTransfer(inv_MPEG_picturePackets + inv_MPEG_displayBuffer * 0x1900);
    inv_MPEG_displayBuffer = 1 - inv_MPEG_displayBuffer;
}

s32 func_00215840(void)
{
    s32 result = 0;

    if (inv_MPEG_frameCounter & 1) {
        result = sceMpegGetPicture(inv_MPEG_decoder, inv_MPEG_pictureBuffer, 0x40);
        if (gu_sceMpegIsEnd(inv_MPEG_decoder) != 0) {
            sceMpegReset(inv_MPEG_decoder);
            sceMpegAddBs(inv_MPEG_decoder, inv_MPEG_bitstreamBuffer, inv_MPEG_bitstreamSize);
        }
    }
    inv_MPEG_frameCounter++;
    return result;
}

s32 inv_VID_fn_bPlayMovie(char* path)
{
    RwRaster* raster;

    inv_MPEG_pictureX = (g_lVIDEO_WantedWidth - 0x80) >> 1;
    inv_MPEG_pictureY = (f32) g_lVIDEO_WantedHeight - 148.0f;
    if (inv_VID_fn_vBuildMoviePath(path, &inv_MPEG_bitstreamBuffer, &inv_MPEG_bitstreamSize) < 0) {
        return -1;
    }

    raster = RwRasterCreate(0x200, 0x100, 0x20, 4);
    inv_MPEG_videoRaster = raster;
    inv_RW3_fn_vLoadVignetteRaster(raster, "loading");
    if (inv_MPEG_bDecoderCreated == 0) {
        inv_MPEG_bDecoderCreated = 1;
        sceMpegCreate(inv_MPEG_decoder, inv_MPEG_decoderWorkBuffer, 0x12800);
    }

    sceMpegAddBs(inv_MPEG_decoder, inv_MPEG_bitstreamBuffer, inv_MPEG_bitstreamSize);
    sceMpegGetPicture(inv_MPEG_decoder, inv_MPEG_pictureBuffer, 0x40);
    sceMpegReset(inv_MPEG_decoder);
    sceMpegAddBs(inv_MPEG_decoder, inv_MPEG_bitstreamBuffer, inv_MPEG_bitstreamSize);
    inv_VID_fn_vDrawVideoFrame();
    inv_GLI_fn_vPrepareLevelFadeOut();
    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);

    inv_MPEG_displayBuffer = D_00393C10 == 0;
    memset(inv_MPEG_retryIconOn, 0, 0x400);
    memset(inv_MPEG_retryIconOn, 0xFF, 4);
    memset(inv_MPEG_retryIconOff, 0, 0x400);
    return 0;
}
#endif
