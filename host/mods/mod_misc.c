#include "game_vars.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../glue_rt.h"
#include "../mods.h"
#include "../runtime.h"
#include "PC.h"

int revo_skip_pressed(void);
u64 revo_frame_count(void);

enum {
#define IPT_INPUT_STRUCTURE GAME_VAR(IPT_g_hInputStructure)
    IPT_ENTRIES = 0x4C,
    IPT_ENTRY_SIZE = 0x18,
    IPT_ENTRY_DIALOGUE = 31,
    BUTTON_PRESSED = 0, /* the kinds pc_mod_button_condition gets: PressedBut, JustPressedBut, ... */
};

int32_t pc_mod_skip_wait(int32_t ticks) {
    static int fifteens;
    const bool skip = revo_mod(REVO_MOD_SHORT_LOADING) && ticks >= 15 && (ticks != 15 || fifteens < 16);
    if (ticks == 15) fifteens++;
    else fifteens = 0;
    if (skip) {
        return 1;
    }
    return 0;
}

int32_t pc_mod_fade_frames(int32_t frames) {
    if (!revo_mod(REVO_MOD_SHORT_LOADING) || frames <= 3) return frames;
    return 3;
}

int32_t pc_mod_skip_vblank_waits(void) {
    const bool skip = revo_mod(REVO_MOD_SHORT_LOADING);
    return skip;
}

static volatile int g_warm_up;

int revo_short_loading_warm_up(void) { return g_warm_up; }

static volatile int g_loading_movie;

/* With no loadings, the level loading movie would only flash a frame or two: it is shown black. */
int revo_loading_movie_hidden(void) { return g_loading_movie && revo_mod(REVO_MOD_SHORT_LOADING); }

void pc_mod_load_mark(int32_t step) {
    if (step == PC_LOAD_BEGIN) g_loading_movie = 1;
    if (step == PC_LOAD_MOVIE_DONE) g_loading_movie = 0;
    if (step == PC_LOAD_READY && revo_mod(REVO_MOD_SHORT_LOADING)) {
        g_warm_up = 1;
    }
}

void pc_mod_load_frame(int32_t frame) {
    if (g_warm_up && frame >= 6) g_warm_up = 0;
}

int32_t pc_mod_fade_in_step(int32_t step) {
    if (!revo_mod(REVO_MOD_SHORT_LOADING)) return step;
    if (g_warm_up) return 0;
    const int32_t units = pc_frame_units_per_tick();
    return 43 / (units > 0 ? units : 1);
}

static u64 g_moved_at;
static u64 g_waited_at;
static u64 g_text_at;

int revo_letterboxed_within(unsigned vblanks);

/* Skip moves the lines on with Fast-forward dialogues, and with Fast-forward cutscenes in a cutscene's lines. */
static bool skipping(void) {
    return (revo_mod(REVO_MOD_SKIP_DIALOGUES) || (revo_mod(REVO_MOD_SKIP_CUTSCENES) && revo_letterboxed_within(60))) &&
           revo_skip_pressed();
}

static bool skip_line_now(bool line) {
    const u64 now = revo_frame_count();
    if (line) g_waited_at = now;
    if (!skipping()) return false;
    if (now - g_moved_at < 4) return false;
    g_moved_at = now;
    return true;
}

static bool writing(u64 now) {
    return g_moved_at && g_waited_at <= g_moved_at + 1 && now - g_moved_at < 400 &&
           (now - g_moved_at < 30 || now - g_text_at < 8);
}

int revo_dialogue_fast(void) { return skipping() && writing(revo_frame_count()); }

int32_t pc_mod_any_button_now(void) { return skip_line_now(false); }

static u32 entry_index(PcAddress entry) {
    const u32 entries = rd32(IPT_INPUT_STRUCTURE + IPT_ENTRIES);
    return entry >= entries ? (entry - entries) / IPT_ENTRY_SIZE : ~0u;
}

int32_t pc_mod_entry_now(PcAddress entry) {
    return entry_index(entry) == IPT_ENTRY_DIALOGUE && skip_line_now(true);
}

/* Some scripts (the cutscenes' lines, Murfy's in Learn_10) test PressedBut(Action_TexteSuivant), whether the button
   is held, rather than JustPressedBut: Skip moves those lines on too, while a text is shown. They test it while the
   text is written too, so it does not mean that a line waits. */
int32_t pc_mod_button_condition(int32_t kind, PcAddress entry, int32_t value) {
    if (kind == BUTTON_PRESSED && !value && entry_index(entry) == IPT_ENTRY_DIALOGUE &&
        revo_frame_count() - g_text_at < 8)
        return skip_line_now(false);
    return value;
}

void pc_mod_text(int32_t id, PcAddress text) {
    g_text_at = revo_frame_count();
}

int32_t pc_hud_narrow(void) {
    extern float widescreen_factor(void);
    extern int render_hud_at_sides(void);
    if (!render_hud_at_sides()) return 65536;
    return (int32_t)(65536.0f / widescreen_factor() + 0.5f);
}

