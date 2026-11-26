#include "common.h"

extern void* inv_GLI_lensFlareReference;

void GLI_vSetLensFlareRef(void* reference)
{
    inv_GLI_lensFlareReference = reference;
}

extern s32 inv_GLI_lNumberOfLensFlare[];

void GLI_vInitLensFlare(void)
{
    inv_GLI_lNumberOfLensFlare[0] = 0;
}
