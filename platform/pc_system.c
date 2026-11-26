#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "PC.h"
#include "mods.h"

/* The game's text languages, in the order of its own Language Choice menu: REVO_LANG_*. */
static const struct {
    const char *code;
    int32_t language;
} languages[REVO_LANG_COUNT] = {
    {"en", PC_LANGUAGE_ENGLISH}, {"fr", PC_LANGUAGE_FRENCH}, {"de", PC_LANGUAGE_GERMAN},
    {"it", PC_LANGUAGE_ITALIAN}, {"es", PC_LANGUAGE_SPANISH},
};

static int language_of(const char *code)
{
    for (int k = 0; code != NULL && k < REVO_LANG_COUNT; k++) {
        if (SDL_strncasecmp(code, languages[k].code, 2) == 0) {
            return k;
        }
    }
    return -1;
}

int revo_language(void)
{
    SDL_Locale *preferred = SDL_GetPreferredLocales();
    int found = -1;

    for (const SDL_Locale *l = preferred; l != NULL && l->language != NULL && found < 0; l++) {
        found = language_of(l->language);
    }
    SDL_free(preferred);
    return found < 0 ? REVO_LANG_ENGLISH : found;
}

int32_t pc_language(void)
{
    return languages[revo_language()].language;
}

int32_t pc_draw_distance(void)
{
    return (int32_t) (revo_draw_distance() * 100.0f + 0.5f);
}
