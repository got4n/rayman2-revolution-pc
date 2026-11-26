#include "common.h"

#include "GMT/GMT.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/types.h"

#include "GAM/GAM.h"

#define CONTACT_TAG_IGNORED 0x63
#define DYNAMICS_FLAG_COLLISION_BIT 22
#define DYNAMICS_FLAG_IN_COLLISION 0x10000
#define MATRIX_INIT_ORTHONORMAL 5

f32 tanf(f32 value);
s32 fn_ucCollSetGetCollComputeFrequencySO(void* context);
void MEC_fn_vCollideBaseMechanic(DNM_stDynamics* dynamics, void* context, s32 count, DNM_stMecObstacle* contacts,
                   f32 deltaTime);
void POS_fn_vGetRotationVector(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* axis);

static inline s32 inv_MEC_fn_lBothMaterialsSpecial(DNM_stMecObstacle* contact)
{
    s32 special = 0;

    if ((contact->contact.hCollidedMaterial->collideMaterial.xIdentifier & 0x4000) &&
        (contact->contact.hMyMaterial->collideMaterial.xIdentifier & 0x4000))
        special = 1;
    return special;
}

void MEC_fn_vCollideBaseMechanic(DNM_stDynamics* p_stDynamic, void* hSupObj, s32 lObstacle, DNM_stMecObstacle* a_stObstacle,
                   f32 xDT)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ response;
    MTH3D_tdstVector_ responseUnit;
    MTH3D_tdstVector_ deltaUnit;
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ reboundVec;
    MTH3D_tdstVector_ slideVec;
    MTH3D_tdstVector_ correction;
    MTH3D_tdstVector_ posCorrected;
    MTH3D_tdstVector_ posDelta;
    MTH3D_tdstVector_ adjust;
    MTH3D_tdstVector_ normalSum;
    MTH3D_tdstVector_ up;
    POS_tdstMatrix_* candidateMatrix;
    POS_tdstMatrix_* referenceMatrix;
    POS_tdstMatrix_* matrix280;
    MTH3D_tdstVector_* previousPos;
    MTH3D_tdstVector_* currentPos;
    s32 correctionFlag;
    DNM_stMecObstacle* first;
    DNM_stMecObstacle* candidate;
    DNM_stMecObstacle* contact;
    MTH4D_tdstVector_* posHot;
    s32 firstIndex;
    s32 candidateIndex;
    f32 scale;
    s32 allType78;
    s32 pick;
    s32 steepOrGround;
    f32 slide;
    f32 friction;
    s32 bothSpecial;
    s32 withinSlope;
    s32 specialIndex;
    s32 i;
    s32 contactType;
    f32 rebound;
    f32 deltaLen;
    f32 responseLen;
    f32 correctionLen;
    f32 proj;
    MTH3D_tdstVector_* normal;
    f32 originalCorrectionLen;
    f32 xInvLength;
    f32 len;
    s32 isNull;

    referenceMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    candidateMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    currentPos = inv_POS_fn_p_stPosMatrixTranslation(candidateMatrix);
    previousPos = inv_POS_fn_p_stPosMatrixTranslation(referenceMatrix);
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4)
        matrix280 = &p_stDynamic->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix;

    {
        f32 zero = 0.0f;
        correctionFlag = 0;
        firstIndex = 0;
        candidateIndex = 0;
        first = 0;
        candidate = 0;
        specialIndex = 0;
        MTH3D_M_vSetXYZ_inv(&normalSum, zero, zero, zero);
    }
    g_lCallCounter++;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x40) {
        for (i = 0; i < lObstacle; i++) {
            s32 contactType = a_stObstacle[i].m_xCollidedEntity;
            if ((contactType ^ 0x63) != 0) {
                if (((u8) MEC_fn_bGroundNormal(p_stDynamic, &a_stObstacle[i].contact.m_stNorm)))
                    a_stObstacle[i].m_xCollidedEntity = 0x63;
            }
        }
    } else {
        for (i = 0; i < lObstacle && candidate == 0; i++) {
            contact = &a_stObstacle[i];
            contactType = contact->m_xCollidedEntity;
            if ((contactType ^ 0x63) != 0) {
                bothSpecial = inv_MEC_fn_lBothMaterialsSpecial(contact);
                if (!bothSpecial &&
                    ((u8) MEC_fn_bGroundNormal(p_stDynamic, &contact->contact.m_stNorm))) {
                    candidate = contact;
                    candidateIndex = i;
                }
            }
        }
    }

    if (candidate != 0 && (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100000) &&
        (((g_stReport.m_ulCurrSurfaceState >> 6) & 1) == 0)) {
        MEC_fn_vGroundObstacleManagement(p_stDynamic, hSupObj, xDT);
        g_stReport.m_ulCurrSurfaceState &= 0xFFFFFFBFu;
        return;
    }

    allType78 = 1;
    for (i = 0; i < lObstacle && first == 0; i++) {
        contact = &a_stObstacle[i];
        contactType = contact->m_xCollidedEntity;
        if ((contactType ^ 0x63) != 0) {
            first = contact;
            firstIndex = i;
        }
    }

    for (i = 0; i < lObstacle; i++) {
        {
            contact = &a_stObstacle[i];
            contactType = contact->m_xCollidedEntity;
            if ((contactType ^ 0x63) != 0) {
                bothSpecial = inv_MEC_fn_lBothMaterialsSpecial(contact);
                pick = specialIndex != 0 ? specialIndex : i;
                if (bothSpecial)
                    specialIndex = pick;
                if ((u16) (contact->m_xCollidedEntity - 7) >= 2)
                    allType78 = 0;
            }
        }
    }

    if (first == 0)
        return;

    if (candidate != 0 && firstIndex < candidateIndex)
        firstIndex = candidateIndex;
    if (firstIndex < specialIndex)
        firstIndex = specialIndex;

    MEC_fn_vReportCollision(p_stDynamic, hSupObj, lObstacle, a_stObstacle, firstIndex);

    if (!allType78 || (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x40)) {
        normal = &first->contact.m_stNorm;
    } else {
        normal = &normalSum;
        for (i = 0; i < lObstacle; i++) {
            s32 ct;
            contact = &a_stObstacle[i];
            ct = contact->m_xCollidedEntity;
            if ((ct ^ 0x63) != 0) {
                MTH3D_M_vAddXYZ_inv(normal, normal, &contact->contact.m_stNorm);
            }
        }
        if (!MTH3D_M_bIsZeroXYZ_inv(normal)) {
            inv_MTH3D_fn_vNormalizeXYZ(normal);
        } else {
            normal = &first->contact.m_stNorm;
        }
    }

    slide = MEC_fn_xGetSlide(p_stDynamic, first);
    rebound = MEC_fn_xGetRebound(p_stDynamic, first);

    {
        f32 zero = 0.0f;
        MTH3D_M_vSetXYZ_inv(&responseUnit, zero, zero, zero);
        MTH3D_M_vSetXYZ_inv(&adjust, zero, zero, zero);
        up.xX = zero;
        up.xY = zero;
    }
    up.xZ = 1.0f;

    proj = inv_MTH_fn_xMulAdd3Grouped(normal->xX, normal->xY, normal->xZ, up.xX, up.xY, up.xZ);
    withinSlope = (inv_MTH_fn_xNegate(p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope) <= proj &&
                   proj <= p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope);
    steepOrGround = p_stDynamic->m_tdstDynamicsBaseBlock.m_xCosSlope <= proj;

    inv_MTH3D_fn_vSubXYZ(&delta, currentPos, previousPos);
    {
        f32 sq;
        sq = MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ);
        deltaLen = MTH_fn_xSqrtOpt2(sq);
    }

    {
        f32 xDot;
        xDot = inv_MTH_fn_xMulAdd3Grouped(delta.xX, delta.xY, delta.xZ, normal->xX, normal->xY,
                                          normal->xZ);
        proj = -xDot;
    }

    inv_MTH3D_fn_vKTimesXYZ(&correction, proj, normal);

    inv_MTH3D_fn_vAddXYZ(&slideVec, &delta, &correction);

    inv_MTH3D_fn_vAddXYZ(&reboundVec, &slideVec, &correction);

    inv_MTH3D_fn_vKTimesXYZ(&correction, rebound, &reboundVec);
    MTH3D_M_vSetXYZ_inv(&response, slide * slideVec.xX + correction.xX,
                        slide * slideVec.xY + correction.xY, slide * slideVec.xZ + correction.xZ);

    {
        f32 sq;
        sq = MTH_fn_xSqrAddSqrAddSqrOpt2(response.xX, response.xY, response.xZ);
        responseLen = MTH_fn_xSqrtOpt2(sq);
    }
    {
        s32 responseIsNull = 0;
        if (inv_MTH_fn_xAbsolute(response.xX - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsolute(response.xY - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsolute(response.xZ - 0.0f) < 0.001f)
            responseIsNull = 1;
        if (!responseIsNull) {
            xInvLength = 1.0f / responseLen;
            inv_MTH3D_fn_vKTimesXYZ(&responseUnit, xInvLength, &response);
        }
    }

    {
        s32 deltaIsNull = 0;
        if (inv_MTH_fn_xAbsolute(delta.xX - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsolute(delta.xY - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsolute(delta.xZ - 0.0f) < 0.001f)
            deltaIsNull = 1;
        if (!deltaIsNull) {
            xInvLength = 1.0f / deltaLen;
            inv_MTH3D_fn_vKTimesXYZ(&deltaUnit, xInvLength, &delta);
        }
    }

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x100) && withinSlope &&
        response.xZ != 0.0f) {
        xInvLength = delta.xZ;
        if (response.xZ != 0.0f) {
            xInvLength /= response.xZ;
            if (-1.0f < xInvLength) {
                if (xInvLength <= 1.0f) {
                    MTH3D_M_vSetXYZ_inv(&response, xInvLength * response.xX,
                                        xInvLength * response.xY, xInvLength * response.xZ);
                } else {
                    MTH3D_tdstVector_ cross1;
                    MTH3D_tdstVector_ cross2;
                    MTH3D_tdstVector_ normalCopy;
                    MTH3D_tdstVector_ tmpComponent;
                    MTH3D_tdstVector_ crossTemp;
                    normalCopy = *normal;
                    if (&cross1 == &up || &cross1 == &normalCopy) {
                        DNM_p_stVectorCrossProduct(&crossTemp, &up, &normalCopy);
                        cross1 = crossTemp;
                    } else {
                        DNM_p_stVectorCrossProduct(&cross1, &up, &normalCopy);
                    }
                    if (&cross2 == &normalCopy || &cross2 == &cross1) {
                        DNM_p_stVectorCrossProduct(&crossTemp, &normalCopy, &cross1);
                        cross2 = crossTemp;
                    } else {
                        DNM_p_stVectorCrossProduct(&cross2, &normalCopy, &cross1);
                    }

                    {
                        f32 d1, d3, r;
                        r = delta.xZ / cross2.xZ;
                        d1 = inv_MTH_fn_xMulAdd3Grouped(response.xX, response.xY, response.xZ,
                                              cross1.xX, cross1.xY, cross1.xZ);
                        d3 = inv_MTH_fn_xMulAdd3Grouped(response.xX, response.xY, response.xZ,
                                              normalCopy.xX, normalCopy.xY, normalCopy.xZ);

                        inv_MTH3D_fn_vKTimesXYZ(&tmpComponent, d3, &normalCopy);

                        MTH3D_M_vSetXYZ_inv(&response,
                                            d1 * cross1.xX + r * cross2.xX + tmpComponent.xX,
                                            d1 * cross1.xY + r * cross2.xY + tmpComponent.xY,
                                            d1 * cross1.xZ + r * cross2.xZ + tmpComponent.xZ);
                    }
                }
            }
        }
    }

    friction = first->contact.m_xRate;
    MEC_fn_vComputeAdjustVector(&adjust, p_stDynamic, lObstacle, a_stObstacle, firstIndex);

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x400000)
        adjust.xZ = 0.0f;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100000) {
        if ((u16) (first->m_xCollidedEntity - 7) < 2) {
            s32 slideNegative = first->contact.m_xRate < 0.0f;
            if (slideNegative) {
                f32 xDot, nx, ux, ny, uy, nz, uz;
                nx = normal->xX;
                ux = deltaUnit.xX;
                ny = normal->xY;
                uy = deltaUnit.xY;
                nz = normal->xZ;
                uz = deltaUnit.xZ;
                xDot = inv_MTH_fn_xMulAdd3Grouped(nx, ny, nz, ux, uy, uz);
                if (xDot < -0.42f) {
                    pos = *previousPos;
                    goto have_pos;
                }
            }
        }
    }
    inv_MTH3D_fn_vAddXYZ(&pos, currentPos, &adjust);
have_pos:;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100000) &&
        (g_xZWater - 1.2f < pos.xZ)) {
        correctionFlag = 1;
        pos.xZ = previousPos->xZ;
    }

    if (friction >= 0.0f)
        scale = 1.0f - friction;
    else
        scale = 0.0f;

    inv_MTH3D_fn_vKTimesXYZ(&correction, scale, &response);
    correctionLen = MTH_fn_xSqrAddSqrAddSqrOpt2(correction.xX, correction.xY, correction.xZ);
    correctionLen = MTH_fn_xSqrtOpt2(correctionLen);

    if (first->m_xZoneRadius < correctionLen) {
        f32 k = first->m_xZoneRadius / correctionLen;
        inv_MTH3D_fn_vKTimesXYZ(&correction, k, &correction);
    }

    if (correctionFlag) {
        {
            f32 sq;
            sq = MTH_fn_xSqrAddSqrAddSqrOpt2(correction.xX, correction.xY, correction.xZ);
            originalCorrectionLen = MTH_fn_xSqrtOpt2(sq);
        }
        correction.xZ = 0.0f;
        {
            isNull = 0;
            if (inv_MTH_fn_xAbsolute(correction.xX - correction.xZ) < 0.001f &&
                inv_MTH_fn_xAbsolute(correction.xY - correction.xZ) < 0.001f &&
                inv_MTH_fn_xAbsolute(correction.xZ) < 0.001f)
                isNull = 1;
            if (!isNull) {
                f32 len;
                inv_MTH3D_fn_vNormalizeXYZ(&correction);
                inv_MTH3D_fn_vKTimesXYZ(&correction, originalCorrectionLen, &correction);
            }
        }
    }

    inv_MTH3D_fn_vAddXYZ(&posCorrected, &pos, &correction);

    inv_MTH3D_fn_vSubXYZ(&posDelta, &posCorrected, previousPos);

    proj = inv_MTH_fn_xMulAdd3Grouped(normal->xX, normal->xY, normal->xZ,
                            posDelta.xX, posDelta.xY, posDelta.xZ);
    posHot = &candidateMatrix->stMatrix.stCol_3;

    if (((0.001f < proj && steepOrGround && 0.0f < rebound) ||
         (candidate != 0 &&
          ((u8) MEC_fn_bGroundNormal(p_stDynamic, &first->contact.m_stNorm)) == 0)) &&
        !(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x40)) {
        if (candidateMatrix->ucType == MAT_eTransform_Identity) {
            candidateMatrix->ucType = MAT_eTransform_Translation;
            __asm__("");
            __asm__("");
            __asm__("");
        }
        *inv_POS_fn_p_stPosMatrixTranslation(candidateMatrix) = pos;
        posHot->xW = 1.0f;
    } else {
        if (candidateMatrix->ucType == MAT_eTransform_Identity)
            candidateMatrix->ucType = MAT_eTransform_Translation;
        *inv_POS_fn_p_stPosMatrixTranslation(candidateMatrix) = posCorrected;
        posHot->xW = 1.0f;
    }

    {
        MTH3D_tdstVector_ v;
        posHot = &referenceMatrix->stMatrix.stCol_3;
        inv_MTH3D_fn_vSubXYZ(&v, currentPos, &response);
        if (referenceMatrix->ucType == MAT_eTransform_Identity)
            referenceMatrix->ucType = MAT_eTransform_Translation;
        *inv_POS_fn_p_stPosMatrixTranslation(referenceMatrix) = v;
    }
    posHot->xW = 1.0f;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
        if (matrix280->ucType == MAT_eTransform_Identity)
            matrix280->ucType = MAT_eTransform_Translation;
        *inv_POS_fn_p_stPosMatrixTranslation(matrix280) = *previousPos;
        matrix280->stMatrix.stCol_3.xW = 1.0f;
    }

    MEC_fn_vComputeSpeed(p_stDynamic, xDT);
}

static inline s32 inv_MEC_fn_bIsNullMove(MTH3D_tdstVector_* move)
{
    s32 isNull = 0;

    if (inv_MTH_fn_xAbsoluteTied(move->xX - 0.0f) < 0.001f &&
        inv_MTH_fn_xAbsoluteTied(move->xY - 0.0f) < 0.001f &&
        inv_MTH_fn_xAbsoluteTied(move->xZ - 0.0f) < 0.001f) {
        isNull = 1;
    }
    return isNull;
}

void MEC_fn_vWallObstacleManagement(MTH3D_tdstVector_* pstResultMove, DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* pstMove, MTH3D_tdstVector_* pstGroundNormal, MTH3D_tdstVector_* pstWallNormal, f32 xDT, u8 bAngleLimit)
{
    f32 norm;
    f32 dotMove;
    f32 factor;
    POS_tdstMatrix_* currentMatrix;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ offset;
    MTH3D_tdstVector_ moveDirection;

    if (inv_MEC_fn_bIsNullMove(pstMove)) {
        pstResultMove->xZ = pstResultMove->xY = pstResultMove->xX = 0.0f;
        return;
    }
    currentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    DNM_p_stVectorCrossProduct(&direction, pstGroundNormal, pstWallNormal);
    if (inv_MEC_fn_bIsNullMove(&direction)) {
        pstResultMove->xZ = pstResultMove->xY = pstResultMove->xX = 0.0f;
        return;
    }

    {
        f32 squared;
        f32 length;

        squared = MTH_fn_xSqrAddSqrAddSqrOpt2(direction.xX, direction.xY, direction.xZ);
        length = MTH_fn_xSqrtOpt2(squared);
        if (length != 0.0f) {
            f32 inverse = 1.0f / length;

            inv_MTH3D_fn_vKTimesXYZ(&direction, inverse, &direction);
        }
    }

    dotMove = inv_MTH_fn_xMulAdd3Grouped(direction.xX, direction.xY, direction.xZ,
                               pstMove->xX, pstMove->xY, pstMove->xZ);

    if (bAngleLimit && g_lCallCounter == 1) {
        if (!inv_MEC_fn_bIsNullMove(pstMove)) {
            f32 squared;
            f32 length;

            squared = MTH_fn_xSqrAddSqrAddSqrOpt2(pstMove->xX, pstMove->xY, pstMove->xZ);
            length = MTH_fn_xSqrtOpt2(squared);
            if (length != 0.0f) {
                f32 inverse = 1.0f / length;

                inv_MTH3D_fn_vKTimesXYZ(&moveDirection, inverse, pstMove);
            } else {
                moveDirection = *pstMove;
            }
        } else {
            moveDirection.xZ = moveDirection.xY = moveDirection.xX = 0.0f;
        }
        {
            f32 squared;
            f32 moveLength;
            f32 dot;
            f32 facing;

            squared = MTH_fn_xSqrAddSqrAddSqrOpt2(pstMove->xX, pstMove->xY, pstMove->xZ);
            dot = inv_MTH_fn_xMulAdd3Grouped(moveDirection.xX, moveDirection.xY, moveDirection.xZ,
                                   pstWallNormal->xX, pstWallNormal->xY, pstWallNormal->xZ);
            moveLength = MTH_fn_xSqrtOpt2(squared);
            facing = inv_MTH_fn_xNegate(dot);
            factor = moveLength * inv_MTH_fn_xSign(dotMove);
            if (facing > 0.92718f) {
                factor = factor * ((1.0f - facing) / 0.07282001f);
            }
        }
    } else {
        factor = dotMove;
    }

    inv_MTH3D_fn_vKTimesXYZ(pstResultMove, factor, &direction);
    {
        f32 squared;

        squared = MTH_fn_xSqrAddSqrAddSqrOpt2(pstResultMove->xX, pstResultMove->xY, pstResultMove->xZ);
        norm = MTH_fn_xSqrtOpt2(squared) * 0.01f;
    }
    inv_MTH3D_fn_vKTimesXYZ(&offset, norm, pstWallNormal);
    inv_MTH3D_fn_vAddXYZ(pstResultMove, pstResultMove, &offset);
    inv_POS_fn_vTranslate(currentMatrix, pstResultMove);
}

void MEC_fn_vOrient(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stNormal)
{
    MTH3D_tdstVector_ stZ;
    MTH3D_tdstVector_ stY;
    MTH3D_tdstVector_ stX;
    MTH3D_tdstVector_ stVector;
    MTH3D_tdstVector_ forward;
    MTH3D_tdstVector_ axis;
    f32 sideX;
    f32 sideY;
    f32 sideZ;
    f32 squared;
    f32 length;
    f32 zero = 0.0f;
    f32 one = 1.0f;

    MTH3D_M_vSetXYZ_inv(&stZ, zero, zero, one);

    POS_fn_vGetRotationVector(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix, &axis);

    MTH3D_M_vScaleXYZ_inv(&stX, &axis, 0.9f);

    forward = *p_stNormal;

    sideX = inv_MTH_fn_xMulSub(forward.xY, stZ.xZ, forward.xZ, stZ.xY);
    sideY = inv_MTH_fn_xMulSub(forward.xZ, stZ.xX, forward.xX, stZ.xZ);
    sideZ = inv_MTH_fn_xMulSub(forward.xX, stZ.xY, forward.xY, stZ.xX);
    squared = MTH_fn_xSqrAddSqrAddSqrOpt2(sideX, sideY, sideZ);

    MTH3D_M_vSetXYZ_inv(&stVector, sideX, sideY, sideZ);

    length = MTH_fn_xSqrtOpt2(squared);
    if (length != zero) {
        f32 inverse;

        inverse = one / length;
        inv_MTH3D_fn_vKTimesXYZ(&stVector, inverse, &stVector);
    }

    DNM_p_stVectorCrossProduct(&axis, &stVector, &forward);

    MTH3D_M_vSetXYZ_inv(&stY, stX.xX - axis.xX * 0.9f, stX.xY - axis.xY * 0.9f,
                        stX.xZ - axis.xZ * 0.9f);

    inv_POS_fn_vTranslate(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                                 &stY);
    inv_POS_fn_vInitMatrixFromVectors(
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix, MATRIX_INIT_ORTHONORMAL, &stVector,
        &forward, &axis,
        inv_POS_fn_p_stPosMatrixTranslation(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix));
    POS_fn_vSetScaleMatrix(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                           &p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale);
}

void MEC_fn_vBaseObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT)
{
    POS_tdstMatrix_ localMatrix;
    s32 count;
    POS_tdstMatrix_* start;
    POS_tdstMatrix_* end;
    MTH4D_tdstVector_* translation;

    end = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    if ((u32) fn_ucCollSetGetCollComputeFrequencySO(hSupObj) >= 2) {
        start = &localMatrix;
        localMatrix = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
        translation = &localMatrix.stMatrix.stCol_3;
        if (start->ucType == MAT_eTransform_Identity) {
            start->ucType = MAT_eTransform_Translation;
        }
        *inv_POS_fn_p_stPosMatrixTranslation(start) =
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation;
        translation->xW = 1.0f;
    } else {
        start = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    }

    if (MEC_fn_bCollision(
            &count, g_a_stObstacle, p_stDynamic, hSupObj, start, end,
            (s32) p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags >> DYNAMICS_FLAG_COLLISION_BIT & 1)) {
        MEC_fn_vEdgeAnalysis(p_stDynamic, count, g_a_stObstacle, NULL);
        MEC_fn_vCollideBaseMechanic(p_stDynamic, hSupObj, count, g_a_stObstacle, xDT);
    } else {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_FLAG_IN_COLLISION;
    }
}

static inline void inv_MEC_fn_vSetTranslationSplitW(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* column,
                                                    MTH3D_tdstVector_* translation)
{
    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    *inv_POS_fn_p_stPosMatrixTranslation(matrix) = *translation;
    ((f32*) column)[3] = 1.0f;
}

static inline u8 inv_MEC_fn_ucIsValidContact(DNM_stMecObstacle* p)
{
    return (u8) (p->m_xCollidedEntity != 99);
}

void MEC_fn_vGiObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT)
{
    MTH3D_tdstVector_ vector;
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ tmpVector;
    MTH3D_tdstVector_ slideMove;
    MTH3D_tdstVector_ move;
    MTH3D_tdstVector_ resultMove;
    MTH3D_tdstVector_ verticalVector;
    MTH3D_tdstVector_ adjustVector;
    POS_tdstMatrix_ a2_stPosition[2];
    MTH3D_tdstVector_ newPosition;
    s32 obstacle;
    s32 face;
    u8 giFace;
    MTH3D_tdstVector_* currentPosition;
    MTH3D_tdstVector_* previousPosition;
    MTH3D_tdstVector_* giContact;
    DNM_stMecObstacle* giObstacle;
    POS_tdstMatrix_* previousMatrix;
    POS_tdstMatrix_* currentMatrix;
    POS_tdstMatrix_* upPosition;
    POS_tdstMatrix_* downPosition;
    DNM_stObstacle* previousCeilObstacle;
    DNM_stMecObstacle* slideObstacle;
    DNM_stMecObstacle* tmpObstacle;
    MTH3D_tdstVector_* giNormal;
    MTH3D_tdstVector_* downTranslation;
    f32 normalMove;
    f32 giCoef;
    f32 norm;
    f32 dot;
    s32 counter1;
    s32 counter;
    u64 flags;

    face = 0;
    giFace = 0;
    giCoef = p_stDynamic->m_tdstDynamicsBaseBlock.m_xSlopeLimit;
    previousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    currentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    upPosition = &a2_stPosition[0];
    downPosition = &a2_stPosition[1];
    MTH3D_M_vSetXYZ_inv(&verticalVector, 0.0f, 0.0f, 1.0f);

    *downPosition = *currentMatrix;
    *upPosition = *currentMatrix;
    currentPosition = inv_POS_fn_p_stPosMatrixTranslation(currentMatrix);
    previousPosition = inv_POS_fn_p_stPosMatrixTranslation(previousMatrix);

    inv_MTH3D_fn_vSubXYZ(&move, currentPosition, previousPosition);

    g_lCallCounter++;
    if (g_lCallCounter >= 4) {
        inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, previousPosition);
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ =
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        return;
    }

    norm = MTH_fn_xSqrAddSqrAddSqrOpt2(move.xX, move.xY, move.xZ);
    normalMove = giCoef * MTH_fn_xSqrtOpt2(norm);
    if (normalMove < 0.45f)
        normalMove = 0.45f;

    giContact = NULL;
    previousCeilObstacle = &p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stCeil;
    if (previousCeilObstacle != NULL) {
        giNormal = &previousCeilObstacle->m_stNorm;
        giContact = &previousCeilObstacle->m_stContact;
    } else {
        giNormal = &verticalVector;
    }

    dot = inv_MTH3D_fn_xDotXYZGrouped(&move, giNormal);
    inv_MTH3D_fn_vKTimesXYZ(&tmpVector, dot, giNormal);
    inv_MTH3D_fn_vSubXYZ(&slideMove, &move, &tmpVector);
    inv_MTH3D_fn_vKTimesXYZ(&vector, normalMove, giNormal);

    inv_MTH3D_fn_vAddXYZ(&position, previousPosition, &slideMove);
    inv_MTH3D_fn_vAddXYZ(&position, &position, &vector);
    inv_POS_fn_vSetTranslation(upPosition, &position);

    inv_MTH3D_fn_vAddXYZ(&position, previousPosition, &slideMove);
    inv_MTH3D_fn_vSubXYZ(&position, &position, &vector);
    downTranslation = inv_POS_fn_p_stPosMatrixTranslation(downPosition);
    inv_MEC_fn_vSetTranslationSplitW(downPosition, downTranslation, &position);

    slideObstacle = NULL;
    giObstacle = NULL;
    if (MEC_fn_bCollision(&obstacle, g_a_stObstacle, p_stDynamic, hSupObj, upPosition, downPosition, 1)) {
        MEC_fn_vDeleteGiBackFace(obstacle, g_a_stObstacle, p_stDynamic, &move);
        for (counter1 = 0; counter1 < obstacle && (slideObstacle == NULL || !giFace); counter1++) {
            tmpObstacle = &g_a_stObstacle[counter1];
            if (inv_MEC_fn_ucIsValidContact(tmpObstacle) &&
                (flags = tmpObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier,
                 ((flags >> 5) & 1) == 1) &&
                !(flags & 0x8000)) {
                if (MEC_fn_bCeilObstacle(p_stDynamic, &tmpObstacle->contact)) {
                    if (slideObstacle == NULL)
                        slideObstacle = tmpObstacle;
                    if (!(tmpObstacle->m_xCollidedEntity == 7 || tmpObstacle->m_xCollidedEntity == 8))
                        giFace = 1;
                }
            }
        }
        if (slideObstacle != NULL) {
            giNormal = &slideObstacle->contact.m_stNorm;
            giContact = &slideObstacle->contact.m_stContact;
        }
        for (counter = 0; counter < obstacle && giObstacle == NULL; counter++) {
            tmpObstacle = &g_a_stObstacle[counter];
            if (inv_MEC_fn_ucIsValidContact(tmpObstacle) &&
                !((flags = tmpObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier,
                   ((flags >> 5) & 1) == 1) &&
                  !(flags & 0x8000))) {
                f32 dotNorm;
                dotNorm = inv_MTH_fn_xMulAdd3(tmpObstacle->contact.m_stNorm.xX, giNormal->xX,
                                   tmpObstacle->contact.m_stNorm.xY, giNormal->xY,
                                   tmpObstacle->contact.m_stNorm.xZ, giNormal->xZ);
                if (dotNorm < 0.707f) {
                    MTH3D_M_vSubXYZ_inv(&vector, &tmpObstacle->contact.m_stContact, giContact);
                    if (-0.01f < inv_MTH3D_fn_xDotXYZGrouped(&vector, giNormal))
                        giObstacle = tmpObstacle;
                }
            }
        }
        if (giObstacle != NULL && counter1 < counter)
            counter1 = counter;
        MEC_fn_vReportCollision(p_stDynamic, hSupObj, obstacle, g_a_stObstacle, counter1 - 1);

        if (slideObstacle != NULL && (slideObstacle->m_xCollidedEntity == 7 || slideObstacle->m_xCollidedEntity == 8) &&
            giObstacle == NULL && !giFace) {
            for (counter = 0; counter < obstacle; counter++) {
                tmpObstacle = &g_a_stObstacle[counter];
                if (inv_MEC_fn_ucIsValidContact(tmpObstacle)) {
                    MTH3D_M_vSubXYZ_inv(&vector, giContact, &tmpObstacle->contact.m_stContact);
                    face += inv_MEC_fn_bIsNullMove(&vector);
                }
            }
            if (face == 1 && !((slideObstacle->m_eType >> 2) & 1)) {
                if (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10))
                    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x10000;
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, previousPosition);
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ =
                    p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY =
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
                if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)
                    p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState =
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState;
                return;
            }
        }

        if (slideObstacle != NULL) {
            if (giObstacle == NULL) {
                MTH3D_M_vAddXYZ_inv(&vector, downTranslation, &slideObstacle->m_stTranslation);
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &vector);
            } else {
                giFace = giObstacle->contact.m_xRate < 0.0f;
                if (giFace && g_lCallCounter >= 3) {
                    MEC_fn_vComputeAdjustVector(&adjustVector, p_stDynamic, obstacle, g_a_stObstacle,
                                                counter1);
                    inv_MTH3D_fn_vAddXYZ(&newPosition, downTranslation, &adjustVector);
                    inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &newPosition);
                    inv_MTH3D_fn_vSubXYZ(&vector, currentPosition, previousPosition);
                    inv_MTH3D_fn_vSubXYZ(&move, &move, &vector);
                    inv_MEC_fn_vSetTranslationSplitW(previousMatrix, previousPosition, currentPosition);
                } else {
                    inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, previousPosition);
                }
                MEC_fn_vWallObstacleManagement(&resultMove, p_stDynamic, &move, &slideObstacle->contact.m_stNorm, &giObstacle->contact.m_stNorm, xDT, 0);
                if (!inv_MEC_fn_bIsNullMove(&resultMove)) {
                    MEC_fn_vGiObstacleManagement(p_stDynamic, hSupObj, xDT);
                    MEC_fn_vComputeSpeed(p_stDynamic, xDT);
                    return;
                } else
                    inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, previousPosition);
            }
        } else if (giObstacle != NULL) {
            giFace = giObstacle->contact.m_xRate < 0.0f;
            if (giFace) {
                MEC_fn_vComputeAdjustVector(&adjustVector, p_stDynamic, obstacle, g_a_stObstacle,
                                            counter1);
                inv_MTH3D_fn_vAddXYZ(&newPosition, downTranslation, &adjustVector);
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &newPosition);
                inv_MTH3D_fn_vSubXYZ(&vector, currentPosition, previousPosition);
                inv_MTH3D_fn_vSubXYZ(&move, &move, &vector);
                inv_MEC_fn_vSetTranslationSplitW(previousMatrix, previousPosition, currentPosition);
            } else {
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, previousPosition);
            }
            MEC_fn_vWallObstacleManagement(&resultMove, p_stDynamic, &move, giNormal, &giObstacle->contact.m_stNorm, xDT, 0);
            if (!inv_MEC_fn_bIsNullMove(&resultMove)) {
                MEC_fn_vGiObstacleManagement(p_stDynamic, hSupObj, xDT);
                MEC_fn_vComputeSpeed(p_stDynamic, xDT);
                return;
            } else
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, previousPosition);
        } else {
            MTH3D_tdstVector_* speed;
            *currentMatrix = *previousMatrix;
            speed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = speed->xY =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
            if (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10))
                p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x10000;
            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)
                p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState =
                    p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState;
            return;
        }
        MEC_fn_vComputeSpeed(p_stDynamic, xDT);
    } else {
        MTH3D_tdstVector_* speed;
        *currentMatrix = *previousMatrix;
        speed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = speed->xY =
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        if (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10))
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x10000;
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)
            p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState;
    }
}

extern MTH3D_tdstVector_ inv_MEC_stFirstCallTangentMove;
void POS_fn_vGetYAxisVector(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* axis);

static inline u8 inv_MEC_fn_ucClimbIsNullVector(MTH3D_tdstVector_* v)
{
    u8 bResult = 0;
    if (inv_MTH_fn_xAbsoluteTied(v->xX - 0.0f) < 0.001f &&
        inv_MTH_fn_xAbsoluteTied(v->xY - 0.0f) < 0.001f &&
        inv_MTH_fn_xAbsoluteTied(v->xZ - 0.0f) < 0.001f)
        bResult = 1;
    return bResult;
}

static inline void inv_MEC_fn_vSetTranslation4D(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* translation)
{
    MTH4D_tdstVector_* column = &matrix->stMatrix.stCol_3;

    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    *(MTH3D_tdstVector_*) column = *translation;
    column->xW = 1.0f;
}

void MEC_fn_vClimbObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT)
{
    MTH3D_tdstVector_ y;
    MTH3D_tdstVector_ tmpVector;
    MTH3D_tdstVector_ adjustVector;
    MTH3D_tdstVector_ move;
    MTH3D_tdstVector_ safeTranslation;
    MTH3D_tdstVector_ tangentMove;
    MTH3D_tdstVector_ slideMove;
    MTH3D_tdstVector_ vertical;
    MTH3D_tdstVector_ vector;
    MTH3D_tdstVector_ normMove;
    MTH3D_tdstVector_ normTangentMove;
    MTH3D_tdstVector_ position;
    POS_tdstMatrix_ a2_stPosition[2];
    s32 obstacle;
    s32 counter2;
    MTH3D_tdstVector_* savedPosition;
    DNM_stMecObstacle* groundObstacle;
    POS_tdstMatrix_* currentMatrix;
    POS_tdstMatrix_* previousMatrix;
    MTH3D_tdstVector_* currentPosition;
    MTH3D_tdstVector_* previousPosition;
    POS_tdstMatrix_* frontPosition;
    POS_tdstMatrix_* backPosition;
    DNM_stMecObstacle* slideObstacle;
    DNM_stMecObstacle* climbObstacle;
    DNM_stMecObstacle* tmpObstacle;
    MTH3D_tdstVector_* backTranslation;
    f32 norm;
    f32 dotProdMin;
    f32 slope;
    f32 normalMove;
    f32 dot;
    u8 stop;
    u8 null;
    s32 counter1;
    s32 counter;

    previousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    currentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    frontPosition = &a2_stPosition[0];
    backPosition = &a2_stPosition[1];
    dotProdMin = 0.0f;
    slope = tanf(1.0471976f);
    counter2 = 0;
    MTH3D_M_vSetXYZ_inv(&vertical, 0.0f, 0.0f, 1.0f);

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10) {
        MTH3D_tdstVector_* normal = &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal;
        if (!(MTH3D_M_bIsZeroXYZ_inv(normal)))
            MEC_fn_vOrient(p_stDynamic, normal);
    }

    *backPosition = *currentMatrix;
    *frontPosition = *currentMatrix;
    currentPosition = inv_POS_fn_p_stPosMatrixTranslation(currentMatrix);
    previousPosition = inv_POS_fn_p_stPosMatrixTranslation(previousMatrix);

    savedPosition = inv_POS_fn_p_stPosMatrixTranslation(previousMatrix);
    inv_MTH3D_fn_vSubXYZ(&move, currentPosition, savedPosition);
    if (!inv_MEC_fn_ucClimbIsNullVector(&move)) {
        f32 len;
        f32 xInvLength;
        len = MTH_fn_xSqrAddSqrAddSqrOpt2(move.xX, move.xY, move.xZ);
        len = MTH_fn_xSqrtOpt2(len);
        if (len != 0.0f) {
            xInvLength = 1.0f / len;
            inv_MTH3D_fn_vKTimesXYZ(&normMove, xInvLength, &move);
        } else {
            normMove = move;
        }
    } else {
        normMove.xZ = normMove.xY = normMove.xX = 0.0f;
    }

    g_lCallCounter++;
    if (g_lCallCounter >= 4) {
        inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, savedPosition);
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ =
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        return;
    }

    POS_fn_vGetYAxisVector(currentMatrix, &y);

    normalMove =
        inv_MTH_fn_xMulAdd3(move.xX, p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal.xX,
                            move.xY, p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal.xY,
                            move.xZ, p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal.xZ);
    MTH3D_M_vKTimesXYZ_inv(&safeTranslation, normalMove,
                           &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal);
    inv_MTH3D_fn_vSubXYZ(&tangentMove, &move, &safeTranslation);

    if (g_lCallCounter == 1)
        inv_MEC_stFirstCallTangentMove = tangentMove;

    if (!inv_MEC_fn_ucClimbIsNullVector(&tangentMove)) {
        f32 len;
        f32 xInvLength;
        len = MTH_fn_xSqrAddSqrAddSqrOpt2(tangentMove.xX, tangentMove.xY, tangentMove.xZ);
        len = MTH_fn_xSqrtOpt2(len);
        if (len != 0.0f) {
            xInvLength = 1.0f / len;
            inv_MTH3D_fn_vKTimesXYZ(&normTangentMove, xInvLength, &tangentMove);
        } else {
            normTangentMove = tangentMove;
        }
    } else {
        normTangentMove.xZ = normTangentMove.xY = normTangentMove.xX = 0.0f;
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10) {
        norm = 0.75f;
    } else {
        f32 len;
        len = MTH_fn_xSqrAddSqrAddSqrOpt2(tangentMove.xX, tangentMove.xY, tangentMove.xZ);
        norm = slope * MTH_fn_xSqrtOpt2(len);
    }
    if (norm < 0.45f)
        norm = 0.45f;

    inv_MTH3D_fn_vAddXYZ(&position, previousPosition, &tangentMove);
    MTH3D_M_vKTimesXYZ_inv(&tmpVector, norm,
                           &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal);
    inv_MTH3D_fn_vAddXYZ(&position, &position, &tmpVector);
    inv_MEC_fn_vSetTranslation4D(frontPosition, &position);

    MTH3D_M_vSetXYZ_inv(&position, previousPosition->xX + tangentMove.xX - tmpVector.xX,
                        previousPosition->xY + tangentMove.xY - tmpVector.xY,
                        previousPosition->xZ + tangentMove.xZ - tmpVector.xZ);
    inv_MEC_fn_vSetTranslation4D(backPosition, &position);
    backTranslation = inv_POS_fn_p_stPosMatrixTranslation(backPosition);

    climbObstacle = 0;
    groundObstacle = 0;
    slideObstacle = 0;

    if (MEC_fn_bCollision(&obstacle, g_a_stObstacle, p_stDynamic, hSupObj, frontPosition, backPosition,
                          1)) {
        MEC_fn_vDeleteClimbBackFace(obstacle, g_a_stObstacle, p_stDynamic, &normTangentMove);

        for (counter = 0; counter < obstacle && climbObstacle == 0; counter++) {
            tmpObstacle = &g_a_stObstacle[counter];
            if (inv_MEC_fn_ucIsValidContact(tmpObstacle)) {
                u64 flags = tmpObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;
                if (((flags >> 6) & 1) == 1 && !(flags & 0x8000)) {
                    if ((u8) MEC_fn_bWallNormal(p_stDynamic, &tmpObstacle->contact.m_stNorm))
                        climbObstacle = tmpObstacle;
                }
            }
        }

        for (counter = 0; counter < obstacle && slideObstacle == 0; counter++) {
            tmpObstacle = &g_a_stObstacle[counter];
            if (inv_MEC_fn_ucIsValidContact(tmpObstacle)) {
                if ((u8) MEC_fn_bGroundNormal(p_stDynamic, &tmpObstacle->contact.m_stNorm)) {
                    slideObstacle = tmpObstacle;
                    MEC_fn_vReportCollision(p_stDynamic, hSupObj, 1, slideObstacle, 0);
                }
            }
        }

        if (climbObstacle != 0) {
            stop = 0;
            for (counter2 = 0; counter2 < obstacle && !stop; counter2++) {
                tmpObstacle = &g_a_stObstacle[counter2];
                if (inv_MEC_fn_ucIsValidContact(tmpObstacle)) {
                    dot = inv_MTH_fn_xMulAdd3(tmpObstacle->contact.m_stNorm.xX,
                                   climbObstacle->contact.m_stNorm.xX,
                                   tmpObstacle->contact.m_stNorm.xY,
                                   climbObstacle->contact.m_stNorm.xY,
                                   tmpObstacle->contact.m_stNorm.xZ,
                                   climbObstacle->contact.m_stNorm.xZ);
                    if (dot < 0.707f) {
                        MTH3D_M_vSubXYZ_inv(&tmpVector, &tmpObstacle->contact.m_stContact,
                                            &climbObstacle->contact.m_stContact);
                        dot = inv_MTH_fn_xMulAdd3(tmpVector.xX, climbObstacle->contact.m_stNorm.xX,
                                       tmpVector.xY, climbObstacle->contact.m_stNorm.xY,
                                       tmpVector.xZ, climbObstacle->contact.m_stNorm.xZ);
                        if (dot > 0.1f) {
                            u64 flags =
                                tmpObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;
                            dot = inv_MTH_fn_xMulAdd3(
                                normTangentMove.xX, tmpObstacle->contact.m_stNorm.xX,
                                normTangentMove.xY, tmpObstacle->contact.m_stNorm.xY,
                                normTangentMove.xZ, tmpObstacle->contact.m_stNorm.xZ);
                            if (((flags >> 6) & 1) == 1 && !(flags & 0x8000)) {
                                groundObstacle = 0;
                                stop = 1;
                            } else if (dot < dotProdMin) {
                                groundObstacle = tmpObstacle;
                                dotProdMin = dot;
                            }
                        }
                    }
                }
            }
        }

        if (groundObstacle != 0) {
            u64 flags = groundObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier;
            if (((flags >> 6) & 1) == 1 && !(flags & 0x8000)) {
                if ((u8) MEC_fn_bWallNormal(p_stDynamic, &groundObstacle->contact.m_stNorm)) {
                    dot = inv_MTH_fn_xMulAdd3(groundObstacle->contact.m_stNorm.xX,
                                   climbObstacle->contact.m_stNorm.xX,
                                   groundObstacle->contact.m_stNorm.xY,
                                   climbObstacle->contact.m_stNorm.xY,
                                   groundObstacle->contact.m_stNorm.xZ,
                                   climbObstacle->contact.m_stNorm.xZ);
                    if (dot != 0.0f)
                        groundObstacle = 0;
                }
            }
        }

        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) &&
            !(p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState & 1) &&
            !(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10) && tangentMove.xZ < 0.0f) {
            for (counter1 = 0; counter1 < obstacle; counter1++) {
                tmpObstacle = &g_a_stObstacle[counter1];
                if (inv_MEC_fn_ucIsValidContact(tmpObstacle) &&
                    !(tmpObstacle->contact.hCollidedMaterial->collideMaterial.xIdentifier &
                      0x8000)) {
                    if ((u8) MEC_fn_bGroundNormal(p_stDynamic, &tmpObstacle->contact.m_stNorm)) {
                        groundObstacle = tmpObstacle;
                        counter = counter1;
                        slideObstacle = tmpObstacle;
                        climbObstacle = tmpObstacle;
                        norm = dot;
                        inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, savedPosition);
                        {
                            MTH3D_tdstVector_* speed =
                                &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = speed->xY =
                                speed->xX = 0.0f;
                        }
                        {
                            u32 flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags | 0x10000;
                            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = flags;
                            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = flags;
                        }
                        MEC_fn_vReportCollision(p_stDynamic, hSupObj, obstacle, g_a_stObstacle,
                                                counter2 - 1);
                        return;
                    }
                }
            }
        }

        if (groundObstacle != 0) {
            if (counter < counter2)
                counter = counter2;
        }
        MEC_fn_vReportCollision(p_stDynamic, hSupObj, obstacle, g_a_stObstacle, counter - 1);

        if (climbObstacle != 0) {
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stWallNormal = climbObstacle->contact.m_stNorm;
            if (groundObstacle == 0) {
                if (!(climbObstacle->m_xCollidedEntity == 7 || climbObstacle->m_xCollidedEntity == 8)) {
                    MTH3D_M_vAddXYZ_inv(&tmpVector, backTranslation,
                                        &climbObstacle->m_stTranslation);
                    inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &tmpVector);
                } else {
                    for (counter = 0; counter < obstacle && groundObstacle == 0; counter++) {
                        tmpObstacle = &g_a_stObstacle[counter];
                        if (inv_MEC_fn_ucIsValidContact(tmpObstacle)) {
                            if ((u8) MEC_fn_bWallNormal(p_stDynamic, &tmpObstacle->contact.m_stNorm)) {
                                if (!(tmpObstacle->m_xCollidedEntity == 7 || tmpObstacle->m_xCollidedEntity == 8))
                                    groundObstacle = tmpObstacle;
                            }
                        }
                    }
                    if (groundObstacle != 0) {
                        MTH3D_M_vAddXYZ_inv(&tmpVector, backTranslation,
                                            &groundObstacle->m_stTranslation);
                        inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &tmpVector);
                    } else {
                        MTH3D_M_vAddXYZ_inv(&tmpVector, backTranslation,
                                            &climbObstacle->m_stTranslation);
                        inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &tmpVector);
                    }
                }
            } else {
                null = groundObstacle->contact.m_xRate < 0.0f;
                if (null &&
                    (g_lCallCounter >= 3 || (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10))) {
                    MEC_fn_vComputeAdjustVector(&adjustVector, p_stDynamic, obstacle, g_a_stObstacle,
                                                counter - 1);
                    dot = inv_MTH_fn_xMulAdd3(adjustVector.xX, climbObstacle->contact.m_stNorm.xX,
                                   adjustVector.xY, climbObstacle->contact.m_stNorm.xY,
                                   adjustVector.xZ, climbObstacle->contact.m_stNorm.xZ);
                    MTH3D_M_vKTimesXYZ_inv(&tmpVector, dot, &climbObstacle->contact.m_stNorm);
                    MTH3D_M_vSubXYZ_inv(&adjustVector, &adjustVector, &tmpVector);
                    MTH3D_M_vAddXYZ_inv(&adjustVector, &adjustVector,
                                        &climbObstacle->m_stTranslation);
                    inv_MTH3D_fn_vAddXYZ(&vector, backTranslation, &adjustVector);
                    inv_MTH3D_fn_vSubXYZ(&tmpVector, &vector, previousPosition);
                    MTH3D_M_vSubXYZ_inv(&move, &move, &tmpVector);
                } else {
                    vector = *savedPosition;
                }
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &vector);
                MEC_fn_vWallObstacleManagement(&slideMove, p_stDynamic, &move, &climbObstacle->contact.m_stNorm, &groundObstacle->contact.m_stNorm, xDT, 1);
                dot = inv_MTH3D_fn_xDotXYZ(&inv_MEC_stFirstCallTangentMove, &slideMove);
                null = inv_MEC_fn_ucClimbIsNullVector(&slideMove);
                if (!null && dot >= 0.0f) {
                    MEC_fn_vClimbObstacleManagement(p_stDynamic, hSupObj, xDT);
                    MEC_fn_vComputeSpeed(p_stDynamic, xDT);
                    return;
                }
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &vector);
            }
        } else if (groundObstacle != 0) {
            MEC_fn_vComputeAdjustVector(&adjustVector, p_stDynamic, obstacle, g_a_stObstacle, counter);
            inv_MTH3D_fn_vAddXYZ(&vector, backTranslation, &adjustVector);
            inv_MTH3D_fn_vSubXYZ(&tmpVector, &vector, previousPosition);
            MTH3D_M_vSubXYZ_inv(&move, &move, &tmpVector);
            inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &vector);
            MEC_fn_vWallObstacleManagement(&slideMove, p_stDynamic, &move, &y, &groundObstacle->contact.m_stNorm, xDT, 1);
            dot = inv_MTH3D_fn_xDotXYZ(&inv_MEC_stFirstCallTangentMove, &slideMove);
            if (dot < 0.0f) {
                inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, savedPosition);
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ =
                    p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY =
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
                return;
            }
            if (!inv_MEC_fn_ucClimbIsNullVector(&slideMove)) {
                MEC_fn_vClimbObstacleManagement(p_stDynamic, hSupObj, xDT);
                MEC_fn_vComputeSpeed(p_stDynamic, xDT);
                return;
            }
            inv_MEC_fn_vSetTranslationSplitW(currentMatrix, currentPosition, &vector);
        } else {
            MTH3D_tdstVector_* speed;
            *currentMatrix = *previousMatrix;
            speed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = speed->xY =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x10000;
            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)
                p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState =
                    p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState;
            return;
        }

        if (climbObstacle != 0) {
            if (!(climbObstacle->m_xCollidedEntity == 7 || climbObstacle->m_xCollidedEntity == 8)) {
                MTH3D_tdstVector_* normal = &climbObstacle->contact.m_stNorm;
                inv_MTH3D_fn_vSubXYZ(&tmpVector, normal, &y);
                if (!inv_MEC_fn_ucClimbIsNullVector(&tmpVector))
                    MEC_fn_vOrient(p_stDynamic, normal);
            }
        }
        MEC_fn_vComputeSpeed(p_stDynamic, xDT);
    } else {
        MTH3D_tdstVector_* speed;
        *currentMatrix = *previousMatrix;
        speed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = speed->xY =
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x10000;
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)
            p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState;
    }
}

extern f32 inv_MEC_xInitialWallDistance[];

f32 acosf(f32 value);

static inline u8 inline_00131DA8_3(DNM_stMecObstacle *_p)
{
    return (u8)(_p->contact.m_xRate < 0.0f);
}

static inline u8 inline_00131DA8_4(DNM_stDynamics *p_stDynamic, DNM_stMecObstacle *_p)
{
    return (u8)MEC_fn_bWallNormal(p_stDynamic, &_p->contact.m_stNorm);
}

static inline u8 inline_00131DA8_5(DNM_stDynamics *p_stDynamic, DNM_stMecObstacle *_p)
{
    return (u8)MEC_fn_bGroundNormal(p_stDynamic, &_p->contact.m_stNorm);
}

void MEC_fn_vGroundObstacleManagement(DNM_stDynamics *p_stDynamic, void *hSupObj, f32 xDT)
{
    POS_tdstMatrix_ a2_stPosition[2];
    POS_tdstMatrix_ stPreviousMatrix;
    MTH3D_tdstVector_ stVector;
    MTH3D_tdstVector_ stUpVector;
    MTH3D_tdstVector_ stDownVector;
    MTH3D_tdstVector_ stMove;
    MTH3D_tdstVector_ stVertical;
    MTH3D_tdstVector_ stVarVector1;
    MTH3D_tdstVector_ stTangentMove;
    MTH3D_tdstVector_ stAbsoluteTangentMove;
    MTH3D_tdstVector_ stAdjustVector;
    MTH3D_tdstVector_ stAdjustVector1;
    MTH3D_tdstVector_ stVarVector2;
    MTH3D_tdstVector_ stWallMove;
    MTH3D_tdstVector_ stSafeTranslation;
    MTH3D_tdstVector_ stVarVector3;
    MTH3D_tdstVector_ stVarVector4;
    u8 bFirstGround;
    u8 bHasStep;
    s32 lObstacle;
    s32 lObstacle2;
    DNM_stMecObstacle *p_stVarObstacle1;
    s32 lCounter;
    s32 lObstacleCounter;
    f32 xSlide;
    f32 xFactor;
    f32 xSpeedZ;
    POS_tdstMatrix_ *p_stPOS_DownPosition;
    POS_tdstMatrix_ *p_stPOS_UpPosition;
    POS_tdstMatrix_ *hCurrentMatrix;
    POS_tdstMatrix_ *hPreviousMatrix;
    MTH3D_tdstVector_ *p_stVarVector1;
    MTH3D_tdstVector_ *p_stVarVector2;
    MTH3D_tdstVector_ *p_stVarVector3;
    MTH3D_tdstVector_ *p_stNormal;
    u8 bAdjusted;
    u8 bForceAdjust;
    f32 xVar2;
    f32 xVar1;
    f32 xVar3;
    f32 xVar4;
    f32 xWallDistance;
    f32 xTmpReal;
    DNM_stMecObstacle *p_stGroundObstacle;
    DNM_stMecObstacle *p_stWallObstacle;
    DNM_stMecObstacle *p_stTmpObstacle;
    DNM_stObstacle *p_stObstacle;

    p_stPOS_DownPosition = a2_stPosition;
    p_stPOS_UpPosition = a2_stPosition + 1;
    bAdjusted = 1;
    bForceAdjust = 0;
    xWallDistance = inv_MEC_xInitialWallDistance[0];

    xVar1 = p_stDynamic->m_tdstDynamicsBaseBlock.m_xSlopeLimit;
    xVar2 = inv_MTH_fn_xNegate(xVar1);
    MTH3D_M_vSetXYZ_inv(&stVertical, 0.0f, 0.0f, 1.0f);
    stAdjustVector.xZ = stAdjustVector.xY = stAdjustVector.xX = 0.0f;
    stVarVector2.xZ = stVarVector2.xY = stVarVector2.xX = 0.0f;
    p_stGroundObstacle = 0;
    p_stWallObstacle = 0;
    p_stTmpObstacle = 0;
    lObstacleCounter = 0;
    lCounter = 0;
    p_stObstacle = 0;

    hCurrentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    hPreviousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000)
        p_stObstacle = &p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stGround;

    *p_stPOS_DownPosition = *hCurrentMatrix;
    *p_stPOS_UpPosition = *hPreviousMatrix;

    if ((u32) fn_ucCollSetGetCollComputeFrequencySO(hSupObj) > 1) {
        hPreviousMatrix = &stPreviousMatrix;
        *hPreviousMatrix = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
        inv_MEC_fn_vSetTranslation4D(hPreviousMatrix, &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation);
    }

    p_stVarVector1 = (MTH3D_tdstVector_ *)&hCurrentMatrix->stMatrix.stCol_3;
    p_stVarVector2 = (MTH3D_tdstVector_ *)&hPreviousMatrix->stMatrix.stCol_3;
    MTH3D_M_vSubXYZ_inv(&stMove, p_stVarVector1, p_stVarVector2);

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10) && !g_lCallCounter) {
        if (MEC_fn_bCollision(&lObstacle, g_a_stObstacle, p_stDynamic, hSupObj, hPreviousMatrix, hPreviousMatrix, 1)) {
            MEC_fn_vDeleteBaseBackFace(lObstacle, g_a_stObstacle, p_stDynamic);
            MEC_fn_vDeleteNullTranslationObstacle(lObstacle, g_a_stObstacle);
            MEC_fn_vComputeAdjustVector(&stAdjustVector, p_stDynamic, lObstacle, g_a_stObstacle, lObstacle);
            MTH3D_M_vAddXYZ_inv(p_stVarVector2, p_stVarVector2, &stAdjustVector);
        }
    }

    g_lCallCounter++;
    if (g_lCallCounter > 3) {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x02000000;
        inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, p_stVarVector2);
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        return;
    }

    p_stNormal = &stVertical;
    if (p_stObstacle && !(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10)) {
        p_stNormal = &p_stObstacle->m_stNorm;
        if (MTH3D_M_bIsZeroXYZ_inv(p_stNormal))
            p_stNormal = &stVertical;
    }

    xVar3 = inv_MTH3D_fn_xDotXYZ(&stMove, p_stNormal);
    MTH3D_M_vKTimesXYZ_inv(&stVarVector1, xVar3, p_stNormal);
    MTH3D_M_vSubXYZ_inv(&stTangentMove, &stMove, &stVarVector1);

    xVar4 = inv_MTH3D_fn_xNormXYZ(&stTangentMove);

    if (xVar4 > 0.001f) {
        MTH3D_M_vSetXYZ_inv(&stVarVector3, stTangentMove.xX / xVar4, stTangentMove.xY / xVar4,
                            stTangentMove.xZ / xVar4);
    } else {
        stVarVector3.xZ = stVarVector3.xY = stVarVector3.xX = 0.0f;
    }

    if (g_lCallCounter == 1) {
        g_bHighSpeed = xVar4 > 0.9f || (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x02000000) ||
                     (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10);
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x02000000;
    } else if (!g_bHighSpeed) {
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x02000000)
            g_bHighSpeed = 1;
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xX < 0.5f) {
        if (MEC_fn_bMainActor(hSupObj))
            g_bHighSpeed = 1;
    }

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) && p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform) {
        MTH3D_tdstVector_ *p_stVarVector4 = (MTH3D_tdstVector_ *)&p_stDynamic->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix.stMatrix.stCol_3;
        MTH3D_M_vSubXYZ_inv(&stAbsoluteTangentMove, p_stVarVector1, p_stVarVector4);
        xVar3 = inv_MTH3D_fn_xDotXYZ(&stAbsoluteTangentMove, p_stNormal);
        MTH3D_M_vKTimesXYZ_inv(&stVector, xVar3, p_stNormal);
        MTH3D_M_vSubXYZ_inv(&stAbsoluteTangentMove, &stAbsoluteTangentMove, &stVector);
    } else {
        stAbsoluteTangentMove = stTangentMove;
    }

    if (xVar4 < 1e-10f && inv_MTH3D_fn_xNormXYZ(&stVarVector1) > 1e-10f &&
        (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) || !p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform) && !(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10)) {
        MEC_fn_vBaseObstacleManagement(p_stDynamic, hSupObj, xDT);
        if (g_stReport.m_ulCurrSurfaceState & 1) {
            p_stGroundObstacle = &g_stReport.m_stGround;
            xSlide = MEC_fn_xGetSlide(p_stDynamic, p_stGroundObstacle);
            if (xSlide == 0.0f)
                p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x8000;
        }
        return;
    }

    xSpeedZ = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ * xDT;
    xFactor = xVar2 * xVar4;
    if (xFactor > inv_MTH_fn_xNegate(0.45f))
        xFactor = inv_MTH_fn_xNegate(0.45f);
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10)
        if (xFactor > xSpeedZ)
            xFactor = xSpeedZ;

    p_stVarVector3 = (MTH3D_tdstVector_ *)&p_stPOS_DownPosition->stMatrix.stCol_3;
    MTH3D_M_vAddXYZ_inv(&stVarVector4, p_stVarVector2, &stTangentMove);
    inv_MEC_fn_vSetTranslation4D(p_stPOS_DownPosition, &stVarVector4);
    MTH3D_M_vKTimesXYZ_inv(&stDownVector, xFactor, p_stNormal);
    inv_POS_fn_vTranslate(p_stPOS_DownPosition, &stDownVector);

    lObstacle2 = 0;
    if (g_bHighSpeed) {
        MTH3D_M_vAddXYZ_inv(&stVarVector4, p_stVarVector2, &stTangentMove);
        inv_MEC_fn_vSetTranslation4D(p_stPOS_UpPosition, &stVarVector4);
        if (MEC_fn_bCollision(&lObstacle2, g_a_stObstacle, p_stDynamic, hSupObj, hPreviousMatrix, p_stPOS_UpPosition, 1)) {
            for (lCounter = 0; lCounter < lObstacle2; lCounter++) {
                p_stTmpObstacle = g_a_stObstacle + lCounter;
                if (inv_MEC_fn_ucIsValidContact(p_stTmpObstacle)) {
                    if (!inline_00131DA8_4(p_stDynamic, p_stTmpObstacle))
                        p_stTmpObstacle->m_xCollidedEntity = 99;
                }
            }
        }
    }

    xFactor = xVar1 * xVar4;
    if (xFactor < 0.45f)
        xFactor = 0.45f;

    MTH3D_M_vAddXYZ_inv(&stVarVector4, p_stVarVector2, &stTangentMove);
    inv_MEC_fn_vSetTranslation4D(p_stPOS_UpPosition, &stVarVector4);
    MTH3D_M_vKTimesXYZ_inv(&stUpVector, xFactor, p_stNormal);
    inv_POS_fn_vTranslate(p_stPOS_UpPosition, &stUpVector);

    D_00393E10 = 75 - lObstacle2;
    MEC_fn_bCollision(&lObstacle, g_a_stObstacle + lObstacle2, p_stDynamic, hSupObj, p_stPOS_UpPosition,
                  p_stPOS_DownPosition, 1);
    D_00393E10 = 75;
    lObstacle += lObstacle2;

    if (lObstacle) {
        p_stGroundObstacle = MEC_fn_pSelectGround(p_stDynamic, lObstacle - lObstacle2, g_a_stObstacle + lObstacle2,
                                           &stTangentMove, &bFirstGround, &p_stVarObstacle1, &bHasStep);

        if (!bHasStep && g_lCallCounter == 1)
            MEC_fn_vEdgeAnalysis(p_stDynamic, lObstacle - lObstacle2, g_a_stObstacle + lObstacle2,
                          p_stVarObstacle1 ? p_stVarObstacle1 : p_stGroundObstacle);

        if (bFirstGround)
            MEC_fn_vDeleteWallEdge(lObstacle, g_a_stObstacle, p_stDynamic);
        if (p_stVarObstacle1)
            MEC_fn_vDeleteUnderGroundWall(lObstacle, g_a_stObstacle, p_stDynamic, p_stVarObstacle1);
        else
            MEC_fn_vDeleteUnderGroundWall(lObstacle, g_a_stObstacle, p_stDynamic, p_stGroundObstacle);
        MEC_fn_vDeleteOnGroundBackFace(lObstacle, g_a_stObstacle, p_stDynamic, &stAbsoluteTangentMove, p_stGroundObstacle);

        if (MEC_fn_bMainActor(hSupObj) && g_lCallCounter == 1)
            MEC_fn_vCrashAnalysis(p_stDynamic, lObstacle, g_a_stObstacle);

        p_stWallObstacle = 0;
        lObstacleCounter = 0;

        if (lObstacle2) {
            for (lCounter = 0; lCounter < lObstacle2; lCounter++) {
                p_stTmpObstacle = g_a_stObstacle + lCounter;
                if (inv_MEC_fn_ucIsValidContact(p_stTmpObstacle)) {
                    if (inline_00131DA8_3(p_stTmpObstacle) && !inline_00131DA8_5(p_stDynamic, p_stTmpObstacle))
                        bForceAdjust = 1;
                }
            }
        }

        for (lCounter = 0; lCounter < lObstacle; lCounter++) {
            p_stTmpObstacle = g_a_stObstacle + lCounter;
            if (inv_MEC_fn_ucIsValidContact(p_stTmpObstacle)) {
                if (inline_00131DA8_5(p_stDynamic, p_stTmpObstacle)) {
                    if (p_stGroundObstacle == p_stTmpObstacle)
                        lObstacleCounter = lCounter;
                } else if (inline_00131DA8_4(p_stDynamic, p_stTmpObstacle)) {
                    MTH3D_M_vSubXYZ_inv(&stVector, &p_stTmpObstacle->contact.m_stContact,
                                        p_stVarVector2);
                    xTmpReal = stVector.xX * stVector.xX + stVector.xY * stVector.xY;
                    if (xTmpReal < xWallDistance) {
                        p_stWallObstacle = p_stTmpObstacle;
                        lObstacleCounter = lCounter;
                        xWallDistance = xTmpReal;
                    }
                } else if (!p_stWallObstacle) {
                    p_stWallObstacle = p_stTmpObstacle;
                    lObstacleCounter = lCounter;
                }
            }
        }

        MEC_fn_vReportCollision(p_stDynamic, hSupObj, lObstacle, g_a_stObstacle, lObstacleCounter);

        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x40) && (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x200000) && !p_stWallObstacle) {
            MTH3D_M_vAddXYZ_inv(&stVarVector4, p_stVarVector2, &stTangentMove);
            inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stVarVector4);
            MEC_fn_vComputeSpeed(p_stDynamic, xDT);
            return;
        }

        if (p_stGroundObstacle) {
            xSlide = MEC_fn_xGetSlide(p_stDynamic, p_stGroundObstacle);
            if (xSlide == 0.0f)
                p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x8000;

            if (!p_stWallObstacle) {
                MTH3D_M_vAddXYZ_inv(&stVector, p_stVarVector3,
                                    &p_stGroundObstacle->m_stTranslation);
                inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stVector);

                if (g_lCallCounter == 1 && (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) &&
                    !(p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState & 4))
                    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x800000;
            } else {
                if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x800000) &&
                    ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) && !(p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState & 4))) {
                    bForceAdjust = 1;
                    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x800000;
                }

                p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x02000000;

                if (p_stWallObstacle &&
                    ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x10) || ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) && p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform) ||
                     bForceAdjust) &&
                    inline_00131DA8_3(p_stWallObstacle) && xSlide == 0.0f) {
                    bAdjusted = MEC_fn_vComputeAdjustVector(&stAdjustVector, p_stDynamic, lObstacle - lObstacle2, g_a_stObstacle + lObstacle2, lObstacle - lObstacle2);
                    if (lObstacle2) {
                        bAdjusted |= MEC_fn_vComputeAdjustVector(&stAdjustVector1, p_stDynamic, lObstacle2, g_a_stObstacle, lObstacle2);
                    } else {
                        stAdjustVector1.xZ = stAdjustVector1.xY = stAdjustVector1.xX = 0.0f;
                    }

                    if (!lObstacle2 || inv_MEC_fn_ucClimbIsNullVector(&stAdjustVector) || inv_MEC_fn_ucClimbIsNullVector(&stAdjustVector1)) {
                        MTH3D_M_vAddXYZ_inv(&stAdjustVector, &stAdjustVector, &stAdjustVector1);
                    } else {
                        f32 xVar5 = inv_MTH3D_fn_xNormXYZ(&stAdjustVector);
                        if (xVar5 != 0.0f) {
                            f32 xInv = 1.0f / xVar5;
                            MTH3D_M_vKTimesXYZ_inv(&stVector, xInv, &stAdjustVector);
                        } else {
                            stVector = stAdjustVector;
                        }
                        xVar3 = inv_MTH3D_fn_xDotXYZ(&stAdjustVector1, &stVector);
                        MTH3D_M_vKTimesXYZ_inv(&stVector, xVar3, &stVector);
                        MTH3D_M_vSubXYZ_inv(&stAdjustVector1, &stAdjustVector1, &stVector);
                        MTH3D_M_vAddXYZ_inv(&stAdjustVector, &stAdjustVector, &stAdjustVector1);
                    }

                    if (!(p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100010)) {
                        xVar3 = inv_MTH_fn_xMulAdd3(
                            stAdjustVector.xX, p_stGroundObstacle->contact.m_stNorm.xX,
                            stAdjustVector.xY, p_stGroundObstacle->contact.m_stNorm.xY,
                            stAdjustVector.xZ, p_stGroundObstacle->contact.m_stNorm.xZ);
                        MTH3D_M_vKTimesXYZ_inv(&stVector, xVar3,
                                               &p_stGroundObstacle->contact.m_stNorm);
                        MTH3D_M_vSubXYZ_inv(&stAdjustVector, &stAdjustVector, &stVector);
                        MTH3D_M_vAddXYZ_inv(&stAdjustVector, &stAdjustVector,
                                            &p_stGroundObstacle->m_stTranslation);

                        if (p_stWallObstacle->contact.m_stNorm.xZ > 0.5f) {
                            f32 xVar6;
                            f32 xVar7;
                            MTH3D_M_vSubXYZ_inv(&stVarVector4,
                                                &p_stWallObstacle->contact.m_stContact,
                                                &p_stGroundObstacle->contact.m_stContact);
                            xVar6 = inv_MTH_fn_xMulAdd3(p_stWallObstacle->contact.m_stNorm.xX,
                                             p_stGroundObstacle->contact.m_stNorm.xX,
                                             p_stWallObstacle->contact.m_stNorm.xY,
                                             p_stGroundObstacle->contact.m_stNorm.xY,
                                             p_stWallObstacle->contact.m_stNorm.xZ,
                                             p_stGroundObstacle->contact.m_stNorm.xZ);
                            xVar6 = acosf(xVar6);
                            xVar7 = stVarVector4.xX;
                            xVar7 = inv_MTH_fn_xMulAdd3(
                                xVar7, p_stGroundObstacle->contact.m_stNorm.xX, stVarVector4.xY,
                                p_stGroundObstacle->contact.m_stNorm.xY, stVarVector4.xZ,
                                p_stGroundObstacle->contact.m_stNorm.xZ);
                            xVar7 = xVar7 / tanf(xVar6);
                            xVar3 = inv_MTH_fn_xMulAdd3(p_stWallObstacle->contact.m_stNorm.xX,
                                             p_stGroundObstacle->contact.m_stNorm.xX,
                                             p_stWallObstacle->contact.m_stNorm.xY,
                                             p_stGroundObstacle->contact.m_stNorm.xY,
                                             p_stWallObstacle->contact.m_stNorm.xZ,
                                             p_stGroundObstacle->contact.m_stNorm.xZ);
                            MTH3D_M_vKTimesXYZ_inv(&stVector, xVar3,
                                                   &p_stGroundObstacle->contact.m_stNorm);
                            MTH3D_M_vSubXYZ_inv(&stVector, &p_stWallObstacle->contact.m_stNorm,
                                                &stVector);
                            if (!inv_MEC_fn_ucClimbIsNullVector(&stVector)) {
                                f32 xVar5 = inv_MTH3D_fn_xNormXYZ(&stVector);
                                if (xVar5 != 0.0f) {
                                    f32 xInv = 1.0f / xVar5;
                                    MTH3D_M_vKTimesXYZ_inv(&stVector, xInv, &stVector);
                                }
                            } else {
                                stVector.xZ = stVector.xY = stVector.xX = 0.0f;
                            }
                            MTH3D_M_vKTimesXYZ_inv(&stVector, xVar7, &stVector);
                            MTH3D_M_vAddXYZ_inv(&stAdjustVector, &stAdjustVector, &stVector);
                        }
                    }

                    MTH3D_M_vAddXYZ_inv(&stSafeTranslation, p_stVarVector3, &stAdjustVector);
                    MTH3D_M_vSubXYZ_inv(&stVector, &stSafeTranslation, p_stVarVector2);
                    MTH3D_M_vSubXYZ_inv(&stMove, &stMove, &stVector);
                } else {
                    stSafeTranslation = *p_stVarVector2;
                }

                inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stSafeTranslation);

                MEC_fn_vWallObstacleManagement(&stWallMove, p_stDynamic, &stMove, p_stNormal, &p_stWallObstacle->contact.m_stNorm, xDT, 0);

                if (bAdjusted && !((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) && p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform)) {
                    MTH3D_M_vSubXYZ_inv(&stVector, &stMove, &stWallMove);
                    if (!inv_MEC_fn_ucClimbIsNullVector(&stWallMove))
                        MEC_fn_vGroundObstacleManagement(p_stDynamic, hSupObj, xDT);
                    else
                        inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stSafeTranslation);
                } else {
                    if (MEC_fn_bCollision(&lObstacle, g_a_stObstacle, p_stDynamic, hSupObj, hCurrentMatrix, hCurrentMatrix, 1)) {
                        MEC_fn_vDeleteBaseBackFace(lObstacle, g_a_stObstacle, p_stDynamic);
                        MEC_fn_vDeleteNullTranslationObstacle(lObstacle, g_a_stObstacle);
                        MEC_fn_vComputeAdjustVector(&stAdjustVector, p_stDynamic, lObstacle, g_a_stObstacle, lObstacle);
                        inv_POS_fn_vTranslate(hCurrentMatrix, &stAdjustVector);
                    }
                }
                MEC_fn_vComputeSpeed(p_stDynamic, xDT);
                return;
            }
        } else if (p_stWallObstacle) {
            MEC_fn_vComputeAdjustVector(&stAdjustVector, p_stDynamic, lObstacle - lObstacle2, g_a_stObstacle + lObstacle2,
                          lObstacle - lObstacle2);
            if (lObstacle2) {
                MEC_fn_vComputeAdjustVector(&stAdjustVector1, p_stDynamic, lObstacle2, g_a_stObstacle, lObstacle2);
            } else {
                stAdjustVector1.xZ = stAdjustVector1.xY = stAdjustVector1.xX = 0.0f;
            }

            if (!lObstacle2 || inv_MEC_fn_ucClimbIsNullVector(&stAdjustVector) || inv_MEC_fn_ucClimbIsNullVector(&stAdjustVector1)) {
                MTH3D_M_vAddXYZ_inv(&stAdjustVector, &stAdjustVector, &stAdjustVector1);
            } else {
                f32 xVar5 = inv_MTH3D_fn_xNormXYZ(&stAdjustVector1);
                if (xVar5 != 0.0f) {
                    f32 xInv = 1.0f / xVar5;
                    MTH3D_M_vKTimesXYZ_inv(&stVector, xInv, &stAdjustVector1);
                } else {
                    stVector = stAdjustVector1;
                }
                xVar3 = inv_MTH3D_fn_xDotXYZ(&stAdjustVector, &stVector);
                if (xVar3 < inv_MTH3D_fn_xNormXYZ(&stAdjustVector1)) {
                    MTH3D_M_vKTimesXYZ_inv(&stVector, xVar3, &stVector);
                    MTH3D_M_vSubXYZ_inv(&stAdjustVector, &stAdjustVector, &stVector);
                    MTH3D_M_vAddXYZ_inv(&stAdjustVector, &stAdjustVector, &stAdjustVector1);
                }
            }

            MTH3D_M_vAddXYZ_inv(&stSafeTranslation, p_stVarVector3, &stAdjustVector);
            MTH3D_M_vSubXYZ_inv(&stVector, &stSafeTranslation, p_stVarVector2);
            MTH3D_M_vSubXYZ_inv(&stMove, &stMove, &stVector);

            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x02000000;

            inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stSafeTranslation);

            MEC_fn_vWallObstacleManagement(&stWallMove, p_stDynamic, &stMove, p_stNormal, &p_stWallObstacle->contact.m_stNorm, xDT, 0);

            if (!inv_MEC_fn_ucClimbIsNullVector(&stWallMove))
                MEC_fn_vGroundObstacleManagement(p_stDynamic, hSupObj, xDT);
            else
                inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stSafeTranslation);
        } else {
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x02000000;

            if (inv_MTH3D_fn_xNormXYZ(&stVarVector1) > 0.9f && stVarVector1.xZ < 0.0f)
                *hCurrentMatrix = *p_stPOS_DownPosition;

            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x200000) {
                MTH3D_M_vAddXYZ_inv(&stVarVector4, p_stVarVector2, &stTangentMove);
                inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stVarVector4);
            }

            if (g_lCallCounter > 1)
                *hCurrentMatrix = *hPreviousMatrix;

            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x10000;
        }

        MEC_fn_vComputeSpeed(p_stDynamic, xDT);
        {
        f32 xVar8;
        xVar8 = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX, p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY,
                           p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ);
        if (xVar8 < 0.005f && xSlide == 0.0f) {
            hCurrentMatrix->stMatrix.stCol_3 = hPreviousMatrix->stMatrix.stCol_3;
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY = p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        }
        }
        return;
    }

    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x02000000;

    if (inv_MTH3D_fn_xNormXYZ(&stVarVector1) > 0.9f && stVarVector1.xZ < 0.0f)
        *hCurrentMatrix = *p_stPOS_DownPosition;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x200000) {
        MTH3D_M_vAddXYZ_inv(&stVarVector4, p_stVarVector2, &stTangentMove);
        inv_MEC_fn_vSetTranslation4D(hCurrentMatrix, &stVarVector4);
    }

    if (g_lCallCounter > 1)
        *hCurrentMatrix = *hPreviousMatrix;

    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x10000;
    MEC_fn_vComputeSpeed(p_stDynamic, xDT);
}

void MEC_fn_vGroundGuess(DNM_stDynamics* p_stDynamic, f32 xDT)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ projection;
    MTH3D_tdstVector_ position;
    f32 dot;
    f32 inverse;
    f32 length;
    f32 x;
    f32 y;
    f32 z;
    MTH3D_tdstVector_* direction = &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stGroundNormal;
    MTH3D_tdstVector_* next;
    MTH3D_tdstVector_* current;
    POS_tdstMatrix_* nextMatrix;

    if (MTH3D_M_bIsZeroXYZ_inv(direction)) {
        return;
    }

    x = direction->xX;
    y = direction->xY;
    z = direction->xZ;

    next =
        (MTH3D_tdstVector_*) &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    current =
        (MTH3D_tdstVector_*) &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;

    length = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    length = MTH_fn_xSqrtOpt2(length);

    nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;

    inv_MTH3D_fn_vSubXYZ(&delta, next, current);

    dot = inv_MTH_fn_xMulAdd3Grouped(delta.xX, delta.xY, delta.xZ, x, y, z);

    if (length != 0.0f) {
        inverse = 1.0f / length;

        MTH3D_M_vSetXYZ_inv(&projection, inverse * x, inverse * y, inverse * z);
    } else {
        projection = *direction;
    }

    inv_MTH3D_fn_vKTimesXYZ(&projection, dot, &projection);

    inv_MTH3D_fn_vSubXYZ(&delta, &delta, &projection);

    inv_MTH3D_fn_vAddXYZ(&position, current, &delta);

    inv_POS_fn_vSetTranslation(nextMatrix, &position);
    MEC_fn_vComputeSpeed(p_stDynamic, xDT);
}

static inline void inv_MEC_fn_vSelectAxes(MTH3D_tdstVector_** y, MTH3D_tdstVector_** x,
                              MTH3D_tdstVector_* xVector, MTH3D_tdstVector_* yVector)
{
    *x = xVector;
    *y = yVector;
}

void MEC_fn_vSpiderObstacleManagement(DNM_stDynamics* dynamics, void* context, f32 deltaTime)
{
    MTH3D_tdstVector_ correction;
    POS_tdstMatrix_ probes[2];
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ x;
    MTH3D_tdstVector_ y;
    MTH3D_tdstVector_ z;
    MTH3D_tdstVector_ side;
    MTH3D_tdstVector_ position;
    POS_tdstMatrix_* current;
    POS_tdstMatrix_* reference;
    POS_tdstMatrix_* above;
    POS_tdstMatrix_* below;
    MTH4D_tdstVector_* currentPosition;
    MTH4D_tdstVector_* referencePosition;
    MTH4D_tdstVector_* abovePosition;
    MTH4D_tdstVector_* belowPosition;
    MTH3D_tdstVector_* zAxis;
    MTH3D_tdstVector_* xAxis;
    MTH3D_tdstVector_* yAxis;
    DNM_stMecObstacle* contact;
    s32 count;
    s32 i;
    f32 tangent;
    f32 scale;
    f32 half;
    f32 one;

    current = &dynamics->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    reference = &dynamics->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    above = &probes[0];
    below = &probes[1];
    *above = *current;
    *below = *current;

    currentPosition = &current->stMatrix.stCol_3;
    referencePosition = &reference->stMatrix.stCol_3;
    zAxis = &z;
    abovePosition = &above->stMatrix.stCol_3;
    belowPosition = &below->stMatrix.stCol_3;

    MTH3D_M_vSubXYZ_inv(&delta, currentPosition, referencePosition);

    POS_fn_vGetRotationVector(current, zAxis);
    tangent = tanf(1.0471976f);
    scale = tangent * MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ));
    half = 0.5f;
    if (scale < half) {
        scale = half;
    }

    inv_MTH3D_fn_vKTimesXYZ(&correction, scale, &z);
    MTH3D_M_vAddXYZ_inv(&side, currentPosition, &correction);
    if (above->ucType == MAT_eTransform_Identity) {
        above->ucType = MAT_eTransform_Translation;
    }
    (*(MTH3D_tdstVector_*) abovePosition) = side;
    one = 1.0f;
    abovePosition->xW = one;

    inv_MTH3D_fn_vKTimesXYZ(&correction, scale, &z);
    MTH3D_M_vSubXYZ_inv(&side, currentPosition, &correction);
    if (below->ucType == MAT_eTransform_Identity) {
        below->ucType = MAT_eTransform_Translation;
    }
    (*(MTH3D_tdstVector_*) belowPosition) = side;
    belowPosition->xW = one;

    if (MEC_fn_bCollision(&count, g_a_stObstacle, dynamics, context, above, below, 1)) {
        MEC_fn_vComputeAdjustVector(&correction, dynamics, count, g_a_stObstacle, count);
        MTH3D_M_vAddXYZ_inv(&position, belowPosition, &correction);
        if (current->ucType == MAT_eTransform_Identity) {
            current->ucType = MAT_eTransform_Translation;
        }
        (*(MTH3D_tdstVector_*) currentPosition) = position;
        currentPosition->xW = one;

        contact = NULL;
        i = 0;
        while (i < count && contact == NULL) {
            DNM_stMecObstacle* candidate = &g_a_stObstacle[i++];

            contact = candidate->m_xCollidedEntity == CONTACT_TAG_IGNORED ? contact : candidate;
        }

        inv_MEC_fn_vSelectAxes(&yAxis, &xAxis, &x, &y);
        POS_fn_vGetRotationVectors(current, xAxis, yAxis, zAxis);
        DNM_p_stVectorCrossProduct(&side, &y, &z);
        z = contact->contact.m_stNorm;
        DNM_p_stVectorCrossProduct(&y, &z, &side);
        DNM_p_stVectorCrossProduct(&x, &y, &z);

        inv_POS_fn_vInitMatrixFromVectors(current, MATRIX_INIT_ORTHONORMAL, xAxis, yAxis, zAxis,
                                          &(*(MTH3D_tdstVector_*) currentPosition));
        POS_fn_vSetScaleMatrix(current, &dynamics->m_tdstDynamicsBaseBlock.m_stScale);
        MEC_fn_vReportCollision(dynamics, context, count, g_a_stObstacle, count - 1);
        MEC_fn_vComputeSpeed(dynamics, deltaTime);
        return;
    }
    MEC_fn_vComputeSpeed(dynamics, deltaTime);
}

static inline u8 inv_MEC_fn_ucBothMaterialsFlaggedByte(DNM_stMecObstacle* c)
{
    s32 bResult = 0;
    if (c->contact.hCollidedMaterial->collideMaterial.xIdentifier & 0x4000) {
        if (c->contact.hMyMaterial->collideMaterial.xIdentifier & 0x4000)
            bResult = 1;
    }
    return bResult;
}

static inline s32 inv_MEC_fn_bIsNullVector(MTH3D_tdstVector_* v)
{
    s32 bResult = 0;
    if (inv_MTH_fn_xAbsoluteTied(v->xX - v->xZ) < 0.001f &&
        __builtin_fabsf(v->xY - v->xZ) < 0.001f && __builtin_fabsf(v->xZ) < 0.001f)
        bResult = 1;
    return bResult;
}

void MEC_fn_vSwimObstacleManagement(DNM_stDynamics* obj, void* ctx, f32 dt)
{
    POS_tdstMatrix_ local;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ response;
    MTH3D_tdstVector_ responseUnit;
    MTH3D_tdstVector_ deltaUnit;
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ reboundVec;
    MTH3D_tdstVector_ slideVec;
    MTH3D_tdstVector_ correction;
    MTH3D_tdstVector_ stProjection;
    MTH3D_tdstVector_ posNew;
    MTH3D_tdstVector_ funcOut;
    MTH3D_tdstVector_ normalSum;
    MTH3D_tdstVector_ normalCopy;
    MTH3D_tdstVector_ up;
    s32 count;
    POS_tdstMatrix_* state2b;
    POS_tdstMatrix_* optional;
    MTH3D_tdstVector_* previousPos;
    s32 allType78;
    s32 hasType78;
    s32 hasSpecial;
    s32 clamped;
    s32 firstIndex;
    s32 candidateIndex;
    POS_tdstMatrix_* state2;
    POS_tdstMatrix_* start;
    MTH3D_tdstVector_* currentPos;
    MTH4D_tdstVector_* state2Pos;
    MTH3D_tdstVector_* normal;
    MTH3D_tdstVector_* savedNormal;
    DNM_stMecObstacle* first;
    DNM_stMecObstacle* candidate;
    DNM_stMecObstacle* c;
    s32 specialIndex;
    s32 i;
    f32 slide;
    f32 rebound;
    f32 proj;
    f32 deltaLen;
    f32 responseLen;
    f32 eps18;
    f32 scale;
    f32 tz;
    f32 limit;
    f32 xScalar;

    state2b = &obj->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    state2 = state2b;
    hasType78 = 0;
    hasSpecial = 0;
    clamped = 0;
    firstIndex = 0;
    candidateIndex = 0;
    if ((u32) fn_ucCollSetGetCollComputeFrequencySO(ctx) > 1) {
        MTH4D_tdstVector_* lp;
        start = &local;
        local = obj->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
        if (start->ucType == MAT_eTransform_Identity)
            start->ucType = MAT_eTransform_Translation;
        *inv_POS_fn_p_stPosMatrixTranslation(start) = obj->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation;
        lp = &start->stMatrix.stCol_3;
        lp->xW = 1.0f;
        state2b = state2;
    } else {
        start = &obj->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    }

    if (MEC_fn_bCollision(&count, g_a_stObstacle, obj, ctx, start, state2b,
                          (u8) (((s32) obj->m_tdstDynamicsBaseBlock.ulEndFlags >> 22) & 1))) {
        if (obj->m_tdstDynamicsBaseBlock.ulEndFlags & 4)
            optional = &obj->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix;
        currentPos = inv_POS_fn_p_stPosMatrixTranslation(&obj->m_tdstDynamicsBaseBlock.m_stCurrentMatrix);
        {
            f32 zero = 0.0f;
            previousPos = inv_POS_fn_p_stPosMatrixTranslation(&obj->m_tdstDynamicsBaseBlock.m_stPreviousMatrix);
            MTH3D_M_vSetXYZ_inv(&normalSum, zero, zero, zero);
            first = 0;
            candidate = 0;
            specialIndex = 0;
        }

        i = 0;
        if (count > 0) {
            do {
                s32 t;
                c = &g_a_stObstacle[i];
                t = c->m_xCollidedEntity;
                if ((t ^ 0x63) != 0 && (u8) MEC_fn_bGroundNormal(obj, &c->contact.m_stNorm)) {
                    candidate = c;
                    candidateIndex = i;
                }
                i++;
            } while (i < count && candidate == 0);
        }

        if (candidate != 0 && ((g_stReport.m_ulCurrSurfaceState >> 6) & 1) == 0) {
            MEC_fn_vGroundObstacleManagement(obj, ctx, dt);
            g_stReport.m_ulCurrSurfaceState &= 0xFFFFFFBF;
            return;
        }

        allType78 = 1;
        i = 0;
        if (count > 0) {
            do {
                s32 t;
                c = &g_a_stObstacle[i];
                t = c->m_xCollidedEntity;
                if ((t ^ 0x63) != 0) {
                    firstIndex = i;
                    first = c;
                }
                i++;
            } while (i < count && first == 0);
        }

        for (i = 0; i < count; i++) {
            {
                s32 special;
                s32 t;
                c = &g_a_stObstacle[i];
                special = c->contact.hCollidedMaterial->collideMaterial.xIdentifier & 0x4000;
                special = special != 0;
                if (special)
                    hasSpecial = 1;
                t = c->m_xCollidedEntity;
                if ((t ^ 0x63) != 0) {
                    s32 both = 0;
                    s32 pick;
                    if (special) {
                        if (c->contact.hMyMaterial->collideMaterial.xIdentifier & 0x4000)
                            both = 1;
                    }
                    pick = specialIndex ? specialIndex : i;
                    if (both)
                        specialIndex = pick;
                    if ((u16) (c->m_xCollidedEntity - 7) >= 2)
                        allType78 = 0;
                    else
                        hasType78 = 1;
                }
            }
        }

        if (first == 0)
            return;

        if (candidate != 0 && firstIndex < candidateIndex)
            firstIndex = candidateIndex;
        if (firstIndex < specialIndex)
            firstIndex = specialIndex;

        MEC_fn_vReportCollision(obj, ctx, count, g_a_stObstacle, firstIndex);
        normal = &first->contact.m_stNorm;

        if (allType78) {
            savedNormal = normal;
            normal = &normalSum;
            for (i = 0; i < count;) {
                s32 t;
                c = &g_a_stObstacle[i];
                t = c->m_xCollidedEntity;
                i++;
                if ((t ^ 0x63) != 0) {
                    MTH3D_M_vAddXYZ_inv(normal, normal, &c->contact.m_stNorm);
                }
            }
            if (!(MTH3D_M_bIsZeroXYZ_inv(normal))) {
                f32 len;
                {
                    f32 dd;
                    dd = inv_MTH_fn_xMulAdd3Grouped(normal->xX, normal->xY, normal->xZ,
                                          normal->xX, normal->xY, normal->xZ);
                    len = MTH_fn_xSqrtOpt2(dd);
                }
                if (len != 0.0f) {
                    f32 xScalar = 1.0f / len;
                    inv_MTH3D_fn_vKTimesXYZ(normal, xScalar, normal);
                }
            } else {
                normal = savedNormal;
            }
        }
        state2Pos = &state2->stMatrix.stCol_3;

        slide = MEC_fn_xGetSlide(obj, first);
        rebound = MEC_fn_xGetRebound(obj, first);

        {
            tz = 0.0f;
            MTH3D_M_vSetXYZ_inv(&responseUnit, tz, tz, tz);
            MTH3D_M_vSetXYZ_inv(&funcOut, tz, tz, tz);
            up.xX = tz;
            up.xY = tz;
        }
        up.xZ = 1.0f;
        eps18 = 0.001f;

        inv_MTH3D_fn_vSubXYZ(&delta, currentPos, previousPos);
        {
            f32 dd;
            dd = inv_MTH_fn_xMulAdd3Grouped(delta.xX, delta.xY, delta.xZ, delta.xX, delta.xY,
                                            delta.xZ);
            deltaLen = MTH_fn_xSqrtOpt2(dd);
        }

        {
            f32 dot;
            dot = inv_MTH_fn_xMulAdd3Grouped(delta.xX, delta.xY, delta.xZ,
                                   normal->xX, normal->xY, normal->xZ);
            proj = -dot;
        }

        inv_MTH3D_fn_vKTimesXYZ(&correction, proj, normal);

        slideVec.xY = delta.xY + correction.xY;
        slideVec.xX = delta.xX + correction.xX;
        slideVec.xZ = delta.xZ + correction.xZ;
        reboundVec.xX = slideVec.xX + correction.xX;
        reboundVec.xY = slideVec.xY + correction.xY;
        correction.xX = rebound * reboundVec.xX;
        correction.xY = rebound * reboundVec.xY;
        response.xX = slide * slideVec.xX + correction.xX;
        response.xY = slide * slideVec.xY + correction.xY;
        reboundVec.xZ = slideVec.xZ + correction.xZ;
        correction.xZ = rebound * reboundVec.xZ;
        response.xZ = slide * slideVec.xZ + correction.xZ;

        {
            f32 dd;

            dd = inv_MTH_fn_xMulAdd3Grouped(response.xX, response.xY, response.xZ,
                                  response.xX, response.xY, response.xZ);

            responseLen = MTH_fn_xSqrtOpt2(dd);
        }
        {
            s32 isNull = 0;
            if (inv_MTH_fn_xAbsoluteTied(response.xX - 0.0f) < eps18 &&
                inv_MTH_fn_xAbsoluteTied(response.xY - 0.0f) < eps18 &&
                inv_MTH_fn_xAbsoluteTied(response.xZ - 0.0f) < eps18)
                isNull = 1;

            if (isNull & 2) {
                responseUnit.xZ = rebound;
                *&stProjection.xX = 0;
                *&stProjection.xY = 0;
                *&stProjection.xZ = 0;
                *&normalSum.xX = 0;
                *&normalSum.xY = 0;
                *&normalSum.xZ = 0;
                *&posNew.xX = 0;
                *&posNew.xY = 0;
            }
            if (!isNull) {
                xScalar = 1.0f / responseLen;
                inv_MTH3D_fn_vKTimesXYZ(&responseUnit, xScalar, &response);
            }
        }
        {
            s32 isNull = 0;
            if (inv_MTH_fn_xAbsoluteTied(delta.xX - 0.0f) < 0.001f &&
                inv_MTH_fn_xAbsoluteTied(delta.xY - 0.0f) < 0.001f &&
                inv_MTH_fn_xAbsoluteTied(delta.xZ - 0.0f) < 0.001f)
                isNull = 1;
            if (!isNull) {
                xScalar = 1.0f / deltaLen;
                inv_MTH3D_fn_vKTimesXYZ(&deltaUnit, xScalar, &delta);
            }
        }

        if (allType78) {
            proj = inv_MTH_fn_xMulAdd3(normal->xX, deltaUnit.xX, normal->xY,
                            deltaUnit.xY, normal->xZ, deltaUnit.xZ);
            if (proj < -0.866f) {
                if (state2->ucType == MAT_eTransform_Identity)
                    state2->ucType = MAT_eTransform_Translation;
                *inv_POS_fn_p_stPosMatrixTranslation(state2b) = *previousPos;
                state2Pos->xW = 1.0f;
                {
                    f32 z = 0.0f;
                    MTH3D_M_vSetXYZ_inv(&obj->m_tdstDynamicsBaseBlock.m_stPreviousSpeed, z, z, z);
                }
                return;
            }
        }

        slide = first->contact.m_xRate;
        MEC_fn_vComputeAdjustVector(&funcOut, obj, count, g_a_stObstacle, firstIndex);

        pos.xX = currentPos->xX + funcOut.xX;
        pos.xY = currentPos->xY + funcOut.xY;
        tz = currentPos->xZ + funcOut.xZ;
        pos.xZ = tz;

        if (lZWaterValidity < 10) {
            limit = g_xZWater - 1.2f;
            if (limit < tz) {
                clamped = 1;
                if (!inv_MEC_fn_ucBothMaterialsFlaggedByte(first)) {
                    f32 one;
                    f32 len;
                    xScalar = 0.0f;
                    one = 1.0f;
                    MTH3D_M_vSetXYZ_inv(&correction, xScalar, xScalar, one);
                    proj = inv_MTH_fn_xMulAdd3(first->contact.m_stNorm.xX, correction.xX,
                                    first->contact.m_stNorm.xY, correction.xY,
                                    first->contact.m_stNorm.xZ, correction.xZ);
                    MTH3D_M_vKTimesXYZ_inv(&stProjection, proj, &first->contact.m_stNorm);
                    MTH3D_M_vSetXYZ_inv(&correction, xScalar - stProjection.xX,
                                        xScalar - stProjection.xY, one - stProjection.xZ);
                    {
                        f32 dd;
                        dd = inv_MTH_fn_xMulAdd3Grouped(correction.xX, correction.xY, correction.xZ,
                                              correction.xX, correction.xY, correction.xZ);
                        proj = MTH_fn_xSqrtOpt2(dd);
                    }
                    if (proj != xScalar) {
                        f32 r = one / proj;
                        xScalar = r * r * (limit - tz);
                        inv_MTH3D_fn_vKTimesXYZ(&correction, xScalar, &correction);
                    } else {
                        MTH3D_M_vSetXYZ_inv(&correction, xScalar, xScalar, limit - tz);
                    }
                } else {
                    MTH3D_M_vSetXYZ_inv(&correction, 0.0f, 0.0f, limit - tz);
                }
                MTH3D_M_vAddXYZ_inv(&pos, &pos, &correction);
            }
        }

        if (slide >= 0.0f)
            scale = 1.0f - slide;
        else
            scale = 0.0f;

        inv_MTH3D_fn_vKTimesXYZ(&correction, scale, &response);

        if (clamped || hasSpecial) {
            {
                f32 dd;
                dd = inv_MTH_fn_xMulAdd3Grouped(correction.xX, correction.xY, correction.xZ,
                                      correction.xX, correction.xY, correction.xZ);
                proj = MTH_fn_xSqrtOpt2(dd);
            }
            *(s32*) &correction.xZ = 0;
            {
                s32 isNull = inv_MEC_fn_bIsNullVector(&correction);
                if (!isNull) {
                    f32 len;
                    {
                        f32 dd;
                        dd = inv_MTH_fn_xMulAdd3Grouped(correction.xX, correction.xY, correction.xZ,
                                              correction.xX, correction.xY, correction.xZ);
                        len = MTH_fn_xSqrtOpt2(dd);
                    }
                    if (len != 0.0f) {
                        f32 xScalar = 1.0f / len;
                        inv_MTH3D_fn_vKTimesXYZ(&correction, xScalar, &correction);
                    }
                    inv_MTH3D_fn_vKTimesXYZ(&correction, proj, &correction);
                }
            }
            response.xZ = 0.0f;
        }

        inv_MTH3D_fn_vAddXYZ(&posNew, &pos, &correction);
        pos = *previousPos;

        if (state2->ucType == MAT_eTransform_Identity)
            state2->ucType = MAT_eTransform_Translation;
        *inv_POS_fn_p_stPosMatrixTranslation(state2b) = posNew;
        state2Pos->xW = 1.0f;

        inv_MTH3D_fn_vSubXYZ(previousPos, currentPos, &response);

        if (obj->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
            if (optional->ucType == MAT_eTransform_Identity)
                optional->ucType = MAT_eTransform_Translation;
            *inv_POS_fn_p_stPosMatrixTranslation(optional) = *previousPos;
            optional->stMatrix.stCol_3.xW = 1.0f;
        }

        normalCopy = first->contact.m_stNorm;

        if (!hasType78) {
            if (MEC_fn_bCollision(&count, g_a_stObstacle, obj, ctx,
                                  &obj->m_tdstDynamicsBaseBlock.m_stPreviousMatrix, state2, 0)) {
                i = 0;
                if (count > 0) {
                    do {
                        s32 t;
                        c = &g_a_stObstacle[i];
                        t = c->m_xCollidedEntity;
                        if ((t ^ 0x63) != 0 && !inv_MEC_fn_ucBothMaterialsFlaggedByte(c)) {
                            s32 isNull = 0;
                            MTH4D_tdstVector_* sp;
                            f32 zc = 0.0f;
                            MTH3D_M_vSubXYZ_inv(&correction, &normalCopy, &c->contact.m_stNorm);
                            if (inv_MTH_fn_xAbsoluteTied(correction.xX - zc) < 0.001f &&
                                inv_MTH_fn_xAbsoluteTied(correction.xY - zc) < 0.001f &&
                                inv_MTH_fn_xAbsoluteTied(correction.xZ - zc) < 0.001f)
                                isNull = 1;
                            if (!isNull) {
                                if (state2->ucType == MAT_eTransform_Identity)
                                    state2->ucType = MAT_eTransform_Translation;
                                *inv_POS_fn_p_stPosMatrixTranslation(state2b) = pos;
                                sp = &state2->stMatrix.stCol_3;
                                sp->xW = 1.0f;
                                *previousPos = pos;
                            }
                        }
                        i++;
                    } while (i < count);
                }
            }
        }
        MEC_fn_vComputeSpeed(obj, dt);
    } else {
        obj->m_tdstDynamicsBaseBlock.ulEndFlags &= 0xFFFEFFFF;
        currentPos = inv_POS_fn_p_stPosMatrixTranslation(state2b);
        if (lZWaterValidity < 10) {
            limit = g_xZWater - 1.2f;
            if (limit < currentPos->xZ) {
                MTH3D_M_vSetXYZ_inv(&correction, 0.0f, 0.0f, limit - currentPos->xZ);
                inv_POS_fn_vTranslate(state2b, &correction);
            }
        }
    }
}
