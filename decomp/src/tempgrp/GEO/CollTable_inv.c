#include "common.h"

#include "COL/COL.h"
#include "GEO/GEO.h"

extern u32 inv_COL_ulNbQueuedEntries;
extern CollisionEntry* inv_COL_a_pQueuedEntries[];

void* memset(void* dst, int value, unsigned int size);

void COL_fn_vInitCollisionTable(Octree* table)
{
    inv_COL_p_stCollisionTable = table;
    inv_COL_ulNbQueuedEntries = 0;
}

void COL_fn_bGetResultCollision(OctreeHit_inv* out, u32* count, u32 max)
{
    CollisionEntry** entry;
    CollisionEntry** end;
    CollisionEntry** pending;
    CollisionEntry** pendingEnd;
    OctreeHit_inv* cursor;
    s32 bit;
    u32 mask;
    u32 b;
    u32* word;
    s32 index;

    if (inv_COL_ulNbQueuedEntries != 0) {
        *count = 0;
        cursor = out;
        end = inv_COL_a_pQueuedEntries + inv_COL_ulNbQueuedEntries;
        for (entry = inv_COL_a_pQueuedEntries; entry < end; entry++) {
            index = *entry - inv_COL_p_stCollisionTable->entries;
            (*entry)->queued = 0;
            for (bit = 0, word = (*entry)->bits; (u32) bit < (*entry)->bitCount; word++) {
                if (*word != 0) {
                    if ((*entry)->bitCount < (u32) (bit + 31)) {
                        mask = 0xFFFFFFFF >> (bit - (u16) ((*entry)->bitCount - 31));
                        for (b = 1; b & mask; b <<= 1) {
                            if (*word & b) {
                                cursor->entry = index;
                                cursor->triangle = bit;
                                cursor++;
                                ++*count;
                                if (*count >= max) {
                                    pending = entry;
                                    goto release;
                                }
                            }
                            bit++;
                        }
                    } else {
                        for (b = 1; b != 0; b <<= 1) {
                            if (*word & b) {
                                cursor->entry = index;
                                cursor->triangle = bit;
                                cursor++;
                                ++*count;
                                if (*count >= max) {
                                    pending = entry;
                                    goto release;
                                }
                            }
                            bit++;
                        }
                    }
                    *word = 0;
                } else {
                    bit += 32;
                }
            }
        }
        pending = entry;
    release:
        pendingEnd = inv_COL_a_pQueuedEntries + inv_COL_ulNbQueuedEntries;
        if (entry < pendingEnd) {
            do {
                CollisionEntry* element;

                (*pending)->queued = 0;
                element = *pending++;
                memset(element->bits, 0,
                       (element->bitCount >> 5) + ((element->bitCount & 31) != 0));
            } while (pending < pendingEnd);
        }
        inv_COL_ulNbQueuedEntries = 0;
    }
}

void COL_fn_vAddInCollisionTable(OctreeNode* leaf)
{
    u16* data = leaf->u.elementList;
    u32 elementCount = *data++;

    while (elementCount-- != 0) {
        CollisionEntry* element = &inv_COL_p_stCollisionTable->entries[data[0]];
        u32 bitCount = data[1];
        u32* bits;

        data += 2;
        if (element->queued == 0 && inv_COL_ulNbQueuedEntries < 0x80) {
            inv_COL_a_pQueuedEntries[inv_COL_ulNbQueuedEntries++] = element;
        }
        do {
            element->queued = 1;
        } while (0);
        bits = element->bits;
        while (bitCount-- != 0) {
            u32 bit = *data++;

            bits[bit >> 5] |= 1 << (bit & 0x1F);
        }
    }
}
