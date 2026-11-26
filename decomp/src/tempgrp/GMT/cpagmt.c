#include "common.h"

#include "GMT/GMT.h"

#define GMT_C_InvalidCollideMaterial ((GMT_tdstCollideMaterial_*) -1)

s32 GMT_fn_b_ulIsValid(GMT_tdstGameMaterial_* hGameMat)
{
    return hGameMat != (GMT_tdstGameMaterial_*) 0xCDCDCDCD && hGameMat != NULL;
}

s32 GMT_fn_hGetSoundMaterial(GMT_tdstGameMaterial_* hGameMat)
{
    return hGameMat->hSoundMaterial;
}

void GMT_fn_vSetSoundMaterial(GMT_tdstGameMaterial_* hGameMat, s32 hSoundMaterial)
{
    hGameMat->hSoundMaterial = hSoundMaterial;
}

GMT_tdstCollideMaterial_* GMT_fn_hGetCollideMaterial(GMT_tdstGameMaterial_* hGameMat)
{
    return &hGameMat->collideMaterial;
}

s32 GMT_fn_xTestCollideIdentifierTypeOfGameMaterial(GMT_tdstGameMaterial_* hGameMat, u16 wType)
{
    GMT_tdstCollideMaterial_* hCollideMaterial;

    hCollideMaterial = GMT_fn_hGetCollideMaterial(hGameMat);
    if (hCollideMaterial == GMT_C_InvalidCollideMaterial)
        return 0;
    return GMT_fn_xTestCollideMaterialTypeOfIdentifier(hCollideMaterial, wType);
}

s32 GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(GMT_tdstGameMaterial_* hGameMat, u16 xMask)
{
    GMT_tdstCollideMaterial_* hCollideMaterial;

    if (xMask == 0)
        return 1;
    hCollideMaterial = GMT_fn_hGetCollideMaterial(hGameMat);
    if (hCollideMaterial == NULL || hCollideMaterial == GMT_C_InvalidCollideMaterial)
        return 0;
    return GMT_fn_xMultipleTestCollideIdentifier(hCollideMaterial, xMask);
}

void func_0011DBA8(GMT_tdstCollideMaterial_* hCollideMaterial, u16 wTypeOfZone)
{
    hCollideMaterial->wTypeOfZone = wTypeOfZone;
}
