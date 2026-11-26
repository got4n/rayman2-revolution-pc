#pragma once

#include <SDL.h>

void revo_ui_init(SDL_Window *window);

void revo_ui_event(const SDL_Event &e);

bool revo_ui_begin(void);

void revo_ui_end(void);

float revo_ui_scale(void);
