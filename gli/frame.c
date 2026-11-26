#include "gli.h"
#include "render.h"

void gli_clear(s32 depth) {
    if (GLI_xIsGliInit() && depth) render_clear_depth();
}

void gli_clear_unless_paused(void) {
    if (!gli_context()->no_clear) gli_clear(1);
}

void gli_background(GuestPtr color) {
    const u8 *c = GUEST(u8, color);
    RenderState saved = render_state;
    state_z_test(true);
    state_z_write(false);
    render_texture_set(0);
    render_blend(false, saved.src, saved.dst);
    render_fog(false);
    render_background((u32)c[3] << 24 | (u32)c[0] << 16 | (u32)c[1] << 8 | c[2]);
    state_z_test(true);
    state_z_write(true);
    render_state.vertex_alpha = saved.vertex_alpha;
    render_state.fog = saved.fog;
}

void gli_show_frame(void) {
    if (!GLI_xIsGliInit()) return;
    GliContext *ctx = gli_context();
    ctx->pass_flags = 0;
    ctx->unk20B8 = 0;
    render_show_frame();
}

s32 gli_before_drawing(void) {
    if (world_exists()) queue_clear_objects();
    return 1;
}
