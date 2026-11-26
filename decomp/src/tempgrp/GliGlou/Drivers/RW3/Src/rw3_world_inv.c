#include "common.h"

#include "GAM/GAM.h"

#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "rwps2.h"
#include "rwps2_macros.h"

typedef struct RwMatrixWords {
    u64 words[8];
} RwMatrixWords;

extern MemoryHeapView* inv_RW3_p_stWorldHeap;
extern void* gp_6F74;

s32 skyTexGetTex0(void* texture, u32* high, u32* low);
void* skyTexSetTex0(void* texture, u32 high, u32 low);
RwMatrix* RwFrameGetMatrix(RwFrame* frame);
RwMatrix* RwMatrixUpdate(RwMatrix* matrix);
RwFrame* RwFrameUpdateObjects(RwFrame* frame);
s32 inv_RW3_fn_bWriteTex0Register(u64 data, s32 context);

static inline void inv_GLI_fn_vCopyMatrix(RwMatrix* destination, RwMatrix* source)
{
    *(RwMatrixWords*) destination = *(RwMatrixWords*) source;
}

void RW3_Utils_DuplicateFrame(RwFrame* destination, RwFrame* source)
{
    RwMatrix* matrix = RwFrameGetMatrix(destination);
    RwMatrix* sourceMatrix = RwFrameGetMatrix(source);

    do {
        inv_GLI_fn_vCopyMatrix(matrix, sourceMatrix);
    } while (0);
    RwMatrixUpdate(matrix);
    RwFrameUpdateObjects(destination);
}

s32 func_00157640(u32 value)
{
    switch (value) {
    case 0x400:
        return 10;
    case 0x200:
        return 9;
    case 0x100:
        return 8;
    case 0x80:
        return 7;
    case 0x40:
        return 6;
    case 0x20:
        return 5;
    case 0x10:
        return 4;
    case 0x8:
        return 3;
    case 0x4:
        return 2;
    case 0x2:
        return 1;
    default:
        return 0;
    }
}

s32 inv_RW3_fn_bWriteTex0Register(u64 data, s32 context)
{
    u128 qword;

    if (context != 0) {
        RWPS2_MAKE_QWORD(qword, 7, data);
    } else {
        RWPS2_MAKE_QWORD(qword, 6, data);
    }
    (inv_GLI_packetCursor++)->q = qword;
    return 1;
}

s32 inv_RW3_fn_bWriteRasterTex0(void* texture, void* unused, s32 context)
{
    GsTex0Reg tex0;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    return inv_RW3_fn_bWriteTex0Register(tex0.value, context);
}

void inv_RW3_fn_vWriteTex0WithBaseOf(void* texture, void* unused, s32 context)
{
    GsTex0Reg tex0;
    GsTex0Reg other;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    skyTexGetTex0(texture, &other.words[1], &other.words[0]);
    tex0.bits.tbp0 = other.bits.tbp0;
    inv_RW3_fn_bWriteTex0Register(tex0.value, context);
    inv_RW3_fn_bWriteTex0Register(tex0.value, context);
    inv_RW3_fn_bWriteTex0Register(tex0.value, context);
}

void func_00157820(void)
{
}

s32 inv_RW3_fn_bSameTextureBase(void* left, void* right)
{
    GsTex0Reg leftTex0;
    GsTex0Reg rightTex0;

    skyTexGetTex0(left, &leftTex0.words[1], &leftTex0.words[0]);
    skyTexGetTex0(right, &rightTex0.words[1], &rightTex0.words[0]);
    return leftTex0.bits.tbp0 == rightTex0.bits.tbp0;
}

s32 inv_RW3_fn_lGetRasterTbp0(void* texture)
{
    GsTex0Reg tex0;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    return tex0.bits.tbp0;
}

s32 inv_RW3_fn_ulGetTextureClutBase(void* texture)
{
    GsTex0Reg tex0;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    return tex0.bits.cbp;
}

void inv_RW3_fn_vSetTextureClutBase(void* texture, u32 cbp)
{
    GsTex0Reg tex0;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    tex0.bits.cbp = cbp;
    skyTexSetTex0(texture, tex0.words[1], tex0.words[0]);
}

void inv_RW3_fn_vCopyRasterPalette(RwRaster* destination, RwRaster* source)
{
    GsTex0Reg tex0;
    GsTex0Reg sourceTex0;

    skyTexGetTex0(source, &sourceTex0.words[1], &sourceTex0.words[0]);
    skyTexGetTex0(destination, &tex0.words[1], &tex0.words[0]);
    tex0.bits.tbw = sourceTex0.bits.tbw;
    tex0.bits.psm = sourceTex0.bits.psm;
    tex0.bits.cbp = sourceTex0.bits.cbp;
    tex0.bits.cpsm = sourceTex0.bits.cpsm;
    tex0.bits.csm = sourceTex0.bits.csm;
    tex0.bits.csa = sourceTex0.bits.csa;
    tex0.bits.cld = sourceTex0.bits.cld;
    skyTexSetTex0(destination, tex0.words[1], tex0.words[0]);
}

void inv_RW3_fn_vCopyRasterSize(RwRaster* destination, RwRaster* source)
{
    GsTex0Reg tex0;
    GsTex0Reg sourceTex0;

    skyTexGetTex0(source, &sourceTex0.words[1], &sourceTex0.words[0]);
    skyTexGetTex0(destination, &tex0.words[1], &tex0.words[0]);
    tex0.bits.tbw = sourceTex0.bits.tbw;
    tex0.bits.tw = sourceTex0.bits.tw;
    tex0.bits.th = sourceTex0.bits.th;
    skyTexSetTex0(destination, tex0.words[1], tex0.words[0]);
}

void inv_RW3_fn_vSetTextureFunction(void* texture, s32 function)
{
    GsTex0Reg tex0;
    s32 tfx = function & 0xFF;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    if (tfx <= 1) {
        if (tfx >= 0) {
            tex0.bits.tfx = tfx;
            skyTexSetTex0(texture, tex0.words[1], tex0.words[0]);
            return;
        }
    }
    printf("ERROR --> Trying to set an unsupported Texture Function in TEX0.TFX : Ox%X\n", tfx);
}

u8 inv_RW3_fn_ucGetTextureFunction(void* texture)
{
    GsTex0Reg tex0;

    skyTexGetTex0(texture, &tex0.words[1], &tex0.words[0]);
    return tex0.bits.tfx;
}

void inv_RW3_fn_vInitDefaultHeap(u32 size)
{
    inv_MEM_fn_vInitDefaultHeap(size);
}

void inv_RW3_fn_vCreateWorldHeap(u32 size)
{
    void* heap = inv_MEM_fn_p_vAllocAligned(NULL, size, 4, 5);

    inv_RW3_p_stWorldHeap = heap;
    gp_6F74 = heap;
}

void inv_RW3_fn_vDestroyWorldHeap(void)
{
    if (gp_6F74 == inv_RW3_p_stWorldHeap) {
        gp_6F74 = NULL;
    }
    inv_MEM_fn_vFree(&inv_RW3_p_stWorldHeap);
}
