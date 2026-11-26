#include "game_vars.h"
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../crt.h"
#include "../../gli/guest.h"

#define IMPURE_PTR GAME_VAR(off_3887FC)
#define REENT_ERRNO 0
#define REENT_RAND_NEXT 88
#define REENT_STRTOK_LAST 92
#define CTYPE_ENTRIES GAME_VAR(inv_FMN_pathCharacterFlags)
#define CTORS GAME_VAR(__CTOR_LIST__)
#define MAIN_DONE GAME_VAR(initialized_10)
#define FRAME_OBJECTS GAME_VAR(dword_412790)
#define NEW_HANDLER GAME_VAR(dword_387B4C)
#define KERNEL_ERRNO GAME_VAR(errno)

static inline u8 *P(u32 a) { return RAM + a; }
static inline u32 impure(void) { return rd32(IMPURE_PTR); }
static inline u8 ctype_of(s32 c) { return *P(CTYPE_ENTRIES + (u32)c); }
enum { CT_UPPER = 1, CT_LOWER = 2, CT_DIGIT = 4, CT_SPACE = 8 };

static inline double dbl(u64 b) { double d; memcpy(&d, &b, 8); return d; }
static inline u64 dbits(double d) { u64 b; memcpy(&b, &d, 8); return b; }

static void global_ctors(void) {
    u32 n = rd32(CTORS);
    if (n == 0xFFFFFFFFu)
        for (n = 0; rd32(CTORS + 4 * (n + 1)); n++) {}
    for (; n; n--) game_fn_void(rd32(CTORS + 4 * n));
}
void crt___main(void) {
    if (rd32(MAIN_DONE)) return;
    wr32(MAIN_DONE, 1);
    global_ctors();
}

void crt_inv___register_frame_info_table(u32 table, u32 ob) {
    wr32(ob + 12, table);
    wr32(ob + 20, rd32(FRAME_OBJECTS));
    wr32(FRAME_OBJECTS, ob);
    wr32(ob + 8, table);
    wr32(ob + 4, 0);
    wr32(ob + 0, 0);
    wr32(ob + 16, 0);
}
u32 crt_inv___deregister_frame_info(u32 begin) {
    for (u32 link = FRAME_OBJECTS; rd32(link); link = rd32(link) + 20) {
        u32 ob = rd32(link);
        if (rd32(ob + 8) != begin) continue;
        wr32(link, rd32(ob + 20));
        if (rd32(ob + 0)) crt_freemem(rd32(ob + 12));
        return ob;
    }
    revo_fatal("__deregister_frame_info: %08X not registered (abort)", begin);
}

u32 crt___builtin_new(u32 size) {
    if (!size) size = 1;
    u32 p;
    while (!(p = crt_malloc(size))) {
        u32 handler = rd32(NEW_HANDLER);
        if (!handler) revo_fatal("operator new: out of memory for %u bytes (bad_alloc)", size);
        game_fn_void(handler);
    }
    return p;
}
u32 crt___builtin_delete(u32 p) {
    if (p) crt_freemem(p);
    return 0;
}

float crt___floatdisf(s32 v) {
    s64 u = v;
    if ((u64)(u + ((1ll << 53) - 1)) > (u64)((1ll << 54) - 2) && (u & 0x7FF)) u |= 0x800;
    double hi = (double)(s32)(u >> 32) * 65536.0 * 65536.0;
    double lo = (double)(s32)(u32)u;
    if ((s32)(u32)u < 0) lo += 4294967296.0;
    return (float)(hi + lo);
}
static double dbl_daz(u64 b) { return dbl((b & 0x7FF0000000000000ull) ? b : b & 0x8000000000000000ull); }

static bool tiny_truncated(double hi, double lo, int e, double *out) {
    double min = ldexp(1.0, -1022 - e), unit = ldexp(1.0, -1074 - e);
    if (hi > min || (hi == min && lo >= 0)) return false;
    double t = trunc(hi / unit) * unit;
    if (t == hi && lo < 0) t -= unit;
    *out = ldexp(t, e);
    return true;
}
double crt_dpadd(double a, double b) { return dbl_daz(dbits(a)) + dbl_daz(dbits(b)); }
double crt_dpmul(double a_, double b_) {
    double a = dbl_daz(dbits(a_)), b = dbl_daz(dbits(b_)), r = a * b, t;
    if (isfinite(r) && r != 0 && fabs(r) <= 0x1p-1022) {
        int ea, eb;
        double ma = frexp(fabs(a), &ea), mb = frexp(fabs(b), &eb), hi = ma * mb;
        if (tiny_truncated(hi, fma(ma, mb, -hi), ea + eb, &t)) r = signbit(r) ? -t : t;
    }
    return r;
}
double crt_dpdiv(double a_, double b_) {
    double a = dbl_daz(dbits(a_)), b = dbl_daz(dbits(b_)), r = a / b, t;
    if (isfinite(r) && r != 0 && fabs(r) <= 0x1p-1022 && isfinite(b)) {
        int ea, eb;
        double ma = frexp(fabs(a), &ea), mb = frexp(fabs(b), &eb), hi = ma / mb;
        if (tiny_truncated(hi, fma(-hi, mb, ma), ea - eb, &t)) r = signbit(r) ? -t : t;
    }
    return r;
}
double crt_litodp(s32 v) { return (double)v; }
s32 crt_dptoli(double d) {
    if (isnan(d)) return 0;
    if (d >= 2147483648.0) return 0x7FFFFFFF;
    if (d <= -2147483648.0) return (s32)0x80000000u;
    return (s32)d;
}
u32 crt_fptoui(float f) {
    if (isnan(f) || f <= 0.0f || signbit(f)) return 0;
    if (f >= 4294967296.0f) return 0xFFFFFFFFu;
    return (u32)f;
}

void crt_srand(u32 seed) { wr32(impure() + REENT_RAND_NEXT, seed); }
s32 crt_rand(void) {
    u32 r = impure();
    u32 next = rd32(r + REENT_RAND_NEXT) * 1103515245u + 12345u;
    wr32(r + REENT_RAND_NEXT, next);
    return (s32)(next & 0x7FFFFFFFu);
}

#define ENOMEM 12
extern u32 revo_program_end;
u32 revo_kernel_heap_end(void);

static u32 sbrk_native(s32 increment) {
    static u32 brk;
    if (!brk) brk = revo_program_end;
    if (brk + (u32)increment > revo_kernel_heap_end()) {
        wr32(KERNEL_ERRNO, ENOMEM);
        return 0xFFFFFFFFu;
    }
    u32 old = brk;
    brk += (u32)increment;
    return old;
}

u32 crt_sbrk_r(u32 reent, s32 increment) {
    wr32(KERNEL_ERRNO, 0);
    u32 p = sbrk_native(increment);
    if (p == 0xFFFFFFFFu && rd32(KERNEL_ERRNO)) wr32(reent + REENT_ERRNO, rd32(KERNEL_ERRNO));
    return p;
}

static s64 strtol_at(u32 nptr, u32 endptr, int base) {
    u32 s = nptr;
    s32 c;
    do c = (s8)*P(s++); while (ctype_of(c) & CT_SPACE);
    bool neg = false;
    if (c == '-') { neg = true; c = (s8)*P(s++); }
    else if (c == '+') c = (s8)*P(s++);
    if ((base == 0 || base == 16) && c == '0' && (*P(s) == 'x' || *P(s) == 'X')) {
        c = (s8)P(s)[1];
        s += 2;
        base = 16;
    }
    if (base == 0) base = c == '0' ? 8 : 10;
    u64 cutoff = neg ? (u64)INT64_MAX + 1 : (u64)INT64_MAX;
    int cutlim = (int)(cutoff % (u64)base);
    cutoff /= (u64)base;
    u64 acc = 0;
    int any = 0;
    for (;; c = (s8)*P(s++)) {
        u8 t = ctype_of(c);
        if (t & CT_DIGIT) c -= '0';
        else if (t & (CT_UPPER | CT_LOWER)) c -= t & CT_UPPER ? 'A' - 10 : 'a' - 10;
        else break;
        if (c >= base) break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) any = -1;
        else { any = 1; acc = acc * (u64)base + (u64)c; }
    }
    s64 v;
    if (any < 0) {
        v = neg ? INT64_MIN : INT64_MAX;
        wr32(impure() + REENT_ERRNO, 34 );
    } else v = neg ? (s64)(0 - acc) : (s64)acc;
    if (endptr) wr32(endptr, any ? s - 1 : nptr);
    return v;
}
s32 crt_atol(u32 s) { return (s32)strtol_at(s, 0, 10); }
s32 crt_atoi(u32 s) { return (s32)strtol_at(s, 0, 10); }

static u32 strtok_at(u32 s, u32 delim, u32 last) {
    if (!s && !(s = rd32(last))) return 0;
    u8 c;
    for (;;) {
        c = *P(s++);
        u32 sp = delim;
        u8 sc;
        while ((sc = *P(sp++)) && sc != c) {}
        if (!sc) break;
    }
    if (!c) { wr32(last, 0); return 0; }
    u32 tok = s - 1;
    for (;;) {
        c = *P(s++);
        u32 sp = delim;
        u8 sc;
        do {
            if ((sc = *P(sp++)) == c) {
                if (!c) s = 0;
                else *P(s - 1) = 0;
                wr32(last, s);
                return tok;
            }
        } while (sc);
    }
}
u32 crt_strtok(u32 s, u32 delim) { return strtok_at(s, delim, impure() + REENT_STRTOK_LAST); }

static u32 g_qsort_cmp;
static int cmp_at(u32 a, u32 b) {
    return game_fn_compare(g_qsort_cmp, a, b);
}
static void swap_bytes(u32 a, u32 b, u32 n) {
    u8 *pa = P(a), *pb = P(b);
    for (u32 i = 0; i < n; i++) { u8 t = pa[i]; pa[i] = pb[i]; pb[i] = t; }
}
static u32 med3(u32 a, u32 b, u32 c) {
    return cmp_at(a, b) < 0 ? (cmp_at(b, c) < 0 ? b : cmp_at(a, c) < 0 ? c : a)
                            : (cmp_at(b, c) > 0 ? b : cmp_at(a, c) < 0 ? a : c);
}
static void insertion(u32 a, u32 n, u32 es) {
    for (u32 pm = a + es; pm < a + n * es; pm += es)
        for (u32 pl = pm; pl > a && cmp_at(pl - es, pl) > 0; pl -= es) swap_bytes(pl, pl - es, es);
}
static void bsd_qsort(u32 a, u32 n, u32 es) {
    for (;;) {
        if (n < 7) { insertion(a, n, es); return; }
        u32 pm = a + (n / 2) * es;
        if (n > 7) {
            u32 pl = a, pn = a + (n - 1) * es;
            if (n > 40) {
                u32 d = (n / 8) * es;
                pl = med3(pl, pl + d, pl + 2 * d);
                pm = med3(pm - d, pm, pm + d);
                pn = med3(pn - 2 * d, pn - d, pn);
            }
            pm = med3(pl, pm, pn);
        }
        swap_bytes(a, pm, es);
        u32 pa = a + es, pb = pa, pc = a + (n - 1) * es, pd = pc;
        bool swapped = false;
        int r;
        for (;;) {
            while (pb <= pc && (r = cmp_at(pb, a)) <= 0) {
                if (r == 0) { swapped = true; swap_bytes(pa, pb, es); pa += es; }
                pb += es;
            }
            while (pb <= pc && (r = cmp_at(pc, a)) >= 0) {
                if (r == 0) { swapped = true; swap_bytes(pc, pd, es); pd -= es; }
                pc -= es;
            }
            if (pb > pc) break;
            swap_bytes(pb, pc, es);
            swapped = true;
            pb += es;
            pc -= es;
        }
        if (!swapped) { insertion(a, n, es); return; }
        u32 pn = a + n * es;
        u32 k = pa - a < pb - pa ? pa - a : pb - pa;
        if (k) swap_bytes(a, pb - k, k);
        k = pd - pc < pn - pd - es ? pd - pc : pn - pd - es;
        if (k) swap_bytes(pb, pn - k, k);
        if ((k = pb - pa) > es) bsd_qsort(a, k / es, es);
        if ((k = pd - pc) <= es) return;
        a = pn - k;
        n = k / es;
    }
}
void crt_qsort(u32 base, u32 n, u32 size, u32 compare) {
    u32 save = g_qsort_cmp;
    g_qsort_cmp = compare;
    bsd_qsort(base, n, size);
    g_qsort_cmp = save;
}

typedef struct { u32 at; va_list *va; } Args;
static u64 next_arg(Args *a, bool eight) {
    if (a->va) {
        u64 v = va_arg(*a->va, u64);
        return eight ? v : (u64)(s64)(s32)v;
    }
    if (!eight) {
        u64 v = (u64)(s64)(s32)rd32(a->at);
        a->at += 4;
        return v;
    }
    a->at = (a->at + 7) & ~7u;
    u64 v = rd64(a->at);
    a->at += 8;
    return v;
}

typedef struct { char *out; size_t len, cap; } Out;
static void put(Out *o, const char *s, size_t n) {
    if (o->len + n + 1 > o->cap) {
        o->cap = (o->len + n + 1) * 2;
        o->out = realloc(o->out, o->cap);
    }
    memcpy(o->out + o->len, s, n);
    o->len += n;
    o->out[o->len] = 0;
}

static void put_char(Out *o, char c, int n) {
    for (int i = 0; i < n; i++) put(o, &c, 1);
}
static void putf(Out *o, const char *spec, ...) {
    va_list ap, ap2;
    va_start(ap, spec);
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, spec, ap);
    va_end(ap);
    if (n > 0) {
        char *t = malloc((size_t)n + 1);
        vsnprintf(t, (size_t)n + 1, spec, ap2);
        put(o, t, (size_t)n);
        free(t);
    }
    va_end(ap2);
}

static size_t format(u32 dst, u32 fmt, Args *args) {
    Out o = {0};
    put(&o, "", 0);
    u32 f = fmt;
    for (;;) {
        u32 start = f;
        while (*P(f) && *P(f) != '%') f++;
        put(&o, (const char *)P(start), f - start);
        if (!*P(f)) break;
        f++;
        char flags[8];
        int nflags = 0, width = -1, prec = -1, longs = 0, shorts = 0;
        for (;; f++) {
            char c = (char)*P(f);
            if (c == '-' || c == '+' || c == ' ' || c == '#' || c == '0') {
                if (nflags < 7 && !memchr(flags, c, (size_t)nflags)) flags[nflags++] = c;
            } else break;
        }
        if (*P(f) == '*') {
            width = (s32)next_arg(args, false);
            if (width < 0) { width = -width; if (nflags < 7) flags[nflags++] = '-'; }
            f++;
        } else if (*P(f) >= '0' && *P(f) <= '9') {
            for (width = 0; *P(f) >= '0' && *P(f) <= '9'; f++) width = width * 10 + (*P(f) - '0');
        }
        if (*P(f) == '.') {
            f++;
            if (*P(f) == '*') { prec = (s32)next_arg(args, false); f++; if (prec < 0) prec = -1; }
            else for (prec = 0; *P(f) >= '0' && *P(f) <= '9'; f++) prec = prec * 10 + (*P(f) - '0');
        }
        for (;; f++) {
            char c = (char)*P(f);
            if (c == 'l') longs++;
            else if (c == 'q' || c == 'L') longs = 2;
            else if (c == 'h') shorts = 1;
            else break;
        }
        char conv = (char)*P(f);
        if (!conv) break;
        f++;
        if (conv == 'D' || conv == 'O' || conv == 'U') { longs = 1; conv = (char)(conv + ('a' - 'A')); }
        char spec[32];
        int k = 0;
        spec[k++] = '%';
        for (int i = 0; i < nflags; i++) spec[k++] = flags[i];
        spec[k] = 0;
        if (width >= 0) k += snprintf(spec + k, sizeof spec - (size_t)k, "%d", width);
        if (prec >= 0) k += snprintf(spec + k, sizeof spec - (size_t)k, ".%d", prec);
        switch (conv) {
        case 'd': case 'i': {
            u64 slot = next_arg(args, longs >= 2);
            long long v = longs == 1 ? (long long)slot : shorts ? (short)slot : (int)(s32)slot;
            snprintf(spec + k, sizeof spec - (size_t)k, "lld");
            putf(&o, spec, v);
            continue;
        }
        case 'u': case 'o': case 'x': case 'X': {
            u64 slot = next_arg(args, longs >= 2);
            unsigned long long v = longs == 1 ? slot : shorts ? (u16)slot : (u32)slot;
            snprintf(spec + k, sizeof spec - (size_t)k, "ll%c", conv);
            putf(&o, spec, v);
            continue;
        }
        case 'p': {
            unsigned long long v = (unsigned long long)(s64)(s32)next_arg(args, false);
            char digits[40];
            int nd = prec >= 0 ? snprintf(digits, sizeof digits, "%.*llx", prec > 30 ? 30 : prec, v)
                               : snprintf(digits, sizeof digits, "%llx", v);
            int pad = width > nd + 2 ? width - nd - 2 : 0;
            if (memchr(flags, '-', (size_t)nflags)) putf(&o, "0x%s%*s", digits, pad, "");
            else if (memchr(flags, '0', (size_t)nflags) && prec < 0) put(&o, "0x", 2), put_char(&o, '0', pad), put(&o, digits, (size_t)nd);
            else putf(&o, "%*s0x%s", pad, "", digits);
            continue;
        }
        case 'c': {
            spec[k++] = 'c';
            spec[k] = 0;
            putf(&o, spec, (int)(u8)next_arg(args, false));
            continue;
        }
        case 's': {
            u32 a = (u32)next_arg(args, false);
            const char *s = a ? (const char *)P(a) : "(null)";
            spec[k++] = 's';
            spec[k] = 0;
            putf(&o, spec, s);
            continue;
        }
        case 'e': case 'E': case 'f': case 'g': case 'G': {
            double d = dbl_daz(next_arg(args, true));
            if (isinf(d) || isnan(d)) {
                const char *sign = isinf(d) && d < 0 ? "-" : memchr(flags, '+', (size_t)nflags) ? "+"
                                   : memchr(flags, ' ', (size_t)nflags) ? " " : "";
                const char *text = isinf(d) ? "Inf" : "NaN";
                int pad = width - (int)strlen(sign) - 3;
                if (pad < 0) pad = 0;
                if (memchr(flags, '-', (size_t)nflags)) putf(&o, "%s%s%*s", sign, text, pad, "");
                else if (memchr(flags, '0', (size_t)nflags)) put(&o, sign, strlen(sign)), put_char(&o, '0', pad), put(&o, text, 3);
                else putf(&o, "%*s%s%s", pad, "", sign, text);
                continue;
            }
            if (conv == 'f' && prec == 0 && d != 0 && fabs(d) <= 0.5) {
                memmove(spec + 2, spec + 1, (size_t)k);
                spec[1] = '#';
                k++;
            }
            spec[k++] = conv;
            spec[k] = 0;
            putf(&o, spec, d);
            continue;
        }
        case 'n': {
            u32 a = (u32)next_arg(args, false);
            if (longs == 1) wr64(a, (u64)(s64)o.len);
            else if (shorts) { u16 v = (u16)o.len; memcpy(P(a), &v, 2); }
            else wr32(a, (u32)o.len);
            continue;
        }
        case '%':
            spec[k++] = 'c';
            spec[k] = 0;
            putf(&o, spec, '%');
            continue;
        default: put(&o, &conv, 1); continue;
        }
    }
    memcpy(P(dst), o.out, o.len + 1);
    size_t len = o.len;
    free(o.out);
    return len;
}
s32 crt_sprintf(u32 dst, u32 fmt, u32 args) {
    Args a = {args};
    return (s32)format(dst, fmt, &a);
}
s32 crt_vsprintf_native(u32 dst, u32 fmt, va_list *va) {
    Args a = {0, va};
    return (s32)format(dst, fmt, &a);
}
