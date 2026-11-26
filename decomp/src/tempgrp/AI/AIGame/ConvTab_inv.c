#include "common.h"

#include "AI/AI.h"

typedef struct AITableEntry14 {
    s8 szTypeParam[0x14];
} AITableEntry14;

extern AITableEntry14 inv_AI_a_stActionParamTypes[];

s8* func_001E6F40(u16 index)
{
    return inv_AI_a_stActionParamTypes[index].szTypeParam;
}

s32 func_001E6F60(void)
{
    return 148;
}

AIFieldAccessFn fn_p_fn_pGetFieldGetFunctionPtr(u16 eFieldId)
{
    return a_stFieldTable[eFieldId].get;
}

AIFieldAccessFn fn_p_fn_pGetFieldSetFunctionPtr(u16 eFieldId)
{
    return a_stFieldTable[eFieldId].set;
}

s32 func_001E6FA8(void)
{
    return 30;
}

s32 func_001E6FB0(void)
{
    return 532;
}
