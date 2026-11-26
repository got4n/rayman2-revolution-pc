#include "common.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

s32* inv_DNM_fn_p_stCreateIdCard(s32 kind)
{
    s32* characteristics;

    switch (kind) {
    case 0:
        characteristics = AI_fn_p_vTrueAlloc(gp_6F74, 0x10);
        *characteristics = 0;
        break;
    case 1:
        characteristics = AI_fn_p_vTrueAlloc(gp_6F74, 0x3C);
        *characteristics = kind;
        break;
    default:
        return NULL;
    }
    return characteristics;
}
