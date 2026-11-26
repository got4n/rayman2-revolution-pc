#ifndef REVO_MTH_MTH_H
#define REVO_MTH_MTH_H

#include "ps2/types.h"

static inline float inv_MTH_fn_xNegateTied(float in)
{
    __asm__("neg.s %0, %0" : "+f"(in));
    return in;
}

static inline float inv_MTH_fn_xDivideBySqrt(float num, float den)
{
    float ret;
    __asm__("rsqrt.s %0, %1, %2" : "=f"(ret) : "f"(num), "f"(den));
    return ret;
}

static inline float inv_MTH_fn_xAbsoluteTied(float in)
{
    __asm__("abs.s %0, %0" : "+f"(in));
    return in;
}

static inline float inv_MTH_fn_xMultiply(float a, float b)
{
    float ret;
    __asm__("mul.s %0, %1, %2" : "=f"(ret) : "f"(a), "f"(b));
    return ret;
}

static inline f32 inv_MTH_fn_xAbsolute(f32 value)
{
    f32 result;
    __asm__("abs.s %0, %1" : "=f"(result) : "f"(value));
    return result;
}

static inline f32 inv_MTH_fn_xNegate(f32 value)
{
    f32 result;
    __asm__("neg.s %0, %1" : "=f"(result) : "f"(value));
    return result;
}

static inline f32 inv_MTH_fn_xZero(void)
{
    f32 result;
    __asm__("mtc1 $0, %0" : "=f"(result) : : "memory");
    return result;
}

static inline f32 inv_MTH_fn_xDivideBySqrtTied(f32 num, f32 den)
{
    f32 result;
    __asm__("rsqrt.s %0, %1, %0" : "=f"(result) : "f"(num), "0"(den));
    return result;
}

static inline f32 inv_MTH_fn_xMinimum(f32 a, f32 b)
{
    f32 result;
    __asm__("min.s %0, %1, %2" : "=f"(result) : "f"(a), "f"(b));
    return result;
}

static inline f32 inv_MTH_fn_xMaximum(f32 a, f32 b)
{
    f32 result;
    __asm__("max.s %0, %1, %2" : "=f"(result) : "f"(a), "f"(b));
    return result;
}

static inline f32 inv_MTH_fn_xMulAdd3(f32 ax, f32 bx, f32 ay, f32 by, f32 az, f32 bz)
{
    f32 result;
    __asm__("mula.s %1, %2\n\t"
            "madda.s %3, %4\n\t"
            "madd.s %0, %5, %6"
            : "=f"(result)
            : "f"(ax), "f"(bx), "f"(ay), "f"(by), "f"(az), "f"(bz));
    return result;
}

static inline f32 inv_MTH_fn_xMulAdd4Grouped(f32 ax, f32 ay, f32 az, f32 aw, f32 bx, f32 by, f32 bz, f32 bw)
{
    f32 result;
    __asm__("mula.s %1, %5\n\t"
            "madda.s %2, %6\n\t"
            "madda.s %3, %7\n\t"
            "madd.s %0, %4, %8"
            : "=f"(result)
            : "f"(ax), "f"(ay), "f"(az), "f"(aw), "f"(bx), "f"(by), "f"(bz), "f"(bw));
    return result;
}

static inline f32 inv_MTH_fn_xMulAdd3Tied(f32 ax, f32 bx, f32 ay, f32 by, f32 az, f32 bz)
{
    f32 result;
    __asm__("mula.s %1, %2\n\t"
            "madda.s %3, %4\n\t"
            "madd.s %0, %5, %6"
            : "=f"(result)
            : "0"(ax), "f"(bx), "f"(ay), "f"(by), "f"(az), "f"(bz));
    return result;
}

static inline f32 inv_MTH_fn_xMulAdd3TiedY(f32 ax, f32 bx, f32 ay, f32 by, f32 az, f32 bz)
{
    f32 result;
    __asm__("mula.s %1, %2\n\t"
            "madda.s %3, %4\n\t"
            "madd.s %0, %5, %6"
            : "=f"(result)
            : "f"(ax), "f"(bx), "0"(ay), "f"(by), "f"(az), "f"(bz));
    return result;
}

static inline f32 inv_MTH_fn_xMulAdd3Grouped(f32 ax, f32 ay, f32 az, f32 bx, f32 by, f32 bz)
{
    f32 result;
    __asm__("mula.s %1, %4\n\t"
            "madda.s %2, %5\n\t"
            "madd.s %0, %3, %6"
            : "=f"(result)
            : "f"(ax), "f"(ay), "f"(az), "f"(bx), "f"(by), "f"(bz));
    return result;
}

static inline s32 inv_MTH_fn_lToInteger(f32 value)
{
    s32 result;
    __asm__("cvt.w.s $f30, %1\n\t"
            "mfc1 %0, $f30"
            : "=r"(result)
            : "f"(value)
            : "$f30");
    return result;
}

static inline s32 inv_MTH_fn_lToIntegerOrdered(f32 value)
{
    s32 result;
    __asm__("cvt.w.s $f30, %1\n\t"
            "mfc1 %0, $f30"
            : "=r"(result)
            : "f"(value)
            : "$f30", "memory");
    return result;
}

static inline f32 inv_MTH_fn_xMulSub(f32 a, f32 b, f32 c, f32 d)
{
    f32 result;
    __asm__("mula.s %1, %2\n\t"
            "msub.s %0, %3, %4"
            : "=f"(result)
            : "f"(a), "f"(b), "f"(c), "f"(d));
    return result;
}

static inline f32 inv_MTH_fn_xMulSubTied(f32 a, f32 b, f32 c, f32 d)
{
    f32 result;
    __asm__("mula.s %1, %2\n\t"
            "msub.s %0, %3, %4"
            : "=f"(result)
            : "0"(a), "f"(b), "f"(c), "f"(d));
    return result;
}

#define MTH_PI_inv 3.1415927f
#define MTH_TWO_PI_inv 6.2831855f
#define MTH_HALF_PI_inv 1.5707964f
#define MTH_QUARTER_PI_inv 0.7853982f
#define MTH_INV_TWO_PI_inv 0.15915494f
#define MTH_DEG_TO_RAD_inv 0.017453292f
#define MTH_RAD_TO_DEG_inv 57.29578f

typedef struct MTH3D_tdstVector_ {
    f32 xX;
    f32 xY;
    f32 xZ;
} MTH3D_tdstVector_;

#define MTH3D_M_vSetXYZ_inv(p, x, y, z) ((p)->xX = (x), (p)->xY = (y), (p)->xZ = (z))
#define MTH3D_M_vZeroXYZ_inv(p) MTH3D_M_vSetXYZ_inv(p, 0.0f, 0.0f, 0.0f)
#define MTH3D_M_vCopyXYZ_inv(d, a) ((d)->xX = (a)->xX, (d)->xY = (a)->xY, (d)->xZ = (a)->xZ)
#define MTH3D_M_vAddXYZ_inv(d, a, b)                                                               \
    ((d)->xX = (a)->xX + (b)->xX, (d)->xY = (a)->xY + (b)->xY, (d)->xZ = (a)->xZ + (b)->xZ)
#define MTH3D_M_vSubXYZ_inv(d, a, b)                                                               \
    ((d)->xX = (a)->xX - (b)->xX, (d)->xY = (a)->xY - (b)->xY, (d)->xZ = (a)->xZ - (b)->xZ)
#define MTH3D_M_vScaleXYZ_inv(d, a, k)                                                             \
    ((d)->xX = (a)->xX * (k), (d)->xY = (a)->xY * (k), (d)->xZ = (a)->xZ * (k))
#define MTH3D_M_vKTimesXYZ_inv(d, k, a)                                                            \
    ((d)->xX = (k) * (a)->xX, (d)->xY = (k) * (a)->xY, (d)->xZ = (k) * (a)->xZ)
#define MTH3D_M_vSubLowerXYZ_inv(d, a, b)                                                          \
    ((d)->x = (a)->x - (b)->x, (d)->y = (a)->y - (b)->y, (d)->z = (a)->z - (b)->z)
#define MTH3D_M_vCopyLowerXYZ_inv(d, a) ((d)->x = (a)->x, (d)->y = (a)->y, (d)->z = (a)->z)
#define MTH3D_M_bIsZeroXYZ_inv(p) ((p)->xX == 0.0f && (p)->xY == 0.0f && (p)->xZ == 0.0f)

typedef struct MTH3D_tdstMatrix_ {
    MTH3D_tdstVector_ stCol_0;
    MTH3D_tdstVector_ stCol_1;
    MTH3D_tdstVector_ stCol_2;
} MTH3D_tdstMatrix_;

typedef struct MTH4D_tdstVector_ {
    f32 xX;
    f32 xY;
    f32 xZ;
    f32 xW;
} MTH4D_tdstVector_;

typedef struct MTH4D_tdstMatrix_ {
    MTH4D_tdstVector_ stCol_0;
    MTH4D_tdstVector_ stCol_1;
    MTH4D_tdstVector_ stCol_2;
    MTH4D_tdstVector_ stCol_3;
} MTH4D_tdstMatrix_;

typedef enum MAT_tdeTranformationType_ {
    MAT_eTransform_Uninitialized,
    MAT_eTransform_Identity,
    MAT_eTransform_Translation,
    MAT_eTransform_Zoom,
    MAT_eTransform_Scale,
    MAT_eTransform_Rotation,
    MAT_eTransform_RotationZoom,
    MAT_eTransform_RotationScale,
    MAT_eTransform_ComplexRotationScale,
    MAT_eTransform_Undefined
} MAT_tdeTranformationType_;

typedef struct POS_tdstMatrix_ {
    MTH4D_tdstMatrix_ stMatrix;
    MTH4D_tdstVector_ stScale;
    u8 ucType;
} POS_tdstMatrix_;

static inline MTH3D_tdstVector_* inv_POS_fn_p_stPosMatrixTranslation(POS_tdstMatrix_* matrix)
{
    return (MTH3D_tdstVector_*) &matrix->stMatrix.stCol_3;
}

static inline void inv_POS_fn_vSetTranslation(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* translation)
{
    if (matrix->ucType == MAT_eTransform_Identity) {
        matrix->ucType = MAT_eTransform_Translation;
    }
    *inv_POS_fn_p_stPosMatrixTranslation(matrix) = *translation;
    matrix->stMatrix.stCol_3.xW = 1.0f;
}

static inline u8 inv_POS_fn_bIsScaleMatrix(POS_tdstMatrix_* matrix)
{
    return matrix->ucType == MAT_eTransform_Zoom || matrix->ucType == MAT_eTransform_Scale ||
           matrix->ucType == MAT_eTransform_RotationScale ||
           matrix->ucType == MAT_eTransform_RotationZoom ||
           matrix->ucType == MAT_eTransform_ComplexRotationScale;
}

#define MTH_fn_xSqrAddSqrAddSqrOpt2(a, b, c)                                                       \
    ({                                                                                             \
        f32 _mthSqrResult;                                                                         \
        __asm__("mula.s %1, %1\n\t"                                                                \
                "madda.s %2, %2\n\t"                                                               \
                "madd.s %0, %3, %3"                                                                \
                : "=f"(_mthSqrResult)                                                              \
                : "f"(a), "f"(b), "f"(c));                                                         \
        _mthSqrResult;                                                                             \
    })

#define MTH_fn_xSqrtOpt2(xA)                                                                       \
    ({                                                                                             \
        f32 _mthSqrtResult;                                                                        \
        __asm__("sqrt.s %0, %1" : "=f"(_mthSqrtResult) : "f"(xA));                                 \
        _mthSqrtResult;                                                                            \
    })

#define MTH4D_fn_xSqrVector(VectA)                                                                 \
    MTH_fn_xSqrAddSqrAddSqrOpt2(((const f32*) (VectA))[0], ((const f32*) (VectA))[1],              \
                                ((const f32*) (VectA))[2])

#define MTH4D_fn_vCopy3DTo4DVector(Dst, Src)                                                       \
    (*(__typeof__(*(Src))*) (Dst) = *(Src), ((f32*) (Dst))[3] = 1.0f)

#define MTH4D_fn_vCopy4DTo3DVector(Dst, Src) (*(Dst) = *(__typeof__(*(Dst))*) (Src))

#define MTH4D_fn_vSetIdentityMatrix(Matrix)                                                        \
    (((f32*) (Matrix))[15] = 1.0f, ((f32*) (Matrix))[10] = 1.0f, ((f32*) (Matrix))[5] = 1.0f,      \
     ((f32*) (Matrix))[0] = 1.0f, ((s32*) (Matrix))[13] = 0, ((s32*) (Matrix))[12] = 0,            \
     ((s32*) (Matrix))[11] = 0, ((s32*) (Matrix))[9] = 0, ((s32*) (Matrix))[8] = 0,                \
     ((s32*) (Matrix))[7] = 0, ((s32*) (Matrix))[6] = 0, ((s32*) (Matrix))[4] = 0,                 \
     ((s32*) (Matrix))[3] = 0, ((s32*) (Matrix))[2] = 0, ((s32*) (Matrix))[1] = 0,                 \
     ((s32*) (Matrix))[14] = 0)

#define MTH4D_fn_vInvertRotationMatrix2(Dst, Src)                                                  \
    do {                                                                                           \
        f32* _mthDst = (f32*) (Dst);                                                               \
        const f32* _mthSrc = (const f32*) (Src);                                                   \
        _mthDst[0] = _mthSrc[0];                                                                   \
        _mthDst[1] = _mthSrc[4];                                                                   \
        _mthDst[2] = _mthSrc[8];                                                                   \
        _mthDst[3] = _mthSrc[3];                                                                   \
        _mthDst[4] = _mthSrc[1];                                                                   \
        _mthDst[5] = _mthSrc[5];                                                                   \
        _mthDst[6] = _mthSrc[9];                                                                   \
        _mthDst[7] = _mthSrc[7];                                                                   \
        _mthDst[8] = _mthSrc[2];                                                                   \
        _mthDst[9] = _mthSrc[6];                                                                   \
        _mthDst[10] = _mthSrc[10];                                                                 \
        _mthDst[11] = _mthSrc[11];                                                                 \
        _mthDst[12] = _mthSrc[12];                                                                 \
        _mthDst[13] = _mthSrc[13];                                                                 \
        _mthDst[14] = _mthSrc[14];                                                                 \
        _mthDst[15] = _mthSrc[15];                                                                 \
    } while (0)

static inline void inv_MTH3D_fn_vNormalizeXYZ(MTH3D_tdstVector_* vector)
{
    f32 xNorm = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(vector->xX, vector->xY, vector->xZ));

    if (xNorm != 0.0f) {
        xNorm = 1.0f / xNorm;
        vector->xX = xNorm * vector->xX;
        vector->xY = xNorm * vector->xY;
        vector->xZ = xNorm * vector->xZ;
    }
}

static inline f32 inv_MTH3D_fn_xDotXYZ(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b)
{
    f32 result;

    __asm__("mula.s %1, %2\n\t"
            "madda.s %3, %4\n\t"
            "madd.s %0, %5, %6"
            : "=f"(result)
            : "f"(a->xX), "f"(b->xX), "f"(a->xY), "f"(b->xY), "f"(a->xZ), "f"(b->xZ));
    return result;
}

static inline f32 inv_MTH3D_fn_xDotXYZGrouped(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b)
{
    f32 result;

    __asm__("mula.s %1, %4\n\t"
            "madda.s %2, %5\n\t"
            "madd.s %0, %3, %6"
            : "=f"(result)
            : "f"(a->xX), "f"(a->xY), "f"(a->xZ), "f"(b->xX), "f"(b->xY), "f"(b->xZ));
    return result;
}

static inline void inv_MTH3D_fn_vSubXYZ(MTH3D_tdstVector_* dst, MTH3D_tdstVector_* a,
                                           MTH3D_tdstVector_* b)
{
    dst->xX = a->xX - b->xX;
    dst->xY = a->xY - b->xY;
    dst->xZ = a->xZ - b->xZ;
}

static inline void inv_MTH3D_fn_vAddXYZ(MTH3D_tdstVector_* dst, MTH3D_tdstVector_* a,
                                           MTH3D_tdstVector_* b)
{
    dst->xX = a->xX + b->xX;
    dst->xY = a->xY + b->xY;
    dst->xZ = a->xZ + b->xZ;
}

static inline void inv_MTH3D_fn_vKTimesXYZ(MTH3D_tdstVector_* dst, f32 scale,
                                                 MTH3D_tdstVector_* a)
{
    dst->xX = scale * a->xX;
    dst->xY = scale * a->xY;
    dst->xZ = scale * a->xZ;
}

static inline f32 inv_MTH_fn_xSign(f32 value)
{
    union {
        f32 f;
        u32 u;
    } bits;

    bits.f = value;
    if ((bits.u & 0x7FFFFFFF) == 0) {
        return 0.0f;
    }
    bits.u = (bits.u & 0x80000000) | 0x3F800000;
    return bits.f;
}

static inline f32 inv_MTH3D_fn_xNormXYZ(MTH3D_tdstVector_* vector)
{
    f32 xSquareNorm;

    xSquareNorm = MTH_fn_xSqrAddSqrAddSqrOpt2(vector->xX, vector->xY, vector->xZ);
    return MTH_fn_xSqrtOpt2(xSquareNorm);
}

#define DNM_p_stVectorCrossProduct(Dst, A, B)                                                      \
    ({                                                                                             \
        ((f32*) (Dst))[0] = inv_MTH_fn_xMulSub(((const f32*) (A))[1], ((const f32*) (B))[2],               \
                                       ((const f32*) (A))[2], ((const f32*) (B))[1]);              \
        ((f32*) (Dst))[1] = inv_MTH_fn_xMulSub(((const f32*) (A))[2], ((const f32*) (B))[0],               \
                                       ((const f32*) (A))[0], ((const f32*) (B))[2]);              \
        ((f32*) (Dst))[2] = inv_MTH_fn_xMulSub(((const f32*) (A))[0], ((const f32*) (B))[1],               \
                                       ((const f32*) (A))[1], ((const f32*) (B))[0]);              \
        (Dst);                                                                                     \
    })

static __inline__ f32 MTH_fn_xSqrOpt1(f32 xA)
{
    return xA * xA;
}

static __inline__ void MTH4D_fn_vNegVector(MTH4D_tdstVector_* dst, MTH4D_tdstVector_* src)
{
    dst->xX = inv_MTH_fn_xNegateTied(src->xX);
    dst->xY = -src->xY;
    dst->xZ = inv_MTH_fn_xNegateTied(src->xZ);
    dst->xW = 1.0f;
}

extern s32 MEC_fn_bIsEqualMatrix(POS_tdstMatrix_* left, POS_tdstMatrix_* right);

extern s32 inv_POS_fn_bIsEqualMatrixEpsilon(POS_tdstMatrix_* _hTransform1,
                                            POS_tdstMatrix_* _hTransform2, f32 _xEps);
extern void POS_fn_vInitPos(void);
extern void POS_fn_vNormalizeMatrix(POS_tdstMatrix_* _hTransform);
extern void POS_fn_vRotatePositionAroundAxis(POS_tdstMatrix_* _hTransform,
                                             MTH3D_tdstVector_* _p_stAxisVector, f32 _xAngle);
extern void POS_fn_vRotateX(POS_tdstMatrix_* _hTransform, f32 _xAngle);
extern void inv_POS_fn_vInitMatrixFromRotationAndTranslation(
    POS_tdstMatrix_* _hTransform, MTH3D_tdstMatrix_* _p_stRotationMatrix,
    MTH3D_tdstVector_* _p_stTranslation);
extern void inv_POS_fn_vTranslate(POS_tdstMatrix_* _hTransform,
                                         MTH3D_tdstVector_* _p_stTranslation);
extern s32 inv_POS_fn_bIsEqualMatrixExact(POS_tdstMatrix_* _hTransform1,
                                          POS_tdstMatrix_* _hTransform2);
extern void POS_fn_vGetMatrixFromAngles(POS_tdstMatrix_* _hTransform, f32 _xAngleZ, f32 _xAngleY,
                                        f32 _xAngleX);
extern void inv_POS_fn_vGetXAxisVector(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecI);
extern void POS_fn_vGetRotationVectors(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecI,
                                       MTH3D_tdstVector_* _p_stVecJ, MTH3D_tdstVector_* _p_stVecK);
extern void POS_fn_vMulMatrixVertex(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform,
                                    MTH3D_tdstVector_* _p_stSource);
extern void POS_fn_vInvertIsoMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hSource);
extern void POS_fn_vInvertMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hSource);
extern void POS_fn_vMulMatrixVertexArray(MTH3D_tdstVector_* _p_stDest,
                                         POS_tdstMatrix_* _hTransform,
                                         MTH3D_tdstVector_* _p_stSource, u32 _ulNumber);
extern void POS_fn_vInitTransformationWithNonDiagonalScale(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stScale, f32* _a_xQuaternion);
extern void POS_fn_vInitTransformationWithQuaternion(POS_tdstMatrix_* _hTransform, f32* _a_xQuaternion);
extern void POS_fn_vApplyMatrixToVector(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stSource);
extern void POS_fn_vGetRotationMatrix(POS_tdstMatrix_* _hTransform, MTH3D_tdstMatrix_* _p_stRotationMatrix);
extern void POS_fn_vMulMatrixMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hA, POS_tdstMatrix_* _hB);
extern void POS_fn_vRotateVector(MTH3D_tdstVector_* _p_stDest, POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stSource);
extern void POS_fn_vRotateZ(POS_tdstMatrix_* _hTransform, f32 _xAngle);
extern void POS_fn_vSetRotationMatrix(POS_tdstMatrix_* _hTransform, MTH3D_tdstMatrix_* _p_stRotationMatrix);
extern void _POS_fn_vMulMatrixMatrix(POS_tdstMatrix_* _hDest, POS_tdstMatrix_* _hA, POS_tdstMatrix_* _hB);
extern void _POS_fn_vSetRotationMatrix(POS_tdstMatrix_* _hTransform, MTH3D_tdstVector_* _p_stVecI, MTH3D_tdstVector_* _p_stVecJ, MTH3D_tdstVector_* _p_stVecK);
extern void gu_MTH4D_M_vSetIdentityMatrix(POS_tdstMatrix_* _hTransform);
extern void inv_POS_fn_vInitMatrixFromVectors(POS_tdstMatrix_* _hTransform, s32 _eType, MTH3D_tdstVector_* _p_stI, MTH3D_tdstVector_* _p_stJ, MTH3D_tdstVector_* _p_stK, MTH3D_tdstVector_* _p_stTranslation);

#endif
