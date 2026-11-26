#include "common.h"

#include "MTH/MTH.h"
#include "GAM/GAM.h"

s32 MEC_fn_bIsEqualMatrix(POS_tdstMatrix_* left, POS_tdstMatrix_* right)
{
    POS_tdstMatrix_ unscaledLeft;
    POS_tdstMatrix_ unscaledRight;
    POS_tdstMatrix_* comparedLeft;
    POS_tdstMatrix_* comparedRight;
    s32 scaleRemoved = 0;
    s32 scaled;

    scaled = inv_POS_fn_bIsScaleMatrix(left);
    if (scaled) {
        comparedLeft = &unscaledLeft;
        inv_POS_fn_vRemoveScaleFromMatrix(&unscaledLeft, left);
        scaleRemoved = 1;
    } else {
        comparedLeft = left;
    }
    scaled = inv_POS_fn_bIsScaleMatrix(right);
    if (scaled) {
        comparedRight = &unscaledRight;
        inv_POS_fn_vRemoveScaleFromMatrix(&unscaledRight, right);
        scaleRemoved = 1;
    } else {
        comparedRight = right;
    }
    if (scaleRemoved)
        return inv_POS_fn_bIsEqualMatrixEpsilon(comparedLeft, comparedRight, 0.0001f);
    return inv_POS_fn_bIsEqualMatrixExact(comparedLeft, comparedRight);
}
