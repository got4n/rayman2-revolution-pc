#include "game_vars.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../glue_rt.h"
#include "../mods.h"
#include "../runtime.h"
#include "PC.h"

u64 revo_frame_count(void);

enum { UPDATE_WORK = 0x0C, IA_FLAGS = 0x72 };
#define IA_PLACED 0x0080u
#define IA_FORCED (0x1000u | 0x2000u | IA_PLACED)
enum { UPDATE_DYNAMICS = 0x04, UPDATE_CAMERA = 0x1C, PARAMETERS = 0x200, TARGET = 0x04, MOVE = 0x10,
       LINEAR_SPEED = 0x38, ANGULAR_SPEED = 0x48 };
enum { LINKED_OBJECT = 0x04, GLOBAL_MATRIX = 0x24, POSITION = 0x30, STANDARD_GAME = 0x04, CUSTOM_BITS = 0x2C };
#define CINE_CAMERA 0x8000u
#define ENGINE_CLOCK GAME_VAR(inv_GAM_engineClockState)

static float f32_at(u32 a) {
    u32 v = rd32(a);
    float f;
    memcpy(&f, &v, 4);
    return f;
}

static void set_f32(u32 a, float f) {
    u32 v;
    memcpy(&v, &f, 4);
    wr32(a, v);
}

static int in_ram(u32 a) { return revo_guest_readable(a, 4) && !(a & 3); }

static u32 parameters_of(u32 update) {
    u32 dynamics = rd32(update + UPDATE_DYNAMICS);
    return in_ram(dynamics) ? dynamics + PARAMETERS : 0;
}

static void vec_at(u32 a, float v[3]) {
    for (int k = 0; k < 3; k++) v[k] = f32_at(a + 4u * (u32)k);
}

static float frame_seconds(void) {
    u32 timer = rd32(ENGINE_CLOCK);
    float s = in_ram(timer) ? f32_at(timer + 0x2C) : 0.0f;
    return s > 0.0f && s < 0.5f ? s : 1.0f / 30.0f;
}

int revo_letterboxed(void);

static float axis(u8 raw) {
    const float v = ((float)raw - 128.0f) / 127.0f, dead = 0.2f;
    const float a = fabsf(v);
    if (a <= dead) return 0.0f;
    const float push = (a > 1.0f ? 1.0f : (a - dead) / (1.0f - dead));
    return v < 0 ? -push * push : push * push;
}

#define YAW_SPEED 2.6f
#define PITCH_SPEED 1.5f
#define LOW (-0.35f)
#define HIGH 1.2f
#define LEAD 2.4f
#define HELD_FRAMES 15

static bool g_free;
static u32 g_shoulders;
static u64 g_shoulders_frame;
static float g_heading, g_elevation;
static float g_game_heading;
static u32 g_game_parameters;

u32 revo_input_now(void);
#define BUTTON_R3 0x0004u
#define BUTTON_L1 0x0400u
#define BUTTON_R1 0x0800u

static float angle_between(float a, float b) {
    float d = fmodf(b - a, 6.2831853f);
    if (d > 3.1415927f) d -= 6.2831853f;
    if (d < -3.1415927f) d += 6.2831853f;
    return d;
}

static void orbit(u32 update) {
    u32 p = parameters_of(update), camera = rd32(update + UPDATE_CAMERA);
    u32 engine = in_ram(camera) ? rd32(camera + LINKED_OBJECT) : 0, matrix = in_ram(camera) ? rd32(camera + GLOBAL_MATRIX) : 0;
    bool cinematic = revo_letterboxed();
    if (in_ram(engine)) {
        u32 standard_game = rd32(engine + STANDARD_GAME);
        if (in_ram(standard_game) && (rd32(standard_game + CUSTOM_BITS) & CINE_CAMERA)) cinematic = true;
    }
    static int forced, clear = HELD_FRAMES;
    const u32 work = rd32(update + UPDATE_WORK);
    if (in_ram(work) && (rd16(work + IA_FLAGS) & IA_FORCED)) {
        forced++;
        clear = 0;
    } else {
        forced = 0;
        clear++;
    }
    const bool held = g_free ? forced > HELD_FRAMES : clear < HELD_FRAMES;
    const u16 ia = in_ram(work) ? rd16(work + IA_FLAGS) : 0;
    g_shoulders = 0;
    if (!revo_mod(REVO_MOD_RIGHT_STICK) || !p || !in_ram(matrix) || cinematic || held) {
        g_free = false;
        g_game_parameters = 0;
        if (revo_mod(REVO_MOD_RIGHT_STICK) && !cinematic && held && (ia & IA_FORCED) == IA_PLACED) {
            u8 sticks[4];
            revo_pad_sticks(sticks);
            float x = axis(sticks[0]);
            if (revo_set(REVO_SET_CAM_INVERT_X)) x = -x;
            if (x < -0.25f) g_shoulders = BUTTON_R1;
            if (x > 0.25f) g_shoulders = BUTTON_L1;
            g_shoulders_frame = revo_frame_count();
        }
        return;
    }

    float t[3], c[3], m[3], d[3];
    vec_at(p + TARGET, t);
    vec_at(matrix + POSITION, c);
    vec_at(p + MOVE, m);
    for (int k = 0; k < 3; k++) d[k] = c[k] + m[k] - t[k];
    const float across = sqrtf(d[0] * d[0] + d[1] * d[1]), length = sqrtf(across * across + d[2] * d[2]);
    if (across < 1e-3f || length < 0.05f) return;
    u8 sticks[4];
    revo_pad_sticks(sticks);
    float x = axis(sticks[0]), y = axis(sticks[1]);
    if (revo_set(REVO_SET_CAM_INVERT_X)) x = -x;
    if (revo_set(REVO_SET_CAM_INVERT_Y)) y = -y;
    const float dt = frame_seconds(), speed = (float)revo_set(REVO_SET_CAM_SPEED) / 100.0f;
    const float yaw = -x * YAW_SPEED * dt * speed;
    const float pitch = y * PITCH_SPEED * dt * speed;

    const float game_heading = atan2f(d[1], d[0]);
    if (p != g_game_parameters || fabsf(angle_between(g_game_heading, game_heading)) > 0.6f + fabsf(yaw))
        g_free = false;
    g_game_parameters = p;
    g_game_heading = game_heading;
    if (revo_input_now() & (BUTTON_R3 | BUTTON_L1 | BUTTON_R1)) g_free = false;
    if (!g_free) {
        if (x == 0.0f && y == 0.0f) return;
        const float now[3] = {c[0] - t[0], c[1] - t[1], c[2] - t[2]};
        const float flat = sqrtf(now[0] * now[0] + now[1] * now[1]);
        if (flat < 1e-3f) return;
        g_free = true;
        g_heading = atan2f(now[1], now[0]);
        g_elevation = atan2f(now[2], flat);
    }
    const float now[3] = {c[0] - t[0], c[1] - t[1], c[2] - t[2]};
    const float now_flat = sqrtf(now[0] * now[0] + now[1] * now[1]);
    const float now_heading = atan2f(now[1], now[0]), now_elevation = atan2f(now[2], now_flat);
    const float ahead = angle_between(now_heading, g_heading);
    if (fabsf(ahead + yaw) < LEAD || fabsf(ahead + yaw) < fabsf(ahead)) g_heading = angle_between(0.0f, g_heading + yaw);
    const float above = g_elevation - now_elevation;
    if (fabsf(above + pitch) < LEAD || fabsf(above + pitch) < fabsf(above)) {
        if (pitch > 0 && g_elevation < HIGH) g_elevation = fminf(g_elevation + pitch, HIGH);
        if (pitch < 0 && g_elevation > LOW) g_elevation = fmaxf(g_elevation + pitch, LOW);
    }

    const float flat = length * cosf(g_elevation);
    const float moved[3] = {t[0] + flat * cosf(g_heading) - c[0], t[1] + flat * sinf(g_heading) - c[1],
                            t[2] + length * sinf(g_elevation) - c[2]};
    for (int k = 0; k < 3; k++) set_f32(p + MOVE + 4u * (u32)k, moved[k]);
    const float turn = fabsf(angle_between(now_heading, g_heading)) + fabsf(g_elevation - now_elevation);
    const float need = sqrtf(moved[0] * moved[0] + moved[1] * moved[1] + moved[2] * moved[2]) / dt * 1.5f;
    if (f32_at(p + LINEAR_SPEED) < need) set_f32(p + LINEAR_SPEED, need);
    if (f32_at(p + ANGULAR_SPEED) < turn / dt * 1.5f) set_f32(p + ANGULAR_SPEED, turn / dt * 1.5f);
}

u32 revo_camera_shoulders(void) { return revo_frame_count() - g_shoulders_frame <= 2 ? g_shoulders : 0; }

u32 revo_pad_reads(void);

void pc_mod_camera_updated(PcAddress update) { orbit(update); }
