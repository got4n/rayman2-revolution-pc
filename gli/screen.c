#include "game_vars.h"
#include <stdlib.h>
#include <string.h>

#include "gli.h"
#include "render.h"

#define DEFAULT_MATERIAL GAME_VAR(gs_hDefaultMaterial)
#define TEXT_GRADIENT GAME_VAR(inv_GLI_cTextGradientMode)
#define TEXT_GRADIENT_COLOR GAME_VAR(inv_GLI_ucTextGradientRed)

typedef struct ScreenVertex {
    float x, y, z, q, u, v;
    float color[4];
} ScreenVertex;

static ScreenVertex g_quad[4];
static ScreenVertex g_prepared[4];
static bool g_prepared_waits;
static bool g_prepared_drawn_in_between;
int revo_drawing_in_between(void);

void screen_init(void) {
    float z = cameras_near_screen_z(), q = fdiv(1.0f, cameras_main_near());
    static const float corners[4][2] = {{1, 1}, {0, 1}, {0, 0}, {1, 0}};
    for (int i = 0; i < 4; i++) {
        g_quad[i] = (ScreenVertex){.z = z, .q = q, .u = corners[i][0], .v = corners[i][1],
                                   .color = {255.0f, 255.0f, 255.0f, 255.0f}};
        g_prepared[i] = g_quad[i];
    }
}

static void draw_quad(const ScreenVertex quad[4], void (*fit)(GsVertex *, int)) {
    static const int order[6] = {2, 1, 0, 3, 2, 0};
    GsState s = render_gs_state_textured();
    GsVertex v[6];
    for (int i = 0; i < 6; i++) {
        const ScreenVertex *p = &quad[order[i]];
        v[i] = (GsVertex){.x = p->x, .y = p->y, .z = p->z, .q = p->q, .s = fmul(p->u, p->q), .t = fmul(p->v, p->q),
                          .r = render_gs_channel(p->color[0]), .g = render_gs_channel(p->color[1]),
                          .b = render_gs_channel(p->color[2]), .a = render_gs_channel(p->color[3]), .fog = 255};
    }
    if (fit) fit(v, 6);
    render_draw(GS_TRIANGLES, &s, v, 6);
}

typedef struct Im2DVertex {
    float x, y, z;
    u32 unk0C;
    float u, v, q;
    u32 unk1C;
    float color[4];
    u32 unk30[4];
} Im2DVertex;

_Static_assert(sizeof(Im2DVertex) == 0x40, "Im2DVertex");

void screen_im2d_triangles(GuestPtr vertices, GuestPtr indices, int count) {
    GsState s = render_gs_state_textured();
    GsVertex *v = calloc((size_t)count, sizeof *v);
    for (int i = 0; i < count; i++) {
        const Im2DVertex *p = &GUEST(Im2DVertex, vertices)[GUEST(u16, indices)[i]];
        v[i] = (GsVertex){.x = p->x, .y = p->y, .z = p->z, .q = p->q, .s = fmul(p->u, p->q), .t = fmul(p->v, p->q),
                          .r = render_gs_channel(p->color[0]), .g = render_gs_channel(p->color[1]),
                          .b = render_gs_channel(p->color[2]), .a = render_gs_channel(p->color[3]), .fog = 255};
    }
    render_2d_widescreen(v, count - count % 3);
    bool perspective = false;
    for (int i = 0; i < count - count % 3 && !perspective; i++) perspective = v[i].q != 1.0f;
    GsView view;
    render_set_source(GS_SOURCE_NONE, perspective ? draw_camera_view(&view) : NULL);
    render_draw(GS_TRIANGLES, &s, v, count - count % 3);
    free(v);
}

void screen_quad(float x0, float y0, float x1, float y1, const float rgba[4]) {
    g_quad[0].x = g_quad[3].x = x1;
    g_quad[0].y = g_quad[1].y = y0;
    g_quad[1].x = g_quad[2].x = x0;
    g_quad[2].y = g_quad[3].y = y1;
    for (int i = 0; i < 4; i++) memcpy(g_quad[i].color, rgba, sizeof g_quad[i].color);
    draw_quad(g_quad, NULL);
}

typedef struct ScreenSize {
    u8 unk00[8];
    s32 height, width;
} ScreenSize;

static void use_material(GuestPtr material, u32 flags) {
    GliContext *ctx = gli_context();
    ctx->material = material;
    *GUEST(u32, material) = flags;
    ctx->draw_flags = (u32)(s32)-0x409;
    ctx->render_flags = (u32)(s32)-0x409;
}

static void forget_blend(void) {
    if (*GUEST(u8, DRIVER_BLEND) != BLEND_UNKNOWN) state_mode(BLEND_UNKNOWN);
}

void gli_2d_letter(GuestPtr size, GuestPtr material, float x0, float y0, float x1, float y1) {
    const ScreenSize *ss = GUEST(ScreenSize, size);
    float sx = fdiv((float)ss->width, 100.0f), sy = fdiv((float)ss->height, 100.0f);
    GliContext *ctx = gli_context();
    s16 fog_was = (s16)ctx->fog_on;
    GLI_vFogOff();
    g_quad[3].x = fmul(x1, sx);
    g_quad[1].y = fmul(y0, sy);
    g_quad[2].x = fmul(x0, sx);
    g_quad[3].y = fmul(y1, sy);
    g_quad[0].x = g_quad[3].x;
    g_quad[1].x = g_quad[2].x;
    g_quad[0].y = g_quad[1].y;
    g_quad[2].y = g_quad[3].y;
    use_material(material, 0xFFFFFBF7u);
    forget_blend();
    GLI_vDoMaterialSelection();
    ctx = gli_context();
    ctx->later_pass = 1;
    if (ctx->state_flags & STATE_NOT_THIS_PASS) state_select_texture();
    const float alpha = ctx->vertex_color[3];
    if (*GUEST(s8, TEXT_GRADIENT)) {
        const u8 *below = GUEST(u8, TEXT_GRADIENT_COLOR);
        for (int i = 0; i < 2; i++) {
            ScreenVertex *top = &g_quad[i], *bottom = &g_quad[2 + i];
            top->color[0] = top->color[1] = top->color[2] = 255.0f;
            top->color[3] = alpha;
            for (int k = 0; k < 3; k++) bottom->color[k] = below[k];
            bottom->color[3] = alpha;
        }
    } else {
        for (int i = 0; i < 4; i++) {
            g_quad[i].color[0] = ctx->vertex_color[2];
            g_quad[i].color[1] = ctx->vertex_color[1];
            g_quad[i].color[2] = ctx->vertex_color[0];
            g_quad[i].color[3] = alpha;
        }
    }
    const u32 icon = icons_image(material);
    if (icon) {
        render_texture_set(icon);
        for (int i = 0; i < 4; i++) g_quad[i].color[0] = g_quad[i].color[1] = g_quad[i].color[2] = 255.0f;
    }
    draw_quad(g_quad, render_2d_hud);
    if (fog_was) GLI_vFogOn();
}

void gli_2d_prepare_rectangle(GuestPtr p0, GuestPtr p1, GuestPtr size, float alpha, float grey) {
    const ScreenSize *ss = GUEST(ScreenSize, size);
    float sx = fdiv((float)ss->width, 100.0f), sy = fdiv((float)ss->height, 100.0f);
    const float *a = GUEST(float, p0), *b = GUEST(float, p1);
    float right = b[0], left = a[0];
    if (render_hud_at_sides() && right != 100.0f && right >= 90.0f) {
        const float narrow = 1.0f / widescreen_factor();
        right = 90.0f - (90.0f - right) / narrow;
        left = left * narrow;
    }
    float x1 = fmul(right, sx), y0 = fmul(a[1], sy), x0 = fmul(left, sx), y1 = fmul(b[1], sy);
    g_prepared[0].x = g_prepared[3].x = x1;
    g_prepared[0].y = g_prepared[1].y = y0;
    g_prepared[1].x = g_prepared[2].x = x0;
    g_prepared[2].y = g_prepared[3].y = y1;
    float c = (float)(fcvtw(grey) & 0xFF), alpha_byte = (float)(fcvtw(alpha) & 0xFF);
    for (int i = 0; i < 4; i++) {
        g_prepared[i].color[0] = g_prepared[i].color[1] = g_prepared[i].color[2] = c;
        g_prepared[i].color[3] = alpha_byte;
    }
    g_prepared_waits = true;
    g_prepared_drawn_in_between = false;
}

void gli_2d_render_prepared(void) {
    if (!g_prepared_waits) return;
    const bool in_between = revo_drawing_in_between();
    if (in_between && g_prepared_drawn_in_between) return;
    GliContext *ctx = gli_context();
    s16 fog_was = (s16)ctx->fog_on;
    use_material(*GUEST(GuestPtr, DEFAULT_MATERIAL), 0xFFFFFBF7u);
    forget_blend();
    GLI_vFogOff();
    state_z_write(false);
    GLI_vDoMaterialSelection();
    gli_context()->later_pass = 1;
    state_select_texture();
    draw_quad(g_prepared, render_2d_hud);
    if (fog_was) GLI_vFogOn();
    state_z_write(true);
    if (in_between) g_prepared_drawn_in_between = true;
    else g_prepared_waits = false;
}
