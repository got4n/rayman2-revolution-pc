#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../glue_rt.h"
#include "../mods.h"
#include "../runtime.h"
#include "PC.h"

enum {
    MATRIX_SIZE = 0x54,
    TYPE = 0x50,
    SO_LINKED = 0x04,
    SO_LOCAL = 0x20,
    SO_GLOBAL = 0x24,
    SO_COMPUTED = 0x28,
    MAX_KEPT = 8192,
};

enum { T_UNINITIALIZED, T_IDENTITY, T_TRANSLATION, T_ZOOM, T_SCALE, T_ROTATION, T_UNDEFINED = 9 };

#define JUMP 4.0f
#define TURN 0.7071f

typedef struct Kept {
    u32 so;
    u32 linked;
    u32 matrix;
    u32 local;
    u32 computed;
    float m[MATRIX_SIZE / 4];
    float l[MATRIX_SIZE / 4];
    bool fresh;
    bool moved;
} Kept;

typedef struct Frame {
    Kept *kept;
    u32 count;
    int *slots;
    u32 mask;
} Frame;

static bool in_ram(u32 a) { return a >= REVO_GAME_LO && a < REVO_GAME_HI; }
static bool matrix_at(u32 a) { return in_ram(a) && !(a & 3) && in_ram(a + MATRIX_SIZE); }

static Frame g_frames[2];
static int g_newer;
static bool g_have_older;
static unsigned long long g_kept_frames;

static Frame *newer(void) { return &g_frames[g_newer]; }
static Frame *older(void) { return &g_frames[g_newer ^ 1]; }

static u32 slot_of(u32 so) { return (so >> 4) * 0x9E3779B1u; }

static void frame_init(Frame *f) {
    if (f->kept) return;
    f->kept = malloc(sizeof *f->kept * MAX_KEPT);
    f->mask = 2 * MAX_KEPT - 1;
    f->slots = malloc(sizeof *f->slots * (f->mask + 1));
}

static void frame_clear(Frame *f) {
    f->count = 0;
    memset(f->slots, 0xFF, sizeof *f->slots * (f->mask + 1));
}

static const Kept *find(const Frame *f, u32 so) {
    for (u32 i = slot_of(so) & f->mask; f->slots[i] >= 0; i = (i + 1) & f->mask)
        if (f->kept[f->slots[i]].so == so) return &f->kept[f->slots[i]];
    return NULL;
}

int32_t pc_inbetween_wanted(void) {
    extern int revo_fast_forwarding(void);
    const bool on = revo_mod(REVO_MOD_INTERPOLATION) && !revo_fast_forwarding();
    if (!on) g_kept_frames = 0;
    return on;
}

void pc_inbetween_keep_begin(void) {
    frame_init(&g_frames[0]);
    frame_init(&g_frames[1]);
    g_have_older = g_kept_frames > 0;
    g_newer ^= 1;
    frame_clear(newer());
    g_kept_frames++;
}

void pc_inbetween_keep(PcAddress so, int32_t fresh) {
    Frame *f = newer();
    if (f->count >= MAX_KEPT || !in_ram(so) || (so & 3)) return;
    const u32 matrix = rd32(so + SO_GLOBAL), local = rd32(so + SO_LOCAL);
    if (!matrix_at(matrix)) return;
    u32 i = slot_of(so) & f->mask;
    for (; f->slots[i] >= 0; i = (i + 1) & f->mask)
        if (f->kept[f->slots[i]].so == so) return;
    Kept *k = &f->kept[f->count];
    k->so = so;
    k->linked = rd32(so + SO_LINKED);
    k->matrix = matrix;
    k->local = matrix_at(local) && local != matrix ? local : 0;
    k->computed = rd32(so + SO_COMPUTED);
    k->fresh = fresh != 0;
    k->moved = false;
    memcpy(k->m, RAM + matrix, MATRIX_SIZE);
    if (k->local) memcpy(k->l, RAM + k->local, MATRIX_SIZE);
    f->slots[i] = (int)f->count++;
}

static float column_length(const float *c) { return sqrtf(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]); }

static void to_quaternion(const float r[3][3], float q[4]) {
#define E(i, j) r[j][i]
    const float trace = E(0, 0) + E(1, 1) + E(2, 2);
    if (trace > 0.0f) {
        const float s = sqrtf(trace + 1.0f) * 2.0f;
        q[3] = 0.25f * s;
        q[0] = (E(2, 1) - E(1, 2)) / s;
        q[1] = (E(0, 2) - E(2, 0)) / s;
        q[2] = (E(1, 0) - E(0, 1)) / s;
    } else if (E(0, 0) > E(1, 1) && E(0, 0) > E(2, 2)) {
        const float s = sqrtf(1.0f + E(0, 0) - E(1, 1) - E(2, 2)) * 2.0f;
        q[3] = (E(2, 1) - E(1, 2)) / s;
        q[0] = 0.25f * s;
        q[1] = (E(0, 1) + E(1, 0)) / s;
        q[2] = (E(0, 2) + E(2, 0)) / s;
    } else if (E(1, 1) > E(2, 2)) {
        const float s = sqrtf(1.0f + E(1, 1) - E(0, 0) - E(2, 2)) * 2.0f;
        q[3] = (E(0, 2) - E(2, 0)) / s;
        q[0] = (E(0, 1) + E(1, 0)) / s;
        q[1] = 0.25f * s;
        q[2] = (E(1, 2) + E(2, 1)) / s;
    } else {
        const float s = sqrtf(1.0f + E(2, 2) - E(0, 0) - E(1, 1)) * 2.0f;
        q[3] = (E(1, 0) - E(0, 1)) / s;
        q[0] = (E(0, 2) + E(2, 0)) / s;
        q[1] = (E(1, 2) + E(2, 1)) / s;
        q[2] = 0.25f * s;
    }
#undef E
}

static void from_quaternion(const float q[4], float r[3][3]) {
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    r[0][0] = 1 - 2 * (y * y + z * z), r[0][1] = 2 * (x * y + z * w), r[0][2] = 2 * (x * z - y * w);
    r[1][0] = 2 * (x * y - z * w), r[1][1] = 1 - 2 * (x * x + z * z), r[1][2] = 2 * (y * z + x * w);
    r[2][0] = 2 * (x * z + y * w), r[2][1] = 2 * (y * z - x * w), r[2][2] = 1 - 2 * (x * x + y * y);
}

static bool split(const float *m, float r[3][3], float len[3]) {
    for (int c = 0; c < 3; c++) {
        len[c] = column_length(m + 4 * c);
        if (len[c] < 1e-6f) return false;
        for (int k = 0; k < 3; k++) r[c][k] = m[4 * c + k] / len[c];
    }
    for (int a = 0; a < 3; a++)
        for (int b = a + 1; b < 3; b++)
            if (fabsf(r[a][0] * r[b][0] + r[a][1] * r[b][1] + r[a][2] * r[b][2]) > 0.02f) return false;
    const float det = r[0][0] * (r[1][1] * r[2][2] - r[2][1] * r[1][2]) -
                      r[1][0] * (r[0][1] * r[2][2] - r[2][1] * r[0][2]) +
                      r[2][0] * (r[0][1] * r[1][2] - r[1][1] * r[0][2]);
    return det > 0.0f;
}

static bool half_way(const float *o, const float *n, float t, float *out) {
    memcpy(out, n, MATRIX_SIZE);
    u8 to, tn;
    memcpy(&to, (const u8 *)o + TYPE, 1);
    memcpy(&tn, (const u8 *)n + TYPE, 1);
    if (to != tn || tn == T_UNINITIALIZED || tn == T_IDENTITY || tn >= T_UNDEFINED) return false;
    const float dx = n[12] - o[12], dy = n[13] - o[13], dz = n[14] - o[14];
    if (dx * dx + dy * dy + dz * dz > JUMP * JUMP) return false;
    for (int k = 12; k < 15; k++) out[k] = o[k] + (n[k] - o[k]) * t;
    if (tn == T_TRANSLATION) return true;
    for (int k = 16; k < 19; k++) out[k] = o[k] + (n[k] - o[k]) * t;
    float ro[3][3], rn[3][3], lo[3], ln[3];
    if (split(o, ro, lo) && split(n, rn, ln)) {
        float qo[4], qn[4], q[4];
        to_quaternion(ro, qo);
        to_quaternion(rn, qn);
        float d = qo[0] * qn[0] + qo[1] * qn[1] + qo[2] * qn[2] + qo[3] * qn[3];
        if (d < 0.0f) {
            for (int k = 0; k < 4; k++) qo[k] = -qo[k];
            d = -d;
        }
        if (d < TURN) {
            memcpy(out, n, MATRIX_SIZE);
            return false;
        }
        float l = 0.0f;
        for (int k = 0; k < 4; k++) {
            q[k] = qo[k] + (qn[k] - qo[k]) * t;
            l += q[k] * q[k];
        }
        l = 1.0f / sqrtf(l);
        for (int k = 0; k < 4; k++) q[k] *= l;
        float r[3][3];
        from_quaternion(q, r);
        for (int c = 0; c < 3; c++) {
            const float len = lo[c] + (ln[c] - lo[c]) * t;
            for (int k = 0; k < 3; k++) out[4 * c + k] = r[c][k] * len;
        }
    } else {
        for (int c = 0; c < 3; c++)
            for (int k = 0; k < 3; k++) out[4 * c + k] = o[4 * c + k] + (n[4 * c + k] - o[4 * c + k]) * t;
    }
    return true;
}

static bool g_drawing_in_between;
int revo_drawing_in_between(void) { return g_drawing_in_between; }

int32_t pc_inbetween_move(void) {
    if (!g_have_older) return 0;
    g_drawing_in_between = true;
    const Frame *was = older();
    Frame *now = newer();
    for (u32 i = 0; i < now->count; i++) {
        Kept *k = &now->kept[i];
        const Kept *o = find(was, k->so);
        if (!o || o->linked != k->linked || o->matrix != k->matrix) continue;
        float m[MATRIX_SIZE / 4];
        if (k->fresh && o->fresh) {
            if (!half_way(o->m, k->m, 0.5f, m)) continue;
            memcpy(RAM + k->matrix, m, MATRIX_SIZE);
        } else if (!k->fresh && k->local && o->local == k->local) {
            if (!half_way(o->l, k->l, 0.5f, m)) continue;
            memcpy(RAM + k->local, m, MATRIX_SIZE);
        } else {
            continue;
        }
        k->moved = true;
    }
    return 1;
}

void pc_inbetween_restore(void) {
    g_drawing_in_between = false;
    Frame *now = newer();
    for (u32 i = 0; i < now->count; i++) {
        Kept *k = &now->kept[i];
        memcpy(RAM + k->matrix, k->m, MATRIX_SIZE);
        if (k->local) memcpy(RAM + k->local, k->l, MATRIX_SIZE);
        wr32(k->so + SO_COMPUTED, k->computed);
        k->moved = false;
    }
}
