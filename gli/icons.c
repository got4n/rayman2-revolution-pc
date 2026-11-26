#include "game_vars.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gli.h"
#include "handles.h"
#include "render.h"

#define FONT GAME_VAR(g_AlphabetCharacterPointer)

#define FIRST_LETTER (800 + '0' - 0x20)
#define LETTERS ('R' - '0' + 1)

int revo_icon_style(void);
int revo_icon_binding(int action);
const char *revo_icon_key(int action);

enum { REVO_ICONS_PS2, REVO_ICONS_XBOX, REVO_ICONS_SWITCH, REVO_ICONS_DECK, REVO_ICONS_KEYBOARD };
#define MOVE_KEYS 17 /* REVO_ACT_COUNT: revo_icon_key's four moves */

typedef enum {
    SHOW_NONE, SHOW_ACTION, SHOW_ACTION_LEFT, SHOW_ACTION_RIGHT, SHOW_LEFT_STICK,
} Show;

enum { ACT_CROSS, ACT_SQUARE, ACT_TRIANGLE, ACT_CIRCLE, ACT_L1, ACT_R1, ACT_L2, ACT_R2, ACT_START, ACT_SELECT };

typedef struct Code { char code; Show show; int action; } Code;
static const Code k_codes[] = {
    {'1', SHOW_ACTION, ACT_CROSS},        {'2', SHOW_ACTION, ACT_CIRCLE},      {'3', SHOW_ACTION, ACT_TRIANGLE},
    {'4', SHOW_ACTION, ACT_SQUARE},       {'5', SHOW_ACTION, ACT_L1},          {'6', SHOW_ACTION, ACT_L2},
    {'7', SHOW_ACTION, ACT_R1},           {'8', SHOW_ACTION, ACT_R2},          {'9', SHOW_ACTION_LEFT, ACT_START},
    {'B', SHOW_ACTION_RIGHT, ACT_START},  {'A', SHOW_ACTION_LEFT, ACT_SELECT}, {'C', SHOW_ACTION_RIGHT, ACT_SELECT},
    {'I', SHOW_LEFT_STICK, -1},           {'J', SHOW_LEFT_STICK, -1},
};

enum {
    PAD_A, PAD_B, PAD_X, PAD_Y, PAD_BACK, PAD_GUIDE, PAD_START, PAD_LEFTSTICK, PAD_RIGHTSTICK, PAD_LEFTSHOULDER,
    PAD_RIGHTSHOULDER, PAD_DPAD_UP, PAD_DPAD_DOWN, PAD_DPAD_LEFT, PAD_DPAD_RIGHT, PAD_MISC1, PAD_PADDLE1,
    PAD_PADDLE2, PAD_PADDLE3, PAD_PADDLE4, PAD_TOUCHPAD,
    PAD_TRIGGER_LEFT = 100, PAD_TRIGGER_RIGHT = 101,
    PAD_LEFT_STICK_MOVED = 102,
};

#define SIZE 64

typedef struct Canvas {
    int width;
    float rgba[SIZE * 2 * SIZE][4];
} Canvas;

typedef struct Rgb { float r, g, b; } Rgb;

static Rgb rgb(u32 hex) { return (Rgb){(hex >> 16 & 255) / 255.0f, (hex >> 8 & 255) / 255.0f, (hex & 255) / 255.0f}; }

typedef float (*Distance)(const void *shape, float x, float y);

static void paint(Canvas *cv, Distance d, const void *shape, Rgb c) {
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < cv->width; x++) {
            float cover = 0.5f - d(shape, x + 0.5f, y + 0.5f);
            if (cover <= 0.0f) continue;
            if (cover > 1.0f) cover = 1.0f;
            float *p = cv->rgba[y * cv->width + x];
            const float a = cover + p[3] * (1.0f - cover);
            const float k = cover / a;
            p[0] += (c.r - p[0]) * k;
            p[1] += (c.g - p[1]) * k;
            p[2] += (c.b - p[2]) * k;
            p[3] = a;
        }
}

typedef struct Disc { float x, y, r; } Disc;
static float disc_distance(const void *s, float x, float y) {
    const Disc *d = s;
    return hypotf(x - d->x, y - d->y) - d->r;
}

typedef struct Ring { float x, y, r, w; } Ring;
static float ring_distance(const void *s, float x, float y) {
    const Ring *d = s;
    return fabsf(hypotf(x - d->x, y - d->y) - d->r) - d->w * 0.5f;
}

typedef struct Box { float x, y, hw, hh, r, line; } Box;
static float box_distance(const void *s, float x, float y) {
    const Box *b = s;
    const float qx = fabsf(x - b->x) - (b->hw - b->r), qy = fabsf(y - b->y) - (b->hh - b->r);
    const float out = hypotf(fmaxf(qx, 0.0f), fmaxf(qy, 0.0f)) + fminf(fmaxf(qx, qy), 0.0f) - b->r;
    return b->line > 0.0f ? fabsf(out + b->line * 0.5f) - b->line * 0.5f : out;
}

typedef struct Glyph { char c; float p[40]; } Glyph;
static const Glyph k_glyphs[] = {
    {'A', {0, 6, 2, 0, 4, 6, -1, 0.7f, 4, 3.3f, 4, -2}},
    {'C', {4, 0.8f, 3.2f, 0, 0.8f, 0, 0, 0.8f, 0, 5.2f, 0.8f, 6, 3.2f, 6, 4, 5.2f, -2}},
    {'D', {0, 0, 2.6f, 0, 4, 1.4f, 4, 4.6f, 2.6f, 6, 0, 6, 0, 0, -2}},
    {'E', {4, 0, 0, 0, 0, 6, 4, 6, -1, 0, 3, 3, 3, -2}},
    {'F', {4, 0, 0, 0, 0, 6, -1, 0, 3, 3, 3, -2}},
    {'G', {4, 0.8f, 3.2f, 0, 0.8f, 0, 0, 0.8f, 0, 5.2f, 0.8f, 6, 3.2f, 6, 4, 5.2f, 4, 3.2f, 2.2f, 3.2f, -2}},
    {'H', {0, 0, 0, 6, -1, 4, 0, 4, 6, -1, 0, 3, 4, 3, -2}},
    {'I', {0.8f, 0, 3.2f, 0, -1, 2, 0, 2, 6, -1, 0.8f, 6, 3.2f, 6, -2}},
    {'J', {1.2f, 0, 4, 0, -1, 3, 0, 3, 5, 2, 6, 1, 6, 0, 5, -2}},
    {'K', {0, 0, 0, 6, -1, 4, 0, 0, 3.6f, -1, 1.4f, 2.6f, 4, 6, -2}},
    {'M', {0, 6, 0, 0, 2, 3, 4, 0, 4, 6, -2}},
    {'N', {0, 6, 0, 0, 4, 6, 4, 0, -2}},
    {'O', {0.8f, 0, 3.2f, 0, 4, 0.8f, 4, 5.2f, 3.2f, 6, 0.8f, 6, 0, 5.2f, 0, 0.8f, 0.8f, 0, -2}},
    {'Q', {0.8f, 0, 3.2f, 0, 4, 0.8f, 4, 5.2f, 3.2f, 6, 0.8f, 6, 0, 5.2f, 0, 0.8f, 0.8f, 0, -1, 2.4f, 4.4f, 4, 6,
           -2}},
    {'U', {0, 0, 0, 5.2f, 0.8f, 6, 3.2f, 6, 4, 5.2f, 4, 0, -2}},
    {'V', {0, 0, 2, 6, 4, 0, -2}},
    {'W', {0, 0, 1, 6, 2, 2.5f, 3, 6, 4, 0, -2}},
    {'0', {0.8f, 0, 3.2f, 0, 4, 0.8f, 4, 5.2f, 3.2f, 6, 0.8f, 6, 0, 5.2f, 0, 0.8f, 0.8f, 0, -1, 3.6f, 0.6f, 0.4f,
           5.4f, -2}},
    {'6', {3.6f, 0.4f, 3, 0, 1, 0, 0.2f, 0.8f, 0.2f, 5.2f, 1, 6, 3, 6, 3.8f, 5.2f, 3.8f, 3.8f, 3, 3, 1, 3, 0.2f,
           3.8f, -2}},
    {'7', {0, 0, 4, 0, 1.5f, 6, -2}},
    {'8', {1, 0, 3, 0, 3.7f, 0.7f, 3.7f, 2.3f, 3, 3, 1, 3, 0.3f, 2.3f, 0.3f, 0.7f, 1, 0, -1, 1, 3, 0.2f, 3.8f,
           0.2f, 5.2f, 1, 6, 3, 6, 3.8f, 5.2f, 3.8f, 3.8f, 3, 3, -2}},
    {'9', {0.4f, 5.6f, 1, 6, 3, 6, 3.8f, 5.2f, 3.8f, 0.8f, 3, 0, 1, 0, 0.2f, 0.8f, 0.2f, 2.2f, 1, 3, 3, 3, 3.8f,
           2.2f, -2}},
    /* The arrow keys. */
    {'l', {4, 3, 0.4f, 3, -1, 2.2f, 1, 0.4f, 3, 2.2f, 5, -2}},
    {'r', {0, 3, 3.6f, 3, -1, 1.8f, 1, 3.6f, 3, 1.8f, 5, -2}},
    {'u', {2, 6, 2, 0.6f, -1, 0, 2.6f, 2, 0.6f, 4, 2.6f, -2}},
    {'d', {2, 0, 2, 5.4f, -1, 0, 3.4f, 2, 5.4f, 4, 3.4f, -2}},
    {',', {2.2f, 5, 1.6f, 6.6f, -2}},
    {'.', {2, 5.6f, 2, 5.9f, -2}},
    {';', {2, 1.8f, 2, 2.1f, -1, 2.2f, 5, 1.6f, 6.6f, -2}},
    {'\'', {2, 0, 2, 1.6f, -2}},
    {'`', {1.4f, 0, 2.4f, 1.4f, -2}},
    {'[', {3, 0, 1.4f, 0, 1.4f, 6, 3, 6, -2}},
    {']', {1, 0, 2.6f, 0, 2.6f, 6, 1, 6, -2}},
    {'=', {0.4f, 2, 3.6f, 2, -1, 0.4f, 4, 3.6f, 4, -2}},
    {'/', {3.6f, 0, 0.4f, 6, -2}},
    {'\\', {0.4f, 0, 3.6f, 6, -2}},
    {'B', {0, 3, 2.6f, 3, 3.5f, 2.3f, 3.5f, 0.7f, 2.6f, 0, 0, 0, 0, 6, 2.8f, 6, 3.8f, 5.2f, 3.8f, 3.8f, 2.8f, 3, -2}},
    {'X', {0, 0, 4, 6, -1, 4, 0, 0, 6, -2}},
    {'Y', {0, 0, 2, 3, 4, 0, -1, 2, 3, 2, 6, -2}},
    {'L', {0, 0, 0, 6, 3.6f, 6, -2}},
    {'R', {0, 6, 0, 0, 2.8f, 0, 3.8f, 0.8f, 3.8f, 2.2f, 2.8f, 3, 0, 3, -1, 2, 3, 4, 6, -2}},
    {'T', {0, 0, 4, 0, -1, 2, 0, 2, 6, -2}},
    {'Z', {0, 0, 4, 0, 0, 6, 4, 6, -2}},
    {'S', {3.8f, 0.8f, 3, 0, 1, 0, 0.2f, 0.8f, 0.2f, 2.2f, 1, 3, 3, 3, 3.8f, 3.8f, 3.8f, 5.2f, 3, 6, 1, 6, 0.2f,
           5.2f, -2}},
    {'P', {0, 6, 0, 0, 2.8f, 0, 3.8f, 0.8f, 3.8f, 2.2f, 2.8f, 3, 0, 3, -2}},
    {'1', {1, 1, 2.2f, 0, 2.2f, 6, -1, 1, 6, 3.4f, 6, -2}},
    {'2', {0.3f, 1, 1.2f, 0, 2.8f, 0, 3.7f, 1, 3.7f, 2.2f, 0.3f, 6, 3.7f, 6, -2}},
    {'3', {0.3f, 0.6f, 1, 0, 3, 0, 3.7f, 0.8f, 3.7f, 2.2f, 3, 3, 1.5f, 3, -1, 3, 3, 3.7f, 3.8f, 3.7f, 5.2f, 3, 6, 1,
           6, 0.3f, 5.4f, -2}},
    {'4', {3, 6, 3, 0, 0, 4.2f, 4, 4.2f, -2}},
    {'5', {3.7f, 0, 0.5f, 0, 0.3f, 2.8f, 2.8f, 2.6f, 3.7f, 3.5f, 3.7f, 5.2f, 3, 6, 1, 6, 0.3f, 5.4f, -2}},
    {'+', {2, 1, 2, 5, -1, 0, 3, 4, 3, -2}},
    {'-', {0.4f, 3, 3.6f, 3, -2}},
};

static const Glyph *glyph(char c) {
    for (size_t i = 0; i < sizeof k_glyphs / sizeof *k_glyphs; i++)
        if (k_glyphs[i].c == c) return &k_glyphs[i];
    return NULL;
}

typedef struct Text { const char *s; float x, y, height, width; } Text;

static float segment_distance(float px, float py, float ax, float ay, float bx, float by) {
    const float dx = bx - ax, dy = by - ay, l = dx * dx + dy * dy;
    float t = l > 0.0f ? ((px - ax) * dx + (py - ay) * dy) / l : 0.0f;
    t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
    return hypotf(px - ax - t * dx, py - ay - t * dy);
}

static float text_distance(const void *s, float x, float y) {
    const Text *t = s;
    const int n = (int)strlen(t->s);
    const float unit = t->height / 6.0f, advance = 5.4f * unit;
    const float left = t->x - (advance * n - 1.4f * unit) * 0.5f, top = t->y - t->height * 0.5f;
    float best = 1e9f;
    for (int i = 0; i < n; i++) {
        const Glyph *g = glyph(t->s[i]);
        if (!g) continue;
        const float ox = left + i * advance;
        for (int k = 0; g->p[k] != -2; k += 2) {
            if (g->p[k] == -1) { k--; continue; }
            if (g->p[k + 2] < 0) continue;
            const float d = segment_distance(x, y, ox + g->p[k] * unit, top + g->p[k + 1] * unit,
                                             ox + g->p[k + 2] * unit, top + g->p[k + 3] * unit);
            if (d < best) best = d;
        }
    }
    return best - t->width * 0.5f;
}

static void write(Canvas *cv, const char *s, float x, float y, float height, float room, Rgb c) {
    const int n = (int)strlen(s);
    const float wide = height / 6.0f * (5.4f * n - 1.4f);
    if (wide > room) height *= room / wide;
    const Text t = {s, x, y, height, fmaxf(3.0f, height * 0.22f)};
    paint(cv, text_distance, &t, c);
}

typedef struct Style {
    Rgb body, edge, text;
    Rgb face[4];
} Style;

static Style style(int s) {
    Style st = {rgb(0x2A2A2E), rgb(0x606066), rgb(0xF2F2F2), {rgb(0xF2F2F2), rgb(0xF2F2F2), rgb(0xF2F2F2), rgb(0xF2F2F2)}};
    if (s == REVO_ICONS_XBOX) {
        st.body = rgb(0x1E1E1E);
        st.face[0] = rgb(0x6CC24A);
        st.face[1] = rgb(0xE8453C);
        st.face[2] = rgb(0x3E8EDE);
        st.face[3] = rgb(0xF6C22E);
    } else if (s == REVO_ICONS_SWITCH) {
        st.body = rgb(0x323236);
        st.edge = rgb(0x6A6A70);
    } else if (s == REVO_ICONS_DECK) {
        st.body = rgb(0x23262E);
        st.edge = rgb(0x5A6070);
        for (int i = 0; i < 4; i++) st.face[i] = rgb(0xDCDFE6);
    }
    return st;
}

static const char *button_name(int s, int b) {
    static const char *const xbox[] = {"A", "B", "X", "Y", NULL, NULL, NULL, "LS", "RS", "LB", "RB",
                                       NULL, NULL, NULL, NULL, NULL, "P1", "P2", "P3", "P4", NULL};
    static const char *const nintendo[] = {"B", "A", "Y", "X", "-", NULL, "+", "L", "R", "L", "R",
                                           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    static const char *const deck[] = {"A", "B", "X", "Y", NULL, NULL, NULL, "L3", "R3", "L1", "R1",
                                       NULL, NULL, NULL, NULL, NULL, "R4", "L4", "R5", "L5", NULL};
    if (b == PAD_TRIGGER_LEFT) return s == REVO_ICONS_SWITCH ? "ZL" : s == REVO_ICONS_DECK ? "L2" : "LT";
    if (b == PAD_TRIGGER_RIGHT) return s == REVO_ICONS_SWITCH ? "ZR" : s == REVO_ICONS_DECK ? "R2" : "RT";
    if (b == PAD_LEFT_STICK_MOVED) return s == REVO_ICONS_XBOX ? "LS" : "L";
    if (b < 0 || b > PAD_TOUCHPAD) return NULL;
    return (s == REVO_ICONS_SWITCH ? nintendo : s == REVO_ICONS_DECK ? deck : xbox)[b];
}

static void face_button(Canvas *cv, const Style *st, int b, const char *name) {
    const Disc rim = {32, 32, 29}, body = {32, 32, 26.5f};
    paint(cv, disc_distance, &rim, st->edge);
    paint(cv, disc_distance, &body, st->body);
    write(cv, name, 32, 32, 36, 44, st->face[b]);
}

static void shoulder(Canvas *cv, const Style *st, const char *name, bool trigger) {
    const Box rim = {32, 32, 31, trigger ? 22 : 18, trigger ? 14 : 8, 0}, body = {32, 32, 28.5f, trigger ? 19.5f : 15.5f,
                                                                                 trigger ? 12 : 6, 0};
    paint(cv, box_distance, &rim, st->edge);
    paint(cv, box_distance, &body, st->body);
    write(cv, name, 32, 32, trigger ? 26 : 24, 48, st->text);
}

static void stick(Canvas *cv, const Style *st, const char *name, float x) {
    const Ring ring = {x, 32, 27, 4.5f};
    const Disc body = {x, 32, 21};
    paint(cv, ring_distance, &ring, st->edge);
    paint(cv, disc_distance, &body, st->body);
    write(cv, name, x, 32, 26, 34, st->text);
}

static void dpad(Canvas *cv, const Style *st, int b) {
    const Box across = {32, 32, 29, 10.5f, 4, 0}, down = {32, 32, 10.5f, 29, 4, 0};
    paint(cv, box_distance, &across, st->edge);
    paint(cv, box_distance, &down, st->edge);
    const Box in_across = {32, 32, 27, 8.5f, 3, 0}, in_down = {32, 32, 8.5f, 27, 3, 0};
    paint(cv, box_distance, &in_across, st->body);
    paint(cv, box_distance, &in_down, st->body);
    const float dx = b == PAD_DPAD_LEFT ? -1 : b == PAD_DPAD_RIGHT ? 1 : 0, dy = b == PAD_DPAD_UP ? -1 : b == PAD_DPAD_DOWN ? 1 : 0;
    const Box lit = {32 + dx * 17, 32 + dy * 17, dx ? 9 : 7, dy ? 9 : 7, 2, 0};
    paint(cv, box_distance, &lit, st->text);
}

static void menu_lines(Canvas *cv, float x, Rgb c) {
    for (int i = -1; i <= 1; i++) {
        const Box line = {x, 32 + i * 9.0f, 13, 2.4f, 2.4f, 0};
        paint(cv, box_distance, &line, c);
    }
}
static void view_windows(Canvas *cv, float x, Rgb c, Rgb body) {
    const Box back = {x + 4, 28, 10, 8, 2.5f, 3.4f}, front = {x - 4, 36, 10, 8, 2.5f, 0}, inside = {x - 4, 36, 6.6f, 4.6f, 1, 0};
    paint(cv, box_distance, &back, c);
    paint(cv, box_distance, &front, c);
    paint(cv, box_distance, &inside, body);
}

static void wide_button(Canvas *cv, const Style *st, int s, int b) {
    const Box rim = {64, 32, 58, 24, 24, 0}, body = {64, 32, 55, 21, 21, 0};
    paint(cv, box_distance, &rim, st->edge);
    paint(cv, box_distance, &body, st->body);
    if (s == REVO_ICONS_SWITCH) write(cv, b == PAD_START ? "+" : "-", 64, 32, 34, 44, st->text);
    else if (b == PAD_START) menu_lines(cv, 64, st->text);
    else view_windows(cv, 64, st->text, st->body);
}

static void other(Canvas *cv, const Style *st, const char *name) {
    const Box rim = {32, 32, 30, 20, 9, 0}, body = {32, 32, 27.5f, 17.5f, 7, 0};
    paint(cv, box_distance, &rim, st->edge);
    paint(cv, box_distance, &body, st->body);
    write(cv, name, 32, 32, 24, 48, st->text);
}

/* A keyboard key: a light cap on a darker lip, the key's name on it. */
static void keycap(Canvas *cv, float x, float y, float hw, float hh, const char *name, float height) {
    const float r = fminf(hw, hh) * 0.3f;
    const Box lip = {x, y + hh * 0.08f, hw, hh, r, 0}, cap = {x, y - hh * 0.06f, hw - 2.5f, hh - 4.0f, r * 0.8f, 0};
    paint(cv, box_distance, &lip, rgb(0x8C8C94));
    paint(cv, box_distance, &cap, rgb(0xEDEDF0));
    write(cv, name, x, y - hh * 0.06f, height, (hw - 6.0f) * 2.0f, rgb(0x202024));
}

/* The four moves as keys in an inverted T, one character each: up, left, down, right. */
static void move_keys(Canvas *cv, const char *k) {
    const char up[2] = {k[0], 0}, left[2] = {k[1], 0}, down[2] = {k[2], 0}, right[2] = {k[3], 0};
    keycap(cv, 32, 17, 10.5f, 10.5f, up, 11);
    keycap(cv, 10.5f, 44, 10.5f, 10.5f, left, 11);
    keycap(cv, 32, 44, 10.5f, 10.5f, down, 11);
    keycap(cv, 53.5f, 44, 10.5f, 10.5f, right, 11);
}

static void draw_key(Canvas *cv, const char *name, bool wide, bool moves) {
    memset(cv->rgba, 0, sizeof cv->rgba);
    cv->width = wide ? SIZE * 2 : SIZE;
    if (moves) move_keys(cv, name);
    else if (wide) keycap(cv, 64, 32, 60, 26, name, 28);
    else keycap(cv, 32, 32, 29, 27, name, strlen(name) == 1 ? 30 : 24);
}

static bool draw_button(Canvas *cv, int s, int b) {
    const Style st = style(s);
    const char *name = button_name(s, b);
    cv->width = SIZE;
    memset(cv->rgba, 0, sizeof cv->rgba);
    if (b >= PAD_A && b <= PAD_Y) face_button(cv, &st, b, name);
    else if (b == PAD_LEFTSHOULDER || b == PAD_RIGHTSHOULDER) shoulder(cv, &st, name, false);
    else if (b == PAD_TRIGGER_LEFT || b == PAD_TRIGGER_RIGHT) shoulder(cv, &st, name, true);
    else if (b == PAD_LEFTSTICK || b == PAD_RIGHTSTICK || b == PAD_LEFT_STICK_MOVED) stick(cv, &st, name, 32);
    else if (b >= PAD_DPAD_UP && b <= PAD_DPAD_RIGHT) dpad(cv, &st, b);
    else if (b == PAD_START || b == PAD_BACK) { cv->width = SIZE * 2; wide_button(cv, &st, s, b); }
    else if (name) other(cv, &st, name);
    else return false;
    return true;
}

static u32 texture_of(const Canvas *cv, int x0) {
    static u32 texels[SIZE * SIZE];
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++) {
            const float *p = cv->rgba[y * cv->width + x0 + x];
            const u32 r = (u32)lrintf(p[0] * 255.0f), g = (u32)lrintf(p[1] * 255.0f), b = (u32)lrintf(p[2] * 255.0f);
            const u32 a = (u32)lrintf(p[3] * 128.0f);
            texels[(SIZE - 1 - y) * SIZE + x] = r | g << 8 | b << 16 | a << 24;
        }
    return render_texture_new(SIZE, SIZE, texels);
}

static GuestPtr g_font;
static GuestPtr g_materials[LETTERS];
static u32 g_images[LETTERS];
static u64 g_keys[LETTERS];

static const Code *code_of(int letter) {
    for (size_t i = 0; i < sizeof k_codes / sizeof *k_codes; i++)
        if (k_codes[i].code - '0' == letter) return &k_codes[i];
    return NULL;
}

static void find_letters(void) {
    const GuestPtr font = *GUEST(GuestPtr, FONT);
    if (font == g_font) return;
    g_font = font;
    for (int i = 0; i < LETTERS; i++)
        g_materials[i] = font && code_of(i) ? GLI_pGetVisualMaterialForLetter((u32)(FIRST_LETTER + i)) : 0;
}

void icons_forget(void) { g_font = 0; }

#define NO_KEY (~(u64)0)

static u64 hash(const char *s) {
    u64 h = 1469598103934665603ull;
    for (; *s; s++) h = (h ^ (u8)*s) * 1099511628211ull;
    return h;
}

/* The keyboard's: the key bound to the action, or the four moves for the stick. */
static u64 keyboard_key_of(const Code *c, const char **name, bool *right, bool *moves) {
    *moves = c->show == SHOW_LEFT_STICK;
    *name = revo_icon_key(*moves ? MOVE_KEYS : c->action);
    if (!**name) return NO_KEY;
    *right = c->show == SHOW_ACTION_RIGHT;
    return (hash(*name) << 3 | (c->show == SHOW_ACTION ? 0 : 4) | (*moves ? 2 : 0) | (*right ? 1 : 0)) ^
           ((u64)REVO_ICONS_KEYBOARD << 60);
}

static u64 key_of(int i, int s, int *button, bool *right) {
    const Code *c = code_of(i);
    *right = false;
    if (!c || s == REVO_ICONS_PS2 || s == REVO_ICONS_KEYBOARD) return NO_KEY;
    switch (c->show) {
    case SHOW_LEFT_STICK: *button = PAD_LEFT_STICK_MOVED; break;
    case SHOW_ACTION: case SHOW_ACTION_LEFT: case SHOW_ACTION_RIGHT: {
        *button = revo_icon_binding(c->action);
        if (*button < 0) return NO_KEY;
        const bool wide = *button == PAD_START || *button == PAD_BACK;
        if (c->show == SHOW_ACTION_RIGHT && !wide) { *right = true; return (u64)s << 8 | 0xFF; }
        *right = c->show == SHOW_ACTION_RIGHT;
        break;
    }
    default: return NO_KEY;
    }
    return (u64)s << 8 | (u64)*button << 1 | (*right ? 1 : 0);
}

u32 icons_image(GuestPtr material) {
    if (!material) return 0;
    find_letters();
    if (!g_font) return 0;
    int i = 0;
    while (i < LETTERS && g_materials[i] != material) i++;
    if (i == LETTERS) return 0;
    const int s = revo_icon_style();
    int button = -1;
    bool right = false, moves = false;
    const char *name = NULL;
    const Code *c = code_of(i);
    const bool keyboard = s == REVO_ICONS_KEYBOARD && c;
    const u64 key = keyboard ? keyboard_key_of(c, &name, &right, &moves) : key_of(i, s, &button, &right);
    if (key == NO_KEY) return 0;
    if (key == g_keys[i] && g_images[i]) return g_images[i];
    if (g_images[i]) render_texture_free(g_images[i]);
    g_images[i] = 0;
    g_keys[i] = key;
    static Canvas cv;
    if (keyboard) {
        /* Start and Select take two letters, so their key is twice as wide. */
        const bool wide = c->show == SHOW_ACTION_LEFT || c->show == SHOW_ACTION_RIGHT;
        draw_key(&cv, name, wide, moves);
        g_images[i] = texture_of(&cv, wide && right ? SIZE : 0);
    } else if ((key & 0xFF) == 0xFF) {
        cv.width = SIZE;
        memset(cv.rgba, 0, sizeof cv.rgba);
        g_images[i] = texture_of(&cv, 0);
    } else if (draw_button(&cv, s, button)) {
        g_images[i] = texture_of(&cv, cv.width > SIZE && right ? SIZE : 0);
    } else {
        g_keys[i] = NO_KEY;
    }
    return g_images[i];
}
