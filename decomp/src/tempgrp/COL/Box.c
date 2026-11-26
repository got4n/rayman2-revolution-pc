#include "common.h"

#include "MTH/MTH.h"

void COL_fn_vMinMaxBox2VerticesBox(MTH3D_tdstVector_* a8_stBoxDest, MTH3D_tdstVector_* p_stMinPoint,
                                   MTH3D_tdstVector_* p_stMaxPoint)
{
    MTH3D_M_vCopyXYZ_inv(&a8_stBoxDest[0], p_stMinPoint);
    MTH3D_M_vSetXYZ_inv(&a8_stBoxDest[1], p_stMinPoint->xX, p_stMinPoint->xY, p_stMaxPoint->xZ);
    MTH3D_M_vSetXYZ_inv(&a8_stBoxDest[2], p_stMinPoint->xX, p_stMaxPoint->xY, p_stMinPoint->xZ);
    MTH3D_M_vSetXYZ_inv(&a8_stBoxDest[3], p_stMinPoint->xX, p_stMaxPoint->xY, p_stMaxPoint->xZ);
    MTH3D_M_vSetXYZ_inv(&a8_stBoxDest[4], p_stMaxPoint->xX, p_stMinPoint->xY, p_stMinPoint->xZ);
    MTH3D_M_vSetXYZ_inv(&a8_stBoxDest[5], p_stMaxPoint->xX, p_stMinPoint->xY, p_stMaxPoint->xZ);
    MTH3D_M_vSetXYZ_inv(&a8_stBoxDest[6], p_stMaxPoint->xX, p_stMaxPoint->xY, p_stMinPoint->xZ);
    MTH3D_M_vCopyXYZ_inv(&a8_stBoxDest[7], p_stMaxPoint);
}

void COL_fn_vComputeBoundingBoxOfSphere(MTH3D_tdstVector_* p_stMinBounding,
                                        MTH3D_tdstVector_* p_stMaxBounding,
                                        MTH3D_tdstVector_* p_stSphereCenter, f32 xSphereRadius)
{
    MTH3D_M_vSetXYZ_inv(p_stMinBounding, p_stSphereCenter->xX - xSphereRadius,
                        p_stSphereCenter->xY - xSphereRadius, p_stSphereCenter->xZ - xSphereRadius);
    MTH3D_M_vSetXYZ_inv(p_stMaxBounding, p_stSphereCenter->xX + xSphereRadius,
                        p_stSphereCenter->xY + xSphereRadius, p_stSphereCenter->xZ + xSphereRadius);
}

void COL_fn_vComputeBoundingBoxOfSphereAndBox(MTH3D_tdstVector_* p_stMinBounding, MTH3D_tdstVector_* p_stMaxBounding,
                   MTH3D_tdstVector_* p_stSphereCenter, MTH3D_tdstVector_* p_stMinPoint,
                   MTH3D_tdstVector_* p_stMaxPoint, f32 xSphereRadius)
{
    MTH3D_M_vSetXYZ_inv(
        p_stMinBounding,
        inv_MTH_fn_xMinimum(p_stSphereCenter->xX - xSphereRadius, p_stMinPoint->xX),
        inv_MTH_fn_xMinimum(p_stSphereCenter->xY - xSphereRadius, p_stMinPoint->xY),
        inv_MTH_fn_xMinimum(p_stSphereCenter->xZ - xSphereRadius, p_stMinPoint->xZ));
    MTH3D_M_vSetXYZ_inv(
        p_stMaxBounding,
        inv_MTH_fn_xMaximum(p_stSphereCenter->xX + xSphereRadius, p_stMaxPoint->xX),
        inv_MTH_fn_xMaximum(p_stSphereCenter->xY + xSphereRadius, p_stMaxPoint->xY),
        inv_MTH_fn_xMaximum(p_stSphereCenter->xZ + xSphereRadius, p_stMaxPoint->xZ));
}

u8 COL_fn_bDetectCollisionBetweenTwoParallelBoxes(MTH3D_tdstVector_* _p_stFirstBoxMaxPoint, MTH3D_tdstVector_* _p_stFirstBoxMinPoint, MTH3D_tdstVector_* _p_stSecondBoxMaxPoint, MTH3D_tdstVector_* _p_stSecondBoxMinPoint)
{
    if (_p_stSecondBoxMaxPoint->xX > _p_stFirstBoxMinPoint->xX &&
        _p_stSecondBoxMinPoint->xX < _p_stFirstBoxMaxPoint->xX &&
        _p_stSecondBoxMaxPoint->xY > _p_stFirstBoxMinPoint->xY &&
        _p_stSecondBoxMinPoint->xY < _p_stFirstBoxMaxPoint->xY &&
        _p_stSecondBoxMaxPoint->xZ > _p_stFirstBoxMinPoint->xZ &&
        _p_stSecondBoxMinPoint->xZ < _p_stFirstBoxMaxPoint->xZ) {
        return 1;
    }
    return 0;
}
