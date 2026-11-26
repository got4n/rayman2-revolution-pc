#include "common.h"

#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "rwps2_macros.h"

extern f32 D_00393720;
extern f32 D_00393724;
extern u8 dword_22EAA0[];
extern u8 dword_22FB90[];
extern u8 dword_230DB0[];
extern u8 dword_2348D0[];
extern void* off_378B80[];

s32 func_00178C70(RpAtomic* atomic, void** pipeline);
void func_00178C20(void);

void inv_RW3_fn_vSetAtomicPipeline(RpAtomic* atomic, void* pipeline)
{
    func_00178C20();
}

void* inv_RW3_fn_pGetAtomicPipeline(RpAtomic* atomic)
{
    void* result = NULL;

    func_00178C70(atomic, &result);
    return result;
}

s32 inv_RW3_fn_bIsLightmapPipeline(void* pipeline)
{
    s32 matches = 0;

    if (pipeline != NULL)
        matches = pipeline == inv_RW3_p_stLightmapPipeline;
    return matches;
}

void func_001519D0(void)
{
    off_378B80[8] = dword_22EAA0;
    off_378B80[10] = dword_22EAA0;
}

void func_001519F0(void)
{
    off_378B80[1] = dword_2348D0;
    off_378B80[2] = dword_230DB0;
    off_378B80[3] = dword_230DB0;
    off_378B80[5] = dword_22FB90;
    printf("*** INFO -> Default pipelines now use FOG ********\n");
    D_00393720 = 30.0f;
    D_00393724 = 190.0f;
}


void sweFinaliseOpenLocalPkt(u32 type, s32 size);

void inv_RW3_fn_vSendFogDistances(f32 width, f32 height)
{
    s32 x = width;
    s32 y = height;
    u64 high;
    u64 low;
    u128 q;

    sweFinaliseOpenLocalPkt(0x88000501, -3);
    if (inv_GLI_packetCursor == NULL) {
        printf("%s(%d): DMA memory failure\n", "rw3_pp_setup.c", 0xBD);
        return;
    }
    if (x == 0 && y == 0) {
        x = D_00393720;
        y = D_00393724;
    } else {
        D_00393720 = x;
        D_00393724 = y;
    }
    low = 0x10000001;
    high = 0x6C0103E201000404ULL;
    RWPS2_MAKE_QWORD(q, high, low);
    (inv_GLI_packetCursor++)->q = q;
    low = ((u64) x << 32) | (u64) y;
    high = low;
    RWPS2_MAKE_QWORD(q, high, low);
    (inv_GLI_packetCursor++)->q = q;
    low = 0xF0000000;
    high = 0;
    RWPS2_MAKE_QWORD(q, high, low);
    (inv_GLI_packetCursor++)->q = q;
    sweFinaliseOpenLocalPkt(1, 0);
}
