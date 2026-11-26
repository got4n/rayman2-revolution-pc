#include "game_vars.h"
#include <string.h>

#include "gli.h"
#include "render.h"

#define HALOS GAME_VAR(inv_RW3_p_stLightHalos)
#define HALO_LIST GAME_VAR(inv_RW3_p_ucQueuedHalos)
#define HALO_MATERIAL GAME_VAR(inv_GLI_p_stHaloMaterial)
#define HALO_FADE_FRAMES 6

typedef struct Halo {
    float x, y;
    float half_width, half_height;
    u32 color;
    u32 start_frame, last_frame;
    u32 visible;
} Halo;

typedef struct HaloCamera {
    u8 unk00[0x60];
    float near_clip;
    u8 unk64[0x20];
    float focal_x, focal_y;
} HaloCamera;

_Static_assert(sizeof(Halo) == 0x20 && offsetof(HaloCamera, focal_x) == 0x84, "halos");

static u8 g_count;

static Halo *halo(u8 index) { return &GUEST(Halo, *GUEST(GuestPtr, HALOS))[index]; }

static void transform(const float *m, const float in[3], float out[3]) {
    for (int k = 0; k < 3; k++) {
        float acc = fmul(m[k], in[0]);
        acc = fadd(acc, fmul(m[4 + k], in[1]));
        out[k] = fadd(fadd(acc, fmul(m[8 + k], in[2])), m[12 + k]);
    }
}

static void set_visible(Halo *h, u32 visible, u32 frame) {
    if (h->visible == visible) return;
    h->start_frame = frame;
    h->visible = visible;
}

void halos_record(GuestPtr object, GuestPtr element) {
    const u8 index = *GUEST(u8, element + 0x0C);
    const float width = *GUEST(float, element + 4), height = *GUEST(float, element + 8);
    const u32 frame = inv_fn_ulGetEngineTrameNumber();
    float position[3];
    memcpy(position, GUEST(float, *GUEST(GuestPtr, object + 0x24)), 12);
    Halo *h = halo(index);
    const GliContext *ctx = gli_context();
    float screen[3];
    transform(GUEST(float, ctx->geometry_matrix), position, screen);
    const HaloCamera *cam = GUEST(HaloCamera, ctx->camera);
    if (screen[2] < cam->near_clip) return;
    const float one = 1.0f, half = 0.5f;
    screen[2] = fdiv(one, fadd(screen[2], screen[2]));
    float sx = fmul(cam->focal_x, screen[2]);
    h->half_width = sx;
    float sy = fmul(cam->focal_y, screen[2]);
    h->half_height = sy;
    float u = fsub(half, fmul(screen[0], sx));
    h->x = u;
    float v = fsub(half, fmul(screen[1], sy));
    h->y = v;
    h->half_width = fmul(sx, width);
    h->half_height = fmul(sy, height);
    if (one < u || u < 0.0f || one < v || v < 0.0f) {
        set_visible(h, 0, frame);
    } else if (half < screen[2]) {
        set_visible(h, 1, frame);
    } else if ((g_count ^ frame) & 1) {
        GuestPtr camera = CAM_fn_hGetActiveCamera(1);
        const float *eye = GUEST(float, *GUEST(GuestPtr, camera + 0x20) + 0x30);
        float d[3] = {fsub(eye[0], position[0]), fsub(eye[1], position[1]), fsub(eye[2], position[2])};
        float length = fsqrt(fadd(fadd(fmul(d[0], d[0]), fmul(d[1], d[1])), fmul(d[2], d[2])));
        if (length != 0.0f) {
            float inverse = fdiv(one, length);
            for (int k = 0; k < 3; k++) d[k] = fmul(inverse, d[k]);
        }
        GuestPtr segment = game_stack_push(24);
        memcpy(GUEST(float, segment), eye, 12);
        for (int k = 0; k < 3; k++) GUEST(float, segment)[3 + k] = fadd(position[k], d[k]);
        u32 visible = inv_COL_fn_bIntersectSegmentWithLevel(segment) == 0;
        game_stack_pop(24);
        set_visible(h, visible, frame);
    }
    GUEST(u8, *GUEST(GuestPtr, HALO_LIST))[g_count] = index;
    g_count++;
}

static void halo_quad(GuestPtr viewport, u32 color, float x0, float y0, float x1, float y1) {
    const float w = (float)*GUEST(s32, viewport + 0xC), h = (float)*GUEST(s32, viewport + 8);
    const float rgba[4] = {(float)(color >> 16 & 0xFF), (float)(color >> 8 & 0xFF), (float)(color & 0xFF),
                           (float)(color >> 24)};
    screen_quad(fmul(x0, w), fmul(y0, h), fmul(x1, w), fmul(y1, h), rgba);
}

void gli_draw_halos(GuestPtr viewport) {
    if (g_count == 0) return;
    const u32 frame = inv_fn_ulGetEngineTrameNumber();
    GuestPtr material = *GUEST(GuestPtr, HALO_MATERIAL);
    GliContext *ctx = gli_context();
    ctx->material = material;
    u32 flags = GUEST(GliMaterial, material)->type;
    ctx->render_flags = flags;
    ctx->draw_flags = flags;
    state_quads();
    state_mode(BLEND_ADD);
    GLI_vFogOff();
    ctx = gli_context();
    ctx->material = material;
    ctx->texture = GUEST(GliMaterial, material)->texture;
    state_select_texture();
    const u8 *list = GUEST(u8, *GUEST(GuestPtr, HALO_LIST));
    for (u8 i = 0; i < g_count; i++) {
        Halo *h = halo(list[i]);
        float alpha;
        // An in-between frame (interpolation) draws the halos a second time in the same engine frame: carry on with
        // the fade then rather than restarting it, or the halos never get past 1/6 of their brightness.
        if (h->last_frame != frame && h->last_frame + 1 != frame) {
            if (h->visible) {
                h->start_frame = frame;
                h->last_frame = frame;
            }
            alpha = 0.0f;
        } else {
            u32 start = h->start_frame;
            h->last_frame = frame;
            if (frame < start + HALO_FADE_FRAMES) {
                float part = fdiv((float)(frame - start), 6.0f);
                alpha = h->visible ? part : fsub(1.0f, part);
            } else {
                alpha = h->visible ? 1.0f : 0.0f;
            }
        }
        if (alpha > 0.0f) {
            u32 color = h->color | (u32)fcvtw(fmul(alpha, 255.0f)) << 24;
            float bottom = fadd(h->y, h->half_height), right = fadd(h->x, h->half_width);
            halo_quad(viewport, color, fsub(h->x, h->half_width), fsub(h->y, h->half_height), right, bottom);
        }
    }
    g_count = 0;
    state_z_write(true);
    state_mode(BLEND_OPAQUE);
    if (*GUEST(u8, DRIVER_BLEND) != BLEND_UNKNOWN) state_mode(BLEND_UNKNOWN);
    twod_flush();
}
