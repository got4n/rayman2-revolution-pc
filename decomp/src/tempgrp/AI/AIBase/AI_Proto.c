#include "common.h"

#include "ps2/libc.h"
#include "GAM/GAM.h"

void* AI_fn_p_vTrueAlloc(void* heap, s32 size)
{
    void* allocation = inv_MEM_fn_p_vAlloc(heap, size);
    memset(allocation, 0, size);
    return allocation;
}
