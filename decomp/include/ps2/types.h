#ifndef REVO_PS2_TYPES_H
#define REVO_PS2_TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef float f32;
typedef double f64;

typedef unsigned int u128 __attribute__((mode(TI)));

typedef union Qword {
    u128 q;

    struct {
        u64 lo;
        u64 hi;
    } d;
} Qword;

typedef union FloatBits {
    f32 value;
    u32 bits;
} FloatBits;

#endif
