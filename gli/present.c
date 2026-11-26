#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mods.h"
#include "render.h"
#include "render_vk.h"

enum { MIN_PERCENT = 25, MAX_PERCENT = 400, MAX_SIDE = 16384 };

static struct {
    bool ready;
    bool vulkan;
    bool window;
    int percent;
    int pw, ph;
    int want_w, want_h;
    int refused_w, refused_h;
    double want_ms;
    int shown;
    u32 draws;
} g;

u64 revo_frame_count(void);
void revo_sleep_ns(u64 ns);
u64 revo_vblank_period_ns(void);

void revo_view_aspect(int *across, int *down);

double now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e3 + (double)t.tv_nsec / 1e6;
}

static int clamp_percent(int p) { return p < MIN_PERCENT ? MIN_PERCENT : p > MAX_PERCENT ? MAX_PERCENT : p; }

static int present_mode(void) {
    if (revo_set(REVO_SET_SCALING) == 2) return 2;
    return revo_mod(REVO_MOD_SMOOTH) ? 1 : 0;
}

void gli_set_resolution(int percent) { g.percent = clamp_percent(percent); }
int gli_resolution(void) { return g.percent ? g.percent : 100; }
void gli_frame_size(int *w, int *h) {
    *w = g.pw;
    *h = g.ph;
}

static void picture(int width, int height, int *w, int *h) {
    int across, down;
    revo_view_aspect(&across, &down);
    *w = width;
    *h = width * down / across;
    if (*h > height) {
        *h = height;
        *w = height * across / down;
    }
}

static void frame_size(int width, int height, int *pw, int *ph) {
    int w, h;
    picture(width, height, &w, &h);
    const int pc = gli_resolution();
    *pw = (w * pc + 50) / 100;
    *ph = (h * pc + 50) / 100;
    *pw = *pw < 64 ? 64 : *pw > MAX_SIDE ? MAX_SIDE : *pw;
    *ph = *ph < 64 ? 64 : *ph > MAX_SIDE ? MAX_SIDE : *ph;
}

static void apply_size(int width, int height) {
    int pw, ph;
    frame_size(width, height, &pw, &ph);
    if ((pw == g.pw && ph == g.ph) || (pw == g.refused_w && ph == g.refused_h)) return;
    const double t = now_ms();
    if (pw != g.want_w || ph != g.want_h) {
        g.want_w = pw;
        g.want_h = ph;
        g.want_ms = t;
        return;
    }
    if (t - g.want_ms < 250.0) return;
    if (!vk_set_size(pw, ph)) {
        revo_log("[gli] no frame buffer of %dx%d, staying at %dx%d", pw, ph, g.pw, g.ph);
        g.want_w = g.want_h = 0;
        g.refused_w = pw;
        g.refused_h = ph;
        return;
    }
    g.pw = pw;
    g.ph = ph;
    revo_log("[gli] frame buffers %dx%d (%d%%)", pw, ph, gli_resolution());
}

static void init(int width, int height) {
    const char *r = getenv("REVO_RESOLUTION");
    if (!g.percent) g.percent = clamp_percent(r ? atoi(r) : 100);
    frame_size(width, height, &g.pw, &g.ph);
    g.vulkan = vk_open(g.pw, g.ph);
    if (!g.vulkan) revo_log("[gli] Vulkan: no frame buffer of %dx%d: nothing is drawn", g.pw, g.ph);
    else revo_log("[gli] frame buffers %dx%d (%d%%)", g.pw, g.ph, gli_resolution());
    g.ready = true;
}

static void release_texture(GsTexture *t) { vk_release_texture(t); }

static void pace(void) {
    static double last, last_frame;
    const bool frame = render_wait_frame(now_ms() - last_frame < 100.0 ? 25 : 8);
    const double t = now_ms();
    if (frame) {
        last_frame = t;
        const double gap = (double)revo_vblank_period_ns() / 1e6 / (revo_mod(REVO_MOD_60FPS) ? 2 : 1) - 0.3;
        if (last && t > last && t < last + gap) revo_sleep_ns((u64)((last + gap - t) * 1e6));
    }
    last = now_ms();
}

void gli_present(int width, int height) {
    if (!g.window) return;
    if (!g.ready) init(width, height);
    if (!g.vulkan) return;
    apply_size(width, height);
    pace();
    const double started = now_ms();
    for (const GsFrame *f; (f = render_next_frame());) {
        render_textures_release(f->number, release_texture);
        g.draws += (u32)vk_draw_frame(f);
        g.shown = f->buffer & 1;
        if (now_ms() - started > 16.0) break;
    }
    int w, h;
    picture(width, height, &w, &h);
    vk_show(g.shown, (width - w) / 2, height - (height - h) / 2 - h, w, h, present_mode(), revo_set(REVO_SET_POST));
}

const char *gli_vulkan_library(void) { return vk_library(); }

bool gli_open_vulkan_window(const char *const *extensions, unsigned n, VkSurfaceMaker make, void *ctx) {
    g.window = vk_open_window(extensions, n, make, ctx);
    return g.window;
}

bool gli_vulkan_window(void) { return g.window; }
void gli_overlay(const u8 *rgba, int w, int h, int x, int y, int zoom) { vk_overlay(rgba, w, h, x, y, zoom); }
void gli_screenshot(const char *name) { vk_screenshot(name); }
void gli_end_present(int width, int height) { vk_present(width, height); }
