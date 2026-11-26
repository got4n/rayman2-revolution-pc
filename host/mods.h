#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REVO_MOD_DIAGONAL,
    REVO_MOD_RIGHT_STICK,
    REVO_MOD_60FPS,
    REVO_MOD_WIDESCREEN,
    REVO_MOD_WIDESCREEN_1610,
    REVO_MOD_INTERPOLATION,
    REVO_MOD_SKIP_CUTSCENES,
    REVO_MOD_SHORT_LOADING,
    REVO_MOD_SKIP_DIALOGUES,
    REVO_MOD_SMOOTH,
    REVO_MOD_AUTOSAVE,
    REVO_MOD_COUNT
};

int revo_mod(int mod);

enum {
    REVO_SET_FOV,
    REVO_SET_HUD_EDGES,
    REVO_SET_NO_BARS,
    REVO_SET_SCALING,
    REVO_SET_POST,
    REVO_SET_PAUSE_FOCUS,
    REVO_SET_PAUSE_MENU,
    REVO_SET_TIMER,
    REVO_SET_RUMBLE,
    REVO_SET_CAM_INVERT_X,
    REVO_SET_CAM_INVERT_Y,
    REVO_SET_CAM_SPEED,
    REVO_SET_VOLUME,
    REVO_SET_DRAW_DISTANCE,
    REVO_SET_BUTTON_ICONS,
    REVO_SET_TEXTURE_FILTER,
    REVO_SET_WIDER,
    REVO_SET_DISCORD,
    REVO_SET_UPDATE_CHECK,
    REVO_SET_SPEED,
    REVO_SET_INF_HEALTH,
    REVO_SET_NOCLIP,
    REVO_SET_COUNT
};

int revo_set(int setting);

/* The game's text languages, in the order of its Language Choice menu. */
enum { REVO_LANG_ENGLISH, REVO_LANG_FRENCH, REVO_LANG_GERMAN, REVO_LANG_ITALIAN, REVO_LANG_SPANISH, REVO_LANG_COUNT };
/* The language the game starts in, which the port's own windows use too: the system's first language
   that the game has, else English. */
int revo_language(void);

float revo_draw_distance(void);
float revo_rayman_speed(void);
int revo_extras_on(void);

void revo_discord_start(void);
int revo_discord_available(void);


void revo_view_aspect(int *across, int *down);

#ifdef __cplusplus
}
#endif
