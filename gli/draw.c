#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "clump.h"
#include "gli.h"
#include "handles.h"
#include "render.h"

typedef struct CamVertex {
    float x, y, z;
    float u, v;
    float lu, lv;
    float r, g, b, a;
    float fog;
    u32 id;
} CamVertex;

enum { CAM_FLOATS = offsetof(CamVertex, id) / sizeof(float) };

static u32 edge_id(u32 a, u32 b) {
    u32 lo = a < b ? a : b, hi = a < b ? b : a;
    return 0x80000000u | ((lo * 0x9E3779B1u) ^ (hi * 0x85EBCA77u) ^ (hi >> 15));
}

static u32 source_id(const void *atomic, const void *frame, int mesh, int pass) {
    uint64_t h = (uint64_t)(uintptr_t)atomic * 0x9E3779B97F4A7C15ull;
    h ^= (uint64_t)(uintptr_t)frame * 0xC2B2AE3D27D4EB4Full;
    h ^= ((uint64_t)(u32)mesh << 8 | (u32)pass) * 0x165667B19E3779F9ull;
    h ^= h >> 29;
    u32 s = (u32)h ^ (u32)(h >> 32);
    return s ? s : 1;
}

static void frame_world(const Frame *f, float m[4][3]) {
    memcpy(m[0], f->right, sizeof m[0]);
    memcpy(m[1], f->up, sizeof m[1]);
    memcpy(m[2], f->at, sizeof m[2]);
    memcpy(m[3], f->pos, sizeof m[3]);
    for (const Frame *p = f->parent; p; p = p->parent) {
        float out[4][3];
        const float *rows[4] = {p->right, p->up, p->at, p->pos};
        for (int i = 0; i < 4; i++)
            for (int k = 0; k < 3; k++)
                out[i][k] = m[i][0] * rows[0][k] + m[i][1] * rows[1][k] + m[i][2] * rows[2][k] + (i == 3 ? rows[3][k] : 0.0f);
        memcpy(m, out, sizeof out);
    }
}

static void transform(const float m[4][3], const float p[3], float out[3]) {
    for (int k = 0; k < 3; k++) out[k] = p[0] * m[0][k] + p[1] * m[1][k] + p[2] * m[2][k] + m[3][k];
}

static void rotate(const float m[4][3], const float n[3], float out[3]) {
    for (int k = 0; k < 3; k++) out[k] = n[0] * m[0][k] + n[1] * m[1][k] + n[2] * m[2][k];
}

static const ShadowView *g_shadow;

void draw_into_shadow(const ShadowView *view) { g_shadow = view; }

static void to_camera(const Camera *c, const float w[3], float out[3]) {
    if (g_shadow) {
        const ShadowView *v = g_shadow;
        float d[3] = {fsub(w[0], v->pos[0]), fsub(w[1], v->pos[1]), fsub(w[2], v->pos[2])};
        const float *axes[3] = {v->right, v->up, v->at};
        for (int k = 0; k < 3; k++) {
            float s = k < 2 ? v->scale[k] : 1.0f, a[3];
            for (int j = 0; j < 3; j++) a[j] = fmul(axes[k][j], s);
            out[k] = fadd(fadd(fmul(d[0], a[0]), fmul(d[1], a[1])), fmul(d[2], a[2]));
        }
        return;
    }
    for (int k = 0; k < 3; k++) out[k] = c->view[0][k] * w[0] + c->view[1][k] * w[1] + c->view[2][k] * w[2] + c->view[3][k];
}

static float clamp01(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

static void light_vertex(const ClumpGeometry *g, const float world[3], const float normal[3], float c[3]) {
    int count;
    Light *const *lights = level_lights(&count);
    for (int i = 0; i < count; i++) {
        const Light *l = lights[i];
        if (!l->on) continue;
        float k;
        if (l->type == RW_LIGHT_AMBIENT) {
            k = g->ambient;
        } else if (l->type == RW_LIGHT_DIRECTIONAL) {
            if (!l->frame) continue;
            float d = -(normal[0] * l->frame->at[0] + normal[1] * l->frame->at[1] + normal[2] * l->frame->at[2]);
            k = d > 0.0f ? d * g->diffuse : 0.0f;
        } else {
            if (!l->frame || l->radius <= 0.0f) continue;
            float to[3] = {l->frame->pos[0] - world[0], l->frame->pos[1] - world[1], l->frame->pos[2] - world[2]};
            float dist = sqrtf(to[0] * to[0] + to[1] * to[1] + to[2] * to[2]);
            if (dist >= l->radius || dist == 0.0f) continue;
            float d = (normal[0] * to[0] + normal[1] * to[1] + normal[2] * to[2]) / dist;
            k = d > 0.0f ? d * (1.0f - dist / l->radius) * g->diffuse : 0.0f;
        }
        for (int j = 0; j < 3; j++) c[j] += k * l->color[j];
    }
}

static const Camera *g_cam;

static void project(void) {
    if (!g_cam) return;
    GsProjection p = {{g_cam->view_window[0], g_cam->view_window[1]}, 0.0f, 0.0f, g_cam->near_plane};
    camera_depth_terms(g_cam, &p.depth_scale, &p.depth_offset);
    render_set_projection(&p);
}
static void rdraw(GsPrim prim, const GsState *s, const GsVertex *v, int n) {
    project();
    render_draw(prim, s, v, n);
}
static void rdraw_ext(GsPrim prim, const GsState *s, const GsVertex *v, const GsVertexExt *x, int n) {
    project();
    render_draw_ext(prim, s, v, x, n);
}

static void screen_xy(const Camera *cam, const CamVertex *v, float *x, float *y) {
    const float q = 1.0f / v->z;
    *x = GS_WIDTH * 0.5f * (1.0f - v->x * q / cam->view_window[0]);
    *y = GS_HEIGHT * 0.5f * (1.0f - v->y * q / cam->view_window[1]);
}

static GsVertex on_screen(const Camera *cam, const CamVertex *v) {
    if (g_shadow) {
        GsVertex g = {.q = 1.0f, .s = v->u, .t = v->v, .z = 65535.0f, .fog = 255};
        g.x = (float)g_shadow->width * 0.5f * (1.0f - v->x);
        g.y = (float)g_shadow->height * 0.5f * (1.0f - v->y);
        g.r = (u8)(v->r < 0 ? 0 : v->r > 255 ? 255 : v->r);
        g.g = (u8)(v->g < 0 ? 0 : v->g > 255 ? 255 : v->g);
        g.b = (u8)(v->b < 0 ? 0 : v->b > 255 ? 255 : v->b);
        g.a = (u8)(v->a < 0 ? 0 : v->a > 255 ? 255 : v->a);
        g.key = v->id;
        return g;
    }
    (void)cam;
    GsVertex g = {0};
    g.x = v->x;
    g.y = v->y;
    g.z = v->z;
    g.q = 1.0f;
    g.s = v->u;
    g.t = v->v;
    g.r = (u8)(v->r < 0 ? 0 : v->r > 255 ? 255 : v->r);
    g.g = (u8)(v->g < 0 ? 0 : v->g > 255 ? 255 : v->g);
    g.b = (u8)(v->b < 0 ? 0 : v->b > 255 ? 255 : v->b);
    g.a = (u8)(v->a < 0 ? 0 : v->a > 255 ? 255 : v->a);
    g.fog = (u8)(v->fog < 0 ? 0 : v->fog > 255 ? 255 : v->fog);
    g.key = v->id;
    return g;
}

static CamVertex lerp(const CamVertex *a, const CamVertex *b, float t) {
    CamVertex v;
    const float *pa = &a->x, *pb = &b->x;
    float *pv = &v.x;
    for (size_t i = 0; i < CAM_FLOATS; i++) pv[i] = pa[i] + (pb[i] - pa[i]) * t;
    v.id = a->id && b->id ? edge_id(a->id, b->id) : 0;
    return v;
}

typedef struct Triangles {
    GsVertex *v;
    int n, cap;
    int *starts, *clipped, nstarts, starts_cap;
    bool with_ext;
    GsVertexExt *x;
    int xcap;
} Triangles;

enum { BATCH_A = 32, BATCH_B = 28 };

static void mark(Triangles *t, int clipped) {
    if (t->nstarts == t->starts_cap) {
        t->starts_cap = t->starts_cap ? t->starts_cap * 2 : 64;
        t->starts = realloc(t->starts, sizeof *t->starts * (size_t)t->starts_cap);
        t->clipped = realloc(t->clipped, sizeof *t->clipped * (size_t)t->starts_cap);
    }
    t->clipped[t->nstarts] = clipped;
    t->starts[t->nstarts++] = t->n;
}

static int start(const Triangles *t, int k) { return k < t->nstarts ? t->starts[k] : t->n; }

static void push(Triangles *t, GsVertex v) {
    if (t->n == t->cap) {
        t->cap = t->cap ? t->cap * 2 : 1024;
        t->v = realloc(t->v, sizeof *t->v * (size_t)t->cap);
    }
    t->v[t->n++] = v;
}

static void emit(Triangles *t, const Camera *cam, const CamVertex *v) {
    push(t, on_screen(cam, v));
    if (!t->with_ext) return;
    if (t->xcap < t->cap) {
        t->xcap = t->cap;
        t->x = realloc(t->x, sizeof *t->x * (size_t)t->xcap);
    }
    t->x[t->n - 1] = (GsVertexExt){v->lu, v->lv};
}

static void triangle(Triangles *out, const Camera *cam, const CamVertex *a, const CamVertex *b, const CamVertex *c) {
    const float near = g_shadow ? g_shadow->near_plane : cam->near_plane;
    const CamVertex *in[3] = {a, b, c};
    int inside = (a->z >= near) + (b->z >= near) + (c->z >= near);
    if (inside == 0) return;
    if (inside == 3 || !g_shadow) {
        for (int i = 0; i < 3; i++) emit(out, cam, in[i]);
        return;
    }
    CamVertex poly[4];
    int n = 0;
    for (int i = 0; i < 3; i++) {
        const CamVertex *p = in[i], *q = in[(i + 1) % 3];
        if (p->z >= near) poly[n++] = *p;
        if ((p->z >= near) != (q->z >= near)) poly[n++] = lerp(p, q, (near - p->z) / (q->z - p->z));
    }
    for (int i = 1; i + 1 < n; i++) {
        emit(out, cam, &poly[0]);
        emit(out, cam, &poly[i]);
        emit(out, cam, &poly[i + 1]);
    }
}

enum { IN_BATCH, CLIPPED, CLIPPED_UNSEEN, CULLED };

static int clipping(const Camera *cam, const CamVertex *const in[3]) {
    const float x0 = -GS_WIDTH * 0.5f, x1 = GS_WIDTH * 1.5f, y0 = -GS_HEIGHT * 0.5f, y1 = GS_HEIGHT * 1.5f;
    int behind = 0;
    for (int i = 0; i < 3; i++) behind += in[i]->z < cam->near_plane;
    if (behind == 3) return CULLED;
    if (behind) return CLIPPED;
    int out[4] = {0}, off[4] = {0};
    for (int i = 0; i < 3; i++) {
        float x, y;
        screen_xy(cam, in[i], &x, &y);
        out[0] += x < x0, out[1] += x > x1, out[2] += y < y0, out[3] += y > y1;
        off[0] += x < 0.0f, off[1] += x > GS_WIDTH, off[2] += y < 0.0f, off[3] += y > GS_HEIGHT;
    }
    if (out[0] == 3 || out[1] == 3 || out[2] == 3 || out[3] == 3) return CULLED;
    if (!out[0] && !out[1] && !out[2] && !out[3]) return IN_BATCH;
    return off[0] == 3 || off[1] == 3 || off[2] == 3 || off[3] == 3 ? CLIPPED_UNSEEN : CLIPPED;
}

static void mesh_triangles(Triangles *out, Triangles *clipped, const Camera *cam, const ClumpGeometry *g,
                           const ClumpMesh *m, const CamVertex *v) {
    out->nstarts = 0;
    if (clipped) clipped->n = clipped->nstarts = 0;
    const u32 step = g->strips ? 1 : 3;
    u32 left = 0;
    bool second = false;
    for (u32 i = 0; i + 2 < m->count; i += step) {
        if (clipped && left == 0) {
            mark(out, clipped->nstarts);
            left = second ? BATCH_B : BATCH_A;
            second = !second;
        }
        left--;
        const CamVertex *in[3] = {&v[m->indices[i]], &v[m->indices[i + 1]], &v[m->indices[i + 2]]};
        const int all = 0;
        if (!all && (in[0] == in[1] || in[1] == in[2] || in[0] == in[2])) continue;
        if (clipped) {
            const int c = clipping(cam, in);
            if (c == CULLED || (c == CLIPPED_UNSEEN && !all)) continue;
            if (c != IN_BATCH) {
                mark(clipped, 0);
                triangle(clipped, cam, in[0], in[1], in[2]);
                continue;
            }
        }
        triangle(out, cam, in[0], in[1], in[2]);
    }
}

static void textured(GsState *s, const Texture *t) {
    s->tme = true;
    s->texture = t->image;
    s->tfx = GS_MODULATE;
    s->tcc = true;
    s->wrap_s = t->address_u == ADDRESS_WRAP ? GS_REPEAT : GS_CLAMP;
    s->wrap_t = t->address_v == ADDRESS_WRAP ? GS_REPEAT : GS_CLAMP;
    s->mmag = s->mmin = 1;
    s->k = -4 * 16;
    s->alpha_test = true;
    s->atst = GS_GEQUAL;
    s->aref = 64;
    s->afail = GS_FB_ONLY;
}

static Triangles g_tris;

static void draw(const GsState *s) {
    if (g_tris.n) rdraw(GS_TRIANGLES, s, g_tris.v, g_tris.n);
}

static const GsView *camera_view(const Camera *cam, GsView *out) {
    if (g_shadow || !cam) return NULL;
    for (int i = 0; i < 4; i++)
        for (int k = 0; k < 3; k++) out->view[i][k] = cam->view[i][k];
    out->window[0] = cam->view_window[0];
    out->window[1] = cam->view_window[1];
    out->origin[0] = out->origin[1] = out->origin[2] = 0.0f;
    return out;
}

const GsView *draw_camera_view(GsView *out) { return camera_view(camera_current(), out); }

typedef struct Passes {
    GsState base, maps[3], fog;
    bool have[3], fogged, lightmaps;
    u32 fog_color;
    const Atomic *atomic;
    int mesh;
    const GsView *view;
} Passes;

static void passes(const Passes *p, const Triangles *t, const Triangles *l, int from, int to, int pass, bool base) {
    static Triangles fogged;
    if (from >= to) return;
    const Atomic *a = p->atomic;
    if (base) {
        render_set_source(source_id(a, a->frame, p->mesh, pass), p->view);
        rdraw(GS_TRIANGLES, &p->base, t->v + from, to - from);
    }
    for (int c = 0; c < 3 && p->lightmaps; c++) {
        if (!p->have[c]) continue;
        render_set_source(source_id(a, a->frame, p->mesh, pass + 1 + c), p->view);
        rdraw(GS_TRIANGLES, &p->maps[c], l->v + from, to - from);
    }
    if (!p->fogged) return;
    fogged.n = 0;
    for (int i = from; i < to; i++) {
        GsVertex gv = t->v[i];
        gv.r = (u8)(p->fog_color >> 16);
        gv.g = (u8)(p->fog_color >> 8);
        gv.b = (u8)p->fog_color;
        gv.a = (u8)((255 - gv.fog) / 2);
        push(&fogged, gv);
    }
    render_set_source(source_id(a, a->frame, p->mesh, pass + 4), p->view);
    rdraw(GS_TRIANGLES, &p->fog, fogged.v, fogged.n);
}

typedef struct Merged {
    u32 red, green, blue;
    u32 image;
} Merged;

static Merged *g_merged;
static u32 g_nmerged;
static GuestPtr g_merged_level;

static const GsTexture *channel(u32 raster) {
    const Texture *map = handle_object(OBJECT_RASTER, raster);
    u32 image = map ? (map->lightmap_image ? map->lightmap_image : map->image) : 0;
    const GsTexture *t = render_texture(image);
    return t && t->texels ? t : NULL;
}

static u32 merged_lightmap(s32 index, const LightmapSlot *slot) {
    const GuestPtr level = *GUEST(GuestPtr, LEVEL_LIGHTMAPS);
    const u32 count = *GUEST(u32, LEVEL_LIGHTMAP_COUNT);
    if (level != g_merged_level || count != g_nmerged) {
        for (u32 i = 0; i < g_nmerged; i++)
            if (g_merged[i].image) render_texture_free(g_merged[i].image);
        free(g_merged);
        g_merged = calloc(count ? count : 1, sizeof *g_merged);
        g_nmerged = count;
        g_merged_level = level;
    }
    if (index < 0 || (u32)index >= g_nmerged) return 0;
    Merged *m = &g_merged[index];
    if (m->red == slot->red && m->green == slot->green && m->blue == slot->blue) return m->image;
    if (m->image) render_texture_free(m->image);
    *m = (Merged){slot->red, slot->green, slot->blue, 0};
    const GsTexture *c[3] = {channel(slot->red), channel(slot->green), channel(slot->blue)};
    if (!c[0] || !c[1] || !c[2]) return 0;
    const int w = c[0]->width, h = c[0]->height;
    if (c[1]->width != w || c[2]->width != w || c[1]->height != h || c[2]->height != h) return 0;
    u32 *texels = malloc(sizeof *texels * (size_t)w * (size_t)h);
    for (int k = 0; k < w * h; k++)
        texels[k] = c[0]->texels[k] >> 24 | (c[1]->texels[k] >> 24) << 8 | (c[2]->texels[k] >> 24) << 16 | 0x80u << 24;
    m->image = render_texture_new(w, h, texels);
    free(texels);
    return m->image;
}

void draw_atomic(const Atomic *a, const Frame *frame, bool lightmapped) {
    const Camera *cam = camera_current();
    const ClumpGeometry *g = a->geometry;
    if (!cam || !g || !g->positions) return;
    g_cam = g_shadow ? NULL : cam;
    const Material *material = handle_object(OBJECT_MATERIAL, a->material);
    const Texture *texture = material ? handle_object(OBJECT_TEXTURE, material->texture) : NULL;
    if (texture && !texture->image) texture = NULL;
    if (g_shadow && !g_shadow->textured) texture = NULL;
    if (g_shadow) lightmapped = false;
    const LightmapSlot *slot = NULL;
    if (lightmapped && a->lightmap >= 0)
        slot = &GUEST(LightmapSlot, *GUEST(GuestPtr, LEVEL_LIGHTMAPS))[a->lightmap];
    const float(*lm_uv)[2] = slot && slot->uvs ? (const float(*)[2])(RAM + slot->uvs) : NULL;

    float world[4][3];
    frame_world(frame ? frame : a->frame, world);
    const bool fog = render_fog_on() && render_state.fog_end > render_state.fog_start;
    const float scale = texture ? 128.0f : 255.0f;
    const u8 *mc = g_shadow ? g_shadow->color : material ? material->color : (const u8[4]){255, 255, 255, 255};

    static CamVertex *v;
    static int cap;
    if (g->vertex_count > cap) {
        cap = g->vertex_count;
        v = realloc(v, sizeof *v * (size_t)cap);
    }
    for (int i = 0; i < g->vertex_count; i++) {
        float w[3], c[3];
        v[i].id = (u32)i + 1;
        transform(world, g->positions[i], w);
        to_camera(cam, w, &v[i].x);
        v[i].u = g->uv ? g->uv[i][0] : 0.0f;
        v[i].v = g->uv ? g->uv[i][1] : 0.0f;
        v[i].lu = lm_uv ? lm_uv[i][0] : 0.0f;
        v[i].lv = lm_uv ? lm_uv[i][1] : 0.0f;
        float z = v[i].z;
        v[i].fog = fog ? 255.0f * clamp01((render_state.fog_end - z) / (render_state.fog_end - render_state.fog_start)) : 255.0f;
        if (slot) {
            v[i].r = v[i].g = v[i].b = v[i].a = 127.0f;
            continue;
        }
        float rgb[3] = {0, 0, 0}, alpha = 1.0f;
        if (g->prelit) {
            for (int k = 0; k < 3; k++) rgb[k] = g->prelit[i][k] / 255.0f;
            alpha = g->prelit[i][3] / 255.0f;
        }
        if ((g->flags & GEOMETRY_LIGHT) && g->normals) {
            float n[3];
            rotate(world, g->normals[i], n);
            float length = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
            if (length > 0.0f)
                for (int k = 0; k < 3; k++) n[k] /= length;
            light_vertex(g, w, n, rgb);
        } else if (!g->prelit) {
            rgb[0] = rgb[1] = rgb[2] = 1.0f;
        }
        for (int k = 0; k < 3; k++) c[k] = clamp01(rgb[k]) * mc[k] / 255.0f * scale;
        v[i].r = c[0];
        v[i].g = c[1];
        v[i].b = c[2];
        v[i].a = alpha * mc[3] / 255.0f * 128.0f;
    }

    GsView seen;
    const GsView *view = camera_view(cam, &seen);
    if (view) memcpy(seen.origin, world[3], sizeof seen.origin);
    for (int m = 0; m < g->mesh_count; m++) {
        const ClumpMesh *mesh = &g->meshes[m];
        static Triangles clipped;

        GsState base = render_gs_state();
        if (g_shadow) base.target = g_shadow->target;
        if (texture) textured(&base, texture);
        const u32 lightmap = slot && view && !base.target && !base.blend ? merged_lightmap(a->lightmap, slot) : 0;
        if (!lightmap) {
            g_tris.n = 0;
            mesh_triangles(&g_tris, slot ? &clipped : NULL, cam, g, mesh, v);
            if (!g_tris.n && !(slot && clipped.n)) continue;
        }
        if (lightmap) {
            static Triangles all;
            all.n = 0;
            all.with_ext = true;
            mesh_triangles(&all, NULL, cam, g, mesh, v);
            if (!all.n) continue;
            GsState s = base;
            s.lightmap = lightmap;
            render_set_source(source_id(a, a->frame, m, 0), view);
            rdraw_ext(GS_TRIANGLES, &s, all.v, all.x, all.n);
            continue;
        }
        if (slot) base.fog = false;
        if (!slot) {
            render_set_source(source_id(a, a->frame, m, 0), view);
            draw(&base);
            continue;
        }

        static Triangles lm, lm_clipped;
        for (int i = 0; i < g->vertex_count; i++) {
            v[i].u = v[i].lu;
            v[i].v = v[i].lv;
        }
        lm.n = 0;
        mesh_triangles(&lm, &lm_clipped, cam, g, mesh, v);
        for (int i = 0; i < g->vertex_count; i++) {
            v[i].u = g->uv ? g->uv[i][0] : 0.0f;
            v[i].v = g->uv ? g->uv[i][1] : 0.0f;
        }
        Passes p = {.base = base, .atomic = a, .mesh = m, .view = view};
        p.lightmaps = lm.n == g_tris.n && lm_clipped.n == clipped.n;
        const u32 rasters[3] = {slot->red, slot->green, slot->blue};
        const u32 masks[3] = {0xFFFFFF00u, 0xFFFF00FFu, 0xFF00FFFFu};
        for (int c = 0; c < 3; c++) {
            const Texture *map = handle_object(OBJECT_RASTER, rasters[c]);
            u32 image = map ? (map->lightmap_image ? map->lightmap_image : map->image) : 0;
            p.have[c] = image != 0;
            GsState *s = &p.maps[c];
            s->tme = true;
            s->texture = image;
            s->tfx = GS_MODULATE;
            s->tcc = true;
            s->wrap_s = s->wrap_t = GS_CLAMP;
            s->mmag = 1;
            s->mmin = 0;
            s->lcm = true;
            s->blend = true;
            s->a = GS_CD;
            s->b = GS_ZERO;
            s->c = GS_AS;
            s->d = GS_ZERO;
            s->ztst = GS_Z_GEQUAL;
            s->fbmsk = masks[c];
            s->colclamp = true;
        }
        p.fogged = fog;
        p.fog_color = render_state.fog_color;
        p.fog.blend = true;
        p.fog.a = GS_CS;
        p.fog.b = GS_CD;
        p.fog.c = GS_AS;
        p.fog.d = GS_CD;
        p.fog.ztst = GS_Z_GEQUAL;
        p.fog.colclamp = true;

        for (int k = 0, done = 0; k < g_tris.nstarts; k++) {
            passes(&p, &g_tris, &lm, start(&g_tris, k), start(&g_tris, k + 1), 0, true);
            if (k % 2 == 0 && k + 1 < g_tris.nstarts) continue;
            const int upto = k + 1 < g_tris.nstarts ? g_tris.clipped[k + 1] : clipped.nstarts;
            const int from = start(&clipped, done), to = start(&clipped, upto);
            if (from < to) {
                render_set_source(source_id(a, a->frame, m, 5), view);
                rdraw(GS_TRIANGLES, &base, clipped.v + from, to - from);
            }
            for (; done < upto; done++)
                passes(&p, &clipped, &lm_clipped, start(&clipped, done), start(&clipped, done + 1), 5, false);
        }
    }
}

void draw_quads(const Quad *quads, int count) {
    const Camera *cam = camera_current();
    if (!cam || count <= 0) return;
    g_cam = g_shadow ? NULL : cam;
    const bool fog = render_fog_on() && render_state.fog_end > render_state.fog_start;
    g_tris.n = 0;
    for (int q = 0; q < count; q++) {
        CamVertex v[4] = {0};
        for (int i = 0; i < 4; i++) {
            const QuadVertex *p = &quads[q].v[i];
            to_camera(cam, p->pos, &v[i].x);
            v[i].u = p->u;
            v[i].v = p->v;
            v[i].lu = v[i].lv = 0.0f;
            v[i].id = 0;
            v[i].r = render_gs_channel(p->color[0]);
            v[i].g = render_gs_channel(p->color[1]);
            v[i].b = render_gs_channel(p->color[2]);
            v[i].a = render_gs_channel(p->color[3]);
            float z = v[i].z;
            v[i].fog = fog ? 255.0f * clamp01((render_state.fog_end - z) / (render_state.fog_end - render_state.fog_start))
                           : 255.0f;
        }
        triangle(&g_tris, cam, &v[1], &v[0], &v[2]);
        triangle(&g_tris, cam, &v[0], &v[2], &v[3]);
    }
    GsState s = render_gs_state_textured();
    GsView view;
    render_set_source(GS_SOURCE_NONE, camera_view(cam, &view));
    draw(&s);
}

void draw_world_triangles(const QuadVertex *v, int count) {
    const Camera *cam = camera_current();
    if (!cam || count < 3) return;
    g_cam = g_shadow ? NULL : cam;
    const bool fog = render_fog_on() && render_state.fog_end > render_state.fog_start;
    g_tris.n = 0;
    for (int i = 0; i + 2 < count; i += 3) {
        CamVertex c[3] = {0};
        for (int k = 0; k < 3; k++) {
            const QuadVertex *p = &v[i + k];
            to_camera(cam, p->pos, &c[k].x);
            c[k].u = p->u;
            c[k].v = p->v;
            c[k].lu = c[k].lv = 0.0f;
            c[k].id = 0;
            c[k].r = render_gs_channel(p->color[0]);
            c[k].g = render_gs_channel(p->color[1]);
            c[k].b = render_gs_channel(p->color[2]);
            c[k].a = render_gs_channel(p->color[3]);
            float z = c[k].z;
            c[k].fog = fog ? 255.0f * clamp01((render_state.fog_end - z) / (render_state.fog_end - render_state.fog_start))
                           : 255.0f;
        }
        triangle(&g_tris, cam, &c[0], &c[1], &c[2]);
    }
    GsState s = render_gs_state_textured();
    GsView view;
    render_set_source(GS_SOURCE_NONE, camera_view(cam, &view));
    draw(&s);
}
