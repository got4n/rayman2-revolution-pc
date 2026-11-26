#ifndef REVO_MMI_OPS_H
#define REVO_MMI_OPS_H

typedef struct { unsigned long long lo, hi; } RevoQ;

static inline unsigned revo_q_w(RevoQ q, int i) { return (unsigned)((i < 2 ? q.lo : q.hi) >> (32 * (i & 1))); }
static inline unsigned short revo_q_h(RevoQ q, int i) { return (unsigned short)((i < 4 ? q.lo : q.hi) >> (16 * (i & 3))); }
static inline unsigned char revo_q_b(RevoQ q, int i) { return (unsigned char)((i < 8 ? q.lo : q.hi) >> (8 * (i & 7))); }
static inline void revo_set_w(RevoQ *q, int i, unsigned v) {
    unsigned long long *p = i < 2 ? &q->lo : &q->hi; int s = 32 * (i & 1);
    *p = (*p & ~(0xFFFFFFFFull << s)) | ((unsigned long long)v << s);
}
static inline void revo_set_h(RevoQ *q, int i, unsigned short v) {
    unsigned long long *p = i < 4 ? &q->lo : &q->hi; int s = 16 * (i & 3);
    *p = (*p & ~(0xFFFFull << s)) | ((unsigned long long)v << s);
}
static inline void revo_set_b(RevoQ *q, int i, unsigned char v) {
    unsigned long long *p = i < 8 ? &q->lo : &q->hi; int s = 8 * (i & 7);
    *p = (*p & ~(0xFFull << s)) | ((unsigned long long)v << s);
}

static inline int revo_mmi(const char *op, RevoQ s, RevoQ t, int sa, RevoQ *out) {
    RevoQ r = {0, 0};
    int i;
#define REVO_IS(name) (__builtin_strcmp(op, name) == 0)
    if (REVO_IS("pcpyld")) { r.lo = t.lo; r.hi = s.lo; }
    else if (REVO_IS("pcpyud")) { r.lo = s.hi; r.hi = t.hi; }
    else if (REVO_IS("pcpyh")) {
        for (i = 0; i < 4; i++) { revo_set_h(&r, i, revo_q_h(t, 0)); revo_set_h(&r, 4 + i, revo_q_h(t, 4)); }
    }
    else if (REVO_IS("por")) { r.lo = s.lo | t.lo; r.hi = s.hi | t.hi; }
    else if (REVO_IS("pand")) { r.lo = s.lo & t.lo; r.hi = s.hi & t.hi; }
    else if (REVO_IS("pxor")) { r.lo = s.lo ^ t.lo; r.hi = s.hi ^ t.hi; }
    else if (REVO_IS("pnor")) { r.lo = ~(s.lo | t.lo); r.hi = ~(s.hi | t.hi); }
    else if (REVO_IS("paddw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, revo_q_w(s, i) + revo_q_w(t, i)); }
    else if (REVO_IS("psubw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, revo_q_w(s, i) - revo_q_w(t, i)); }
    else if (REVO_IS("paddh")) { for (i = 0; i < 8; i++) revo_set_h(&r, i, revo_q_h(s, i) + revo_q_h(t, i)); }
    else if (REVO_IS("psubh")) { for (i = 0; i < 8; i++) revo_set_h(&r, i, revo_q_h(s, i) - revo_q_h(t, i)); }
    else if (REVO_IS("paddb")) { for (i = 0; i < 16; i++) revo_set_b(&r, i, revo_q_b(s, i) + revo_q_b(t, i)); }
    else if (REVO_IS("psubb")) { for (i = 0; i < 16; i++) revo_set_b(&r, i, revo_q_b(s, i) - revo_q_b(t, i)); }
    else if (REVO_IS("pcgth")) { for (i = 0; i < 8; i++) revo_set_h(&r, i, (short)revo_q_h(s, i) > (short)revo_q_h(t, i) ? 0xFFFF : 0); }
    else if (REVO_IS("pcgtw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, (int)revo_q_w(s, i) > (int)revo_q_w(t, i) ? 0xFFFFFFFFu : 0); }
    else if (REVO_IS("pceqw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, revo_q_w(s, i) == revo_q_w(t, i) ? 0xFFFFFFFFu : 0); }
    else if (REVO_IS("pminh")) { for (i = 0; i < 8; i++) { short a = revo_q_h(s, i), b = revo_q_h(t, i); revo_set_h(&r, i, a < b ? a : b); } }
    else if (REVO_IS("pmaxh")) { for (i = 0; i < 8; i++) { short a = revo_q_h(s, i), b = revo_q_h(t, i); revo_set_h(&r, i, a > b ? a : b); } }
    else if (REVO_IS("pminw")) { for (i = 0; i < 4; i++) { int a = revo_q_w(s, i), b = revo_q_w(t, i); revo_set_w(&r, i, a < b ? a : b); } }
    else if (REVO_IS("pmaxw")) { for (i = 0; i < 4; i++) { int a = revo_q_w(s, i), b = revo_q_w(t, i); revo_set_w(&r, i, a > b ? a : b); } }
    else if (REVO_IS("pextlb")) { for (i = 0; i < 8; i++) { revo_set_b(&r, 2 * i, revo_q_b(t, i)); revo_set_b(&r, 2 * i + 1, revo_q_b(s, i)); } }
    else if (REVO_IS("pextub")) { for (i = 0; i < 8; i++) { revo_set_b(&r, 2 * i, revo_q_b(t, 8 + i)); revo_set_b(&r, 2 * i + 1, revo_q_b(s, 8 + i)); } }
    else if (REVO_IS("pextlh")) { for (i = 0; i < 4; i++) { revo_set_h(&r, 2 * i, revo_q_h(t, i)); revo_set_h(&r, 2 * i + 1, revo_q_h(s, i)); } }
    else if (REVO_IS("pextuh")) { for (i = 0; i < 4; i++) { revo_set_h(&r, 2 * i, revo_q_h(t, 4 + i)); revo_set_h(&r, 2 * i + 1, revo_q_h(s, 4 + i)); } }
    else if (REVO_IS("pextlw")) { for (i = 0; i < 2; i++) { revo_set_w(&r, 2 * i, revo_q_w(t, i)); revo_set_w(&r, 2 * i + 1, revo_q_w(s, i)); } }
    else if (REVO_IS("pextuw")) { for (i = 0; i < 2; i++) { revo_set_w(&r, 2 * i, revo_q_w(t, 2 + i)); revo_set_w(&r, 2 * i + 1, revo_q_w(s, 2 + i)); } }
    else if (REVO_IS("ppacb")) { for (i = 0; i < 8; i++) { revo_set_b(&r, i, revo_q_b(t, 2 * i)); revo_set_b(&r, 8 + i, revo_q_b(s, 2 * i)); } }
    else if (REVO_IS("ppach")) { for (i = 0; i < 4; i++) { revo_set_h(&r, i, revo_q_h(t, 2 * i)); revo_set_h(&r, 4 + i, revo_q_h(s, 2 * i)); } }
    else if (REVO_IS("ppacw")) { revo_set_w(&r, 0, revo_q_w(t, 0)); revo_set_w(&r, 1, revo_q_w(t, 2)); revo_set_w(&r, 2, revo_q_w(s, 0)); revo_set_w(&r, 3, revo_q_w(s, 2)); }
    else if (REVO_IS("pexew")) { revo_set_w(&r, 0, revo_q_w(t, 2)); revo_set_w(&r, 1, revo_q_w(t, 1)); revo_set_w(&r, 2, revo_q_w(t, 0)); revo_set_w(&r, 3, revo_q_w(t, 3)); }
    else if (REVO_IS("pexcw")) { revo_set_w(&r, 0, revo_q_w(t, 0)); revo_set_w(&r, 1, revo_q_w(t, 2)); revo_set_w(&r, 2, revo_q_w(t, 1)); revo_set_w(&r, 3, revo_q_w(t, 3)); }
    else if (REVO_IS("prot3w")) { revo_set_w(&r, 0, revo_q_w(t, 1)); revo_set_w(&r, 1, revo_q_w(t, 2)); revo_set_w(&r, 2, revo_q_w(t, 0)); revo_set_w(&r, 3, revo_q_w(t, 3)); }
    else if (REVO_IS("psllh")) { for (i = 0; i < 8; i++) revo_set_h(&r, i, revo_q_h(t, i) << (sa & 15)); }
    else if (REVO_IS("psrlh")) { for (i = 0; i < 8; i++) revo_set_h(&r, i, revo_q_h(t, i) >> (sa & 15)); }
    else if (REVO_IS("psrah")) { for (i = 0; i < 8; i++) revo_set_h(&r, i, (short)revo_q_h(t, i) >> (sa & 15)); }
    else if (REVO_IS("psllw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, revo_q_w(t, i) << sa); }
    else if (REVO_IS("psrlw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, revo_q_w(t, i) >> sa); }
    else if (REVO_IS("psraw")) { for (i = 0; i < 4; i++) revo_set_w(&r, i, (int)revo_q_w(t, i) >> sa); }
    else if (REVO_IS("plzcw")) {
        for (i = 0; i < 2; i++) {
            unsigned v = revo_q_w(s, i);
            if (v & 0x80000000u) v = ~v;
            revo_set_w(&r, i, v ? __builtin_clz(v) - 1 : 31);
        }
    }
    else if (REVO_IS("qfsrv")) {
        int sh = sa & 127;
        RevoQ lo = t, hi = s;
        if (sh == 0) r = lo;
        else if (sh < 64) { r.lo = (lo.lo >> sh) | (lo.hi << (64 - sh)); r.hi = (lo.hi >> sh) | (hi.lo << (64 - sh)); }
        else if (sh == 64) { r.lo = lo.hi; r.hi = hi.lo; }
        else { int k = sh - 64; r.lo = (lo.hi >> k) | (hi.lo << (64 - k)); r.hi = (hi.lo >> k) | (hi.hi << (64 - k)); }
    }
    else return 0;
#undef REVO_IS
    *out = r;
    return 1;
}

#endif
