#include "common.h"

#include "GLI/GLI.h"

void inv_GLI_vSetLightShadowCaster(GLI_tdstLight_* light, u8 bShadowCaster, f32 xShadowIntensity)
{
    light->inv_bShadowCaster = bShadowCaster;
    light->inv_xShadowIntensity = xShadowIntensity;
}

void inv_GLI_vSetLightShadowCasterFlag(GLI_tdstLight_* light, u8 bShadowCaster)
{
    light->inv_bShadowCaster = bShadowCaster;
}

u8 inv_GLI_bIsLightShadowCaster(GLI_tdstLight_* light)
{
    return light->inv_bShadowCaster;
}

void inv_GLI_vSetLightShadowIntensity(GLI_tdstLight_* light, f32 xShadowIntensity)
{
    light->inv_xShadowIntensity = xShadowIntensity;
}

f32 inv_GLI_xGetLightShadowIntensity(GLI_tdstLight_* light)
{
    return light->inv_xShadowIntensity;
}
