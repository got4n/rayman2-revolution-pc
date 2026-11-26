#include <stdlib.h>

#include "gliglou.h"
#include "gli.h"
#include "mods.h"
#include "render.h"

#define MAX_RECTANGLES 4

static GliRectangle g_rects[MAX_RECTANGLES];
static int g_count;

void gli_2d_reset(void) { g_count = 0; }

void gli_2d_add_rectangle(s32 x0, s32 y0, s32 x1, s32 y1) {
    if (g_count < MAX_RECTANGLES) g_rects[g_count++] = (GliRectangle){x0, y0, x1, y1};
}

u64 revo_frame_count(void);
static u64 g_bars_at;

int revo_letterboxed_within(unsigned vblanks) { return g_bars_at && revo_frame_count() - g_bars_at < vblanks; }
int revo_letterboxed(void) { return revo_letterboxed_within(8); }
unsigned long long revo_letterbox_age(void) { return g_bars_at ? revo_frame_count() - g_bars_at : ~0ull; }

void twod_flush(void) {
    if (!g_count) return;
    g_bars_at = revo_frame_count();
    if (revo_set(REVO_SET_NO_BARS) && widescreen_factor() != 1.0f) return;
    RenderState saved = render_state;
    render_texture_set(0);
    render_fog(false);
    render_z_test(false);
    render_z_write(true);
    state_forget_fog();
    render_rectangles(g_rects, g_count, cameras_near_screen_z());
    render_state.texture = saved.texture;
    render_state.fog = saved.fog;
    render_state.z_test = saved.z_test;
    render_state.z_write = saved.z_write;
}

s32 gli_2d_render(void) {
    state_z_write(true);
    state_blend(BLEND_OPAQUE);
    state_forget_blend();
    twod_flush();
    return 1;
}
