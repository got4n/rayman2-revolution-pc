#ifndef REVO_PS2_COPY_TYPES_H
#define REVO_PS2_COPY_TYPES_H

#include "ps2/types.h"

typedef struct {
    s32 v;
} __attribute__((packed)) UnalignedS32;

typedef struct {
    u64 first;
    u64 second;
    u32 third;
} __attribute__((packed, aligned(4))) PackedU64U64U32Aligned4;

typedef struct {
    u64 first;
    u64 second;
} __attribute__((packed)) UnalignedU64x2;

typedef struct {
    u64 first;
    u64 second;
} __attribute__((packed, aligned(4))) PackedU64x2Aligned4;

#endif
