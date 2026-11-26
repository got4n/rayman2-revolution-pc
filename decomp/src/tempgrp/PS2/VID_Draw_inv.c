#include "rwps2_macros.h"
#include "include_asm.h"
#include "common.h"
#include "renderware.h"
#include "ps2/libc.h"
#include "HIE/HIE.h"
#include "SCT/SCT.h"
#include "GLI/GLI.h"
#include "FMN/FMN.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "ps2/PS2.h"

#define DISPLAY_CHANNEL_MAX 255.0f
#define TRANSFER_WORD_MASK 0x0FFFFFFFU
#define WIDE(value)                                                                                \
    ({                                                                                             \
        u64 wide = (value);                                                                        \
        wide;                                                                                      \
    })

typedef struct TransferWords {
    u32 word0;
    u32 word1;
    u32 word2;
    u32 word3;
} TransferWords;

extern u8 inv_VID_a_wQuadIndices[];
extern void* D_00393C58;
extern u64 D_00393C20;
extern u64 D_00393C30;
extern u64 D_00393C40;
extern u64 D_00393C48;
extern u32 D_00393CA8;
extern RwVideoMode* inv_RW_currentVideoMode;

f32 RwCameraGetNearClipPlane(void* camera);
void RwIm2DRenderIndexedPrimitive(s32 type, RwSky2DVertexFields* vertices, s32 vertexCount,
                                  void* indices, s32 indexCount);
void freemem(void* block);
void inv_VID_fn_vBuildTilePacket(u32 destination, u32 source, s32 step, s32 x, s32 y, s32 width, s32 height);
void inv_VID_fn_vSetDmaTag(u64* out, u32 bit63, u32 high, u32 bit31, u32 bit28, u32 bit26, u32 low);
void inv_VID_fn_vSetGifTag(TransferWords* out, u64 value, s32 bit28, s32 bit26, s32 bit15, s32 bit14,
                   s32 high15, s32 low);
void inv_VID_fn_vSetRegister50(TransferWords* out, u32 low, u32 bit48, u32 bit56);
void inv_VID_fn_vSetRegister51(TransferWords* out, u32 bit59, u32 low, u32 bit48);
void inv_VID_fn_vSetRegister52(TransferWords* out, u32 low, u32 high);
void inv_VID_fn_vSetRegister53(TransferWords* out, u32 low);
void inv_VID_fn_vAddNodeStaticLights(HIE_tdstSuperObject_* node);
void inv_RW3_fn_vSendDisplayChangePacket(void);
void sceGsResetGraph(s32 mode, s32 interlace, s32 videoMode, s32 field);
void sceGsSwapDBuffDc(void* doubleBuffer, void* field);
s32 sceGsSyncV(s32 mode);
void sceMpegReset(void* decoder);
void skyTexCacheFlush(void);
void sweOpenLocalPkt(s32 type, s32 size);

void inv_VID_fn_vFreeMpegVideo(void)
{
#ifdef PC_PORT
    pc_movie_close();
#else
    sceMpegReset(inv_MPEG_decoder);
#endif
    if (inv_MPEG_bitstreamBuffer != NULL) {
        freemem(inv_MPEG_bitstreamBuffer);
    }
    if (inv_MPEG_videoRaster != NULL) {
        RwRasterDestroy(inv_MPEG_videoRaster);
        inv_MPEG_videoRaster = NULL;
    }
}

void inv_VID_fn_vDrawVideoFrame(void)
{
    RwRGBA clear;
    RwSky2DVertexFields vertices[4];
    RwSky2DVertexFields vertex;
    f32 nearZ;
    f32 recipZ;

    clear.green = 0;
    clear.red = 0;
    clear.blue = 0;
    clear.alpha = 0;
    RwCameraClear(inv_GLI_currentCamera, &clear, 3);

    if (RwCameraBeginUpdate(inv_GLI_currentCamera) != NULL) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, NULL);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*) rwSHADEMODEGOURAUD);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*) rwFILTERNEAREST);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, NULL);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, NULL);
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*) rwTEXTUREADDRESSWRAP);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDZERO);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, NULL);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*) rwBLENDONE);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*) rwBLENDZERO);

        nearZ = RwIm2DGetNearScreenZ();
        recipZ = 1.0f / RwCameraGetNearClipPlane(inv_GLI_currentCamera);

        vertex.scrVertex.x = 0;
        vertex.scrVertex.y = 0;
        vertex.scrVertex.z = nearZ;
        vertex.camVertex_z = 0;
        vertex.u = 0.0f;
        vertex.v = 0.0f;
        vertex.recipZ = recipZ;
        vertex.pad1 = 0;
        vertex.color.red = DISPLAY_CHANNEL_MAX;
        vertex.color.green = DISPLAY_CHANNEL_MAX;
        vertex.color.blue = DISPLAY_CHANNEL_MAX;
        vertex.color.alpha = DISPLAY_CHANNEL_MAX;
        vertex.objNormal.x = 0;
        vertex.objNormal.y = 0;
        vertex.objNormal.z = 0;
        vertex.pad2 = 0;

        vertices[0] = vertex;
        vertex.u = 1.0f;
        vertex.v = 0.0f;
        vertices[1] = vertex;
        vertex.v = 1.0f;
        vertices[2] = vertex;
        vertex.u = 0.0f;
        vertices[3] = vertex;

        vertices[0].scrVertex.x = 0.0f;
        vertices[0].scrVertex.y = 0.0f;
        vertices[1].scrVertex.x = g_lVIDEO_WantedWidth;
        vertices[1].scrVertex.y = 0.0f;
        vertices[2].scrVertex.x = g_lVIDEO_WantedWidth;
        vertices[2].scrVertex.y = g_lVIDEO_WantedHeight;
        vertices[3].scrVertex.x = 0.0f;
        vertices[3].scrVertex.y = g_lVIDEO_WantedHeight;

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, inv_MPEG_videoRaster);
        RwIm2DRenderIndexedPrimitive(3, vertices, 4, inv_VID_a_wQuadIndices, 6);
        RwCameraEndUpdate(inv_GLI_currentCamera);
    }

    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);
}

#ifndef PC_PORT
void inv_VID_fn_vBuildImagePacket(u8* destination, u8* source, s32 x, s32 y, s32 width, s32 height)
{
    inv_VID_fn_vBuildTilePacket((u32) destination, (u32) source, 0x400, x, y, width, height);
}

void inv_VID_fn_vBuildRepeatedTilePacket(s32 destination, s32 source, s32 x, s32 y, s32 width, s32 height)
{
    inv_VID_fn_vBuildTilePacket(destination, source, 0, x, y, width, height);
}

void inv_VID_fn_vBuildTilePacket(u32 destination, u32 source, s32 step, s32 x, s32 y, s32 width, s32 height)
{
    u8* packet;
    u8* tile;
    s32 bufferWidth;
    s32 rows;
    s32 cols;
    s32 i;
    s32 j;
    s32 eop;

    bufferWidth = (g_lVIDEO_WantedWidth + 63) >> 6;
    rows = width >> 4;
    cols = height >> 4;
    packet = (u8*) ((destination & 0x0FFFFFFF) | 0x20000000);
    inv_VID_fn_vSetDmaTag((u64*) packet, 0, 0, 0, 1, 0, 3);
    packet += 0x10;
    inv_VID_fn_vSetGifTag((TransferWords*) packet, 14, 1, 0, 0, 0, 0, 2);
    packet += 0x10;
    inv_VID_fn_vSetRegister50((TransferWords*) packet, 0, bufferWidth, 0);
    packet += 0x10;
    inv_VID_fn_vSetRegister52((TransferWords*) packet, 16, 16);
    packet += 0x10;
    i = 0;
    if (i < rows) {
        tile = packet + 0x10;
        do {
            for (j = 0; j < cols;) {
                eop = 0;
                if (j == cols - 1) {
                    eop = (i == rows - 1);
                }
                inv_VID_fn_vSetDmaTag((u64*) packet, 0, 0, 0, 1, 0, 4);
                packet = tile + 0x50;
                inv_VID_fn_vSetGifTag((TransferWords*) tile, 14, 1, 0, 0, 0, 0, 2);
                inv_VID_fn_vSetRegister51((TransferWords*) (tile + 0x10), 0, x + i * 16, y + j * 16);
                j++;
                inv_VID_fn_vSetRegister53((TransferWords*) (tile + 0x20), 0);
                inv_VID_fn_vSetGifTag((TransferWords*) (tile + 0x30), 0, 0, 2, 0, 0, eop, 0x40);
                inv_VID_fn_vSetDmaTag((u64*) (tile + 0x40), 0, source & 0x0FFFFFFF, 0, 3, 0, 0x40);
                source += step;
            next:
                tile += 0x60;
            }
            i++;
        } while (i < rows);
    }
    inv_VID_fn_vSetDmaTag((u64*) packet, 0, 0, 0, 7, 0, 0);
    packet += 0x10;
    if ((u8) (u32) packet & 0x100) {
        g_lVIDEO_WantedWidth = (u32) &&next;
    }
}
#endif

void inv_VID_fn_vSetDmaTag(u64* out, u32 bit63, u32 high, u32 bit31, u32 bit28, u32 bit26, u32 low)
{
    u64 group0 = WIDE(bit63) << 63 | WIDE(low);
    u64 group1 = WIDE(bit28) << 28 | WIDE(bit31) << 31;
    u64 group2 = WIDE(high & ~0xF) << 32 | WIDE(bit26) << 26;

    *out = (group0 | group1) | group2;
}

void inv_VID_fn_vSetGifTag(TransferWords* out, u64 value, s32 bit28, s32 bit26, s32 bit15, s32 bit14,
                   s32 high15, s32 low)
{
    u32 group0 = bit14 << 14 | bit28 << 28;
    u32 group1 = bit26 << 26 | bit15 << 15;
    u32 word1 = group0 | group1;
    u32 word0 = high15 << 15 | low;

    out->word3 = value >> 32;
    out->word1 = word1;
    out->word2 = value & 0xFFFFFFFF;
    out->word0 = word0;
}

void inv_VID_fn_vSetRegisterData(TransferWords* out, s32 tag, u64 value)
{
    out->word2 = tag;
    out->word3 = 0;
    out->word1 = value >> 32;
    out->word0 = value & 0xFFFFFFFF;
}

void inv_VID_fn_vSetRegister50(TransferWords* out, u32 low, u32 bit48, u32 bit56)
{
    u64 field56 = bit56;
    u64 field48 = bit48;
    u64 field32 = low;

    inv_VID_fn_vSetRegisterData(out, 0x50, (field56 << 56 | field48 << 48) | field32 << 32);
}

void inv_VID_fn_vSetRegister51(TransferWords* out, u32 bit59, u32 low, u32 bit48)
{
    u64 field59 = bit59;
    u64 field48 = bit48;
    u64 field32 = low;

    inv_VID_fn_vSetRegisterData(out, 0x51, (field59 << 59 | field48 << 48) | field32 << 32);
}

void inv_VID_fn_vSetRegister52(TransferWords* out, u32 low, u32 high)
{
    u64 upper = high;

    inv_VID_fn_vSetRegisterData(out, 0x52, upper << 32 | low);
}

void inv_VID_fn_vSetRegister53(TransferWords* out, u32 low)
{
    inv_VID_fn_vSetRegisterData(out, 0x53, low);
}

#ifndef PC_PORT
void inv_VID_fn_vStartPacketTransfer(u32 address)
{
    *(volatile u32*) 0x1000A030 = address & TRANSFER_WORD_MASK;
    *(volatile u32*) 0x1000A020 = 0;
    *(volatile u32*) 0x1000A000 = 0x105;
}
#endif

void inv_VID_fn_vNullLightNode(HIE_tdstSuperObject_* node)
{
}

void inv_VID_fn_vAddNodeStaticLights(HIE_tdstSuperObject_* node)
{
    HIE_tdstSuperObject_* child;

    HIE_fn_vPushSuperObjectMatrix(node);
    if (node->lTypeOfLinkedObject == HIE_TYPE_SECTOR_inv) {
        SECT_tdstSector_* sector;
        SECT_tdstListOfStaticLights_* light;
        s16 remaining;

        GLI_vClearListOfLightInViewport(0);
        sector = node->hLinkedObject;
        remaining = sector->stListOfStaticLights.lNumberOfElementsOpt;
        light = sector->stListOfStaticLights.hFirstElementOpt;
        while (remaining > 0) {
            GLI_tdstLight_* current = light->hStaticLights;

            if (GLI_ucGetObjectLighted(current) & 1) {
                GLI_vAddLightToViewport(0, current);
            }
            light++;
            remaining--;
        }
        for (child = node->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
            inv_VID_fn_vNullLightNode(child);
        }
    } else {
        for (child = node->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
            inv_VID_fn_vAddStaticLightsFromRoot(child);
        }
    }
    HIE_fn_vPopMatrix();
}

void inv_VID_fn_vAddStaticLightsFromRoot(HIE_tdstSuperObject_* node)
{
    HIE_fn_vPushMatrix();
    HIE_fn_vInvalidateAllGlobalMatrices();
    GLI_vResetTableOfActiveLightsForChar(1);
    inv_VID_fn_vAddNodeStaticLights(node);
    HIE_fn_vPopMatrix();
}

void inv_VID_fn_vClearScreen(RwCamera* camera)
{
    RwRGBA color;
    void* clear = &color;
    s32 count;

    memset(clear, 0, sizeof(color));
    color.alpha = 0xFF;
    count = 1;
    do {
        RwCameraClear(camera, &color, 1);
        RwCameraShowRaster(camera, NULL, 0);
    } while (--count >= 0);
    count = 5;
#ifdef PC_PORT
    if (pc_mod_skip_vblank_waits()) {
        return;
    }
#endif
    do {
#ifdef PC_PORT
        pc_frame_wait_vblank();
#else
        sceGsSyncV(0);
#endif
    } while (--count >= 0);
}

void inv_RW3_fn_vSendDisplayChangePacket(void)
{
    Qword* packet;
    u128 address;
    u128 gif;
    u128 tag;
    u128 prim;
    u128 nreg;
    u128 eop;
    u128 flush;
    u64 eight;
    u64 forty;
    u64 eight2;
    u64 forty2;
    u64 zero;
    u64 low;
    u64 high;

    high = 0x19;
    low = D_00393C20;
    RWPS2_MAKE_QWORD(address, high, low);
    eight = 8;
    forty = 0x44;
    D_00393C48 = eight;
    D_00393C40 = forty;
    D_00393CA8 = 0;
    D_00393C30 = 0;
    eight = 0xE;
    RWPS2_MAKE_QWORD(gif, eight, 0x1000000000008006ULL);
    RWPS2_MAKE_QWORD(tag, 0x42, forty);
    eight2 = 8;
    zero = 0;
    RWPS2_MAKE_QWORD(prim, eight2, zero);
    forty = 0x49;
    RWPS2_MAKE_QWORD(nreg, forty, zero);
    low = 0x8000000000ULL;
    high = 0x3B;
    RWPS2_MAKE_QWORD(eop, high, low);
    low = 0x6071243571603524ULL;
    forty2 = 0x44;
    RWPS2_MAKE_QWORD(flush, forty2, low);

    sweOpenLocalPkt(1, -7);
    packet = inv_GLI_packetCursor;
    if (packet == NULL) {
        printf("%s(%d): DMA memory failure\n", "rw3_dspchg.c", 0x8C);
        return;
    }

    packet->q = gif;
    inv_GLI_packetCursor = packet + 1;
    packet[1].q = tag;
    inv_GLI_packetCursor = packet + 2;
    packet[2].q = prim;
    inv_GLI_packetCursor = packet + 3;
    packet[3].q = nreg;
    inv_GLI_packetCursor = packet + 4;
    packet[4].q = eop;
    inv_GLI_packetCursor = packet + 5;
    packet[5].q = flush;
    inv_GLI_packetCursor = packet + 6;
    packet[6].q = address;
    inv_GLI_packetCursor = packet + 7;
}

void inv_VID_fn_vRestoreDisplay(void)
{
#ifdef PC_PORT
    if (!pc_mod_skip_vblank_waits()) {
        pc_frame_wait_vblank();
    }
    skyTexCacheFlush();
#else
    sceGsSyncV(0);
    if (inv_RW_currentVideoMode->height == 256 || inv_RW_currentVideoMode->height == 512) {
        if (inv_RW_currentVideoMode->flags & rwVIDEOMODEFFINTERLACE) {
            sceGsResetGraph(0, 1, 3, 1);
        } else if (inv_RW_currentVideoMode->flags & rwVIDEOMODEINTERLACE) {
            sceGsResetGraph(0, 1, 3, 0);
        } else {
            sceGsResetGraph(0, 0, 3, 1);
        }
    } else {
        if (inv_RW_currentVideoMode->flags & rwVIDEOMODEFFINTERLACE) {
            sceGsResetGraph(0, 1, 2, 1);
        } else if (inv_RW_currentVideoMode->flags & rwVIDEOMODEINTERLACE) {
            sceGsResetGraph(0, 1, 2, 0);
        } else {
            sceGsResetGraph(0, 0, 2, 1);
        }
    }
    sceGsSwapDBuffDc(inv_MPEG_gsDoubleBuffer, D_00393C58);
    inv_RW3_fn_vSendDisplayChangePacket();
    skyTexCacheFlush();
#endif
}

Bigfile* inv_FMN_fn_bMountBigfile(void* path, void* name)
{
    Bigfile* descriptor = malloc(sizeof(Bigfile));

    memset(descriptor, 0, sizeof(Bigfile));
    if (inv_ubi_bigfile_parseDescriptor(descriptor, name) == 1) {
        descriptor->handle = inv_FMN_fn_ulOpenVirtualOrFallback(path);
        if (descriptor->handle != -1) {
            return descriptor;
        }
    }
    inv_UBI_FILE_fn_vUnmountBigfileIfNotNull(descriptor);
    return NULL;
}
