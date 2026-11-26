#pragma once

#include <SDL.h>

void revo_settings_load(void);


bool revo_settings_event(const SDL_Event &e);

bool revo_settings_open(void);

void revo_settings_draw(int width, int height);

void revo_settings_toggle_fullscreen(void);

bool revo_settings_fullscreen(void);

void revo_window_set_fullscreen(bool on);
bool revo_window_fullscreen(void);

void revo_window_fit_aspect(void);

void revo_settings_dir(char *out, size_t size);

enum {
    REVO_ACT_CROSS, REVO_ACT_SQUARE, REVO_ACT_TRIANGLE, REVO_ACT_CIRCLE, REVO_ACT_L1, REVO_ACT_R1, REVO_ACT_L2,
    REVO_ACT_R2, REVO_ACT_START, REVO_ACT_SELECT, REVO_ACT_L3, REVO_ACT_R3, REVO_ACT_UP, REVO_ACT_DOWN,
    REVO_ACT_LEFT, REVO_ACT_RIGHT, REVO_ACT_SKIP, REVO_ACT_COUNT
};
enum { REVO_PAD_TRIGGER_LEFT = 100, REVO_PAD_TRIGGER_RIGHT = 101 };

int revo_action_bit(int action);
int revo_key_binding(int action, int slot);
int revo_pad_binding(int action);

enum { REVO_ICONS_PS2, REVO_ICONS_XBOX, REVO_ICONS_SWITCH, REVO_ICONS_DECK, REVO_ICONS_KEYBOARD };
int revo_controller_style(void);
/* Whether the keyboard was what the player last pressed, rather than a controller. */
bool revo_keyboard_last(void);
