#include "game_vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clump.h"
#include "gli.h"
#include "handles.h"
#include "render.h"
#include "txd.h"

#define VIEWPORT_LIGHTS GAME_VAR(gs_aDEF_p_CurrentListOfLight)
#define VIEWPORT_LIGHT_COUNT GAME_VAR(gs_CurrentNumberOfLightInViewport)
#define MAX_POINT_LIGHTS 5

static s32 g_mask;
static int g_directional;
static int g_point;

static Light *light_of(u32 driver_light) { return handle_object(OBJECT_LIGHT, driver_light); }

static GuestPtr viewport_light(int i) { return GUEST(GuestPtr, VIEWPORT_LIGHTS)[i]; }
static s32 viewport_light_count(void) { return *GUEST(s32, VIEWPORT_LIGHT_COUNT); }

s32 lights_mask(void) { return g_mask; }
s32 lights_viewport_count(void) { return viewport_light_count(); }
GuestPtr lights_viewport(int i) { return viewport_light(i); }

static void set_color(Light *l, GuestPtr color) { memcpy(l->color, GUEST(float, color), sizeof l->color); }

void gli_ambient_color(GuestPtr color) {
    Light *ambient = level_ambient();
    if (!ambient) return;
    set_color(ambient, color);
    ambient->on = true;
}

void gli_ambient_off(void) {
    Light *ambient = level_ambient();
    if (ambient) ambient->on = false;
}

void gli_light_mask(s32 mask) { g_mask = mask; }

u32 gli_remove_light(u32 driver_light) {
    Light *l = light_of(driver_light);
    if (!l) return driver_light;
    if (l != level_ambient() && l->on) {
        if (l->type == RW_LIGHT_DIRECTIONAL) g_directional--;
        else g_point--;
    }
    l->on = false;
    return driver_light;
}

void lights_all_out(void) {
    int count;
    Light *const *lights = level_lights(&count);
    for (int i = 0; i < count; i++) gli_remove_light(lights[i]->handle);
}

static void put_in(GuestPtr light) {
    GliLight *gl = GUEST(GliLight, light);
    Light *l = light_of(gl->driver_light);
    if (!l) return;
    if (l == level_ambient()) {
        memcpy(l->color, gl->color, sizeof l->color);
        l->on = true;
        return;
    }
    if (l->on) return;
    if (l->type == RW_LIGHT_DIRECTIONAL) {
        if (g_directional == 0) {
            l->on = true;
            g_directional++;
        } else {
        }
    } else if (g_point < MAX_POINT_LIGHTS) {
        l->on = true;
        g_point++;
    } else {
    }
}

void lights_of_viewport_in(void) {
    for (int i = 0; i < viewport_light_count(); i++)
        if ((g_mask >> i) & 1) {
            put_in(viewport_light(i));
        }
}

void lights_of_viewport(LightList *list) {
    list->count = 0;
    for (int i = 0; i < viewport_light_count(); i++)
        if (((u32)g_mask >> i) & 1 && list->count < LIGHT_LIST_SIZE) list->lights[list->count++] = viewport_light(i);
}

void lights_put_in(const LightList *list) {
    for (int i = 0; i < list->count; i++) put_in(list->lights[i]);
}

void lights_recount(void) { g_directional = g_point = 0; }

void gli_light_matrix(GuestPtr light) {
    GliLight *gl = GUEST(GliLight, light);
    Light *l = light_of(gl->driver_light);
    if (l && l != level_ambient() && l->frame) frame_from_matrix(l->frame, gl->matrix);
}

void gli_light_color(GuestPtr light, GuestPtr color) {
    Light *l = light_of(GUEST(GliLight, light)->driver_light);
    if (l) set_color(l, color);
}

void gli_light_cone_angle(GuestPtr light) {
    GliLight *gl = GUEST(GliLight, light);
    Light *l = light_of(gl->driver_light);
    if (l) l->cone_angle = gl->angle1;
}

void gli_light_radius(GuestPtr light) {
    GliLight *gl = GUEST(GliLight, light);
    Light *l = light_of(gl->driver_light);
    if (l) l->radius = gl->far_distance;
}

