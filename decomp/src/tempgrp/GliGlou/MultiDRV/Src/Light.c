#include "common.h"

#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

#define GLI_MAX_VIEWPORT_LIGHTS 0x20
#define GLI_LIGHT_HOTSPOT 5
#define GLI_LIGHT_TYPE_4 4

#define LIGHT_SWITCH_OBJECT_LIGHTED 0x1
#define LIGHT_SWITCH_SECTOR 0x2

s32 gs_CurrentNumberOfLightInViewport;

f32 tanf(f32 angle);
void inv_GLI_vUpdateHotSpotLightBoundingSphere(GLI_tdstLight_* light);

void GLI_vResetTableOfActiveLightsForChar(s32 _cValue)
{
    memset(g_ucTableOfActiveLightsForChar, (s8) _cValue, 0x20);
}

void inv_GLI_vSetActiveLightForChar(s32 index, u8 value)
{
    g_ucTableOfActiveLightsForChar[index] = value;
}

static inline s32 inv_GLI_fn_bIsInLightRange(GLI_tdstLight_* light, MTH3D_tdstVector_* center, f32 radius,
                                 f32 reach)
{
    MTH3D_tdstVector_ delta;
    MTH4D_tdstVector_* position = &light->stMatrix.stMatrix.stCol_3;

    MTH3D_M_vSubXYZ_inv(&delta, center, position);
    return MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ) <
           (radius + reach) * (radius + reach);
}

s32 GLI_bDoesLightAffectBV(GLI_tdstLight_* light, MTH3D_tdstVector_* center, f32 radius)
{
    switch (light->lTypeOfLight) {
    case 1:
    case GLI_LIGHT_TYPE_4:
        return 1;
    case 2:
    case 3:
        return inv_GLI_fn_bIsInLightRange(light, center, radius, light->xFar);
    case GLI_LIGHT_HOTSPOT:
        return inv_GLI_fn_bIsInLightRange(light, center, radius, light->xRadius);
    }
    return 1;
}

void GLI_vAddLightToViewport(void* viewport, GLI_tdstLight_* light)
{
    if (gs_CurrentNumberOfLightInViewport != GLI_MAX_VIEWPORT_LIGHTS) {
        gs_aDEF_p_CurrentListOfLight[gs_CurrentNumberOfLightInViewport++] = light;
    }
}

void GLI_vPopLightsInViewport(void* lUnused, s32 lNbLightsToPop)
{
    gs_CurrentNumberOfLightInViewport -= lNbLightsToPop;
    if (gs_CurrentNumberOfLightInViewport < 0) {
        gs_CurrentNumberOfLightInViewport = 0;
    }
}

void GLI_vClearListOfLightInViewport(GLD_tdstViewportAttributes_* _p_stVpt)
{
    gs_CurrentNumberOfLightInViewport = 0;
}

void GLI_xSetViewportLightsOnOff(u8 bOnOff)
{
    s32 i;

    for (i = 0; i < gs_CurrentNumberOfLightInViewport; i++) {
        gs_aDEF_p_CurrentListOfLight[i]->bValid = bOnOff;
    }
}

void GLI_xCreateLight(GLI_tdstLight_** h_stLight, s32 lTypeOfLight)
{
    f32 value;

    if ((*h_stLight = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(GLI_tdstLight_))) != NULL) {
        (*h_stLight)->lTypeOfLight = lTypeOfLight;
        gu_MTH4D_M_vSetIdentityMatrix(&(*h_stLight)->stMatrix);
        value = 0.8f;
        (*h_stLight)->b_lOnOff = 1;
        (*h_stLight)->stColor.xR = value;
        (*h_stLight)->stColor.xG = value;
        (*h_stLight)->stColor.xB = value;
        (*h_stLight)->stColor.xA = value;
        (*h_stLight)->ucIsAlpha = 2;
        (*h_stLight)->bValid = 1;
        (*h_stLight)->bIsPainting = 0;
        GLI_vSetLightSwitch(*h_stLight, 0);
        (*h_stLight)->stBackgroundColor.xX = 0;
        (*h_stLight)->stBackgroundColor.xY = 0;
        (*h_stLight)->stBackgroundColor.xZ = 0;
        (*h_stLight)->stBackgroundColor.xW = 0;
        (*h_stLight)->backendLight = NULL;
        (*h_stLight)->inv_bShadowCaster = 0;
        (*h_stLight)->inv_xShadowIntensity = 0;
    }
}

void GLI_vCopyLight(GLI_tdstLight_* p_stLightDst, const GLI_tdstLight_* p_stLightSrc)
{
    *p_stLightDst = *p_stLightSrc;
}

void inv_GLI_vUpdateHotSpotLightBoundingSphere(GLI_tdstLight_* light)
{
    f32 delta[3];
    f32 sumX;
    f32 sumY;
    f32 sumZ;
    f32 distance;

    if (light == NULL) {
        return;
    }
    if (light->lTypeOfLight != GLI_LIGHT_HOTSPOT) {
        return;
    }

    sumX = light->xExter_Max_Pos.xX + light->xExter_Min_Pos.xX;
    sumY = light->xExter_Max_Pos.xY + light->xExter_Min_Pos.xY;
    sumZ = light->xExter_Max_Pos.xZ + light->xExter_Min_Pos.xZ;
    delta[0] = light->xExter_Max_Pos.xX - light->xExter_Min_Pos.xX;
    delta[1] = light->xExter_Max_Pos.xY - light->xExter_Min_Pos.xY;
    delta[2] = light->xExter_Max_Pos.xZ - light->xExter_Min_Pos.xZ;

    distance = MTH_fn_xSqrAddSqrAddSqrOpt2(delta[0], delta[1], delta[2]);
    distance = MTH_fn_xSqrtOpt2(distance);

    MTH3D_M_vSetXYZ_inv(&light->xCenterBox, sumX * 0.5f, sumY * 0.5f, sumZ * 0.5f);
    light->xRadius = distance * 0.5f;
}

void GLI_xSetLightState(GLI_tdstLight_* p_stLight, s32 b_lOnOff)
{
    p_stLight->b_lOnOff = b_lOnOff != 0 ? 1 : 0;
}

void GLI_xGetLightState(const GLI_tdstLight_* p_stLight, s32* pb_lOnOff)
{
    *pb_lOnOff = p_stLight->b_lOnOff != 0;
}

void GLI_xSetLightColor(GLI_tdstLight_* p_stLight, GEO_tdstColor_* p_stColor)
{
    p_stLight->stColor = *p_stColor;
    RW3_Light_SetColor(p_stLight, p_stColor);
}

void GLI_xGetLightColor(const GLI_tdstLight_* p_stLight, GEO_tdstColor_* p_stColor)
{
    *p_stColor = p_stLight->stColor;
}

void GLI_xSetLightMatrix(GLI_tdstLight_* p_stLight, POS_tdstMatrix_* p_stMatrix)
{
    p_stLight->stMatrix = *p_stMatrix;
    RW3_Light_UpdateMatrixFromGLILightMatrix(p_stLight);
}

void GLI_vSetLightNearFar(GLI_tdstLight_* p_stLight, f32 xNear, f32 xFar)
{
    p_stLight->xNear = xNear;
    p_stLight->xFar = xFar;
    if (p_stLight->lTypeOfLight != GLI_LIGHT_TYPE_4) {
        RW3_Light_SetRadius(p_stLight);
    }
}

void GLI_vGetLightNearFar(GLI_tdstLight_* p_stLight, f32* p_xNear, f32* p_xFar)
{
    *p_xNear = p_stLight->xNear;
    *p_xFar = p_stLight->xFar;
}

void GLI_vSetLightAlphas(GLI_tdstLight_* p_stLight, f32 xLittleAlpha, f32 xBigAlpha)
{
    p_stLight->xLittleAlpha = xLittleAlpha;
    p_stLight->xBigAlpha = xBigAlpha;
    p_stLight->xLittleTangent = tanf(xLittleAlpha * 0.5f);
    p_stLight->xBigTangent = tanf(xBigAlpha * 0.5f);
    RW3_Light_SetAngle(p_stLight);
}

void GLI_vGetLightAlphas(GLI_tdstLight_* p_stLight, f32* p_xLittleAlpha, f32* p_xBigAlpha)
{
    *p_xLittleAlpha = p_stLight->xLittleAlpha;
    *p_xBigAlpha = p_stLight->xBigAlpha;
}

u16 GLI_lGetLightType(GLI_tdstLight_* p_stLight)
{
    return p_stLight->lTypeOfLight;
}

void GLI_vSetLightType(GLI_tdstLight_* p_stLight, u16 lNewType)
{
    p_stLight->lTypeOfLight = lNewType;
}

void inv_GLI_vSetLightPainting(GLI_tdstLight_* light)
{
    light->bIsPainting = 1;
}

void inv_GLI_vSetLightIsAlpha(GLI_tdstLight_* light, u8 value)
{
    light->ucIsAlpha = value;
}

void inv_GLI_vSetLightAlphaOn(GLI_tdstLight_* light)
{
    light->ucIsAlpha = 1;
}

void inv_GLI_xSetLightBackgroundColor(GLI_tdstLight_* light, const MTH4D_tdstVector_* color)
{
    light->stBackgroundColor = *color;
}

void GLI_xGetLightBackgroundColor(const GLI_tdstLight_* light, MTH4D_tdstVector_* color)
{
    *color = light->stBackgroundColor;
}

void inv_GLI_vSetLightIntensityMinMax(GLI_tdstLight_* light, f32 valueC8, f32 valueCC)
{
    light->xIntensity_Min = valueC8;
    light->xIntensity_Max = valueCC;
}

void inv_GLI_vSetLightInterMinPos(GLI_tdstLight_* light, const MTH3D_tdstVector_* source)
{
    MTH3D_tdstVector_ value = *source;

    light->xInter_Min_Pos = value;
    inv_GLI_vUpdateHotSpotLightBoundingSphere(light);
}

void inv_GLI_vSetLightInterMaxPos(GLI_tdstLight_* light, const MTH3D_tdstVector_* source)
{
    MTH3D_tdstVector_ value = *source;

    light->xInter_Max_Pos = value;
    inv_GLI_vUpdateHotSpotLightBoundingSphere(light);
}

void inv_GLI_vSetLightExterMinPos(GLI_tdstLight_* light, const MTH3D_tdstVector_* source)
{
    MTH3D_tdstVector_ value = *source;

    light->xExter_Min_Pos = value;
    inv_GLI_vUpdateHotSpotLightBoundingSphere(light);
}

void inv_GLI_vSetLightExterMaxPos(GLI_tdstLight_* light, const MTH3D_tdstVector_* source)
{
    MTH3D_tdstVector_ value = *source;

    light->xExter_Max_Pos = value;
    inv_GLI_vUpdateHotSpotLightBoundingSphere(light);
}

MTH3D_tdstVector_* inv_GLI_p_stGetLightInterMinPos(GLI_tdstLight_* light)
{
    return &light->xInter_Min_Pos;
}

MTH3D_tdstVector_* inv_GLI_p_stGetLightInterMaxPos(GLI_tdstLight_* light)
{
    return &light->xInter_Max_Pos;
}

MTH3D_tdstVector_* inv_GLI_p_stGetLightExterMinPos(GLI_tdstLight_* light)
{
    return &light->xExter_Min_Pos;
}

MTH3D_tdstVector_* inv_GLI_p_stGetLightExterMaxPos(GLI_tdstLight_* light)
{
    return &light->xExter_Max_Pos;
}

f32* inv_GLI_p_xGetLightIntensityMin(GLI_tdstLight_* light)
{
    return &light->xIntensity_Min;
}

f32* inv_GLI_p_xGetLightIntensityMax(GLI_tdstLight_* light)
{
    return &light->xIntensity_Max;
}

u8* inv_GLI_p_ucGetLightOnOff(GLI_tdstLight_* light)
{
    return &light->b_lOnOff;
}

u16* inv_GLI_p_uwGetLightType(GLI_tdstLight_* light)
{
    return &light->lTypeOfLight;
}

f32* inv_GLI_p_xGetLightFar(GLI_tdstLight_* light)
{
    return &light->xFar;
}

f32* inv_GLI_p_xGetLightNear(GLI_tdstLight_* light)
{
    return &light->xNear;
}

f32* inv_GLI_p_xGetLightLittleAlpha(GLI_tdstLight_* light)
{
    return &light->xLittleAlpha;
}

f32* inv_GLI_p_xGetLightBigAlpha(GLI_tdstLight_* light)
{
    return &light->xBigAlpha;
}

GEO_tdstColor_* inv_GLI_p_stGetLightColor(GLI_tdstLight_* light)
{
    return &light->stColor;
}

MTH4D_tdstVector_* inv_GLI_p_stGetLightBackgroundColor(GLI_tdstLight_* light)
{
    return &light->stBackgroundColor;
}

POS_tdstMatrix_* inv_GLI_p_stGetLightMatrix(GLI_tdstLight_* light)
{
    return &light->stMatrix;
}

u32 GLI_ucGetObjectLighted(GLI_tdstLight_* p_stLight)
{
    return p_stLight->ucObjectLighted;
}

void GLI_vSetObjectLighted(GLI_tdstLight_* light, u8 enabled)
{
    u8 flags = light->ucObjectLighted;

    if (enabled != 0) {
        light->ucObjectLighted = flags | LIGHT_SWITCH_OBJECT_LIGHTED;
    } else {
        light->ucObjectLighted = flags & ~LIGHT_SWITCH_OBJECT_LIGHTED;
    }
}

void inv_GLI_vSetLightSectorSwitch(GLI_tdstLight_* light, u8 enabled)
{
    u8 flags = light->ucObjectLighted;

    if (enabled != 0) {
        light->ucObjectLighted = flags | LIGHT_SWITCH_SECTOR;
    } else {
        light->ucObjectLighted = flags & ~LIGHT_SWITCH_SECTOR;
    }
}

void GLI_vSetLightSwitch(GLI_tdstLight_* light, u8 value)
{
    light->ucObjectLighted = value;
}

f32 GLI_fn_xFogGetNear(GLI_tdstLight_* p_stLight)
{
    return p_stLight->xNear;
}

f32 GLI_fn_xFogGetFar(GLI_tdstLight_* p_stLight)
{
    return p_stLight->xFar;
}

f32 GLI_fn_xFogGetInfinite(GLI_tdstLight_* p_stLight)
{
    return p_stLight->xLittleAlpha;
}

f32 GLI_fn_xFogGetBlendNear(GLI_tdstLight_* p_stLight)
{
    return p_stLight->xBigAlpha;
}

f32 GLI_fn_xFogGetBlendFar(GLI_tdstLight_* p_stLight)
{
    return p_stLight->xIntensity_Min;
}
