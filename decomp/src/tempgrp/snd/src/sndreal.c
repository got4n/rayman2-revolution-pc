#include "common.h"

#include "snd/snd.h"

s32 SND_fn_rAbsRealSnd(s32 value);
f32 sqrtf(f32 value);

s32 SND_fn_rMulRealRealSnd(s32 left, s32 right)
{
    u32 a = __builtin_abs(left);
    u32 b = __builtin_abs(right);
    u32 aHigh;
    u32 bHigh;
    u32 low;
    u32 cross1;
    u32 cross2;
    u32 high;
    s32 result;

    if (a < 0x800 && b < 0x800 && a >= 0x100 && b >= 0x100)
        return (left >> 8) * (right >> 8);
    if (a < 0x7F)
        return (left * (right >> 8)) >> 8;
    if (b < 0x7F)
        return (right * (left >> 8)) >> 8;

    bHigh = b >> 16;
    aHigh = a >> 16;
    a &= 0xFFFF;
    b &= 0xFFFF;
    low = a * b;
    cross1 = a * bHigh;
    cross2 = aHigh * b;
    high = aHigh * bHigh;
    result = ((low >> 16) + cross2) + ((high << 16) + cross1);
    if ((left < 0) ^ (right < 0)) {
        if (bHigh & 0x10000)
            return 0;
        return -result;
    }
    if (bHigh & 0x10000)
        return 0;
    return result;
}

s32 SND_fn_rMulRealRealQuickSnd(s32 left, s32 right)
{
    return (left >> 8) * (right >> 8);
}

s32 func_00160D68(s32 value)
{
    return (s32) (sqrtf((f32) value * (1.0f / 65536.0f)) * 65536.0f);
}

s32 SND_fn_rDivRealRealQuickSnd(s32 numerator, s32 denominator)
{
    if (SND_fn_rAbsRealSnd(numerator) <= 0x7FFF)
        return (numerator << 16) / denominator;
    if (SND_fn_rAbsRealSnd(numerator) <= 0x7FFFFF)
        return (numerator << 8) / (denominator >> 8);
    if (SND_fn_rAbsRealSnd(denominator) > 0x8000)
        return (numerator / (denominator >> 8)) << 8;
    return (numerator / denominator) << 16;
}

s32 SND_fn_rDivRealRealSnd(s32 numerator, s32 denominator)
{
    return SND_fn_rDivRealRealQuickSnd(numerator, denominator);
}

s32 SND_fn_rAbsRealSnd(s32 value)
{
    return value < 0 ? -value : value;
}

s32 SND_fn_rPseudoNormeRealSnd(s32 x, s32 y, s32 z)
{
    SndFixedVector3 vector;

    vector.x = x;
    vector.y = y;
    vector.z = z;
    return SND_fn_rPseudoNormeVectorSnd(&vector);
}

void SND_fn_vSndRealTwinToSndReal(s32 twin, s32* high, s32* low)
{
    if (high != NULL)
        *high = (twin >> 16) << 8;
    if (low != NULL)
        *low = (twin << 16) >> 8;
}

s32 func_00160F28(s32 high, s32 low)
{
    return ((high << 8) & 0xFFFF0000) | ((low >> 8) & 0xFFFF);
}
