#include "game_vars.h"
#include <math.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gli.h"
#include "handles.h"
#include "render.h"

#define FONT GAME_VAR(g_AlphabetCharacterPointer)

void gli_overlay(const u8 *rgba, int w, int h, int x, int y, int zoom);

typedef struct Strip {
    int w, h;
    float *rgba;
} Strip;

static _Atomic(Strip *) g_made;
static Strip g_shown;
static u8 *g_out;
static size_t g_out_cap;

static int letter_index(unsigned c) {
    if (c < 0x20) return -1;
    if (c >= 'a' && c <= 'z') return (int)c - 0x40;
    if (c >= 'A' && c <= 'Z') return (int)c - 0x20;
    if (c >= 0xA0) return (int)c - 0x40;
    return (int)c - 0x20;
}

static const GsTexture *letter_texture(int index) {
    if (index <= 0) return NULL;
    const GuestPtr m = GLI_pGetVisualMaterialForLetter((u32)index);
    if (!m) return NULL;
    const GuestPtr gt = GUEST(GliMaterial, m)->texture;
    const Texture *t = gt ? handle_object(OBJECT_TEXTURE, GUEST(GliTexture, gt)->handle) : NULL;
    const GsTexture *g = t ? render_texture(t->image) : NULL;
    return g && g->texels && g->width > 0 && g->height > 0 ? g : NULL;
}

static float alpha_of(u32 t) {
    const u32 a = t >> 24;
    return a >= 0x80 ? 1.0f : (float)a / 128.0f;
}

static void columns(const GsTexture *g, int *first, int *last) {
    *first = g->width;
    *last = -1;
    for (int x = 0; x < g->width; x++)
        for (int y = 0; y < g->height; y++)
            if (alpha_of(g->texels[y * g->width + x]) > 0.1f) {
                if (x < *first) *first = x;
                *last = x;
                break;
            }
}

static const float k_blue[2][3] = {{215, 230, 238}, {118, 190, 246}};
static const float k_red[2][3] = {{255, 225, 225}, {242, 82, 84}};
static const char k_reds[] = "01010111" "010010";

static void tint(float *rgba, int width, int height, const int *letter_x, const bool *red, int letters) {
    int top = height, bottom = -1;
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            if (rgba[((size_t)y * width + x) * 4 + 3] > 0.1f) {
                if (y < top) top = y;
                bottom = y;
                break;
            }
    if (bottom <= top) return;
    for (int l = 0; l < letters; l++) {
        const float(*c)[3] = red[l] ? k_red : k_blue;
        const int x1 = l + 1 < letters ? letter_x[l + 1] : width;
        for (int y = 0; y < height; y++) {
            float f = (float)(y - top) / (float)(bottom - top);
            f = f < 0 ? 0 : f > 1 ? 1 : f;
            for (int x = letter_x[l]; x < x1; x++) {
                float *p = &rgba[((size_t)y * width + x) * 4];
                for (int k = 0; k < 3; k++) p[k] *= (c[0][k] + (c[1][k] - c[0][k]) * f) / 255.0f;
            }
        }
    }
}

int gli_hud_words_make(const char *latin1) {
    if (!*GUEST(GuestPtr, FONT) || !latin1 || !*latin1) return 0;
    const size_t n = strlen(latin1);
    int height = 0;
    for (size_t i = 0; i < n; i++) {
        const GsTexture *g = letter_texture(letter_index((u8)latin1[i]));
        if (g && g->height > height) height = g->height;
    }
    if (!height) return 0;
    const int gap = height / 16, space = height * 2 / 5;
    int width = 0;
    for (size_t i = 0; i < n; i++) {
        const GsTexture *g = letter_texture(letter_index((u8)latin1[i]));
        int a, b;
        if (g) columns(g, &a, &b);
        if (!g || b < a) width += space;
        else width += (b - a + 1) * height / g->height + gap;
    }
    if (width <= 0) return 0;
    float *rgba = calloc((size_t)width * (size_t)height * 4, sizeof *rgba);
    int *letter_x = calloc(n, sizeof *letter_x);
    bool *red = calloc(n, sizeof *red);
    if (!rgba || !letter_x || !red) {
        free(rgba);
        free(letter_x);
        free(red);
        return 0;
    }
    int x0 = 0, letters = 0;
    for (size_t i = 0; i < n; i++) {
        const GsTexture *g = letter_texture(letter_index((u8)latin1[i]));
        int a, b;
        if (g) columns(g, &a, &b);
        if (!g || b < a) {
            x0 += space;
            continue;
        }
        letter_x[letters] = x0;
        red[letters] = k_reds[letters % (sizeof k_reds - 1)] == '1';
        letters++;
        const int cw = (b - a + 1) * height / g->height;
        for (int y = 0; y < height; y++)
            for (int x = 0; x < cw; x++) {
                const int sx = a + x * g->height / height, sy = y * g->height / height;
                const u32 t = g->texels[(g->height - 1 - sy) * g->width + sx];
                float *p = &rgba[((size_t)y * width + x0 + x) * 4];
                const float al = alpha_of(t);
                p[0] = (float)(t & 255) / 255.0f * al;
                p[1] = (float)(t >> 8 & 255) / 255.0f * al;
                p[2] = (float)(t >> 16 & 255) / 255.0f * al;
                p[3] = al;
            }
        x0 += cw + gap;
    }
    tint(rgba, width, height, letter_x, red, letters);
    free(letter_x);
    free(red);
    Strip *made = malloc(sizeof *made);
    if (!made) {
        free(rgba);
        return 0;
    }
    *made = (Strip){width, height, rgba};
    Strip *old = atomic_exchange(&g_made, made);
    if (old) free(old->rgba);
    free(old);
    return 1;
}

static float sample(const Strip *s, float x, float y, int k) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    const int x0 = (int)x, y0 = (int)y;
    const int x1 = x0 + 1 < s->w ? x0 + 1 : s->w - 1, y1 = y0 + 1 < s->h ? y0 + 1 : s->h - 1;
    const float fx = x - (float)x0, fy = y - (float)y0;
    const float *r0 = &s->rgba[(size_t)y0 * s->w * 4], *r1 = &s->rgba[(size_t)y1 * s->w * 4];
    const float top = r0[x0 * 4 + k] + (r0[x1 * 4 + k] - r0[x0 * 4 + k]) * fx;
    const float bot = r1[x0 * 4 + k] + (r1[x1 * 4 + k] - r1[x0 * 4 + k]) * fx;
    return top + (bot - top) * fy;
}

int gli_hud_words_width(int height) {
    Strip *made = atomic_exchange(&g_made, NULL);
    if (made) {
        free(g_shown.rgba);
        g_shown = *made;
        free(made);
    }
    if (!g_shown.rgba || height <= 0) return 0;
    return g_shown.w * height / g_shown.h;
}

static float alpha_at(float x, float y) {
    if (x <= -1.0f || y <= -1.0f || x >= (float)g_shown.w || y >= (float)g_shown.h) return 0.0f;
    return sample(&g_shown, x, y, 3);
}

int gli_hud_words_draw(float opacity, int right, int bottom, int height) {
    const int tw = gli_hud_words_width(height);
    if (tw <= 0) return 0;
    const int d = height / 20 > 1 ? height / 20 : 1, w = tw + d, h = height + d;
    const size_t need = (size_t)w * (size_t)h * 4;
    if (need > g_out_cap) {
        u8 *p = realloc(g_out, need);
        if (!p) return 0;
        g_out = p;
        g_out_cap = need;
    }
    const float step = (float)g_shown.h / (float)height;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            const float sx = ((float)x + 0.5f) * step - 0.5f, sy = ((float)y + 0.5f) * step - 0.5f;
            const float a = x < tw && y < height ? alpha_at(sx, sy) : 0.0f;
            const float shadow = 0.6f * alpha_at(sx - (float)d * step, sy - (float)d * step);
            const float out = a + shadow * (1.0f - a);
            u8 *o = &g_out[((size_t)y * w + x) * 4];
            for (int k = 0; k < 3; k++) {
                const float c = a > 0.0f && out > 0.0f ? sample(&g_shown, sx, sy, k) / out : 0.0f;
                o[k] = (u8)lrintf((c > 1.0f ? 1.0f : c) * 255.0f);
            }
            o[3] = (u8)lrintf((out > 1.0f ? 1.0f : out) * opacity * 255.0f);
        }
    gli_overlay(g_out, w, h, right - w, bottom - h, 1);
    return 1;
}
