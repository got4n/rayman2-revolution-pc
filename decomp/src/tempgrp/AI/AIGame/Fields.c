#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"
#include "COL/COL.h"

extern u16 unk_383B80[];
extern u8 unk_383B88[];
extern s16 inv_AI_wPadHorizontalAxis[];
extern s16 inv_AI_wPadVerticalAxis[];
extern f32 inv_AI_xPadAnalogForce[];
extern f32 inv_AI_xPadTrueAnalogForce[];
extern f32 inv_AI_xPadRotationAngle[];
extern s32 inv_AI_lPadSector[];

s32 func_001E6FD8(void)
{
    return 3;
}

s32 func_001E6FE0(void)
{
    return 0;
}

s32 func_001E6FE8(u8 index)
{
    return unk_383B80[index];
}

s32 func_001E7008(void)
{
    return 4;
}

s32 func_001E7010(void)
{
    return 2;
}

s32 func_001E7018(u8 index)
{
    return unk_383B88[index];
}

void func_001E7030(void)
{
}

void fn_ucSetFieldDefaultFunction(void)
{
}

void fn_ucGetFieldPrevComportIntell(HIE_tdstSuperObject_* hSuperObjPerso, tdstGetSetParam_* p_stGetSetParam)
{
    void* behavior = inv_AI_fn_p_stGetMindSO(hSuperObjPerso)->p_stIntelligence->p_stPrevComport;

    p_stGetSetParam->ucType = E_vt_Comport;
    p_stGetSetParam->u.pvValue = behavior;
}

void fn_ucGetFieldPrevComportReflex(HIE_tdstSuperObject_* hSuperObjPerso, tdstGetSetParam_* p_stGetSetParam)
{
    void* behavior = inv_AI_fn_p_stGetMindSO(hSuperObjPerso)->p_stReflex->p_stPrevComport;

    p_stGetSetParam->ucType = E_vt_Comport;
    p_stGetSetParam->u.pvValue = behavior;
}

void fn_ucGetFieldPosition(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    POS_tdstMatrix_* matrix = superObject->gu_hGlobalMatrix;

    out->u.stVector = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    out->ucType = E_vt_Vector;
}

void fn_ucSetFieldOrientation(void)
{
}

#ifdef PC_PORT
void fn_ucGetFieldOrientation(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
#else
s32 fn_ucGetFieldOrientation(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
#endif
{
    f32 x, y, z;
    f32 length;
    f32 inverse;
    s32 type;
    void* matrix;

    matrix = superObject->gu_hGlobalMatrix;
    MTH3D_M_vSetXYZ_inv(&out->u.stVector, 0.0f, -1.0f, 0.0f);
    POS_fn_vApplyMatrixToVector(&out->u.stVector, matrix, &out->u.stVector);
    x = out->u.stVector.xX;
    y = out->u.stVector.xY;
    z = out->u.stVector.xZ;
    do {
    } while (0);
    length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z));
    type = E_vt_Vector;
    if (length != 0.0f) {
        inverse = 1.0f / length;
        MTH3D_M_vSetXYZ_inv(&out->u.stVector, inverse * x, inverse * y, inverse * z);
    }
    out->ucType = type;
#ifndef PC_PORT
    return type;
#endif
}

void fn_ucGetFieldSpeed(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    tdstDynam_* dynam = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;

    out->ucType = E_vt_Vector;
    if (dynam != NULL) {
        out->u.stVector = dynam->p_stDynamics->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
    } else {
        f32 zero = 0.0f;

        MTH3D_M_vSetXYZ_inv(&out->u.stVector, zero, zero, zero);
    }
}

void fn_ucGetFieldNormSpeed(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    tdstDynam_* dynam = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;

    out->ucType = E_vt_Float;
    if (dynam != NULL) {
        MTH3D_tdstVector_* speed = &dynam->p_stDynamics->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;

        out->u.xValue = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(speed->xX, speed->xY, speed->xZ));
        return;
    }
    out->u.lValue = 0;
}

void fn_ucGetFieldPadGlobalVector(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.stVector = qword_383BD0[0].left.stGlobalVector;
    out->ucType = E_vt_Vector;
}

void fn_ucSetFieldPadGlobalVector(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* param)
{
    qword_383BD0[0].left.stGlobalVector = param->u.stVector;
}

void fn_ucGetFieldPadHorizontalAxis(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.lValue = inv_AI_wPadHorizontalAxis[0];
    out->ucType = E_vt_Integer;
}

void fn_ucSetFieldPadHorizontalAxis(HIE_tdstSuperObject_* superObject, s16* value)
{
    inv_AI_wPadHorizontalAxis[0] = *value;
}

void fn_ucGetFieldPadVerticalAxis(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.lValue = inv_AI_wPadVerticalAxis[0];
    out->ucType = E_vt_Integer;
}

void fn_ucSetFieldPadVerticalAxis(HIE_tdstSuperObject_* superObject, s16* value)
{
    inv_AI_wPadVerticalAxis[0] = *value;
}

void fn_ucGetFieldPadAnalogForce(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.xValue = inv_AI_xPadAnalogForce[0];
    out->ucType = E_vt_Float;
}

void fn_ucSetFieldPadAnalogForce(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* param)
{
    inv_AI_xPadAnalogForce[0] = param->u.xValue;
}

void fn_ucGetFieldPadTrueAnalogForce(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.xValue = inv_AI_xPadTrueAnalogForce[0];
    out->ucType = E_vt_Float;
}

void fn_ucSetFieldPadTrueAnalogForce(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* param)
{
    inv_AI_xPadTrueAnalogForce[0] = param->u.xValue;
}

void fn_ucGetFieldPadRotationAngle(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.xValue = inv_AI_xPadRotationAngle[0];
    out->ucType = E_vt_Float;
}

void fn_ucSetFieldPadRotationAngle(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* param)
{
    inv_AI_xPadRotationAngle[0] = param->u.xValue;
}

void fn_ucGetFieldPadSector(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* out)
{
    out->u.lValue = inv_AI_lPadSector[0];
    out->ucType = E_vt_Integer;
}

void fn_ucSetFieldPadSector(HIE_tdstSuperObject_* superObject, tdstGetSetParam_* param)
{
    inv_AI_lPadSector[0] = param->u.lValue;
}

void func_001E7348(HIE_tdstSuperObject_* sender)
{
    MTH3D_tdstVector_ unused0;
    MTH3D_tdstVector_ unused1;
    MTH3D_tdstVector_ min;
    MTH3D_tdstVector_ max;
    HIE_tdstSuperObject_* hit;
    HIE_tdstSuperObject_* sectorObject;
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    SECT_tdstListOfSOSectorsInActivityInteraction_* border;
    SECT_tdstListOfSOCharacters_* node;
    HIE_tdstSuperObject_* character;
    HIE_tdstSuperObject_* sector;
    SECT_tdstSector_* borderSector;
    s32 i;
    f32 zero;

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&unused1, zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&g_stCollideResultPoint[0], zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&g_stCollideResultVector[0], zero, zero, zero);
    g_hSuperObjectHit = NULL;
    hit = NULL;
    sectorObject = SECT_fn_hResearchInWhatSectorIAm(SECT_hFatherSector, g_stCollideParameterPoint);
    MTH3D_M_vSetXYZ_inv(&unused0, zero, zero, zero);
    if (g_ulMode != 1) {
        inv_HIE_ulPickExcludeFlags = g_ulOverRideRayTraceSpoMask;
        if (sectorObject == NULL) {
            HIE_bIntersectSegmentWithFirstSuperObject(
                g_stCollideParameterPoint, g_stCollideParameterVector, SECT_hFatherSector, g_stCollideResultPoint, g_stCollideResultVector, &g_hSuperObjectHit);
        } else {
            HIE_bIntersectSegmentWithFirstSuperObject(
                g_stCollideParameterPoint, g_stCollideParameterVector, sectorObject, g_stCollideResultPoint, g_stCollideResultVector, &g_hSuperObjectHit);
            for (i = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt,
                neighbor = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
                 i > 0; i--, neighbor++) {
                if (neighbor->cDisplayMode == 2) {
                    break;
                }
                sector = neighbor->hPointerOfSectorSO;
                if (!SECT_fn_bIsThisSectorVirtual(sector)) {
                    HIE_bIntersectSegmentWithFirstSuperObject(g_stCollideParameterPoint, g_stCollideParameterVector,
                                                                     sector, g_stCollideResultPoint,
                                                                     g_stCollideResultVector, &g_hSuperObjectHit);
                }
            }
        }
        if (g_ulMode == 0) {
            return;
        }
    }
    inv_HIE_ulPickExcludeFlags = 0;
    for (i = 0, node = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfCharacters.hFirstElementDyn; node != NULL;
         node = node->hNextBrotherDyn, i++) {
        character = node->hPointerOfCharacter;
        if (character != sender) {
            if ((fn_ulStandardGameGetCustomBitsSO(character) & 0x10) &&
                ((inv_HIE_fn_p_stGetEngineObject(character)->h_StandardGame->ucMiscFlags >> 2) & 1)) {
                if (HIE_bIntersectSegmentWithFirstSuperObject(
                        g_stCollideParameterPoint, g_stCollideParameterVector, character, g_stCollideResultPoint, g_stCollideResultVector, &hit)) {
                    while (hit != NULL && hit->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
                        hit = hit->hFatherSta;
                    }
                    if (hit != NULL) {
                        g_hSuperObjectHit = hit;
                    }
                }
            }
        }
    }
    for (i = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt, border = inv_HIE_fn_p_stGetSector(sectorObject)->stListOfSectorsInActivityInteraction.hFirstElementOpt; i > 0;
         i--, border++) {
        sector = border->hPointerOfSectorSO;
        borderSector = sector->hLinkedObject;
        SECT_fn_vGetMinPointInBorder(borderSector, &min);
        SECT_fn_vGetMaxPointInBorder(borderSector, &max);
        if (INT_fn_bDetectIntersectSegmentWithBox(g_stCollideParameterPoint, g_stCollideParameterVector, &min, &max, NULL)) {
            for (node = inv_HIE_fn_p_stGetSector(sector)->stListOfCharacters.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
                character = node->hPointerOfCharacter;
                if (character != sender) {
                    if ((fn_ulStandardGameGetCustomBitsSO(character) & 0x10) &&
                        ((inv_HIE_fn_p_stGetEngineObject(character)->h_StandardGame->ucMiscFlags >> 2) & 1)) {
                        if (HIE_bIntersectSegmentWithFirstSuperObject(
                                g_stCollideParameterPoint, g_stCollideParameterVector, character, g_stCollideResultPoint, g_stCollideResultVector,
                                &hit)) {
                            while (hit != NULL && hit->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
                                hit = hit->hFatherSta;
                            }
                            if (hit != NULL) {
                                g_hSuperObjectHit = hit;
                            }
                        }
                    }
                }
            }
        }
    }
}
