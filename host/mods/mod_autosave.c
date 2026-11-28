#include "game_vars.h"
#include <stdio.h>
#include <string.h>

#include "../glue_rt.h"
#include "../mods.h"
#include "../runtime.h"
#include "PC.h"

enum {
    NODE_SIZE = 8,
    NODE_DEPTH = 6, NODE_TYPE = 7,
    TYPE_KEYWORD = 0, TYPE_OPERATOR = 2, TYPE_END = 8, TYPE_PERSO = 23, TYPE_TEXT = 27,
    KEYWORD_THEN = 10, OPERATOR_DOT = 12,
    TEXT_SAVE_QUESTION = 410,
    TEXT_SAVING = 438,
    TEXT_TABLES = 0x0C,
    MAX_BRANCH = 400,
};

u64 revo_frame_count(void);
int gli_hud_words_make(const char *latin1);

static int in_ram(u32 a) { return revo_guest_readable(a, 1); }

static u32 then_node(PcAddress condition, int n) {
    const u32 depth = rd8(condition + NODE_DEPTH);
    if (depth == 0) return 0;
    u32 node = condition + NODE_SIZE;
    if (rd8(node + NODE_TYPE) != TYPE_KEYWORD || (rd32(node) & 0xFFFF) != KEYWORD_THEN ||
        rd8(node + NODE_DEPTH) != depth - 1)
        return 0;
    node += NODE_SIZE * (u32)(n + 1);
    if (!in_ram(node) || rd8(node + NODE_TYPE) == TYPE_END || rd8(node + NODE_DEPTH) < depth) return 0;
    return node;
}

int32_t pc_mod_autosave_prompt(PcAddress condition) {
    if (!revo_mod(REVO_MOD_AUTOSAVE) || !in_ram(condition) || (condition & 3)) return 0;
    for (int n = 0; n < MAX_BRANCH; n++) {
        const u32 node = then_node(condition, n);
        if (!node) return 0;
        if (rd8(node + NODE_TYPE) == TYPE_TEXT && (rd32(node) & 0xFFFF) == TEXT_SAVE_QUESTION) return 1;
    }
    return 0;
}

PcAddress pc_mod_autosave_clock(PcAddress condition) {
    for (int n = 1; n < MAX_BRANCH; n++) {
        const u32 node = then_node(condition, n);
        if (!node) return 0;
        const u32 before = node - NODE_SIZE;
        if (rd8(node + NODE_TYPE) == TYPE_PERSO && rd8(before + NODE_TYPE) == TYPE_OPERATOR &&
            (rd32(before) & 0xFFFF) == OPERATOR_DOT)
            return node;
    }
    return 0;
}

static char g_text[64] = "SAVING GAME";
static volatile u32 g_saves;

enum { LATIN = 64 };

static void saving_latin(char *latin) {
    const u32 texts = rd32(GAME_VAR(inv_TXT_textTableState) + TEXT_TABLES);
    const u32 text = in_ram(texts) ? rd32(texts + 4 * TEXT_SAVING) : 0;
    if (!in_ram(text)) return;
    u32 n = 0, k = 0;
    while (k < 200 && rd8(text + k) && !(rd8(text + k) == '/' && (rd8(text + k + 1) | 0x20) == 'o')) k++;
    while (k < 200 && rd8(text + k) && rd8(text + k) != ':') k++;
    if (k >= 200 || rd8(text + k) != ':') return;
    for (k++; n < LATIN - 1 && rd8(text + k) && rd8(text + k) != '/'; k++) latin[n++] = (char)rd8(text + k);
    while (n && latin[n - 1] == ' ') n--;
    if (n == 0) return;
    latin[n] = 0;
    char out[sizeof g_text];
    u32 o = 0;
    for (u32 i = 0; i < n && o + 2 < sizeof out; i++) {
        unsigned c = (u8)latin[i];
        if (c >= 'a' && c <= 'z') c -= 0x20;
        else if (c >= 0xE0 && c <= 0xFE && c != 0xF7) c -= 0x20;
        if (c < 0x80) out[o++] = (char)c;
        else {
            out[o++] = (char)(0xC0 | (c >> 6));
            out[o++] = (char)(0x80 | (c & 0x3F));
        }
    }
    out[o] = 0;
    memcpy(g_text, out, o + 1);
}

static void saving_words(void) {
    char latin[LATIN] = "SAVING GAME";
    saving_latin(latin);
    if (!gli_hud_words_make(latin)) revo_log("[autosave] the game's letters not found: the words in the menu's font");
}

void pc_mod_autosave_done(int32_t saved) {
    revo_log("[autosave %llu] %s", (unsigned long long)revo_frame_count(),
             saved ? "the game saved, no question" : "the save failed: the question as in the original");
    if (!saved) return;
    saving_words();
    revo_log("[autosave] shown: %s", g_text);
    g_saves++;
}

u32 revo_autosave_count(const char **words) {
    *words = g_text;
    return g_saves;
}
