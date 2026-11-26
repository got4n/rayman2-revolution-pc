#include "common.h"

#include "MTH/MTH.h"

f32 cosf(f32 angle);

void func_00113B58(POS_tdstMatrix_* matrix, f32 angle)
{
    MTH3D_tdstVector_ column0;
    MTH3D_tdstVector_ column1;
    MTH3D_tdstVector_ column2;
    f32 reduced;
    f32 cosine;
    f32 sign;
    f32 sine;

    angle = angle + MTH_HALF_PI_inv;
    reduced = angle - (f32) inv_MTH_fn_lToInteger(angle * MTH_INV_TWO_PI_inv) * MTH_TWO_PI_inv;
    cosine = cosf(angle);
    if (inv_MTH_fn_xAbsolute(reduced) < MTH_PI_inv) {
        sign = inv_MTH_fn_xSign(reduced);
    } else {
        sign = -inv_MTH_fn_xSign(reduced);
    }
    sine = MTH_fn_xSqrtOpt2(1.0f - cosine * cosine) * sign;

    MTH3D_M_vSetXYZ_inv(&column0, cosine, sine, 0.0f);
    MTH3D_M_vSetXYZ_inv(&column1, inv_MTH_fn_xNegate(sine), cosine, 0.0f);
    MTH3D_M_vSetXYZ_inv(&column2, 0.0f, 0.0f, 1.0f);
    _POS_fn_vSetRotationMatrix(matrix, &column0, &column1, &column2);
}

void fn_vTurnMatrixX(POS_tdstMatrix_* matrix, f32 angle)
{
    POS_tdstMatrix_ rotation;
    MTH3D_tdstVector_ column0;
    MTH3D_tdstVector_ column1;
    MTH3D_tdstVector_ column2;
    s32 turns;
    f32 reduced;
    f32 cosine;
    f32 sign;
    f32 sine;
    f32 zero;

    gu_MTH4D_M_vSetIdentityMatrix(&rotation);
    turns = inv_MTH_fn_lToIntegerOrdered(angle * MTH_INV_TWO_PI_inv);
    reduced = angle - (f32) turns * MTH_TWO_PI_inv;
    cosine = cosf(angle);
    if (inv_MTH_fn_xAbsolute(reduced) < MTH_PI_inv) {
        sign = inv_MTH_fn_xSign(reduced);
    } else {
        sign = -inv_MTH_fn_xSign(reduced);
    }
    sine = MTH_fn_xSqrtOpt2(1.0f - cosine * cosine) * sign;
    if (inv_MTH_fn_xAbsolute(cosine) < 1e-4f) {
        cosine = 0.0f;
    }
    if (inv_MTH_fn_xAbsolute(sine) < 1e-4f) {
        sine = 0.0f;
    }

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&column0, 1.0f, zero, zero);
    MTH3D_M_vSetXYZ_inv(&column1, zero, cosine, sine);
    MTH3D_M_vSetXYZ_inv(&column2, zero, inv_MTH_fn_xNegate(sine), cosine);
    _POS_fn_vSetRotationMatrix(&rotation, &column0, &column1, &column2);
    _POS_fn_vMulMatrixMatrix(matrix, &rotation, matrix);
}

void fn_vTurnMatrixY(POS_tdstMatrix_* matrix, f32 angle)
{
    POS_tdstMatrix_ rotation;
    MTH3D_tdstVector_ I;
    MTH3D_tdstVector_ J;
    MTH3D_tdstVector_ K;
    s32 turns;
    f32 xCos;
    f32 xSin;
    f32 sign;
    f32 sine;

    gu_MTH4D_M_vSetIdentityMatrix(&rotation);
    turns = inv_MTH_fn_lToIntegerOrdered(angle * MTH_INV_TWO_PI_inv);
    xCos = angle - (f32) turns * MTH_TWO_PI_inv;
    xSin = cosf(angle);
    if (inv_MTH_fn_xAbsolute(xCos) < MTH_PI_inv) {
        sign = inv_MTH_fn_xSign(xCos);
    } else {
        sign = -inv_MTH_fn_xSign(xCos);
    }
    sine = MTH_fn_xSqrtOpt2(1.0f - xSin * xSin) * sign;
    if (inv_MTH_fn_xAbsolute(xSin) < 1e-4f) {
        xSin = 0.0f;
    }
    if (inv_MTH_fn_xAbsolute(sine) < 1e-4f) {
        sine = 0.0f;
    }

    MTH3D_M_vSetXYZ_inv(&I, xSin, 0.0f, inv_MTH_fn_xNegate(sine));
    MTH3D_M_vSetXYZ_inv(&J, 0.0f, 1.0f, 0.0f);
    MTH3D_M_vSetXYZ_inv(&K, sine, 0.0f, xSin);
    _POS_fn_vSetRotationMatrix(&rotation, &I, &J, &K);
    _POS_fn_vMulMatrixMatrix(matrix, &rotation, matrix);
}

void fn_vTurnMatrixZ(POS_tdstMatrix_* matrix, f32 angle)
{
    POS_tdstMatrix_ rotation;
    MTH3D_tdstVector_ I;
    MTH3D_tdstVector_ J;
    MTH3D_tdstVector_ K;
    s32 turns;
    f32 xCos;
    f32 xSin;
    f32 sign;
    f32 sine;

    gu_MTH4D_M_vSetIdentityMatrix(&rotation);
    turns = inv_MTH_fn_lToIntegerOrdered(angle * MTH_INV_TWO_PI_inv);
    xCos = angle - (f32) turns * MTH_TWO_PI_inv;
    xSin = cosf(angle);
    if (inv_MTH_fn_xAbsolute(xCos) < MTH_PI_inv) {
        sign = inv_MTH_fn_xSign(xCos);
    } else {
        sign = -inv_MTH_fn_xSign(xCos);
    }
    sine = MTH_fn_xSqrtOpt2(1.0f - xSin * xSin) * sign;
    if (inv_MTH_fn_xAbsolute(xSin) < 1e-4f) {
        xSin = 0.0f;
    }
    if (inv_MTH_fn_xAbsolute(sine) < 1e-4f) {
        sine = 0.0f;
    }

    MTH3D_M_vSetXYZ_inv(&I, xSin, sine, 0.0f);
    MTH3D_M_vSetXYZ_inv(&J, inv_MTH_fn_xNegate(sine), xSin, 0.0f);
    MTH3D_M_vSetXYZ_inv(&K, I.xZ, I.xZ, 1.0f);
    _POS_fn_vSetRotationMatrix(&rotation, &I, &J, &K);
    _POS_fn_vMulMatrixMatrix(matrix, &rotation, matrix);
}
