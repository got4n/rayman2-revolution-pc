#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../host/crt.h"
#include "files.h"
#include "gli.h"
#include "handles.h"
#include "render.h"

#define BAND_ROWS 64

Raster *raster_new(int width, int height) {
    Raster *r = calloc(1, sizeof *r);
    r->width = width;
    r->height = height;
    r->texels = calloc((size_t)width * (size_t)height, 4);
    return r;
}

void raster_free(Raster *r) {
    if (!r) return;
    if (r->texture) render_texture_free(r->texture);
    free(r->texels);
    free(r);
}

u32 raster_texture(Raster *r) {
    if (!r->texture) r->texture = render_texture_new(r->width, r->height, r->texels);
    return r->texture;
}

static void raster_changed(Raster *r) {
    if (r->texture) render_texture_free(r->texture);
    r->texture = 0;
}

static u32 game_malloc(u32 size) { return crt_malloc(size); }
static void game_free(GuestPtr p) { crt_freemem(p); }

static void read_image(Raster *r, GuestPtr name) {
    const u32 bytes = (u32)r->height * (u32)r->width * 4u;
    const CallerFrame frame = {0xE0, 0};
    char path[0x80];
    snprintf(path, sizeof path, "vignette/%s.rwr", GUEST(char, name));
    u32 file = file_open(path, frame);
    if (!file_opened(file)) file = file_open("vignette/missing.rwr", frame);
    if (!file_opened(file)) return;
    GuestPtr buffer = game_malloc(bytes);
    if (buffer) file_read_into(file, buffer, bytes);
    file_close(file);
    if (!buffer) return;
    memcpy(r->texels, GUEST(u8, buffer), bytes);
    raster_changed(r);
    game_free(buffer);
}

int revo_loading_movie_hidden(void);

void gli_raster_read_image(u32 raster, GuestPtr name) {
    Raster *r = handle_object(OBJECT_RASTER, raster);
    if (r && revo_loading_movie_hidden()) {
        /* With no loadings, the loading movie's background ("Rayman Revolution - loading") is left
           black: it would only flash. */
        memset(r->texels, 0, (size_t)r->width * (size_t)r->height * 4);
        raster_changed(r);
        return;
    }
    if (r) {
        read_image(r, name);
        return;
    }
    Raster *scratch = raster_new(512, 256);
    read_image(scratch, name);
    raster_free(scratch);
}

static int power_of_two(int n) {
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

static void draw_scaled(const Raster *r, int x, int y, int width, int height) {
    GsState s = {.tme = true, .tfx = GS_DECAL, .tcc = true, .wrap_s = GS_REPEAT, .wrap_t = GS_REPEAT,
                 .mmag = 1, .mmin = 1, .blend = true, .a = GS_CS, .b = GS_CD, .c = GS_AS, .d = GS_CD,
                 .ztst = GS_Z_ALWAYS, .colclamp = true};
    u32 top = (u32)y << 16;
    for (int row = 0; row < r->height;) {
        const int left = r->height - row;
        const int rows = left < BAND_ROWS ? left : BAND_ROWS;
        const int span = left > BAND_ROWS ? rows - 1 : rows;
        const u32 bottom = top + (u32)(((u64)span << 16) * (u64)height / (u64)r->height);
        const int texture_rows = power_of_two(rows);
        u32 *texels = calloc((size_t)r->width * (size_t)texture_rows, 4);
        memcpy(texels, r->texels + (size_t)row * (size_t)r->width, (size_t)r->width * (size_t)rows * 4);
        s.texture = render_texture_new(r->width, texture_rows, texels);
        free(texels);
        const float w = (float)r->width, h = (float)texture_rows;
        GsVertex v[2] = {
            {.x = (float)x, .y = (float)(top >> 16), .s = 0.5f / w, .t = 0.5f / h, .q = 1.0f, .fog = 255},
            {.x = (float)(x + width), .y = (float)(bottom >> 16), .s = (w - 0.5f) / w, .t = ((float)rows - 0.5f) / h,
             .q = 1.0f, .r = 0xFF, .g = 0xFF, .b = 0xFF, .a = 0x80, .fog = 255},
        };
        render_2d_widescreen(v, 2);
        render_draw(GS_SPRITE, &s, v, 2);
        render_texture_free(s.texture);
        top = bottom;
        row += span;
    }
}

void gli_startup_image(GuestPtr name, s32 width, s32 height, u8 flip) {
    GuestPtr frame = game_stack_push(0xD0), path = frame + 0x10;
    snprintf(GUEST(char, path), 0x40, "%s", GUEST(char, name));
    for (char *c = GUEST(char, path); *c; c++)
        if (*c >= 'a' && *c <= 'z') *c = (char)(*c - ('a' - 'A'));
    FMN_ConvertPathUnix2Dos(path);
    Raster *r = raster_new(512, 256);
    read_image(r, path);
    if (flip == 1) {
        render_clear_color(0xFF000000u);
        render_clear_depth();
    }
    draw_scaled(r, 0, 0, width, height);
    if (flip == 1) render_show_frame();
    fn_vTimerWait(50);
    raster_free(r);
    game_stack_pop(0xD0);
}

void gli_show_raster(void) { render_show_frame(); }
