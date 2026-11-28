#include "game_vars.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../mods.h"
#include "../runtime.h"

enum {
    ACTOR = 2,
    SO_LINKED = 0x04, SO_FIRST_CHILD = 0x08, SO_NEXT_BROTHER = 0x14,
    EO_BRAIN = 0x0C,
    MIND_MODEL = 0x00, MIND_INTELLIGENCE = 0x04,
    MODEL_INTELLIGENCE = 0x00,
    INTEL_COMPORT = 0x08,
    SCRIPT_COMPORTS = 0x00, SCRIPT_COMPORT_COUNT = 0x04, COMPORT_SIZE = 0x0C,
    COMPORT_RULES = 0x00, COMPORT_SCHEDULE = 0x04, COMPORT_RULE_COUNT = 0x08,
    NODE_SIZE = 8, END_TREE = 8, MAX_NODES = 8192,
};

typedef struct ScriptAt30 {
    u64 fingerprint;
    u32 comports;
    const char *what;
} ScriptAt30;

static const ScriptAt30 k_scripts[] = {
    {0x699D4555140197F5ull, 1u << 3 | 1u << 4, "the magic well"},
};

static bool in_ram(u32 a, u32 align) { return revo_guest_readable(a, align) && !(a & (align - 1)); }

static u64 mix(u64 h, u32 v) {
    for (int k = 0; k < 4; k++) {
        h ^= (v >> (8 * k)) & 0xFF;
        h *= 0x100000001B3ull;
    }
    return h;
}

static bool keeps_param(u8 type) {
    return type <= 0x05 || type == 0x09 || type == 0x0A || type == 0x0B || type == 0x0C || type == 0x13;
}

static u64 tree(u64 h, u32 node) {
    if (!in_ram(node, 4)) return mix(h, 0xFFFFFFFEu);
    for (int k = 0; k < MAX_NODES; k++, node += NODE_SIZE) {
        const u8 type = rd8(node + 7);
        if (type == END_TREE) break;
        h = mix(h, type);
        h = mix(h, rd8(node + 6));
        if (keeps_param(type)) h = mix(h, rd32(node));
    }
    return mix(h, 0xFFFFFFFFu);
}

u64 revo_script_fingerprint(u32 model) {
    const u32 script = in_ram(model, 4) ? rd32(model + MODEL_INTELLIGENCE) : 0;
    if (!in_ram(script, 4)) return 0;
    const u32 first = rd32(script + SCRIPT_COMPORTS), count = rd32(script + SCRIPT_COMPORT_COUNT);
    if (!in_ram(first, 4) || count > 256) return 0;
    u64 h = mix(0xCBF29CE484222325ull, count);
    for (u32 i = 0; i < count; i++) {
        const u32 c = first + i * COMPORT_SIZE;
        const u32 rules = rd32(c + COMPORT_RULES);
        const u8 n = in_ram(rules, 4) ? rd8(c + COMPORT_RULE_COUNT) : 0;
        h = mix(h, n);
        h = tree(h, rd32(c + COMPORT_SCHEDULE));
        for (u8 r = 0; r < n; r++) h = tree(h, rd32(rules + 4u * r));
    }
    return h;
}

typedef struct Found {
    u32 so, model;
    int script;
} Found;

typedef struct Model {
    u32 at;
    int script;
} Model;

enum { FOUND = 16, MODELS = 512, RESCAN = 300 };

static Found g_found[FOUND];
static int g_nfound;
static Model g_models[MODELS];
static int g_nmodels;
static char g_level[32];
static int g_scan_in;
static int g_running = -1;

static u32 mind_of(u32 so) {
    const u32 eo = in_ram(so, 4) ? rd32(so + SO_LINKED) : 0;
    const u32 brain = in_ram(eo, 4) ? rd32(eo + EO_BRAIN) : 0;
    const u32 mind = in_ram(brain, 4) ? rd32(brain) : 0;
    return in_ram(mind, 4) ? mind : 0;
}

static int script_of(u32 model) {
    for (int i = 0; i < g_nmodels; i++)
        if (g_models[i].at == model) return g_models[i].script;
    const u64 f = revo_script_fingerprint(model);
    int script = -1;
    for (size_t k = 0; k < sizeof k_scripts / sizeof *k_scripts; k++)
        if (f == k_scripts[k].fingerprint) script = (int)k;
    if (g_nmodels < MODELS) g_models[g_nmodels++] = (Model){model, script};
    return script;
}

static void look(u32 so, int depth, int *budget) {
    u32 guard = 0;
    for (u32 c = rd32(so + SO_FIRST_CHILD); in_ram(c, 4) && guard < 4096 && --*budget > 0;
         c = rd32(c + SO_NEXT_BROTHER), guard++) {
        const u32 mind = rd32(c) == ACTOR ? mind_of(c) : 0;
        const u32 model = mind ? rd32(mind + MIND_MODEL) : 0;
        if (in_ram(model, 4)) {
            const int script = script_of(model);
            bool known = false;
            for (int i = 0; i < g_nfound && !known; i++) known = g_found[i].so == c;
            if (script >= 0 && !known && g_nfound < FOUND) {
                g_found[g_nfound++] = (Found){c, model, script};
                revo_log("[mods] %s in %s (actor %08X): its sequences run at 30 Hz", k_scripts[script].what, g_level, c);
            }
        }
        if (depth < 4) look(c, depth + 1, budget);
    }
}

static s32 comport_now(u32 intel) {
    const u32 comport = rd32(intel + INTEL_COMPORT);
    const u32 slot = rd32(intel);
    const u32 script = in_ram(slot, 4) ? rd32(slot) : 0;
    if (!comport || !in_ram(script, 4)) return -1;
    const u32 base = rd32(script + SCRIPT_COMPORTS), n = rd32(script + SCRIPT_COMPORT_COUNT);
    if (comport < base || (comport - base) % COMPORT_SIZE || (comport - base) / COMPORT_SIZE >= n) return -1;
    return (s32)((comport - base) / COMPORT_SIZE);
}

const char *revo_current_level(void);

int revo_script_at_30hz(void) {
    const char *level = revo_current_level();
    if (strcmp(level, g_level)) {
        snprintf(g_level, sizeof g_level, "%s", level);
        g_nfound = g_nmodels = 0;
        g_scan_in = 30;
        g_running = -1;
    }
    if (g_scan_in > 0 && --g_scan_in == 0) {
        const u32 world = rd32(GAME_VAR(gp_stDynamicWorld));
        int budget = 8192;
        if (in_ram(world, 4)) look(world, 0, &budget);
        g_scan_in = RESCAN;
    }
    int running = -1;
    for (int i = 0; i < g_nfound && running < 0; i++) {
        const u32 mind = mind_of(g_found[i].so);
        if (!mind || rd32(mind + MIND_MODEL) != g_found[i].model) continue;
        const u32 intel = rd32(mind + MIND_INTELLIGENCE);
        const s32 c = in_ram(intel, 4) ? comport_now(intel) : -1;
        if (c >= 0 && c < 32 && (k_scripts[g_found[i].script].comports >> c & 1)) running = g_found[i].script;
    }
    if (running != g_running) {
        const int was = running >= 0 ? running : g_running;
        revo_log("[mods] %s: %s", k_scripts[was].what, running >= 0 ? "30 Hz" : "back to 60 Hz");
        g_running = running;
    }
    return running >= 0;
}
