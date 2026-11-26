#include "game_vars.h"
#include <math.h>
#include <string.h>

#include "clump.h"
#include "gli.h"
#include "handles.h"
#include "render.h"

#define DRAW_PASS GAME_VAR(inv_SCT_ucDrawPass)
#define SCROLL_TIME GAME_VAR(gs_fCoefOfMultiplicationForAnimatedTexture)
#define FRAME_NUMBER GAME_VAR(gs_lActualRefreshNumberForScolledTextures)
#define FRAME_REPEATED GAME_VAR(g_bTakeCareOfAnimationCoordination)
#define NO_ANIMATION GAME_VAR(byte_2538AA)
#define UNLIT_COLOR GAME_VAR(qword_38A3A0)
#define FADE_COLOR GAME_VAR(qword_38A3B0)

enum { OBJECT_FOLLOWS_MATRIX = 0x0B };
enum { OBJECT_LIT = OBJECT_FRAME_PER_MESH };
enum { REQUEST_KEEP_FRAME = 0x01, REQUEST_UNLIT = 0x10 };
enum { RESET_SORTED = 0x8000 };
enum { RENDER_NO_GLOBAL_ALPHA = 0x08 };

#define MAX_REQUESTS 300

typedef struct Request {
    Atomic *atomic;
    u32 comparison;
    float alpha;
    u8 mode, flags;
    Frame frame;
    LightList lights;
    GuestPtr fog;
} Request;

static Request g_requests[MAX_REQUESTS];
static int g_nrequests;

#define MAX_QUAD_REQUESTS 150

typedef struct QuadRequest {
    Quad quad;
    GuestPtr texture;
    GuestPtr fog;
} QuadRequest;

static QuadRequest g_quad_requests[MAX_QUAD_REQUESTS];
static int g_nquad_requests;

static int g_order[MAX_REQUESTS + MAX_QUAD_REQUESTS];

static u8 *driver_blend(void) { return GUEST(u8, DRIVER_BLEND); }

static void use_fog(void) {
    GliContext *ctx = gli_context();
    if (ctx->fog_on) {
        if (ctx->applied_fog != ctx->fog) {
            state_apply_fog(ctx->fog);
            ctx->applied_fog = ctx->fog;
        }
        state_fog(true);
    } else {
        state_fog(false);
    }
}

static u8 sorted_mode(void) {
    const GliMaterial *m = GUEST(GliMaterial, gli_context()->material);
    if (m->texture) {
        const GliTexture *t = GUEST(GliTexture, m->texture);
        if (t->ready) return t->flags & TEXTURE_ADDITIVE ? BLEND_ADD : BLEND_TEXTURE;
    }
    return BLEND_UNTEXTURED;
}

static u8 unsorted_mode(void) {
    GliContext *ctx = gli_context();
    const GliMaterial *m = GUEST(GliMaterial, ctx->material);
    const GliTexture *t = m->texture ? GUEST(GliTexture, m->texture) : NULL;
    const bool textured = t && t->ready;
    ctx->later_pass = 0;
    ctx->state_flags &= ~(u32)RESET_SORTED;
    state_z_write(true);
    use_fog();
    state_z_test(ctx->draw_flags & DRAW_Z_TEST);
    state_z_write(ctx->draw_flags & DRAW_Z_WRITE);
    if (textured) {
        if (t->flags & TEXTURE_BLENDED) return BLEND_MATERIAL;
        if (ctx->pass_flags & 2) return BLEND_TEXTURE;
    }
    return BLEND_OPAQUE;
}

static void material_alpha(const Atomic *a, float alpha) {
    Material *m = handle_object(OBJECT_MATERIAL, a->material);
    if (m) m->color[3] = (u8)fcvtw(fmul(alpha, 255.0f));
}

static bool is_second_camera(void) { return camera_current() == camera_second(); }

static void scroll(const GeometricObject *go, int element, Atomic *a) {
    const GliMaterial *m = GUEST(GliMaterial, GUEST(GeometricElement, GUEST(GuestPtr, go->elements)[element])->material);
    if (!(m->scroll_flags & MATERIAL_SCROLL_ANY)) return;
    ClumpGeometry *g = level_geometry(a);
    if (!g || !g->uv) return;
    DriverObject *d = GUEST(DriverObject, go->driver_object);
    float *offset = GUEST(float, d->scroll_offsets + (u32)element * 8);
    float time = *GUEST(float, SCROLL_TIME);
    double whole;
    float u = (float)modf((double)fmul(fmul(m->scroll_u, time), 0.5f), &whole);
    u = fadd(u, u);
    float v = (float)modf((double)fmul(fmul(m->scroll_v, time), 0.5f), &whole);
    v = fadd(v, v);
    float du = fsub(u, offset[0]), dv = fsub(v, offset[1]);
    for (int i = g->vertex_count - 1; i >= 0; i--) {
        g->uv[i][0] = fadd(g->uv[i][0], du);
        g->uv[i][1] = fadd(g->uv[i][1], dv);
    }
    offset[0] = u;
    offset[1] = v;
}

static void animate(const GeometricObject *go) {
    if (*GUEST(u8, NO_ANIMATION)) return;
    const DriverObject *d = GUEST(DriverObject, go->driver_object);
    u32 frame = *GUEST(u32, FRAME_NUMBER);
    int mesh = 0;
    for (int e = 0; e < go->element_count; e++) {
        if (GUEST(s16, go->element_types)[e] != ELEMENT_MESH) continue;
        GuestPtr material = GUEST(GeometricElement, GUEST(GuestPtr, go->elements)[e])->material;
        GliMaterial *m = GUEST(GliMaterial, material);
        if (m->frame <= frame && m->texture) {
            Atomic *a = handle_object(OBJECT_ATOMIC, GUEST(u32, d->atomics)[mesh]);
            if (a) scroll(go, e, a);
            GLI_xRefreshAnimatedTexture(material);
        }
        mesh++;
        if (!*GUEST(u8, FRAME_REPEATED)) m->frame = frame;
    }
}

void gli_object_focal(float focal) {
    GliContext *ctx = gli_context();
    ctx->focal = focal;
    ctx->inverse_focal = fdiv(1.0f, focal);
}

void gli_lock_lightmap_rasters(void) {}

static void draw_now(Atomic *a) { draw_atomic(a, NULL, a->lightmap >= 0); }

void gli_send_object(void) {
    GliContext *ctx = gli_context();
    const GeometricObject *go = GUEST(GeometricObject, ctx->geometric_object);
    DriverObject *d = GUEST(DriverObject, go->driver_object);
    Frame *frame = handle_object(OBJECT_FRAME, d->frame);
    if (frame && (go->flags & OBJECT_FOLLOWS_MATRIX))
        frame_from_matrix(frame, GUEST(PosMatrix, ctx->current_matrix)->m);
    animate(go);

    int atomic_index = 0;
    for (int e = 0; e < go->element_count; e++) {
        if (GUEST(s16, go->element_types)[e] != ELEMENT_MESH) continue;
        GuestPtr material = GUEST(GeometricElement, GUEST(GuestPtr, go->elements)[e])->material;
        ctx->material = material;
        const GliMaterial *m = GUEST(GliMaterial, material);
        ctx->texture = m->texture;
        const GliTexture *t = m->texture ? GUEST(GliTexture, m->texture) : NULL;
        const bool textured = t && t->ready;
        const u8 pass = *GUEST(u8, DRAW_PASS);
        if (textured && !(go->flags & OBJECT_LIT)) {
            if (t->flags & TEXTURE_BLENDED) {
                if (pass == 0) {
                    atomic_index++;
                    continue;
                }
            } else if (pass == ELEMENT_MESH) {
                atomic_index++;
                continue;
            }
        }

        ctx->later_pass = 0;
        if (ctx->pass_flags != 0) ctx->pass_flags = 3;
        ctx->draw_flags = m->type & ctx->render_flags;
        u8 *mode = GUEST(u8, d->atomic_states) + atomic_index;
        bool sorted;
        if (ctx->pass_flags == 0 && textured && (t->flags & TEXTURE_SORTED)) sorted = true;
        else if (ctx->draw_flags & DRAW_IGNORE_GLOBAL_ALPHA) sorted = false;
        else sorted = ctx->global_alpha < 0.98f;
        if (sorted) {
            ctx->state_flags |= RESET_SORTED;
            if (textured && !(t->flags & TEXTURE_NOT_DEFERRED)) ctx->later_pass = 1;
            *mode = sorted_mode();
        } else {
            *mode = unsorted_mode();
        }

        Atomic *a = handle_object(OBJECT_ATOMIC, GUEST(u32, d->atomics)[atomic_index]);
        atomic_index++;
        if (!a) continue;
        if (!sorted) {
            material_alpha(a, 1.0f);
            state_mode(*mode);
            if (!is_second_camera()) {
                if ((go->flags & OBJECT_LIT) && (!textured || !(t->flags & TEXTURE_UNLIT))) lights_of_viewport_in();
                else gli_ambient_color(UNLIT_COLOR);
            }
            draw_now(a);
            if (!is_second_camera()) lights_all_out();
        } else if (!is_second_camera()) {
            if (g_nrequests >= MAX_REQUESTS) continue;
            g_order[g_nrequests + g_nquad_requests] = g_nrequests;
            Request *r = &g_requests[g_nrequests++];
            r->atomic = a;
            r->comparison = ctx->draw_flags;
            r->alpha = (ctx->render_flags & RENDER_NO_GLOBAL_ALPHA) ? 1.0f : ctx->global_alpha;
            r->mode = *mode;
            r->flags = d->frames | (textured && (t->flags & TEXTURE_UNLIT) ? REQUEST_UNLIT : 0);
            r->fog = ctx->fog_on ? ctx->fog : 0;
            if (!(r->flags & REQUEST_KEEP_FRAME) && a->frame && a->frame->parent) {
                r->frame = *a->frame->parent;
                r->frame.parent = NULL;
            }
            lights_of_viewport(&r->lights);
        } else {
            material_alpha(a, ctx->global_alpha);
            state_mode(*mode);
            state_z_write(false);
            draw_now(a);
        }
    }
}

static void draw_request(Request *r) {
    lights_all_out();
    if ((r->flags & REQUEST_KEEP_FRAME) || r->mode == BLEND_ADD || (r->flags & REQUEST_UNLIT))
        gli_ambient_color(UNLIT_COLOR);
    else
        lights_put_in(&r->lights);
    GliContext *ctx = gli_context();
    ctx->draw_flags = r->comparison;
    ctx->fog_on = r->mode == BLEND_ADD ? 0 : r->fog != 0;
    ctx->fog = r->fog;
    use_fog();
    state_z_test(ctx->draw_flags & DRAW_Z_TEST);
    material_alpha(r->atomic, r->alpha);
    bool lightmapped = r->atomic->lightmap >= 0;
    if (lightmapped && r->alpha != 1.0f) {
        gli_ambient_color(FADE_COLOR);
        lightmapped = false;
    }
    state_mode(r->mode);
    draw_atomic(r->atomic, (r->flags & REQUEST_KEEP_FRAME) ? NULL : &r->frame, lightmapped);
}

void queue_clear_objects(void) { g_nrequests = 0; }

void queue_init(const Quad *quad) {
    for (int i = 0; i < MAX_QUAD_REQUESTS; i++) {
        g_quad_requests[i].quad = *quad;
        g_quad_requests[i].texture = g_quad_requests[i].fog = 0;
    }
}

Quad *queue_quad(GuestPtr texture) {
    if (g_nquad_requests >= MAX_QUAD_REQUESTS) return NULL;
    GliContext *ctx = gli_context();
    QuadRequest *r = &g_quad_requests[g_nquad_requests];
    r->texture = texture;
    r->fog = ctx->fog_on ? ctx->fog : 0;
    g_order[g_nrequests + g_nquad_requests] = ~g_nquad_requests;
    g_nquad_requests++;
    return &r->quad;
}

void gli_draw_queued(GuestPtr viewport) {
    const int count = g_nrequests + g_nquad_requests;
    if (!count) return;
    if (*driver_blend() != BLEND_UNKNOWN) state_mode(BLEND_UNKNOWN);
    state_z_write(false);
    state_z_test(false);
    for (int i = 0; i < count; i++) {
        if (g_order[i] >= 0) {
            draw_request(&g_requests[g_order[i]]);
            continue;
        }
        QuadRequest *r = &g_quad_requests[~g_order[i]];
        GliContext *ctx = gli_context();
        ctx->fog_on = r->fog != 0;
        ctx->fog = r->fog;
        use_fog();
        sprites_draw_quad(viewport, &r->quad, r->texture);
    }
    lights_all_out();
    g_nrequests = 0;
    g_nquad_requests = 0;
}
