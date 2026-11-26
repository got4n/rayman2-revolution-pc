#include "common.h"

#include "MTH/MTH.h"

#include "ps2/r5900.h"

#include "include_asm.h"
#include "GAM/GAM.h"

#define MAT_SCALE_BIAS_POINT(out, in, scale, bias)                                                 \
    __asm__("lwc1 $f1, 0(%2)\n\tlwc1 $f2, 0(%1)\n\tlwc1 $f0, 0(%3)\n\t"                          \
            "mul.s $f2, $f1, $f2\n\tlwc1 $f4, 4(%2)\n\tlwc1 $f5, 4(%1)\n\t"                       \
            "add.s $f0, $f2, $f0\n\tmul.s $f5, $f4, $f5\n\tlwc1 $f3, 4(%3)\n\t"                   \
            "swc1 $f0, 0(%0)\n\tlwc1 $f4, 8(%2)\n\tadd.s $f3, $f5, $f3\n\t"                       \
            "lwc1 $f1, 8(%1)\n\tlwc1 $f0, 8(%3)\n\tmul.s $f1, $f4, $f1\n\t"                       \
            "swc1 $f3, 4(%0)\n\tadd.s $f0, $f1, $f0\n\tswc1 $f0, 8(%0)"                           \
            :                                                                                      \
            : "r"(out), "r"(in), "r"(scale), "r"(bias)                                             \
            : "$f0", "$f1", "$f2", "$f3", "$f4", "$f5")

static inline void inv_POS_fn_vSetVector3(MTH4D_tdstVector_* v, f32 x, f32 y, f32 z)
{
    MTH3D_M_vSetXYZ_inv(v, x, y, z);
    v->xW = 0.0f;
}

static inline void inv_POS_fn_vSetVector4(MTH4D_tdstVector_* v, f32 x, f32 y, f32 z, f32 w)
{
    MTH3D_M_vSetXYZ_inv(v, x, y, z);
    v->xW = w;
}

static inline void inv_POS_fn_vCrossColumn(MTH4D_tdstVector_* r, MTH4D_tdstVector_* a, MTH4D_tdstVector_* b)
{
    DNM_p_stVectorCrossProduct(r, a, b);
    r->xW = 1.0f;
}

static inline void inv_POS_fn_vNormalizeColumn(MTH4D_tdstVector_* v)
{
    f32 xNorm;
    f32 xInvNorm;

    xNorm = MTH_fn_xSqrAddSqrAddSqrOpt2(v->xX, v->xY, v->xZ);
    xInvNorm = inv_MTH_fn_xDivideBySqrt(1.0f, xNorm);
    MTH3D_M_vKTimesXYZ_inv(v, xInvNorm, v);
    v->xW = 1.0f;
}

extern f32 flt_393490[];
extern POS_tdstMatrix_ inv_POS_stIdentityMatrix[];

extern POS_tdstMatrix_* inv_POS_hIdentityMatrix;

void POS_fn_vInitPos(void)
{
    POS_tdstMatrix_* hIdentity;

    hIdentity = inv_POS_stIdentityMatrix;
    inv_POS_hIdentityMatrix = hIdentity;
    gu_MTH4D_M_vSetIdentityMatrix(hIdentity);
}

void func_00120D60(POS_tdstMatrix_** p_hIdentity)
{
    *p_hIdentity = inv_POS_hIdentityMatrix;
}

POS_tdstMatrix_* func_00120D70(void)
{
    return inv_POS_hIdentityMatrix;
}

void gu_MTH4D_M_vSetIdentityMatrix(POS_tdstMatrix_* _hTransform)
{
    MTH4D_tdstVector_* p_stScale = &_hTransform->stScale;

    _hTransform->ucType = MAT_eTransform_Identity;
    p_stScale->xX = 1.0f;
    MTH4D_fn_vSetIdentityMatrix(&_hTransform->stMatrix);
    p_stScale->xY = 1.0f;
    p_stScale->xZ = 1.0f;
    p_stScale->xW = 1.0f;
}

s32 func_00120DE0(POS_tdstMatrix_* _hTransform, f32 _xEps)
{
    f32* p_xMatrix;
    f32* p_xReference;

    if (_hTransform->ucType == MAT_eTransform_Identity)
        return 1;

    p_xMatrix = &_hTransform->stMatrix.stCol_0.xX;
    p_xReference = &inv_POS_hIdentityMatrix->stMatrix.stCol_0.xX;
    if (inv_MTH_fn_xAbsoluteTied(p_xMatrix[0] - p_xReference[0]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[1] - p_xReference[1]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[2] - p_xReference[2]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[3] - p_xReference[3]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[4] - p_xReference[4]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[5] - p_xReference[5]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[6] - p_xReference[6]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[7] - p_xReference[7]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[8] - p_xReference[8]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[9] - p_xReference[9]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[10] - p_xReference[10]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[11] - p_xReference[11]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[12] - p_xReference[12]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[13] - p_xReference[13]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[14] - p_xReference[14]) < _xEps &&
        inv_MTH_fn_xAbsoluteTied(p_xMatrix[15] - p_xReference[15]) < _xEps) {
        return 1;
    }
    return 0;
}

void POS_fn_vMulMatrixVertexArray(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform,
                                  MTH3D_tdstVector_* _p_stSource, u32 _ulNumber)
{
    MTH3D_tdstVector_ stSource;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
        if (_p_stDest != _p_stSource) {
            while (_ulNumber--) {
                *_p_stDest++ = *_p_stSource++;
            }
        }
        break;
    case MAT_eTransform_Translation: {
        f32* p_xTranslation = &_hTransform->stMatrix.stCol_3.xX;
        while (_ulNumber--) {
            (_p_stDest++)->xX = p_xTranslation[0] + (_p_stSource++)->xX;
            (_p_stDest++)->xY = p_xTranslation[1] + (_p_stSource++)->xY;
            (_p_stDest++)->xZ = p_xTranslation[2] + (_p_stSource++)->xZ;
        }
        break;
    }
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale: {
        f32* p_xBias = &_hTransform->stMatrix.stCol_3.xX;
        f32* p_xScale = &_hTransform->stScale.xX;
        while (_ulNumber--) {
            MAT_SCALE_BIAS_POINT(_p_stDest, _p_stSource, p_xScale, p_xBias);
            _p_stSource++;
            _p_stDest++;
        }
        break;
    }
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        while (_ulNumber--) {
            MTH3D_tdstVector_* p_stDest = _p_stDest;
            MTH3D_tdstVector_* p_stSource = _p_stSource;
            _p_stSource++;
            _p_stDest++;
            if (p_stSource == p_stDest) {
                stSource = *p_stSource;
                R5900_TRANSFORM_POINT_inv(p_stDest, &stSource, _hTransform);
            } else {
                R5900_TRANSFORM_POINT_inv(p_stDest, p_stSource, _hTransform);
            }
        }
        break;
    }
}

void POS_fn_vMulMatrixVertex(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform,
                              MTH3D_tdstVector_* _p_stSource)
{
    MTH3D_tdstVector_ stSource;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
        if (_p_stDest != _p_stSource) {
            *_p_stDest = *_p_stSource;
        }
        break;
    case MAT_eTransform_Translation:
        MTH3D_M_vAddXYZ_inv(_p_stDest, &_hTransform->stMatrix.stCol_3, _p_stSource);
        break;
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale: {
        f32* p_xBias;
        f32* p_xScale;

        p_xBias = &_hTransform->stMatrix.stCol_3.xX;
        p_xScale = &_hTransform->stScale.xX;
        MAT_SCALE_BIAS_POINT(_p_stDest, _p_stSource, p_xScale, p_xBias);
        break;
    }
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        if (_p_stSource == _p_stDest) {
            stSource = *_p_stSource;
            R5900_TRANSFORM_POINT_inv(_p_stDest, &stSource, _hTransform);
        } else {
            R5900_TRANSFORM_POINT_inv(_p_stDest, _p_stSource, _hTransform);
        }
        break;
    }
}

void POS_fn_vApplyMatrixToVector(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform,
                                 MTH3D_tdstVector_* _p_stSource)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
        if (_p_stDest != _p_stSource) {
            *_p_stDest = *_p_stSource;
        }
        break;
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        MTH3D_M_vSetXYZ_inv(_p_stDest, _p_stSource->xX * _hTransform->stScale.xX,
                            _p_stSource->xY * _hTransform->stScale.xY,
                            _p_stSource->xZ * _hTransform->stScale.xZ);
        break;
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale: {
        if (_p_stSource == _p_stDest) {
            MTH3D_tdstVector_ stSource = *_p_stDest;
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(p_stMatrix->stCol_0.xX, p_stMatrix->stCol_1.xX,
                                        p_stMatrix->stCol_2.xX, stSource.xX, stSource.xY,
                                        stSource.xZ);
                _p_stDest->xX = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(p_stMatrix->stCol_0.xY, p_stMatrix->stCol_1.xY,
                                        p_stMatrix->stCol_2.xY, stSource.xX, stSource.xY,
                                        stSource.xZ);
                _p_stDest->xY = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(p_stMatrix->stCol_0.xZ, p_stMatrix->stCol_1.xZ,
                                        p_stMatrix->stCol_2.xZ, stSource.xX, stSource.xY,
                                        stSource.xZ);
                _p_stDest->xZ = xDot;
            }
        } else {
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xX, _p_stSource->xX,
                                           p_stMatrix->stCol_1.xX, _p_stSource->xY,
                                           p_stMatrix->stCol_2.xX, _p_stSource->xZ);
                _p_stDest->xX = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xY, _p_stSource->xX,
                                           p_stMatrix->stCol_1.xY, _p_stSource->xY,
                                           p_stMatrix->stCol_2.xY, _p_stSource->xZ);
                _p_stDest->xY = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xZ, _p_stSource->xX,
                                           p_stMatrix->stCol_1.xZ, _p_stSource->xY,
                                           p_stMatrix->stCol_2.xZ, _p_stSource->xZ);
                _p_stDest->xZ = xDot;
            }
        }
        break;
    }
    }
}

void POS_fn_vRotateVector(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform,
                          MTH3D_tdstVector_* _p_stSource)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        if (_p_stDest != _p_stSource) {
            *_p_stDest = *_p_stSource;
        }
        break;
    case MAT_eTransform_Rotation:
        if (_p_stSource == _p_stDest) {
            MTH3D_tdstVector_ stSource = *_p_stDest;
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(p_stMatrix->stCol_0.xX, p_stMatrix->stCol_1.xX,
                                        p_stMatrix->stCol_2.xX, stSource.xX, stSource.xY,
                                        stSource.xZ);
                _p_stDest->xX = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(p_stMatrix->stCol_0.xY, p_stMatrix->stCol_1.xY,
                                        p_stMatrix->stCol_2.xY, stSource.xX, stSource.xY,
                                        stSource.xZ);
                _p_stDest->xY = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(p_stMatrix->stCol_0.xZ, p_stMatrix->stCol_1.xZ,
                                        p_stMatrix->stCol_2.xZ, stSource.xX, stSource.xY,
                                        stSource.xZ);
                _p_stDest->xZ = xDot;
            }
        } else {
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xX, _p_stSource->xX,
                                           p_stMatrix->stCol_1.xX, _p_stSource->xY,
                                           p_stMatrix->stCol_2.xX, _p_stSource->xZ);
                _p_stDest->xX = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xY, _p_stSource->xX,
                                           p_stMatrix->stCol_1.xY, _p_stSource->xY,
                                           p_stMatrix->stCol_2.xY, _p_stSource->xZ);
                _p_stDest->xY = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xZ, _p_stSource->xX,
                                           p_stMatrix->stCol_1.xZ, _p_stSource->xY,
                                           p_stMatrix->stCol_2.xZ, _p_stSource->xZ);
                _p_stDest->xZ = xDot;
            }
        }
        break;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale: {
        POS_tdstMatrix_ stTemp;
        inv_POS_fn_vRemoveScaleFromMatrix(&stTemp, _hTransform);
        if (_p_stSource == _p_stDest) {
            MTH3D_tdstVector_ stSource = *_p_stDest;
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(
                    stTemp.stMatrix.stCol_0.xX, stTemp.stMatrix.stCol_1.xX,
                    stTemp.stMatrix.stCol_2.xX, stSource.xX, stSource.xY, stSource.xZ);
                _p_stDest->xX = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(
                    stTemp.stMatrix.stCol_0.xY, stTemp.stMatrix.stCol_1.xY,
                    stTemp.stMatrix.stCol_2.xY, stSource.xX, stSource.xY, stSource.xZ);
                _p_stDest->xY = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(
                    stTemp.stMatrix.stCol_0.xZ, stTemp.stMatrix.stCol_1.xZ,
                    stTemp.stMatrix.stCol_2.xZ, stSource.xX, stSource.xY, stSource.xZ);
                _p_stDest->xZ = xDot;
            }
        } else {
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(
                    stTemp.stMatrix.stCol_0.xX, stTemp.stMatrix.stCol_1.xX,
                    stTemp.stMatrix.stCol_2.xX, _p_stSource->xX, _p_stSource->xY, _p_stSource->xZ);
                _p_stDest->xX = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(
                    stTemp.stMatrix.stCol_0.xY, stTemp.stMatrix.stCol_1.xY,
                    stTemp.stMatrix.stCol_2.xY, _p_stSource->xX, _p_stSource->xY, _p_stSource->xZ);
                _p_stDest->xY = xDot;
            }
            {
                f32 xDot;
                xDot = inv_MTH_fn_xMulAdd3Grouped(
                    stTemp.stMatrix.stCol_0.xZ, stTemp.stMatrix.stCol_1.xZ,
                    stTemp.stMatrix.stCol_2.xZ, _p_stSource->xX, _p_stSource->xY, _p_stSource->xZ);
                _p_stDest->xZ = xDot;
            }
        }
        break;
    }
    case MAT_eTransform_ComplexRotationScale:
        break;
    }
}

static inline void inv_POS_fn_vTransposeRotationInPlace(MTH4D_tdstMatrix_* p_stMatrix)
{
    f32 x;

    x = p_stMatrix->stCol_0.xY;
    p_stMatrix->stCol_0.xY = p_stMatrix->stCol_1.xX;
    p_stMatrix->stCol_1.xX = x;
    x = p_stMatrix->stCol_0.xZ;
    p_stMatrix->stCol_0.xZ = p_stMatrix->stCol_2.xX;
    p_stMatrix->stCol_2.xX = x;
    x = p_stMatrix->stCol_1.xZ;
    p_stMatrix->stCol_1.xZ = p_stMatrix->stCol_2.xY;
    p_stMatrix->stCol_2.xY = x;
}

static inline void inv_POS_fn_vTransposeRotation(MTH4D_tdstMatrix_* p_stDest, MTH4D_tdstMatrix_* p_stSource)
{
    if (p_stDest == p_stSource) {
        inv_POS_fn_vTransposeRotationInPlace(p_stDest);
    } else {
        MTH3D_M_vSetXYZ_inv(&p_stDest->stCol_0, p_stSource->stCol_0.xX, p_stSource->stCol_1.xX,
                            p_stSource->stCol_2.xX);
        p_stDest->stCol_0.xW = p_stSource->stCol_0.xW;
        MTH3D_M_vSetXYZ_inv(&p_stDest->stCol_1, p_stSource->stCol_0.xY, p_stSource->stCol_1.xY,
                            p_stSource->stCol_2.xY);
        p_stDest->stCol_1.xW = p_stSource->stCol_1.xW;
        MTH3D_M_vSetXYZ_inv(&p_stDest->stCol_2, p_stSource->stCol_0.xZ, p_stSource->stCol_1.xZ,
                            p_stSource->stCol_2.xZ);
        p_stDest->stCol_2.xW = p_stSource->stCol_2.xW;
        MTH3D_M_vCopyXYZ_inv(&p_stDest->stCol_3, &p_stSource->stCol_3);
        p_stDest->stCol_3.xW = p_stSource->stCol_3.xW;
    }
}

void POS_fn_vInvertMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hSource)
{
    MTH4D_tdstVector_ tempVector0;
    MTH4D_tdstVector_ tempVector1;
    POS_tdstMatrix_ stTemp;

    {
        switch (_hSource->ucType) {
        case MAT_eTransform_Identity:
            if (_hDest != _hSource) {
                gu_MTH4D_M_vSetIdentityMatrix(_hDest);
            }
            break;

        case MAT_eTransform_Translation:
            if (_hDest != _hSource) {
                gu_MTH4D_M_vSetIdentityMatrix(_hDest);
                _hDest->ucType = MAT_eTransform_Translation;
            }
            MTH4D_fn_vNegVector(&_hDest->stMatrix.stCol_3, &_hSource->stMatrix.stCol_3);
            break;

        case MAT_eTransform_Zoom:
        case MAT_eTransform_Scale: {
            MTH4D_tdstVector_* p_stScale = &_hDest->stScale;
            f32 xInvX;
            f32 xInvY;
            f32 xInvZ;
            if (_hDest != _hSource) {
                *_hDest = *_hSource;
            }
            {
                MTH4D_tdstVector_* pDst = &_hDest->stMatrix.stCol_3;
                MTH4D_tdstVector_* pSrc = &_hSource->stMatrix.stCol_3;
                xInvX = 1.0f / _hSource->stMatrix.stCol_0.xX;
                _hDest->stMatrix.stCol_0.xX = xInvX;
                p_stScale->xX = xInvX;
                xInvY = 1.0f / _hSource->stMatrix.stCol_1.xY;
                _hDest->stMatrix.stCol_1.xY = xInvY;
                p_stScale->xY = xInvY;
                xInvZ = 1.0f / _hSource->stMatrix.stCol_2.xZ;
                _hDest->stMatrix.stCol_2.xZ = xInvZ;
                p_stScale->xZ = xInvZ;
                MTH3D_M_vSetXYZ_inv(pDst, inv_MTH_fn_xNegateTied(pSrc->xX),
                                    inv_MTH_fn_xNegateTied(pSrc->xY),
                                    inv_MTH_fn_xNegateTied(pSrc->xZ));
                pDst->xW = 1.0f;
                MTH3D_M_vSetXYZ_inv(pDst, p_stScale->xX * pDst->xX, p_stScale->xY * pDst->xY,
                                    p_stScale->xZ * pDst->xZ);
                pDst->xW = 1.0f;
            }
            break;
        }

        case MAT_eTransform_Rotation: {
            MTH4D_tdstVector_* pDst = &_hDest->stMatrix.stCol_3;

            _hDest->ucType = MAT_eTransform_Rotation;
            inv_POS_fn_vTransposeRotation(&_hDest->stMatrix, &_hSource->stMatrix);

            {
                MTH4D_tdstVector_* pSrc = &_hSource->stMatrix.stCol_3;
                MTH4D_tdstVector_* p_stScale = &_hDest->stScale;
                MTH3D_M_vSetXYZ_inv(pDst, inv_MTH_fn_xNegateTied(pSrc->xX),
                                    inv_MTH_fn_xNegateTied(pSrc->xY),
                                    inv_MTH_fn_xNegateTied(pSrc->xZ));
                pDst->xW = 1.0f;
                tempVector0 = *pDst;
                MTH3D_M_vSetXYZ_inv(
                    pDst,
                    inv_MTH_fn_xMulAdd3Grouped(_hDest->stMatrix.stCol_0.xX,
                                               _hDest->stMatrix.stCol_1.xX,
                                               _hDest->stMatrix.stCol_2.xX, tempVector0.xX,
                                               tempVector0.xY, tempVector0.xZ),
                    inv_MTH_fn_xMulAdd3Grouped(_hDest->stMatrix.stCol_0.xY,
                                               _hDest->stMatrix.stCol_1.xY,
                                               _hDest->stMatrix.stCol_2.xY, tempVector0.xX,
                                               tempVector0.xY, tempVector0.xZ),
                    inv_MTH_fn_xMulAdd3Grouped(_hDest->stMatrix.stCol_0.xZ,
                                               _hDest->stMatrix.stCol_1.xZ,
                                               _hDest->stMatrix.stCol_2.xZ, tempVector0.xX,
                                               tempVector0.xY, tempVector0.xZ));
                pDst->xW = 1.0f;
                MTH3D_M_vSetXYZ_inv(p_stScale, 1.0f, 1.0f, 1.0f);
                p_stScale->xW = 1.0f;
            }
            break;
        }

        case MAT_eTransform_RotationZoom: {
            MTH4D_tdstVector_* pDst3 = &_hDest->stMatrix.stCol_3;
            f32 xZoom;
            f32 xZero;

            _hDest->ucType = MAT_eTransform_RotationZoom;
            inv_POS_fn_vTransposeRotation(&_hDest->stMatrix, &_hSource->stMatrix);

            {
                MTH4D_tdstVector_* pDst1 = &_hDest->stMatrix.stCol_1;
                MTH4D_tdstVector_* pDst2 = &_hDest->stMatrix.stCol_2;
                MTH4D_tdstVector_* p_stScale = &_hDest->stScale;
                MTH4D_tdstVector_* pSrc3 = &_hSource->stMatrix.stCol_3;

                xZoom = 1.0f / (_hSource->stScale.xX * _hSource->stScale.xX);
                xZero = 0.0f;
                MTH3D_M_vKTimesXYZ_inv(&_hDest->stMatrix.stCol_0, xZoom, &_hDest->stMatrix.stCol_0);
                _hDest->stMatrix.stCol_0.xW = 1.0f;
                MTH3D_M_vKTimesXYZ_inv(pDst1, xZoom, pDst1);
                pDst1->xW = 1.0f;
                MTH3D_M_vKTimesXYZ_inv(pDst2, xZoom, pDst2);
                pDst2->xW = 1.0f;
                *(volatile f32*) &_hDest->stMatrix.stCol_0.xW = xZero;
                *(volatile f32*) &_hDest->stMatrix.stCol_2.xW = xZero;
                *(volatile f32*) &_hDest->stMatrix.stCol_1.xW = xZero;

                xZoom = 1.0f / _hSource->stScale.xX;
                MTH3D_M_vSetXYZ_inv(p_stScale, xZoom, xZoom, xZoom);
                p_stScale->xW = 1.0f;

                MTH3D_M_vSetXYZ_inv(pDst3, -pSrc3->xX, -pSrc3->xY,
                                    inv_MTH_fn_xNegateTied(pSrc3->xZ));
                pDst3->xW = 1.0f;
                tempVector0 = *pDst3;
                MTH3D_M_vSetXYZ_inv(
                    pDst3,
                    inv_MTH_fn_xMulAdd3Grouped(_hDest->stMatrix.stCol_0.xX,
                                               _hDest->stMatrix.stCol_1.xX,
                                               _hDest->stMatrix.stCol_2.xX, tempVector0.xX,
                                               tempVector0.xY, tempVector0.xZ),
                    inv_MTH_fn_xMulAdd3Grouped(_hDest->stMatrix.stCol_0.xY,
                                               _hDest->stMatrix.stCol_1.xY,
                                               _hDest->stMatrix.stCol_2.xY, tempVector0.xX,
                                               tempVector0.xY, tempVector0.xZ),
                    inv_MTH_fn_xMulAdd3Grouped(_hDest->stMatrix.stCol_0.xZ,
                                               _hDest->stMatrix.stCol_1.xZ,
                                               _hDest->stMatrix.stCol_2.xZ, tempVector0.xX,
                                               tempVector0.xY, tempVector0.xZ));
                pDst3->xW = 1.0f;
            }
            break;
        }

        case MAT_eTransform_RotationScale: {
            MTH4D_tdstVector_* pSrc1 = &_hSource->stMatrix.stCol_1;
            MTH4D_tdstVector_* pDst1 = &_hDest->stMatrix.stCol_1;
            MTH4D_tdstVector_* pSrc2 = &_hSource->stMatrix.stCol_2;
            MTH4D_tdstVector_* pDst2 = &_hDest->stMatrix.stCol_2;
            MTH4D_tdstVector_* pSrc3 = &_hSource->stMatrix.stCol_3;
            MTH4D_tdstVector_* pDst3 = &_hDest->stMatrix.stCol_3;

            _hDest->ucType = MAT_eTransform_ComplexRotationScale;
            MTH3D_M_vSetXYZ_inv(&tempVector0, 1.0f / (_hSource->stScale.xX * _hSource->stScale.xX),
                                1.0f / (_hSource->stScale.xY * _hSource->stScale.xY),
                                1.0f / (_hSource->stScale.xZ * _hSource->stScale.xZ));

            MTH3D_M_vSetXYZ_inv(&_hDest->stMatrix.stCol_0,
                                tempVector0.xX * _hSource->stMatrix.stCol_0.xX,
                                tempVector0.xX * _hSource->stMatrix.stCol_0.xY,
                                tempVector0.xX * _hSource->stMatrix.stCol_0.xZ);
            _hDest->stMatrix.stCol_0.xW = 1.0f;
            pDst1->xX = tempVector0.xY * pSrc1->xX;
            pDst1->xY = tempVector0.xY * pSrc1->xY;
            {
                f32 z = pSrc1->xZ;
                pDst1->xW = 1.0f;
                *(volatile f32*) &pDst1->xZ = tempVector0.xY * z;
            }
            {
                f32 invZ = *(volatile f32*) &tempVector0.xZ;
                MTH3D_M_vKTimesXYZ_inv(pDst2, invZ, pSrc2);
                pDst2->xW = 1.0f;
            }
            *pDst3 = *pSrc3;
            _hDest->stMatrix.stCol_2.xW = 0.0f;
            _hDest->stMatrix.stCol_1.xW = 0.0f;
            _hDest->stMatrix.stCol_0.xW = 0.0f;

            {
                inv_POS_fn_vTransposeRotationInPlace(&_hDest->stMatrix);
            }

            MTH3D_M_vSetXYZ_inv(pDst3, -pSrc3->xX, -pSrc3->xY, inv_MTH_fn_xNegateTied(pSrc3->xZ));
            pDst3->xW = 1.0f;
            tempVector1 = *pDst3;
            {
                f32 x, y;
                x = inv_MTH_fn_xMulAdd3Grouped(
                    _hDest->stMatrix.stCol_0.xX, _hDest->stMatrix.stCol_1.xX,
                    _hDest->stMatrix.stCol_2.xX, tempVector1.xX, tempVector1.xY, tempVector1.xZ);
                y = inv_MTH_fn_xMulAdd3Grouped(
                    _hDest->stMatrix.stCol_0.xY, _hDest->stMatrix.stCol_1.xY,
                    _hDest->stMatrix.stCol_2.xY, tempVector1.xX, tempVector1.xY, tempVector1.xZ);
                pDst3->xX = x;
                pDst3->xY = y;
                {
                    f32 az = *(volatile f32*) &_hDest->stMatrix.stCol_2.xZ;
                    f32 ax = *(volatile f32*) &_hDest->stMatrix.stCol_0.xZ;
                    f32 bx = *(volatile f32*) &tempVector1.xX;
                    f32 ay = *(volatile f32*) &_hDest->stMatrix.stCol_1.xZ;
                    f32 by = *(volatile f32*) &tempVector1.xY;
                    f32 bz = *(volatile f32*) &tempVector1.xZ;
                    pDst3->xZ = inv_MTH_fn_xMulAdd3(ax, bx, ay, by, az, bz);
                }
                pDst3->xW = 1.0f;
            }
            break;
        }

        case MAT_eTransform_ComplexRotationScale: {
            POS_tdstMatrix_* hResult;
            MTH4D_tdstVector_* pOut1;
            MTH4D_tdstVector_* pOut2;
            MTH4D_tdstVector_* pSrc3;
            MTH4D_tdstVector_* pOut3;
            f32 xDet;
            f32 xC22;

            hResult = &stTemp;
            if (_hDest != _hSource) {
                hResult = _hDest;
            }
            hResult->ucType = MAT_eTransform_ComplexRotationScale;
            pOut1 = &hResult->stMatrix.stCol_1;
            pOut2 = &hResult->stMatrix.stCol_2;
            pSrc3 = &_hSource->stMatrix.stCol_3;
            pOut3 = &hResult->stMatrix.stCol_3;

            {
                f32 m00, m01;
                hResult->stMatrix.stCol_0.xX = inv_MTH_fn_xMulSub(
                    _hSource->stMatrix.stCol_1.xY, _hSource->stMatrix.stCol_2.xZ,
                    _hSource->stMatrix.stCol_1.xZ, _hSource->stMatrix.stCol_2.xY);
                m00 = _hSource->stMatrix.stCol_0.xX;
                hResult->stMatrix.stCol_1.xX = inv_MTH_fn_xMulSub(
                    _hSource->stMatrix.stCol_1.xZ, _hSource->stMatrix.stCol_2.xX,
                    _hSource->stMatrix.stCol_1.xX, _hSource->stMatrix.stCol_2.xZ);
                xDet = m00 * hResult->stMatrix.stCol_0.xX;
                m01 = _hSource->stMatrix.stCol_0.xY;
                hResult->stMatrix.stCol_2.xX = inv_MTH_fn_xMulSub(
                    _hSource->stMatrix.stCol_1.xX, _hSource->stMatrix.stCol_2.xY,
                    _hSource->stMatrix.stCol_1.xY, _hSource->stMatrix.stCol_2.xX);
                xDet += m01 * hResult->stMatrix.stCol_1.xX;
            }
            xDet += _hSource->stMatrix.stCol_0.xZ * hResult->stMatrix.stCol_2.xX;
            xDet = 1.0f / xDet;

            hResult->stMatrix.stCol_0.xY =
                inv_MTH_fn_xMulSub(_hSource->stMatrix.stCol_0.xZ, _hSource->stMatrix.stCol_2.xY,
                                   _hSource->stMatrix.stCol_0.xY, _hSource->stMatrix.stCol_2.xZ);
            hResult->stMatrix.stCol_0.xZ =
                inv_MTH_fn_xMulSub(_hSource->stMatrix.stCol_0.xY, _hSource->stMatrix.stCol_1.xZ,
                                   _hSource->stMatrix.stCol_0.xZ, _hSource->stMatrix.stCol_1.xY);
            hResult->stMatrix.stCol_1.xY =
                inv_MTH_fn_xMulSub(_hSource->stMatrix.stCol_0.xX, _hSource->stMatrix.stCol_2.xZ,
                                   _hSource->stMatrix.stCol_0.xZ, _hSource->stMatrix.stCol_2.xX);
            hResult->stMatrix.stCol_1.xZ =
                inv_MTH_fn_xMulSub(_hSource->stMatrix.stCol_0.xZ, _hSource->stMatrix.stCol_1.xX,
                                   _hSource->stMatrix.stCol_0.xX, _hSource->stMatrix.stCol_1.xZ);
            hResult->stMatrix.stCol_2.xY =
                inv_MTH_fn_xMulSub(_hSource->stMatrix.stCol_0.xY, _hSource->stMatrix.stCol_2.xX,
                                   _hSource->stMatrix.stCol_0.xX, _hSource->stMatrix.stCol_2.xY);
            xC22 = inv_MTH_fn_xMulSub(_hSource->stMatrix.stCol_0.xX, _hSource->stMatrix.stCol_1.xY,
                                      _hSource->stMatrix.stCol_0.xY, _hSource->stMatrix.stCol_1.xX);

            hResult->stMatrix.stCol_0.xY = xDet * hResult->stMatrix.stCol_0.xY;
            hResult->stMatrix.stCol_0.xZ = xDet * hResult->stMatrix.stCol_0.xZ;
            hResult->stMatrix.stCol_0.xW = 1.0f;
            hResult->stMatrix.stCol_0.xX = xDet * hResult->stMatrix.stCol_0.xX;
            hResult->stMatrix.stCol_2.xZ = xC22;
            hResult->stMatrix.stCol_1.xX = xDet * hResult->stMatrix.stCol_1.xX;
            pOut1->xW = 1.0f;
            pOut1->xY = xDet * pOut1->xY;
            pOut1->xZ = xDet * pOut1->xZ;
            hResult->stMatrix.stCol_2.xX = xDet * hResult->stMatrix.stCol_2.xX;
            pOut2->xW = 1.0f;
            pOut2->xY = xDet * pOut2->xY;
            pOut2->xZ = xDet * pOut2->xZ;

            hResult->stMatrix.stCol_2.xW = 0.0f;
            hResult->stMatrix.stCol_1.xW = 0.0f;
            hResult->stMatrix.stCol_0.xW = 0.0f;

            MTH3D_M_vSetXYZ_inv(pOut3, -pSrc3->xX, -pSrc3->xY, inv_MTH_fn_xNegateTied(pSrc3->xZ));
            pOut3->xW = 1.0f;
            tempVector1 = *pOut3;
            {
                f32 bx = tempVector1.xX;
                f32 by = tempVector1.xY;
                f32 bz = tempVector1.xZ;
                pOut3->xX = inv_MTH_fn_xMulAdd3Grouped(hResult->stMatrix.stCol_0.xX,
                                             hResult->stMatrix.stCol_1.xX,
                                             hResult->stMatrix.stCol_2.xX, bx, by, bz);
            }
            pOut3->xY = inv_MTH_fn_xMulAdd3Grouped(
                hResult->stMatrix.stCol_0.xY, hResult->stMatrix.stCol_1.xY,
                hResult->stMatrix.stCol_2.xY, tempVector1.xX, tempVector1.xY, tempVector1.xZ);
            {
                f32 bx = tempVector1.xX;
                f32 by = tempVector1.xY;
                f32 bz = tempVector1.xZ;
                f32 ay = hResult->stMatrix.stCol_1.xZ;
                f32 az = hResult->stMatrix.stCol_2.xZ;
                pOut3->xZ =
                    inv_MTH_fn_xMulAdd3Grouped(hResult->stMatrix.stCol_0.xZ, ay, az, bx, by, bz);
            }
            pOut3->xW = 1.0f;

            MTH3D_M_vSetXYZ_inv(pOut3, -pSrc3->xX, -pSrc3->xY, -pSrc3->xZ);
            pOut3->xW = 1.0f;
            tempVector1 = *pOut3;
            MTH3D_M_vSetXYZ_inv(
                pOut3,
                inv_MTH_fn_xMulAdd3Grouped(
                    hResult->stMatrix.stCol_0.xX, hResult->stMatrix.stCol_1.xX,
                    hResult->stMatrix.stCol_2.xX, tempVector1.xX, tempVector1.xY, tempVector1.xZ),
                inv_MTH_fn_xMulAdd3Grouped(
                    hResult->stMatrix.stCol_0.xY, hResult->stMatrix.stCol_1.xY,
                    hResult->stMatrix.stCol_2.xY, *(volatile f32*) &tempVector1.xX,
                    *(volatile f32*) &tempVector1.xY, tempVector1.xZ),
                inv_MTH_fn_xMulAdd3(*(volatile f32*) &hResult->stMatrix.stCol_0.xZ, tempVector1.xX,
                                    hResult->stMatrix.stCol_1.xZ, tempVector1.xY,
                                    hResult->stMatrix.stCol_2.xZ, tempVector1.xZ));
            pOut3->xW = 1.0f;

            if (_hDest == _hSource) {
                *_hDest = *hResult;
            }
            break;
        }
        }
    }
}

static inline float inv_POS_fn_xDot3Add(float ax, float bx, float ay, float by, float az, float bz, float c)
{
    float xResult;
    __asm__("mula.s %1, %2\n\t"
            "madda.s %3, %4\n\t"
            "madd.s %0, %5, %6\n\t"
            "add.s %0, %0, %7"
            : "=f"(xResult)
            : "f"(ax), "f"(bx), "f"(ay), "f"(by), "f"(az), "f"(bz), "f"(c));
    return xResult;
}

static inline void inv_POS_fn_vScaleVector(MTH4D_tdstVector_* d, f32 s, MTH4D_tdstVector_* v)
{
    MTH3D_M_vKTimesXYZ_inv(d, s, v);
    d->xW = 1.0f;
}

static inline void inv_POS_fn_vMultiplyVectors(MTH4D_tdstVector_* d, MTH4D_tdstVector_* a, MTH4D_tdstVector_* b)
{
    MTH3D_M_vSetXYZ_inv(d, a->xX * b->xX, a->xY * b->xY, a->xZ * b->xZ);
    d->xW = 1.0f;
}

static inline void inv_POS_fn_vSetOnes(MTH4D_tdstVector_* v)
{
    MTH3D_M_vSetXYZ_inv(v, 1.0f, 1.0f, 1.0f);
    v->xW = 1.0f;
}

static inline void inv_POS_fn_vScaleMatrix(POS_tdstMatrix_* r, POS_tdstMatrix_* m, MTH4D_tdstVector_* s)
{
    inv_POS_fn_vScaleVector(&r->stMatrix.stCol_0, s->xX, &m->stMatrix.stCol_0);
    inv_POS_fn_vScaleVector(&r->stMatrix.stCol_1, s->xY, &m->stMatrix.stCol_1);
    inv_POS_fn_vScaleVector(&r->stMatrix.stCol_2, s->xZ, &m->stMatrix.stCol_2);
    r->stMatrix.stCol_3 = m->stMatrix.stCol_3;
    r->stMatrix.stCol_2.xW = 0.0f;
    r->stMatrix.stCol_1.xW = 0.0f;
    r->stMatrix.stCol_0.xW = 0.0f;
}

static inline void MTH4D_fn_vSpecializedMulMatrixMatrixWithoutBuffer(MTH4D_tdstMatrix_* d,
                                                                     MTH4D_tdstMatrix_* a,
                                                                     MTH4D_tdstMatrix_* b)
{
    MTH3D_M_vSetXYZ_inv(&d->stCol_0,
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xX, b->stCol_0.xX, a->stCol_1.xX,
                                            b->stCol_0.xY, a->stCol_2.xX, b->stCol_0.xZ),
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xY, b->stCol_0.xX, a->stCol_1.xY,
                                            b->stCol_0.xY, a->stCol_2.xY, b->stCol_0.xZ),
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xZ, b->stCol_0.xX, a->stCol_1.xZ,
                                            b->stCol_0.xY, a->stCol_2.xZ, b->stCol_0.xZ));
    MTH3D_M_vSetXYZ_inv(&d->stCol_1,
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xX, b->stCol_1.xX, a->stCol_1.xX,
                                            b->stCol_1.xY, a->stCol_2.xX, b->stCol_1.xZ),
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xY, b->stCol_1.xX, a->stCol_1.xY,
                                            b->stCol_1.xY, a->stCol_2.xY, b->stCol_1.xZ),
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xZ, b->stCol_1.xX, a->stCol_1.xZ,
                                            b->stCol_1.xY, a->stCol_2.xZ, b->stCol_1.xZ));
    MTH3D_M_vSetXYZ_inv(&d->stCol_2,
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xX, b->stCol_2.xX, a->stCol_1.xX,
                                            b->stCol_2.xY, a->stCol_2.xX, b->stCol_2.xZ),
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xY, b->stCol_2.xX, a->stCol_1.xY,
                                            b->stCol_2.xY, a->stCol_2.xY, b->stCol_2.xZ),
                        inv_MTH_fn_xMulAdd3(a->stCol_0.xZ, b->stCol_2.xX, a->stCol_1.xZ,
                                            b->stCol_2.xY, a->stCol_2.xZ, b->stCol_2.xZ));
    MTH3D_M_vSetXYZ_inv(
        &d->stCol_3,
        inv_POS_fn_xDot3Add(a->stCol_0.xX, b->stCol_3.xX, a->stCol_1.xX, b->stCol_3.xY,
                            a->stCol_2.xX, b->stCol_3.xZ, a->stCol_3.xX),
        inv_POS_fn_xDot3Add(a->stCol_0.xY, b->stCol_3.xX, a->stCol_1.xY, b->stCol_3.xY,
                            a->stCol_2.xY, b->stCol_3.xZ, a->stCol_3.xY),
        inv_POS_fn_xDot3Add(a->stCol_0.xZ, b->stCol_3.xX, a->stCol_1.xZ, b->stCol_3.xY,
                            a->stCol_2.xZ, b->stCol_3.xZ, a->stCol_3.xZ));
    d->stCol_3.xW = 1.0f;
    d->stCol_2.xW = 0.0f;
    d->stCol_1.xW = 0.0f;
    d->stCol_0.xW = 0.0f;
}

static inline u8 inv_POS_fn_bIsUniform(MTH4D_tdstVector_* v)
{
    if (__builtin_fabsf(v->xX - v->xY) < 0.0001f) {
        if (__builtin_fabsf(v->xX - v->xZ) < 0.0001f) {
            return 1;
        }
    }
    return 0;
}

static inline void inv_POS_fn_vAddVectors(MTH4D_tdstVector_* d, MTH4D_tdstVector_* a, MTH4D_tdstVector_* b)
{
    MTH3D_M_vAddXYZ_inv(d, a, b);
    d->xW = 1.0f;
}

static inline f32 inv_POS_fn_xDot4(f32 ax, f32 bx, f32 ay, f32 by, f32 az, f32 bz, f32 aw, f32 bw)
{
    f32 xResult;

    xResult = inv_MTH_fn_xMulAdd4Grouped(ax, ay, az, aw, bx, by, bz, bw);
    return xResult;
}

static inline void MTH4D_fn_vMulMatrixVectorWithoutBuffer(MTH4D_tdstVector_* VectDest,
                                                          MTH4D_tdstMatrix_* MatA,
                                                          MTH4D_tdstVector_* VectA)
{
    MTH3D_M_vSetXYZ_inv(VectDest,
                        inv_POS_fn_xDot4(MatA->stCol_0.xX, VectA->xX, MatA->stCol_1.xX, VectA->xY,
                                         MatA->stCol_2.xX, VectA->xZ, MatA->stCol_3.xX, VectA->xW),
                        inv_POS_fn_xDot4(MatA->stCol_0.xY, VectA->xX, MatA->stCol_1.xY, VectA->xY,
                                         MatA->stCol_2.xY, VectA->xZ, MatA->stCol_3.xY, VectA->xW),
                        inv_POS_fn_xDot4(MatA->stCol_0.xZ, VectA->xX, MatA->stCol_1.xZ, VectA->xY,
                                         MatA->stCol_2.xZ, VectA->xZ, MatA->stCol_3.xZ, VectA->xW));
    VectDest->xW = inv_POS_fn_xDot4(MatA->stCol_0.xW, VectA->xX, MatA->stCol_1.xW, VectA->xY, MatA->stCol_2.xW,
                        VectA->xZ, MatA->stCol_3.xW, VectA->xW);
}

static inline void inv_POS_fn_vCopyRotationAndScale(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hSource)
{
    _hDest->stMatrix.stCol_0 = _hSource->stMatrix.stCol_0;
    _hDest->stMatrix.stCol_1 = _hSource->stMatrix.stCol_1;
    _hDest->stMatrix.stCol_2 = _hSource->stMatrix.stCol_2;
    _hDest->stScale = _hSource->stScale;
}

static inline void inv_POS_fn_vMultiplyScales(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hA,
                                  POS_tdstMatrix_* _hB)
{
    inv_POS_fn_vMultiplyVectors(&_hDest->stScale, &_hA->stScale, &_hB->stScale);
}

static inline void inv_POS_fn_vSimplifyScaleType(POS_tdstMatrix_* _hTransform)
{
    switch (_hTransform->ucType) {
    case MAT_eTransform_Scale:
        if (!inv_POS_fn_bIsUniform(&_hTransform->stScale)) {
            break;
        }
        _hTransform->ucType = MAT_eTransform_Zoom;
    case MAT_eTransform_Zoom:
        if (__builtin_fabsf(_hTransform->stScale.xX - 1.0f) < 0.0001f) {
            _hTransform->ucType = MAT_eTransform_Translation;
        }
        break;
    case MAT_eTransform_RotationScale:
        if (!inv_POS_fn_bIsUniform(&_hTransform->stScale)) {
            break;
        }
        _hTransform->ucType = MAT_eTransform_RotationZoom;
    case MAT_eTransform_RotationZoom:
        if (__builtin_fabsf(_hTransform->stScale.xX - 1.0f) < 0.0001f) {
            _hTransform->ucType = MAT_eTransform_Rotation;
        }
        break;
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation:
    case MAT_eTransform_ComplexRotationScale:
        break;
    }
}

void _POS_fn_vMulMatrixMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hA, POS_tdstMatrix_* _hB)
{
    POS_tdstMatrix_ stTemp;
    MTH4D_tdstMatrix_ stProduct;
    POS_tdstMatrix_* hResult;
    u8 bComposeMatrixes;
    u8 bComposeTranslations;
    u8 bTestScale;

    if (_hA->ucType == MAT_eTransform_Identity) {
        *_hDest = *_hB;
        return;
    }
    if (_hB->ucType == MAT_eTransform_Identity) {
        *_hDest = *_hA;
        return;
    }
    if (_hDest == _hA || _hDest == _hB) {
        hResult = &stTemp;
        stTemp.ucType = MAT_eTransform_Identity;
    } else {
        hResult = _hDest;
    }

    bComposeMatrixes = 0;
    bComposeTranslations = 0;
    bTestScale = 0;

    switch (_hA->ucType) {
    case MAT_eTransform_Translation:
        hResult->ucType = _hB->ucType;
        inv_POS_fn_vAddVectors(&hResult->stMatrix.stCol_3, &_hA->stMatrix.stCol_3, &_hB->stMatrix.stCol_3);
        inv_POS_fn_vCopyRotationAndScale(hResult, _hB);
        break;
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
        bComposeTranslations = 1;
        switch (_hB->ucType) {
        case MAT_eTransform_Translation:
            hResult->ucType = _hA->ucType;
            inv_POS_fn_vCopyRotationAndScale(hResult, _hA);
            break;
        case MAT_eTransform_Rotation:
            hResult->ucType = _hA->ucType;
            hResult->stScale = _hA->stScale;
            bComposeMatrixes = 1;
            break;
        case MAT_eTransform_Zoom:
        case MAT_eTransform_RotationZoom:
            hResult->ucType = MAT_eTransform_RotationZoom;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_Scale:
        case MAT_eTransform_RotationScale:
            hResult->ucType = MAT_eTransform_RotationScale;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_ComplexRotationScale:
            hResult->ucType = MAT_eTransform_ComplexRotationScale;
            hResult->stScale = _hB->stScale;
            bComposeMatrixes = 1;
            break;
        }
        break;
    case MAT_eTransform_Zoom:
        bComposeTranslations = 1;
        switch (_hB->ucType) {
        case MAT_eTransform_Translation:
            hResult->ucType = _hA->ucType;
            inv_POS_fn_vCopyRotationAndScale(hResult, _hA);
            break;
        case MAT_eTransform_Rotation:
            hResult->ucType = MAT_eTransform_RotationZoom;
            hResult->stScale = _hA->stScale;
            bComposeMatrixes = 1;
            break;
        case MAT_eTransform_Zoom:
        case MAT_eTransform_Scale:
        case MAT_eTransform_RotationZoom:
        case MAT_eTransform_RotationScale:
            bTestScale = 1;
        case MAT_eTransform_ComplexRotationScale:
            hResult->ucType = _hB->ucType;
            bComposeMatrixes = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        }
        break;
    case MAT_eTransform_Scale:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        bComposeTranslations = 1;
        switch (_hB->ucType) {
        case MAT_eTransform_Translation:
            hResult->ucType = _hA->ucType;
            inv_POS_fn_vCopyRotationAndScale(hResult, _hA);
            break;
        case MAT_eTransform_Zoom:
        case MAT_eTransform_Scale:
            hResult->ucType = _hA->ucType;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_Rotation:
        case MAT_eTransform_RotationZoom:
        case MAT_eTransform_RotationScale:
            bTestScale = 1;
        case MAT_eTransform_ComplexRotationScale:
            hResult->ucType = MAT_eTransform_ComplexRotationScale;
            bComposeMatrixes = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        }
        break;
    }

    if (bComposeMatrixes) {
        if (hResult == _hA || hResult == _hB) {
            MTH4D_fn_vSpecializedMulMatrixMatrixWithoutBuffer(&stProduct, &_hA->stMatrix,
                                                              &_hB->stMatrix);
            hResult->stMatrix.stCol_0 = stProduct.stCol_0;
            hResult->stMatrix.stCol_1 = stProduct.stCol_1;
            hResult->stMatrix.stCol_2 = stProduct.stCol_2;
            hResult->stMatrix.stCol_3 = stProduct.stCol_3;
        } else {
            MTH4D_fn_vSpecializedMulMatrixMatrixWithoutBuffer(&hResult->stMatrix, &_hA->stMatrix,
                                                              &_hB->stMatrix);
        }
    } else if (bComposeTranslations) {
        MTH4D_tdstVector_* p_stDest = &hResult->stMatrix.stCol_3;
        MTH4D_tdstVector_* p_stSource = &_hB->stMatrix.stCol_3;

        if (p_stSource == p_stDest) {
            stProduct.stCol_0 = *p_stSource;
            MTH4D_fn_vMulMatrixVectorWithoutBuffer(p_stDest, &_hA->stMatrix, &stProduct.stCol_0);
        } else {
            MTH4D_fn_vMulMatrixVectorWithoutBuffer(p_stDest, &_hA->stMatrix, p_stSource);
        }
    }

    if (bTestScale) {
        inv_POS_fn_vSimplifyScaleType(hResult);
    }

    if (_hDest != hResult) {
        *_hDest = *hResult;
    }
}

void POS_fn_vMulMatrixMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hA, POS_tdstMatrix_* _hB)
{
    POS_tdstMatrix_ stTemp;
    MTH4D_tdstMatrix_ stProduct;
    POS_tdstMatrix_* hResult;
    u8 bComposeMatrixes;
    u8 bTestScale;

    if (_hA->ucType == MAT_eTransform_Identity || _hA->ucType == MAT_eTransform_Translation) {
        if (_hDest != _hB) {
            *_hDest = *_hB;
        }
        return;
    }
    if (_hB->ucType == MAT_eTransform_Identity) {
        if (_hDest != _hA) {
            *_hDest = *_hA;
        }
        return;
    }
    if (_hDest == _hA || _hDest == _hB) {
        hResult = &stTemp;
        stTemp.ucType = MAT_eTransform_Identity;
    } else {
        hResult = _hDest;
    }

    inv_POS_fn_vSetOnes(&hResult->stScale);
    bComposeMatrixes = 0;
    bTestScale = 0;

    switch (_hA->ucType) {
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
        switch (_hB->ucType) {
        case MAT_eTransform_Translation:
            hResult->ucType = _hA->ucType;
            inv_POS_fn_vCopyRotationAndScale(hResult, _hA);
            break;
        case MAT_eTransform_Rotation:
            hResult->ucType = MAT_eTransform_Rotation;
            bComposeMatrixes = 1;
            inv_POS_fn_vSetOnes(&hResult->stScale);
            break;
        case MAT_eTransform_Zoom:
            hResult->ucType = MAT_eTransform_RotationZoom;
            bTestScale = 1;
            inv_POS_fn_vScaleMatrix(hResult, _hA, &_hB->stScale);
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_RotationZoom:
            hResult->ucType = MAT_eTransform_RotationZoom;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_Scale:
        case MAT_eTransform_RotationScale:
            hResult->ucType = MAT_eTransform_RotationScale;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_ComplexRotationScale:
            hResult->ucType = MAT_eTransform_ComplexRotationScale;
            bComposeMatrixes = 1;
            hResult->stScale = _hB->stScale;
            break;
        }
        break;
    case MAT_eTransform_Zoom:
        switch (_hB->ucType) {
        case MAT_eTransform_Translation:
            hResult->ucType = _hA->ucType;
            inv_POS_fn_vCopyRotationAndScale(hResult, _hA);
            break;
        case MAT_eTransform_Rotation:
            hResult->ucType = MAT_eTransform_RotationZoom;
            inv_POS_fn_vScaleMatrix(hResult, _hB, &_hA->stScale);
            hResult->stScale = _hA->stScale;
            break;
        case MAT_eTransform_Zoom:
        case MAT_eTransform_Scale:
        case MAT_eTransform_RotationZoom:
        case MAT_eTransform_RotationScale:
        case MAT_eTransform_ComplexRotationScale:
            hResult->ucType = _hB->ucType;
            inv_POS_fn_vScaleMatrix(hResult, _hB, &_hA->stScale);
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        }
        break;
    case MAT_eTransform_Scale:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        switch (_hB->ucType) {
        case MAT_eTransform_Translation:
            hResult->ucType = _hA->ucType;
            inv_POS_fn_vCopyRotationAndScale(hResult, _hA);
            break;
        case MAT_eTransform_Zoom:
        case MAT_eTransform_Scale:
            hResult->ucType = _hA->ucType;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        case MAT_eTransform_Rotation:
        case MAT_eTransform_RotationZoom:
        case MAT_eTransform_RotationScale:
        case MAT_eTransform_ComplexRotationScale:
            hResult->ucType = MAT_eTransform_ComplexRotationScale;
            bComposeMatrixes = 1;
            bTestScale = 1;
            inv_POS_fn_vMultiplyScales(hResult, _hA, _hB);
            break;
        }
        break;
    }

    if (bComposeMatrixes) {
        if (hResult == _hA || hResult == _hB) {
            MTH4D_fn_vSpecializedMulMatrixMatrixWithoutBuffer(&stProduct, &_hA->stMatrix,
                                                              &_hB->stMatrix);
            hResult->stMatrix.stCol_0 = stProduct.stCol_0;
            hResult->stMatrix.stCol_1 = stProduct.stCol_1;
            hResult->stMatrix.stCol_2 = stProduct.stCol_2;
            hResult->stMatrix.stCol_3 = stProduct.stCol_3;
        } else {
            MTH4D_fn_vSpecializedMulMatrixMatrixWithoutBuffer(&hResult->stMatrix, &_hA->stMatrix,
                                                              &_hB->stMatrix);
        }
    }

    hResult->stMatrix.stCol_3 = _hB->stMatrix.stCol_3;

    if (bTestScale) {
        inv_POS_fn_vSimplifyScaleType(hResult);
    }

    if (_hDest != hResult) {
        *_hDest = *hResult;
    }
}

void inv_POS_fn_vTranslate(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stTranslation)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;
    s32 eType;

    eType = _hTransform->ucType;
    if (eType != MAT_eTransform_Identity) {
        if (eType != MAT_eTransform_Uninitialized) {
            if (eType < MAT_eTransform_Undefined) {
                MTH3D_M_vAddXYZ_inv(&p_stMatrix->stCol_3, &p_stMatrix->stCol_3, _p_stTranslation);
            }
        }
        return;
    }
    _hTransform->ucType = MAT_eTransform_Translation;
    MTH4D_fn_vCopy3DTo4DVector(&p_stMatrix->stCol_3, _p_stTranslation);
}

extern f32 flt_39348C[];

void POS_fn_vUpdateMatrixType(POS_tdstMatrix_* _hTransform)
{
    POS_tdstMatrix_* hMatrix;
    MTH4D_tdstVector_* p_stJ;
    MTH4D_tdstVector_* p_stK;
    MTH4D_tdstVector_* p_stScale;
    MTH4D_tdstVector_* p_stTranslation;
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    MTH4D_tdstVector_* p_stUnitScale;
    MTH4D_tdstVector_* p_stZoom;
    MTH4D_tdstVector_* p_stAxialScale;
    s32 bDiagonal;
    s32 bUnitDiagonal;
    s32 bNoTranslation;
    s32 bUniformDiagonal;
    s32 bUniformScale;
    f32 xEps;
    f32 xEpsOrtho;
    f32 xOne;
    f32 xDiag0;
    f32 xDiag1;
    f32 xDiag2;
    f32 xSqrI;
    f32 xSqrJ;
    f32 xSqrK;
    f32 xMax;
    f32 xDot;
    f32 xRoot0;
    f32 xRoot1;
    f32 xRoot2;
    f32 xM01;
    f32 xI0;
    f32 xI1;
    f32 xI2;
    f32 xJ0;
    f32 xJ1;
    f32 xJ2;
    f32 xK0;
    f32 xK1;
    f32 xK2;

    hMatrix = _hTransform;

    xEps = 0.0001f;
    xM01 = hMatrix->stMatrix.stCol_0.xY;
    bDiagonal = 0;
    if (inv_MTH_fn_xAbsolute(xM01) < xEps && __builtin_fabsf(hMatrix->stMatrix.stCol_0.xZ) < xEps &&
        __builtin_fabsf(hMatrix->stMatrix.stCol_1.xX) < xEps &&
        __builtin_fabsf(hMatrix->stMatrix.stCol_1.xZ) < xEps &&
        __builtin_fabsf(hMatrix->stMatrix.stCol_2.xX) < xEps &&
        __builtin_fabsf(hMatrix->stMatrix.stCol_2.xY) < xEps) {
        bDiagonal = 1;
    }

    bUnitDiagonal = (bDiagonal && __builtin_fabsf(hMatrix->stMatrix.stCol_0.xX - 1.0f) < 0.0001f &&
                     __builtin_fabsf(hMatrix->stMatrix.stCol_1.xY - 1.0f) < 0.0001f &&
                     __builtin_fabsf(hMatrix->stMatrix.stCol_2.xZ - 1.0f) < 0.0001f);

    p_stTranslation = &hMatrix->stMatrix.stCol_3;
    bNoTranslation =
        (MTH3D_M_bIsZeroXYZ_inv(p_stTranslation));

    bUniformDiagonal = bNoTranslation;
    if (bUnitDiagonal) {
        if (bUniformDiagonal) {
            gu_MTH4D_M_vSetIdentityMatrix(hMatrix);
            return;
        }

        hMatrix->ucType = MAT_eTransform_Translation;
        p_stCol1 = &hMatrix->stMatrix.stCol_1;
        hMatrix->stMatrix.stCol_0.xX = 1.0f;
        p_stCol2 = &hMatrix->stMatrix.stCol_2;
        hMatrix->stMatrix.stCol_0.xY = 0.0f;
        p_stUnitScale = &hMatrix->stScale;
        hMatrix->stMatrix.stCol_0.xZ = 0.0f;
        hMatrix->stMatrix.stCol_0.xW = 0.0f;
        hMatrix->stMatrix.stCol_1.xX = 0.0f;
        p_stCol1->xY = 1.0f;
        p_stCol1->xZ = 0.0f;
        p_stCol1->xW = 0.0f;
        hMatrix->stMatrix.stCol_2.xX = 0.0f;
        p_stCol2->xY = 0.0f;
        p_stCol2->xZ = 1.0f;
        p_stCol2->xW = 0.0f;
        MTH3D_M_vSetXYZ_inv(p_stUnitScale, 1.0f, 1.0f, 1.0f);
        p_stUnitScale->xW = 1.0f;
        return;
    }

    bUniformDiagonal = 0;
    if (bDiagonal) {
        xDiag0 = hMatrix->stMatrix.stCol_0.xX;
        xDiag1 = hMatrix->stMatrix.stCol_1.xY;
        if (__builtin_fabsf(xDiag0 - xDiag1) < 0.0001f) {
            if (__builtin_fabsf(xDiag0 - hMatrix->stMatrix.stCol_2.xZ) < 0.0001f) {
                bUniformDiagonal = 1;
            }
        }

        if (bUniformDiagonal) {
            hMatrix->ucType = MAT_eTransform_Zoom;
            p_stZoom = &hMatrix->stScale;
            MTH3D_M_vSetXYZ_inv(p_stZoom, xDiag0, xDiag0, xDiag0);
            p_stZoom->xW = 1.0f;
            return;
        }

        xDiag2 = hMatrix->stMatrix.stCol_2.xZ;
        p_stAxialScale = &hMatrix->stScale;
        hMatrix->ucType = MAT_eTransform_Scale;
        MTH3D_M_vSetXYZ_inv(p_stAxialScale, xDiag0, xDiag1, xDiag2);
        p_stAxialScale->xW = 1.0f;
        return;
    }

    p_stJ = &hMatrix->stMatrix.stCol_1;
    p_stK = &hMatrix->stMatrix.stCol_2;
    xSqrI = MTH_fn_xSqrAddSqrAddSqrOpt2(hMatrix->stMatrix.stCol_0.xX, xM01,
                                        hMatrix->stMatrix.stCol_0.xZ);
    p_stScale = &hMatrix->stScale;
    xSqrJ = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stJ->xX, p_stJ->xY, p_stJ->xZ);
    xSqrK = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stK->xX, p_stK->xY, p_stK->xZ);

    xI2 = 1.0f;
    MTH3D_M_vSetXYZ_inv(p_stScale, xSqrI, xSqrJ, xSqrK);
    p_stScale->xW = xI2;

    xMax = (p_stScale->xX > p_stScale->xY) ? p_stScale->xX : p_stScale->xY;
    xDot = (xMax > p_stScale->xZ) ? xMax : p_stScale->xZ;

    xEpsOrtho = 0.0001f;
    if (__builtin_fabsf(xDot - xI2) < xEpsOrtho) {
        hMatrix->ucType = MAT_eTransform_Rotation;
        MTH3D_M_vSetXYZ_inv(p_stScale, xI2, xI2, xI2);
        p_stScale->xW = xI2;
        return;
    }

    xI0 = hMatrix->stMatrix.stCol_0.xX;
    xJ0 = hMatrix->stMatrix.stCol_1.xX;
    xI1 = hMatrix->stMatrix.stCol_0.xY;
    xJ1 = hMatrix->stMatrix.stCol_1.xY;
    xI2 = hMatrix->stMatrix.stCol_0.xZ;
    xJ2 = hMatrix->stMatrix.stCol_1.xZ;
    xDot = inv_MTH_fn_xMulAdd3Grouped(xI0, xI1, xI2, xJ0, xJ1, xJ2);
    if (__builtin_fabsf(xDot) < xEpsOrtho) {
        xK0 = hMatrix->stMatrix.stCol_2.xX;
        xK1 = hMatrix->stMatrix.stCol_2.xY;
        xK2 = hMatrix->stMatrix.stCol_2.xZ;
        xDot = inv_MTH_fn_xMulAdd3Grouped(xI0, xI1, xI2, xK0, xK1, xK2);
        if (__builtin_fabsf(xDot) < xEpsOrtho) {
            xDot = inv_MTH_fn_xMulAdd3Grouped(xJ0, xJ1, xJ2, xK0, xK1, xK2);
            bUniformScale = 0;
            if (__builtin_fabsf(xDot) < xEpsOrtho) {
                if (__builtin_fabsf(p_stScale->xX - p_stScale->xY) < xEpsOrtho) {
                    if (__builtin_fabsf(p_stScale->xX - p_stScale->xZ) < xEpsOrtho) {
                        bUniformScale = 1;
                    }
                }

                if (bUniformScale) {
                    xOne = 1.0f;
                    hMatrix->ucType = MAT_eTransform_RotationZoom;
                    xDot = p_stScale->xX;
                    xDot = MTH_fn_xSqrtOpt2(xDot);
                    hMatrix->stScale.xX = xDot;
                    p_stScale->xY = xDot;
                    p_stScale->xZ = xDot;
                    p_stScale->xW = xOne;
                    return;
                }

                hMatrix->ucType = MAT_eTransform_RotationScale;
                p_stScale->xW = 1.0f;
                xRoot0 = MTH_fn_xSqrtOpt2(p_stScale->xX);
                xRoot1 = MTH_fn_xSqrtOpt2(p_stScale->xY);
                xRoot2 = MTH_fn_xSqrtOpt2(p_stScale->xZ);
                p_stScale->xY = xRoot1;
                p_stScale->xZ = xRoot2;
                p_stScale->xX = xRoot0;
                return;
            }
        }
    }

    xDot = flt_39348C[0];
    hMatrix->stScale.xX = xDot;
    hMatrix->ucType = MAT_eTransform_ComplexRotationScale;
    xOne = 1.0f;
    p_stScale->xY = xDot;
    p_stScale->xZ = xDot;
    p_stScale->xW = xOne;
}

void inv_POS_fn_vInitMatrixFromRotationAndTranslation(POS_tdstMatrix_* _hTransform,
                                                      MTH3D_tdstMatrix_* _p_stRotationMatrix,
                                                      MTH3D_tdstVector_* _p_stTranslation)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;

    MTH4D_fn_vCopy3DTo4DVector(&p_stMatrix->stCol_0, &_p_stRotationMatrix->stCol_0);
    MTH4D_fn_vCopy3DTo4DVector(&p_stMatrix->stCol_1, &_p_stRotationMatrix->stCol_1);
    *(MTH3D_tdstVector_*) &p_stMatrix->stCol_2 = _p_stRotationMatrix->stCol_2;
    p_stMatrix->stCol_3.xW = 1.0f;
    p_stMatrix->stCol_2.xW = 1.0f;
    p_stMatrix->stCol_3.xZ = 0.0f;
    p_stMatrix->stCol_3.xY = 0.0f;
    p_stMatrix->stCol_3.xX = 0.0f;
    MTH4D_fn_vCopy3DTo4DVector(&p_stMatrix->stCol_3, _p_stTranslation);
    POS_fn_vUpdateMatrixType(_hTransform);
}

void inv_POS_fn_vInitMatrixFromVectors(POS_tdstMatrix_* _hTransform, s32 _eType,
                                       MTH3D_tdstVector_* _p_stI, MTH3D_tdstVector_* _p_stJ,
                                       MTH3D_tdstVector_* _p_stK,
                                       MTH3D_tdstVector_* _p_stTranslation)
{
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    u32 eType = _eType & 0xFF;

    _hTransform->ucType = (u8) eType;

    if (_p_stI) {
        MTH4D_fn_vCopy3DTo4DVector(&_hTransform->stMatrix.stCol_0, _p_stI);
    } else {
        _hTransform->stMatrix.stCol_0.xY = 0.0f;
        _hTransform->stMatrix.stCol_0.xW = 1.0f;
        _hTransform->stMatrix.stCol_0.xX = 1.0f;
        _hTransform->stMatrix.stCol_0.xZ = 0.0f;
    }
    _hTransform->stMatrix.stCol_0.xW = 0.0f;

    if (_p_stJ) {
        p_stCol1 = &_hTransform->stMatrix.stCol_1;
        MTH4D_fn_vCopy3DTo4DVector(p_stCol1, _p_stJ);
    } else {
        p_stCol1 = &_hTransform->stMatrix.stCol_1;
        p_stCol1->xY = 0.0f;
        p_stCol1->xW = 1.0f;
        p_stCol1->xZ = 1.0f;
        _hTransform->stMatrix.stCol_1.xX = 0.0f;
    }
    _hTransform->stMatrix.stCol_1.xW = 0.0f;

    if (_p_stK) {
        p_stCol2 = &_hTransform->stMatrix.stCol_2;
        MTH4D_fn_vCopy3DTo4DVector(p_stCol2, _p_stK);
    } else {
        p_stCol2 = &_hTransform->stMatrix.stCol_2;
        p_stCol2->xZ = 0.0f;
        p_stCol2->xW = 1.0f;
        p_stCol2->xY = 1.0f;
        _hTransform->stMatrix.stCol_2.xX = 0.0f;
    }
    _hTransform->stMatrix.stCol_2.xW = 0.0f;

    if (_p_stTranslation) {
        MTH4D_fn_vCopy3DTo4DVector(&_hTransform->stMatrix.stCol_3, _p_stTranslation);
    } else {
        MTH4D_tdstVector_* p_stCol3 = &_hTransform->stMatrix.stCol_3;
        _hTransform->stMatrix.stCol_3.xX = 0.0f;
        p_stCol3->xW = 1.0f;
        p_stCol3->xZ = 0.0f;
        p_stCol3->xY = 0.0f;
    }

    switch (eType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation: {
        MTH4D_tdstVector_* p_stScale;
        p_stScale = &_hTransform->stScale;
        MTH3D_M_vSetXYZ_inv(p_stScale, 1.0f, 1.0f, 1.0f);
        p_stScale->xW = 1.0f;
        return;
    }
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale: {
        f32 xScaleX;
        f32 xScaleY;
        f32 xScaleZ;
        f32 xOne;
        MTH4D_tdstVector_* p_stScale;
        xOne = 1.0f;
        xScaleX = _hTransform->stMatrix.stCol_0.xX;
        p_stScale = &_hTransform->stScale;
        xScaleY = _hTransform->stMatrix.stCol_1.xY;
        xScaleZ = _hTransform->stMatrix.stCol_2.xZ;
        MTH3D_M_vSetXYZ_inv(p_stScale, xScaleX, xScaleY, xScaleZ);
        p_stScale->xW = xOne;
        return;
    }
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale: {
        f32 xScaleX, xScaleY, xScaleZ;
        MTH4D_tdstVector_* p_stScale;
        p_stScale = &_hTransform->stScale;
        xScaleX = MTH4D_fn_xSqrVector(&_hTransform->stMatrix.stCol_0);
        xScaleX = MTH_fn_xSqrtOpt2(xScaleX);
        xScaleY = MTH_fn_xSqrAddSqrAddSqrOpt2(_hTransform->stMatrix.stCol_1.xX, p_stCol1->xY,
                                              p_stCol1->xZ);
        xScaleY = MTH_fn_xSqrtOpt2(xScaleY);
        xScaleZ = MTH_fn_xSqrAddSqrAddSqrOpt2(_hTransform->stMatrix.stCol_2.xX, p_stCol2->xY,
                                              p_stCol2->xZ);
        p_stScale->xX = xScaleX;
        xScaleZ = MTH_fn_xSqrtOpt2(xScaleZ);
        p_stScale->xY = xScaleY;
        p_stScale->xZ = xScaleZ;
        p_stScale->xW = 1.0f;
        return;
    }
    case MAT_eTransform_ComplexRotationScale: {
        f32 xScale;
        f32 xOne;
        MTH4D_tdstVector_* p_stScale;
        xScale = flt_393490[0];
        p_stScale = &_hTransform->stScale;
        xOne = 1.0f;
        MTH3D_M_vSetXYZ_inv(p_stScale, xScale, xScale, xScale);
        p_stScale->xW = xOne;
        break;
    }
    default:
        break;
    }
}

s32 inv_POS_fn_bIsEqualMatrixExact(POS_tdstMatrix_* _hTransform1, POS_tdstMatrix_* _hTransform2)
{
    MTH4D_tdstMatrix_* p_stMatrix1 = &_hTransform1->stMatrix;
    MTH4D_tdstMatrix_* p_stMatrix2 = &_hTransform2->stMatrix;

    s32 bEquivalent;

    if (_hTransform1->ucType != _hTransform2->ucType) {
        return 0;
    }

    switch (_hTransform1->ucType) {
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        if (_hTransform1->stScale.xX != _hTransform2->stScale.xX ||
            _hTransform1->stScale.xY != _hTransform2->stScale.xY ||
            _hTransform1->stScale.xZ != _hTransform2->stScale.xZ) {
            return 0;
        }
    case MAT_eTransform_Translation:
        bEquivalent = 0;
        if (p_stMatrix1->stCol_3.xX == p_stMatrix2->stCol_3.xX &&
            p_stMatrix1->stCol_3.xY == p_stMatrix2->stCol_3.xY &&
            p_stMatrix1->stCol_3.xZ == p_stMatrix2->stCol_3.xZ) {
            bEquivalent = 1;
        }
        return bEquivalent;
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        bEquivalent = 0;
        if (!(p_stMatrix1->stCol_0.xX == p_stMatrix2->stCol_0.xX &&
              p_stMatrix1->stCol_0.xY == p_stMatrix2->stCol_0.xY &&
              p_stMatrix1->stCol_0.xZ == p_stMatrix2->stCol_0.xZ &&
              p_stMatrix1->stCol_0.xW == p_stMatrix2->stCol_0.xW &&
              p_stMatrix1->stCol_1.xX == p_stMatrix2->stCol_1.xX &&
              p_stMatrix1->stCol_1.xY == p_stMatrix2->stCol_1.xY &&
              p_stMatrix1->stCol_1.xZ == p_stMatrix2->stCol_1.xZ &&
              p_stMatrix1->stCol_1.xW == p_stMatrix2->stCol_1.xW &&
              p_stMatrix1->stCol_2.xX == p_stMatrix2->stCol_2.xX &&
              p_stMatrix1->stCol_2.xY == p_stMatrix2->stCol_2.xY &&
              p_stMatrix1->stCol_2.xZ == p_stMatrix2->stCol_2.xZ &&
              p_stMatrix1->stCol_2.xW == p_stMatrix2->stCol_2.xW &&
              p_stMatrix1->stCol_3.xX == p_stMatrix2->stCol_3.xX &&
              p_stMatrix1->stCol_3.xY == p_stMatrix2->stCol_3.xY &&
              p_stMatrix1->stCol_3.xZ == p_stMatrix2->stCol_3.xZ &&
              p_stMatrix1->stCol_3.xW == p_stMatrix2->stCol_3.xW)) {
            break;
        }
    case MAT_eTransform_Identity:
        return 1;
    default:
        bEquivalent = 0;
        break;
    }
    return bEquivalent;
}

s32 inv_POS_fn_bIsEqualMatrixEpsilon(POS_tdstMatrix_* _hTransform1, POS_tdstMatrix_* _hTransform2,
                                     f32 _xEps)
{
    MTH4D_tdstMatrix_* p_stMatrix1 = &_hTransform1->stMatrix;
    MTH4D_tdstMatrix_* p_stMatrix2 = &_hTransform2->stMatrix;

    s32 bEquivalent;
    MTH4D_tdstVector_* p_stScale1;
    MTH4D_tdstVector_* p_stScale2;
    MTH4D_tdstVector_* p_stTrans1;
    MTH4D_tdstVector_* p_stTrans2;

    if (_hTransform1->ucType != _hTransform2->ucType) {
        return 0;
    }
    switch (_hTransform1->ucType) {
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        p_stScale1 = &_hTransform1->stScale;
        p_stScale2 = &_hTransform2->stScale;
        bEquivalent = 0;
        if (inv_MTH_fn_xAbsoluteTied(p_stScale1->xX - p_stScale2->xX) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stScale1->xY - p_stScale2->xY) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stScale1->xZ - p_stScale2->xZ) < _xEps) {
            bEquivalent = 1;
        }
        if (!bEquivalent) {
            break;
        }
    case MAT_eTransform_Translation:
        p_stTrans1 = &p_stMatrix1->stCol_3;
        p_stTrans2 = &p_stMatrix2->stCol_3;
        bEquivalent = 0;
        if (inv_MTH_fn_xAbsoluteTied(p_stTrans1->xX - p_stTrans2->xX) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stTrans1->xY - p_stTrans2->xY) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stTrans1->xZ - p_stTrans2->xZ) < _xEps) {
            bEquivalent = 1;
        }
        return bEquivalent;
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        bEquivalent = 0;
        if (inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_0.xX - p_stMatrix2->stCol_0.xX) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_0.xY - p_stMatrix2->stCol_0.xY) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_0.xZ - p_stMatrix2->stCol_0.xZ) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_0.xW - p_stMatrix2->stCol_0.xW) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_1.xX - p_stMatrix2->stCol_1.xX) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_1.xY - p_stMatrix2->stCol_1.xY) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_1.xZ - p_stMatrix2->stCol_1.xZ) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_1.xW - p_stMatrix2->stCol_1.xW) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_2.xX - p_stMatrix2->stCol_2.xX) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_2.xY - p_stMatrix2->stCol_2.xY) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_2.xZ - p_stMatrix2->stCol_2.xZ) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_2.xW - p_stMatrix2->stCol_2.xW) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_3.xX - p_stMatrix2->stCol_3.xX) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_3.xY - p_stMatrix2->stCol_3.xY) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_3.xZ - p_stMatrix2->stCol_3.xZ) < _xEps &&
            inv_MTH_fn_xAbsoluteTied(p_stMatrix1->stCol_3.xW - p_stMatrix2->stCol_3.xW) < _xEps) {
            bEquivalent = 1;
        }
        break;
    case MAT_eTransform_Identity:
        bEquivalent = 1;
        break;
    default:
        bEquivalent = 0;
        break;
    }
    return bEquivalent;
}

static inline void inv_POS_fn_vSetW(MTH4D_tdstVector_* column, f32 w)
{
    column->xW = w;
}

static inline f32 inv_POS_fn_xDotXyz(MTH4D_tdstVector_* a, MTH4D_tdstVector_* b)
{
    f32 result;

    result = inv_MTH_fn_xMulAdd3Grouped(a->xX, a->xY, a->xZ, b->xX, b->xY, b->xZ);
    return result;
}

void POS_fn_vInitTransformationWithNonDiagonalScale(POS_tdstMatrix_* _hTransform,
                                                    MTH3D_tdstVector_* _p_stScale,
                                                    f32* _a_xQuaternion)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;
    MTH4D_tdstMatrix_ stRotation;
    MTH4D_tdstMatrix_ stTempMat;
    MTH4D_tdstVector_ st4DScaleVec;
    MTH4D_tdstVector_* p_stScale;
    f32 xZero;

    *(MTH3D_tdstVector_*) &st4DScaleVec = *_p_stScale;
    st4DScaleVec.xW = 1.0f;
    POS_fn_vQuatToMat(&stRotation, _a_xQuaternion[0], _a_xQuaternion[1], _a_xQuaternion[2],
                       _a_xQuaternion[3]);
    xZero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&stTempMat.stCol_0, stRotation.stCol_0.xX * st4DScaleVec.xX,
                        stRotation.stCol_0.xY * st4DScaleVec.xY,
                        stRotation.stCol_0.xZ * st4DScaleVec.xZ);
    inv_POS_fn_vSetW(&stTempMat.stCol_0, 1.0f);
    MTH3D_M_vSetXYZ_inv(&stTempMat.stCol_1, stRotation.stCol_1.xX * st4DScaleVec.xX,
                        stRotation.stCol_1.xY * st4DScaleVec.xY,
                        stRotation.stCol_1.xZ * st4DScaleVec.xZ);
    inv_POS_fn_vSetW(&stTempMat.stCol_1, 1.0f);
    MTH3D_M_vSetXYZ_inv(&stTempMat.stCol_2, stRotation.stCol_2.xX * st4DScaleVec.xX,
                        stRotation.stCol_2.xY * st4DScaleVec.xY,
                        stRotation.stCol_2.xZ * st4DScaleVec.xZ);
    inv_POS_fn_vSetW(&stTempMat.stCol_2, 1.0f);
    stTempMat.stCol_2.xW = xZero;
    stTempMat.stCol_1.xW = xZero;
    stTempMat.stCol_0.xW = xZero;
    MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_0,
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_0, &stRotation.stCol_0),
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_1, &stRotation.stCol_0),
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_2, &stRotation.stCol_0));
    MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_1,
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_0, &stRotation.stCol_1),
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_1, &stRotation.stCol_1),
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_2, &stRotation.stCol_1));
    MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_2,
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_0, &stRotation.stCol_2),
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_1, &stRotation.stCol_2),
                        inv_POS_fn_xDotXyz(&stTempMat.stCol_2, &stRotation.stCol_2));
    p_stMatrix->stCol_3.xZ = xZero;
    p_stMatrix->stCol_3.xY = xZero;
    p_stMatrix->stCol_3.xX = xZero;
    p_stMatrix->stCol_2.xW = xZero;
    p_stMatrix->stCol_1.xW = xZero;
    p_stMatrix->stCol_3.xW = 1.0f;
    p_stMatrix->stCol_0.xW = xZero;
    _hTransform->ucType = MAT_eTransform_ComplexRotationScale;
    p_stScale = &_hTransform->stScale;
    MTH3D_M_vSetXYZ_inv(p_stScale, 1.0f, 1.0f, 1.0f);
    p_stScale->xW = 1.0f;
}

void POS_fn_vInitTransformationWithQuaternion(POS_tdstMatrix_* _hTransform, f32* _a_xQuaternion)
{
    MTH4D_tdstVector_* p_stScale = &_hTransform->stScale;

    _hTransform->ucType = MAT_eTransform_Rotation;
    MTH3D_M_vSetXYZ_inv(p_stScale, 1.0f, 1.0f, 1.0f);
    p_stScale->xW = 1.0f;
    POS_fn_vQuatToMat(&_hTransform->stMatrix, _a_xQuaternion[0], _a_xQuaternion[1],
                       _a_xQuaternion[2], _a_xQuaternion[3]);
}

void POS_fn_vNormalizeMatrix(POS_tdstMatrix_* _hTransform)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_ComplexRotationScale:
        break;
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        _hTransform->ucType = MAT_eTransform_Translation;
        inv_POS_fn_vSetVector3(&p_stMatrix->stCol_0, 1.0f, 0.0f, 0.0f);
        inv_POS_fn_vSetVector3(&p_stMatrix->stCol_1, 0.0f, 1.0f, 0.0f);
        inv_POS_fn_vSetVector3(&p_stMatrix->stCol_2, 0.0f, 0.0f, 1.0f);
        inv_POS_fn_vSetVector4(&_hTransform->stScale, 1.0f, 1.0f, 1.0f, 1.0f);
        break;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        inv_POS_fn_vRemoveScaleFromMatrix(_hTransform, _hTransform);
    case MAT_eTransform_Rotation:
        inv_POS_fn_vCrossColumn(&p_stMatrix->stCol_1, &p_stMatrix->stCol_2, &p_stMatrix->stCol_0);
        inv_POS_fn_vNormalizeColumn(&p_stMatrix->stCol_0);
        inv_POS_fn_vNormalizeColumn(&p_stMatrix->stCol_1);
        inv_POS_fn_vCrossColumn(&p_stMatrix->stCol_2, &p_stMatrix->stCol_0, &p_stMatrix->stCol_1);
        p_stMatrix->stCol_0.xW = p_stMatrix->stCol_1.xW = p_stMatrix->stCol_2.xW = 0.0f;
        break;
    }
}

void POS_fn_vInvertIsoMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hSource)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hDest->stMatrix;
    MTH4D_tdstVector_ stVec;
    MTH4D_tdstVector_* p_stTranslation;
    MTH4D_tdstVector_* p_stSourceVec;
    MTH4D_tdstVector_* p_stUnitScale;
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    MTH4D_tdstVector_* p_stScale;
    MTH4D_tdstVector_* p_stDestVec;
    MTH4D_tdstVector_* p_stDestTranslation;
    MTH4D_tdstVector_* p_stSourceTranslation;
    f32 xSwap;
    f32 x10;
    f32 x20;
    f32 x21;

    switch (_hSource->ucType) {
    case MAT_eTransform_Identity:
        if (_hDest != _hSource) {
            gu_MTH4D_M_vSetIdentityMatrix(_hDest);
        }
        return;

    case MAT_eTransform_Translation:
        if (_hDest == _hSource) {
            p_stTranslation = &p_stMatrix->stCol_3;
            p_stMatrix->stCol_3.xX = inv_MTH_fn_xNegateTied(p_stMatrix->stCol_3.xX);
            p_stTranslation->xY = inv_MTH_fn_xNegateTied(p_stTranslation->xY);
            p_stTranslation->xZ = inv_MTH_fn_xNegateTied(p_stTranslation->xZ);
            p_stTranslation->xW = 1.0f;
            return;
        }

    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        p_stCol1 = &p_stMatrix->stCol_1;
        p_stCol2 = &p_stMatrix->stCol_2;
        p_stScale = &_hDest->stScale;
        p_stDestVec = &p_stMatrix->stCol_3;
        p_stSourceVec = &_hSource->stMatrix.stCol_3;
        _hDest->ucType = MAT_eTransform_Translation;
        MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_0, 1.0f, 0.0f, 0.0f);
        p_stMatrix->stCol_0.xW = 0.0f;
        p_stMatrix->stCol_1.xX = 0.0f;
        p_stCol1->xY = 1.0f;
        p_stCol1->xZ = 0.0f;
        p_stCol1->xW = 0.0f;
        p_stMatrix->stCol_2.xX = 0.0f;
        p_stCol2->xY = 0.0f;
        p_stCol2->xZ = 1.0f;
        p_stCol2->xW = 0.0f;
        _hDest->stScale.xX = 1.0f;
        p_stScale->xY = 1.0f;
        p_stScale->xZ = 1.0f;
        p_stScale->xW = 1.0f;
        MTH3D_M_vSetXYZ_inv(p_stDestVec, -_hSource->stMatrix.stCol_3.xX, -p_stSourceVec->xY,
                            inv_MTH_fn_xNegateTied(p_stSourceVec->xZ));
        p_stDestVec->xW = 1.0f;
        return;

    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        inv_POS_fn_vRemoveScaleFromMatrix(_hDest, _hSource);
        _hSource = _hDest;

    case MAT_eTransform_Rotation:
        _hDest->ucType = MAT_eTransform_Rotation;
        p_stDestTranslation = &p_stMatrix->stCol_3;
        if (_hDest == _hSource) {
            xSwap = p_stMatrix->stCol_0.xY;
            x10 = p_stMatrix->stCol_1.xX;
            p_stMatrix->stCol_1.xX = xSwap;
            xSwap = p_stMatrix->stCol_0.xZ;
            x20 = p_stMatrix->stCol_2.xX;
            p_stMatrix->stCol_2.xX = xSwap;
            xSwap = p_stMatrix->stCol_1.xZ;
            x21 = p_stMatrix->stCol_2.xY;
            p_stMatrix->stCol_0.xY = x10;
            p_stMatrix->stCol_0.xZ = x20;
            p_stMatrix->stCol_1.xZ = x21;
            p_stMatrix->stCol_2.xY = xSwap;
        } else {
            MTH4D_fn_vInvertRotationMatrix2(_hDest, _hSource);
        }
        p_stSourceTranslation = &_hSource->stMatrix.stCol_3;
        MTH3D_M_vSetXYZ_inv(
            p_stDestTranslation, inv_MTH_fn_xNegateTied(_hSource->stMatrix.stCol_3.xX),
            -p_stSourceTranslation->xY, inv_MTH_fn_xNegateTied(p_stSourceTranslation->xZ));
        p_stDestTranslation->xW = 1.0f;
        stVec = *p_stDestTranslation;
        MTH3D_M_vSetXYZ_inv(
            p_stDestTranslation,
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xX, stVec.xX, p_stMatrix->stCol_1.xX, stVec.xY,
                                p_stMatrix->stCol_2.xX, stVec.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xY, stVec.xX, p_stMatrix->stCol_1.xY, stVec.xY,
                                p_stMatrix->stCol_2.xY, stVec.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xZ, stVec.xX, p_stMatrix->stCol_1.xZ, stVec.xY,
                                p_stMatrix->stCol_2.xZ, stVec.xZ));
        p_stDestTranslation->xW = 1.0f;
        _hDest->stScale.xX = 1.0f;
        p_stUnitScale = &_hDest->stScale;
        p_stUnitScale->xY = 1.0f;
        p_stUnitScale->xZ = 1.0f;
        p_stUnitScale->xW = 1.0f;
        return;

    case MAT_eTransform_ComplexRotationScale:
        return;
    }
}

f32 cosf(f32 x);

static inline f32 inv_POS_fn_xSinFromCos(f32 c, f32 n, f32 pi)
{
    f32 sign;
    if (__builtin_fabsf(n) < pi) {
        sign = inv_MTH_fn_xSign(n);
    } else {
        sign = -inv_MTH_fn_xSign(n);
    }
    return MTH_fn_xSqrtOpt2(1.0f - c * c) * sign;
}

void POS_fn_vRotatePositionAroundAxis(POS_tdstMatrix_* _hTransform,
                                       MTH3D_tdstVector_* _p_stAxisVector, f32 _xAngle)
{
    MTH3D_tdstVector_ stQuatAxis;
    MTH4D_tdstMatrix_ stMatrix;
    f32 xHalfAngle;
    f32 xReducedAngle;
    f32 xCosAngle;
    f32 xSinAngle;
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;

    xHalfAngle = _xAngle * 0.5f;
    xReducedAngle =
        xHalfAngle - (f32) inv_MTH_fn_lToInteger(xHalfAngle * MTH_INV_TWO_PI_inv) * MTH_TWO_PI_inv;
    xCosAngle = cosf(xHalfAngle);
    xSinAngle = inv_POS_fn_xSinFromCos(xCosAngle, xReducedAngle, MTH_PI_inv);
    inv_MTH3D_fn_vKTimesXYZ(&stQuatAxis, xSinAngle, _p_stAxisVector);

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
        _hTransform->ucType = MAT_eTransform_Rotation;
        POS_fn_vQuatToMat(&_hTransform->stMatrix, stQuatAxis.xX, stQuatAxis.xY, stQuatAxis.xZ,
                           xCosAngle);
        break;
    case MAT_eTransform_Translation: {
        MTH4D_tdstVector_ stSaveTranslation;

        stSaveTranslation = _hTransform->stMatrix.stCol_3;
        _hTransform->ucType = MAT_eTransform_Rotation;
        POS_fn_vQuatToMat(&_hTransform->stMatrix, stQuatAxis.xX, stQuatAxis.xY, stQuatAxis.xZ,
                           xCosAngle);
        _hTransform->stMatrix.stCol_3 = stSaveTranslation;
        break;
    }
    case MAT_eTransform_Zoom: {
        f32 xZoomFactor;

        _hTransform->ucType = MAT_eTransform_RotationZoom;
        xZoomFactor = _hTransform->stScale.xX;
        POS_fn_vQuatToMat(&stMatrix, stQuatAxis.xX, stQuatAxis.xY, stQuatAxis.xZ, xCosAngle);
        p_stCol1 = &_hTransform->stMatrix.stCol_1;
        p_stCol2 = &_hTransform->stMatrix.stCol_2;
        MTH3D_M_vKTimesXYZ_inv(&_hTransform->stMatrix.stCol_0, xZoomFactor, &stMatrix.stCol_0);
        _hTransform->stMatrix.stCol_0.xW = 1.0f;
        MTH3D_M_vKTimesXYZ_inv(p_stCol1, xZoomFactor, &stMatrix.stCol_1);
        p_stCol1->xW = 1.0f;
        MTH3D_M_vKTimesXYZ_inv(p_stCol2, xZoomFactor, &stMatrix.stCol_2);
        p_stCol2->xW = 1.0f;
        break;
    }
    case MAT_eTransform_Scale: {
        MTH4D_tdstVector_* p_stScale;
        f32 scaleX;
        f32 scaleY;
        f32 scaleZ;

        _hTransform->ucType = MAT_eTransform_RotationScale;
        p_stScale = &_hTransform->stScale;
        POS_fn_vQuatToMat(&stMatrix, stQuatAxis.xX, stQuatAxis.xY, stQuatAxis.xZ, xCosAngle);
        p_stCol1 = &_hTransform->stMatrix.stCol_1;
        p_stCol2 = &_hTransform->stMatrix.stCol_2;
        scaleX = p_stScale->xX;
        MTH3D_M_vKTimesXYZ_inv(&_hTransform->stMatrix.stCol_0, scaleX, &stMatrix.stCol_0);
        _hTransform->stMatrix.stCol_0.xW = 1.0f;
        scaleY = p_stScale->xY;
        MTH3D_M_vKTimesXYZ_inv(p_stCol1, scaleY, &stMatrix.stCol_1);
        p_stCol1->xW = 1.0f;
        scaleZ = p_stScale->xZ;
        MTH3D_M_vKTimesXYZ_inv(p_stCol2, scaleZ, &stMatrix.stCol_2);
        p_stCol2->xW = 1.0f;
        break;
    }
    case MAT_eTransform_RotationScale:
        _hTransform->ucType = MAT_eTransform_ComplexRotationScale;
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_ComplexRotationScale: {
        MTH4D_tdstMatrix_ stResult;
        MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;

        POS_fn_vQuatToMat(&stMatrix, stQuatAxis.xX, stQuatAxis.xY, stQuatAxis.xZ, xCosAngle);
        MTH3D_M_vSetXYZ_inv(
            &stResult.stCol_0,
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xX, stMatrix.stCol_0.xX, p_stMatrix->stCol_1.xX,
                     stMatrix.stCol_0.xY, p_stMatrix->stCol_2.xX, stMatrix.stCol_0.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xY, stMatrix.stCol_0.xX, p_stMatrix->stCol_1.xY,
                     stMatrix.stCol_0.xY, p_stMatrix->stCol_2.xY, stMatrix.stCol_0.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xZ, stMatrix.stCol_0.xX, p_stMatrix->stCol_1.xZ,
                     stMatrix.stCol_0.xY, p_stMatrix->stCol_2.xZ, stMatrix.stCol_0.xZ));
        MTH3D_M_vSetXYZ_inv(
            &stResult.stCol_1,
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xX, stMatrix.stCol_1.xX, p_stMatrix->stCol_1.xX,
                     stMatrix.stCol_1.xY, p_stMatrix->stCol_2.xX, stMatrix.stCol_1.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xY, stMatrix.stCol_1.xX, p_stMatrix->stCol_1.xY,
                     stMatrix.stCol_1.xY, p_stMatrix->stCol_2.xY, stMatrix.stCol_1.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xZ, stMatrix.stCol_1.xX, p_stMatrix->stCol_1.xZ,
                     stMatrix.stCol_1.xY, p_stMatrix->stCol_2.xZ, stMatrix.stCol_1.xZ));
        MTH3D_M_vSetXYZ_inv(
            &stResult.stCol_2,
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xX, stMatrix.stCol_2.xX, p_stMatrix->stCol_1.xX,
                     stMatrix.stCol_2.xY, p_stMatrix->stCol_2.xX, stMatrix.stCol_2.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xY, stMatrix.stCol_2.xX, p_stMatrix->stCol_1.xY,
                     stMatrix.stCol_2.xY, p_stMatrix->stCol_2.xY, stMatrix.stCol_2.xZ),
            inv_MTH_fn_xMulAdd3(p_stMatrix->stCol_0.xZ, stMatrix.stCol_2.xX, p_stMatrix->stCol_1.xZ,
                     stMatrix.stCol_2.xY, p_stMatrix->stCol_2.xZ, stMatrix.stCol_2.xZ));
        stResult.stCol_3.xX =
            inv_POS_fn_xDot3Add(p_stMatrix->stCol_0.xX, stMatrix.stCol_3.xX, p_stMatrix->stCol_1.xX,
                    stMatrix.stCol_3.xY, p_stMatrix->stCol_2.xX, stMatrix.stCol_3.xZ,
                    p_stMatrix->stCol_3.xX);
        stResult.stCol_3.xY =
            inv_POS_fn_xDot3Add(p_stMatrix->stCol_0.xY, stMatrix.stCol_3.xX, p_stMatrix->stCol_1.xY,
                    stMatrix.stCol_3.xY, p_stMatrix->stCol_2.xY, stMatrix.stCol_3.xZ,
                    p_stMatrix->stCol_3.xY);
        stResult.stCol_3.xZ =
            inv_POS_fn_xDot3Add(p_stMatrix->stCol_0.xZ, stMatrix.stCol_3.xX, p_stMatrix->stCol_1.xZ,
                    stMatrix.stCol_3.xY, p_stMatrix->stCol_2.xZ, stMatrix.stCol_3.xZ,
                    p_stMatrix->stCol_3.xZ);
        stResult.stCol_3.xW = 1.0f;
        stResult.stCol_2.xW = 0.0f;
        stResult.stCol_1.xW = 0.0f;
        stResult.stCol_0.xW = 0.0f;
        p_stMatrix->stCol_0 = stResult.stCol_0;
        p_stMatrix->stCol_1 = stResult.stCol_1;
        p_stMatrix->stCol_2 = stResult.stCol_2;
        p_stMatrix->stCol_3 = stResult.stCol_3;
        break;
    }
    }
}

void POS_fn_vRotateX(POS_tdstMatrix_* _hTransform, f32 _xAngle)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;
    f32 xReduced;
    f32 xCos;
    f32 xSin;
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    MTH4D_tdstVector_* p_stCol3;
    MTH4D_tdstVector_* p_stScale;

    xReduced = _xAngle - (f32) inv_MTH_fn_lToInteger(_xAngle * MTH_INV_TWO_PI_inv) * MTH_TWO_PI_inv;
    xCos = cosf(_xAngle);
    xSin = inv_POS_fn_xSinFromCos(xCos, xReduced, MTH_PI_inv);

    p_stCol1 = &p_stMatrix->stCol_1;
    p_stCol2 = &p_stMatrix->stCol_2;
    p_stCol3 = &p_stMatrix->stCol_3;
    p_stScale = &_hTransform->stScale;
    p_stMatrix->stCol_0.xX = 1.0f;
    _hTransform->ucType = MAT_eTransform_Rotation;
    p_stMatrix->stCol_0.xY = 0.0f;
    p_stMatrix->stCol_0.xZ = 0.0f;
    p_stMatrix->stCol_0.xW = 0.0f;
    MTH3D_M_vSetXYZ_inv(p_stCol1, 0.0f, xCos, xSin);
    p_stCol1->xW = 0.0f;
    MTH3D_M_vSetXYZ_inv(p_stCol2, 0.0f, inv_MTH_fn_xNegate(xSin), xCos);
    p_stCol2->xW = 0.0f;
    MTH3D_M_vZeroXYZ_inv(p_stCol3);
    p_stCol3->xW = 1.0f;
    MTH3D_M_vSetXYZ_inv(p_stScale, 1.0f, 1.0f, 1.0f);
    p_stScale->xW = 1.0f;
}

void POS_fn_vRotateZ(POS_tdstMatrix_* _hTransform, f32 _xAngle)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;
    f32 xReduced;
    f32 xCos;
    f32 xSin;
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    MTH4D_tdstVector_* p_stCol3;
    MTH4D_tdstVector_* p_stScale;

    xReduced = _xAngle - (f32) inv_MTH_fn_lToInteger(_xAngle * MTH_INV_TWO_PI_inv) * MTH_TWO_PI_inv;
    xCos = cosf(_xAngle);
    xSin = inv_POS_fn_xSinFromCos(xCos, xReduced, MTH_PI_inv);

    p_stCol1 = &p_stMatrix->stCol_1;
    p_stCol2 = &p_stMatrix->stCol_2;
    p_stCol3 = &p_stMatrix->stCol_3;
    p_stScale = &_hTransform->stScale;
    p_stMatrix->stCol_0.xX = xCos;
    _hTransform->ucType = MAT_eTransform_Rotation;
    p_stMatrix->stCol_0.xZ = 0.0f;
    p_stMatrix->stCol_0.xW = 0.0f;
    p_stCol1->xX = inv_MTH_fn_xNegate(xSin);
    p_stMatrix->stCol_0.xY = xSin;
    p_stCol1->xY = xCos;
    p_stCol1->xZ = 0.0f;
    p_stCol1->xW = 0.0f;
    MTH3D_M_vSetXYZ_inv(p_stCol2, 0.0f, 0.0f, 1.0f);
    p_stCol2->xW = 0.0f;
    MTH3D_M_vZeroXYZ_inv(p_stCol3);
    p_stCol3->xW = 1.0f;
    MTH3D_M_vSetXYZ_inv(p_stScale, 1.0f, 1.0f, 1.0f);
    p_stScale->xW = 1.0f;
}

void POS_fn_vGetMatrixFromAngles(POS_tdstMatrix_* _hTransform, f32 _xAngleZ, f32 _xAngleY,
                                 f32 _xAngleX)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;

    f32 cz, sz, cy, sy, cx, sx;
    f32 y, a, b, d, e, h, i, j;
    f32 n, ms, zero;
    s32 lTurns;

    lTurns = inv_MTH_fn_lToInteger(_xAngleZ * MTH_INV_TWO_PI_inv);
    n = _xAngleZ - (f32) lTurns * MTH_TWO_PI_inv;
    cz = cosf(_xAngleZ);
    sz = inv_POS_fn_xSinFromCos(cz, n, MTH_PI_inv);

    lTurns = inv_MTH_fn_lToInteger(_xAngleY * MTH_INV_TWO_PI_inv);
    n = _xAngleY - (f32) lTurns * MTH_TWO_PI_inv;
    cy = cosf(_xAngleY);
    sy = inv_POS_fn_xSinFromCos(cy, n, MTH_PI_inv);

    lTurns = inv_MTH_fn_lToInteger(_xAngleX * MTH_INV_TWO_PI_inv);
    n = _xAngleX - (f32) lTurns * MTH_TWO_PI_inv;
    cx = cosf(_xAngleX);
    sx = inv_POS_fn_xSinFromCos(cx, n, MTH_PI_inv);

    p_stMatrix->stCol_0.xW = 0;
    p_stMatrix->stCol_2.xW = 1.0f;
    zero = p_stMatrix->stCol_0.xW;
    p_stMatrix->stCol_3.xZ = zero;
    p_stMatrix->stCol_3.xY = zero;
    p_stMatrix->stCol_3.xX = zero;

    e = -cz * sy;
    h = cz * sx;
    _hTransform->ucType = MAT_eTransform_Rotation;
    ms = -sz;
    a = sz * sy;
    b = ms * sy;
    i = sz * sx;
    d = cz * sy;
    p_stMatrix->stCol_1.xX = -cy * sx;
    j = sz * cx;
    *(s32*) &p_stMatrix->stCol_1.xW = 0;
    p_stMatrix->stCol_2.xX = sy;
    p_stMatrix->stCol_1.xY = b * sx + cz * cx;
    p_stMatrix->stCol_2.xZ = cz * cy;
    p_stMatrix->stCol_2.xY = ms * cy;
    p_stMatrix->stCol_0.xX = cy * cx;
    y = e * cx;
    p_stMatrix->stCol_0.xY = a * cx + h;
    p_stMatrix->stCol_0.xZ = y + i;
    p_stMatrix->stCol_1.xZ = d * sx + j;
}

void POS_fn_vGetRotationMatrix(POS_tdstMatrix_* _hTransform, MTH3D_tdstMatrix_* _p_stRotationMatrix)
{
    MTH4D_tdstVector_* p_stScaleVec;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        MTH3D_M_vSetXYZ_inv(&_p_stRotationMatrix->stCol_0, 1.0f, 0.0f, 0.0f);
        MTH3D_M_vSetXYZ_inv(&_p_stRotationMatrix->stCol_1, 0.0f, 1.0f, 0.0f);
        MTH3D_M_vSetXYZ_inv(&_p_stRotationMatrix->stCol_2, 0.0f, 0.0f, 1.0f);
        break;
    case MAT_eTransform_Rotation:
        _p_stRotationMatrix->stCol_0 = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_0;
        _p_stRotationMatrix->stCol_1 = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_1;
        _p_stRotationMatrix->stCol_2 = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_2;
        break;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        p_stScaleVec = &_hTransform->stScale;
        _p_stRotationMatrix->stCol_0.xX =
            _hTransform->stMatrix.stCol_0.xX / _hTransform->stScale.xX;
        _p_stRotationMatrix->stCol_0.xY =
            _hTransform->stMatrix.stCol_0.xY / _hTransform->stScale.xX;
        _p_stRotationMatrix->stCol_0.xZ =
            _hTransform->stMatrix.stCol_0.xZ / _hTransform->stScale.xX;
        MTH3D_M_vSetXYZ_inv(&_p_stRotationMatrix->stCol_1,
                            _hTransform->stMatrix.stCol_1.xX / p_stScaleVec->xY,
                            _hTransform->stMatrix.stCol_1.xY / p_stScaleVec->xY,
                            _hTransform->stMatrix.stCol_1.xZ / p_stScaleVec->xY);
        MTH3D_M_vSetXYZ_inv(&_p_stRotationMatrix->stCol_2,
                            _hTransform->stMatrix.stCol_2.xX / p_stScaleVec->xZ,
                            _hTransform->stMatrix.stCol_2.xY / p_stScaleVec->xZ,
                            _hTransform->stMatrix.stCol_2.xZ / p_stScaleVec->xZ);
        break;
    case MAT_eTransform_ComplexRotationScale:
        break;
    }
}

void POS_fn_vGetRotationVectors(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecI,
                                MTH3D_tdstVector_* _p_stVecJ, MTH3D_tdstVector_* _p_stVecK)
{
    MTH4D_tdstVector_* p_stScaleVec;
    f32 xZero, xOne, xInv;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        xZero = 0.0f;
        xOne = 1.0f;
        MTH3D_M_vSetXYZ_inv(_p_stVecI, xOne, xZero, xZero);
        MTH3D_M_vSetXYZ_inv(_p_stVecJ, xZero, xOne, xZero);
        MTH3D_M_vSetXYZ_inv(_p_stVecK, xZero, xZero, xOne);
        break;
    case MAT_eTransform_Rotation:
        *_p_stVecI = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_0;
        *_p_stVecJ = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_1;
        *_p_stVecK = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_2;
        break;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        p_stScaleVec = &_hTransform->stScale;
        xInv = 1.0f / p_stScaleVec->xX;
        MTH3D_M_vKTimesXYZ_inv(_p_stVecI, xInv, &_hTransform->stMatrix.stCol_0);
        xInv = 1.0f / p_stScaleVec->xY;
        MTH3D_M_vKTimesXYZ_inv(_p_stVecJ, xInv, &_hTransform->stMatrix.stCol_1);
        xInv = 1.0f / p_stScaleVec->xZ;
        MTH3D_M_vKTimesXYZ_inv(_p_stVecK, xInv, &_hTransform->stMatrix.stCol_2);
        break;
    case MAT_eTransform_ComplexRotationScale:
    default:
        break;
    }
}

void inv_POS_fn_vGetXAxisVector(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecI)
{
    f32 xZero;
    f32 xOne;
    f32 xInv;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        xZero = 0.0f;
        xOne = 1.0f;
        MTH3D_M_vSetXYZ_inv(_p_stVecI, xOne, xZero, xZero);
        break;
    case MAT_eTransform_Rotation:
        *_p_stVecI = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_0;
        break;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        xInv = 1.0f / _hTransform->stScale.xX;
        MTH3D_M_vKTimesXYZ_inv(_p_stVecI, xInv, &_hTransform->stMatrix.stCol_0);
        break;
    case MAT_eTransform_ComplexRotationScale:
        break;
    }
}

void POS_fn_vGetYAxisVector(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecJ)
{
    static void* const jpt[8] = {
        &&Lone, &&Lone, &&Lone, &&Lone, &&Lcopy, &&Lscale, &&Lscale, &&LscaleDone,
    };
    u32 uiIndex;

    uiIndex = _hTransform->ucType - 1;
    if (uiIndex < 8) {
        goto* jpt[uiIndex];
    }
    return;

Lone:
    MTH3D_M_vSetXYZ_inv(_p_stVecJ, 0.0f, 1.0f, 0.0f);
    return;

Lcopy:
    *_p_stVecJ = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_1;
    return;

Lscale:
    MTH3D_M_vSetXYZ_inv(_p_stVecJ,
                        (1.0f / _hTransform->stScale.xY) * _hTransform->stMatrix.stCol_1.xX,
                        (1.0f / _hTransform->stScale.xY) * _hTransform->stMatrix.stCol_1.xY,
                        (1.0f / _hTransform->stScale.xY) * _hTransform->stMatrix.stCol_1.xZ);
LscaleDone:
    return;
}

void POS_fn_vGetRotationVector(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecK)
{
    static void* const jpt[8] = {
        &&Lone, &&Lone, &&Lone, &&Lone, &&Lcopy, &&Lscale, &&Lscale, &&LscaleDone,
    };
    u32 uiIndex;

    uiIndex = _hTransform->ucType - 1;
    if (uiIndex < 8) {
        goto* jpt[uiIndex];
    }
    return;

Lone:
    _p_stVecK->xY = _p_stVecK->xX = 0.0f;
    _p_stVecK->xZ = 1.0f;
    return;

Lcopy:
    *_p_stVecK = *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_2;
    return;

Lscale:
    MTH3D_M_vSetXYZ_inv(_p_stVecK,
                        (1.0f / _hTransform->stScale.xZ) * _hTransform->stMatrix.stCol_2.xX,
                        (1.0f / _hTransform->stScale.xZ) * _hTransform->stMatrix.stCol_2.xY,
                        (1.0f / _hTransform->stScale.xZ) * _hTransform->stMatrix.stCol_2.xZ);
LscaleDone:
    return;
}

void POS_fn_vSetRotationMatrix(POS_tdstMatrix_* _hTransform, MTH3D_tdstMatrix_* _p_stRotationMatrix)
{
    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
        _hTransform->ucType = MAT_eTransform_Rotation;
    case MAT_eTransform_Rotation:
        *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_0 = _p_stRotationMatrix->stCol_0;
        _hTransform->stMatrix.stCol_0.xW = 1.0f;
        *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_1 = _p_stRotationMatrix->stCol_1;
        _hTransform->stMatrix.stCol_1.xW = 1.0f;
        *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_2 = _p_stRotationMatrix->stCol_2;
        _hTransform->stMatrix.stCol_0.xW = 0.0f;
        _hTransform->stMatrix.stCol_1.xW = 0.0f;
        _hTransform->stMatrix.stCol_2.xW = 0.0f;
        break;
    case MAT_eTransform_Zoom:
        _hTransform->ucType = MAT_eTransform_RotationZoom;
    case MAT_eTransform_RotationZoom:
        _hTransform->stMatrix.stCol_0.xX =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_0.xX;
        _hTransform->stMatrix.stCol_0.xY =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_0.xY;
        _hTransform->stMatrix.stCol_0.xZ =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_0.xZ;
        _hTransform->stMatrix.stCol_1.xX =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_1.xX;
        _hTransform->stMatrix.stCol_1.xY =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_1.xY;
        _hTransform->stMatrix.stCol_1.xZ =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_1.xZ;
        _hTransform->stMatrix.stCol_2.xX =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_2.xX;
        _hTransform->stMatrix.stCol_2.xY =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_2.xY;
        _hTransform->stMatrix.stCol_2.xZ =
            _hTransform->stScale.xX * _p_stRotationMatrix->stCol_2.xZ;
        break;
    case MAT_eTransform_Scale:
        _hTransform->ucType = MAT_eTransform_RotationScale;
    case MAT_eTransform_RotationScale: {
        MTH4D_tdstVector_* p_stScaleVec = &_hTransform->stScale;
        MTH3D_M_vSetXYZ_inv(&_hTransform->stMatrix.stCol_0,
                            p_stScaleVec->xX * _p_stRotationMatrix->stCol_0.xX,
                            p_stScaleVec->xX * _p_stRotationMatrix->stCol_0.xY,
                            p_stScaleVec->xX * _p_stRotationMatrix->stCol_0.xZ);
        MTH3D_M_vSetXYZ_inv(&_hTransform->stMatrix.stCol_1,
                            p_stScaleVec->xY * _p_stRotationMatrix->stCol_1.xX,
                            p_stScaleVec->xY * _p_stRotationMatrix->stCol_1.xY,
                            p_stScaleVec->xY * _p_stRotationMatrix->stCol_1.xZ);
        MTH3D_M_vSetXYZ_inv(&_hTransform->stMatrix.stCol_2,
                            p_stScaleVec->xZ * _p_stRotationMatrix->stCol_2.xX,
                            p_stScaleVec->xZ * _p_stRotationMatrix->stCol_2.xY,
                            p_stScaleVec->xZ * _p_stRotationMatrix->stCol_2.xZ);
        break;
    }
    case MAT_eTransform_ComplexRotationScale:
        break;
    }
}

void _POS_fn_vSetRotationMatrix(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecI,
                                MTH3D_tdstVector_* _p_stVecJ, MTH3D_tdstVector_* _p_stVecK)
{
#define FP1_SQRT(v) __asm__("sqrt.s %0, %0" : "+f"(v))

    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    MTH4D_tdstVector_* p_stScaleVec;
    f32 xOne;
    f32 xZoom;
    f32 xNormI;
    f32 xNormJ;
    f32 xNormK;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
        _hTransform->ucType = MAT_eTransform_Rotation;
    case MAT_eTransform_Rotation:
        *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_0 = *_p_stVecI;
        xOne = 1.0f;
        _hTransform->stMatrix.stCol_0.xW = xOne;
        *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_1 = *_p_stVecJ;
        _hTransform->stMatrix.stCol_1.xW = xOne;
        *(MTH3D_tdstVector_*) &_hTransform->stMatrix.stCol_2 = *_p_stVecK;
        _hTransform->stMatrix.stCol_0.xW = 0.0f;
        _hTransform->stMatrix.stCol_1.xW = 0.0f;
        _hTransform->stMatrix.stCol_2.xW = 0.0f;
        return;
    case MAT_eTransform_Zoom:
        _hTransform->ucType = MAT_eTransform_RotationZoom;
    case MAT_eTransform_RotationZoom:
        xZoom = _hTransform->stScale.xX;
        MTH3D_M_vKTimesXYZ_inv(&_hTransform->stMatrix.stCol_0, xZoom, _p_stVecI);
        MTH3D_M_vKTimesXYZ_inv(&_hTransform->stMatrix.stCol_1, xZoom, _p_stVecJ);
        _hTransform->stMatrix.stCol_2.xX = xZoom * _p_stVecK->xX;
        _hTransform->stMatrix.stCol_2.xY = xZoom * _p_stVecK->xY;
        xZoom = xZoom * _p_stVecK->xZ;
        _hTransform->stMatrix.stCol_2.xZ = xZoom;
        return;
    case MAT_eTransform_Scale:
        _hTransform->ucType = MAT_eTransform_RotationScale;
    case MAT_eTransform_RotationScale:
        xZoom = _hTransform->stScale.xX;
        p_stScaleVec = &_hTransform->stScale;
        _hTransform->stMatrix.stCol_0.xX = xZoom * _p_stVecI->xX;
        _hTransform->stMatrix.stCol_0.xY = xZoom * _p_stVecI->xY;
        xZoom = xZoom * _p_stVecI->xZ;
        _hTransform->stMatrix.stCol_0.xZ = xZoom;
        MTH3D_M_vSetXYZ_inv(&_hTransform->stMatrix.stCol_1, p_stScaleVec->xY * _p_stVecJ->xX,
                            p_stScaleVec->xY * _p_stVecJ->xY, p_stScaleVec->xY * _p_stVecJ->xZ);
        MTH3D_M_vSetXYZ_inv(&_hTransform->stMatrix.stCol_2, p_stScaleVec->xZ * _p_stVecK->xX,
                            p_stScaleVec->xZ * _p_stVecK->xY, p_stScaleVec->xZ * _p_stVecK->xZ);
        return;
    case MAT_eTransform_ComplexRotationScale:
        _hTransform->ucType = MAT_eTransform_RotationScale;
        p_stCol1 = &_hTransform->stMatrix.stCol_1;
        p_stCol2 = &_hTransform->stMatrix.stCol_2;
        xNormI = _hTransform->stMatrix.stCol_0.xX;
        xNormI = MTH_fn_xSqrAddSqrAddSqrOpt2(xNormI, _hTransform->stMatrix.stCol_0.xY, _hTransform->stMatrix.stCol_0.xZ);
        FP1_SQRT(xNormI);
        xNormJ = _hTransform->stMatrix.stCol_1.xX;
        _hTransform->stScale.xX = xNormI;
        xNormK = _hTransform->stMatrix.stCol_2.xX;
        _hTransform->stMatrix.stCol_0.xX = xNormI * _p_stVecI->xX;
        _hTransform->stMatrix.stCol_0.xY = xNormI * _p_stVecI->xY;
        xNormI = xNormI * _p_stVecI->xZ;
        _hTransform->stMatrix.stCol_0.xZ = xNormI;
        xNormJ = MTH_fn_xSqrAddSqrAddSqrOpt2(xNormJ, p_stCol1->xY, p_stCol1->xZ);
        FP1_SQRT(xNormJ);
        _hTransform->stScale.xY = xNormJ;
        _hTransform->stMatrix.stCol_1.xX = xNormJ * _p_stVecJ->xX;
        _hTransform->stMatrix.stCol_1.xY = xNormJ * _p_stVecJ->xY;
        xNormJ = xNormJ * _p_stVecJ->xZ;
        _hTransform->stMatrix.stCol_1.xZ = xNormJ;
        xNormK = MTH_fn_xSqrAddSqrAddSqrOpt2(xNormK, p_stCol2->xY, p_stCol2->xZ);
        FP1_SQRT(xNormK);
        _hTransform->stScale.xZ = xNormK;
        _hTransform->stMatrix.stCol_2.xX = xNormK * _p_stVecK->xX;
        _hTransform->stMatrix.stCol_2.xY = xNormK * _p_stVecK->xY;
        xNormK = xNormK * _p_stVecK->xZ;
        _hTransform->stMatrix.stCol_2.xZ = xNormK;
        break;
    }
}

void func_00126B30(POS_tdstMatrix_* _hTransform)
{
    MTH4D_tdstMatrix_* p_stMatrix = &_hTransform->stMatrix;
    MTH4D_tdstVector_* p_stCol1;
    MTH4D_tdstVector_* p_stCol2;
    MTH4D_tdstVector_* p_stCol1Zoom;
    MTH4D_tdstVector_* p_stCol2Zoom;
    MTH4D_tdstVector_* p_stCol1Scale;
    MTH4D_tdstVector_* p_stCol2Scale;
    f32 xScaleX;
    f32 xZoom;
    f32 xScaleY;
    f32 xScaleZ;

    switch (_hTransform->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
    case MAT_eTransform_ComplexRotationScale:
        break;
    case MAT_eTransform_Rotation:
        p_stCol1 = &p_stMatrix->stCol_1;
        p_stCol2 = &p_stMatrix->stCol_2;
        _hTransform->ucType = MAT_eTransform_Translation;
        MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_0, 1.0f, 0.0f, 0.0f);
        p_stMatrix->stCol_0.xW = 0.0f;
        p_stMatrix->stCol_1.xX = 0.0f;
        p_stCol1->xY = 1.0f;
        p_stCol1->xZ = 0.0f;
        p_stCol1->xW = 0.0f;
        p_stMatrix->stCol_2.xX = 0.0f;
        p_stCol2->xY = 0.0f;
        p_stCol2->xZ = 1.0f;
        p_stCol2->xW = 0.0f;
        break;
    case MAT_eTransform_RotationZoom:
        p_stCol1Zoom = &p_stMatrix->stCol_1;
        p_stCol2Zoom = &p_stMatrix->stCol_2;
        _hTransform->ucType = MAT_eTransform_Zoom;
        MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_0, 1.0f, 0.0f, 0.0f);
        p_stMatrix->stCol_0.xW = 0.0f;
        p_stMatrix->stCol_1.xX = 0.0f;
        p_stCol1Zoom->xY = 1.0f;
        p_stCol1Zoom->xZ = 0.0f;
        p_stCol1Zoom->xW = 0.0f;
        p_stMatrix->stCol_2.xX = 0.0f;
        p_stCol2Zoom->xY = 0.0f;
        p_stCol2Zoom->xZ = 1.0f;
        p_stCol2Zoom->xW = 0.0f;
        xZoom = _hTransform->stScale.xX;
        p_stMatrix->stCol_2.xZ = xZoom;
        p_stMatrix->stCol_1.xY = xZoom;
        p_stMatrix->stCol_0.xX = xZoom;
        break;
    case MAT_eTransform_RotationScale:
        p_stCol1Scale = &p_stMatrix->stCol_1;
        p_stCol2Scale = &p_stMatrix->stCol_2;
        _hTransform->ucType = MAT_eTransform_Scale;
        MTH3D_M_vSetXYZ_inv(&p_stMatrix->stCol_0, 1.0f, 0.0f, 0.0f);
        p_stMatrix->stCol_0.xW = 0.0f;
        p_stMatrix->stCol_1.xX = 0.0f;
        p_stCol1Scale->xY = 1.0f;
        p_stCol1Scale->xZ = 0.0f;
        p_stCol1Scale->xW = 0.0f;
        p_stMatrix->stCol_2.xX = 0.0f;
        p_stCol2Scale->xY = 0.0f;
        p_stCol2Scale->xZ = 1.0f;
        p_stCol2Scale->xW = 0.0f;
        xScaleX = _hTransform->stScale.xX;
        xScaleY = _hTransform->stScale.xY;
        xScaleZ = _hTransform->stScale.xZ;
        p_stMatrix->stCol_0.xX = xScaleX;
        p_stMatrix->stCol_1.xY = xScaleY;
        p_stMatrix->stCol_2.xZ = xScaleZ;
        break;
    }
}
