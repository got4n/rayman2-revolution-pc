#ifndef REVO_RT_H
#define REVO_RT_H

#include <math.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

extern u8 *RAM;

#define REVO_RAM_SIZE 0x02000000u
#define REVO_FAST_LIMIT 0xFFFFFFFFu
#define REVO_GAME_LO 0x00010000u
#define REVO_GAME_HI 0x80000000u

int revo_guest_readable(u32 a, u32 n);

u32 mem_slow_rd32(u32 a);
u64 mem_slow_rd64(u32 a);
u16 mem_slow_rd16(u32 a);
u8 mem_slow_rd8(u32 a);
void mem_slow_wr32(u32 a, u32 v);
void mem_slow_wr64(u32 a, u64 v);
void mem_slow_wr16(u32 a, u16 v);
void mem_slow_wr8(u32 a, u8 v);

static inline u8 rd8(u32 a) { return a < REVO_FAST_LIMIT ? RAM[a] : mem_slow_rd8(a); }
static inline u16 rd16(u32 a) { u16 v; if (a < REVO_FAST_LIMIT) { memcpy(&v, RAM + a, 2); return v; } return mem_slow_rd16(a); }
static inline u32 rd32(u32 a) { u32 v; if (a < REVO_FAST_LIMIT) { memcpy(&v, RAM + a, 4); return v; } return mem_slow_rd32(a); }
static inline u64 rd64(u32 a) { u64 v; if (a < REVO_FAST_LIMIT) { memcpy(&v, RAM + a, 8); return v; } return mem_slow_rd64(a); }
static inline void wr8(u32 a, u8 v) { if (a < REVO_FAST_LIMIT) RAM[a] = v; else mem_slow_wr8(a, v); }
static inline void wr16(u32 a, u16 v) { if (a < REVO_FAST_LIMIT) memcpy(RAM + a, &v, 2); else mem_slow_wr16(a, v); }
static inline void wr32(u32 a, u32 v) { if (a < REVO_FAST_LIMIT) memcpy(RAM + a, &v, 4); else mem_slow_wr32(a, v); }
static inline void wr64(u32 a, u64 v) { if (a < REVO_FAST_LIMIT) memcpy(RAM + a, &v, 8); else mem_slow_wr64(a, v); }

#define SX32(x) ((u64)(s64)(s32)(u32)(x))

static inline float fbits(u32 x) { float f; memcpy(&f, &x, 4); return f; }
static inline u32 bitsf(float f) { u32 x; memcpy(&x, &f, 4); return x; }

static inline float fclamp(float f) {
    u32 b = bitsf(f);
    u32 e = b & 0x7F800000u;
    if (e == 0x7F800000u) return fbits((b & 0x80000000u) | 0x7F7FFFFFu);
    if (e == 0) return fbits(b & 0x80000000u);
    return f;
}
static inline float fin(float f) { return fclamp(f); }
static inline float fadd(float a, float b) { return fclamp(fin(a) + fin(b)); }
static inline float fsub(float a, float b) { return fclamp(fin(a) - fin(b)); }
static inline float fmul(float a, float b) { return fclamp(fin(a) * fin(b)); }
static inline float fdiv(float a, float b) {
    a = fin(a); b = fin(b);
    if (b == 0.0f) {
        u32 s = (bitsf(a) ^ bitsf(b)) & 0x80000000u;
        return fbits(s | 0x7F7FFFFFu);
    }
    return fclamp(a / b);
}
static inline float fsqrt(float a) { return sqrtf(fabsf(fin(a))); }
static inline float frsqrt(float a, float b) {
    float s = sqrtf(fabsf(fin(b)));
    return fdiv(a, s);
}
static inline s32 fcvtw(float f) {
    f = fin(f);
    if (f >= 2147483648.0f) return 0x7FFFFFFF;
    if (f <= -2147483648.0f) return (s32)0x80000000;
    return (s32)f;
}
#define FCOND (1u << 23)

extern int revo_budget;
void revo_preempt(void);

#ifdef __cplusplus
}
#endif

#endif
