#include "common.h"

#include "AI/AI.h"

u8 fn_GetDsgVarType(u8 type)
{
    return a_stDsgVarTypeTable[type].typeCode;
}
