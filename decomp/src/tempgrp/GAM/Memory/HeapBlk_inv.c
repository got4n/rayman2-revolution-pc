#include "common.h"

#include "GAM/GAM.h"

#include "ps2/libc.h"

#define HEAP_USE_LISTS 0x0001
#define HEAP_USE_CALLBACKS 0x0002
#define HEAP_ADDRESS_ORDER 0x0004
#define HEAP_NOT_OWNED 0x8000

typedef struct gu_Ptr14 {
    u8 pad00[0x14];
    void* value;
} gu_Ptr14;

void freemem(void* ptr);

typedef void* (*MemAllocFn)(u32 size);
typedef void* (*MemResizeFn)(void* ptr, u32 size);
typedef void (*MemReleaseFn)(void* ptr);
typedef void* (*MemMoveFn)(void* dst, const void* src, u32 n);

extern MemoryHeapView* inv_MEM_defaultHeap;
extern MemAllocFn inv_MEM_allocateCallback;
extern MemResizeFn inv_MEM_resizeCallback;
extern MemReleaseFn inv_MEM_releaseCallback;
extern MemMoveFn inv_MEM_moveCallback;

void inv_MEM_fn_vSetCallbacks(MemAllocFn allocate, MemResizeFn resize, MemReleaseFn release, MemMoveFn move);
void inv_MEM_fn_vResetHeap(MemoryHeapView* heap);
void* inv_MEM_fn_p_vAllocInDefaultHeap(u32 size);
void* inv_MEM_fn_p_vReallocInDefaultHeap(void* ptr, u32 size);
void inv_MEM_fn_vFreeInDefaultHeap(void* ptr);
u32 inv_fn_ulGetBiggestFreeBlockSizeInList(MemoryHeapView* heap);
void inv_MEM_fn_vListAddHead(MemoryListView* list, MemoryBlockView* block);
void inv_MEM_fn_vListAddTail(MemoryListView* list, MemoryBlockView* block);
void inv_MEM_fn_vListInit(MemoryListView* list);
void inv_MEM_fn_vListRemove(MemoryListView* list, MemoryBlockView* block, u8 clearLinks);
void inv_MEM_fn_vListInsertBefore(MemoryListView* list, MemoryBlockView* before, MemoryBlockView* block);

static inline MemoryBlockView* inv_GAM_fn_p_stMemoryListFirst(MemoryListView* list)
{
    return list->count != 0 ? list->first : NULL;
}

static inline MemoryBlockView* inv_GAM_fn_p_stMemoryListNext(MemoryListView* list, MemoryBlockView* block)
{
    return block->next != (MemoryBlockView*) list ? block->next : NULL;
}

static inline u32 inv_GAM_fn_ulMemoryBlockPayloadSize(MemoryBlockView* block)
{
    u32 size = (u32) block->end - sizeof(MemoryBlockView);

    return size - (u32) block;
}

void inv_MEM_fn_vInitDefaultHeap(u32 size)
{
    if (inv_MEM_defaultHeap != NULL) {
        return;
    }

    inv_MEM_fn_vSetCallbacks(malloc, realloc, freemem, memmove);
    inv_MEM_defaultHeap = inv_MEM_fn_p_vAllocAligned(NULL, size, 0x80, HEAP_USE_LISTS | HEAP_ADDRESS_ORDER);
    inv_MEM_fn_vSetCallbacks(inv_MEM_fn_p_vAllocInDefaultHeap, inv_MEM_fn_p_vReallocInDefaultHeap, inv_MEM_fn_vFreeInDefaultHeap, memmove);
}

u32 inv_fn_ulGetBiggestFreeBlockSize(void)
{
    return inv_fn_ulGetBiggestFreeBlockSizeInList(inv_MEM_defaultHeap);
}

void* inv_MEM_fn_p_vAllocInDefaultHeap(u32 size)
{
    return inv_MEM_fn_p_vAlloc(inv_MEM_defaultHeap, size);
}

void* inv_MEM_fn_p_vReallocInDefaultHeap(void* ptr, u32 size)
{
    return inv_MEM_fn_p_vRealloc(inv_MEM_defaultHeap, ptr, size);
}

void inv_MEM_fn_vFreeInDefaultHeap(void* ptr)
{
    inv_MEM_fn_vFreeInHeap(inv_MEM_defaultHeap, ptr);
}

void* inv_MEM_fn_p_vAllocWithCallback(MemoryHeapView* heap, u32 size)
{
    if (heap->flags & HEAP_USE_CALLBACKS) {
        heap->usedSize += size;
        if (heap->peakSize < heap->usedSize) {
            heap->peakSize = heap->usedSize;
        }
        return inv_MEM_allocateCallback(size);
    }
    return NULL;
}

MemoryBlockView* inv_MEM_fn_p_stAlignBlock(MemoryHeapView* heap, MemoryBlockView* address)
{
    u16 align = heap->align;
    u32 addr = (u32) address;
    u32 remainder = addr % align;
    u32 block;

    if (remainder != 0) {
        block = addr - remainder + align;
    } else {
        block = addr;
    }

    block -= sizeof(MemoryBlockView);
    while (block < addr) {
        block += align;
    }
    return (MemoryBlockView*) block;
}

MemoryBlockView* inv_MEM_fn_p_stAlignBlockEnd(MemoryHeapView* heap, MemoryBlockView* block)
{
    return block->end = inv_MEM_fn_p_stAlignBlock(heap, block->end);
}

void inv_MEM_fn_vResizeBlock(MemoryHeapView* heap, MemoryBlockView* block, u32 need, u8 fromFreeList)
{
    MemoryBlockView* oldEnd;
    MemoryBlockView* neighbor;
    MemoryBlockView* split;
    s32 remain;
    s32 size;
    u32 used;

    oldEnd = block->end;
    size = (u8*) oldEnd - (u8*) block;
    if (fromFreeList == 0) {
        heap->usedSize -= size;
        oldEnd = block->end;
    }
    neighbor = NULL;
    if (oldEnd < heap->end) {
        neighbor = oldEnd;
    }
    block->end = (MemoryBlockView*) ((u8*) block + need);
    split = inv_MEM_fn_p_stAlignBlockEnd(heap, block);
    heap->usedSize += (u8*) split - (u8*) block;

    if (fromFreeList != 0) {
        inv_MEM_fn_vListRemove(&heap->freeList, block, 0);
        if (heap->flags & HEAP_ADDRESS_ORDER) {
            inv_MEM_fn_vListInsertBefore(&heap->used.list, neighbor, block);
        } else {
            inv_MEM_fn_vListAddHead(&heap->used.list, block);
        }
    }

    remain = (u8*) oldEnd - (u8*) split;
    if (remain > (s32) sizeof(MemoryBlockView)) {
        if (heap->flags & HEAP_ADDRESS_ORDER) {
            while (neighbor != NULL && neighbor->prev == block) {
                block = neighbor;
                neighbor = NULL;
                if (block->end != heap->end) {
                    neighbor = block->end;
                }
            }
            if (oldEnd == neighbor) {
                inv_MEM_fn_vListRemove(&heap->freeList, oldEnd, 0);
                inv_MEM_fn_vListInsertBefore(&heap->freeList, oldEnd->next, split);
                split->end = oldEnd->end;
            } else {
                inv_MEM_fn_vListInsertBefore(&heap->freeList, neighbor, split);
                split->end = oldEnd;
            }
        } else {
            inv_MEM_fn_vListAddHead(&heap->freeList, split);
            split->end = oldEnd;
        }
    } else {
        block->end = oldEnd;
        heap->usedSize += remain;
    }

    used = heap->usedSize;
    if (heap->peakSize < used) {
        heap->peakSize = used;
    }
}

void inv_MEM_fn_vSetCallbacks(MemAllocFn allocate, MemResizeFn resize, MemReleaseFn release, MemMoveFn move)
{
    inv_MEM_allocateCallback = allocate;
    inv_MEM_resizeCallback = resize;
    inv_MEM_releaseCallback = release;
    inv_MEM_moveCallback = move;
}

MemoryHeapView* inv_MEM_fn_p_vAllocAligned(void* memory, u32 size, u16 align, u16 flags)
{
    MemoryHeapView* heap;
    MemoryBlockView* limit;
    MemoryBlockView* end;

    if (memory != NULL) {
        heap = memory;
        heap->flags |= HEAP_NOT_OWNED;
    } else {
        heap = inv_MEM_allocateCallback(size);
        if (heap == NULL) {
            return NULL;
        }
        heap->flags = 0;
        memory = heap;
    }
    heap->align = align;
    if (flags & HEAP_USE_LISTS) {
        limit = (MemoryBlockView*) ((u8*) memory + size);
        end = inv_MEM_fn_p_stAlignBlock(heap, limit);
        while (limit < end) {
            end = (MemoryBlockView*) ((u8*) end - align);
        }
        heap->end = end;
    } else {
        size -= ((u32) heap + size) % align;
        heap->end = (MemoryBlockView*) ((u8*) heap + size);
        heap->used.top = (u8*) heap->end;
    }
    heap->peakSize = 0;
    heap->usedSize = 0;
    heap->flags |= flags;
    inv_MEM_fn_vResetHeap(heap);
    return heap;
}

void inv_MEM_fn_vResetHeap(MemoryHeapView* heap)
{
    if (heap->flags & HEAP_USE_LISTS) {
        MemoryBlockView* first;

        inv_MEM_fn_vListInit(&heap->used.list);
        inv_MEM_fn_vListInit(&heap->freeList);
        first = inv_MEM_fn_p_stAlignBlock(heap, (MemoryBlockView*) (heap + 1));
        inv_MEM_fn_vListAddHead(&heap->freeList, first);
        first->end = heap->end;
        inv_MEM_fn_p_stAlignBlockEnd(heap, first);
    } else {
        heap->used.top = (u8*) heap->end;
    }
    heap->usedSize = 0;
}

void inv_MEM_fn_vFree(MemoryHeapView** slot)
{
    MemoryHeapView* heap = *slot;

    if (!(heap->flags & HEAP_NOT_OWNED)) {
        inv_MEM_releaseCallback(heap);
    }
    *slot = NULL;
}

s32 inv_MEM_fn_bIsInHeap(void* heapPtr, void* ptr)
{
    MemoryHeapView* heap = heapPtr;

    return (u8*) ptr <= (u8*) heap->end && (u8*) ptr > (u8*) (heap + 1);
}

void* inv_MEM_fn_p_vAlloc(MemoryHeapView* heap, u32 size)
{
    MemoryBlockView* best = NULL;

    if (size == 0) {
        return NULL;
    }

    if (heap->flags & HEAP_USE_LISTS) {
        MemoryBlockView* block;
        u32 need = size + sizeof(MemoryBlockView);
        u32 bestSize = 0xFFFFFFFF;

        for (block = inv_GAM_fn_p_stMemoryListFirst(&heap->freeList); block != NULL;
             block = inv_GAM_fn_p_stMemoryListNext(&heap->freeList, block)) {
            u32 blockSize = (u8*) block->end - (u8*) block;

            if (blockSize < bestSize && blockSize >= need) {
                bestSize = blockSize;
                best = block;
            }
        }

        if (best != NULL) {
            inv_MEM_fn_vResizeBlock(heap, best, need, 1);
            return best + 1;
        }
        return inv_MEM_fn_p_vAllocWithCallback(heap, size);
    } else {
        u16 align = heap->align;
        u32 remainder = size % align;
        u8* top;

        if (remainder != 0) {
            size += align - remainder;
        }
        heap->usedSize += size;
        if (heap->peakSize < heap->usedSize) {
            heap->peakSize = heap->usedSize;
        }
        top = heap->used.top - size;
        if (top < (u8*) (heap + 1)) {
            return NULL;
        }
        heap->used.top = top;
        return top;
    }
}

void* inv_MEM_fn_p_vRealloc(MemoryHeapView* heap, void* ptr, u32 size)
{
    MemoryBlockView* block;
    s32 extra;
    void* result;

    if (size == 0) {
        return NULL;
    }
    if (ptr == NULL) {
        return inv_MEM_fn_p_vAlloc(heap, size);
    }
    if (!inv_MEM_fn_bIsInHeap(heap, ptr)) {
        if (heap->flags & HEAP_USE_CALLBACKS) {
            return inv_MEM_resizeCallback(ptr, size);
        }
        return NULL;
    }

    block = (MemoryBlockView*) ptr - 1;
    extra = size - ((u8*) block->end - (u8*) ptr);
    if (extra < 0) {
        result = ptr;
        inv_MEM_fn_vResizeBlock(heap, block, size + sizeof(MemoryBlockView), 0);
    } else if (extra > 0) {
        result = inv_MEM_fn_p_vAlloc(heap, size);
        if (result != NULL) {
            inv_MEM_moveCallback(result, ptr, (u8*) block->end - (u8*) ptr);
            inv_MEM_fn_vFreeInHeap(heap, ptr);
        }
    } else {
        result = ptr;
    }
    return result;
}

void inv_MEM_fn_vFreeInHeap(MemoryHeapView* heap, void* ptr)
{
    if (inv_MEM_fn_bIsInHeap(heap, ptr)) {
        MemoryBlockView* block = (MemoryBlockView*) ptr - 1;

        heap->usedSize -= (u8*) block->end - (u8*) block;
        if (heap->flags & HEAP_ADDRESS_ORDER) {
            MemoryBlockView* neighbor;
            MemoryBlockView* before;
            MemoryBlockView* previous;

            neighbor = block->end;
            before = NULL;
            if (neighbor < heap->end) {
                before = neighbor;
            }
            if (before != NULL && before->prev == block) {
                MemoryBlockView* current;

                do {
                    current = before;
                    before = NULL;
                    if (current->end != heap->end) {
                        before = current->end;
                    }
                } while (before != NULL && before->prev == current);
            }

            inv_MEM_fn_vListRemove(&heap->used.list, block, 0);
            inv_MEM_fn_vListInsertBefore(&heap->freeList, before, block);

            if (neighbor == before) {
                inv_MEM_fn_vListRemove(&heap->freeList, neighbor, 0);
                block->end = neighbor->end;
                neighbor = block->end;
            }

            previous = block->prev;
            if (previous != NULL && (u8*) previous >= (u8*) (heap + 1) && previous->end == block) {
                inv_MEM_fn_vListRemove(&heap->freeList, block, 0);
                previous->end = neighbor;
            }
        } else {
            inv_MEM_fn_vListRemove(&heap->used.list, block, 0);
            inv_MEM_fn_vListAddHead(&heap->freeList, block);
        }
    } else if (heap->flags & HEAP_USE_CALLBACKS) {
        inv_MEM_releaseCallback(ptr);
    }
}

u32 inv_fn_ulGetBiggestFreeBlockSizeInList(MemoryHeapView* heap)
{
    MemoryListView* list = &heap->freeList;
    MemoryBlockView* block;
    MemoryBlockView* next;
    u32 best = 0;

    next = list->count != 0 ? list->first : NULL;
    for (block = next; block != NULL; block = next) {
        MemoryBlockView* link = block->next;
        u32 size = inv_GAM_fn_ulMemoryBlockPayloadSize(block);

        if (link == (MemoryBlockView*) list) {
            link = NULL;
        }
        if (best < size) {
            best = size;
        }
        next = link;
    }
    return best;
}

u32 inv_fn_ulGetTotalFreeSizeInList(MemoryHeapView* heap)
{
    MemoryListView* list = &heap->freeList;
    MemoryBlockView* block;
    u32 total = 0;

    for (block = inv_GAM_fn_p_stMemoryListFirst(list); block != NULL; block = inv_GAM_fn_p_stMemoryListNext(list, block)) {
        total += inv_GAM_fn_ulMemoryBlockPayloadSize(block);
    }
    return total;
}

void* func_0014CE60(gu_Ptr14* object)
{
    return object->value;
}

u32 inv_MEM_fn_ulGetBlockSize(MemoryHeapView* heap, void* ptr)
{
    if (inv_MEM_fn_bIsInHeap(heap, ptr)) {
        return (u8*) ((MemoryBlockView*) ptr - 1)->end - (u8*) ptr;
    }
    return 0;
}

void* inv_MEM_fn_p_vFindBlockFrom(MemoryListView* list, void* address)
{
    MemoryBlockView* block = inv_GAM_fn_p_stMemoryListFirst(list);

    while (block != NULL && (void*) block < address) {
        block = inv_GAM_fn_p_stMemoryListNext(list, block);
    }
    return block != NULL ? block + 1 : NULL;
}

void inv_MEM_fn_vListAddHead(MemoryListView* list, MemoryBlockView* block)
{
    block->next = list->first;
    block->prev = (MemoryBlockView*) list;
    if (list->count != 0) {
        list->first->prev = block;
    }
    list->first = block;
    if (list->count == 0) {
        list->last = block;
    }
    list->count++;
}

void inv_MEM_fn_vListAddTail(MemoryListView* list, MemoryBlockView* block)
{
    block->next = (MemoryBlockView*) list;
    block->prev = list->last;
    list->last->next = block;
    list->last = block;
    list->count++;
}

void inv_MEM_fn_vListInit(MemoryListView* list)
{
    list->first = (MemoryBlockView*) list;
    list->last = (MemoryBlockView*) list;
    list->count = 0;
}

void inv_MEM_fn_vListRemove(MemoryListView* list, MemoryBlockView* block, u8 clearLinks)
{
    if (list->count == 0) {
        return;
    }
    block->prev->next = block->next;
    block->next->prev = block->prev;
    if (clearLinks) {
        block->next = NULL;
        block->prev = NULL;
    }
    list->count--;
}

void inv_MEM_fn_vListInsertAfter(MemoryListView* list, MemoryBlockView* after, MemoryBlockView* block)
{
    if (after != NULL) {
        after->next->prev = block;
        block->next = after->next;
        after->next = block;
        block->prev = after;
        list->count++;
    } else {
        inv_MEM_fn_vListAddHead(list, block);
    }
}

void inv_MEM_fn_vListInsertBefore(MemoryListView* list, MemoryBlockView* before, MemoryBlockView* block)
{
    if (before != NULL) {
        before->prev->next = block;
        block->prev = before->prev;
        before->prev = block;
        block->next = before;
        list->count++;
    } else {
        inv_MEM_fn_vListAddTail(list, block);
    }
}
