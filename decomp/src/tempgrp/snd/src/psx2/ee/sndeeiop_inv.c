#include "common.h"

s32 sceSifFreeIopHeap(void* block);
void RpAtomicDestroy(void);
void RpClumpDestroy(void);
void freemem(void* block);

void inv_FreeIopHeap(s32 type, void* block)
{
    switch (type) {
    case 0:
#ifdef PC_PORT
        freemem(block);
#else
        sceSifFreeIopHeap(block);
#endif
        break;
    case 2:
        freemem(block);
        break;
    }
}

#ifndef PC_PORT
void j_RpClumpDestroy(void)
{
    RpClumpDestroy();
}

void j_RpAtomicDestroy(void)
{
    RpAtomicDestroy();
}
#endif
