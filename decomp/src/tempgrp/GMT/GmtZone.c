#include "common.h"

#include "GMT/GMT.h"

u16 GMT_fn_hGetCollideMaterialIdentifier(GMT_tdstCollideMaterial_* hCollideMaterial)
{
    return hCollideMaterial->xIdentifier;
}

void func_0011DBB8(GMT_tdstCollideMaterial_* hCollideMaterial, u16 xIdentifier)
{
    hCollideMaterial->xIdentifier = xIdentifier;
}

s32 GMT_fn_xTestCollideMaterialTypeOfIdentifier(GMT_tdstCollideMaterial_* hCollideMaterial, u32 wType)
{
    u16 xMask = 1 << wType;

    return (xMask & GMT_fn_hGetCollideMaterialIdentifier(hCollideMaterial)) != 0;
}

s32 GMT_fn_xMultipleTestCollideIdentifier(GMT_tdstCollideMaterial_* hCollideMaterial, u16 xMask)
{
    return (GMT_fn_hGetCollideMaterialIdentifier(hCollideMaterial) & xMask) == xMask;
}

void GMT_fn_vResetEntireMask(u16* p_xMask)
{
    *p_xMask = 0;
}
