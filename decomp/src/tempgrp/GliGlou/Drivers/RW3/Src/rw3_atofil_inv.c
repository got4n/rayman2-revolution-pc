#include "common.h"

#include "ps2/libc.h"
#include "renderware.h"
#include "AI/AI.h"
#include "FMN/FMN.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"

typedef RpAtomic* (*AtomicCallback)(RpAtomic* atomic, void* data);

char* strlwr(char* s);
s32 FMN_Read(u32 file, void* data, s32 count, s32 size);
s32 RpClumpGetNumAtomics(RpClump* clump);
RpClump* RpClumpForAllAtomics(RpClump* clump, AtomicCallback callback, void* data);

RpAtomic* func_0014DA90(RpAtomic* atomic, void* frame)
{
    RpAtomicSetFrame(atomic, frame);
    return atomic;
}

s32 inv_ATO_fn_bLoadLevelAtomicClump(char* name)
{
    RwMemory source;
    char path[0x80];
    u32 file;
    s32 size;
    void* data;
    RwStream* stream;
    RpClump* clump;

    strlwr(name);
    sprintf(path, "%s/%s.ato.%d", name, name, 0);
    printf("opening ATO : %s\n", path);
    if (0) printf("ERROR -> BAD Clump File : %s\n", path);
    if (0) printf("ERROR -> Cannot open Clump File : %s\n", path);
    file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
    if (file < 0xF) {
        size = FMN_Length(file);
        inv_MEM_fn_vCreateLevelSlotHeap(size + 0x400, 1);
        data = AI_fn_p_vTrueAlloc(gp_6F74, size);
        source.start = data;
        source.length = size;
        FMN_Read(file, data, 1, size);
        FMN_Close(file);
        stream = RwStreamOpen(3, 1, &source);
        if (stream != NULL) {
            if (RwStreamFindChunk(stream, 0x10, NULL, NULL) != 0) {
                clump = RpClumpStreamRead(stream);
                RwStreamClose(stream, NULL);
                inv_RW3_levelClump = clump;
                printf("INFO -> Clump loaded from binary file : %s\n", path);
                inv_MEM_fn_vFreeLevelSlotHeap(1);
            } else {
                printf("ERROR -> BAD Clump File : %s\n", path);
                return 0;
            }
        } else {
            printf("ERROR -> Cannot open Clump File : %s\n", path);
            return 0;
        }
    }
    printf("ATO : %s file loaded with %d atomics.\n", path, RpClumpGetNumAtomics(inv_RW3_levelClump));
    RpClumpForAllAtomics(inv_RW3_levelClump, func_0014DA90, NULL);
    RwFrameDestroyHierarchy(inv_RW3_levelClump->object.parent);
    inv_RW3_levelClump->object.parent = NULL;
    return 1;
}

typedef struct ClumpCallbackState {
    void* result;
} ClumpCallbackState;

s32 func_0014DC60(void* object, ClumpCallbackState* state)
{
    state->result = object;
    return 0;
}
