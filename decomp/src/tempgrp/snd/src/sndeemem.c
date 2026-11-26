#include "common.h"

#include "ps2/libc.h"
#include "snd/snd.h"

#define SND_PSX_MEMORY_BLOCK_COUNT 512
#define SND_PSX_MEMORY_IOP 0
#define SND_PSX_MEMORY_EE 2

typedef struct SndPsxMemoryBlock {
    s32 kind;
    s32 size;
    void* data;
} SndPsxMemoryBlock;

struct SndPsxMemory {
    s32 initialized;
    s32 count;
    SndPsxMemoryBlock blocks[SND_PSX_MEMORY_BLOCK_COUNT];
};

extern u32 D_00393930;

void sceSifInitIopHeap(void);
void* sceSifAllocIopHeap(s32 size);
SndPsxMemoryBlock* func_00169EC8(SndPsxMemory* memory);
SndPsxMemoryBlock* fn_iGetFromPointer_sndpsxmem(SndPsxMemory* memory, void* data);

void func_00169C58(void)
{
}

void fn_vConstruct_sndpsxmem(SndPsxMemory* memory)
{
    SndPsxMemoryBlock* end = &memory->blocks[SND_PSX_MEMORY_BLOCK_COUNT];
    SndPsxMemoryBlock* block = memory->blocks;

    memory->initialized = 1;
    memory->count = 0;
    for (; block < end; block++) {
        block->kind = 0;
        block->size = 0;
        block->data = NULL;
    }
#ifndef PC_PORT
    sceSifInitIopHeap();
#endif
}

void fn_vDestruct_sndpsxmem(SndPsxMemory* memory)
{
    SndPsxMemoryBlock* end = &memory->blocks[SND_PSX_MEMORY_BLOCK_COUNT];
    SndPsxMemoryBlock* block = memory->blocks;

    for (; block < end; block++) {
        if (block->data != NULL && block->size != 0) {
            inv_FreeIopHeap(block->kind, block->data);
            block->kind = 0;
            block->size = 0;
            block->data = NULL;
            memory->count--;
        }
    }
    memory->initialized = 0;
    inv_SND_ulIopBlockCount = memory->count;
}

void func_00169D40(SndPsxMemory* memory)
{
    SndPsxMemoryBlock* end = &memory->blocks[SND_PSX_MEMORY_BLOCK_COUNT];
    SndPsxMemoryBlock* block = memory->blocks;
    SndPsxMemoryBlock* packed = block;

    while (block < end) {
        if (block->data == NULL) {
            block++;
            continue;
        }
        if (packed != block) {
            *packed = *block;
            block->kind = 0;
            block->size = 0;
            block->data = NULL;
        }
        D_00393930++;
        packed++;
        block++;
    }
}

void func_00169DC0(SndPsxMemory* memory)
{
    func_00163100();
}

void* fn_pvMalloc_sndpsxmem(SndPsxMemory* memory, s32 kind, s32 size)
{
    void* result = NULL;
    s32 count;

    if (size > 0 && memory->count < SND_PSX_MEMORY_BLOCK_COUNT) {
        void* data;
        s32 allocated = size & 0x3F;

        if (allocated != 0) {
            allocated = size - allocated + 64;
        } else {
            allocated = size;
        }

        switch (kind) {
        case SND_PSX_MEMORY_IOP:
#ifdef PC_PORT
            data = malloc(allocated);
#else
            data = sceSifAllocIopHeap(allocated);
#endif
            break;
        case SND_PSX_MEMORY_EE:
            data = malloc(allocated);
            break;
        default:
            data = NULL;
            break;
        }

        if (data != NULL) {
            SndPsxMemoryBlock* block = func_00169EC8(memory);

            if (block != NULL) {
                block->kind = kind;
                block->size = allocated;
                block->data = data;
                result = data;
                memory->count++;
            }
        }
    }

    count = memory->count;
    inv_SND_ulIopBlockCount = count;
    if (count > 200) {
        printf("c trop!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    }
    return result;
}

SndPsxMemoryBlock* func_00169EC8(SndPsxMemory* memory)
{
    SndPsxMemoryBlock* end = &memory->blocks[SND_PSX_MEMORY_BLOCK_COUNT];
    SndPsxMemoryBlock* block = memory->blocks;
    SndPsxMemoryBlock* found = NULL;

    for (; block < end; block++) {
        if (block->data == NULL && block->size == 0) {
            found = block;
            break;
        }
    }
    return found;
}

void* fn_pvReAlloc_sndpsxmem(SndPsxMemory* memory, void* data, s32 size)
{
    SndPsxMemoryBlock* block = fn_iGetFromPointer_sndpsxmem(memory, data);
    void* result = NULL;

    if (block != NULL) {
        if (block->size < size) {
            void* grown;
            s32 allocated = size & 0x3F;

            if (allocated != 0)
                allocated = size - allocated + 64;
            else
                allocated = size;

            switch (block->kind) {
            case SND_PSX_MEMORY_IOP:
#ifdef PC_PORT
                grown = malloc(allocated);
#else
                grown = sceSifAllocIopHeap(allocated);
#endif
                break;
            case SND_PSX_MEMORY_EE:
                grown = malloc(allocated);
                break;
            default:
                grown = NULL;
                break;
            }

            if (grown != NULL) {
                memcpy(grown, block->data, block->size);
                inv_FreeIopHeap(block->kind, block->data);
                block->size = allocated;
                block->data = grown;
                result = grown;
            }
        } else {
            block->size = size;
            result = data;
        }
    }
    return result;
}

s32 fn_iFree_sndpsxmem(SndPsxMemory* memory, void* data)
{
    SndPsxMemoryBlock* block = fn_iGetFromPointer_sndpsxmem(memory, data);

    if (block != NULL) {
        inv_FreeIopHeap(block->kind, block->data);
        memory->count--;
        block->kind = 0;
        block->size = 0;
        block->data = NULL;
    }
    inv_SND_ulIopBlockCount = memory->count;
    return -1;
}

SndPsxMemoryBlock* fn_iGetFromPointer_sndpsxmem(SndPsxMemory* memory, void* data)
{
    SndPsxMemoryBlock* end = &memory->blocks[SND_PSX_MEMORY_BLOCK_COUNT];
    SndPsxMemoryBlock* block = memory->blocks;
    SndPsxMemoryBlock* found = NULL;

    for (; block < end; block++) {
        if (block->data == data) {
            found = block;
            break;
        }
    }
    return found;
}
