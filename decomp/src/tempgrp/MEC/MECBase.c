#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"

#include "include_asm.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "AI/AI.h"

u32 fn_ucCollSetGetCollComputeFrequencySO(HIE_tdstSuperObject_*);

extern void POS_fn_vGetYAxisVector(POS_tdstMatrix_* m, MTH3D_tdstVector_* v);
f32 cosf(f32 value);
f32 cosf(f32 value);
extern f32 acosf(f32 value);

s32 MEC_fn_lComputeCall(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj, f32 xDT,
                        f32 xCoef)
{
    MTH3D_tdstVector_ move;
    f32 squared;
    f32 distance;
    f32 lengthSquared;
    s32 count;
    s32 frames;
    MTH3D_tdstVector_* previous =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    MTH3D_tdstVector_* current =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;

    inv_MTH3D_fn_vSubXYZ(&move, current, previous);
    if ((s32) fn_ulStandardGameGetCustomBitsSO(hSupObj) < 0) {
        squared = inv_MTH_fn_xMulAdd3Tied(move.xX, move.xX, move.xY, move.xY, move.xZ, move.xZ);
        distance = MTH_fn_xSqrtOpt2(squared);
        count = inv_MTH_fn_lToIntegerOrdered((distance / 0.9f) * xCoef + 1.0f);
    } else {
        frames = fn_lGetTemporalCurveType();
        count = 1;
        if (frames > 4) {
            lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(move.xX, move.xY, move.xZ);
            if (MTH_fn_xSqrtOpt2(lengthSquared) > 0.5f) {
                count = 2;
            }
        }
    }
    return count;
}

void MEC_fn_vBaseManager(DNM_stDynamics* obj, void* ctx, f32 dt)
{
    s32 count;
    s32 i;

    if ((obj->m_tdstDynamicsBaseBlock.ulEndFlags & 0x04000000) != 0 || MEC_g_xBaseCoeff == 0.0f) {
        MEC_fn_vBaseObstacleManagement(obj, ctx, dt);
        return;
    }
    i = 1;
    count = MEC_fn_lComputeCall(obj, ctx, dt, MEC_g_xBaseCoeff);
    MEC_fn_vSlowMode(obj, count);
    MEC_fn_vBaseObstacleManagement(obj, ctx, dt);
    if (i < count) {
        i = count - 1;
        do {
            func_0013B6E8(obj);
            i--;
            MEC_fn_vBaseObstacleManagement(obj, ctx, dt);
        } while (i != 0);
    }
    MEC_fn_vNormalMode(obj, count);
}

void MEC_fn_vWalkManager(DNM_stDynamics* obj, void* ctx, f32 dt)
{
    s32 count;
    s32 i;

    if ((obj->m_tdstDynamicsBaseBlock.ulEndFlags & 0x04000000) != 0 || MEC_g_xWalkCoeff == 0.0f) {
        MEC_fn_vGroundObstacleManagement(obj, ctx, dt);
        return;
    }

    i = 1;
    count = MEC_fn_lComputeCall(obj, ctx, dt, MEC_g_xWalkCoeff);
    MEC_fn_vSlowMode(obj, count);
    MEC_fn_vGroundObstacleManagement(obj, ctx, dt);
    if (i < count) {
        i = count - 1;
        do {
            g_lCallCounter = 0;
            func_0013B6E8(obj);
            --i;
            MEC_fn_vGroundObstacleManagement(obj, ctx, dt);
        } while (i != 0);
    }
    MEC_fn_vNormalMode(obj, count);
}

DNM_stDynamics* MEC_p_stDynamicsBaseMechanics(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj)
{
    typedef struct UpdateState {
        MTH3D_tdstVector_ first;
        f32 scalar;
        MTH3D_tdstVector_ second;
    } UpdateState;

    POS_tdstMatrix_ stRefMatrix;
    POS_tdstMatrix_ stInvMatrix;
    POS_tdstMatrix_ stPrevPreviousReferenceMatrix;
    POS_tdstMatrix_ stPreviousReferenceMatrix;
    POS_tdstMatrix_ stInvPreviousReferenceMatrix;
    MTH3D_tdstVector_ stComputedSpeed;
    MTH3D_tdstVector_ stStablePreviousPosition;
    MTH3D_tdstVector_ stSpeed;
    MTH3D_tdstVector_ stYAxis;
    UpdateState st;
    MTH3D_tdstVector_ stMatrixTranslation;
    MTH3D_tdstVector_ stVector;
    POS_tdstMatrix_* hPrevPreviousMatrix;
    POS_tdstMatrix_* m280;
    POS_tdstMatrix_* hCurrentMatrix;
    POS_tdstMatrix_* p_stRefMatrix;
    POS_tdstMatrix_* copyDst;
    POS_tdstMatrix_* copySrc;
    POS_tdstMatrix_* p_stPrevPreviousReferenceMatrix;
    POS_tdstMatrix_* hInvRefMatrix;
    DNM_stReport_* sub;
    s32 hasPlatform;
    s32 result27C;
    s32 count;
    s32 i;
    s32 copied;
    s32 stateByte;
    s32 bit29;
    u32 flags08;
    f32 frameDt;
    f32 countf;
    f32 totalDt;
    f32 len;
    f32 limit;
    POS_tdstMatrix_* ctxMat;
    POS_tdstMatrix_* hPreviousMatrix;
    POS_tdstMatrix_* p_stInvPreviousReferenceMatrix;
    f32* p24;
    f32* p30;
    MTH4D_tdstVector_* pCCv30;
    f32* pM0v40;
    MTH3D_tdstVector_* pVec54;
    MTH3D_tdstVector_* pVec6C;
    s32 copyAlign;
    MTH4D_tdstVector_* speedCC;
    POS_tdstMatrix_* render;
    MTH3D_tdstVector_* pVec3C;
    MTH3D_tdstVector_* p_stVector;
    MTH3D_tdstVector_* p_stStablePreviousPosition;

#ifdef PC_PORT
    /* Noclip: Rayman is put where the stick takes him, and the game's mechanics are left out. */
    if (pc_mod_noclip((PcAddress) p_stDynamic, (PcAddress) hSupObj)) {
        HIE_fn_vComputeNewRelativeMatrix(hSupObj);
        HIE_fn_vRefreshHierarchy(hSupObj);
        return p_stDynamic;
    }
#endif
    sub = 0;
    hasPlatform = 0;
    result27C = 0;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) {
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4004) == 0x4004) {
            st.second.xZ = st.second.xY = st.second.xX = st.scalar = st.first.xZ = st.first.xY =
                st.first.xX = 0.0f;
        }
        sub = p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport;
        if (sub->m_ulCurrSurfaceState & 0x80) {
            sub->m_ulCurrSurfaceState = 0x10000;
        } else {
            sub->m_ulCurrSurfaceState = 0;
        }
    }

    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
        (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags | 0x2000) & 0xFEFFFFFF;
    if (fn_bGetCharacterForceCollisionWhenNotMoving(hSupObj) == 0 && MEC_fn_bNoMove(p_stDynamic) != 0 &&
        (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x1080) == 0) {
        return p_stDynamic;
    }

    frameDt = inv_fn_xGetEngineClockScale();
    hCurrentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    count = fn_lGetTemporalCurveType();
    hPreviousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
    countf = (f32) count;
    totalDt = countf * frameDt;
#ifdef PC_PORT
    if (inv_GAM_fn_xGetEngineClockTicks() != countf) {
        frameDt = frameDt * inv_GAM_fn_xGetEngineClockTicks() / countf;
        totalDt = countf * frameDt;
    }
    /* Rayman's speed setting: his time step is scaled, so his moves keep their collisions. */
    {
        s32 percent = pc_mod_mechanics_percent((PcAddress) hSupObj);
        if (percent != 100) {
            frameDt = frameDt * (f32) percent / 100.0f;
            totalDt = countf * frameDt;
        }
    }
#endif

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
        hPrevPreviousMatrix = &p_stDynamic->m_tdstDynamicsComplexBlock.m_stPrevPreviousMatrix;
        m280 = &p_stDynamic->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix;
    }

    p_stRefMatrix = &stRefMatrix;
    gu_MTH4D_M_vSetIdentityMatrix(p_stRefMatrix);
    p_stPrevPreviousReferenceMatrix = &stPrevPreviousReferenceMatrix;
    gu_MTH4D_M_vSetIdentityMatrix(p_stPrevPreviousReferenceMatrix);
    p_stInvPreviousReferenceMatrix = &stInvPreviousReferenceMatrix;
    ctxMat = hSupObj->gu_hGlobalMatrix;
    gu_MTH4D_M_vSetIdentityMatrix(p_stInvPreviousReferenceMatrix);
    hInvRefMatrix = &stPreviousReferenceMatrix;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x800;
    g_stReport.m_ulCurrSurfaceState = 0;
    g_lCallCounter = 0;
    g_bHighSpeed = 0;
    if (lZWaterValidity < 10) {
        lZWaterValidity++;
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) {
        *hCurrentMatrix = *ctxMat;
        copyAlign = (s32) hPreviousMatrix | (s32) &stRefMatrix;
        *hPreviousMatrix = *ctxMat;
        pVec3C = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
            *hPrevPreviousMatrix = *ctxMat;
            *m280 = *ctxMat;
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation = *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix);
        }
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x1000) == 0) {
            POS_fn_vGetScaleMatrix(ctxMat, &stSpeed);
            MTH3D_M_vCopyXYZ_inv(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale, &stSpeed);
        }
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = pVec3C->xY =
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x80;
    } else {
        copyAlign = (s32) hPreviousMatrix | (s32) &stRefMatrix;
    }

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x14) == 0x14) {
        MEC_fn_vCancelTilt(p_stDynamic);
    }

    *p_stInvPreviousReferenceMatrix = *hPreviousMatrix;
    copied = copyAlign;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x20000000) {
        POS_fn_vSetRotationMatrix(p_stInvPreviousReferenceMatrix,
                                  &p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeRotationMatrix);
    }

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) &&
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_cCollideCounter <= 0) {
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_cCollideCounter =
            (s8) fn_ucCollSetGetCollComputeFrequencySO(hSupObj);
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x200) {
        MTH3D_M_vKTimesXYZ_inv(&stMatrixTranslation, totalDt,
                               &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed);
        bit29 = ((s32) p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags >> 29) & 1;
        MEC_fn_vSetInitialFrameTranslation(p_stDynamic, &stMatrixTranslation);
        *p_stRefMatrix = *hPreviousMatrix;

        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0xE0000) {
            MEC_fn_vImposeSpeed(p_stDynamic, totalDt, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x20000;
            p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x40000;
            p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x80000;
        }
        if (bit29) {
            MEC_fn_vImposeRotation(p_stDynamic);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x20000000;
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 2) {
            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
                p_stDynamic->m_tdstDynamicsAdvancedBlock.m_cCollideCounter--;
                if (p_stDynamic->m_tdstDynamicsAdvancedBlock.m_cCollideCounter == 0) {
                    MEC_fn_vBaseManager(p_stDynamic, hSupObj, totalDt);
                    if (fn_ucCollSetGetCollComputeFrequencySO(hSupObj) >= 2) {
                        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation =
                            *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix);
                    }
                }
            } else {
                MEC_fn_vBaseManager(p_stDynamic, hSupObj, totalDt);
            }
        }

        MEC_fn_vComputeSpeed(p_stDynamic, totalDt);
        MEC_fn_vCorrectMatrix(p_stDynamic, hCurrentMatrix);
        copied = 0;
        if (inv_POS_fn_bIsEqualMatrixExact(ctxMat, hCurrentMatrix) == 0) {
            *ctxMat = *hCurrentMatrix;
            HIE_fn_vComputeNewRelativeMatrix(hSupObj);
            copied = 1;
        }
        if (copied || inv_POS_fn_bIsEqualMatrixExact(hPreviousMatrix, hCurrentMatrix) == 0) {
            if (inv_fn_ucSuperObjectGetPlatFormType(hSupObj) != 0) {
                HIE_fn_vRefreshHierarchy(hSupObj);
                if (bit29) {
                    render = &stInvMatrix;
                    POS_fn_vInvertMatrix(render, p_stRefMatrix);
                } else {
                    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x800) {
                        POS_fn_vInvertMatrix(hInvRefMatrix, p_stInvPreviousReferenceMatrix);
                        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x800;
                    }
                    render = hInvRefMatrix;
                }
                MEC_fn_vUpdatePlatform(p_stDynamic, hSupObj, p_stRefMatrix, render);
            } else if (fn_ulStandardGameGetCustomBitsSO(hSupObj) & 2) {
                HIE_fn_vRefreshHierarchy(hSupObj);
            } else {
                HIE_fn_vInvalidateHierarchyFrame(hSupObj);
            }
        }
        MEC_fn_vInitMatrix(p_stDynamic);
        MEC_fn_vFillCollisionReport(p_stDynamic);
        return p_stDynamic;
    }

    stStablePreviousPosition = *inv_POS_fn_p_stPosMatrixTranslation(hPreviousMatrix);
    copyDst = p_stRefMatrix;
    copySrc = hPreviousMatrix;
    *copyDst = *copySrc;
    pVec54 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim;
    p_stVector = &stVector;
    pM0v40 = &p_stRefMatrix->stScale.xX;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
        *p_stPrevPreviousReferenceMatrix = *hPrevPreviousMatrix;
    }
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x20;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x40;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x2000) {
        f32 px;
        s32 moved;
        px = p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xX;
        moved = 0;
        if (!(inv_MTH_fn_xAbsolute(px - 1.0f) < 0.001f &&
              inv_MTH_fn_xAbsolute(p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xY - 1.0f) <
                  0.001f &&
              inv_MTH_fn_xAbsolute(p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale.xZ - 1.0f) <
                  0.001f))
            moved = 1;
        if (moved) {
            MEC_fn_vAdjustScaleData(p_stDynamic, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
        }
    }

    i = 0;
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xX =
        frameDt * p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xX;
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xY =
        frameDt * p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xY;
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xZ =
        frameDt * p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xZ;
    POS_fn_vRotateVector(pVec54, hCurrentMatrix, pVec54);

    {
        f32 vx;
        f32 vy;
        f32 vz;

        vx = p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xX;
        vy = p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xY;
        vz = p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xZ;
        MTH3D_M_vSetXYZ_inv(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation, vx / countf,
                            vy / countf, vz / countf);
    }
    MEC_fn_vTranslateMatrixOn(p_stDynamic, p_stVector);
    while (i < count) {
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 1) {
            MEC_fn_vSetInitialFrameTranslation(p_stDynamic, pVec54);
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x700000) {
            MEC_fn_vProposeSpeed(p_stDynamic, frameDt, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
        }
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x800) &&
            (len = p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamPrority) == 0.0f) {
            MEC_fn_vAddStream(p_stDynamic, frameDt, 1);
        }
        MEC_fn_vInertia(p_stDynamic, frameDt, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 4) {
            MEC_fn_vGravitySpeed(p_stDynamic, frameDt);
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0xE0000) {
            MEC_fn_vImposeSpeed(p_stDynamic, frameDt, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x3800000) {
            MEC_fn_vAddSpeed(p_stDynamic, frameDt, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x20000000) {
            MEC_fn_vImposeRotation(p_stDynamic);
            MEC_fn_vAdditionnalTranslation(p_stDynamic);
        } else if ((s32) p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags < 0) {
            MEC_fn_vAdditionnalTranslation(p_stDynamic);
        }
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x800) &&
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamPrority == 1.0f) {
            MEC_fn_vAddStream(p_stDynamic, frameDt, 0);
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x200) {
            MEC_fn_vSpeedLimit(p_stDynamic, frameDt, p_stInvPreviousReferenceMatrix, hInvRefMatrix);
        }
        MEC_fn_vComputeSpeed(p_stDynamic, frameDt);
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
            MEC_fn_vComputeInertiaTranslation(p_stDynamic, frameDt);
        }
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
            *hPrevPreviousMatrix = *hPreviousMatrix;
        }
        *hPreviousMatrix = *hCurrentMatrix;

        if (MTH3D_M_bIsZeroXYZ_inv(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed)) {
            i = count;
        }
        i++;
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
        *hPrevPreviousMatrix = *p_stPrevPreviousReferenceMatrix;
    }
    *hPreviousMatrix = *p_stRefMatrix;
    MEC_fn_vTranslateMatrixOff(p_stDynamic, p_stVector);
    MEC_fn_vAdjustSpeed(p_stDynamic, totalDt);
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 8) {
        MEC_fn_vAddTiltSpeed(p_stDynamic, totalDt);
    }
    pCCv30 = &hCurrentMatrix->stMatrix.stCol_3;
    stateByte = MEC_fn_bDynamicsNeedCollide(p_stDynamic) & 0xFF;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
        hasPlatform = p_stDynamic->m_tdstDynamicsComplexBlock.m_hPlatform != 0;
    }
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x8000;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_cCollideCounter--;
        if (p_stDynamic->m_tdstDynamicsAdvancedBlock.m_cCollideCounter == 0) {
            if (!(stateByte == 0 && !hasPlatform &&
                  fn_bGetCharacterForceCollisionWhenNotMoving(hSupObj) == 0)) {
                flags08 = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
                if (flags08 & 0x10) {
                    MEC_fn_vGiObstacleManagement(p_stDynamic, hSupObj, totalDt);
                } else if (flags08 & 0x40) {
                    MEC_fn_vClimbObstacleManagement(p_stDynamic, hSupObj, totalDt);
                    {
                        f32 norm;
                        f32 threshold;
                        threshold = a333;
                        norm = MTH_fn_xSqrAddSqrAddSqrOpt2(
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX,
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY,
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ);
                        norm = MTH_fn_xSqrtOpt2(norm);
                        if (norm < threshold) {
                            if (hCurrentMatrix->ucType == MAT_eTransform_Identity) {
                                hCurrentMatrix->ucType = MAT_eTransform_Translation;
                            }
                            *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix) = *inv_POS_fn_p_stPosMatrixTranslation(hPreviousMatrix);
                            pCCv30->xW = 1.0f;
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ =
                                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY =
                                    p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
                        }
                    }
                } else if (flags08 & 0x20) {
                    MEC_fn_vGroundObstacleManagement(p_stDynamic, hSupObj, totalDt);
                } else if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100) {
                    MEC_fn_vSpiderObstacleManagement(p_stDynamic, hSupObj, totalDt);
                } else if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100000) {
                    MEC_fn_vSwimObstacleManagement(p_stDynamic, hSupObj, totalDt);
                } else if (flags08 & 2) {
                    MEC_fn_vBaseObstacleManagement(p_stDynamic, hSupObj, totalDt);
                }
            }
            if (fn_ucCollSetGetCollComputeFrequencySO(hSupObj) >= 2) {
                p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation =
                    *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix);
            }
        } else {
            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x20) {
                MEC_fn_vGroundGuess(p_stDynamic, totalDt);
            }
            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) {
                sub->m_ulCurrSurfaceState = sub->m_ulPrevSurfaceState;
            }
        }
    } else {
        if (!(stateByte == 0 && fn_bGetCharacterForceCollisionWhenNotMoving(hSupObj) == 0)) {
            flags08 = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
            if (flags08 & 0x10) {
                MEC_fn_vGiObstacleManagement(p_stDynamic, hSupObj, totalDt);
            } else if (flags08 & 0x40) {
                MEC_fn_vClimbObstacleManagement(p_stDynamic, hSupObj, totalDt);
                {
                    f32 norm;
                    f32 threshold;
                    threshold = a333;
                    norm = MTH_fn_xSqrAddSqrAddSqrOpt2(
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX,
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY,
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ);
                    norm = MTH_fn_xSqrtOpt2(norm);
                    if (norm < threshold) {
                        if (hCurrentMatrix->ucType == MAT_eTransform_Identity) {
                            hCurrentMatrix->ucType = MAT_eTransform_Translation;
                        }
                        *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix) = *inv_POS_fn_p_stPosMatrixTranslation(hPreviousMatrix);
                        pCCv30->xW = 1.0f;
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ =
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY =
                                p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = 0.0f;
                    }
                }
            } else if (flags08 & 0x20) {
                MEC_fn_vWalkManager(p_stDynamic, hSupObj, totalDt);
            } else if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100) {
                MEC_fn_vSpiderObstacleManagement(p_stDynamic, hSupObj, totalDt);
            } else if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x100000) {
                MEC_fn_vSwimObstacleManagement(p_stDynamic, hSupObj, totalDt);
            } else if (flags08 & 2) {
                MEC_fn_vBaseManager(p_stDynamic, hSupObj, totalDt);
            }
        }
    }

    p_stStablePreviousPosition = &stStablePreviousPosition;
    MEC_fn_vPositionLimit(p_stDynamic, p_stStablePreviousPosition);
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
        result27C = MEC_fn_vBasePlatformManagement(p_stDynamic, hSupObj);
    }
    MEC_fn_vCorrectMatrix(p_stDynamic, hCurrentMatrix);

    speedCC = pCCv30;
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4004) == 0x4004) {
        POS_tdstMatrix_* speedBase;
        f32* base30;
        speedBase = m280;
        base30 = (f32*) inv_POS_fn_p_stPosMatrixTranslation(speedBase);
        MTH3D_M_vSetXYZ_inv(&stComputedSpeed,
                            (inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix))->xX - base30[0],
                            speedCC->xY - base30[1], speedCC->xZ - base30[2]);
        stComputedSpeed.xX /= totalDt;
        stComputedSpeed.xY /= totalDt;
        stComputedSpeed.xZ /= totalDt;
        POS_fn_vGetYAxisVector(speedBase, &stYAxis);
        st.scalar = 1.0f;
        st.second = stYAxis;
        st.first = stComputedSpeed;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrSpeed.m_stLinear = st.first;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrSpeed.m_stAngular.m_xAngle =
            st.scalar;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrSpeed.m_stAngular.m_stAxis =
            st.second;
    }

    copied = 0;
    if (inv_POS_fn_bIsEqualMatrixEpsilon(ctxMat, hCurrentMatrix, 0.001f) == 0 || result27C != 0) {
        *ctxMat = *hCurrentMatrix;
        HIE_fn_vComputeNewRelativeMatrix(hSupObj);
        copied = 1;
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x01000000;
    }

    if (copied || inv_POS_fn_bIsEqualMatrixExact(hPreviousMatrix, hCurrentMatrix) == 0) {
        if (inv_fn_ucSuperObjectGetPlatFormType(hSupObj) != 0) {
            HIE_fn_vRefreshHierarchy(hSupObj);
            if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x20000000) {
                render = &stInvMatrix;
                POS_fn_vInvertMatrix(render, p_stRefMatrix);
            } else {
                if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x800) {
                    POS_fn_vInvertMatrix(hInvRefMatrix, p_stInvPreviousReferenceMatrix);
                    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x800;
                }
                render = hInvRefMatrix;
            }
            MEC_fn_vUpdatePlatform(p_stDynamic, hSupObj, p_stRefMatrix, render);
        } else if (fn_ulStandardGameGetCustomBitsSO(hSupObj) & 2) {
            HIE_fn_vRefreshHierarchy(hSupObj);
        } else {
            HIE_fn_vInvalidateHierarchyFrame(hSupObj);
        }
    }
    MEC_fn_vInitMatrix(p_stDynamic);
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4004) == 0x4004) {
        stMatrixTranslation = *(MTH3D_tdstVector_*) speedCC;
        st.scalar = 0.0f;
        st.second.xZ = st.second.xY = st.second.xX = st.scalar;
        st.first = stMatrixTranslation;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrPosition.m_stLinear = st.first;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrPosition.m_stAngular.m_xAngle =
            st.scalar;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrPosition.m_stAngular.m_stAxis =
            st.second;
    }
    MEC_fn_vFillCollisionReport(p_stDynamic);
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) {
        sub->m_ulPrevSurfaceState = sub->m_ulCurrSurfaceState;
    }

    p_stDynamic->m_tdstDynamicsBaseBlock.temporalCurveType = (u8) count;
    p24 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX;
    p30 = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xX;
    pVec6C = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation;
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xZ = pVec6C->xY =
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xX =
            p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xZ = pVec54->xY =
                p_stDynamic->m_tdstDynamicsBaseBlock.m_stSpeedAnim.xX =
                    p_stDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xZ = p30[1] =
                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xX =
                            p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xZ = p24[1] =
                                p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = 0.0f;

    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x4000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x8000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x10000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x20000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x40000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x80000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x100000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x200000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x400000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x800000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x1000000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x2000000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x20000000;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x80000000;

    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x10;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x1000;
    MEC_g_xBaseCoeff = 0.0f;
    MEC_g_xWalkCoeff = 0.0f;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
        if (inv_MEC_stReportObstacle != 0 && inv_MEC_stReportObstacle[-1] != 0) {
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide = MEC_fn_xGetSlide(p_stDynamic, (DNM_stMecObstacle*) inv_MEC_stReportObstacle);
        } else {
            *&p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide = 0;
        }
    }

    if (inv_MEC_stReportGround != 0 && (inv_MEC_stReportGround[-27] & 1) && MEC_fn_xGetSlide(p_stDynamic, (DNM_stMecObstacle*) inv_MEC_stReportGround) != 0.0f) {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x20000;
    } else {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x20000;
    }

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) {
        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xZ =
            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xY =
                p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xX =
                    p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xZ =
                        p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xY =
                            p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xX = 0.0f;
    }

    return p_stDynamic;
}

void MEC_vImposeRotationMatrix(DNM_stDynamics* p_stDynamic, POS_tdstMatrix_* hRotationMatrix)
{
    MTH3D_tdstVector_* wantedPosition;
    MTH3D_tdstVector_* currentPosition;
    MTH3D_tdstVector_ delta;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) == 0) {
        p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags | 0x20000000;
        POS_fn_vGetRotationMatrix(hRotationMatrix, &p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeRotationMatrix);

        currentPosition = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
        wantedPosition = (MTH3D_tdstVector_*) &hRotationMatrix->stMatrix.stCol_3;
        inv_MTH3D_fn_vSubXYZ(&delta, wantedPosition, currentPosition);
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation = delta;

        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
            MEC_fn_vCancelTilt(p_stDynamic);
        }
    }
}

void MEC_vImposeRotationVector(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stX, MTH3D_tdstVector_* p_stY, MTH3D_tdstVector_* p_stZ)
{
    f32* vectors;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) == 0) {
        vectors = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeRotationMatrix.stCol_0.xX;
        vectors[0] = p_stX->xX;
        vectors[1] = p_stX->xY;
        vectors[2] = p_stX->xZ;
        vectors[3] = p_stY->xX;
        vectors[4] = p_stY->xY;
        vectors[5] = p_stY->xZ;
        vectors[6] = p_stZ->xX;
        vectors[7] = p_stZ->xY;
        vectors[8] = p_stZ->xZ;
        p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x20000000;
    }
}

void MEC_vImposeRelativeRotation(DNM_stDynamics* p_stDynamic, MTH3D_tdstMatrix_* p_stRotationMatrix)
{
    u8 rotationMatrix[0x60];

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) == 0) {
        inv_POS_fn_vInitMatrixFromVectors((POS_tdstMatrix_*) rotationMatrix, 5, &p_stRotationMatrix->stCol_0, &p_stRotationMatrix->stCol_1,
                                          &p_stRotationMatrix->stCol_2, 0);
        _POS_fn_vMulMatrixMatrix(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                                 &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                                 (POS_tdstMatrix_*) rotationMatrix);
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4) {
            MEC_fn_vCancelTilt(p_stDynamic);
        }
    }
}

void MEC_vImposeTranslation(DNM_stDynamics* p_stDynamic, const MTH3D_tdstVector_* p_stVector)
{
    MTH3D_tdstVector_ delta;
    const MTH3D_tdstVector_* currentPosition;
    f32 zero;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) != 0) {
        return;
    }
    currentPosition = (const MTH3D_tdstVector_*) &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3.xX;
    p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x80000000;
    MTH3D_M_vSetXYZ_inv(&delta, p_stVector->xX - currentPosition->xX,
                        p_stVector->xY - currentPosition->xY, p_stVector->xZ - currentPosition->xZ);
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation = delta;
    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed, zero, zero, zero);
}

void MEC_vInitTranslation(DNM_stDynamics* p_stDynamic, HIE_tdstSuperObject_* hSupObj,
                          MTH3D_tdstVector_* p_stVector)
{
    POS_tdstMatrix_* matrix;
    POS_tdstMatrix_* hCurrentMatrix;
    POS_tdstMatrix_* hPreviousMatrix;
    MTH3D_tdstVector_* zeroB;
    MTH3D_tdstVector_* zeroA;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) != 0) {
        matrix = hSupObj->gu_hGlobalMatrix;
        if (matrix->ucType == MAT_eTransform_Identity) {
            matrix->ucType = MAT_eTransform_Translation;
        }
        *inv_POS_fn_p_stPosMatrixTranslation(matrix) = *p_stVector;
        matrix->stMatrix.stCol_3.xW = 1.0f;
    } else {
        hCurrentMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
        hPreviousMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix;
        matrix = hSupObj->gu_hGlobalMatrix;

        if (hCurrentMatrix->ucType == MAT_eTransform_Identity) {
            hCurrentMatrix->ucType = MAT_eTransform_Translation;
        }
        *inv_POS_fn_p_stPosMatrixTranslation(hCurrentMatrix) = *p_stVector;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3.xW = 1.0f;

        if (hPreviousMatrix->ucType == MAT_eTransform_Identity) {
            hPreviousMatrix->ucType = MAT_eTransform_Translation;
        }
        *inv_POS_fn_p_stPosMatrixTranslation(hPreviousMatrix) = *p_stVector;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3.xW = 1.0f;

        *matrix = *hCurrentMatrix;

        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) != 0) {
            POS_tdstMatrix_* hAbsolutePreviousMatrix =
                &p_stDynamic->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix;
            POS_tdstMatrix_* hPrevPreviousMatrix =
                &p_stDynamic->m_tdstDynamicsComplexBlock.m_stPrevPreviousMatrix;

            if (hAbsolutePreviousMatrix->ucType == MAT_eTransform_Identity) {
                hAbsolutePreviousMatrix->ucType = MAT_eTransform_Translation;
            }
            *inv_POS_fn_p_stPosMatrixTranslation(hAbsolutePreviousMatrix) = *p_stVector;
            p_stDynamic->m_tdstDynamicsComplexBlock.m_stAbsolutePreviousMatrix.stMatrix.stCol_3.xW =
                1.0f;

            if (hPrevPreviousMatrix->ucType == MAT_eTransform_Identity) {
                hPrevPreviousMatrix->ucType = MAT_eTransform_Translation;
            }
            *inv_POS_fn_p_stPosMatrixTranslation(hPrevPreviousMatrix) = *p_stVector;
            p_stDynamic->m_tdstDynamicsComplexBlock.m_stPrevPreviousMatrix.stMatrix.stCol_3.xW =
                1.0f;
        }
    }

    zeroA = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;
    zeroB = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation;

    HIE_fn_vComputeNewRelativeMatrix(hSupObj);

    {
        f32 zero = 0.0f;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX = zero;
        zeroA->xY = zero;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ = zero;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xX = zero;
        zeroB->xY = zero;
        p_stDynamic->m_tdstDynamicsBaseBlock.m_stAddTranslation.xZ = zero;

        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 2) != 0) {
            MTH3D_M_vSetXYZ_inv(&p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stInertiaTranslation,
                                zero, zero, zero);
        }
    }
}

void MEC_vTurnAroundZAxis(DNM_stDynamics* p_stDynamic, f32 xAngle)
{
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_ axis;
    MTH3D_tdstVector_ translation;
    MTH3D_tdstVector_ i;
    MTH3D_tdstVector_ j;
    MTH3D_tdstVector_ k;
    POS_tdstMatrix_ rotation;
    POS_tdstMatrix_* rotationMatrix;
    MTH4D_tdstVector_* translationColumn;
    f32 reduced;
    f32 cosine;
    f32 sine;
    f32 sign;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) {
        return;
    }
    if (xAngle == 0.0f) {
        return;
    }

    matrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 8) {
        rotationMatrix = &rotation;
        reduced =
            xAngle - (f32) inv_MTH_fn_lToInteger(xAngle * MTH_INV_TWO_PI_inv) * MTH_TWO_PI_inv;
        cosine = cosf(xAngle);
        if (__builtin_fabsf(reduced) < MTH_PI_inv) {
            sign = inv_MTH_fn_xSign(reduced);
        } else {
            sign = -inv_MTH_fn_xSign(reduced);
        }
        translationColumn = (MTH4D_tdstVector_*) &matrix->stMatrix.stCol_3.xX;
        translation = *(MTH3D_tdstVector_*) translationColumn;
        sine = MTH_fn_xSqrtOpt2(1.0f - cosine * cosine) * sign;
        gu_MTH4D_M_vSetIdentityMatrix(rotationMatrix);
        MTH3D_M_vSetXYZ_inv(&i, cosine, sine, 0.0f);
        MTH3D_M_vSetXYZ_inv(&j, inv_MTH_fn_xNegate(sine), cosine, 0.0f);
        MTH3D_M_vSetXYZ_inv(&k, i.xZ, i.xZ, 1.0f);
        _POS_fn_vSetRotationMatrix(rotationMatrix, &i, &j, &k);
        _POS_fn_vMulMatrixMatrix(matrix, rotationMatrix, matrix);
        if (matrix->ucType == MAT_eTransform_Identity) {
            matrix->ucType = MAT_eTransform_Translation;
        }
        *(MTH3D_tdstVector_*) translationColumn = translation;
        translationColumn->xW = 1.0f;
    } else {
        translationColumn = (MTH4D_tdstVector_*) &matrix->stMatrix.stCol_3.xX;
        translation = *(MTH3D_tdstVector_*) translationColumn;
        MTH3D_M_vSetXYZ_inv(&axis, 0.0f, 0.0f, -1.0f);
        POS_fn_vRotatePositionAroundAxis(matrix, &axis, xAngle);
        if (matrix->ucType == MAT_eTransform_Identity) {
            matrix->ucType = MAT_eTransform_Translation;
        }
        *(MTH3D_tdstVector_*) translationColumn = translation;
        translationColumn->xW = 1.0f;
    }
}

void MEC_vTurnAbsolute(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stVector)
{
    MTH3D_tdstMatrix_ buf;
    MTH3D_tdstVector_ VectTmp;
    POS_tdstMatrix_* matrix;
    f32 length_sq;
    f32 length;
    f32 xInvLength;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) == 0) {
        if (MTH3D_M_bIsZeroXYZ_inv(p_stVector)) {
            return;
        }
        matrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
        POS_fn_vGetRotationMatrix(matrix, &buf);
        length_sq = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stVector->xX, p_stVector->xY, p_stVector->xZ);
        length = MTH_fn_xSqrtOpt2(length_sq);
        if (length != 0.0f) {
            xInvLength = 1.0f / length;
            MTH3D_M_vKTimesXYZ_inv(&buf.stCol_1, xInvLength, &(*(MTH3D_tdstVector_*) p_stVector));
        } else {
            buf.stCol_1 = *(MTH3D_tdstVector_*) p_stVector;
        }
        MTH3D_M_vSetXYZ_inv(&buf.stCol_1, inv_MTH_fn_xNegate(buf.stCol_1.xX),
                            inv_MTH_fn_xNegate(buf.stCol_1.xY), inv_MTH_fn_xNegate(buf.stCol_1.xZ));

        if (&buf.stCol_0 == &buf.stCol_1 || &buf.stCol_0 == &buf.stCol_2) {
            DNM_p_stVectorCrossProduct(&VectTmp, &buf.stCol_1, &buf.stCol_2);
            buf.stCol_0 = VectTmp;
        } else {
            DNM_p_stVectorCrossProduct(&buf.stCol_0, &buf.stCol_1, &buf.stCol_2);
        }

        if (&buf.stCol_1 == &buf.stCol_2 || &buf.stCol_1 == &buf.stCol_0) {
            DNM_p_stVectorCrossProduct(&VectTmp, &buf.stCol_2, &buf.stCol_0);
            buf.stCol_1 = VectTmp;
        } else {
            DNM_p_stVectorCrossProduct(&buf.stCol_1, &buf.stCol_2, &buf.stCol_0);
        }

        inv_POS_fn_vInitMatrixFromVectors(matrix, 5, &buf.stCol_0, &buf.stCol_1, &buf.stCol_2,
                                          (MTH3D_tdstVector_*) &matrix->stMatrix.stCol_3);
        POS_fn_vSetScaleMatrix(matrix, &p_stDynamic->m_tdstDynamicsBaseBlock.m_stScale);
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 4) {
            MEC_fn_vCancelTilt(p_stDynamic);
        }
    }
}

void DNM_fn_vSetHangingLimit(DNM_stDynamics* object, f32 value)
{
    HIE_tdstSuperObject_* platform = object->m_tdstDynamicsComplexBlock.m_hPlatform;
    if (platform != 0) {
        POS_tdstMatrix_* platformMatrix = platform->gu_hGlobalMatrix;
        object->m_tdstDynamicsComplexBlock.m_xHangingLimit =
            value - platformMatrix->stMatrix.stCol_3.xZ;
    } else {
        object->m_tdstDynamicsComplexBlock.m_xHangingLimit = value;
    }
}

void MEC_fn_vUpdateSpeed(HIE_tdstSuperObject_* hSupObj)
{
    f32 xAngle;
    f32 angle;
    MTH_stMove* p_stMove;
    MTH3D_tdstVector_ stAxis;
    MTH3D_tdstVector_ stPreviousY;
    MTH3D_tdstVector_ stCurrentY;
    MTH3D_tdstVector_ cross;
    DNM_stDynamics* dynamics;
    tdstDynam_* component;

    component = ((tdstEngineObject_*) hSupObj->hLinkedObject)->h_Dynam;
    if (component == NULL) {
        return;
    }
    dynamics = component->p_stDynamics;
    if ((dynamics->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4004) != 0x4004) {
        return;
    }
    POS_fn_vGetYAxisVector(&dynamics->m_tdstDynamicsBaseBlock.m_stPreviousMatrix, &stAxis);
    p_stMove = &dynamics->m_tdstDynamicsBaseBlock.m_pstReport->m_stAbsoluteCurrSpeed;
    stPreviousY = p_stMove->m_stAngular.m_stAxis;
    xAngle = inv_MTH3D_fn_xDotXYZ(&stPreviousY, &stAxis);
    DNM_p_stVectorCrossProduct(&cross, &stPreviousY, &stAxis);
    xAngle = inv_MTH_fn_xMinimum(inv_MTH_fn_xMaximum(xAngle, -1.0f), 1.0f);
    if (cross.xZ > 0.0f) {
        angle = acosf(xAngle);
    } else {
        angle = -acosf(xAngle);
    }
    stCurrentY.xY = stCurrentY.xX = 0.0f;
    stCurrentY.xZ = 1.0f;
    p_stMove->m_stAngular.m_xAngle = angle;
    p_stMove->m_stAngular.m_stAxis = stCurrentY;
}
