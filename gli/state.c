#include "gliglou.h"
#include "gli.h"
#include "handles.h"
#include "render.h"
#include "mods.h"

static u8 *driver_blend(void) { return GUEST(u8, DRIVER_BLEND); }
static u8 *driver_z_write(void) { return GUEST(u8, DRIVER_Z_WRITE); }
static u8 *driver_z_test(void) { return GUEST(u8, DRIVER_Z_TEST); }

static bool g_fog_on;
static u8 g_addressing = 0xFF;

void state_blend(enum Blend blend) {
    if (*driver_blend() == blend) return;
    *driver_blend() = (u8)blend;
    switch (blend) {
    case BLEND_OPAQUE: render_blend(false, FACTOR_ONE, FACTOR_ZERO); break;
    case BLEND_ADD: render_blend(true, FACTOR_SRC_ALPHA, FACTOR_ONE); break;
    case BLEND_TEXTURE:
    case BLEND_MATERIAL: render_blend(true, FACTOR_SRC_ALPHA, FACTOR_INV_SRC_ALPHA); break;
    default: break;
    }
}

static void z_switch(u8 *last, bool on, void (*set)(bool)) {
    if (on == (*last != 0)) return;
    *last = on;
    set(on);
}

void state_z_write(bool on) { z_switch(driver_z_write(), on, render_z_write); }
void state_z_test(bool on) { z_switch(driver_z_test(), on, render_z_test); }

void state_mode(u8 mode) {
    if (*driver_blend() == mode) return;
    switch (mode) {
    case BLEND_UNTEXTURED:
        render_blend(true, FACTOR_SRC_ALPHA, FACTOR_INV_SRC_ALPHA);
        render_texture_set(0);
        break;
    case BLEND_ADD: render_blend(true, FACTOR_SRC_ALPHA, FACTOR_ONE); break;
    case BLEND_OPAQUE: render_blend(false, FACTOR_ONE, FACTOR_ZERO); break;
    case BLEND_TEXTURE:
    case BLEND_MATERIAL: render_blend(true, FACTOR_SRC_ALPHA, FACTOR_INV_SRC_ALPHA); break;
    case 6: case 7: case 8:
        revo_log("[gli] blend mode %u not drawn natively yet", mode);
        break;
    default:
        *driver_blend() = BLEND_UNKNOWN;
        state_z_test(true);
        state_z_write(true);
        break;
    }
    *driver_blend() = mode;
}

void state_forget_blend(void) {
    if (*driver_blend() == BLEND_UNKNOWN) return;
    *driver_blend() = BLEND_UNKNOWN;
    render_z_test(true);
    *driver_z_test() = 1;
    render_z_write(true);
    *driver_z_write() = 1;
}

void state_fog(bool on) {
    if (g_fog_on == on) return;
    g_fog_on = on;
    render_fog(on);
}

void state_forget_fog(void) { g_fog_on = false; }

void state_apply_fog(GuestPtr fog_address) {
    if (!fog_address) return;
    GliFog *fog = GUEST(GliFog, fog_address);
    float span = fsub(fog->intensity_far, fog->intensity_near);
    if (span != 0.0f) {
        float per_unit = fdiv(fsub(fog->far_distance, fog->near_distance), span);
        float start = fsub(fog->near_distance, fmul(fog->intensity_near, per_unit));
        float end = fadd(start, fmul(per_unit, 255.0f));
        fog->end_distance = 500.0f < end ? 500.0f : end;
        // The original's fog, whatever the draw distance: farther, the scenery is drawn into it.
        render_fog_range(start, end);
    }
    s32 r = fcvtw(fmul(fog->red, 255.0f));
    s32 g = fcvtw(fmul(fog->green, 255.0f));
    s32 b = fcvtw(fmul(fog->blue, 255.0f));
    r = r < 0 ? 0 : r;
    g = g < 0 ? 0 : g;
    b = b < 0 ? 0 : b;
    render_fog_color((u32)(r << 16 | b) | (u32)(g << 8) | 0xFF000000u);
}

void state_forget_addressing(void) { g_addressing = 0xFF; }

static void use_addressing(u8 addressing) {
    if (addressing == g_addressing) return;
    g_addressing = addressing;
    if (addressing <= 3) render_addressing(addressing & 2, addressing & 1);
}

void state_quads(void) {
    if (*driver_blend() != BLEND_UNKNOWN) state_mode(BLEND_UNKNOWN);
    state_z_write(false);
    state_mode(BLEND_TEXTURE);
}

void state_select_texture(void) {
    GliContext *ctx = gli_context();
    state_z_test(ctx->draw_flags & DRAW_Z_TEST);
    GliTexture *tex = ctx->texture ? GUEST(GliTexture, ctx->texture) : NULL;
    bool added = false;
    if (tex && tex->ready) {
        added = tex->flags & TEXTURE_ADDITIVE;
        state_mode(added ? BLEND_ADD : BLEND_TEXTURE);
        use_addressing(tex->addressing);
        const Texture *t = handle_object(OBJECT_TEXTURE, tex->handle);
        render_texture_set(t ? t->image : 0);
    } else {
        state_mode(BLEND_UNTEXTURED);
    }
    if (added) ctx->fog_on = 0;
    if (ctx->fog_on) {
        if (ctx->applied_fog != ctx->fog) {
            state_apply_fog(ctx->fog);
            ctx->applied_fog = ctx->fog;
        }
        state_fog(true);
    } else {
        state_fog(false);
    }
    render_linear_filter(true);
}

void gli_select_material(void) {
    GliContext *ctx = gli_context();
    GliTexture *tex = ctx->texture ? GUEST(GliTexture, ctx->texture) : NULL;
    const bool textured = tex && tex->ready;

    ctx->later_pass = 0;
    bool this_pass;
    if (ctx->pass_flags != 0) {
        ctx->pass_flags = 3;
        this_pass = true;
    } else {
        this_pass = !(textured && (tex->flags & TEXTURE_NOT_SOLID));
    }
    if (this_pass && !(ctx->draw_flags & DRAW_IGNORE_GLOBAL_ALPHA) && ctx->global_alpha < 0.98f)
        this_pass = false;
    if (this_pass && !(GUEST(GliMaterial, ctx->material)->type & MATERIAL_SOLID_PASS))
        this_pass = false;
    if (!this_pass) {
        ctx->state_flags |= STATE_NOT_THIS_PASS;
        if (textured && !(tex->flags & TEXTURE_NOT_DEFERRED)) ctx->later_pass = 1;
        return;
    }
    ctx->state_flags &= ~(u32)STATE_NOT_THIS_PASS;

    state_z_test(ctx->draw_flags & DRAW_Z_TEST);
    if (ctx->fog_on) {
        if (ctx->applied_fog != ctx->fog) {
            state_apply_fog(ctx->fog);
            ctx->applied_fog = ctx->fog;
        }
        state_fog(true);
    } else {
        state_fog(false);
    }
    state_z_write(ctx->draw_flags & DRAW_Z_WRITE);

    if (textured) {
        if (tex->flags & TEXTURE_BLENDED) state_blend(BLEND_MATERIAL);
        else if (ctx->pass_flags & 2) state_blend(BLEND_TEXTURE);
        else state_blend(BLEND_OPAQUE);
        use_addressing(tex->addressing);
        const Texture *t = handle_object(OBJECT_TEXTURE, tex->handle);
        render_texture_set(t ? t->image : 0);
    } else {
        state_blend(BLEND_OPAQUE);
        render_texture_set(0);
    }
    render_linear_filter(true);
}
