#include "common.h"

#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "GAM/GAM.h"

extern volatile f32 D_003928B8;
extern f32 D_003928BC;

void inv_POS_fn_vZoom(POS_tdstMatrix_* matrix, f32 zoom)
{
    if (__builtin_fabsf(zoom - 1.0f) < D_003928B8) {
        return;
    }

    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation: {
        MTH4D_tdstVector_* scale = &matrix->stScale;

        matrix->ucType = MAT_eTransform_Zoom;
        matrix->stMatrix.stCol_2.xZ = zoom;
        matrix->stMatrix.stCol_1.xY = zoom;
        matrix->stMatrix.stCol_0.xX = zoom;
        MTH3D_M_vSetXYZ_inv(scale, zoom, zoom, zoom);
        scale->xW = 1.0f;
        return;
    }
    case MAT_eTransform_Rotation:
        matrix->ucType = MAT_eTransform_RotationZoom;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        MTH3D_M_vSetXYZ_inv(&matrix->stScale, zoom * matrix->stScale.xX, zoom * matrix->stScale.xY,
                            zoom * matrix->stScale.xZ);
    case MAT_eTransform_ComplexRotationScale: {
        MTH4D_tdstVector_* col1 = &matrix->stMatrix.stCol_1;
        MTH4D_tdstVector_* col2 = &matrix->stMatrix.stCol_2;
        f32 zero = 0.0f;

        MTH3D_M_vKTimesXYZ_inv(&matrix->stMatrix.stCol_0, zoom, &matrix->stMatrix.stCol_0);
        matrix->stMatrix.stCol_0.xW = 1.0f;
        col1->xX = zoom * col1->xX;
        col1->xW = 1.0f;
        col1->xY = zoom * col1->xY;
        col1->xZ = zoom * col1->xZ;
        col2->xX = zoom * col2->xX;
        col2->xW = 1.0f;
        col2->xY = zoom * col2->xY;
        col2->xZ = zoom * col2->xZ;
        matrix->stMatrix.stCol_2.xW = zero;
        matrix->stMatrix.stCol_1.xW = zero;
        matrix->stMatrix.stCol_0.xW = zero;
        return;
    }
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        matrix->stMatrix.stCol_0.xX = matrix->stMatrix.stCol_0.xX * zoom;
        matrix->stMatrix.stCol_1.xY = matrix->stMatrix.stCol_1.xY * zoom;
        matrix->stMatrix.stCol_2.xZ = matrix->stMatrix.stCol_2.xZ * zoom;
        MTH3D_M_vKTimesXYZ_inv(&matrix->stScale, zoom, &matrix->stScale);
        return;
    }
}

void inv_POS_fn_vScale(POS_tdstMatrix_* matrix, const MTH3D_tdstVector_* scale)
{
    MTH4D_tdstVector_* col0 = &matrix->stMatrix.stCol_0;
    MTH4D_tdstVector_* col1;
    MTH4D_tdstVector_* col2;
    f32 scaleX = scale->xX;
    f32 epsilon = D_003928BC;
    s32 uniform = 0;
    f32 x, y, z, one, zero, scaleY, scaleZ, col1X;

    if (__builtin_fabsf(scaleX - scale->xY) < epsilon) {
        if (__builtin_fabsf(scaleX - scale->xZ) < epsilon) {
            uniform = 1;
        }
    }
    if ((u8) uniform) {
        inv_POS_fn_vZoom(matrix, scale->xX);
        return;
    }

    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
        matrix->ucType = MAT_eTransform_Scale;
        matrix->stMatrix.stCol_0.xX = scale->xX;
        matrix->stMatrix.stCol_1.xY = scale->xY;
        matrix->stMatrix.stCol_2.xZ = scale->xZ;
        *(MTH3D_tdstVector_*) &matrix->stScale = (*scale);
        return;
    case MAT_eTransform_Zoom:
        matrix->ucType = MAT_eTransform_Scale;
        scaleX = scale->xX;
    case MAT_eTransform_Scale:
        matrix->stMatrix.stCol_0.xX = matrix->stMatrix.stCol_0.xX * scaleX;
        matrix->stMatrix.stCol_1.xY = matrix->stMatrix.stCol_1.xY * scale->xY;
        matrix->stMatrix.stCol_2.xZ = matrix->stMatrix.stCol_2.xZ * scale->xZ;
        MTH3D_M_vSetXYZ_inv(&matrix->stScale, matrix->stScale.xX * scale->xX,
                            matrix->stScale.xY * scale->xY, matrix->stScale.xZ * scale->xZ);
        return;
    case MAT_eTransform_Rotation:
    case MAT_eTransform_RotationZoom:
        matrix->ucType = MAT_eTransform_RotationScale;
        scaleX = scale->xX;
    case MAT_eTransform_RotationScale:
        MTH3D_M_vSetXYZ_inv(&matrix->stScale, matrix->stScale.xX * scaleX,
                            matrix->stScale.xY * scale->xY, matrix->stScale.xZ * scale->xZ);
        scaleX = scale->xX;
    case MAT_eTransform_ComplexRotationScale:
        col1 = &matrix->stMatrix.stCol_1;
        col2 = &matrix->stMatrix.stCol_2;
        x = scaleX * col0->xX;
        y = scaleX * col0->xY;
        z = scaleX * col0->xZ;
        one = 1.0f;
        col1X = col1->xX;
        col0->xX = x;
        col0->xW = one;
        col0->xY = y;
        col0->xZ = z;
        zero = 0.0f;
        scaleY = scale->xY;
        col1->xX = scaleY * col1X;
        col1->xW = one;
        col1->xY = scaleY * col1->xY;
        col1->xZ = scaleY * col1->xZ;
        scaleZ = scale->xZ;
        col2->xX = scaleZ * col2->xX;
        col2->xW = one;
        col2->xY = scaleZ * col2->xY;
        col2->xZ = scaleZ * col2->xZ;
        matrix->stMatrix.stCol_2.xW = zero;
        matrix->stMatrix.stCol_1.xW = zero;
        matrix->stMatrix.stCol_0.xW = zero;
        return;
    }
}

#define R5900_LEN4_ORDERED(out, a, b, c, d)                                                        \
    __asm__ __volatile__("mula.s %1, %1\n\t"                                                       \
                         "madda.s %2, %2\n\t"                                                      \
                         "madda.s %3, %3\n\t"                                                      \
                         "madd.s %0, %4, %4"                                                       \
                         : "=f"(out)                                                               \
                         : "f"(a), "f"(b), "f"(c), "f"(d)                                          \
                         : "$f0")

extern volatile f32 D_003928C0;
extern f32 D_003928C4;

void POS_fn_vSetZoomMatrix(POS_tdstMatrix_* matrix, f32 scale)
{
    if (__builtin_fabsf(scale - 1.0f) < D_003928C0) {
        inv_POS_fn_vRemoveScaleFromMatrix(matrix, matrix);
        return;
    }

    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Scale:
        matrix->ucType = MAT_eTransform_Zoom;
    case MAT_eTransform_Zoom: {
        MTH4D_tdstVector_* zoom = &matrix->stScale;
        matrix->stMatrix.stCol_2.xZ = scale;
        matrix->stMatrix.stCol_1.xY = scale;
        matrix->stMatrix.stCol_0.xX = scale;
        MTH3D_M_vSetXYZ_inv(zoom, scale, scale, scale);
        zoom->xW = 1.0f;
        return;
    }
    case MAT_eTransform_Rotation: {
        MTH4D_tdstVector_* column1 = &matrix->stMatrix.stCol_1;
        MTH4D_tdstVector_* column2 = &matrix->stMatrix.stCol_2;
        MTH4D_tdstVector_* zoom = &matrix->stScale;
        f32 zero = 0.0f;
        matrix->ucType = MAT_eTransform_RotationZoom;
        MTH3D_M_vKTimesXYZ_inv(&matrix->stMatrix.stCol_0, scale, &matrix->stMatrix.stCol_0);
        matrix->stMatrix.stCol_0.xW = 1.0f;
        column1->xX = scale * column1->xX;
        column1->xW = 1.0f;
        column1->xY = scale * column1->xY;
        column1->xZ = scale * column1->xZ;
        column2->xX = scale * column2->xX;
        column2->xW = 1.0f;
        column2->xY = scale * column2->xY;
        column2->xZ = scale * column2->xZ;
        zoom->xX = scale;
        matrix->stMatrix.stCol_2.xW = zero;
        matrix->stMatrix.stCol_1.xW = zero;
        matrix->stMatrix.stCol_0.xW = zero;
        zoom->xY = scale;
        zoom->xZ = scale;
        zoom->xW = 1.0f;
        return;
    }
    case MAT_eTransform_RotationScale:
        matrix->ucType = MAT_eTransform_RotationZoom;
    case MAT_eTransform_RotationZoom: {
        MTH4D_tdstVector_* column1 = &matrix->stMatrix.stCol_1;
        MTH4D_tdstVector_* column2 = &matrix->stMatrix.stCol_2;
        MTH4D_tdstVector_* zoom = &matrix->stScale;
        f32 scaleRatio;
        f32 one = 1.0f;
        f32 zero = 0.0f;

        scaleRatio = scale / zoom->xX;
        MTH3D_M_vKTimesXYZ_inv(&matrix->stMatrix.stCol_0, scaleRatio, &matrix->stMatrix.stCol_0);
        matrix->stMatrix.stCol_0.xW = one;
        scaleRatio = scale / zoom->xY;
        column1->xX = scaleRatio * column1->xX;
        column1->xW = one;
        column1->xY = scaleRatio * column1->xY;
        column1->xZ = scaleRatio * column1->xZ;
        scaleRatio = scale / zoom->xZ;
        column2->xX = scaleRatio * column2->xX;
        column2->xW = one;
        column2->xY = scaleRatio * column2->xY;
        column2->xZ = scaleRatio * column2->xZ;
        zoom->xX = scale;
        matrix->stMatrix.stCol_2.xW = zero;
        matrix->stMatrix.stCol_1.xW = zero;
        matrix->stMatrix.stCol_0.xW = zero;
        zoom->xY = scale;
        zoom->xZ = scale;
        zoom->xW = one;
        return;
    }
    case MAT_eTransform_ComplexRotationScale:
        return;
    }
}

static inline s32 inv_POS_fn_bIsUniformScale(MTH3D_tdstVector_* scale)
{
    s32 uniform = 0;

    if (__builtin_fabsf(scale->xX - scale->xY) < D_003928C4 &&
        __builtin_fabsf(scale->xX - scale->xZ) < D_003928C4) {
        uniform = 1;
    }
    return uniform;
}

void POS_fn_vSetScaleMatrix(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* scale)
{
    u8 uniform = inv_POS_fn_bIsUniformScale(scale);

    if (uniform) {
        POS_fn_vSetZoomMatrix(matrix, scale->xX);
        return;
    }

    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Zoom:
        matrix->ucType = MAT_eTransform_Scale;
    case MAT_eTransform_Scale:
        matrix->stMatrix.stCol_0.xX = scale->xX;
        matrix->stMatrix.stCol_1.xY = scale->xY;
        matrix->stMatrix.stCol_2.xZ = scale->xZ;
        *(MTH3D_tdstVector_*) &matrix->stScale = *scale;
        return;
    case MAT_eTransform_Rotation: {
        MTH4D_tdstVector_* column1 = &matrix->stMatrix.stCol_1;
        MTH4D_tdstVector_* column2 = &matrix->stMatrix.stCol_2;
        f32 zero = 0.0f;
        f32 x;
        f32 y;
        f32 z;

        matrix->ucType = MAT_eTransform_RotationScale;
        x = scale->xX;
        MTH3D_M_vKTimesXYZ_inv(&matrix->stMatrix.stCol_0, x, &matrix->stMatrix.stCol_0);
        matrix->stMatrix.stCol_0.xW = 1.0f;
        y = scale->xY;
        column1->xX = y * column1->xX;
        column1->xW = 1.0f;
        column1->xY = y * column1->xY;
        column1->xZ = y * column1->xZ;
        z = scale->xZ;
        column2->xX = z * column2->xX;
        column2->xW = 1.0f;
        column2->xY = z * column2->xY;
        column2->xZ = z * column2->xZ;
        matrix->stMatrix.stCol_2.xW = zero;
        matrix->stMatrix.stCol_1.xW = zero;
        matrix->stMatrix.stCol_0.xW = zero;
        *(MTH3D_tdstVector_*) &matrix->stScale = *scale;
        return;
    }
    case MAT_eTransform_RotationZoom:
        matrix->ucType = MAT_eTransform_RotationScale;
    case MAT_eTransform_RotationScale: {
        MTH3D_tdstVector_* zoom = (MTH3D_tdstVector_*) &matrix->stScale;
        MTH4D_tdstVector_* column1 = &matrix->stMatrix.stCol_1;
        MTH4D_tdstVector_* column2 = &matrix->stMatrix.stCol_2;
        f32 scaleRatio;
        f32 one = 1.0f;
        f32 zero = 0.0f;

        scaleRatio = scale->xX / zoom->xX;
        MTH3D_M_vKTimesXYZ_inv(&matrix->stMatrix.stCol_0, scaleRatio, &matrix->stMatrix.stCol_0);
        matrix->stMatrix.stCol_0.xW = one;
        scaleRatio = scale->xY / zoom->xY;
        column1->xX = scaleRatio * column1->xX;
        column1->xW = one;
        column1->xY = scaleRatio * column1->xY;
        column1->xZ = scaleRatio * column1->xZ;
        scaleRatio = scale->xZ / zoom->xZ;
        column2->xX = scaleRatio * column2->xX;
        column2->xW = one;
        column2->xY = scaleRatio * column2->xY;
        column2->xZ = scaleRatio * column2->xZ;
        matrix->stMatrix.stCol_2.xW = zero;
        matrix->stMatrix.stCol_1.xW = zero;
        matrix->stMatrix.stCol_0.xW = zero;
        *zoom = *scale;
        return;
    }
    case MAT_eTransform_ComplexRotationScale:
        return;
    }
}

void inv_POS_fn_vRemoveScaleFromMatrix(POS_tdstMatrix_* destination, POS_tdstMatrix_* source)
{
    switch (source->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation:
        if (destination != source) {
            *destination = *source;
        }
        return;

    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale: {
        MTH4D_tdstVector_* column1 = &destination->stMatrix.stCol_1;
        MTH4D_tdstVector_* column2 = &destination->stMatrix.stCol_2;
        MTH4D_tdstVector_* zoom = &destination->stScale;
        f32 one = 1.0f;

        destination->ucType = MAT_eTransform_Translation;
        MTH3D_M_vSetXYZ_inv(&destination->stMatrix.stCol_0, one, 0.0f, 0.0f);
        destination->stMatrix.stCol_0.xW = 0.0f;
        MTH3D_M_vSetXYZ_inv(column1, 0.0f, one, 0.0f);
        column1->xW = 0.0f;
        MTH3D_M_vSetXYZ_inv(column2, 0.0f, 0.0f, one);
        column2->xW = 0.0f;
        destination->stMatrix.stCol_3 = source->stMatrix.stCol_3;
        MTH3D_M_vSetXYZ_inv(zoom, one, one, one);
        zoom->xW = one;
        return;
    }

    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale: {
        MTH4D_tdstVector_* sourceZoom;
        MTH4D_tdstVector_* sourceColumn1;
        MTH4D_tdstVector_* column1;
        MTH4D_tdstVector_* sourceColumn2;
        MTH4D_tdstVector_* column2;
        MTH4D_tdstVector_* zoom;
        f32 inverseX;
        f32 inverseY;
        f32 inverseZ;
        f32 xZ;
        f32 yZ;
        f32 zZ;
        f32 one = 1.0f;
        f32 zero = 0.0f;

        destination->ucType = MAT_eTransform_Rotation;
        sourceZoom = &source->stScale;
        sourceColumn1 = &source->stMatrix.stCol_1;
        column1 = &destination->stMatrix.stCol_1;
        inverseX = sourceZoom->xX;
        sourceColumn2 = &source->stMatrix.stCol_2;
        column2 = &destination->stMatrix.stCol_2;
        inverseX = one / inverseX;
        zoom = &destination->stScale;
        destination->stMatrix.stCol_0.xX = inverseX * source->stMatrix.stCol_0.xX;
        destination->stMatrix.stCol_0.xY = inverseX * source->stMatrix.stCol_0.xY;
        xZ = source->stMatrix.stCol_0.xZ;
        destination->stMatrix.stCol_0.xW = one;
        destination->stMatrix.stCol_0.xZ = inverseX * xZ;

        inverseY = sourceZoom->xY;
        inverseY = one / inverseY;
        column1->xX = inverseY * sourceColumn1->xX;
        column1->xY = inverseY * sourceColumn1->xY;
        yZ = sourceColumn1->xZ;
        column1->xW = one;
        column1->xZ = inverseY * yZ;

        inverseZ = sourceZoom->xZ;
        inverseZ = one / inverseZ;
        column2->xX = inverseZ * sourceColumn2->xX;
        column2->xY = inverseZ * sourceColumn2->xY;
        zZ = sourceColumn2->xZ;
        column2->xW = one;
        column2->xZ = inverseZ * zZ;

        destination->stMatrix.stCol_3 = source->stMatrix.stCol_3;
        destination->stMatrix.stCol_2.xW = zero;
        destination->stMatrix.stCol_1.xW = zero;
        destination->stMatrix.stCol_0.xW = zero;
        zoom->xX = one;
        zoom->xY = one;
        zoom->xZ = one;
        zoom->xW = one;
        return;
    }

    case MAT_eTransform_ComplexRotationScale:
        return;
    }
}

void POS_fn_vGetScaleMatrix(POS_tdstMatrix_* matrix, MTH3D_tdstVector_* scale)
{
    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation:
        MTH3D_M_vSetXYZ_inv(scale, 1.0f, 1.0f, 1.0f);
        break;
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
        *scale = *(MTH3D_tdstVector_*) &matrix->stScale;
        break;
    case MAT_eTransform_ComplexRotationScale:
        break;
    }
}

extern f32 D_003928C8;

static inline f32 inv_POS_fn_xSquareLength(MTH4D_tdstVector_* column)
{
    f32 result;

    result = MTH_fn_xSqrAddSqrAddSqrOpt2(column->xX, column->xY, column->xZ);
    return result;
}

f32 POS_fn_xGetMaxScale(POS_tdstMatrix_* matrix)
{
    MTH4D_tdstVector_* scale;
    f32 first;
    f32 second;
    f32 third;
    f32 maximum;

    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation:
        maximum = 1.0f;
        break;
    case MAT_eTransform_Zoom:
    case MAT_eTransform_RotationZoom:
        maximum = inv_MTH_fn_xAbsolute(matrix->stScale.xX);
        break;
    case MAT_eTransform_Scale:
    case MAT_eTransform_RotationScale:
        scale = &matrix->stScale;
        maximum =
            inv_MTH_fn_xMaximum(inv_MTH_fn_xAbsolute(scale->xX), inv_MTH_fn_xAbsolute(scale->xY));
        maximum = inv_MTH_fn_xMaximum(maximum, inv_MTH_fn_xAbsolute(scale->xZ));
        break;
    case MAT_eTransform_ComplexRotationScale:
        first = inv_POS_fn_xSquareLength(&matrix->stMatrix.stCol_0);
        second = inv_POS_fn_xSquareLength(&matrix->stMatrix.stCol_1);
        maximum = first > second ? first : second;
        third = inv_POS_fn_xSquareLength(&matrix->stMatrix.stCol_2);
        maximum = maximum > third ? maximum : third;
        maximum = MTH_fn_xSqrtOpt2(maximum);
        maximum = maximum * D_003928C8;
        break;
    default:
        maximum = 0.0f;
        break;
    }
    return maximum;
}

f32 POS_fn_xGetXScale(POS_tdstMatrix_* matrix)
{
    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation:
        return 1.0f;

    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        return matrix->stScale.xX;

    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        return MTH_fn_xSqrtOpt2(matrix->stMatrix.stCol_0.xX * matrix->stMatrix.stCol_0.xX +
                          matrix->stMatrix.stCol_1.xX * matrix->stMatrix.stCol_1.xX +
                          matrix->stMatrix.stCol_2.xX * matrix->stMatrix.stCol_2.xX);

    default:
        return 0.0f;
    }
}

f32 func_00127A98(POS_tdstMatrix_* matrix)
{
    f32 x;
    f32 y;
    f32 z;

    switch (matrix->ucType) {
    case MAT_eTransform_Identity:
    case MAT_eTransform_Translation:
    case MAT_eTransform_Rotation:
        return 1.0f;
    case MAT_eTransform_Zoom:
    case MAT_eTransform_Scale:
        return matrix->stScale.xY;
    case MAT_eTransform_RotationZoom:
    case MAT_eTransform_RotationScale:
    case MAT_eTransform_ComplexRotationScale:
        x = matrix->stMatrix.stCol_0.xY;
        y = matrix->stMatrix.stCol_1.xY;
        z = matrix->stMatrix.stCol_2.xY;
        return MTH_fn_xSqrtOpt2(MTH_fn_xSqrOpt1(x) + MTH_fn_xSqrOpt1(y) + MTH_fn_xSqrOpt1(z));
    default:
        return 0.0f;
    }
}

void POS_fn_vQuatToMat(MTH4D_tdstMatrix_* matrix, f32 x, f32 y, f32 z, f32 w)
{
    f32 inverseLength;
    f32 one;
    f32 a;
    f32 b;
    f32 c;
    f32 d;
    f32 x2;
    f32 y2;
    f32 z2;
    f32 xx;
    f32 xy;
    f32 xz;
    f32 yy;
    f32 yz;
    f32 zz;
    f32 wx;
    f32 wy;
    f32 wz;

    R5900_LEN4_ORDERED(inverseLength, x, y, z, w);
    one = 1.0f;
    inverseLength = inv_MTH_fn_xDivideBySqrtTied(one, inverseLength);

    a = x * inverseLength;
    b = y * inverseLength;
    c = z * inverseLength;
    d = w * inverseLength;

    x2 = a + a;
    y2 = b + b;
    z2 = c + c;

    xx = a * x2;
    yy = b * y2;
    zz = c * z2;
    xy = a * y2;
    xz = a * z2;
    wx = d * x2;
    yz = b * z2;
    wy = d * y2;
    wz = d * z2;

    matrix->stCol_1.xX = xy + wz;
    matrix->stCol_2.xX = xz - wy;
    matrix->stCol_2.xY = yz + wx;
    MTH3D_M_vSetXYZ_inv(&matrix->stCol_0, one - (yy + zz), xy - wz, xz + wy);
    matrix->stCol_1.xY = one - (xx + zz);
    matrix->stCol_2.xZ = one - (xx + yy);
    matrix->stCol_1.xZ = yz - wx;

    MTH3D_M_vZeroXYZ_inv(&matrix->stCol_3);
    matrix->stCol_0.xW = 0.0f;
    matrix->stCol_1.xW = 0.0f;
    matrix->stCol_2.xW = 0.0f;
    matrix->stCol_3.xW = one;
}
