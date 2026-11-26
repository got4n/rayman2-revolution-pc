#include "common.h"

#include "GAM/GAM.h"

extern MemoryHeapView* inv_MEM_p_stLevelHeap;
extern MemoryHeapView* inv_MEM_a_pLevelSlotHeaps[];

void inv_MEM_fn_vCreateLevelSlotHeap(u32 size, s32 index)
{
    void* heap = inv_MEM_fn_p_vAllocAligned(NULL, size, 0x80, 5);

    gp_6F74 = heap;
    inv_MEM_a_pLevelSlotHeaps[index] = heap;
}

void inv_MEM_fn_vFreeLevelSlotHeap(s32 index)
{
    if (gp_6F74 == inv_MEM_a_pLevelSlotHeaps[index])
        gp_6F74 = NULL;
    inv_MEM_fn_vFree(&inv_MEM_a_pLevelSlotHeaps[index]);
}

void* inv_MEM_fn_p_vAllocAlignedHeap(u32 size)
{
    return inv_MEM_fn_p_vAllocAligned(NULL, size, 4, 5);
}

void inv_MEM_fn_vFreeHeap(MemoryHeapView** slot)
{
    inv_MEM_fn_vFree(slot);
}

void* inv_MEM_fn_p_vAllocInHeap(MemoryHeapView* heap, u32 size)
{
    return inv_MEM_fn_p_vAlloc(heap, size);
}

void inv_fn_vAllocateLevelMemory(s32 size)
{
    void* heap = inv_MEM_fn_p_vAllocAligned(NULL, size, 0x80, 5);

    inv_MEM_p_stLevelHeap = heap;
    gp_6F74 = heap;
}

void fn_vFreeLevelAllowedMemory(void)
{
    if (gp_6F74 == inv_MEM_p_stLevelHeap)
        gp_6F74 = NULL;
    inv_MEM_fn_vFree(&inv_MEM_p_stLevelHeap);
}

void* inv_MEM_fn_p_vAllocInLevelHeap(u32 size)
{
    return inv_MEM_fn_p_vAlloc(inv_MEM_p_stLevelHeap, size);
}

u32 inv_MEM_fn_ulGetBlockSizeInGpHeap(void* block)
{
    void* heap = gp_6F74;

    if (heap != NULL)
        return inv_MEM_fn_ulGetBlockSize(heap, block);
    return 0;
}
