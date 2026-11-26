#include "common.h"

#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"

f32 acosf(f32 x);
f32 cosf(f32 x);

void MTH_fn_vVectorPlaneProjection(MTH3D_tdstVector_* _p_stResult, MTH3D_tdstVector_* _p_stSource,
                                   MTH3D_tdstVector_* _p_stPlaneNorm, MTH3D_tdstVector_* _p_stProjDirection)
{
    MTH3D_tdstVector_ buffer;
    MTH3D_tdstVector_* offset = &buffer;
    f32 directionDot;
    f32 sourceDot;
    f32 alpha;

    if (_p_stProjDirection != _p_stPlaneNorm) {
        directionDot =
            inv_MTH_fn_xMulAdd3(_p_stProjDirection->xX, _p_stPlaneNorm->xX, _p_stProjDirection->xY,
                                _p_stPlaneNorm->xY, _p_stProjDirection->xZ, _p_stPlaneNorm->xZ);
    } else {
        directionDot = 1.0f;
    }
    if (directionDot != 0.0f) {
        sourceDot = inv_MTH_fn_xMulAdd3(_p_stSource->xX, _p_stPlaneNorm->xX, _p_stSource->xY,
                             _p_stPlaneNorm->xY, _p_stSource->xZ, _p_stPlaneNorm->xZ);
        alpha = inv_MTH_fn_xNegateTied(sourceDot);
        alpha /= directionDot;
        *offset = *_p_stProjDirection;
        inv_MTH3D_fn_vKTimesXYZ(offset, alpha, offset);
        if (_p_stResult != _p_stSource) {
            *_p_stResult = *_p_stSource;
        }
        inv_MTH3D_fn_vAddXYZ(_p_stResult, _p_stResult, offset);
    }
}

void* MTH_p_stRotationComputeRotationWith2Vectors(MTH_stRotation* _p_stResult, MTH3D_tdstVector_* _p_stSrcUnitVector,
                                                  MTH3D_tdstVector_* _p_stImaUnitVector)
{
    MTH3D_tdstVector_* pAxis = &_p_stResult->m_stAxis;
    f32 ang;
    f32 len2;
    f32 len;
    f32 inv;
    f32 dot;

    DNM_p_stVectorCrossProduct(pAxis, _p_stSrcUnitVector, _p_stImaUnitVector);

    if (__builtin_fabsf(pAxis->xX) < 0.00001f && __builtin_fabsf(pAxis->xY) < 0.00001f &&
        __builtin_fabsf(pAxis->xZ) < 0.00001f) {
        if (__builtin_fabsf(_p_stSrcUnitVector->xX - _p_stImaUnitVector->xX) < 0.00001f &&
            __builtin_fabsf(_p_stSrcUnitVector->xY - _p_stImaUnitVector->xY) < 0.00001f &&
            __builtin_fabsf(_p_stSrcUnitVector->xZ - _p_stImaUnitVector->xZ) < 0.00001f) {
            _p_stResult->m_xAngle = 0;
            ang = _p_stResult->m_xAngle;
            _p_stResult->m_stAxis.xX = ang;
            pAxis->xY = ang;
            _p_stResult->m_stAxis.xZ = ang;
        } else {
            _p_stResult->m_xAngle = MTH_PI_inv;
        }
    } else {
        len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(pAxis->xX, pAxis->xY, pAxis->xZ);
        len = MTH_fn_xSqrtOpt2(len2);
        if (len != 0.0f) {
            inv = 1.0f / len;
            inv_MTH3D_fn_vKTimesXYZ(pAxis, inv, pAxis);
        }
        dot = inv_MTH3D_fn_xDotXYZ(_p_stSrcUnitVector, _p_stImaUnitVector);
        if (dot <= -1.0f) {
            _p_stResult->m_xAngle = MTH_PI_inv;
        } else if (1.0f <= dot) {
            _p_stResult->m_xAngle = 0.0f;
            _p_stResult->m_stAxis.xX = 0.0f;
            pAxis->xY = 0.0f;
            _p_stResult->m_stAxis.xZ = 0.0f;
        } else {
            _p_stResult->m_xAngle = acosf(dot);
        }
    }
    return _p_stResult;
}

static inline void inv_DNM_fn_vRotationCrossDirect(MTH3D_tdstVector_* d, MTH3D_tdstVector_* a,
                                                   MTH3D_tdstVector_* b)
{
    DNM_p_stVectorCrossProduct(&(*(MTH3D_tdstVector_*) d), &(*(MTH3D_tdstVector_*) a), &(*(MTH3D_tdstVector_*) b));
}

static inline void inv_DNM_fn_vRotationCross(MTH3D_tdstVector_* d, MTH3D_tdstVector_* a,
                                             MTH3D_tdstVector_* b)
{
    if (d == a || d == b) {
        MTH3D_tdstVector_ tmp;
        inv_DNM_fn_vRotationCrossDirect(&tmp, a, b);
        *d = tmp;
    } else {
        inv_DNM_fn_vRotationCrossDirect(d, a, b);
    }
}

MTH3D_tdstVector_* MTH_p_stRotationMulVector(MTH_stRotation* _p_stRotation, MTH3D_tdstVector_* _p_stSource,
                                              MTH3D_tdstVector_* _p_stResult)
{
    f32 angle;
    f32 reduced;
    f32 cosine;
    f32 sine;
    f32 sgn;
    f32 dot;
    s32 ni;
    MTH3D_tdstVector_* axis;
    MTH3D_tdstVector_ cross;
    MTH3D_tdstVector_ axisTerm;

    angle = _p_stRotation->m_xAngle;
    ni = inv_MTH_fn_lToInteger(angle * MTH_INV_TWO_PI_inv);
    reduced = angle - (f32) ni * MTH_TWO_PI_inv;
    cosine = cosf(angle);

    if (__builtin_fabsf(reduced) < MTH_PI_inv) {
        sgn = inv_MTH_fn_xSign(reduced);
    } else {
        sgn = -inv_MTH_fn_xSign(reduced);
    }
    sine = MTH_fn_xSqrtOpt2(1.0f - cosine * cosine) * sgn;

    axis = &_p_stRotation->m_stAxis;
    inv_DNM_fn_vRotationCross(&cross, &(*axis), &(*_p_stSource));
    MTH3D_M_vKTimesXYZ_inv(&cross, sine, &cross);

    dot = inv_MTH_fn_xMulAdd3Grouped(axis->xX, axis->xY, axis->xZ,
                           _p_stSource->xX, _p_stSource->xY, _p_stSource->xZ);
    axisTerm = *axis;
    dot = dot * (1.0f - cosine);
    MTH3D_M_vKTimesXYZ_inv(&axisTerm, dot, &axisTerm);

    if (_p_stSource != _p_stResult) {
        *_p_stResult = *_p_stSource;
    }
    MTH3D_M_vSetXYZ_inv(_p_stResult, cosine * _p_stResult->xX + axisTerm.xX + cross.xX,
                        cosine * _p_stResult->xY + axisTerm.xY + cross.xY,
                        cosine * _p_stResult->xZ + axisTerm.xZ + cross.xZ);
    return _p_stResult;
}
