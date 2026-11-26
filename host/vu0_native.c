#include "game_vars.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "PC.h"
#include "rt.h"
#include "runtime.h"
#include "vu0_native.h"

#pragma STDC FP_CONTRACT OFF

typedef float Vec[4];
enum { X = 8, Y = 4, Z = 2, W = 1, XYZ = 14, XYZW = 15 };

static Vec r[32];
static Vec acc;
static float q;
static u32 vi4, vi5, vi6, vi13;
static const Vec VF0 = {0.0f, 0.0f, 0.0f, 1.0f};

static inline u32 bits(float f) { union { float f; u32 u; } x = {f}; return x.u; }
static inline float from_bits(u32 u) { union { u32 u; float f; } x = {u}; return x.f; }

static inline float clampf(float f) {
    u32 u = bits(f), e = u & 0x7F800000u;
    if (e == 0x7F800000u) return from_bits((u & 0x80000000u) | 0x7F7FFFFFu);
    if (e == 0) return from_bits(u & 0x80000000u);
    return f;
}

static inline int lane(int m, int k) { return m & (8 >> k); }

static void put(Vec d, const float t[4], int m) {
    for (int k = 0; k < 4; k++)
        if (lane(m, k)) d[k] = t[k];
}

static void add(Vec d, const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] + b[k]); put(d, t, m); }
static void sub(Vec d, const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] - b[k]); put(d, t, m); }
static void mul(Vec d, const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] * b[k]); put(d, t, m); }
static void maxv(Vec d, const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] > b[k] ? a[k] : b[k]); put(d, t, m); }
static void minv(Vec d, const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] < b[k] ? a[k] : b[k]); put(d, t, m); }

static void adds(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] + s); put(d, t, m); }
static void subs(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] - s); put(d, t, m); }
static void muls(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] * s); put(d, t, m); }
static void maxs(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] > s ? a[k] : s); put(d, t, m); }
static void mins(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] < s ? a[k] : s); put(d, t, m); }

static void adda(const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] + b[k]); put(acc, t, m); }
static void suba(const Vec a, const Vec b, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] - b[k]); put(acc, t, m); }
static void mulas(const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(a[k] * s); put(acc, t, m); }
static void maddas(const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(acc[k] + a[k] * s); put(acc, t, m); }
static void madds(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(acc[k] + a[k] * s); put(d, t, m); }
static void msubs(Vec d, const Vec a, float s, int m) { float t[4]; for (int k = 0; k < 4; k++) t[k] = clampf(acc[k] - a[k] * s); put(d, t, m); }

static void opmula(const Vec a, const Vec b) {
    const float t[4] = {clampf(a[1] * b[2]), clampf(a[2] * b[0]), clampf(a[0] * b[1]), 0.0f};
    put(acc, t, XYZ);
}
static void opmsub(Vec d, const Vec a, const Vec b) {
    const float t[4] = {clampf(acc[0] - a[1] * b[2]), clampf(acc[1] - a[2] * b[0]), clampf(acc[2] - a[0] * b[1]), 0.0f};
    put(d, t, XYZ);
}

static void move(Vec d, const Vec s, int m) { put(d, s, m); }
static void copy(Vec d, const Vec s) { memcpy(d, s, sizeof(Vec)); }

static float vdiv(float s, float t) {
    if (t == 0.0f) return from_bits(((bits(s) ^ bits(t)) & 0x80000000u) | 0x7F7FFFFFu);
    return clampf(s / t);
}
static float vsqrt(float t) { return sqrtf(fabsf(t)); }
static float vrsqrt(float s, float t) {
    const float d = sqrtf(fabsf(t));
    if (d == 0.0f) return from_bits(((bits(s) ^ bits(t)) & 0x80000000u) | 0x7F7FFFFFu);
    return clampf(s / d);
}

static int neg(const Vec t, int m) {
    for (int k = 0; k < 4; k++)
        if (lane(m, k) && (bits(t[k]) & 0x80000000u)) return 1;
    return 0;
}
static int zero(const Vec t, int m) {
    for (int k = 0; k < 4; k++)
        if (lane(m, k) && !(bits(t[k]) & 0x7FFFFFFFu)) return 1;
    return 0;
}

static int sum13(void) {
    adds(r[13], r[13], r[13][1], X);
    const int s = neg(r[13], X);
    adds(r[13], r[13], r[13][2], X);
    return s;
}

static void triangle_plane(void) {
    sub(r[7], r[4], r[3], XYZW);
    sub(r[8], r[5], r[3], XYZW);
    opmula(r[7], r[8]);
    opmsub(r[12], r[8], r[7]);
    mul(r[8], r[12], r[12], XYZW);
    adds(r[7], r[8], r[8][2], X);
    adds(r[7], r[7], r[8][1], X);
    q = vrsqrt(VF0[3], r[7][0]);
    muls(r[12], r[12], q, XYZW);
    mul(r[13], r[12], r[3], XYZW);
    sum13();
    sub(r[6], VF0, r[13], X);
}

static int point_in_triangle(void) {
    Vec keep[6];
    copy(keep[0], r[20]); copy(keep[1], r[21]); copy(keep[2], r[22]);
    copy(keep[3], r[16]); copy(keep[4], r[17]); copy(keep[5], r[18]);
    sub(r[20], r[3], r[14], XYZW);
    sub(r[21], r[4], r[14], XYZW);
    sub(r[22], r[5], r[14], XYZW);
    opmula(r[20], r[21]);
    opmsub(r[16], r[21], r[20]);
    opmula(r[21], r[22]);
    opmsub(r[17], r[22], r[21]);
    opmula(r[22], r[20]);
    opmsub(r[18], r[20], r[22]);
    int inside = 0;
    mul(r[13], r[16], r[12], XYZW);
    sum13();
    adds(r[6], r[19], r[13][0], W);
    mul(r[13], r[17], r[12], XYZW);
    if (!neg(r[6], W) && !zero(r[6], W)) {
        sum13();
        adds(r[6], r[19], r[13][0], W);
        mul(r[13], r[18], r[12], XYZW);
        if (!neg(r[6], W) && !zero(r[6], W)) {
            sum13();
            adds(r[6], r[19], r[13][0], W);
            inside = !neg(r[6], W) && !zero(r[6], W);
        }
    }
    copy(r[18], keep[5]); copy(r[17], keep[4]); copy(r[16], keep[3]);
    copy(r[22], keep[2]); copy(r[21], keep[1]); copy(r[20], keep[0]);
    return inside;
}

static int segment_through_triangle(void) {
    Vec keep[5];
    copy(keep[0], r[7]); copy(keep[1], r[10]); copy(keep[2], r[11]); copy(keep[3], r[8]); copy(keep[4], r[9]);
    sub(r[19], r[19], r[19], XYZW);
    add(r[7], r[1], r[2], XYZW);
    minv(r[10], r[3], r[4], XYZW);
    maxv(r[11], r[3], r[4], XYZW);
    minv(r[8], r[1], r[7], XYZW);
    maxv(r[9], r[1], r[7], XYZW);
    minv(r[10], r[10], r[5], XYZW);
    maxv(r[11], r[11], r[5], XYZW);
    int through = 0;
    Vec t;
    sub(t, r[11], r[8], XYZW);
    const int apart_low = neg(t, XYZW);
    sub(t, r[9], r[10], XYZW);
    const int apart_high = neg(t, XYZW);
    sub(r[15], r[19], r[6], Y);
    if (!apart_low && !apart_high) {
        mul(r[13], r[12], r[1], XYZW);
        sum13();
        adds(r[6], r[19], r[13][0], Z);
        adds(r[6], r[6], r[6][0], Z);
        q = vdiv(r[6][2], r[15][1]);
        sub(r[13], r[19], r[6], Z);
        adds(r[15], r[19], q, X);
        adda(r[19], r[1], XYZW);
        madds(r[14], r[2], r[15][0], XYZW);
        sub(t, r[19], r[6], Z);
        const int start_front = neg(t, Z) || zero(t, Z);
        adds(t, r[6], r[6][1], Z);
        const int end_back = neg(t, Z) || zero(t, Z);
        if (start_front && end_back) through = point_in_triangle();
    }
    copy(r[9], keep[4]); copy(r[8], keep[3]); copy(r[11], keep[2]); copy(r[10], keep[1]); copy(r[7], keep[0]);
    return through;
}

static int segment_hits_triangle(void) {
    mul(r[13], r[12], r[2], XYZW);
    adds(r[13], r[13], r[13][1], X);
    adds(r[13], r[13], r[13][2], X);
    adds(r[6], VF0, r[13][0], Y);
    return segment_through_triangle();
}

static void transform13(void) {
    mulas(r[20], r[13][0], XYZW);
    maddas(r[21], r[13][1], XYZW);
    maddas(r[22], r[13][2], XYZW);
    madds(r[13], r[23], VF0[3], XYZW);
}

static void rotate13(void) {
    mulas(r[20], r[13][0], XYZ);
    maddas(r[21], r[13][1], XYZ);
    madds(r[13], r[22], r[13][2], XYZ);
}

static void rotate13_scaled(void) {
    muls(r[28], r[20], r[26][3], XYZW);
    muls(r[29], r[21], r[26][3], XYZW);
    muls(r[30], r[22], r[26][3], XYZW);
    mulas(r[28], r[13][0], XYZ);
    maddas(r[29], r[13][1], XYZ);
    madds(r[13], r[30], r[13][2], XYZ);
}

static int quadratic(void) {
    add(r[3], r[1], r[1], X);
    muls(r[2], r[1], r[1][0], Z);
    Vec t;
    add(t, r[1], VF0, X);
    const int a_zero = zero(t, X);
    sub(r[3], VF0, r[1], Y);
    mul(r[4], r[1], r[1], Y);
    q = vdiv(VF0[3], r[3][0]);
    muls(r[2], r[2], 4.0f, Z);
    if (a_zero) {
        add(t, r[1], VF0, Y);
        q = vdiv(r[1][2], r[1][1]);
        if (zero(t, Y)) return 0;
        subs(r[1], VF0, q, X);
        subs(r[1], VF0, q, Y);
        return 0;
    }
    subs(r[2], r[4], r[2][2], Y);
    adds(r[3], VF0, q, X);
    const int d_neg = neg(r[2], Y), d_zero = zero(r[2], Y);
    q = vsqrt(r[2][1]);
    if (!d_neg) {
        adds(r[4], r[3], q, Y);
        subs(r[3], r[3], q, Y);
        muls(r[1], r[3], r[4][1], X);
        muls(r[1], r[3], r[3][0], Y);
        return 2;
    }
    if (d_zero) {
        muls(r[1], r[3], r[3][1], X);
        adds(r[1], VF0, r[1][0], Y);
    }
    return 0;
}

static int edge_near_sphere(void) {
    mul(r[13], r[2], r[2], XYZW);
    sub(r[5], r[1], r[3], XYZW);
    mul(r[6], r[4], r[4], X);
    mul(r[9], r[5], r[5], XYZW);
    sum13();
    adds(r[6], VF0, r[13][0], Y);
    mul(r[13], r[5], r[2], XYZW);
    sum13();
    adds(r[7], VF0, r[13][0], Y);
    add(r[14], r[1], r[2], XYZW);
    move(r[13], r[9], XYZW);
    sum13();
    Vec t;
    add(t, r[7], VF0, Y);
    const int before_start = neg(t, Y);
    sub(r[14], r[14], r[3], XYZW);
    adds(r[24], VF0, r[13][0], Y);
    mul(r[13], r[14], r[14], XYZW);
    sum13();
    add(t, r[7], r[6], Y);
    const int past_end = neg(t, Y);
    adds(r[7], VF0, r[13][0], Z);
    mins(r[7], r[7], r[24][1], Z);
    opmula(r[5], r[2]);
    opmsub(r[13], r[2], r[5]);
    mul(r[13], r[13], r[13], XYZW);
    const int s = sum13();
    muls(r[6], r[6], r[6][1], X);
    vi4 = s ? 2 : 0;
    sub(t, r[13], r[6], X);
    const int closer = neg(t, X);
    if (vi4) return 1;
    if (!before_start) return 0;
    if (past_end) return 0;
    return closer;
}

static int edge_cylinder(void) {
    mul(r[13], r[4], r[4], XYZW);
    sub(r[10], r[1], r[3], XYZW);
    mul(r[5], r[5], r[5], X);
    move(r[15], r[1], XYZW);
    sum13();
    add(r[11], VF0, r[13], X);
    const int no_edge = zero(r[11], X);
    mul(r[13], r[4], r[2], XYZW);
    adds(r[16], VF0, 1.0f, X);
    adds(r[5], VF0, r[5][0], Z);
    q = vdiv(r[16][0], r[11][0]);
    if (no_edge) return 0;
    sum13();
    adds(r[11], VF0, r[13][0], Y);
    mul(r[13], r[4], r[10], XYZW);
    sum13();
    adds(r[11], VF0, r[13][0], Z);
    adds(r[11], VF0, q, X);
    Vec keep2, keep3, keep4;
    copy(keep2, r[2]); copy(keep3, r[3]); copy(keep4, r[4]);
    muls(r[12], r[11], r[11][0], Y);
    muls(r[9], r[4], r[12][1], XYZ);
    muls(r[12], r[11], r[11][0], Z);
    sub(r[9], r[9], r[2], XYZW);
    const int along = zero(r[9], X) && zero(r[9], Y) && zero(r[9], Z);
    muls(r[14], r[4], r[12][2], XYZW);
    if (along) return 0;
    sub(r[14], r[14], r[10], XYZW);
    mul(r[13], r[9], r[9], XYZW);
    sum13();
    add(r[1], VF0, r[13], X);
    mul(r[13], r[9], r[14], XYZW);
    sum13();
    adds(r[1], VF0, r[13][0], Y);
    mul(r[13], r[14], r[14], XYZW);
    sum13();
    add(r[1], r[1], r[1], Y);
    subs(r[1], r[5], r[13][0], Z);
    sub(r[1], VF0, r[1], Z);
    if (!quadratic()) return 0;
    mins(r[12], r[1], r[1][1], X);
    copy(r[4], keep4);
    Vec t;
    sub(t, r[12], VF0, X);
    const int not_ahead = neg(t, X) || zero(t, X);
    copy(r[3], keep3);
    copy(r[2], keep2);
    sub(t, r[12], r[16], X);
    const int within = neg(t, X);
    adda(VF0, r[11], XYZW);
    madds(r[7], r[2], r[12][0], XYZ);
    mulas(r[12], r[11][1], X);
    if (not_ahead) return 0;
    maddas(r[16], r[11][2], X);
    madds(r[17], VF0, 1.0f, X);
    mul(r[17], r[17], r[11], X);
    if (!within) return 0;
    sub(t, r[17], r[16], X);
    const int below_end = neg(t, X);
    const int before_line = neg(r[17], X);
    adda(VF0, r[3], XYZW);
    madds(r[8], r[4], r[17][0], XYZ);
    if (before_line) return 0;
    return below_end;
}

static void edge_contact(void) {
    muls(r[28], r[26], r[26][2], Y);
    sum13();
    add(r[28], VF0, r[13], X);
    mul(r[13], r[15], r[15], XYZW);
    sum13();
    q = vdiv(r[28][0], r[13][0]);
    add(r[13], VF0, r[12], XYZW);
    move(r[31], r[28], XYZW);
    rotate13_scaled();
    adda(VF0, r[8], XYZW);
    move(r[28], r[31], XYZW);
    add(r[24], VF0, r[13], XYZW);
    adds(r[28], VF0, q, X);
    mins(r[28], r[28], 0.998f, X);
    maxs(r[28], r[28], 0.002f, X);
    madds(r[14], r[15], r[28][0], XYZ);
    sub(r[13], r[27], r[14], XYZW);
    rotate13();
    add(r[30], VF0, r[13], XYZW);
    mul(r[13], r[13], r[24], XYZW);
    sum13();
    subs(r[28], r[28], r[13][0], Y);
    Vec t;
    sub(t, r[10], r[14], Z);
    add(r[13], VF0, r[14], XYZW);
    muls(r[31], r[24], r[28][1], XYZ);
    vi4 = neg(t, Z) ? 2 : 0;
    transform13();
    add(r[14], VF0, r[13], XYZW);
    sub(r[13], VF0, r[18], XYZW);
    rotate13();
    add(r[9], VF0, r[13], XYZW);
    add(r[13], VF0, r[27], XYZW);
    transform13();
    add(r[25], VF0, r[13], XYZW);
    adds(r[7], VF0, -1.0f, X);
}

static int edge_sweep(void) {
    Vec keep11, keep10, keep15, keep8, keep12;
    copy(keep11, r[11]); copy(keep10, r[10]); copy(keep15, r[15]); copy(keep8, r[8]); copy(keep12, r[12]);
    add(r[1], VF0, r[17], XYZW);
    sub(r[2], VF0, r[18], XYZW);
    add(r[3], VF0, r[8], XYZW);
    add(r[4], VF0, r[15], XYZW);
    adds(r[30], VF0, q, W);
    adds(r[5], VF0, r[26][1], X);
    move(r[31], VF0, XYZW);
    const int hit = edge_cylinder();
    sub(r[29], r[8], r[7], XYZW);
    add(r[13], VF0, r[8], XYZW);
    vi13 = 1;
    if (hit) {
        muls(r[29], r[29], r[30][3], XYZ);
        adds(r[7], VF0, r[12][0], X);
        copy(r[12], keep12);
        transform13();
        add(r[14], VF0, r[13], XYZW);
        add(r[13], VF0, r[12], XYZW);
        copy(r[6], keep8);
        rotate13_scaled();
        add(r[24], VF0, r[13], XYZW);
        sub(r[13], VF0, r[18], XYZW);
        copy(r[15], keep15);
        rotate13();
        add(r[9], VF0, r[13], XYZW);
        add(r[13], VF0, r[27], XYZW);
        copy(r[10], keep10);
        transform13();
        Vec t;
        sub(t, r[10], r[8], Z);
        copy(r[11], keep11);
        add(r[8], VF0, r[6], XYZW);
        add(r[25], VF0, r[13], XYZW);
        vi4 = neg(t, Z) ? 2 : 0;
        return 1;
    }
    add(r[3], VF0, r[27], XYZW);
    copy(r[12], keep12);
    adds(r[4], VF0, r[26][1], X);
    copy(r[8], keep8);
    add(r[1], VF0, r[8], XYZW);
    copy(r[15], keep15);
    add(r[2], VF0, r[15], XYZW);
    copy(r[10], keep10);
    const int near = edge_near_sphere();
    mul(r[13], r[28], r[15], XYZW);
    copy(r[11], keep11);
    if (!near) {
        vi13 = 0;
        return 0;
    }
    vi13 = 1;
    edge_contact();
    mul(r[13], r[30], r[30], XYZW);
    sum13();
    adds(r[7], VF0, 1.5f, X);
    q = vsqrt(r[13][0]);
    vi4 = 2;
    adds(r[13], VF0, q, X);
    q = vdiv(VF0[3], r[13][0]);
    suba(VF0, r[30], XYZW);
    muls(r[30], r[30], q, XYZW);
    madds(r[31], r[30], r[26][1], XYZ);
    return 1;
}

static int edge_test(void) {
    sub(r[2], r[11], r[8], XYZW);
    add(r[1], VF0, r[8], XYZW);
    add(r[3], VF0, r[16], XYZW);
    move(r[4], r[26], X);
    add(r[15], VF0, r[2], XYZW);
    sub(r[28], r[27], r[8], XYZW);
    q = vdiv(VF0[3], r[26][1]);
    int hit = edge_near_sphere();
    adds(r[4], VF0, r[26][1], X);
    add(r[3], VF0, r[17], XYZW);
    if (!hit) {
        vi13 = 0;
        return 0;
    }
    hit = edge_near_sphere();
    mul(r[13], r[28], r[15], XYZW);
    if (!hit) return vi13 = (u32)edge_sweep();
    edge_contact();
    vi13 = 1;
    return 1;
}

static int face_test(void) {
    sub(r[19], VF0, VF0, XYZW);
    adds(r[6], r[19], 1.0f, W);
    muls(r[24], r[12], r[26][0], XYZ);
    q = vdiv(VF0[3], r[26][2]);
    add(r[1], r[24], r[16], XYZW);
    muls(r[2], r[24], -2.0f, XYZW);
    adds(r[26], r[19], q, W);
    if (!segment_hits_triangle()) return 0;
    muls(r[24], r[12], r[26][1], XYZ);
    sub(r[9], r[19], r[18], XYZW);
    sub(r[7], r[3], r[27], XYZW);
    sub(r[8], r[3], r[17], XYZW);
    muls(r[2], r[24], -2.0f, XYZW);
    add(r[1], r[17], r[24], XYZW);
    if (segment_hits_triangle()) {
        mul(r[13], r[7], r[12], XYZW);
        sum13();
        adds(r[6], r[19], r[13][0], Y);
        muls(r[14], r[12], r[6][1], XYZW);
        add(r[10], r[6], r[26], Y);
        add(r[14], r[14], r[27], XYZW);
        muls(r[31], r[12], r[10][1], XYZW);
        move(r[13], r[31], XYZW);
        if (neg(r[10], Y) || zero(r[10], Y)) return 0;
        rotate13_scaled();
        add(r[31], VF0, r[13], XYZW);
        const int inside = point_in_triangle();
        move(r[13], r[14], XYZW);
        if (!inside) return 0;
        transform13();
        move(r[14], r[13], XYZW);
        subs(r[15], r[19], 1.0f, X);
        move(r[13], r[12], XYZW);
        rotate13_scaled();
        move(r[24], r[13], XYZW);
    } else {
        mul(r[13], r[12], r[9], XYZW);
        adda(r[19], r[17], XYZW);
        msubs(r[1], r[12], r[26][1], XYZW);
        sum13();
        adds(r[6], r[19], r[13][0], Y);
        move(r[2], r[9], XYZW);
        if (!segment_through_triangle()) return 0;
        add(r[31], r[19], r[19], XYZW);
        move(r[13], r[14], XYZW);
        transform13();
        add(r[14], VF0, r[13], XYZW);
        move(r[13], r[12], XYZW);
        rotate13_scaled();
        add(r[24], VF0, r[13], XYZW);
    }
    move(r[13], r[9], XYZW);
    rotate13();
    add(r[9], VF0, r[13], XYZW);
    move(r[13], r[27], XYZW);
    transform13();
    add(r[25], VF0, r[13], XYZW);
    return 1;
}

static struct { float t; u32 kind; Vec p14, p24, p25, p31, p9; } g_first;

static int contact(void) {
    if (vi5++ == 0) {
        g_first.t = r[15][0];
        g_first.kind = vi4 & 0xFFFF;
        copy(g_first.p14, r[14]); copy(g_first.p24, r[24]); copy(g_first.p25, r[25]);
        copy(g_first.p31, r[31]); copy(g_first.p9, r[9]);
        return 0;
    }
    add(r[16], VF0, r[19], X);
    adds(r[17], VF0, r[19][1], X);
    vi6 = g_first.kind;
    r[1][0] = g_first.t;
    copy(r[2], g_first.p14); copy(r[3], g_first.p24); copy(r[4], g_first.p25);
    copy(r[5], g_first.p31); copy(r[6], g_first.p9);
    return 1;
}

static void sphere_sweep(void) {
    triangle_plane();
    sub(r[18], r[17], r[27], XYZW);
    const int face = face_test();
    add(r[19], VF0, r[16], X);
    adds(r[19], VF0, r[17][0], Y);
    vi4 = 1;
    vi13 = (u32)face;
    if (face) {
        vi5 = 1;
        return;
    }
    Vec corner[3];
    copy(corner[0], r[3]); copy(corner[1], r[4]); copy(corner[2], r[5]);
    add(r[8], VF0, r[3], XYZW);
    add(r[11], VF0, r[4], XYZW);
    add(r[10], VF0, r[5], XYZW);
    for (int e = 0; e < 3; e++) {
        if (e) {
            add(r[16], VF0, r[19], X);
            adds(r[17], VF0, r[19][1], X);
            copy(r[e == 1 ? 10 : 11], corner[0]);
            copy(r[e == 1 ? 8 : 10], corner[1]);
            copy(r[e == 1 ? 11 : 8], corner[2]);
        }
        const int hit = edge_test();
        add(r[15], VF0, r[7], XYZW);
        if (e == 0) vi5 = 0;
        if (hit && contact()) return;
    }
    add(r[16], VF0, r[19], X);
    adds(r[17], VF0, r[19][1], X);
    if (vi5) {
        add(r[16], VF0, r[19], X);
        adds(r[17], VF0, r[19][1], X);
        vi4 = g_first.kind;
        r[15][0] = g_first.t;
        copy(r[14], g_first.p14); copy(r[24], g_first.p24); copy(r[25], g_first.p25);
        copy(r[31], g_first.p31); copy(r[9], g_first.p9);
    }
}

static void triangle_normal(void) {
    sub(r[2], r[2], r[1], XYZW);
    sub(r[3], r[3], r[1], XYZW);
    opmula(r[2], r[3]);
    opmsub(r[4], r[3], r[2]);
    mul(r[2], r[4], r[4], XYZW);
    adds(r[1], r[2], r[2][2], X);
    adds(r[1], r[1], r[2][1], X);
    q = vrsqrt(VF0[3], r[1][0]);
    muls(r[4], r[4], q, XYZW);
}

static void load(const PcVu0 *s) {
    memcpy(r, s->vf, sizeof r);
    copy(r[0], VF0);
    copy(acc, s->acc);
    q = s->q;
    vi4 = s->vi[4] & 0xFFFF;
    vi5 = s->vi[5] & 0xFFFF;
    vi6 = s->vi[6] & 0xFFFF;
    vi13 = s->vi[13] & 0xFFFF;
}

static void save(PcVu0 *s) {
    memcpy(s->vf[1], r[1], sizeof(Vec) * 31);
    copy(s->vf[0], VF0);
    copy(s->acc, acc);
    s->q = q;
    s->vi[4] = vi4 & 0xFFFF;
    s->vi[5] = vi5 & 0xFFFF;
    s->vi[6] = vi6 & 0xFFFF;
    s->vi[13] = vi13 & 0xFFFF;
}

void vu0_sphere_sweep(PcVu0 *s) { load(s); sphere_sweep(); save(s); }
void vu0_triangle_normal(PcVu0 *s) { load(s); triangle_normal(); save(s); }
void vu0_triangle_plane(PcVu0 *s) { load(s); triangle_plane(); save(s); }
void vu0_segment_crosses_triangle(PcVu0 *s) {
    load(s);
    triangle_plane();
    vi13 = (u32)segment_hits_triangle();
    save(s);
}

#define COLLISION_PROGRAM GAME_VAR(dword_250EB0)
#define SHADOW_PROGRAM GAME_VAR(dword_251ED0)

static enum { LOADED_NONE, LOADED_COLLISION, LOADED_SHADOW } g_loaded;

void pc_vu0_load(PcAddress program) {
    if (program == COLLISION_PROGRAM) g_loaded = LOADED_COLLISION;
    else if (program == SHADOW_PROGRAM) g_loaded = LOADED_SHADOW;
    else revo_fatal("[vu0] a program at 0x%08X sent: not one of the game's two", program);
}

void pc_vu0_call(uint32_t address, PcVu0 *v) {
    const u32 addr = address << 3;
    if (addr == 0x0000 && g_loaded == LOADED_COLLISION) vu0_sphere_sweep(v);
    else if (addr == 0x0000 && g_loaded == LOADED_SHADOW) vu0_triangle_normal(v);
    else if (addr == 0x0F30) vu0_segment_crosses_triangle(v);
    else if (addr == 0x0FD0) vu0_triangle_plane(v);
    else revo_fatal("[vu0] vcallms 0x%04X with %s loaded: no such routine", addr,
                    g_loaded == LOADED_COLLISION ? "the collision program"
                    : g_loaded == LOADED_SHADOW  ? "the shadow program"
                                                 : "no program");
}
