#include "common.h"

#include "ps2/libc.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "AI/AI.h"
#include "GAM/GAM.h"

#define TXT_HIGH_ID_BASE 20000

typedef struct TextAllocation {
    s32 unk00;
    s16 unk04;
} TextAllocation;

TextAllocation* func_001159D0(void)
{
    TextAllocation* allocation = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(TextAllocation));

    allocation->unk00 = 0;
    allocation->unk04 = 0;
    return allocation;
}

void inv_TXT_fn_vSetLanguage(u8 language)
{
    u32 index;
    char* destination;

    inv_TXT_textTableState.language = language;
    destination = (char*) (inv_TXT_textTableState.texts + inv_TXT_textTableState.count);
    for (index = 0; index < inv_TXT_textTableState.count; index++) {
        u32 length = strlen(inv_TXT_textTableState.languageTexts[language][index]) + 1;
        char* copy = destination;

        inv_TXT_textTableState.texts[index] = destination;
        destination += length;
        memcpy(copy, inv_TXT_textTableState.languageTexts[language][index], length);
    }
}

#ifdef PC_PORT
static char* fn_p_GetTextById_game(s32 id);

char* fn_p_GetTextById(s32 id)
{
    char* text = fn_p_GetTextById_game(id);

    pc_mod_text(id, (PcAddress) text);
    return text;
}

#define fn_p_GetTextById fn_p_GetTextById_game
#endif
char* fn_p_GetTextById(s32 id)
{
    char** table;

    if (id >= TXT_HIGH_ID_BASE) {
        id -= TXT_HIGH_ID_BASE;
        table = inv_TXT_textTableState.highIdTexts;
    } else {
        table = inv_TXT_textTableState.texts;
    }
    return table[id];
}
#ifdef PC_PORT
#undef fn_p_GetTextById
#endif
