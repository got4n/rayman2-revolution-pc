#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "GEO/GEO.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

extern volatile f32 D_00392FBC;
extern volatile f32 D_00392FC0;
extern volatile f32 D_00392FC4;
extern volatile f32 D_00392FC8;
extern volatile f32 D_00392FCC;
extern f32 D_00392FD0;
extern volatile f32 D_00392FD4;
extern volatile f32 D_00392FD8;
extern volatile f32 D_00392FDC;
extern volatile f32 D_00392FE0;
extern volatile f32 D_00392FE4;

f32 atanf(f32 value);

char* fn_szGetStringFromTextOrStringParam(tdstGetSetParam_* p_stValue)
{
    u8 type = p_stValue->ucType;
    char* string = NULL;

    if (type == E_vt_Text) {
        string = fn_p_GetTextById(p_stValue->u.lValue);
    } else if (type == E_vt_String) {
        string = p_stValue->u.pvValue;
    }
    return string;
}

void fn_vGetCenterOfBoundingVolume(HIE_tdstSuperObject_* hPerso, MTH3D_tdstVector_* p_stVertex)
{
    MTH3D_tdstVector_* localCenter = hPerso->hCollideBoundingVolume;

    if (localCenter != NULL) {
        POS_fn_vMulMatrixVertex(p_stVertex, hPerso->gu_hGlobalMatrix, localCenter);
        return;
    }
    *p_stVertex = *inv_POS_fn_p_stPosMatrixTranslation(hPerso->gu_hGlobalMatrix);
}

f32 fn_xDistanceBetweenPersoAndWP(HIE_tdstSuperObject_* p_SuperObjPerso, void* hWP)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_* position = inv_POS_fn_p_stPosMatrixTranslation(p_SuperObjPerso->gu_hGlobalMatrix);

    WP_fnv_WayPoint_ComputeLocation(hWP, &delta);
    inv_MTH3D_fn_vSubXYZ(&delta, position, &delta);
    return MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ));
}

f32 fn_xComputeDirectionAngle(MTH3D_tdstVector_* p_stVector)
{
    f32 x;
    f32 y;
    f32 angle;

    x = p_stVector->xX;
    y = p_stVector->xY;
    if (x != 0.0f) {
        angle = atanf(y / x);
    } else if (y < 0.0f) {
        angle = inv_MTH_fn_xNegate(D_00392FBC);
    } else {
        angle = D_00392FC0;
    }
    if (x < 0.0f) {
        angle += D_00392FC4;
    }
    if (angle < 0.0f) {
        angle += D_00392FC8;
    }
    angle += D_00392FCC;
    if (D_00392FD0 < angle) {
        angle -= D_00392FD0;
    }
    return angle;
}

f32 fn_xComputeBestEcartDirectionAngle(f32 xAngleFollowed, f32 xAngleToFollow)
{
    f32 difference = xAngleToFollow - xAngleFollowed;

    if (0.0f < difference) {
        if (D_00392FD4 < difference) {
            difference -= D_00392FD8;
        }
    } else if (difference < -D_00392FDC) {
        difference += D_00392FE0;
    }
    return difference;
}

DNM_stReport_* fn_pstGetDNMReport(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    tdstDynam_* component = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Dynam;

    if (component == NULL) {
        return NULL;
    }
    return component->p_stDynamics->m_tdstDynamicsBaseBlock.m_pstReport;
}

f32 fn_xRealModuloAround(f32 xFrom, f32 xModulus, f32 xValue)
{
    f32 remainder = xValue - xFrom;

    while (remainder < 0.0f) {
        remainder += xModulus;
    }
    while (xModulus <= remainder) {
        remainder -= xModulus;
    }
    return remainder + xFrom;
}

void fn_ucIsThereFloatIfYesConvertItToInteger(tdstGetSetParam_* p_stGetSetParam)
{
    s32 value;

    if (p_stGetSetParam->ucType == E_vt_Float) {
        value = inv_MTH_fn_lToIntegerOrdered(p_stGetSetParam->u.xValue);
        p_stGetSetParam->u.lValue = value;
        p_stGetSetParam->ucType = E_vt_Integer;
    }
}

u8 fn_ucAreThereFloatIfYesConvertThem(tdstGetSetParam_* p_stParam1, tdstGetSetParam_* p_stParam2)
{
    u8 ucIsThereFloat = p_stParam1->ucType == E_vt_Float;

    if (ucIsThereFloat) {
        if (p_stParam2->ucType != E_vt_Float) {
            p_stParam2->u.xValue = p_stParam2->u.lValue;
        }
    } else {
        ucIsThereFloat = p_stParam2->ucType == E_vt_Float;
        if (ucIsThereFloat) {
            p_stParam1->u.xValue = p_stParam1->u.lValue;
        }
    }
    return ucIsThereFloat;
}

#ifdef PC_PORT
s32 fn_ucInternalTurn(HIE_tdstSuperObject_* object, f32 angle)
#else
s32 fn_ucInternalTurn(HIE_tdstSuperObject_* object, f32 xAngle)
#endif
{
    tdstEngineObject_* engineObject = object->hLinkedObject;

    if (engineObject->h_Dynam != NULL) {
#ifdef PC_PORT
        MEC_vTurnAroundZAxis(((tdstDynam_*) engineObject->h_Dynam)->p_stDynamics, angle);
#else
        MEC_vTurnAroundZAxis(engineObject->h_Dynam->p_stDynamics, xAngle);
#endif
        return 1;
    }
    return 0;
}

static inline f32 inv_AI_fn_xPercentToUnit(u8 percent, f32 scale)
{
    f32 value = percent;

    if (value < 0.0f) {
        return 0.0f;
    }
    if (100.0f < value) {
        return 1.0f;
    }
    return value * scale;
}

void fn_vConvertParamToColor(tdstGetSetParam_* p_stParam, GEO_tdstColor_* p_stColor)
{
    f32 fValue = D_00392FE4;
    f32 lValue = 0.0f;
    f32 fScale = p_stParam->u.aucValue[0];

    if (!(fScale < lValue)) {
        if (100.0f < fScale) {
            lValue = 1.0f;
        } else {
            lValue = fScale * fValue;
        }
    }
    p_stColor->xR = lValue;
    p_stColor->xG = inv_AI_fn_xPercentToUnit(p_stParam->u.aucValue[1], fValue);
    p_stColor->xB = inv_AI_fn_xPercentToUnit(p_stParam->u.aucValue[2], fValue);
    p_stColor->xA = inv_AI_fn_xPercentToUnit(p_stParam->u.aucValue[3], fValue);
}

static inline void inv_AI_fn_vConvertFirstColorComponents(GEO_tdstColor_* color, tdstGetSetParam_* param)
{
    s32 red;
    s32 green;
    s32 blue;

    red = inv_MTH_fn_lToIntegerOrdered(color->xR * 100.0f);
    param->u.aucValue[0] = red;
    green = inv_MTH_fn_lToIntegerOrdered(color->xG * 100.0f);
    param->u.aucValue[1] = green;
    blue = inv_MTH_fn_lToIntegerOrdered(color->xB * 100.0f);
    param->u.aucValue[2] = blue;
}

static inline void inv_AI_fn_vConvertLastColorComponent(f32 alpha, tdstGetSetParam_* param, s32 type)
{
    s32 value;

    param->ucType = type;
    value = inv_MTH_fn_lToIntegerOrdered(alpha * 100.0f);
    param->u.aucValue[3] = value;
}

void fn_vConvertColorToParam(GEO_tdstColor_* p_stColor, tdstGetSetParam_* p_stParam)
{
    inv_AI_fn_vConvertFirstColorComponents(p_stColor, p_stParam);
    inv_AI_fn_vConvertLastColorComponent(p_stColor->xA, p_stParam, E_vt_Color);
}
