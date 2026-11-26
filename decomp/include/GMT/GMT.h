#ifndef REVO_GMT_GMT_H
#define REVO_GMT_GMT_H

#include "ps2/types.h"

typedef struct GMT_tdstCollideMaterial_ {
    u16 wTypeOfZone;
    u16 xIdentifier;
} GMT_tdstCollideMaterial_;

typedef struct GMT_tdstGameMaterial_ {
    s32 hSoundMaterial;
    GMT_tdstCollideMaterial_ collideMaterial;
} GMT_tdstGameMaterial_;

extern u16 GMT_fn_hGetCollideMaterialIdentifier(GMT_tdstCollideMaterial_* hCollideMaterial);
extern s32 GMT_fn_xMultipleTestCollideIdentifier(GMT_tdstCollideMaterial_* hCollideMaterial,
                                                 u16 xMask);
extern s32 GMT_fn_xTestCollideMaterialTypeOfIdentifier(GMT_tdstCollideMaterial_* hCollideMaterial,
                                                       u32 wType);
extern void GMT_fn_vResetEntireMask(u16* p_xMask);

extern GMT_tdstCollideMaterial_* GMT_fn_hGetCollideMaterial(GMT_tdstGameMaterial_* hGameMat);
extern s32 GMT_fn_hGetSoundMaterial(GMT_tdstGameMaterial_* hGameMat);
extern s32 GMT_fn_b_ulIsValid(GMT_tdstGameMaterial_* hGameMat);
extern s32 GMT_fn_xTestCollideIdentifierTypeOfGameMaterial(GMT_tdstGameMaterial_* hGameMat,
                                                           u16 wType);
extern s32 GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(GMT_tdstGameMaterial_* hGameMat, u16 xMask);

#endif
