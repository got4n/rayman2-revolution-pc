#include "game_vars.h"
#include <stdlib.h>
#include <string.h>

#include "clump.h"
#include "render.h"
#include "gli.h"
#include "handles.h"

typedef struct GeometryView {
    ClumpGeometry *g;
    u32 handle;
    u32 morph;
    GuestPtr vertices, normals;
} GeometryView;

static GeometryView **g_views;
static int g_nviews, g_views_cap;

static GeometryView *view_of(const ClumpGeometry *g) {
    for (int i = 0; i < g_nviews; i++)
        if (g_views[i]->g == g) return g_views[i];
    return NULL;
}

u32 geometry_handle(ClumpGeometry *g) {
    GeometryView *v = view_of(g);
    if (v) return v->handle;
    if (g_nviews == g_views_cap) {
        g_views_cap = g_views_cap ? g_views_cap * 2 : 256;
        g_views = realloc(g_views, sizeof *g_views * (size_t)g_views_cap);
    }
    v = calloc(1, sizeof *v);
    v->g = g;
    v->handle = handle_new(OBJECT_GEOMETRY, v);
    g_views[g_nviews++] = v;
    return v->handle;
}

void morphs_clear(void) {
    for (int i = 0; i < g_nviews; i++) {
        GeometryView *v = g_views[i];
        handle_free(v->handle);
        if (v->morph) handle_free(v->morph);
        if (v->vertices) window_free(v->vertices);
        if (v->normals) window_free(v->normals);
        free(v);
    }
    g_nviews = 0;
}

static GeometryView *geometry_of(u32 value) { return handle_object(OBJECT_GEOMETRY, value); }

static GuestPtr lend(const float (*from)[3], int count) {
    GuestPtr b = window_alloc((u32)count * 12u);
    if (b) memcpy(GUEST(u8, b), from, (size_t)count * 12u);
    return b;
}

u32 morph_get_target(u32 geometry, s32 index) {
    GeometryView *v = geometry_of(geometry);
    if (!v || index != 0) return 0;
    if (!v->morph) {
        v->morph = handle_new(OBJECT_MORPH_TARGET, v);
        v->vertices = lend((const float (*)[3])v->g->positions, v->g->vertex_count);
        if (v->g->normals) v->normals = lend((const float (*)[3])v->g->normals, v->g->vertex_count);
    }
    return v->morph;
}

u32 morph_get_vertices(u32 target) {
    GeometryView *v = handle_object(OBJECT_MORPH_TARGET, target);
    return v ? v->vertices : 0;
}
u32 morph_get_normals(u32 target) {
    GeometryView *v = handle_object(OBJECT_MORPH_TARGET, target);
    return v ? v->normals : 0;
}

u32 morph_lock(u32 geometry, u32 flags) {
    (void)flags;
    return geometry;
}

u32 morph_vertex_count(u32 geometry) {
    GeometryView *v = geometry_of(geometry);
    return v ? (u32)v->g->vertex_count : 0;
}

u32 morph_unlock(u32 geometry) {
    GeometryView *v = geometry_of(geometry);
    if (v && v->vertices) {
        const size_t bytes = (size_t)v->g->vertex_count * 12u;
        memcpy(v->g->positions, GUEST(u8, v->vertices), bytes);
        if (v->normals && v->g->normals) memcpy(v->g->normals, GUEST(u8, v->normals), bytes);
    }
    return geometry;
}

#define WOBBLE_MAX_VERTICES 0x400
#define WOBBLE_SINES GAME_VAR(a64_fSinTable)
#define WOBBLE_PHASE GAME_VAR(a3_xSinEffectBaseMesh)
#define WOBBLE_AMPLITUDE GAME_VAR(stSinEffectAmp)
#define WATER_PHASE GAME_VAR(GLI_fF2pow24)
#define WATER_PHASE_SECOND GAME_VAR(GLI_fSinEffectBase)
#define MATRIX_BYTES 0x80u

static u32 float_bits(float f) {
    u32 b;
    memcpy(&b, &f, 4);
    return b;
}

void gli_object_wobble(GuestPtr object) {
    const s16 count = *GUEST(s16, object + 0x20);
    if (count > WOBBLE_MAX_VERTICES) return;
    const GuestPtr source = *GUEST(GuestPtr, object + 0x24);
    const GuestPtr frame = game_stack_push(2 * MATRIX_BYTES + (u32)WOBBLE_MAX_VERTICES * 12u);
    const GuestPtr matrix = frame, inverse = frame + MATRIX_BYTES, moved = frame + 2 * MATRIX_BYTES;
    const GliContext *ctx = gli_context();
    const float bias = *GUEST(float, ctx->unk20B8 ? WATER_PHASE_SECOND : WATER_PHASE);
    const float *base = GUEST(float, WOBBLE_PHASE), *amplitude = GUEST(float, WOBBLE_AMPLITUDE);
    const float *sines = GUEST(float, WOBBLE_SINES);
    const float phase[3] = {fadd(base[0], bias), fadd(base[1], bias), fadd(base[2], bias)};
    _POS_fn_vMulMatrixMatrix(matrix, ctx->geometry_matrix, ctx->current_matrix);
    POS_fn_vInvertMatrix(inverse, matrix);
    POS_fn_vMulMatrixVertexArray(moved, matrix, source, (u32)count);
    for (int i = 0; i < count; i++) {
        float *v = GUEST(float, moved + (u32)i * 12u);
        const float x = v[0], y = v[1], z = v[2];
        const float angle_z = fadd(fadd(phase[2], y), x);
        const float angle_x = fadd(fadd(phase[0], y), z);
        const float angle_y = fadd(fadd(phase[1], x), z);
        v[0] = fadd(x, fmul(sines[float_bits(angle_x) & 0x3F], amplitude[0]));
        v[1] = fadd(y, fmul(sines[float_bits(angle_y) & 0x3F], amplitude[1]));
        v[2] = fadd(z, fmul(sines[float_bits(angle_z) & 0x3F], amplitude[2]));
    }
    POS_fn_vMulMatrixVertexArray(moved, inverse, moved, (u32)count);

    const DriverObject *d = GUEST(DriverObject, *GUEST(GuestPtr, object + 0x1C));
    const GuestPtr lists = *GUEST(GuestPtr, *GUEST(GuestPtr, object + 0x1C) + 8);
    for (int k = 0; k < d->atomic_count && lists; k++) {
        Atomic *a = handle_object(OBJECT_ATOMIC, GUEST(u32, d->atomics)[k]);
        if (!a) continue;
        ClumpGeometry *g = level_geometry(a);
        const u16 *index = GUEST(u16, GUEST(GuestPtr, lists)[k]);
        for (int j = 0; j < g->vertex_count; j++)
            memcpy(g->positions[j], GUEST(float, moved + (u32)index[j] * 12u), 12);
    }
    game_stack_pop(2 * MATRIX_BYTES + (u32)WOBBLE_MAX_VERTICES * 12u);
}
