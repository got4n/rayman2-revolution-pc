#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GMT/GMT.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

void POS_fn_vGetYAxisVector(void*, MTH3D_tdstVector_*);
void POS_fn_vGetRotationVector(void*, MTH3D_tdstVector_*);

f32 CAM_fn_xComputeHorizontalAngle(MTH3D_tdstVector_* _p_stOrgVector,
                                   MTH3D_tdstVector_* _p_stDestVector,
                                   MTH3D_tdstVector_* _p_stAxisZ)
{
    MTH3D_tdstVector_ stVect1;
    MTH3D_tdstVector_ stVect2;
    MTH3D_tdstVector_ stAxisZ;
    MTH_stRotation stRotation;
    f32 xAngle;
    f32 xNorm;

    stVect1 = *_p_stOrgVector;
    stVect2 = *_p_stDestVector;
    stAxisZ = *_p_stAxisZ;

    MTH_fn_vVectorPlaneProjection(&stVect1, &stVect1, &stAxisZ, &stAxisZ);
    {
        s32 bIsNull;
        bIsNull = 0;
        if (inv_MTH_fn_xAbsoluteTied(stVect1.xX - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsoluteTied(stVect1.xY - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsoluteTied(stVect1.xZ - 0.0f) < 0.001f) {
            bIsNull = 1;
        }
        if (bIsNull != 0) {
            return 0.0f;
        }
    }
    inv_MTH3D_fn_vNormalizeXYZ(&stVect1);

    MTH_fn_vVectorPlaneProjection(&stVect2, &stVect2, &stAxisZ, &stAxisZ);
    {
        s32 bIsNull;
        bIsNull = 0;
        if (inv_MTH_fn_xAbsoluteTied(stVect2.xX - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsoluteTied(stVect2.xY - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsoluteTied(stVect2.xZ - 0.0f) < 0.001f) {
            bIsNull = 1;
        }
        __asm__("");
        if (bIsNull != 0) {
            return 0.0f;
        }
    }
    inv_MTH3D_fn_vNormalizeXYZ(&stVect2);

    MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stVect1, &stVect2);
    xNorm = inv_MTH_fn_xMulAdd3(stRotation.m_stAxis.xX, stAxisZ.xX, stRotation.m_stAxis.xY,
                                stAxisZ.xY, stRotation.m_stAxis.xZ, stAxisZ.xZ);
    xAngle = stRotation.m_xAngle;
    if (xNorm < 0.0f) {
        xAngle = -xAngle;
    }
    return xAngle;
}

f32 CAM_fn_xComputeVerticalAngle(MTH3D_tdstVector_* _p_stOrgVector,
                                 MTH3D_tdstVector_* _p_stDestVector, MTH3D_tdstVector_* _p_stAxisZ)
{
    MTH3D_tdstVector_ stVector;
    MTH3D_tdstVector_ stAxisZ;
    MTH3D_tdstVector_ stProj;
    MTH_stRotation stRotation;
    f32 xOrgAngle;
    f32 xDestAngle;
    f32 xOrgEpsilon;
    f32 xDestEpsilon;
    f32 xNorm;
    s32 bIsNull;

    stAxisZ = *_p_stAxisZ;
    stVector = *_p_stOrgVector;

    xOrgEpsilon = 0.001f;
    xNorm = inv_MTH_fn_xMulAdd3Tied(stVector.xX, stAxisZ.xX, stVector.xY, stAxisZ.xY, stVector.xZ,
                                    stAxisZ.xZ);
    if (inv_MTH_fn_xAbsoluteTied(xNorm) < xOrgEpsilon) {
        xOrgAngle = 0.0f;
    } else {
        MTH_fn_vVectorPlaneProjection(&stProj, &stVector, &stAxisZ, &stAxisZ);
        bIsNull = 0;
        if (inv_MTH_fn_xAbsoluteTied(stProj.xX - 0.0f) < xOrgEpsilon &&
            inv_MTH_fn_xAbsoluteTied(stProj.xY - 0.0f) < xOrgEpsilon &&
            inv_MTH_fn_xAbsoluteTied(stProj.xZ - 0.0f) < xOrgEpsilon) {
            bIsNull = 1;
        }
        if (bIsNull == 0) {
            inv_MTH3D_fn_vNormalizeXYZ(&stVector);
            inv_MTH3D_fn_vNormalizeXYZ(&stProj);
            MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stVector, &stProj);
            xOrgAngle = stRotation.m_xAngle;
            MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stVector, &stAxisZ);
            if (stRotation.m_xAngle > MTH_HALF_PI_inv) {
                xOrgAngle = inv_MTH_fn_xNegateTied(xOrgAngle);
            }
        } else {
            xOrgAngle = 0.0f;
        }
    }

    stVector = *_p_stDestVector;
    xNorm = inv_MTH_fn_xMulAdd3Tied(stVector.xX, stAxisZ.xX, stVector.xY, stAxisZ.xY, stVector.xZ,
                                    stAxisZ.xZ);
    if (xNorm == 0.0f) {
        return 0.0f - xOrgAngle;
    }
    MTH_fn_vVectorPlaneProjection(&stProj, &stVector, &stAxisZ, &stAxisZ);
    xDestEpsilon = 0.001f;
    bIsNull = 0;
    if (inv_MTH_fn_xAbsoluteTied(stProj.xX - 0.0f) < xDestEpsilon &&
        inv_MTH_fn_xAbsoluteTied(stProj.xY - 0.0f) < xDestEpsilon &&
        inv_MTH_fn_xAbsoluteTied(stProj.xZ - 0.0f) < xDestEpsilon) {
        bIsNull = 1;
    }
    if (bIsNull == 0) {
        inv_MTH3D_fn_vNormalizeXYZ(&stVector);
        inv_MTH3D_fn_vNormalizeXYZ(&stProj);
        MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stVector, &stProj);
        xDestAngle = stRotation.m_xAngle;
        MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stVector, &stAxisZ);
        if (stRotation.m_xAngle > MTH_HALF_PI_inv) {
            xDestAngle = inv_MTH_fn_xNegateTied(xDestAngle);
        }
    } else {
        xDestAngle = 0.0f;
    }
    return xDestAngle - xOrgAngle;
}

void DNM_fn_vDynamicsTakeCareOfLastMove(MTH3D_tdstVector_* _p_stShiftingResult,
                                        MTH3D_tdstVector_* _p_stLastShifting, f32 _xCutAngleFactor)
{
    MTH3D_tdstVector_ stTempVector1;
    MTH3D_tdstVector_ stTempVector;
    MTH_stRotation stRotation;
    f32 xNorm;
    f32 lastNorm;
    f32 xInv;
    f32 x, y, z;
    f32 lastX, lastY, lastZ;

    if (_xCutAngleFactor == 1.0f) {
        return;
    }
    x = _p_stShiftingResult->xX;
    y = _p_stShiftingResult->xY;
    z = _p_stShiftingResult->xZ;
    xNorm = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z));
    lastX = _p_stLastShifting->xX;
    lastY = _p_stLastShifting->xY;
    lastZ = _p_stLastShifting->xZ;
    lastNorm = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(lastX, lastY, lastZ));
    if (!(xNorm > 0.1f)) {
        return;
    }
    if (!(lastNorm > 0.1f)) {
        return;
    }
    xInv = 1.0f / xNorm;
    MTH3D_M_vSetXYZ_inv(&stTempVector1, xInv * x, xInv * y, xInv * z);
    xInv = 1.0f / lastNorm;
    MTH3D_M_vSetXYZ_inv(&stTempVector, xInv * lastX, xInv * lastY, xInv * lastZ);
    MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stTempVector, &stTempVector1);
    stRotation.m_xAngle = stRotation.m_xAngle / _xCutAngleFactor;
    MTH_p_stRotationMulVector(&stRotation, &stTempVector, &stTempVector1);
    inv_MTH3D_fn_vNormalizeXYZ(&stTempVector1);
    inv_MTH3D_fn_vKTimesXYZ(_p_stShiftingResult, xNorm, &stTempVector1);
}

void DNM_fn_vDynamicsCameraPositionParsing(f32 _xSpeed, f32 _xIncSpeed, f32 _xDecSpeed,
                                           f32 _xLastNorm, s8 _cIgnoreParsing, s8 _cIgnoreInertia,
                                           MTH3D_tdstVector_* _p_stActualPos,
                                           MTH3D_tdstVector_* _p_stShiftingSrc,
                                           MTH3D_tdstVector_* _p_stDestPos,
                                           MTH3D_tdstVector_* _p_stShiftingResult)
{
    u8 bNormWantedIsModified;
    f32 xNormWanted;
    f32 xTmpNorm;
    f32 xNormSrc;
    f32 xNorm;
    f32 xScale;
    f32 xSqrNorm;

    xSqrNorm = MTH_fn_xSqrAddSqrAddSqrOpt2(_p_stShiftingSrc->xX, _p_stShiftingSrc->xY,
                                           _p_stShiftingSrc->xZ);
    bNormWantedIsModified = 0;
    xNorm = MTH_fn_xSqrtOpt2(xSqrNorm);
    xNormSrc = xNorm;
    if (_cIgnoreParsing || xNorm <= 0.001f) {
        inv_MTH3D_fn_vAddXYZ(_p_stDestPos, _p_stActualPos, _p_stShiftingSrc);
        *_p_stShiftingResult = *_p_stShiftingSrc;
        return;
    }

    if (_xSpeed == 0.0f) {
        *_p_stDestPos = *_p_stActualPos;
        MTH3D_M_vZeroXYZ_inv(_p_stShiftingSrc);
        return;
    }

    *_p_stShiftingResult = *_p_stShiftingSrc;
    if (_xSpeed < xNorm) {
        xNormSrc = _xSpeed;
        bNormWantedIsModified = 1;
    }
    if (!_cIgnoreInertia) {
        if (_xDecSpeed != 0.0f && xNorm < _xSpeed * _xDecSpeed) {
            xTmpNorm = xNorm / _xDecSpeed;
            if (0.001f < xTmpNorm && xTmpNorm < xNormSrc) {
                xNormSrc = xTmpNorm;
                bNormWantedIsModified = 1;
            }
        }
        if (_xIncSpeed != 0.0f) {
            xNormWanted = _xLastNorm + _xSpeed / _xIncSpeed;
            if (xNormWanted < xNormSrc) {
                xNormSrc = xNormWanted;
                bNormWantedIsModified = 1;
            }
        }
    }
    if (bNormWantedIsModified) {
        xScale = xNormSrc / xNorm;
        inv_MTH3D_fn_vKTimesXYZ(_p_stShiftingResult, xScale, _p_stShiftingResult);
    }
    inv_MTH3D_fn_vAddXYZ(_p_stDestPos, _p_stActualPos, _p_stShiftingResult);
}

f32 DNM_fn_xDynamicsCameraAngleParsing(f32 _xSpeed, f32 _xIncSpeed, f32 _xDecSpeed,
                                       f32 _xRequestAngle, f32 _xLastAngle, s8 _cIgnoreParsing,
                                       s8 _cIgnoreInertia)
{
    f32 xAngle;
    f32 xRetAngle;

    if (_cIgnoreParsing || _xRequestAngle < 0.001f)
        return _xRequestAngle;
    xAngle = _xRequestAngle;
    if (_xSpeed < xAngle)
        xAngle = _xSpeed;
    if (!_cIgnoreInertia) {
        if (_xRequestAngle < _xSpeed * _xDecSpeed) {
            xRetAngle = _xRequestAngle / _xDecSpeed;
            if (xRetAngle < xAngle)
                xAngle = xRetAngle;
        }
        xRetAngle = _xLastAngle + _xSpeed / _xIncSpeed;
        if (xRetAngle < xAngle)
            xAngle = xRetAngle;
    }
    return xAngle;
}

DNM_stDynamics* DNM_p_stDynamicsCameraParsing(DNM_stDynamics* _p_stDynamics,
                                              HIE_tdstSuperObject_* _h_SupObj,
                                              DNM_stParsingDatas* _p_stExternData, f32 _xDT)
{
    stMACDPID* p_stDNMMACDPID;
    DNM_stReport_* p_stReport;
    POS_tdstMatrix_* pGlobal;
    MTH3D_tdstVector_ stPreviousPos __attribute__((aligned(16)));
    MTH3D_tdstVector_ stShiftingSrc __attribute__((aligned(16)));
    MTH3D_tdstVector_ stShiftingResult __attribute__((aligned(16)));
    MTH3D_tdstVector_ stTempVector __attribute__((aligned(16)));
    MTH3D_tdstVector_ stYVector __attribute__((aligned(16)));
    f32 xSpeed;
    MTH3D_tdstVector_ stTempVector1 __attribute__((aligned(16)));
    MTH3D_tdstVector_ stTempVector2 __attribute__((aligned(16)));
    MTH_stRotation stRotation __attribute__((aligned(16)));
    MTH3D_tdstVector_ stDNMTempVector __attribute__((aligned(16)));
    f32 xNorm;
    f32 xAngle;
    f32 xZero;
    s32 bNull;

    p_stDNMMACDPID = &_p_stDynamics->m_tdstDynamicsComplexBlock.m_stExternalDatas;
    p_stReport = _p_stDynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    pGlobal = _h_SupObj->gu_hGlobalMatrix;

    stPreviousPos = *inv_POS_fn_p_stPosMatrixTranslation(pGlobal);
    stShiftingSrc = p_stDNMMACDPID->m_stData2;
    stTempVector = p_stReport->m_stAbsolutePrevSpeed.m_stLinear;
    xSpeed = p_stDNMMACDPID->m_stData6.m_stAxis.xX;
    stTempVector1 = _p_stExternData->stPosition;
    DNM_fn_vDynamicsCameraPositionParsing(
        xSpeed * _xDT, p_stDNMMACDPID->m_stData6.m_stAxis.xY, p_stDNMMACDPID->m_stData6.m_stAxis.xZ,
        MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector.xX, stTempVector.xY, stTempVector.xZ)),
        (s8) (p_stDNMMACDPID->m_uwData9 & 1), (s8) (p_stDNMMACDPID->m_uwData9 & 2), &stPreviousPos,
        &stShiftingSrc, &stTempVector1, &stShiftingResult);
    _p_stExternData->stPosition = stTempVector1;

    if (!(p_stDNMMACDPID->m_uwData9 & 4)) {
        xSpeed = p_stDNMMACDPID->m_stData7.m_stAxis.xX;
        xSpeed *= _xDT;
        MTH3D_M_vSubXYZ_inv(&stTempVector, &p_stDNMMACDPID->m_stData10, &stPreviousPos);
        bNull = 0;
        if (inv_MTH_fn_xAbsoluteTied(stTempVector.xX - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsoluteTied(stTempVector.xY - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsoluteTied(stTempVector.xZ - 0.0f) < 0.001f) {
            bNull = 1;
        }
        if (!bNull) {
            inv_MTH3D_fn_vNormalizeXYZ(&stTempVector);
        }

        stTempVector1 = p_stDNMMACDPID->m_stData10;
        stTempVector2 = _p_stExternData->stPosition;
        MTH3D_M_vSubXYZ_inv(&stTempVector1, &stTempVector1, &stTempVector2);
        xNorm = MTH_fn_xSqrtOpt2(
            MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector1.xX, stTempVector1.xY, stTempVector1.xZ));
        if (xNorm != 0.0f) {
            MTH3D_M_vKTimesXYZ_inv(&stTempVector1, 1.0f / xNorm, &stTempVector1);
        }

        MTH_p_stRotationComputeRotationWith2Vectors(&stRotation, &stTempVector, &stTempVector1);
        xAngle = DNM_fn_xDynamicsCameraAngleParsing(
            xSpeed, p_stDNMMACDPID->m_stData7.m_stAxis.xY, p_stDNMMACDPID->m_stData7.m_stAxis.xZ,
            stRotation.m_xAngle, _p_stExternData->xOutAlpha, (s8) (p_stDNMMACDPID->m_uwData9 & 4),
            (s8) (p_stDNMMACDPID->m_uwData9 & 8));
        _p_stExternData->xOutAlpha = xAngle;
        if (xAngle != 0.0f) {
            stRotation.m_xAngle = xAngle;
            MTH_p_stRotationMulVector(&stRotation, &stTempVector, &stTempVector);
        }
        MTH3D_M_vSetXYZ_inv(&stTempVector, inv_MTH_fn_xNegateTied(xNorm * stTempVector.xX),
                            inv_MTH_fn_xNegateTied(xNorm * stTempVector.xY),
                            inv_MTH_fn_xNegateTied(xNorm * stTempVector.xZ));
        MTH3D_M_vAddXYZ_inv(&_p_stExternData->stPosition, &stTempVector,
                            &p_stDNMMACDPID->m_stData10);
    }

    if ((*(u32*) &p_stDNMMACDPID->m_cData8 & 0x50000) != 0x50000) {
        inv_MTH3D_fn_vSubXYZ(&stTempVector, &_p_stExternData->stPosition, &stPreviousPos);
        DNM_fn_vDynamicsTakeCareOfLastMove(&stTempVector,
                                           &p_stReport->m_stAbsolutePrevSpeed.m_stLinear,
                                           p_stDNMMACDPID->m_xData11);
        inv_MTH3D_fn_vAddXYZ(&_p_stExternData->stPosition, &stTempVector, &stPreviousPos);
    }

    if (p_stDNMMACDPID->m_uwData9 & 0x80) {
        POS_fn_vGetYAxisVector(pGlobal, &stYVector);
        MTH3D_M_vSetXYZ_inv(&stYVector, inv_MTH_fn_xNegateTied(stYVector.xX),
                            inv_MTH_fn_xNegateTied(stYVector.xY),
                            inv_MTH_fn_xNegateTied(stYVector.xZ));
        stTempVector = *inv_POS_fn_p_stPosMatrixTranslation(pGlobal);
        xZero = 0.0f;
        MTH3D_M_vAddXYZ_inv(&_p_stExternData->stVector, &stYVector, &stTempVector);
        MTH3D_M_vSetXYZ_inv(&stShiftingResult, xZero, xZero, xZero);
        stDNMTempVector = stShiftingResult;
        p_stReport->m_stAbsolutePrevPosition.m_stLinear = stDNMTempVector;
    } else {
        stPreviousPos = _p_stExternData->stVector;
        MTH3D_M_vSubXYZ_inv(&stShiftingSrc, &p_stDNMMACDPID->m_stData1, &stPreviousPos);
        xSpeed = p_stDNMMACDPID->m_stData3.xX;
        stTempVector = p_stReport->m_stAbsolutePrevPosition.m_stLinear;
        stTempVector1 = _p_stExternData->stVector;
        DNM_fn_vDynamicsCameraPositionParsing(
            xSpeed * _xDT, p_stDNMMACDPID->m_stData3.xY, p_stDNMMACDPID->m_stData3.xZ,
            MTH_fn_xSqrtOpt2(
                MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector.xX, stTempVector.xY, stTempVector.xZ)),
            (s8) (p_stDNMMACDPID->m_uwData9 & 0x10), (s8) (p_stDNMMACDPID->m_uwData9 & 0x20),
            &stPreviousPos, &stShiftingSrc, &stTempVector1, &stShiftingResult);
        DNM_fn_vDynamicsTakeCareOfLastMove(&stShiftingResult,
                                           &p_stReport->m_stAbsolutePrevPosition.m_stLinear,
                                           p_stDNMMACDPID->m_xData13);
        MTH3D_M_vAddXYZ_inv(&_p_stExternData->stVector, &stShiftingResult, &stPreviousPos);
        p_stReport->m_stAbsolutePrevPosition.m_stLinear = stShiftingResult;
    }
    return _p_stDynamics;
}

s32 CAM_lHaveITakeSThgIntoTheMug(DNM_stMecObstacle* _p_stResultObstacle,
                                 HIE_tdstSuperObject_* _p_stSupObj,
                                 POS_tdstMatrix_* _p_stStartPosition,
                                 POS_tdstMatrix_* _p_stEndPosition)
{
    MTH3D_tdstVector_ stStartVec;
    MTH3D_tdstVector_ stTempVec;
    DNM_stMecObstacle* p_stObstacle;
    DNM_stMecObstacle* p_stReturnObstacle;
    f32 xDot;
    f32 xMoveDot;
    GMT_tdstGameMaterial_* hGameMat;
    GMT_tdstCollideMaterial_* hCollMat;
    s32 lCollision;
    s32 lCounter;
    s32 bWater;
    s32 bLessDetected;

    bLessDetected = 0;
    lCounter = 0;
    p_stReturnObstacle =
        (DNM_stMecObstacle*) ((u32) _p_stResultObstacle & ~(u32) _p_stResultObstacle);
    stStartVec = *inv_POS_fn_p_stPosMatrixTranslation(_p_stStartPosition);
    stTempVec = *inv_POS_fn_p_stPosMatrixTranslation(_p_stEndPosition);
    lCollision = inv_COL_fn_lTestCharacterAgainstZdxList(g_a_stObstacle, _p_stSupObj,
                                                         _p_stStartPosition, _p_stEndPosition);
    if (lCollision > 0) {
        for (; lCounter < lCollision; lCounter++) {
            bWater = 0;
            p_stObstacle = &g_a_stObstacle[lCounter];
            hGameMat = p_stObstacle->contact.hCollidedMaterial;
            if (GMT_fn_b_ulIsValid(hGameMat) != 0) {
                hCollMat = GMT_fn_hGetCollideMaterial(hGameMat);
                if (hCollMat != (GMT_tdstCollideMaterial_*) -1) {
                    if (GMT_fn_hGetCollideMaterialIdentifier(hCollMat) & 0xC000)
                        bWater = 1;
                }
            }
            if (bWater != 0) {
                continue;
            }
            MTH3D_M_vSubXYZ_inv(&stTempVec, &p_stObstacle->contact.m_stContact, &stStartVec);
            xDot = inv_MTH_fn_xMulAdd3Grouped(
                stTempVec.xX, stTempVec.xY, stTempVec.xZ, p_stObstacle->contact.m_stNorm.xX,
                p_stObstacle->contact.m_stNorm.xY, p_stObstacle->contact.m_stNorm.xZ);
            if (xDot <= 0.0f) {
                if (p_stObstacle->contact.m_xRate >= 0.0f) {
                    xMoveDot = inv_MTH_fn_xMulAdd3Grouped(
                        p_stObstacle->m_stZoneMove.xX, p_stObstacle->m_stZoneMove.xY,
                        p_stObstacle->m_stZoneMove.xZ, p_stObstacle->contact.m_stNorm.xX,
                        p_stObstacle->contact.m_stNorm.xY, p_stObstacle->contact.m_stNorm.xZ);
                    if (xMoveDot <= -0.005f) {
                        *_p_stResultObstacle = *p_stObstacle;
                        return 1;
                    }
                } else {
                    p_stReturnObstacle = p_stObstacle;
                    bLessDetected = 1;
                }
            }
        }
    }

    if (bLessDetected != 0) {
        *_p_stResultObstacle = *p_stReturnObstacle;
        return 1;
    }
    return 0;
}

s32 CAM_fn_bVectorIsNull(MTH3D_tdstVector_* _p_stVector, f32 _xEpsilon)
{
    return inv_MTH_fn_xAbsoluteTied(inv_MTH_fn_xAbsoluteTied(_p_stVector->xX) +
                                    (inv_MTH_fn_xAbsoluteTied(_p_stVector->xY) +
                                     inv_MTH_fn_xAbsoluteTied(_p_stVector->xZ))) < _xEpsilon;
}

s8 MTH_bCompareVectors(f32 _xEpsilon, MTH3D_tdstVector_* _p_stVector1,
                       MTH3D_tdstVector_* _p_stVector2)
{
    MTH3D_tdstVector_ stDelta = *_p_stVector1;

    MTH3D_M_vSubXYZ_inv(&stDelta, &stDelta, _p_stVector2);
    return CAM_fn_bVectorIsNull(&stDelta, _xEpsilon);
}

s32 inv_CAM_fn_bVectorExistsInArray(MTH3D_tdstVector_* vectors, MTH3D_tdstVector_* wanted,
                                    u32 count)
{
    u32 index;

    for (index = 0; index < count; index++) {
        if (vectors[index].xX == wanted->xX && vectors[index].xY == wanted->xY &&
            vectors[index].xZ == wanted->xZ) {
            return 1;
        }
    }
    return 0;
}

typedef struct DNM_ObstacleStack_ {
    MTH_stRotation stRotation;
    MTH3D_tdstVector_ a_stNormals[5];
    u8 padNormals[4];
    MTH3D_tdstVector_ a_stCross[5];
    u8 padCross[4];
    MTH3D_tdstVector_ a_stVectors[11];
    u8 padVectors[12];
    POS_tdstMatrix_ stBeginMatrix;
    u8 padBeginMatrix[0xC];
    POS_tdstMatrix_ stEndMatrix;
    u8 padEndMatrix[0xC];
    MTH3D_tdstVector_ VectTmp;
    u8 padVectTmp[4];
    MTH3D_tdstVector_ stCross;
    u8 padEnd[4];
} DNM_ObstacleStack_;

DNM_stMecObstacle* DNM_p_stObstacleCameraObstacle(DNM_stMecObstacle* _p_stObstacle,
                                                  DNM_stDynamics* _p_stDynamics,
                                                  DNM_stParsingDatas* _p_stExternData,
                                                  HIE_tdstSuperObject_* _hSupObj,
                                                  MTH3D_tdstVector_* _p_stWantedPosition,
                                                  MTH3D_tdstVector_* _p_stNewPosition, f32 _xDT)
{
    f32 xL;
    f32 xSq;
    f32 xSq4;
    f32 xSq1;
    f32 xInv;
    f32 xT;
    DNM_ObstacleStack_ stLocal;
    DNM_stReport_* p_stReport;
    stMACDPID* p_stMACDPID;
    MTH3D_tdstVector_* p_stObstacleNorm;
    MTH3D_tdstVector_* p_stTempVector;
    MTH3D_tdstVector_* p_stTempVector1;
    MTH3D_tdstVector_* p_stComputedCurrentPosition;
    MTH3D_tdstVector_* p_stWanted;
    MTH3D_tdstVector_* p_stDirection;
    MTH3D_tdstVector_* p_stImpactVector;
    MTH3D_tdstVector_* p_stProjectedVector;
    u32 uiNumNormals;
    u32 uiNumCross;
    s32 lBigAngle;
    s32 lExists;
    MTH3D_tdstVector_* p_stProjectionVector;
    MTH3D_tdstVector_* p_stStartPosition;
    MTH3D_tdstVector_* p_stLastPosition;
    MTH3D_tdstVector_* p_stNormals;
    POS_tdstMatrix_* p_stBeginMatrix;
    MTH4D_tdstVector_* p_stBeginTranslation;
    POS_tdstMatrix_* p_stEndMatrix;
    MTH4D_tdstVector_* p_stEndTranslation;
    MTH3D_tdstVector_* p_stLastNormal;
    MTH3D_tdstVector_* p_stNormalsBase;
    s32 lNear;
    s32 lNear2;
    f32 xZero;
    f32 zb;
    f32 zc;
    f32 zd;
    f32 ze;
    f32 zf;
    f32 xTx;
    f32 xEps;
    f32 xTy;
    f32 xTz;

    p_stReport = _p_stDynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    p_stMACDPID = &_p_stDynamics->m_tdstDynamicsComplexBlock.m_stExternalDatas;

    uiNumNormals = 0;
    uiNumCross = 0;
    p_stObstacleNorm = &stLocal.a_stVectors[0];
    p_stTempVector = &stLocal.a_stVectors[2];
    p_stTempVector1 = &stLocal.a_stVectors[3];
    p_stComputedCurrentPosition = &stLocal.a_stVectors[4];
    p_stDirection = &stLocal.a_stVectors[6];
    p_stImpactVector = &stLocal.a_stVectors[8];
    p_stProjectedVector = &stLocal.a_stVectors[9];
    p_stProjectionVector = &stLocal.a_stVectors[1];
    p_stStartPosition = &stLocal.a_stVectors[7];
    p_stWanted = &stLocal.a_stVectors[5];

    *p_stStartPosition = *inv_POS_fn_p_stPosMatrixTranslation(_hSupObj->gu_hGlobalMatrix);
    *p_stComputedCurrentPosition = *p_stStartPosition;
    *p_stWanted = *_p_stWantedPosition;
    *p_stDirection = p_stReport->m_stAbsolutePrevSpeed.m_stLinear;
    p_stLastPosition = &stLocal.a_stVectors[10];
    lNear = 0;
    if ((inv_MTH_fn_xAbsoluteTied(p_stDirection->xX - 0.0f) < 0.001f) &&
        (inv_MTH_fn_xAbsoluteTied(p_stDirection->xY - 0.0f) < 0.001f) &&
        (inv_MTH_fn_xAbsoluteTied(p_stDirection->xZ - 0.0f) < 0.001f)) {
        lNear = 1;
    }
    if (lNear == 0) {
        inv_MTH3D_fn_vNormalizeXYZ(p_stDirection);
    }

    lBigAngle = 0;
    p_stReport->m_cBitField &= ~1;
    p_stReport->m_ulCurrSurfaceState = 0;

    if (p_stMACDPID->m_uwData9 & 0x40) {
        *p_stComputedCurrentPosition = *_p_stWantedPosition;
    } else {
        if (0) {
        __hit_negative:
            p_stReport->m_cBitField |= 1;
            xSq = MTH_fn_xSqrAddSqrAddSqrOpt2(_p_stObstacle->m_stTranslation.xX,
                                              _p_stObstacle->m_stTranslation.xY,
                                              _p_stObstacle->m_stTranslation.xZ);
            ze = MTH_fn_xSqrtOpt2(xSq);
            xT = 1.0f / ze;
            ze = ze + 0.02f;
            MTH3D_M_vSetXYZ_inv(&stLocal.VectTmp, ze * (xT * _p_stObstacle->m_stTranslation.xX),
                                ze * (xT * _p_stObstacle->m_stTranslation.xY),
                                ze * (xT * _p_stObstacle->m_stTranslation.xZ));
            inv_MTH3D_fn_vAddXYZ(p_stComputedCurrentPosition, &stLocal.VectTmp, p_stWanted);
            goto __hit_done;
        __full:
            *p_stComputedCurrentPosition = *p_stStartPosition;
            goto __hit_done;
        }
        p_stBeginMatrix = &stLocal.stBeginMatrix;
        gu_MTH4D_M_vSetIdentityMatrix(p_stBeginMatrix);
        p_stEndMatrix = &stLocal.stEndMatrix;
        gu_MTH4D_M_vSetIdentityMatrix(p_stEndMatrix);
        if ((MTH_bCompareVectors(0.02f, p_stComputedCurrentPosition, p_stWanted) == 0) ||
            !(p_stMACDPID->m_uwData9 & 0x100)) {
            p_stBeginTranslation = &stLocal.stBeginMatrix.stMatrix.stCol_3;
            p_stEndTranslation = &stLocal.stEndMatrix.stMatrix.stCol_3;

            goto __test;
            do {
                {
                    f32 xD1, xI;
                    f32 xCmp;
                    p_stReport->m_stObstacle.m_stNorm = _p_stObstacle->contact.m_stNorm;
                    *p_stObstacleNorm = _p_stObstacle->contact.m_stNorm;
                    inv_MTH3D_fn_vSubXYZ(p_stTempVector, _p_stWantedPosition, p_stStartPosition);
                    xSq1 = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stTempVector->xX, p_stTempVector->xY,
                                                       p_stTempVector->xZ);
                    xL = MTH_fn_xSqrtOpt2(xSq1);
                    if (!(p_stReport->m_cBitField & 1) && (xL != 0.0f)) {
                        xCmp = -0.7f;
                        p_stReport->m_cBitField &= ~2;
                        xD1 = inv_MTH_fn_xMulAdd3(p_stTempVector->xX, p_stObstacleNorm->xX,
                                                  p_stTempVector->xY, p_stObstacleNorm->xY,
                                                  p_stTempVector->xZ, p_stObstacleNorm->xZ);
                        ze = xD1 / xL;
                        if (ze < xCmp) {
                            xI = 1.0f / xL;
                            MTH3D_M_vSubXYZ_inv(p_stTempVector1, &p_stMACDPID->m_stData10,
                                                p_stStartPosition);
                            inv_MTH3D_fn_vKTimesXYZ(p_stTempVector, xI, p_stTempVector);
                            inv_MTH3D_fn_vNormalizeXYZ(p_stTempVector1);
                            ze = inv_MTH_fn_xMulAdd3(p_stTempVector->xX, p_stTempVector1->xX,
                                          p_stTempVector->xY, p_stTempVector1->xY,
                                          p_stTempVector->xZ, p_stTempVector1->xZ);
                            if (0.7f < ze) {
                                p_stReport->m_cBitField |= 2;
                            }
                        }
                    }
                    p_stReport->m_ulCurrSurfaceState = 4;
                    p_stReport->m_cBitField |= 1;

                    xZero = 0.0f;
                    if (_p_stObstacle->contact.m_xRate < xZero)
                        goto __hit_negative;
                    xT = _p_stObstacle->contact.m_xRate;

                    p_stNormals = stLocal.a_stNormals;
                    inv_MTH3D_fn_vSubXYZ(p_stImpactVector, p_stWanted, p_stComputedCurrentPosition);
                    inv_MTH3D_fn_vKTimesXYZ(p_stImpactVector, xT, p_stImpactVector);
                    inv_MTH3D_fn_vAddXYZ(p_stImpactVector, p_stImpactVector,
                                         p_stComputedCurrentPosition);
                    lExists = inv_CAM_fn_bVectorExistsInArray(p_stNormals, p_stObstacleNorm,
                                                              uiNumNormals);
                    if (lExists == 0) {
                        if (uiNumNormals == 5) {
                            goto __full;
                        }
                        stLocal.a_stNormals[uiNumNormals] = *p_stObstacleNorm;
                        uiNumNormals = uiNumNormals + 1;
                        {
                            f32 _x, _y, _z, _l, _l2;
                            _x = p_stImpactVector->xX - _p_stObstacle->contact.m_stContact.xX;
                            p_stProjectionVector->xX = _x;
                            _y = p_stImpactVector->xY - _p_stObstacle->contact.m_stContact.xY;
                            p_stProjectionVector->xY = _y;
                            _z = p_stImpactVector->xZ - _p_stObstacle->contact.m_stContact.xZ;
                            p_stProjectionVector->xZ = _z;
                            _l2 = MTH_fn_xSqrAddSqrAddSqrOpt2(_x, _y, _z);
                            _l = MTH_fn_xSqrtOpt2(_l2);
                            if (_l != xZero) {
                                _l = 1.0f / _l;
                                MTH3D_M_vSetXYZ_inv(p_stProjectionVector, _l * _x, _l * _y,
                                                    _l * _z);
                            }
                        }
                        MTH_p_stRotationComputeRotationWith2Vectors(
                            &stLocal.stRotation, p_stProjectionVector, p_stObstacleNorm);
                        ze = stLocal.stRotation.m_xAngle;
                        if (ze < 0.5235988f) {
                            *p_stProjectionVector = *p_stObstacleNorm;
                        }
                        lNear = 0;
                        inv_MTH3D_fn_vSubXYZ(p_stTempVector, p_stImpactVector,
                                             p_stComputedCurrentPosition);
                        if ((inv_MTH_fn_xAbsoluteTied(p_stTempVector->xX - xZero) < 0.001f) &&
                            (inv_MTH_fn_xAbsoluteTied(p_stTempVector->xY - xZero) < 0.001f) &&
                            (inv_MTH_fn_xAbsoluteTied(p_stTempVector->xZ - xZero) < 0.001f)) {
                            lNear = 1;
                        }
                        if (lNear == 0) {
                            xSq4 = MTH_fn_xSqrAddSqrAddSqrOpt2(
                                p_stTempVector->xX, p_stTempVector->xY, p_stTempVector->xZ);
                            xL = MTH_fn_xSqrtOpt2(xSq4);

                            xInv = xL - 0.02f;
                            if (xInv < 0.0f) {
                                MTH3D_M_vZeroXYZ_inv(p_stTempVector);
                            } else {
                                xT = xInv / xL;
                                inv_MTH3D_fn_vKTimesXYZ(p_stTempVector, xT, p_stTempVector);
                            }
                            *p_stLastPosition = *p_stComputedCurrentPosition;
                            inv_MTH3D_fn_vAddXYZ(p_stComputedCurrentPosition,
                                                 p_stComputedCurrentPosition, p_stTempVector);
                        }
                        inv_MTH3D_fn_vSubXYZ(p_stProjectedVector, _p_stWantedPosition,
                                             p_stComputedCurrentPosition);
                        MTH_fn_vVectorPlaneProjection(p_stProjectedVector, p_stProjectedVector,
                                                      p_stProjectionVector,
                                                      p_stProjectionVector);
                        lNear = 0;
                        zc = 0.0f;
                        if ((inv_MTH_fn_xAbsoluteTied(p_stDirection->xX - zc) < 0.001f) &&
                            (inv_MTH_fn_xAbsoluteTied(p_stDirection->xY - zc) < 0.001f) &&
                            (inv_MTH_fn_xAbsoluteTied(p_stDirection->xZ - zc) < 0.001f)) {
                            lNear = 1;
                        }
                        if (lNear == 0) {
                            zd = 0.0f;
                            lNear2 = 0;
                            if ((inv_MTH_fn_xAbsoluteTied(p_stProjectedVector->xX - zd) < 0.001f) &&
                                (inv_MTH_fn_xAbsoluteTied(p_stProjectedVector->xY - zd) < 0.001f) &&
                                (inv_MTH_fn_xAbsoluteTied(p_stProjectedVector->xZ - zd) < 0.001f)) {
                                lNear2 = 1;
                            }
                            if (lNear2 == 0) {
                                {
                                    f32 _l2, _l, _i;
                                    _l2 = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stProjectedVector->xX,
                                                                      p_stProjectedVector->xY,
                                                                      p_stProjectedVector->xZ);
                                    _l = MTH_fn_xSqrtOpt2(_l2);
                                    if (_l != 0.0f) {
                                        _i = 1.0f / _l;
                                        inv_MTH3D_fn_vKTimesXYZ(p_stTempVector, _i,
                                                                p_stProjectedVector);
                                    } else {
                                        *p_stTempVector = *p_stProjectedVector;
                                    }
                                }
                                MTH_p_stRotationComputeRotationWith2Vectors(
                                    &stLocal.stRotation, p_stTempVector, p_stDirection);
                                ze = stLocal.stRotation.m_xAngle;
                                if (ze > 2.6179938f) {
                                    lBigAngle = 1;
                                }
                            }
                        }
                    }

                    if ((lBigAngle != 0) || (lExists != 0)) {
                        if (uiNumNormals < 2U)
                            goto __flag2;
                        {
                            p_stNormalsBase = &stLocal.a_stNormals[-1];
                            p_stLastNormal = p_stNormalsBase + uiNumNormals;
                            if ((p_stObstacleNorm->xX == p_stLastNormal->xX) &&
                                (p_stObstacleNorm->xY == p_stLastNormal->xY) &&
                                (p_stObstacleNorm->xZ == p_stLastNormal->xZ)) {
                                goto __flag2;
                            }
                            if (uiNumCross == 5) {
                            __flag2:
                                p_stReport->m_cBitField |= 2;
                                goto __hit_done;
                            }
                            *p_stProjectedVector = *_p_stWantedPosition;
                            MTH3D_M_vSubXYZ_inv(p_stProjectedVector, p_stProjectedVector,
                                                p_stComputedCurrentPosition);
                            if ((p_stTempVector == p_stNormals + (uiNumNormals - 1)) ||
                                (p_stTempVector == p_stObstacleNorm)) {
                                p_stLastNormal = p_stNormalsBase + uiNumNormals;
                                DNM_p_stVectorCrossProduct(&stLocal.stCross, p_stLastNormal,
                                                           p_stObstacleNorm);
                                *p_stTempVector = stLocal.stCross;
                            } else {
                                p_stLastNormal = p_stNormalsBase + uiNumNormals;
                                DNM_p_stVectorCrossProduct(p_stTempVector, p_stLastNormal,
                                                           p_stObstacleNorm);
                            }
                            inv_MTH3D_fn_vNormalizeXYZ(p_stTempVector);
                            if (inv_CAM_fn_bVectorExistsInArray(stLocal.a_stCross, p_stTempVector,
                                                                uiNumCross) != 0) {
                                goto __hit_done;
                            }
                            stLocal.a_stCross[uiNumCross] = *p_stTempVector;
                            uiNumCross = uiNumCross + 1;
                            MTH_fn_vVectorPlaneProjection(p_stTempVector1, p_stProjectedVector,
                                                          p_stTempVector, p_stTempVector);
                            MTH3D_M_vSubXYZ_inv(p_stProjectedVector, p_stProjectedVector,
                                                p_stTempVector1);
                        }
                    }

                    if ((p_stProjectedVector->xX == 0.0f) && (p_stProjectedVector->xY == 0.0f) &&
                        (p_stProjectedVector->xZ == 0.0f)) {
                        MTH3D_M_vZeroXYZ_inv(p_stDirection);
                    } else {
                        inv_MTH3D_fn_vAddXYZ(p_stWanted, p_stComputedCurrentPosition,
                                             p_stProjectedVector);
                        {
                            f32 _l2, _l, _i;
                            _l2 = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stProjectedVector->xX,
                                                              p_stProjectedVector->xY,
                                                              p_stProjectedVector->xZ);
                            _l = MTH_fn_xSqrtOpt2(_l2);
                            if (_l != 0.0f) {
                                _i = 1.0f / _l;
                                inv_MTH3D_fn_vKTimesXYZ(p_stDirection, _i, p_stProjectedVector);
                            } else {
                                *p_stDirection = *p_stProjectedVector;
                            }
                        }
                    }
                }
                if (MTH_bCompareVectors(0.02f, p_stComputedCurrentPosition, p_stWanted) != 0) {
                    goto __hit_done;
                }
            __test:
                if (stLocal.stBeginMatrix.ucType == MAT_eTransform_Identity) {
                    p_stBeginMatrix->ucType = MAT_eTransform_Translation;
                }
                *(MTH3D_tdstVector_*) &stLocal.stBeginMatrix.stMatrix.stCol_3 =
                    *p_stComputedCurrentPosition;
                p_stBeginTranslation->xW = 1.0f;
                if (stLocal.stEndMatrix.ucType == MAT_eTransform_Identity) {
                    p_stEndMatrix->ucType = MAT_eTransform_Translation;
                }
                *(MTH3D_tdstVector_*) &stLocal.stEndMatrix.stMatrix.stCol_3 = *p_stWanted;
                p_stEndTranslation->xW = 1.0f;
            } while (CAM_lHaveITakeSThgIntoTheMug(_p_stObstacle, _hSupObj, p_stBeginMatrix,
                                                  p_stEndMatrix) > 0);
            *p_stComputedCurrentPosition = *p_stWanted;
        __hit_done:;
        }
    }

    lNear = 0;

    ze = 0.0f;
    xEps = 0.001f;
    MTH3D_M_vSubXYZ_inv(p_stTempVector, p_stComputedCurrentPosition, p_stStartPosition);
    p_stReport->m_stAbsolutePrevSpeed.m_stLinear = *p_stTempVector;
    xTx = p_stMACDPID->m_stData10.xX;
    xTy = p_stMACDPID->m_stData10.xY;
    xTz = p_stMACDPID->m_stData10.xZ;
    MTH3D_M_vSetXYZ_inv(p_stTempVector, xTx - p_stStartPosition->xX, xTy - p_stStartPosition->xY,
                        xTz - p_stStartPosition->xZ);
    if ((inv_MTH_fn_xAbsoluteTied(p_stTempVector->xX - ze) < xEps) &&
        (inv_MTH_fn_xAbsoluteTied(p_stTempVector->xY - ze) < xEps) &&
        (inv_MTH_fn_xAbsoluteTied(p_stTempVector->xZ - ze) < xEps)) {
        lNear = 1;
    }
    if (lNear == 0) {
        f32 eps2;
        {
            f32 _x, _y, _z, _l, _l2;
            _x = p_stTempVector->xX;
            _y = p_stTempVector->xY;
            _z = p_stTempVector->xZ;
            _l2 = MTH_fn_xSqrAddSqrAddSqrOpt2(_x, _y, _z);
            zf = 0.0f;
            _l = MTH_fn_xSqrtOpt2(_l2);
            if (_l != zf) {
                _l = 1.0f / _l;
                MTH3D_M_vSetXYZ_inv(p_stTempVector, _l * _x, _l * _y, _l * _z);
            }
        }
        lNear2 = 0;
        eps2 = 0.001f;
        p_stTempVector1->xX = xTx - p_stComputedCurrentPosition->xX;
        p_stTempVector1->xY = xTy - p_stComputedCurrentPosition->xY;
        zb = xTz - p_stComputedCurrentPosition->xZ;
        p_stTempVector1->xZ = zb;
        if ((inv_MTH_fn_xAbsoluteTied(p_stTempVector1->xX - zf) < eps2) &&
            inv_MTH_fn_xAbsoluteTied(p_stTempVector1->xY - zf) < eps2 &&
            inv_MTH_fn_xAbsoluteTied(zb - zf) < eps2) {
            lNear2 = 1;
        }
        if (lNear2 == 0) {
            inv_MTH3D_fn_vNormalizeXYZ(p_stTempVector1);
            MTH_p_stRotationComputeRotationWith2Vectors(&stLocal.stRotation, p_stTempVector,
                                                        p_stTempVector1);
            ze = stLocal.stRotation.m_xAngle;
        }
    }
    _p_stExternData->xOutAlpha = ze;
    *_p_stNewPosition = *p_stComputedCurrentPosition;
    return _p_stObstacle;
}

void CAM_fn_vComputeCameraSystemAxis(void* _p_stDynamics, HIE_tdstSuperObject_* _h_SupObj,
                                     MTH3D_tdstVector_* _p_stNewPosition,
                                     MTH3D_tdstVector_* _p_stNewTarget,
                                     MTH3D_tdstVector_* _p_stAxisX, MTH3D_tdstVector_* _p_stAxisY,
                                     MTH3D_tdstVector_* _p_stAxisZ)
{
    MTH3D_tdstVector_ stYVector;
    s32 bIsNull = 0;
    f32 xNorm;

    {
        f32 dx;
        f32 dy;
        f32 dz;

        dx = _p_stNewPosition->xX - _p_stNewTarget->xX;
        _p_stAxisY->xX = dx;
        dy = _p_stNewPosition->xY - _p_stNewTarget->xY;
        _p_stAxisY->xY = dy;
        dz = _p_stNewPosition->xZ - _p_stNewTarget->xZ;
        _p_stAxisY->xZ = dz;

        stYVector = *_p_stAxisY;

        if (inv_MTH_fn_xAbsolute(dx - 0.0f) < 0.001f && inv_MTH_fn_xAbsolute(dy - 0.0f) < 0.001f &&
            inv_MTH_fn_xAbsolute(dz - 0.0f) < 0.001f) {
            bIsNull = 1;
        }
    }

    if (!bIsNull) {
        inv_MTH3D_fn_vNormalizeXYZ(_p_stAxisY);
    }

    MTH_fn_vVectorPlaneProjection(&stYVector, &stYVector, _p_stAxisZ, _p_stAxisZ);

    xNorm = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(stYVector.xX, stYVector.xY, stYVector.xZ));
    if (xNorm < 0.2f) {
        POS_fn_vGetRotationVector(_h_SupObj->gu_hGlobalMatrix, _p_stAxisZ);
    }

    DNM_p_stVectorCrossProduct(_p_stAxisX, _p_stAxisY, _p_stAxisZ);

    if (MTH3D_M_bIsZeroXYZ_inv(_p_stAxisX)) {
        POS_fn_vGetRotationVector(_h_SupObj->gu_hGlobalMatrix, _p_stAxisZ);
        DNM_p_stVectorCrossProduct(_p_stAxisX, _p_stAxisY, _p_stAxisZ);
    }

    inv_MTH3D_fn_vNormalizeXYZ(_p_stAxisX);

    DNM_p_stVectorCrossProduct(_p_stAxisZ, _p_stAxisX, _p_stAxisY);
    inv_MTH3D_fn_vNormalizeXYZ(_p_stAxisZ);
}

DNM_stDynamics*
DNM_p_stDynamicsCameraUpdate(DNM_stDynamics* _p_stDynamics, DNM_stParsingDatas* _p_stParsingDatas,
                             HIE_tdstSuperObject_* _hSupObj, DNM_stMecObstacle* _p_stObstacle,
                             MTH3D_tdstVector_* _p_stPosition, MTH3D_tdstVector_* _p_stTarget)
{
    POS_tdstMatrix_* p_stMatrix;
    MTH3D_tdstVector_ stAxisX __attribute__((aligned(16)));
    MTH3D_tdstVector_ stAxisY __attribute__((aligned(16)));
    MTH3D_tdstVector_ stAxisZ __attribute__((aligned(16)));

    p_stMatrix = _hSupObj->gu_hLocalMatrix;
    if (!(_p_stDynamics->m_tdstDynamicsComplexBlock.m_stExternalDatas.m_uwData9 & 0x80)) {
        stAxisZ = _p_stDynamics->m_tdstDynamicsComplexBlock.m_stExternalDatas.m_stData12;
        CAM_fn_vComputeCameraSystemAxis(_p_stDynamics, _hSupObj, _p_stPosition, _p_stTarget,
                                        &stAxisX, &stAxisY, &stAxisZ);
        _POS_fn_vSetRotationMatrix(p_stMatrix, &stAxisX, &stAxisY, &stAxisZ);
    }
    if (p_stMatrix->ucType == MAT_eTransform_Identity) {
        p_stMatrix->ucType = MAT_eTransform_Translation;
    }
    MTH4D_fn_vCopy3DTo4DVector(&p_stMatrix->stMatrix.stCol_3, _p_stPosition);
    *_hSupObj->gu_hGlobalMatrix = *p_stMatrix;
    return _p_stDynamics;
}

#ifdef PC_PORT
void* DNM_p_stDynamicsCameraMechanics(DNM_stDynamics* _p_stDynamics, HIE_tdstSuperObject_* _hSupObj,
                                      DNM_stParsingDatas* _p_stParsingDatas, void* unused, f32 _xDT)
#else
DNM_stDynamics* DNM_p_stDynamicsCameraMechanics(DNM_stDynamics* _p_stDynamics,
                                                HIE_tdstSuperObject_* _hSupObj,
                                                DNM_stParsingDatas* _p_stParsingDatas, f32 _xDT)
#endif
{
    DNM_stMecObstacle stObstacle;
    MTH3D_tdstVector_ stUnused __attribute__((aligned(16)));
    MTH3D_tdstVector_ stPosition1 __attribute__((aligned(16)));
    MTH3D_tdstVector_ stTarget __attribute__((aligned(16)));
    MTH3D_tdstVector_ stPosition __attribute__((aligned(16)));

#ifdef PC_PORT
    /* Rayman's speed setting: the camera follows him with its time step scaled as his is. */
    _xDT = _xDT * (f32) pc_mod_camera_percent() / 100.0f;
    /* Noclip: the camera orbits Rayman where the host puts it, through walls. */
    stPosition = *inv_POS_fn_p_stPosMatrixTranslation(_hSupObj->gu_hGlobalMatrix);
    if (pc_mod_noclip_camera((PcAddress) &stPosition, (PcAddress) &stTarget)) {
        DNM_p_stDynamicsCameraUpdate(_p_stDynamics, _p_stParsingDatas, _hSupObj, &stObstacle, &stPosition,
                                     &stTarget);
        _p_stParsingDatas->stPosition = stPosition;
        _p_stParsingDatas->stVector = stTarget;
        return _p_stDynamics;
    }
#endif
    DNM_p_stDynamicsCameraParsing(_p_stDynamics, _hSupObj, _p_stParsingDatas, _xDT);
    MTH3D_M_vSetXYZ_inv(&stUnused, 0.0f, 0.0f, -1.0f);
    stPosition = _p_stParsingDatas->stPosition;
    if (DNM_p_stObstacleCameraObstacle(&stObstacle, _p_stDynamics, _p_stParsingDatas, _hSupObj,
                                       &stPosition, &stPosition1, _xDT) == NULL) {
        return NULL;
    }
    _p_stParsingDatas->stPosition = stPosition;
    stTarget = _p_stParsingDatas->stVector;
    if (DNM_p_stDynamicsCameraUpdate(_p_stDynamics, _p_stParsingDatas, _hSupObj, &stObstacle,
                                     &stPosition1, &stTarget) == NULL) {
        return NULL;
    }
    _p_stParsingDatas->stVector = stTarget;
    return _p_stDynamics;
}
