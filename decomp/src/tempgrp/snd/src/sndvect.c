#include "common.h"

#include "snd/snd.h"

static inline s32 inv_SND_fn_lAbsolute(s32 value)
{
    return value < 0 ? -value : value;
}

s32 calcul_pseudo_d(s32 x, s32 y)
{
    x = x < 0 ? -x : x;
    y = y < 0 ? -y : y;
    if (x < y)
        return y + (x >> 1);
    return x + (y >> 1);
}

s32 SND_fn_rPseudoNormeVectorSnd(const SndFixedVector3* vector)
{
    s32 z = inv_SND_fn_lAbsolute(vector->z);
    s32 y = inv_SND_fn_lAbsolute(vector->y);
    s32 x = inv_SND_fn_lAbsolute(vector->x);

    return calcul_pseudo_d(calcul_pseudo_d(x, y), z);
}

s32 SND_fn_rScalaireVectorSnd(const SndFixedVector3* left, const SndFixedVector3* right)
{
    s32 x = SND_fn_rMulRealRealSnd(left->x, right->x);
    s32 y = SND_fn_rMulRealRealSnd(left->y, right->y);
    s32 z = SND_fn_rMulRealRealSnd(left->z, right->z);

    return x + y + z;
}

s32 SND_fn_rPseudoScalaireNormeVectorSnd(const SndFixedVector3* left, const SndFixedVector3* right)
{
    s32 x = SND_fn_rMulRealRealQuickSnd(left->x, right->x);
    s32 y = SND_fn_rMulRealRealQuickSnd(left->y, right->y);
    s32 z = SND_fn_rMulRealRealQuickSnd(left->z, right->z);

    return x + y + z;
}

void SND_fn_vProduitVectorSnd(SndFixedVector3* left, SndFixedVector3* right,
                              SndFixedVector3* result)
{
    result->x =
        SND_fn_rMulRealRealSnd(left->y, right->z) - SND_fn_rMulRealRealSnd(left->z, right->y);
    result->y =
        SND_fn_rMulRealRealSnd(left->z, right->x) - SND_fn_rMulRealRealSnd(left->x, right->z);
    result->z =
        SND_fn_rMulRealRealSnd(left->x, right->y) - SND_fn_rMulRealRealSnd(left->y, right->x);
}
