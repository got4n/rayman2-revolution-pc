#include "game_vars.h"
#include <stdio.h>
#include <stdlib.h>

#include "gli.h"
#include "handles.h"
#include "render.h"

u32 rw_camera_begin_update(GuestPtr camera) { return camera; }
u32 rw_camera_end_update(GuestPtr camera) { return camera; }

u32 rw_camera_show_raster(GuestPtr camera, GuestPtr device, u32 flags) {
    (void)device;
    (void)flags;
    render_show_frame();
    return camera;
}

u32 rw_camera_clear(GuestPtr camera, GuestPtr color, s32 flags) {
    const u8 *c = GUEST(u8, color);
    if (flags & 1) render_clear_color((u32)c[3] << 24 | (u32)c[0] << 16 | (u32)c[1] << 8 | c[2]);
    if (flags & 2) render_clear_depth();
    return camera;
}

enum {
    RS_TEXTURE_RASTER = 1, RS_TEXTURE_ADDRESS = 2, RS_Z_TEST = 6, RS_SHADE_MODE = 7, RS_Z_WRITE = 8,
    RS_TEXTURE_FILTER = 9, RS_SRC_BLEND = 0xA, RS_DEST_BLEND = 0xB, RS_VERTEX_ALPHA = 0xC, RS_FOG = 0xE,
};
enum { ADDRESS_WRAP_RW = 1 };
enum { FILTER_NEAREST_RW = 1 };
enum { BLEND_ZERO_RW = 1, BLEND_ONE_RW = 2, BLEND_SRC_ALPHA_RW = 5, BLEND_INV_SRC_ALPHA_RW = 6 };

static void unsupported(const char *what, u32 state, u32 value) {
    static int logged;
    if (logged++ < 8) revo_log("[gli] render state %u = %u: %s not drawn natively yet", state, value, what);
}

static bool blend_factor(u32 value, BlendFactor *out) {
    switch (value) {
    case BLEND_ZERO_RW: *out = FACTOR_ZERO; return true;
    case BLEND_ONE_RW: *out = FACTOR_ONE; return true;
    case BLEND_SRC_ALPHA_RW: *out = FACTOR_SRC_ALPHA; return true;
    case BLEND_INV_SRC_ALPHA_RW: *out = FACTOR_INV_SRC_ALPHA; return true;
    default: return false;
    }
}

u32 rw_render_state_set(u32 state, u32 value) {
    BlendFactor f;
    switch (state) {
    case RS_TEXTURE_RASTER: {
        Raster *r = handle_object(OBJECT_RASTER, value);
        render_texture_set(r ? raster_texture(r) : 0);
        break;
    }
    case RS_TEXTURE_ADDRESS: {
        if (value != ADDRESS_WRAP_RW) unsupported("texture addressing", state, value);
        render_addressing(value == ADDRESS_WRAP_RW, value == ADDRESS_WRAP_RW);
        break;
    }
    case RS_Z_TEST: render_z_test(value != 0); break;
    case RS_SHADE_MODE: if (value != 2) unsupported("flat shading", state, value); break;
    case RS_Z_WRITE: render_z_write(value != 0); break;
    case RS_TEXTURE_FILTER: render_linear_filter(value != FILTER_NEAREST_RW); break;
    case RS_SRC_BLEND:
        if (blend_factor(value, &f)) render_blend(render_state.vertex_alpha, f, render_state.dst);
        else unsupported("source blend", state, value);
        break;
    case RS_DEST_BLEND:
        if (blend_factor(value, &f)) render_blend(render_state.vertex_alpha, render_state.src, f);
        else unsupported("destination blend", state, value);
        break;
    case RS_VERTEX_ALPHA: render_blend(value != 0, render_state.src, render_state.dst); break;
    case RS_FOG: render_fog(value != 0); break;
    default: unsupported("state", state, value); break;
    }
    return 1;
}

float rw_im2d_get_near_screen_z(void) { return cameras_near_screen_z(); }

float rw_camera_get_near_clip_plane(GuestPtr camera) {
    (void)camera;
    return cameras_main_near();
}

u32 rw_im2d_render_indexed(s32 type, GuestPtr vertices, s32 count, GuestPtr indices, s32 index_count) {
    enum { TRIANGLE_LIST = 3 };
    (void)count;
    if (type == TRIANGLE_LIST) screen_im2d_triangles(vertices, indices, index_count);
    else revo_log("[gli] Im2D primitive %d not drawn natively yet", type);
    return 1;
}

u32 rw_raster_create(s32 width, s32 height, s32 depth, u32 flags) {
    (void)flags;
    if (depth != 32) revo_log("[gli] a raster of %d bits: not made natively yet", depth);
    Raster *r = raster_new(width, height);
    r->handle = handle_new(OBJECT_RASTER, r);
    return r->handle;
}

u32 rw_raster_destroy(u32 raster) {
    Raster *r = handle_object(OBJECT_RASTER, raster);
    if (r) {
        handle_free(r->handle);
        raster_free(r);
    }
    return 1;
}

void rw_texture_cache_flush(void) {}

#define PACKET_POINTER GAME_VAR(inv_GLI_packetCursor)
void rw_open_local_packet(u32 type, u32 size) {
    (void)type;
    (void)size;
    static GuestPtr room;
    if (!room) room = window_alloc(0x400);
    *GUEST(u32, PACKET_POINTER) = room;
}
