/* The Extras tab's cheats: Rayman's speed, infinite health and noclip. */
#include "game_vars.h"
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#include "../glue_rt.h"
#include "../mods.h"
#include "../runtime.h"
#include "PC.h"

enum {
#define MAIN_ACTORS GAME_VAR(dword_252618)
    SO_LINKED = 0x04, SO_GLOBAL_MATRIX = 0x24,
    MATRIX_SIZE = 0x54, POSITION = 0x30,
    DYN_FLAGS = 0x0C, DYN_IMPOSE_SPEED = 0x24, DYN_PROPOSE_SPEED = 0x30, DYN_PREVIOUS_SPEED = 0x3C,
    DYN_SPEED_ANIM = 0x54, DYN_ADD_TRANSLATION = 0x6C, DYN_PREVIOUS_MATRIX = 0x78, DYN_CURRENT_MATRIX = 0xCC,
    DYN_ABSOLUTE_PREVIOUS_MATRIX = 0x280, DYN_PREV_PREVIOUS_MATRIX = 0x2D4,
    DYNAMICS_COMPLETE = 0x4,
};
#define ENGINE_CLOCK GAME_VAR(inv_GAM_engineClockState)

#define BUTTON_R2 0x0200u
#define BUTTON_CROSS 0x4000u
#define BUTTON_SQUARE 0x8000u

#define FLY_SPEED 10.0f
#define FLY_FAST 3.0f
#define YAW_SPEED 2.6f
#define PITCH_SPEED 1.5f
#define LOW (-1.2f)
#define HIGH 1.4f
#define LOOK_HEIGHT 1.2f
#define START_ELEVATION 0.35f
#define NEAREST 4.0f
#define FARTHEST 10.0f

static bool in_ram(u32 a) { return revo_guest_readable(a, 4) && !(a & 3); }

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

static void set_vec(u32 a, float x, float y, float z) {
    set_f32(a, x);
    set_f32(a + 4, y);
    set_f32(a + 8, z);
}

static float frame_seconds(void) {
    u32 timer = rd32(ENGINE_CLOCK);
    float s = in_ram(timer) ? f32_at(timer + 0x2C) : 0.0f;
    return s > 0.0f && s < 0.5f ? s : 1.0f / 30.0f;
}

static float axis(u8 raw) {
    const float v = ((float)raw - 128.0f) / 127.0f, dead = 0.2f, a = fabsf(v);
    if (a <= dead) return 0.0f;
    const float push = a > 1.0f ? 1.0f : (a - dead) / (1.0f - dead);
    return v < 0 ? -push : push;
}

static bool is_rayman(u32 so) { return so && so == rd32(MAIN_ACTORS); }

static bool rayman_position(float p[3]) {
    const u32 so = rd32(MAIN_ACTORS);
    if (!in_ram(so)) return false;
    const u32 m = rd32(so + SO_GLOBAL_MATRIX);
    if (!in_ram(m)) return false;
    for (int k = 0; k < 3; k++) p[k] = f32_at(m + POSITION + 4u * (u32)k);
    return true;
}

/* Rayman's speed, in percent: his mechanics run with his time step times the setting, so walls
   and floors still stop him. */
int32_t pc_mod_mechanics_percent(PcAddress so) {
    return is_rayman(so) ? (int32_t)(100.0f * revo_rayman_speed() + 0.5f) : 100;
}

int revo_letterboxed(void);

/* The camera's time step in percent: scaled as Rayman's, out of the cutscenes, so it keeps up. */
int32_t pc_mod_camera_percent(void) {
    return revo_letterboxed() ? 100 : (int32_t)(100.0f * revo_rayman_speed() + 0.5f);
}

/* Infinite health: hits on Rayman take nothing off. */
int32_t pc_mod_no_damage(PcAddress engine_object) {
    if (!revo_set(REVO_SET_INF_HEALTH)) return 0;
    const u32 so = rd32(MAIN_ACTORS);
    return in_ram(so) && engine_object == rd32(so + SO_LINKED);
}

/* Noclip. The camera orbits Rayman at a heading, an elevation and a distance of its own, through
   walls; the left stick moves him where it looks. */
static bool g_noclip, g_orbit_set;
static float g_heading, g_elevation, g_distance;

const char *revo_current_level(void);

/* Only the Rayman the player moves: not the one of the menus or of a cutscene. */
static bool noclip_on(void) {
    return revo_set(REVO_SET_NOCLIP) != 0 && in_ram(rd32(MAIN_ACTORS)) && strncasecmp(revo_current_level(), "menu", 4) &&
           !revo_letterboxed();
}

static void copy_matrix(u32 dst, u32 src) {
    for (u32 k = 0; k < MATRIX_SIZE; k += 4) wr32(dst + k, rd32(src + k));
}

static void at_rest(u32 dynamics, u32 m) {
    set_vec(dynamics + DYN_IMPOSE_SPEED, 0, 0, 0);
    set_vec(dynamics + DYN_PROPOSE_SPEED, 0, 0, 0);
    set_vec(dynamics + DYN_PREVIOUS_SPEED, 0, 0, 0);
    set_vec(dynamics + DYN_SPEED_ANIM, 0, 0, 0);
    set_vec(dynamics + DYN_ADD_TRANSLATION, 0, 0, 0);
    copy_matrix(dynamics + DYN_CURRENT_MATRIX, m);
    copy_matrix(dynamics + DYN_PREVIOUS_MATRIX, m);
    if (rd32(dynamics + DYN_FLAGS) & DYNAMICS_COMPLETE) {
        copy_matrix(dynamics + DYN_ABSOLUTE_PREVIOUS_MATRIX, m);
        copy_matrix(dynamics + DYN_PREV_PREVIOUS_MATRIX, m);
    }
}

static void fly(u32 dynamics, u32 so) {
    const u32 m = rd32(so + SO_GLOBAL_MATRIX);
    if (!in_ram(m)) return;
    float p[3] = {f32_at(m + POSITION), f32_at(m + POSITION + 4), f32_at(m + POSITION + 8)};
    u8 s[4];
    revo_pad_sticks(s);
    const float sx = axis(s[2]), sy = -axis(s[3]);
    const u32 held = ~revo_pad_buttons() & 0xFFFFu;
    const float up = ((held & BUTTON_CROSS) ? 1.0f : 0.0f) - ((held & BUTTON_SQUARE) ? 1.0f : 0.0f);
    const float step = FLY_SPEED * revo_rayman_speed() * ((held & BUTTON_R2) ? FLY_FAST : 1.0f) * frame_seconds();
    /* Forward is from the camera to Rayman, flat. */
    const float fx = -cosf(g_heading), fy = -sinf(g_heading);
    const float mx = fy * sx + fx * sy, my = -fx * sx + fy * sy;
    p[0] += mx * step;
    p[1] += my * step;
    p[2] += up * step;
    set_vec(m + POSITION, p[0], p[1], p[2]);
    const float flat = sqrtf(mx * mx + my * my);
    if (flat > 0.05f) {
        const float dx = mx / flat, dy = my / flat;
        set_vec(m + 0x00, -dy, dx, 0.0f);
        set_vec(m + 0x10, -dx, -dy, 0.0f);
        set_vec(m + 0x20, 0.0f, 0.0f, 1.0f);
    }
    at_rest(dynamics, m);
}

int32_t pc_mod_noclip_active(PcAddress so) { return g_noclip && noclip_on() && is_rayman(so); }

int32_t pc_mod_noclip(PcAddress dynamics, PcAddress so) {
    if (!is_rayman(so)) return 0;
    const bool on = noclip_on() && in_ram(dynamics);
    if (on != g_noclip) {
        revo_log("[noclip] %s", on ? "on" : "off");
        g_noclip = on;
        g_orbit_set = false;
    }
    if (!on || !g_orbit_set) return on;
    fly(dynamics, so);
    return 1;
}

/* Called by the camera's mechanics: in noclip, gives where the camera is and what it looks at,
   and the game's own collision and target are left out. */
int32_t pc_mod_noclip_camera(PcAddress position, PcAddress target) {
    if (!g_noclip || !noclip_on()) return 0;
    float r[3];
    if (!rayman_position(r)) return 0;
    r[2] += LOOK_HEIGHT;
    if (!g_orbit_set) {
        /* Start where the game's camera is. */
        float c[3] = {f32_at(position), f32_at(position + 4), f32_at(position + 8)};
        const float d[3] = {c[0] - r[0], c[1] - r[1], c[2] - r[2]};
        const float across = sqrtf(d[0] * d[0] + d[1] * d[1]);
        g_distance = sqrtf(across * across + d[2] * d[2]);
        if (!(g_distance >= NEAREST)) g_distance = NEAREST;
        if (g_distance > FARTHEST) g_distance = FARTHEST;
        g_heading = across > 1e-3f ? atan2f(d[1], d[0]) : 0.0f;
        g_elevation = atan2f(d[2], across > 1e-3f ? across : 1.0f);
        /* From above him, or the camera may start under the floor he stands on. */
        if (g_elevation < START_ELEVATION) g_elevation = START_ELEVATION;
        g_orbit_set = true;
    }
    u8 s[4];
    revo_pad_sticks(s);
    float x = axis(s[0]), y = axis(s[1]);
    if (revo_set(REVO_SET_CAM_INVERT_X)) x = -x;
    if (revo_set(REVO_SET_CAM_INVERT_Y)) y = -y;
    const float dt = frame_seconds(), speed = (float)revo_set(REVO_SET_CAM_SPEED) / 100.0f;
    g_heading -= x * YAW_SPEED * dt * speed;
    g_elevation += y * PITCH_SPEED * dt * speed;
    if (g_elevation < LOW) g_elevation = LOW;
    if (g_elevation > HIGH) g_elevation = HIGH;
    const float flat = g_distance * cosf(g_elevation);
    set_vec(position, r[0] + flat * cosf(g_heading), r[1] + flat * sinf(g_heading),
            r[2] + g_distance * sinf(g_elevation));
    set_vec(target, r[0], r[1], r[2]);
    return 1;
}
