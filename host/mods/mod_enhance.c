#include "game_vars.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "../glue_rt.h"
#include "../mods.h"
#include "../runtime.h"
#include "PC.h"

enum {
#define ENGINE GAME_VAR(g_stEngineStructure)
    LEVEL_NAME = 0x01, NEXT_LEVEL_NAME = 0x1F, NAME_SIZE = 0x1E,
#define MAIN_ACTORS GAME_VAR(dword_252618)
    ENGINE_OBJECT = 2,
    SO_LINKED = 0x04, EO_STANDARD_GAME = 0x04, HIT_POINTS = 0x29,
};

u64 revo_time_ticks(u32 frequency);
u64 revo_frame_count(void);
void revo_rumble(int strength, int ms);

static bool in_ram(u32 a) { return a >= REVO_GAME_LO && a < REVO_GAME_HI; }

static u32 standard_game(void) {
    const u32 so = rd32(MAIN_ACTORS);
    if (!in_ram(so) || (so & 3) || rd32(so) != ENGINE_OBJECT) return 0;
    const u32 engine_object = rd32(so + SO_LINKED);
    if (!in_ram(engine_object) || (engine_object & 3)) return 0;
    const u32 sg = rd32(engine_object + EO_STANDARD_GAME);
    return in_ram(sg) && !(sg & 3) ? sg : 0;
}

static char g_level[NAME_SIZE + 1];
static u64 g_run_start, g_level_start;
static u64 g_run_time, g_level_time;
static u64 g_level_frame;

static void level_and_timer(void) {
    char name[NAME_SIZE + 1];
    for (u32 k = 0; k < NAME_SIZE; k++) name[k] = (char)rd8(ENGINE + LEVEL_NAME + k);
    name[NAME_SIZE] = 0;
    const u64 now = revo_time_ticks(1000);
    const bool menu = !name[0] || !strncasecmp(name, "menu", 4);
    if (strcmp(name, g_level)) {
        memcpy(g_level, name, sizeof g_level);
        g_level_start = now;
        g_level_frame = 0;
        if (menu) g_run_start = 0;
        else if (!g_run_start) g_run_start = now;
    }
    if (!menu && !g_level_frame && standard_game()) g_level_frame = revo_frame_count();
    g_run_time = g_run_start ? now - g_run_start : 0;
    g_level_time = menu ? 0 : now - g_level_start;
}

const char *revo_current_level(void) { return g_level; }

u64 revo_level_frame(void) { return g_level_frame; }

u32 revo_input_now(void);

void revo_timer(u64 *run_ms, u64 *level_ms) {
    *run_ms = g_run_time;
    *level_ms = g_level_time;
}

void revo_timer_reset(void) { g_run_start = g_level_start = revo_time_ticks(1000); }

static void hit_points(void) {
    static u32 last_sg;
    static int last_hp = -1;
    const u32 sg = standard_game();
    if (!sg) {
        last_sg = 0;
        last_hp = -1;
        return;
    }
    const int hp = rd8(sg + HIT_POINTS);
    if (sg == last_sg && last_hp >= 0 && hp < last_hp) {
        const int strength = revo_set(REVO_SET_RUMBLE);
        if (strength > 0) revo_rumble(strength, hp == 0 ? 600 : 250);
    }
    last_sg = sg;
    last_hp = hp;
}

void revo_enhance_vblank(void) {
    level_and_timer();
    hit_points();
    { extern u64 revo_frame_count(void); void revo_progress_frame(u64); revo_progress_frame(revo_frame_count()); }
}
