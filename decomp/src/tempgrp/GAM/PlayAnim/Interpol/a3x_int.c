#include "common.h"

#include "GAM/GAM.h"

#include "MTH/MTH.h"

#define SINE_MASK 0x1FFF

f32 sinf(f32 x);

void fn_v_InitSinTab(void)
{
    s32 slCnt;

    for (slCnt = 0; slCnt < 0x1000; slCnt++) {
        ax_SinTab[slCnt] =
            inv_MTH_fn_lToIntegerOrdered(32767.0f * sinf(MTH_TWO_PI_inv * (f32) slCnt / 8192.0f) + 0.5f);
    }
    for (slCnt = 0x1000; slCnt < 0x2000; slCnt++) {
        ax_SinTab[slCnt] =
            inv_MTH_fn_lToIntegerOrdered(32767.0f * sinf(MTH_TWO_PI_inv * (f32) slCnt / 8192.0f) - 0.5f);
    }
}

void fn_v_ExpandQuat(f32* _a4_xQuatD, s16* _a4_xQuatS)
{
    f32 xCoef = 1.0f / 32767.0f;

    _a4_xQuatD[0] = _a4_xQuatS[0] * xCoef;
    _a4_xQuatD[1] = _a4_xQuatS[1] * xCoef;
    _a4_xQuatD[2] = _a4_xQuatS[2] * xCoef;
    _a4_xQuatD[3] = _a4_xQuatS[3] * xCoef;
}

void fn_v_CopyQuat(f32* destination, f32* source)
{
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = source[2];
    destination[3] = source[3];
}

void fn_v_QuatToMatrix(f32 (*_a3a3_xMtx)[3], f32* _a4_xQuat)
{
    f32 xXS;
    f32 xYS;
    f32 xZS;
    f32 xWX;
    f32 xWY;
    f32 xWZ;
    f32 xXX;
    f32 xXY;
    f32 xXZ;
    f32 xYY;
    f32 xYZ;
    f32 xZZ;

    xXS = _a4_xQuat[0];
    xXS += xXS;
    xYS = _a4_xQuat[1];
    xYS += xYS;
    xZS = _a4_xQuat[2];
    xZS += xZS;
    xWX = _a4_xQuat[3] * xXS;
    xWY = _a4_xQuat[3] * xYS;
    xWZ = _a4_xQuat[3] * xZS;
    xXX = _a4_xQuat[0] * xXS;
    xXY = _a4_xQuat[0] * xYS;
    xXZ = _a4_xQuat[0] * xZS;
    xYY = _a4_xQuat[1] * xYS;
    xYZ = _a4_xQuat[1] * xZS;
    xZZ = _a4_xQuat[2] * xZS;

    _a3a3_xMtx[0][0] = 1.0f - (xYY + xZZ);
    _a3a3_xMtx[1][0] = xXY + xWZ;
    _a3a3_xMtx[2][0] = xXZ - xWY;
    _a3a3_xMtx[0][1] = xXY - xWZ;
    _a3a3_xMtx[1][1] = 1.0f - (xXX + xZZ);
    _a3a3_xMtx[2][1] = xYZ + xWX;
    _a3a3_xMtx[0][2] = xXZ + xWY;
    _a3a3_xMtx[1][2] = xYZ - xWX;
    _a3a3_xMtx[2][2] = 1.0f - (xXX + xYY);
}

void fn_v_InterpolQuatWithOmega(f32* result, f32* from, f32* to, s16 angle, f32 t)
{
    f32 target[4];
    s32 sine;
    s32 step;
    f32 reciprocal;
    f32 toWeight;
    f32 fromWeight;

    fn_v_CopyQuat(target, to);

    sine = ax_SinTab[((s64) angle + 0x800) & SINE_MASK];
    if (sine < 0) {
        target[0] = -target[0];
        target[1] = -target[1];
        target[2] = -target[2];
        target[3] = -target[3];
        sine = (s16) -sine;
        angle = 0x1000 - angle;
    }

    if (sine >= -0x7FFD) {
        sine = ax_SinTab[angle & SINE_MASK];
        if (sine != 0) {
            step = inv_MTH_fn_lToInteger(t * (f32) angle);
            step = (s16) step;
            reciprocal = 1.0f / (f32) sine;
            toWeight = (f32) ax_SinTab[step & SINE_MASK] * reciprocal;
            fromWeight = (f32) ax_SinTab[(angle - step) & SINE_MASK] * reciprocal;
        } else {
            fromWeight = 1.0f - t;
            toWeight = t;
        }
        result[0] = fromWeight * from[0] + toWeight * target[0];
        result[1] = fromWeight * from[1] + toWeight * target[1];
        result[2] = fromWeight * from[2] + toWeight * target[2];
        result[3] = fromWeight * from[3] + toWeight * target[3];
    } else {
        f32 fromX = from[0];
        f32 targetX;
        f32 targetY;
        f32 targetZ;
        f32 targetW;
        s32 rawStep;

        rawStep = inv_MTH_fn_lToInteger(t * 2048.0f);
        targetY = target[1];
        targetX = target[0];
        step = (s16) rawStep;
        targetW = target[3];
        targetZ = target[2];
        toWeight = ax_SinTab[step & SINE_MASK];
        fromWeight = ax_SinTab[((s64) 0x800 - (s64) (s16) step) & SINE_MASK];
        result[0] = fromWeight * fromX - toWeight * targetY;
        result[1] = fromWeight * from[1] + toWeight * targetX;
        result[2] = fromWeight * from[2] - toWeight * targetW;
        result[3] = targetZ;
    }
}
