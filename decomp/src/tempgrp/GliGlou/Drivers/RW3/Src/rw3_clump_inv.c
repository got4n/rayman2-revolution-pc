#include "common.h"

#include "ps2/libc.h"
#include "renderware_fwd.h"
#include "GLI/GLI.h"

typedef struct AtomicSelection {
    u32 index;
    void* atomic;
} AtomicSelection;

typedef void* (*AtomicCallback)(void* atomic, void* data);

extern u32 inv_RW3_lAtomicEnumIndex;

RpClump* RpClumpForAllAtomics(void* clump, AtomicCallback callback, void* data);
void* RpClumpRemoveAtomic(void* clump, void* atomic);
void* func_0014DC60(void* atomic, void* data);

void* func_0014DC70(void)
{
    RpClump* clump = inv_RW3_levelClump;
    void* atomic = NULL;

    if (clump == NULL) {
        printf("ERROR --> TheClump is NULL\n");
        return NULL;
    }
    RpClumpForAllAtomics(clump, func_0014DC60, &atomic);
    if (atomic != NULL)
        RpClumpRemoveAtomic(inv_RW3_levelClump, atomic);
    return atomic;
}

void* func_0014DCD0(void* atomic, AtomicSelection* selection)
{
    if (atomic == NULL)
        inv_RW3_lAtomicEnumIndex = 0xFFFFFFFF;
    else
        inv_RW3_lAtomicEnumIndex++;
    if (inv_RW3_lAtomicEnumIndex == selection->index) {
        selection->atomic = atomic;
        inv_RW3_lAtomicEnumIndex = 0xFFFFFFFF;
        return NULL;
    }
    return atomic;
}

void* func_0014DD18(s32 index)
{
    RpClump* clump = inv_RW3_levelClump;
    AtomicSelection selection;

    selection.index = index;
    selection.atomic = NULL;
    if (clump == NULL) {
        printf("ERROR --> TheClump is NULL\n");
        return NULL;
    }
    RpClumpForAllAtomics(clump, (AtomicCallback) func_0014DCD0, &selection);
    return selection.atomic;
}
