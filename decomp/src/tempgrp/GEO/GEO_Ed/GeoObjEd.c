#include "common.h"

#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "MTH/MTH.h"

u8 GEO_bIsLookAt(GeometricObjectView* p_stObj)
{
    return p_stObj->flags & 3;
}

static inline void inv_GEO_fn_vNormalize(MTH4D_tdstVector_* vector)
{
    f32 length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(vector->xX, vector->xY, vector->xZ));

    if (length != 0.0f) {
        f32 inverse = 1.0f / length;

        MTH3D_M_vKTimesXYZ_inv(vector, inverse, vector);
    }
}

void GEO_vTurnLookAt(GeometricObjectView* geometry, GLD_tdstViewportAttributes_* viewport,
                     POS_tdstMatrix_* matrix)
{
    POS_tdstMatrix_ inverse;
    MTH4D_tdstVector_ direction;
    MTH4D_tdstVector_ side;
    MTH4D_tdstVector_ up;
    MTH4D_tdstVector_* target;
    MTH4D_tdstVector_* origin;

    POS_fn_vInvertIsoMatrix(&inverse, &(*(GLI_tdstCamera_**) viewport->p_vSpecificToXD)->stMatrix);
    direction.xX = inverse.stMatrix.stCol_3.xX - matrix->stMatrix.stCol_3.xX;
    target = &inverse.stMatrix.stCol_3;
    origin = &matrix->stMatrix.stCol_3;
    direction.xY = target->xY - origin->xY;
    direction.xZ = target->xZ - origin->xZ;
    if (MTH3D_M_bIsZeroXYZ_inv(&direction))
        return;
    inv_GEO_fn_vNormalize(&direction);
    if (geometry->flags & 2)
        *(MTH3D_tdstVector_*) &up = *(MTH3D_tdstVector_*) &matrix->stMatrix.stCol_2;
    else
        *(MTH3D_tdstVector_*) &up = *(MTH3D_tdstVector_*) &inverse.stMatrix.stCol_1;
    inv_GEO_fn_vNormalize(&up);
    {
        f32 y = direction.xY;
        f32 z = direction.xZ;
        f32 x = direction.xX;
        f32 length;

        MTH3D_M_vSetXYZ_inv(&side, inv_MTH_fn_xMulSub(up.xY, z, up.xZ, y),
                            inv_MTH_fn_xMulSub(up.xZ, x, up.xX, z),
                            inv_MTH_fn_xMulSub(up.xX, y, up.xY, x));
        length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(side.xX, side.xY, side.xZ));
        if (0.0f < length) {
            MTH3D_M_vSetXYZ_inv(&side, side.xX / length, side.xY / length, side.xZ / length);
            DNM_p_stVectorCrossProduct(&direction, &side, &up);
            _POS_fn_vSetRotationMatrix(matrix, (MTH3D_tdstVector_*) &direction, (MTH3D_tdstVector_*) &side,
                                       (MTH3D_tdstVector_*) &up);
        }
    }
}
