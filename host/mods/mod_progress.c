#include <stdatomic.h>
#include <string.h>

#include "../game_vars.h"
#include "../runtime.h"

enum {
    GLOBAL_VARS = 48,
    GLOBAL_SIZE = 1684,
    VAR_CAGES_IN_WORLD = 0,
    VAR_LUMS_IN_WORLD = 1,
    VAR_CAGES_FREED = 8,
    VAR_LUMS_TAKEN = 10,
    TYPE_INTEGER_ARRAY = 23,
};

static atomic_int g_lums = -1, g_cages = -1;

static int in_ram(u32 a) { return revo_guest_readable(a, 4) && !(a & 3); }

typedef struct { u32 vars, buf; } Dsg;

static int dsg_of(u32 so, Dsg *d) {
    const u32 object = rd32(so + 0x04), brain = in_ram(object) ? rd32(object + 0x0C) : 0;
    const u32 mind = in_ram(brain) ? rd32(brain) : 0, mem = in_ram(mind) ? rd32(mind + 0x0C) : 0;
    const u32 pp = in_ram(mem) ? rd32(mem) : 0, desc = in_ram(pp) ? rd32(pp) : 0;
    if (!in_ram(desc) || rd8(desc + 0x0A) != GLOBAL_VARS || rd16(desc + 8) != GLOBAL_SIZE) return 0;
    d->vars = rd32(desc + 4);
    d->buf = rd32(mem + 8);
    return in_ram(d->vars & ~3u) && in_ram(d->buf & ~3u);
}

static unsigned type_of(const Dsg *d, unsigned v) { return rd8(d->vars + 6 * v + 4); }

static u32 at(const Dsg *d, unsigned v) { return d->buf + rd16(d->vars + 6 * v); }

static int sum(const Dsg *d, unsigned taken, unsigned most) {
    const u32 a = at(d, taken), m = at(d, most);
    const int count = (int)((rd32(a + 4) >> 8) & 0xFF);
    if (count != (int)((rd32(m + 4) >> 8) & 0xFF)) return -1;
    int total = 0;
    for (int k = 0; k < count - 1 && k < 64; k++) {
        const int n = (int)rd32(a + 12 + 4 * (u32)k), max = (int)rd32(m + 12 + 4 * (u32)k);
        if (n < 0 || n > 1000 || max < 0 || max > 1000) return -1;
        total += n < max ? n : max;
    }
    return total;
}

static int is_global(const Dsg *d) {
    return type_of(d, VAR_LUMS_IN_WORLD) == TYPE_INTEGER_ARRAY && type_of(d, VAR_CAGES_IN_WORLD) == TYPE_INTEGER_ARRAY &&
           type_of(d, VAR_LUMS_TAKEN) == TYPE_INTEGER_ARRAY && type_of(d, VAR_CAGES_FREED) == TYPE_INTEGER_ARRAY;
}

static u32 find(u32 so, int depth, Dsg *d) {
    for (u32 c = rd32(so + 0x08); in_ram(c); c = rd32(c + 0x14)) {
        if (dsg_of(c, d) && is_global(d)) return c;
        if (depth < 2) {
            const u32 found = find(c, depth + 1, d);
            if (found) return found;
        }
    }
    return 0;
}

void revo_progress_frame(u64 frame) {
    if (frame % 30) return;
    const u32 world = rd32(GAME_VAR(gp_stDynamicWorld));
    Dsg d;
    if (!in_ram(world) || !find(world, 0, &d)) {
        atomic_store(&g_lums, -1);
        atomic_store(&g_cages, -1);
        return;
    }
    atomic_store(&g_lums, sum(&d, VAR_LUMS_TAKEN, VAR_LUMS_IN_WORLD));
    atomic_store(&g_cages, sum(&d, VAR_CAGES_FREED, VAR_CAGES_IN_WORLD));
}

void revo_progress(int *lums, int *cages) {
    *lums = atomic_load(&g_lums);
    *cages = atomic_load(&g_cages);
}
