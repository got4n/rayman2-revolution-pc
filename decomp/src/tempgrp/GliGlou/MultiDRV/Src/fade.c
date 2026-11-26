#include "common.h"

#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "rwps2_macros.h"
#include "rwps2.h"

void func_0014D010(u32 start, u32 end);

void sweOpenLocalPkt(u32 type, s32 size);

void func_0014D010(u32 start, u32 end)
{
    u32 width = g_lVIDEO_WantedWidth;
    u32 blocks = (width + 0x3F) >> 6;
    u32 height = g_lVIDEO_WantedHeight;
    u64 tagHigh;
    u64 tagLow;
    u64 low;
    u64 origin;
    u128 q0;
    u128 q1;
    u128 q2;
    u128 q3;
    u128 q4;

    sweOpenLocalPkt(1, -5);
    if (inv_GLI_packetCursor == NULL) {
        printf("%s(%d): DMA memory failure\n", "fade.c", 0x38);
        return;
    }
    tagLow = 0x1000000000008004ULL;
    tagHigh = 0xE;
    RWPS2_MAKE_QWORD(q0, tagHigh, tagLow);
    (inv_GLI_packetCursor++)->q = q0;
    origin = 0;
    low = ((u64) end | ((u64) blocks << 48)) | (((u64) start << 32) | ((u64) blocks << 16));
    RWPS2_MAKE_QWORD(q1, 0x50, low);
    (inv_GLI_packetCursor++)->q = q1;
    low = (u64) height << 32 | (u64) width;
    RWPS2_MAKE_QWORD(q2, 0x52, low);
    (inv_GLI_packetCursor++)->q = q2;
    RWPS2_MAKE_QWORD(q3, 0x51, origin);
    inv_GLI_packetCursor[0].q = q3;
    inv_GLI_packetCursor = inv_GLI_packetCursor + 1;
    RWPS2_MAKE_QWORD(q4, 0x53, 2);
    inv_GLI_packetCursor[0].q = q4;
    inv_GLI_packetCursor = inv_GLI_packetCursor + 1;
}

void inv_GLI_fn_vPrepareLevelFadeOut(void)
{
    if (D_00393C10 == 0)
        func_0014D010((u32) (g_lVIDEO_WantedWidth * g_lVIDEO_WantedHeight) >> 6, 0);
    else
        func_0014D010(0, (u32) (g_lVIDEO_WantedWidth * g_lVIDEO_WantedHeight) >> 6);
}
