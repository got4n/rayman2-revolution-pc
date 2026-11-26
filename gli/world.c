#include "game_vars.h"
#include "gli.h"
#include "handles.h"

#define WORLD GAME_VAR(RW3_g_pWorld)

static struct { u32 handle; } g_world;

void world_create(void) {
    if (!g_world.handle) g_world.handle = handle_new(OBJECT_WORLD, &g_world);
    *GUEST(u32, WORLD) = g_world.handle;
}

bool world_exists(void) { return g_world.handle != 0; }

const float *world_origin(void) {
    static float origin[3];
    if (origin[0] == 0.0f) {
        const float low = -4000.0f, high = 4000.0f, side = fsub(high, low);
        const float diagonal = fsqrt(fadd(fadd(fmul(side, side), fmul(side, side)), fmul(side, side)));
        origin[0] = origin[1] = origin[2] = fsub(diagonal, low);
    }
    return origin;
}
