#include "common.h"

#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/ee_f32_minmax.h"

#define DYNAMICS_INVERSE_DIRTY 0x800

void POS_fn_vGetYAxisVector(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* axis);
void POS_fn_vGetRotationVector(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* axis);

void MEC_fn_vInertiaCoeff(DNM_stDynamics* p_stDynamic, f32 xDT, f32 xInertiaX, f32 xInertiaY,
                   POS_tdstMatrix_* hMatrix, f32 xInertiaZ, POS_tdstMatrix_* hInvMatrix)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ velocityDelta;
    MTH3D_tdstVector_ adjustedDelta;
    MTH3D_tdstVector_ newPosition;
    MTH4D_tdstVector_* nextPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    MTH4D_tdstVector_* currentPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    POS_tdstMatrix_* nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    u32 flags;
    f32 precision;
    f32 frequency;

    delta.xX = nextPosition->xX - currentPosition->xX;
    delta.xY = nextPosition->xY - currentPosition->xY;
    flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
    delta.xZ = nextPosition->xZ - currentPosition->xZ;
    if ((flags & DYNAMICS_INVERSE_DIRTY) != 0) {
        POS_fn_vInvertMatrix(hInvMatrix, hMatrix);
        flags = p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags;
        flags &= ~DYNAMICS_INVERSE_DIRTY;
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = flags;
    }
    {
        MTH3D_tdstVector_* speed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed;

        inv_MTH3D_fn_vKTimesXYZ(&velocityDelta, xDT, speed);
    }
    if ((flags & 0x10) == 0) {
        MTH3D_M_vSubXYZ_inv(&velocityDelta, &velocityDelta,
                            &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stInertiaTranslation);
    }
    POS_fn_vRotateVector(&delta, hInvMatrix, &delta);
    POS_fn_vRotateVector(&velocityDelta, hInvMatrix, &velocityDelta);
    frequency = 1.0f / xDT;
    precision = (frequency - 60.0f) * 1e-06f / 140.0f + (200.0f - frequency) * 0.0001f / 140.0f;
    MTH3D_M_vSetXYZ_inv(&adjustedDelta, xInertiaX * (velocityDelta.xX - delta.xX) + delta.xX,
                        xInertiaY * (velocityDelta.xY - delta.xY) + delta.xY,
                        xInertiaZ * (velocityDelta.xZ - delta.xZ) + delta.xZ);
    POS_fn_vRotateVector(&adjustedDelta, hMatrix, &adjustedDelta);
    MEC_fn_vRoundVector(&adjustedDelta, precision);
    MTH3D_M_vAddXYZ_inv(&newPosition, currentPosition, &adjustedDelta);
    if (nextMatrix->ucType == MAT_eTransform_Identity) {
        nextMatrix->ucType = MAT_eTransform_Translation;
    }
    (*(MTH3D_tdstVector_*) nextPosition) = newPosition;
    nextPosition->xW = 1.0f;
}

void MEC_fn_vGravitySpeed(DNM_stDynamics* p_stDynamic, f32 xDT)
{
    DNM_stDynamicsBase_* base = &p_stDynamic->m_tdstDynamicsBaseBlock;
    f32 speed = base->m_xGravity;

    if ((base->ulEndFlags & 0x8000) == 0) {
        POS_tdstMatrix_* matrix = &base->m_stCurrentMatrix;
        f32 z;

        speed = -speed;
        speed *= xDT;
        speed += base->m_stPreviousSpeed.xZ;
        speed *= xDT;
        z = base->m_stPreviousMatrix.stMatrix.stCol_3.xZ + speed;
        if (matrix->ucType == MAT_eTransform_Identity)
            matrix->ucType = MAT_eTransform_Translation;
        matrix->stMatrix.stCol_3.xZ = z;
    }
}

void MEC_fn_vInertia(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* hMatrix,
                   POS_tdstMatrix_* hInvMatrix)
{
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 scale;
    f32 scaledX;
    f32 scaledY;
    f32 scaledZ;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x400) != 0) {
        scaleX = MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaX, 1);
        scaleY = MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaY, 1);
        scaleZ = MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaZ, 1);
        MEC_fn_vInertiaCoeff(p_stDynamic, xDT, scaleX, scaleY, hMatrix, scaleZ, hInvMatrix);
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x20) != 0 &&
        (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x20000) != 0) {
        scale = MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xPreviousSlide, 1);
        scaledX =
            scale * MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorX, 0);
        scaledY =
            scale * MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorY, 0);
        scaledZ =
            scale * MEC_fn_xConvertScale(p_stDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorZ, 0);
        MEC_fn_vInertiaCoeff(p_stDynamic, xDT, scaledX, scaledY, hMatrix, scaledZ, hInvMatrix);
    }
}

void MEC_fn_vAddTiltSpeed(DNM_stDynamics* p_stDynamic, f32 xDt)
{
    MTH3D_tdstVector_ xAxis;
    MTH3D_tdstVector_ yAxis;
    MTH3D_tdstVector_ rotationAxis;
    MTH3D_tdstVector_ saved;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ pivot;
    MTH3D_tdstVector_ sum;
    POS_tdstMatrix_* matrix;
    MTH4D_tdstVector_* translation;
    f32 weight;
    f32 oneMinusWeight;
    f32 dot;
    f32 speed;
    f32 ten;
    f32 current;
    f32 zero;
    f32 target;
    f32 angle;
    f32 scale;

    matrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    weight = p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltInertia;
    inv_POS_fn_vGetXAxisVector(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix, &xAxis);
    POS_fn_vGetYAxisVector(matrix, &yAxis);
    POS_fn_vGetRotationVector(matrix, &rotationAxis);
    oneMinusWeight = 1.0f - weight;
    dot = inv_MTH3D_fn_xDotXYZ(&xAxis, &yAxis);
    dot = dot * p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltIntensity;
    speed = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xX,
                                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xY,
                                        p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousSpeed.xZ);
    ten = 10.0f;
    speed = MTH_fn_xSqrtOpt2(speed);
    angle = dot * speed;
    angle = angle / ten;
    target = p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltAngle;
    current = target;
    zero = 0.0f;
    target = current * weight + angle * oneMinusWeight;
    angle = target - current;
    if (angle == zero) {
        return;
    }
    translation = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    scale = p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltOrigin;
    inv_MTH3D_fn_vKTimesXYZ(&pivot, scale, &rotationAxis);
    p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltAngle = target;
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 1.0f, 0.0f);
    saved = (*(MTH3D_tdstVector_*) translation);
    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    (*(MTH3D_tdstVector_*) translation) = pivot;
    translation->xW = 1.0f;
    POS_fn_vRotatePositionAroundAxis(matrix, &up, angle);
    MTH3D_M_vSubXYZ_inv(&up, translation, &pivot);
    inv_MTH3D_fn_vAddXYZ(&sum, &saved, &up);
    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    (*(MTH3D_tdstVector_*) translation) = sum;
    translation->xW = 1.0f;
}

void MEC_fn_vCancelTilt(DNM_stDynamics* p_stDynamic)
{
    f32 tilt;
    f32 scale;
    f32 angle;
    POS_tdstMatrix_* matrix;
    MTH4D_tdstVector_* translation;
    MTH3D_tdstVector_ saved;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ rotationAxis;
    MTH3D_tdstVector_ pivot;

    tilt = p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltAngle;
    if (tilt == 0.0f) {
        return;
    }
    matrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    translation = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    POS_fn_vGetRotationVector(matrix, &rotationAxis);
    scale = p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltOrigin;
    angle = -tilt;
    inv_MTH3D_fn_vKTimesXYZ(&pivot, scale, &rotationAxis);
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 1.0f, 0.0f);
    saved = (*(MTH3D_tdstVector_*) translation);
    MTH3D_M_vSubXYZ_inv(translation, translation, &pivot);
    POS_fn_vRotatePositionAroundAxis(matrix, &up, angle);
    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    (*(MTH3D_tdstVector_*) translation) = saved;
    translation->xW = 1.0f;
    p_stDynamic->m_tdstDynamicsComplexBlock.m_xTiltAngle = 0.0f;
}

void MEC_fn_vImposeSpeed(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* matrix,
                         POS_tdstMatrix_* inverseMatrix)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ velocityDelta;
    MTH3D_tdstVector_ newPosition;
    MTH3D_tdstVector_* imposedSpeed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed;
    MTH4D_tdstVector_* nextPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    POS_tdstMatrix_* nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH4D_tdstVector_* currentPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    u32 optionFlags;

    inv_MTH3D_fn_vKTimesXYZ(&velocityDelta, xDT, imposedSpeed);
    delta.xX = nextPosition->xX - currentPosition->xX;
    delta.xY = nextPosition->xY - currentPosition->xY;
    optionFlags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
    delta.xZ = nextPosition->xZ - currentPosition->xZ;
    if ((optionFlags & 0x4000) == 0 && (!MTH3D_M_bIsZeroXYZ_inv(&delta))) {
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_INVERSE_DIRTY) != 0) {
            POS_fn_vInvertMatrix(inverseMatrix, matrix);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_INVERSE_DIRTY;
        }
        POS_fn_vRotateVector(&delta, inverseMatrix, &delta);
        optionFlags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
    }
    if ((optionFlags & 0x20000) != 0) {
        delta.xX = velocityDelta.xX;
    }
    if ((optionFlags & 0x40000) != 0) {
        delta.xY = velocityDelta.xY;
    }
    if ((optionFlags & 0x80000) != 0) {
        delta.xZ = velocityDelta.xZ;
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x4000) == 0 &&
        (!MTH3D_M_bIsZeroXYZ_inv(&delta))) {
        POS_fn_vRotateVector(&delta, matrix, &delta);
    }
    MTH3D_M_vAddXYZ_inv(&newPosition, currentPosition, &delta);
    inv_POS_fn_vSetTranslation(nextMatrix, &newPosition);
}

void MEC_fn_vAddSpeed(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* hMatrix,
                      POS_tdstMatrix_* hInvMatrix)
{
    MTH3D_tdstVector_ diff;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ newPosition;
    POS_tdstMatrix_* nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH4D_tdstVector_* currentPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;

    {
        MTH4D_tdstVector_* nextPosition;
        MTH3D_tdstVector_* speed;

        speed = &p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed;
        nextPosition = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
        inv_MTH3D_fn_vKTimesXYZ(&delta, xDT, speed);
        MTH3D_M_vSubXYZ_inv(&diff, nextPosition, currentPosition);
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x10000) == 0 &&
        (!MTH3D_M_bIsZeroXYZ_inv(&diff))) {
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_INVERSE_DIRTY) != 0) {
            POS_fn_vInvertMatrix(hInvMatrix, hMatrix);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_INVERSE_DIRTY;
        }
        POS_fn_vRotateVector(&diff, hInvMatrix, &diff);
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x800000) != 0) {
        diff.xX += delta.xX;
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x1000000) != 0) {
        diff.xY += delta.xY;
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x2000000) != 0) {
        diff.xZ += delta.xZ;
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x10000) == 0 &&
        (!MTH3D_M_bIsZeroXYZ_inv(&diff))) {
        POS_fn_vRotateVector(&diff, hMatrix, &diff);
    }
    MTH3D_M_vAddXYZ_inv(&newPosition, currentPosition, &diff);
    inv_POS_fn_vSetTranslation(nextMatrix, &newPosition);
}

void MEC_fn_vProposeSpeed(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* matrix,
                          POS_tdstMatrix_* inverseMatrix)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ velocityDelta;
    MTH3D_tdstVector_ newPosition;
    MTH3D_tdstVector_* proposedSpeed = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed;
    MTH4D_tdstVector_* nextPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    POS_tdstMatrix_* nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH4D_tdstVector_* currentPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    u32 optionFlags;

    inv_MTH3D_fn_vKTimesXYZ(&velocityDelta, xDT, proposedSpeed);
    delta.xX = nextPosition->xX - currentPosition->xX;
    delta.xY = nextPosition->xY - currentPosition->xY;
    optionFlags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
    delta.xZ = nextPosition->xZ - currentPosition->xZ;
    if ((optionFlags & 0x8000) == 0 && (!MTH3D_M_bIsZeroXYZ_inv(&delta))) {
        if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_INVERSE_DIRTY) != 0) {
            POS_fn_vInvertMatrix(inverseMatrix, matrix);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_INVERSE_DIRTY;
        }
        POS_fn_vRotateVector(&delta, inverseMatrix, &delta);
        optionFlags = p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags;
    }
    if ((optionFlags & 0x100000) != 0) {
        delta.xX = velocityDelta.xX;
    }
    if ((optionFlags & 0x200000) != 0) {
        delta.xY = velocityDelta.xY;
    }
    if ((optionFlags & 0x400000) != 0) {
        delta.xZ = velocityDelta.xZ;
    }
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x8000) == 0 &&
        (!MTH3D_M_bIsZeroXYZ_inv(&delta))) {
        POS_fn_vRotateVector(&delta, matrix, &delta);
    }
    MTH3D_M_vAddXYZ_inv(&newPosition, currentPosition, &delta);
    inv_POS_fn_vSetTranslation(nextMatrix, &newPosition);
}

#ifdef PC_PORT
void MEC_fn_vAddStream(DNM_stDynamics* p_stDynamic, f32 dt, u8 bSlide)
#else
void MEC_fn_vAddStream(DNM_stDynamics* p_stDynamic, f32 xDT, u8 bSlide)
#endif
{
    POS_tdstMatrix_* matrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    MTH3D_tdstVector_ translation;

#ifdef PC_PORT
    MEC_fn_vComputeStream(&translation, p_stDynamic, bSlide, dt);
#else
    MEC_fn_vComputeStream(&translation, p_stDynamic, bSlide, xDT);
#endif
    inv_POS_fn_vTranslate(matrix, &translation);
}

void MEC_fn_vImposeRotation(DNM_stDynamics* dynamics)
{
    POS_fn_vSetRotationMatrix(&dynamics->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                              &dynamics->m_tdstDynamicsBaseBlock.m_stImposeRotationMatrix);
}

void MEC_fn_vAdditionnalTranslation(DNM_stDynamics* dynamics)
{
    inv_POS_fn_vTranslate(&dynamics->m_tdstDynamicsBaseBlock.m_stCurrentMatrix,
                                 &dynamics->m_tdstDynamicsBaseBlock.m_stAddTranslation);
}

void MEC_fn_vPositionLimit(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stSafePreviousPosition)
{
    POS_tdstMatrix_* nextMatrix;
    MTH3D_tdstVector_* currentPosition;
    MTH3D_tdstVector_* nextPosition;
    f32 low;
    f32 next;
    f32 high;
    f32 bound;

    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x1C000000) == 0) {
        return;
    }
    nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    currentPosition = inv_POS_fn_p_stPosMatrixTranslation(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix);
    nextPosition = inv_POS_fn_p_stPosMatrixTranslation(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix);
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x04000000) {
        low = currentPosition->xX;
        next = nextPosition->xX;
        high = inv_MTH_fn_xMaximum(low, next);
        low = inv_MTH_fn_xMinimum(low, next);
        bound = p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit.xX;
        if (low <= bound && bound < high) {
            f32 value = p_stSafePreviousPosition->xX;

            if (nextMatrix->ucType == MAT_eTransform_Identity) {
                nextMatrix->ucType = MAT_eTransform_Translation;
            }
            nextMatrix->stMatrix.stCol_3.xX = value;
        }
    }
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x08000000) {
        low = currentPosition->xY;
        next = nextPosition->xY;
        high = inv_MTH_fn_xMaximum(low, next);
        low = inv_MTH_fn_xMinimum(low, next);
        bound = p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit.xY;
        if (low <= bound && bound < high) {
            f32 value = p_stSafePreviousPosition->xY;

            if (nextMatrix->ucType == MAT_eTransform_Identity) {
                nextMatrix->ucType = MAT_eTransform_Translation;
            }
            nextMatrix->stMatrix.stCol_3.xY = value;
        }
    }
    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x10000000) {
        low = currentPosition->xZ;
        next = nextPosition->xZ;
        R5900_FMAX_VOLATILE(high, low, next);
        low = inv_MTH_fn_xMinimum(low, next);
        bound = p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit.xZ;
        if (low <= bound && bound < high) {
            f32 value = p_stSafePreviousPosition->xZ;

            if (nextMatrix->ucType == MAT_eTransform_Identity) {
                nextMatrix->ucType = MAT_eTransform_Translation;
            }
            nextMatrix->stMatrix.stCol_3.xZ = value;
        }
    }
}

void MEC_fn_vSpeedLimit(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* hMatrix,
                        POS_tdstMatrix_* hInvMatrix)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ limitedDelta;
    MTH3D_tdstVector_ limit;
    MTH3D_tdstVector_ newPosition;
    MTH4D_tdstVector_* nextPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    MTH4D_tdstVector_* currentPosition =
        &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    POS_tdstMatrix_* nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;

    MTH3D_M_vSubXYZ_inv(&delta, nextPosition, currentPosition);
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_INVERSE_DIRTY) != 0) {
        POS_fn_vInvertMatrix(hInvMatrix, hMatrix);
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_INVERSE_DIRTY;
    }
    POS_fn_vRotateVector(&delta, hInvMatrix, &delta);
    limit = p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stMaxSpeed;
    inv_MTH3D_fn_vKTimesXYZ(&limit, xDT, &limit);
    limitedDelta = delta;
    if (limit.xX < __builtin_fabsf(delta.xX)) {
        limitedDelta.xX = limit.xX * inv_MTH_fn_xSign(delta.xX);
    }
    if (limit.xY < __builtin_fabsf(delta.xY)) {
        limitedDelta.xY = limit.xY * inv_MTH_fn_xSign(delta.xY);
    }
    if (limit.xZ < __builtin_fabsf(delta.xZ)) {
        limitedDelta.xZ = limit.xZ * inv_MTH_fn_xSign(delta.xZ);
    }
    POS_fn_vRotateVector(&limitedDelta, hMatrix, &limitedDelta);
    MTH3D_M_vAddXYZ_inv(&newPosition, currentPosition, &limitedDelta);
    inv_POS_fn_vSetTranslation(nextMatrix, &newPosition);
}

void MEC_fn_vAdjustScaleData(DNM_stDynamics* p_stDynamic, POS_tdstMatrix_* hMatrix,
                             POS_tdstMatrix_* hInvMatrix)
{
    MTH3D_tdstVector_ vector;
    MTH3D_tdstVector_ newPosition;
    MTH4D_tdstVector_* currentPosition;
    MTH4D_tdstVector_* nextPosition;
    POS_tdstMatrix_* nextMatrix;

    nextMatrix = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix;
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_INVERSE_DIRTY) != 0) {
        POS_fn_vInvertMatrix(hInvMatrix, hMatrix);
        p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_INVERSE_DIRTY;
    }
    currentPosition = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stPreviousMatrix.stMatrix.stCol_3;
    nextPosition = &p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix.stMatrix.stCol_3;
    MTH3D_M_vSubXYZ_inv(&vector, nextPosition, currentPosition);
    POS_fn_vRotateVector(&vector, hInvMatrix, &vector);
    MEC_fn_vAdjustScaleVector(p_stDynamic, &vector);
    POS_fn_vRotateVector(&vector, hMatrix, &vector);
    MTH3D_M_vAddXYZ_inv(&newPosition, currentPosition, &vector);
    if (nextMatrix->ucType == MAT_eTransform_Identity) {
        nextMatrix->ucType = MAT_eTransform_Translation;
    }
    *inv_POS_fn_p_stPosMatrixTranslation(&p_stDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix) = newPosition;
    nextPosition->xW = 1.0f;

    vector = p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed;
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x4000) != 0) {
        POS_fn_vRotateVector(&vector, hInvMatrix, &vector);
    }
    MEC_fn_vAdjustScaleVector(p_stDynamic, &vector);
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x4000) != 0) {
        POS_fn_vRotateVector(&vector, hMatrix, &vector);
    }
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed = vector;

    vector = p_stDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed;
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x8000) != 0) {
        POS_fn_vRotateVector(&vector, hInvMatrix, &vector);
    }
    MEC_fn_vAdjustScaleVector(p_stDynamic, &vector);
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x4000) != 0) {
        POS_fn_vRotateVector(&vector, hMatrix, &vector);
    }
    p_stDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed = vector;

    vector = p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed;
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x10000) != 0) {
        POS_fn_vRotateVector(&vector, hInvMatrix, &vector);
    }
    MEC_fn_vAdjustScaleVector(p_stDynamic, &vector);
    if ((p_stDynamic->m_tdstDynamicsBaseBlock.ulFlags & 0x4000) != 0) {
        POS_fn_vRotateVector(&vector, hMatrix, &vector);
    }
    p_stDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed = vector;
}
