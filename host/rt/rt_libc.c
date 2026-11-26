#include "game_vars.h"
#include <stdbool.h>
#include <string.h>

#include "../crt.h"
#include "../../gli/guest.h"

static inline u8 *P(u32 a) { return revo_ptr(a); }
#define CTYPE_ENTRIES GAME_VAR(inv_FMN_pathCharacterFlags)
#define CTYPE_UPPER 0x01
#define CTYPE_LOWER 0x02
static inline u8 ctype_of(s32 c) { return *revo_ptr(CTYPE_ENTRIES + (u32)c); }
static inline u8 lower(u8 c) { return ctype_of((s8)c) & CTYPE_UPPER ? (u8)(c + ('a' - 'A')) : c; }
static inline u8 upper(u8 c) { return ctype_of((s8)c) & CTYPE_LOWER ? (u8)(c - ('a' - 'A')) : c; }
static inline s32 lower_signed(u8 c) { return ctype_of((s8)c) & CTYPE_UPPER ? (s8)c + ('a' - 'A') : (s8)c; }
static inline s32 lower_unsigned(u8 c) { return ctype_of(c) & CTYPE_UPPER ? c + ('a' - 'A') : c; }

u32 crt_memcpy(u32 d, u32 s, u32 n) {
    u8 *dp = P(d), *sp = P(s);
    if (n && dp && sp) {
        if (d > s && d < s + n)
            for (u32 i = 0; i < n; i++) dp[i] = sp[i];
        else
            memmove(dp, sp, n);
    }
    return d;
}
u32 crt_memmove(u32 d, u32 s, u32 n) {
    if (n) memmove(P(d), P(s), n);
    return d;
}
u32 crt_memset(u32 d, s32 c, u32 n) {
    if (n) memset(P(d), (u8)c, n);
    return d;
}
s32 crt_memcmp(u32 a_, u32 b_, u32 n) {
    const u8 *a = P(a_), *b = P(b_);
    for (u32 i = 0; i < n; i++)
        if (a[i] != b[i]) return (s32)a[i] - (s32)b[i];
    return 0;
}

u32 crt_strlen(u32 s) { return (u32)strlen((const char *)P(s)); }
u32 crt_strcpy(u32 d, u32 s_) {
    const u8 *s = P(s_);
    u8 *dp = P(d);
    u32 i = 0;
    do dp[i] = s[i]; while (s[i++]);
    return d;
}
u32 crt_strncpy(u32 d, u32 s_, u32 n) {
    const u8 *s = P(s_);
    u8 *dp = P(d);
    u32 i = 0;
    for (; i < n && s[i]; i++) dp[i] = s[i];
    for (; i < n; i++) dp[i] = 0;
    return d;
}
u32 crt_strncat(u32 d, u32 s_, u32 n) {
    const u8 *s = P(s_);
    u8 *dp = P(d);
    dp += strlen((const char *)dp);
    u32 i = 0;
    for (; i < n && s[i]; i++) dp[i] = s[i];
    dp[i] = 0;
    return d;
}
s32 crt_strcmp(u32 a_, u32 b_) {
    const u8 *a = P(a_), *b = P(b_);
    while (*a && *a == *b) a++, b++;
    return (s32)*a - (s32)*b;
}
s32 crt_strncmp(u32 a_, u32 b_, u32 n) {
    const u8 *a = P(a_), *b = P(b_);
    if (!n) return 0;
    while (--n && *a && *a == *b) a++, b++;
    return (s32)*a - (s32)*b;
}
s32 crt_strcasecmp(u32 a_, u32 b_) {
    const u8 *a = P(a_), *b = P(b_);
    while (*a && lower_signed(*a) == lower_signed(*b)) a++, b++;
    return lower_unsigned(*a) - lower_unsigned(*b);
}
u32 crt_index(u32 s, s32 c_) {
    const u8 *p = P(s);
    u8 c = (u8)c_;
    for (u32 i = 0;; i++) {
        if (p[i] == c) return s + i;
        if (!p[i]) return 0;
    }
}
u32 crt_rindex(u32 s, s32 c_) {
    u32 found = 0;
    const u8 *p = P(s);
    u8 c = (u8)c_;
    for (u32 i = 0;; i++) {
        if (p[i] == c) found = s + i;
        if (!p[i]) break;
    }
    return found;
}
u32 crt_strstr(u32 h, u32 n) {
    const char *hp = (const char *)P(h), *np = (const char *)P(n);
    const char *r = strstr(hp, np);
    return r ? h + (u32)(r - hp) : 0;
}
u32 crt_strlwr(u32 s) {
    for (u8 *p = P(s); *p; p++) *p = lower(*p);
    return s;
}
u32 crt_strupr(u32 s) {
    for (u8 *p = P(s); *p; p++) *p = upper(*p);
    return s;
}

float crt_fabsf(float x) {
    u32 b;
    memcpy(&b, &x, 4);
    b &= 0x7FFFFFFFu;
    memcpy(&x, &b, 4);
    return x;
}
