#include "common.h"

#include "snd/snd.h"

extern SndPsxMemory inv_SND_memoryPool;

void SND_fn_vInitMallocSnd(void)
{
    fn_vConstruct_sndpsxmem(&inv_SND_memoryPool);
}

void func_00169B80(void)
{
    func_00169DC0(&inv_SND_memoryPool);
}

void SND_fn_vDesInitMallocSnd(void)
{
    fn_vDestruct_sndpsxmem(&inv_SND_memoryPool);
}

void* SND_fn_pvMallocSnd(s32 size)
{
    return fn_pvMalloc_sndpsxmem(&inv_SND_memoryPool, 2, size);
}

void* SND_fn_pvMallocSndEx(s32 unused, s32 size)
{
    return fn_pvMalloc_sndpsxmem(&inv_SND_memoryPool, 2, size);
}

void SND_fn_vFreeSnd(void* block)
{
    fn_iFree_sndpsxmem(&inv_SND_memoryPool, block);
}

void SND_fn_vFreeSndEx(s32 unused, void* block)
{
    fn_iFree_sndpsxmem(&inv_SND_memoryPool, block);
}

void* SND_fn_pvReAllocSnd(void* block, s32 size)
{
    return fn_pvReAlloc_sndpsxmem(&inv_SND_memoryPool, block, size);
}

void* SND_fn_pvReAllocSndEx(s32 unused, void* block, s32 size)
{
    return fn_pvReAlloc_sndpsxmem(&inv_SND_memoryPool, block, size);
}
