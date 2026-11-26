#include "common.h"

#include "MTH/MTH.h"

#include "ps2/vu0.h"
#include "renderware.h"

#include "include_asm.h"
#include "COL/COL.h"
#include "ps2/PS2.h"
#include "ps2/dma.h"

#ifdef PC_PORT
#define SCRATCH_VECTOR ((ScratchVector*) PC_SCRATCH(0x70003FF0))
#else
#define SCRATCH_VECTOR ((ScratchVector*) 0x70003FF0)
#endif

extern u8 D_F30[];
extern u8 D_3A8[];
extern u8 D_BASE[] __asm__("0x0");

extern MTH3D_tdstVector_ inv_INT_stBoxNormalMinX;
extern MTH3D_tdstVector_ inv_INT_stBoxNormalMaxX;
extern MTH3D_tdstVector_ inv_INT_stBoxNormalMinY;
extern MTH3D_tdstVector_ inv_INT_stBoxNormalMaxY;
extern MTH3D_tdstVector_ inv_INT_stBoxNormalMinZ;
extern MTH3D_tdstVector_ inv_INT_stBoxNormalMaxZ;

s32 INT_fn_xTrinomeResolution(f32* p_xT1, f32* p_xT2, f32 xA, f32 xB, f32 xC)
{
    f32 xDelta;

    if (xA != 0.0f) {
        xDelta = xA * xC * 4.0f;
        xDelta = xB * xB - xDelta;
        if (xDelta > 0.0f) {
            f32 xDiv;

            xDelta = MTH_fn_xSqrtOpt2(xDelta);
            xB = inv_MTH_fn_xNegateTied(xB);
            xDiv = 1.0f / (xA + xA);
            *p_xT1 = (xB + xDelta) * xDiv;
            *p_xT2 = (xB - xDelta) * xDiv;
            return 2;
        }
        if (xDelta < 0.0f)
            return 0;
        if (xDelta == 0.0f) {
            *p_xT1 = inv_MTH_fn_xNegateTied(xB / (xA + xA));
            return 1;
        }
    } else {
        if (xB == 0.0f)
            return 0;
        *p_xT1 = inv_MTH_fn_xNegateTied(xC / xB);
        return 1;
    }
    return 0;
}

s32 INT_fn_bIntersectBoxWithBox(MTH3D_tdstVector_* p_stMinPoint1, MTH3D_tdstVector_* p_stMaxPoint1,
                                 MTH3D_tdstVector_* p_stMinPoint2, MTH3D_tdstVector_* p_stMaxPoint2)
{
    s32 bIntersect = 0;

    if (inv_MTH_fn_xMaximum(p_stMinPoint1->xX, p_stMinPoint2->xX) <=
            inv_MTH_fn_xMinimum(p_stMaxPoint1->xX, p_stMaxPoint2->xX) &&
        inv_MTH_fn_xMaximum(p_stMinPoint1->xY, p_stMinPoint2->xY) <=
            inv_MTH_fn_xMinimum(p_stMaxPoint1->xY, p_stMaxPoint2->xY) &&
        inv_MTH_fn_xMaximum(p_stMinPoint1->xZ, p_stMinPoint2->xZ) <=
            inv_MTH_fn_xMinimum(p_stMaxPoint1->xZ, p_stMaxPoint2->xZ))
        bIntersect = 1;
    return bIntersect;
}

u8 INT_fn_bIntersectSphereWithBox(MTH3D_tdstVector_* p_stCenter, MTH3D_tdstVector_* p_stMinPoint, MTH3D_tdstVector_* p_stMaxPoint, f32 xRadius)
{
    f32 xDist = 0.0f;
    f32 delta;
    s32 intersects;

    if (p_stCenter->xX < p_stMinPoint->xX) {
        delta = p_stCenter->xX - p_stMinPoint->xX;
        xDist += delta * delta;
    } else if (p_stMaxPoint->xX < p_stCenter->xX) {
        delta = p_stCenter->xX - p_stMaxPoint->xX;
        xDist += delta * delta;
    }
    if (p_stCenter->xY < p_stMinPoint->xY) {
        delta = p_stCenter->xY - p_stMinPoint->xY;
        xDist += delta * delta;
    } else if (p_stMaxPoint->xY < p_stCenter->xY) {
        delta = p_stCenter->xY - p_stMaxPoint->xY;
        xDist += delta * delta;
    }
    if (p_stCenter->xZ < p_stMinPoint->xZ) {
        delta = p_stCenter->xZ - p_stMinPoint->xZ;
        xDist += delta * delta;
    } else if (p_stMaxPoint->xZ < p_stCenter->xZ) {
        delta = p_stCenter->xZ - p_stMaxPoint->xZ;
        xDist += delta * delta;
    }
    intersects = 1;
    if (!(xDist < xRadius * xRadius)) {
        intersects = 0;
    }
    return intersects;
}

s32 INT_fn_bIntersectSphereWithSphere(MTH3D_tdstVector_* p_stCenter1,
                                         MTH3D_tdstVector_* p_stCenter2, f32 xRadius1, f32 xRadius2)
{
    f32 radiusSumSquared = (xRadius1 + xRadius2) * (xRadius1 + xRadius2);
    f32 dx = p_stCenter1->xX - p_stCenter2->xX;
    f32 dy = p_stCenter1->xY - p_stCenter2->xY;
    f32 dz = p_stCenter1->xZ - p_stCenter2->xZ;
    f32 xDist;
    s32 intersects;

    xDist = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    intersects = xDist <= radiusSumSquared;
    return intersects;
}

s32 INT_fn_ulGetPositionPointWithBox(MTH3D_tdstVector_* p_stPoint, MTH3D_tdstVector_* p_stMinPoint,
                                      MTH3D_tdstVector_* p_stMaxPoint)
{
    s32 ulOutCode = 0;
    if (p_stPoint->xX < p_stMinPoint->xX) {
        ulOutCode = 1;
    } else {
        if (p_stMaxPoint->xX < p_stPoint->xX) {
            ulOutCode = 2;
        }
    }
    if (p_stPoint->xY < p_stMinPoint->xY) {
        ulOutCode |= 4;
    } else {
        if (p_stMaxPoint->xY < p_stPoint->xY) {
            ulOutCode |= 8;
        }
    }
    if (p_stPoint->xZ < p_stMinPoint->xZ) {
        return ulOutCode | 0x10;
    }
    if (p_stMaxPoint->xZ < p_stPoint->xZ) {
        ulOutCode |= 0x20;
    }
    return ulOutCode;
}

s32 inv_INT_fn_bPointInBBoxXY(MTH3D_tdstVector_* p_stPoint, MTH3D_tdstVector_* p_stMinPoint,
                              MTH3D_tdstVector_* p_stMaxPoint)
{
    if (p_stPoint->xX < p_stMinPoint->xX) {
        return 0;
    }
    if (p_stMaxPoint->xX < p_stPoint->xX) {
        return 0;
    }
    if (p_stPoint->xY < p_stMinPoint->xY) {
        return 0;
    }
    if (p_stMaxPoint->xY < p_stPoint->xY) {
        return 0;
    }
    return 1;
}

s32 inv_INT_fn_bPointInBBoxXYObj(MTH3D_tdstVector_* p_stPoint, MTH3D_tdstVector_* p_stMinPoint,
                                 MTH3D_tdstVector_* p_stMaxPoint)
{
    if (p_stPoint->xY < p_stMinPoint->xY) {
        return 0;
    }
    if (p_stMaxPoint->xY < p_stPoint->xY) {
        return 0;
    }
    if (p_stPoint->xZ < p_stMinPoint->xZ) {
        return 0;
    }
    if (p_stMaxPoint->xZ < p_stPoint->xZ) {
        return 0;
    }
    return 1;
}

s32 inv_INT_fn_bPointInBBoxXZ(MTH3D_tdstVector_* p_stPoint, MTH3D_tdstVector_* p_stMinPoint,
                              MTH3D_tdstVector_* p_stMaxPoint)
{
    if (p_stPoint->xX < p_stMinPoint->xX) {
        return 0;
    }
    if (p_stMaxPoint->xX < p_stPoint->xX) {
        return 0;
    }
    if (p_stPoint->xZ < p_stMinPoint->xZ) {
        return 0;
    }
    if (p_stMaxPoint->xZ < p_stPoint->xZ) {
        return 0;
    }
    return 1;
}

s32 INT_fn_bGetInclusionPointInBox(f32* p_stPoint, f32* p_stMinPoint, f32* p_stMaxPoint)
{
    if (p_stPoint[0] < p_stMinPoint[0]) {
        return 0;
    }
    if (p_stMaxPoint[0] < p_stPoint[0]) {
        return 0;
    }
    if (p_stPoint[1] < p_stMinPoint[1]) {
        return 0;
    }
    if (p_stMaxPoint[1] < p_stPoint[1]) {
        return 0;
    }
    if (p_stPoint[2] < p_stMinPoint[2]) {
        return 0;
    }
    if (p_stMaxPoint[2] < p_stPoint[2]) {
        return 0;
    }
    return 1;
}

s32 func_0020DCC8(RwV3d* vector0, RwV3d* vector1, RwV3d* vector2, RwV3d* factor0, RwV3d* factor1)
{
    typedef struct IntersectionVector4 {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    } IntersectionVector4;

    IntersectionVector4 products[4];
    s32 result;

    result = 0;
    products[0].x = factor0->x * vector0->x;
    products[0].y = factor0->y * vector0->y;
    products[0].z = factor0->z * vector0->z;
    products[1].x = factor1->x * vector0->x;
    products[1].y = factor1->y * vector0->y;
    products[1].z = factor1->z * vector0->z;
    products[2].x = factor0->x * vector2->x;
    products[2].y = factor0->y * vector2->y;
    products[2].z = factor0->z * vector2->z;
    products[3].x = factor1->x * vector1->x;
    products[3].y = factor1->y * vector1->y;
    products[3].z = factor1->z * vector1->z;

    if (products[0].x + products[0].y > products[2].x + products[2].y) {
        result = 1;
    }
    if (products[0].x + products[1].y > products[2].x + products[3].y) {
        result |= 2;
    }
    if (products[1].x + products[0].y > products[3].x + products[2].y) {
        result |= 4;
    }
    if (products[1].x + products[1].y > products[3].x + products[3].y) {
        result |= 8;
    }
    if (products[0].x + products[0].z > products[2].x + products[2].z) {
        result |= 0x10;
    }
    if (products[0].x + products[1].z > products[2].x + products[3].z) {
        result |= 0x20;
    }
    if (products[1].x + products[0].z > products[3].x + products[2].z) {
        result |= 0x40;
    }
    if (products[1].x + products[1].z > products[3].x + products[3].z) {
        result |= 0x80;
    }
    if (products[0].y + products[0].z > products[2].y + products[2].z) {
        result |= 0x100;
    }
    if (products[0].y + products[1].z > products[2].y + products[3].z) {
        result |= 0x200;
    }
    if (products[1].y + products[0].z > products[3].y + products[2].z) {
        result |= 0x400;
    }
    if (products[1].y + products[1].z > products[3].y + products[3].z) {
        result |= 0x800;
    }
    return result;
}

s32 inv_INT_fn_bClipPointAgainstBBoxFaces(MTH3D_tdstVector_* p_stVertex1,
                                          MTH3D_tdstVector_* p_stVertex2,
                                          MTH3D_tdstVector_* p_stVect12,
                                          MTH3D_tdstVector_* p_stMinPoint,
                                          MTH3D_tdstVector_* p_stMaxPoint, s32 flags,
                                          MTH3D_tdstVector_* p_stHit)
{
    f32 parameter;

    if (flags & 1) {
        parameter = (p_stMinPoint->xX - p_stVertex1->xX) / p_stVect12->xX;
        MTH3D_M_vSetXYZ_inv(p_stHit, p_stMinPoint->xX, p_stVect12->xY * parameter + p_stVertex1->xY,
                            p_stVect12->xZ * parameter + p_stVertex1->xZ);
        if (inv_INT_fn_bPointInBBoxXYObj(p_stHit, p_stMinPoint, p_stMaxPoint)) {
            return 1;
        }
    }
    if (flags & 2) {
        parameter = (p_stMaxPoint->xX - p_stVertex1->xX) / p_stVect12->xX;
        MTH3D_M_vSetXYZ_inv(p_stHit, p_stMaxPoint->xX, p_stVect12->xY * parameter + p_stVertex1->xY,
                            p_stVect12->xZ * parameter + p_stVertex1->xZ);
        if (inv_INT_fn_bPointInBBoxXYObj(p_stHit, p_stMinPoint, p_stMaxPoint)) {
            return 1;
        }
    }
    if (flags & 4) {
        parameter = (p_stMinPoint->xY - p_stVertex1->xY) / p_stVect12->xY;
        MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * parameter + p_stVertex1->xX, p_stMinPoint->xY,
                            p_stVect12->xZ * parameter + p_stVertex1->xZ);
        if (inv_INT_fn_bPointInBBoxXZ(p_stHit, p_stMinPoint, p_stMaxPoint)) {
            return 1;
        }
    }
    if (flags & 8) {
        parameter = (p_stMaxPoint->xY - p_stVertex1->xY) / p_stVect12->xY;
        MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * parameter + p_stVertex1->xX, p_stMaxPoint->xY,
                            p_stVect12->xZ * parameter + p_stVertex1->xZ);
        if (inv_INT_fn_bPointInBBoxXZ(p_stHit, p_stMinPoint, p_stMaxPoint)) {
            return 1;
        }
    }
    if (flags & 0x10) {
        parameter = (p_stMinPoint->xZ - p_stVertex1->xZ) / p_stVect12->xZ;
        MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * parameter + p_stVertex1->xX,
                            p_stVect12->xY * parameter + p_stVertex1->xY, p_stMinPoint->xZ);
        if (inv_INT_fn_bPointInBBoxXY(p_stHit, p_stMinPoint, p_stMaxPoint)) {
            return 1;
        }
    }
    if (flags & 0x20) {
        parameter = (p_stMaxPoint->xZ - p_stVertex1->xZ) / p_stVect12->xZ;
        MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * parameter + p_stVertex1->xX,
                            p_stVect12->xY * parameter + p_stVertex1->xY, p_stMaxPoint->xZ);
        if (inv_INT_fn_bPointInBBoxXY(p_stHit, p_stMinPoint, p_stMaxPoint)) {
            return 1;
        }
    }
    return 0;
}

s32 INT_fn_bIntersectSegmentWithBox(
    MTH3D_tdstVector_* p_stVertex1, MTH3D_tdstVector_* p_stVertex2, MTH3D_tdstVector_* p_stVect12,
    MTH3D_tdstVector_* p_stMinPoint, MTH3D_tdstVector_* p_stMaxPoint, f32* p_xT,
    MTH3D_tdstVector_* p_stHit, MTH3D_tdstVector_* p_stNormal)
{
    s32 flags;

    flags = INT_fn_ulGetPositionPointWithBox(p_stVertex1, p_stMinPoint, p_stMaxPoint);
    if (flags != 0) {
        if ((flags & INT_fn_ulGetPositionPointWithBox(p_stVertex2, p_stMinPoint, p_stMaxPoint)) ==
            0) {
            if (flags & 1) {
                *p_xT = (p_stMinPoint->xX - p_stVertex1->xX) / p_stVect12->xX;
                MTH3D_M_vSetXYZ_inv(p_stHit, p_stMinPoint->xX,
                                    p_stVect12->xY * *p_xT + p_stVertex1->xY,
                                    p_stVect12->xZ * *p_xT + p_stVertex1->xZ);
                if (inv_INT_fn_bPointInBBoxXYObj(p_stHit, p_stMinPoint, p_stMaxPoint)) {
                    *p_stNormal = inv_INT_stBoxNormalMinX;
                    return 1;
                }
            }
            if (flags & 2) {
                *p_xT = (p_stMaxPoint->xX - p_stVertex1->xX) / p_stVect12->xX;
                MTH3D_M_vSetXYZ_inv(p_stHit, p_stMaxPoint->xX,
                                    p_stVect12->xY * *p_xT + p_stVertex1->xY,
                                    p_stVect12->xZ * *p_xT + p_stVertex1->xZ);
                if (inv_INT_fn_bPointInBBoxXYObj(p_stHit, p_stMinPoint, p_stMaxPoint)) {
                    *p_stNormal = inv_INT_stBoxNormalMaxX;
                    return 1;
                }
            }
            if (flags & 4) {
                *p_xT = (p_stMinPoint->xY - p_stVertex1->xY) / p_stVect12->xY;
                MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * *p_xT + p_stVertex1->xX,
                                    p_stMinPoint->xY, p_stVect12->xZ * *p_xT + p_stVertex1->xZ);
                if (inv_INT_fn_bPointInBBoxXZ(p_stHit, p_stMinPoint, p_stMaxPoint)) {
                    *p_stNormal = inv_INT_stBoxNormalMinY;
                    return 1;
                }
            }
            if (flags & 8) {
                *p_xT = (p_stMaxPoint->xY - p_stVertex1->xY) / p_stVect12->xY;
                MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * *p_xT + p_stVertex1->xX,
                                    p_stMaxPoint->xY, p_stVect12->xZ * *p_xT + p_stVertex1->xZ);
                if (inv_INT_fn_bPointInBBoxXZ(p_stHit, p_stMinPoint, p_stMaxPoint)) {
                    *p_stNormal = inv_INT_stBoxNormalMaxY;
                    return 1;
                }
            }
            if (flags & 0x10) {
                *p_xT = (p_stMinPoint->xZ - p_stVertex1->xZ) / p_stVect12->xZ;
                MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * *p_xT + p_stVertex1->xX,
                                    p_stVect12->xY * *p_xT + p_stVertex1->xY, p_stMinPoint->xZ);
                if (inv_INT_fn_bPointInBBoxXY(p_stHit, p_stMinPoint, p_stMaxPoint)) {
                    *p_stNormal = inv_INT_stBoxNormalMinZ;
                    return 1;
                }
            }
            if (flags & 0x20) {
                *p_xT = (p_stMaxPoint->xZ - p_stVertex1->xZ) / p_stVect12->xZ;
                MTH3D_M_vSetXYZ_inv(p_stHit, p_stVect12->xX * *p_xT + p_stVertex1->xX,
                                    p_stVect12->xY * *p_xT + p_stVertex1->xY, p_stMaxPoint->xZ);
                if (inv_INT_fn_bPointInBBoxXY(p_stHit, p_stMinPoint, p_stMaxPoint)) {
                    *p_stNormal = inv_INT_stBoxNormalMaxZ;
                    return 1;
                }
            }
        }
    }
    return 0;
}

u8 ucPlaneCrossingdetect(f32* p_stVertex1, f32* p_stVect12, f32* p_stMinPoint, f32* p_stMaxPoint,
                         s8 cFace, f32 xT)
{
    s32 idx1;
    s32 idx2;
    f32 xTmp;
    s32 savedSecond;

    switch (cFace) {
    case 'X':
        idx1 = 1;
        idx2 = 2;
        break;
    case 'Y':
        idx1 = 0;
        idx2 = 2;
        break;
    case 'Z':
        idx1 = 0;
        idx2 = 1;
        break;
    default:
        idx1 = 0;
        idx2 = 0;
        break;
    }

    xTmp = p_stVect12[idx1] * xT + p_stVertex1[idx1];
    if (0.0f < p_stVect12[idx1]) {
        if (p_stMaxPoint[idx1] < xTmp) {
            goto return_zero;
        }
        if (!(p_stMinPoint[idx1] <= xTmp)) {
            goto return_two;
        }

        xTmp = p_stVect12[idx2] * xT + p_stVertex1[idx2];
        if (!(0.0f < p_stVect12[idx2])) {
            goto first_negative_secondary;
        }
        goto positive_secondary;
    } else {
        goto negative_primary;
    }

positive_secondary_accept:
    if (p_stMinPoint[idx2] <= xTmp) {
        return 1;
    }
    goto return_two;

first_negative_secondary:
    if (xTmp < p_stMinPoint[idx2]) {
        goto return_zero;
    }
    if (!(xTmp <= p_stMaxPoint[idx2])) {
        goto return_two;
    }
    return 1;

negative_primary:
    if (xTmp < p_stMinPoint[idx1]) {
        goto return_zero;
    }
    if (!(xTmp <= p_stMaxPoint[idx1])) {
        goto return_two;
    }

    savedSecond = idx2;
    xTmp = p_stVect12[idx2] * xT + p_stVertex1[idx2];
    if (!(0.0f < p_stVect12[idx2])) {
        goto second_negative_secondary;
    }

positive_secondary:
    if (!(p_stMaxPoint[idx2] < xTmp)) {
        goto positive_secondary_accept;
    }
return_zero:
    return 0;

second_negative_secondary:
    if (xTmp < p_stMinPoint[savedSecond]) {
        goto return_zero;
    }
    if (xTmp <= p_stMaxPoint[savedSecond]) {
        return 1;
    }
return_two:
    return 2;
}

s32 INT_fn_bDetectIntersectSegmentWithBox(MTH3D_tdstVector_* p_stVertex1,
                                           MTH3D_tdstVector_* p_stVect12,
                                           MTH3D_tdstVector_* p_stMinPoint,
                                           MTH3D_tdstVector_* p_stMaxPoint, f32* _p_xT)
{
    MTH3D_tdstVector_ stVertex2;
    f32 xT;
    f32* p_xT;
    s32 bWithxT;
    s32 ulVertex1Test;
    s32 ulVertex2Test;
    s32 ulExploredPlan;
    s32 bResult;

    if (_p_xT == 0) {
        bWithxT = 0;
        p_xT = &xT;
    } else {
        p_xT = _p_xT;
        bWithxT = 1;
    }

    ulVertex1Test = INT_fn_ulGetPositionPointWithBox(p_stVertex1, p_stMinPoint, p_stMaxPoint);
    if (ulVertex1Test == 0) {
        *p_xT = 0.0f;
        return 1;
    }

    inv_MTH3D_fn_vAddXYZ(&stVertex2, p_stVertex1, p_stVect12);
    ulVertex2Test = INT_fn_ulGetPositionPointWithBox(&stVertex2, p_stMinPoint, p_stMaxPoint);

    if (ulVertex2Test == 0) {
        *p_xT = 1.0f;
        return 1;
    }

    if (ulVertex1Test & ulVertex2Test)
        return 0;

    ulExploredPlan = ulVertex1Test | ulVertex2Test;

    if ((ulExploredPlan & 0x0F) == 0) {
        if (!bWithxT)
            return 1;
        if (ulVertex1Test & 0x10)
            *p_xT = p_stMinPoint->xZ - p_stVertex1->xZ;
        else
            *p_xT = p_stMaxPoint->xZ - p_stVertex1->xZ;
        *p_xT /= p_stVect12->xZ;
        return 1;
    }

    if ((ulExploredPlan & 0x33) == 0) {
        if (!bWithxT)
            return 1;
        if (ulVertex1Test & 4)
            *p_xT = p_stMinPoint->xY - p_stVertex1->xY;
        else
            *p_xT = p_stMaxPoint->xY - p_stVertex1->xY;
        *p_xT /= p_stVect12->xY;
        return 1;
    }

    if ((ulExploredPlan & 0x3C) == 0) {
        if (!bWithxT)
            return 1;
        if (ulVertex1Test & 1)
            *p_xT = p_stMinPoint->xX - p_stVertex1->xX;
        else
            *p_xT = p_stMaxPoint->xX - p_stVertex1->xX;
        *p_xT /= p_stVect12->xX;
        return 1;
    }

    if (ulVertex1Test & 1) {
        *p_xT = p_stMinPoint->xX - p_stVertex1->xX;
        *p_xT /= p_stVect12->xX;
        bResult = ucPlaneCrossingdetect(&p_stVertex1->xX, &p_stVect12->xX, &p_stMinPoint->xX,
                                        &p_stMaxPoint->xX, 'X', *p_xT);
        if ((u32) bResult < 2)
            return bResult;
    }
    if (ulVertex1Test & 2) {
        *p_xT = p_stMaxPoint->xX - p_stVertex1->xX;
        *p_xT /= p_stVect12->xX;
        bResult = ucPlaneCrossingdetect(&p_stVertex1->xX, &p_stVect12->xX, &p_stMinPoint->xX,
                                        &p_stMaxPoint->xX, 'X', *p_xT);
        if ((u32) bResult < 2)
            return bResult;
    }
    if (ulVertex1Test & 4) {
        *p_xT = p_stMinPoint->xY - p_stVertex1->xY;
        *p_xT /= p_stVect12->xY;
        bResult = ucPlaneCrossingdetect(&p_stVertex1->xX, &p_stVect12->xX, &p_stMinPoint->xX,
                                        &p_stMaxPoint->xX, 'Y', *p_xT);
        if ((u32) bResult < 2)
            return bResult;
    }
    if (ulVertex1Test & 8) {
        *p_xT = p_stMaxPoint->xY - p_stVertex1->xY;
        *p_xT /= p_stVect12->xY;
        bResult = ucPlaneCrossingdetect(&p_stVertex1->xX, &p_stVect12->xX, &p_stMinPoint->xX,
                                        &p_stMaxPoint->xX, 'Y', *p_xT);
        if ((u32) bResult < 2)
            return bResult;
    }
    if (ulVertex1Test & 0x10) {
        *p_xT = p_stMinPoint->xZ - p_stVertex1->xZ;
        *p_xT /= p_stVect12->xZ;
        bResult = ucPlaneCrossingdetect(&p_stVertex1->xX, &p_stVect12->xX, &p_stMinPoint->xX,
                                        &p_stMaxPoint->xX, 'Z', *p_xT);
        if ((u32) bResult < 2)
            return bResult;
    }
    if (ulVertex1Test & 0x20) {
        *p_xT = p_stMaxPoint->xZ - p_stVertex1->xZ;
        *p_xT /= p_stVect12->xZ;
        bResult = ucPlaneCrossingdetect(&p_stVertex1->xX, &p_stVect12->xX, &p_stMinPoint->xX,
                                        &p_stMaxPoint->xX, 'Z', *p_xT);
        if ((u32) bResult < 2)
            return bResult;
    }
    return 0;
}

s32 INT_fn_bGetPointInTriangle(MTH3D_tdstVector_* p_stPoint,
                                         MTH3D_tdstVector_* p_stVertex0,
                                         MTH3D_tdstVector_* p_stVertex1,
                                         MTH3D_tdstVector_* p_stVertex2,
                                         MTH3D_tdstVector_* p_stNormal)
{
    MTH4D_tdstVector_ e1;
    MTH4D_tdstVector_ e2;
    MTH4D_tdstVector_ e3;
    f32 cross;

    MTH3D_M_vSubXYZ_inv(&e1, p_stVertex0, p_stPoint);
    MTH3D_M_vSubXYZ_inv(&e2, p_stVertex1, p_stPoint);
    MTH3D_M_vSubXYZ_inv(&e3, p_stVertex2, p_stPoint);

    if (0.57735026f <= inv_MTH_fn_xAbsolute(p_stNormal->xZ)) {
        cross = inv_MTH_fn_xMulSub(e1.xX, e2.xY, e1.xY, e2.xX);
        cross = cross * p_stNormal->xZ;
        if (0.0f < cross) {
            cross = inv_MTH_fn_xMulSub(e2.xX, e3.xY, e2.xY, e3.xX);
            cross = cross * p_stNormal->xZ;
            if (0.0f < cross) {
                cross = inv_MTH_fn_xMulSub(e3.xX, e1.xY, e3.xY, e1.xX);
                cross = cross * p_stNormal->xZ;
                return 0.0f < cross;
            }
        }
    } else if (0.57735026f <= inv_MTH_fn_xAbsolute(p_stNormal->xX)) {
        cross = inv_MTH_fn_xMulSub(e1.xY, e2.xZ, e1.xZ, e2.xY);
        cross = cross * p_stNormal->xX;
        if (0.0f < cross) {
            cross = inv_MTH_fn_xMulSub(e2.xY, e3.xZ, e2.xZ, e3.xY);
            cross = cross * p_stNormal->xX;
            if (0.0f < cross) {
                cross = inv_MTH_fn_xMulSub(e3.xY, e1.xZ, e3.xZ, e1.xY);
                cross = cross * p_stNormal->xX;
                return 0.0f < cross;
            }
        }
    } else {
        cross = inv_MTH_fn_xMulSub(e1.xZ, e2.xX, e1.xX, e2.xZ);
        cross = cross * p_stNormal->xY;
        if (0.0f < cross) {
            cross = inv_MTH_fn_xMulSub(e2.xZ, e3.xX, e2.xX, e3.xZ);
            cross = cross * p_stNormal->xY;
            if (0.0f < cross) {
                cross = inv_MTH_fn_xMulSub(e3.xZ, e1.xX, e3.xX, e1.xZ);
                cross = cross * p_stNormal->xY;
                return 0.0f < cross;
            }
        }
    }
    return 0;
}

s32 INT_fn_bFastRejectIntersectSegmentWithTriangle(MTH3D_tdstVector_* p_stVertexA, MTH3D_tdstVector_* p_stVectAB,
                  MTH3D_tdstVector_* p_stVertex1, MTH3D_tdstVector_* p_stVertex2,
                  MTH3D_tdstVector_* p_stVertex3)
{
    MTH3D_tdstVector_ e;
    f32 x, y;
    s32 px, py, pz, t;

    inv_MTH3D_fn_vAddXYZ(&e, p_stVertexA, p_stVectAB);
    px = 0.0f < p_stVectAB->xX;
    t = px;
    if (t)
        y = p_stVertexA->xX;
    else
        y = e.xX;
    if (p_stVertex1->xX > p_stVertex2->xX && p_stVertex1->xX > p_stVertex3->xX)
        x = p_stVertex1->xX;
    else if (p_stVertex2->xX > p_stVertex3->xX)
        x = p_stVertex2->xX;
    else
        x = p_stVertex3->xX;
    if (x < y)
        return 0;
    if (px)
        x = e.xX;
    else
        x = p_stVertexA->xX;
    if (p_stVertex1->xX < p_stVertex2->xX && p_stVertex1->xX < p_stVertex3->xX)
        y = p_stVertex1->xX;
    else if (p_stVertex2->xX < p_stVertex3->xX)
        y = p_stVertex2->xX;
    else
        y = p_stVertex3->xX;
    if (x < y)
        return 0;
    py = 0.0f < p_stVectAB->xY;
    t = py;
    if (t)
        y = p_stVertexA->xY;
    else
        y = e.xY;
    if (p_stVertex1->xY > p_stVertex2->xY && p_stVertex1->xY > p_stVertex3->xY)
        x = p_stVertex1->xY;
    else if (p_stVertex2->xY > p_stVertex3->xY)
        x = p_stVertex2->xY;
    else
        x = p_stVertex3->xY;
    if (x < y)
        return 0;
    if (py)
        x = e.xY;
    else
        x = p_stVertexA->xY;
    if (p_stVertex1->xY < p_stVertex2->xY && p_stVertex1->xY < p_stVertex3->xY)
        y = p_stVertex1->xY;
    else if (p_stVertex2->xY < p_stVertex3->xY)
        y = p_stVertex2->xY;
    else
        y = p_stVertex3->xY;
    if (x < y)
        return 0;
    pz = 0.0f < p_stVectAB->xZ;
    t = pz;
    if (t)
        y = p_stVertexA->xZ;
    else
        y = e.xZ;
    if (p_stVertex1->xZ > p_stVertex2->xZ && p_stVertex1->xZ > p_stVertex3->xZ)
        x = p_stVertex1->xZ;
    else if (p_stVertex2->xZ > p_stVertex3->xZ)
        x = p_stVertex2->xZ;
    else
        x = p_stVertex3->xZ;
    if (x < y)
        return 0;
    if (pz)
        x = e.xZ;
    else
        x = p_stVertexA->xZ;
    if (p_stVertex1->xZ < p_stVertex2->xZ && p_stVertex1->xZ < p_stVertex3->xZ)
        y = p_stVertex1->xZ;
    else if (p_stVertex2->xZ < p_stVertex3->xZ)
        y = p_stVertex2->xZ;
    else
        y = p_stVertex3->xZ;
    if (x < y)
        return 0;
    return 1;
}

static inline void inv_COL_fn_vStageVector(ScratchVector* scratch, MTH3D_tdstVector_* vector)
{
    scratch->xyz = *vector;
    scratch->w = 0;
}

s32 inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
    MTH3D_tdstVector_* origin, MTH3D_tdstVector_* direction, MTH3D_tdstVector_* a,
    MTH3D_tdstVector_* b, MTH3D_tdstVector_* c, f32* parameter, MTH3D_tdstVector_* point)
{
    ScratchVector* scratch;
    u32 flags;

    scratch = SCRATCH_VECTOR;
    inv_COL_fn_vStageVector(scratch, origin);
    VU0_LQC2(1, scratch, 0);
    inv_COL_fn_vStageVector(scratch, direction);
    VU0_LQC2(2, scratch, 0);
    inv_COL_fn_vStageVector(scratch, a);
    VU0_LQC2(3, scratch, 0);
    inv_COL_fn_vStageVector(scratch, b);
    VU0_LQC2(4, scratch, 0);
    inv_COL_fn_vStageVector(scratch, c);
    VU0_LQC2(5, scratch, 0);
    inv_PS2_fn_vCallVu0Microprogram((u32) (D_F30 - D_BASE) >> 3);
    if ((u8) (u32) origin & 0x100) {
        return flags + (u32) parameter + (u32) point;
    }
    while (inv_PS2_fn_bIsVu0Busy() != 0) {
    }
    VU0_CFC2_NI(flags, 13);
    if (flags == 0)
        return 0;
    VU0_SQC2(14, scratch, 0);
    *point = scratch->xyz;
    VU0_SQC2(15, scratch, 0);
    *parameter = scratch->xyz.xX;
    return 1;
}

s32 INT_fn_bDetectIntersectSegmentWithTriangle(MTH3D_tdstVector_* p_stVertexA,
                                                MTH3D_tdstVector_* p_stVectAB,
                                                MTH3D_tdstVector_* p_stVertex1,
                                                MTH3D_tdstVector_* p_stVertex2,
                                                MTH3D_tdstVector_* p_stVertex3)
{
    ScratchVector* scratch;
    u32 flags;

    scratch = SCRATCH_VECTOR;
    inv_COL_fn_vStageVector(scratch, p_stVertexA);
    VU0_LQC2(1, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVectAB);
    VU0_LQC2(2, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVertex1);
    VU0_LQC2(3, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVertex2);
    VU0_LQC2(4, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVertex3);
    VU0_LQC2(5, scratch, 0);
    inv_PS2_fn_vCallVu0Microprogram((u32) (D_F30 - D_BASE) >> 3);
    if ((u8) (u32) p_stVertexA & 0x100) {
        return flags;
    }
    while (inv_PS2_fn_bIsVu0Busy() != 0) {
    }
    VU0_CFC2_NI(flags, 13);
    return flags != 0;
}

s32 inv_INT_fn_bDetectIntersectSegmentWithTriangleAlt(MTH3D_tdstVector_* p_stVertexA,
                                                      MTH3D_tdstVector_* p_stVectAB,
                                                      MTH3D_tdstVector_* p_stVertex1,
                                                      MTH3D_tdstVector_* p_stVertex2,
                                                      MTH3D_tdstVector_* p_stVertex3)
{
    ScratchVector* scratch;
    u32 flags;

    scratch = SCRATCH_VECTOR;
    inv_COL_fn_vStageVector(scratch, p_stVertexA);
    VU0_LQC2(1, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVectAB);
    VU0_LQC2(2, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVertex1);
    VU0_LQC2(3, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVertex2);
    VU0_LQC2(4, scratch, 0);
    inv_COL_fn_vStageVector(scratch, p_stVertex3);
    VU0_LQC2(5, scratch, 0);
    inv_PS2_fn_vCallVu0Microprogram((u32) (D_3A8 - dword_251ED0) >> 3);
    if ((u8) (u32) p_stVertexA & 0x100) {
        return flags;
    }
    while (inv_PS2_fn_bIsVu0Busy() != 0) {
    }
    VU0_CFC2_NI(flags, 13);
    return flags != 0;
}

s32 INT_fn_bGetInclusionPointInSphere(MTH3D_tdstVector_* p_stSphereCenter,
                                    MTH3D_tdstVector_* p_stPoint, f32 xSphereRadius)
{
    f32 dx = p_stSphereCenter->xX - p_stPoint->xX;
    f32 dy = p_stSphereCenter->xY - p_stPoint->xY;
    f32 dz;
    s32 inside;

    xSphereRadius = MTH_fn_xSqrOpt1(xSphereRadius);
    dz = p_stSphereCenter->xZ - p_stPoint->xZ;
    inside = 1;
    if (!(MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz) <= xSphereRadius)) {
        inside = 0;
    }
    return inside;
}

s32 inv_INT_fn_bSegmentIntersectsBBox(MTH3D_tdstVector_* start, MTH3D_tdstVector_* direction, MTH3D_tdstVector_* center, f32 extent)
{
    f32 relativeStart[4];
    f32 relativeEnd[4];
    f32 negativeExtent;

    negativeExtent = -extent;
    relativeStart[0] = start->xX - center->xX;
    relativeStart[1] = start->xY - center->xY;
    relativeStart[2] = start->xZ - center->xZ;
    relativeEnd[0] = relativeStart[0] + direction->xX;
    relativeEnd[1] = relativeStart[1] + direction->xY;
    relativeEnd[2] = relativeStart[2] + direction->xZ;

    if (direction->xX > 0.0f) {
        if (extent < relativeStart[0]) {
            return 0;
        }
        if (relativeEnd[0] < negativeExtent) {
            return 0;
        }
    } else {
        if (relativeStart[0] < negativeExtent) {
            return 0;
        }
        if (extent < relativeEnd[0]) {
            return 0;
        }
    }

    if (direction->xY > 0.0f) {
        if (extent < relativeStart[1]) {
            return 0;
        }
        if (relativeEnd[1] < negativeExtent) {
            return 0;
        }
    } else {
        if (relativeStart[1] < negativeExtent) {
            return 0;
        }
        if (extent < relativeEnd[1]) {
            return 0;
        }
    }

    if (direction->xZ > 0.0f) {
        if (extent < relativeStart[2]) {
            return 0;
        }
        if (relativeEnd[2] < negativeExtent) {
            return 0;
        }
    } else {
        if (relativeStart[2] < negativeExtent) {
            return 0;
        }
        if (extent < relativeEnd[2]) {
            return 0;
        }
    }

    return 1;
}

s32 inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(MTH3D_tdstVector_* origin, MTH3D_tdstVector_* direction, MTH3D_tdstVector_* center,
                                                         f32* outT, MTH3D_tdstVector_* hit, MTH3D_tdstVector_* normal,
                                                         f32 radius)
{
    f32 relative[3];
    f32 roots[2];
    f32 scale;
    f32 a;
    f32 b;
    f32 c;
    f32 t;
    s32 count;

    if (inv_INT_fn_bSegmentIntersectsBBox(origin, direction, center, radius) != 0) {
        relative[0] = origin->xX - center->xX;
        relative[1] = origin->xY - center->xY;
        relative[2] = origin->xZ - center->xZ;
        scale = radius * radius;
        c = MTH_fn_xSqrAddSqrAddSqrOpt2(relative[0], relative[1], relative[2]);
        b = inv_MTH_fn_xMulAdd3Grouped(direction->xX, direction->xY, direction->xZ, relative[0],
                                       relative[1], relative[2]);
        c = c - scale;
        b = b + b;
        a = MTH_fn_xSqrAddSqrAddSqrOpt2(direction->xX, direction->xY, direction->xZ);
        count = INT_fn_xTrinomeResolution(&roots[0], &roots[1], a, b, c);
        switch (count) {
        case 2:
            t = inv_MTH_fn_xMinimum(roots[0], roots[1]);
            if (0.0f <= t && t <= 1.0f) {
                scale = 1.0f / radius;
                *outT = t;
                hit->xX = t * direction->xX + origin->xX;
                hit->xY = t * direction->xY + origin->xY;
                hit->xZ = t * direction->xZ + origin->xZ;
                normal->xX = hit->xX - center->xX;
                normal->xY = hit->xY - center->xY;
                normal->xZ = hit->xZ - center->xZ;
                normal->xX = scale * normal->xX;
                normal->xY = scale * normal->xY;
                normal->xZ = scale * normal->xZ;
                return 1;
            }
            return 0;
        case 1:
            if (0.0f <= roots[0] && roots[0] <= 1.0f) {
                *outT = roots[0];
                scale = 1.0f / radius;
                hit->xX = roots[0] * direction->xX + origin->xX;
                hit->xY = roots[0] * direction->xY + origin->xY;
                hit->xZ = roots[0] * direction->xZ + origin->xZ;
                normal->xX = hit->xX - center->xX;
                normal->xY = hit->xY - center->xY;
                normal->xZ = hit->xZ - center->xZ;
                normal->xX = scale * normal->xX;
                normal->xY = scale * normal->xY;
                normal->xZ = scale * normal->xZ;
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

u8 INT_fn_bDetectIntersectSegmentWithSphere(MTH3D_tdstVector_* p_stVertexA, MTH3D_tdstVector_* p_stVectAB, MTH3D_tdstVector_* p_stSphereCenter, f32 xSphereRadius)
{
    MTH3D_tdstVector_ relativeStart;
    MTH3D_tdstVector_ relativeEnd;
    f32 radiusSquared;
    f32 negativeRadius;
    f32 endDistanceSquared;
    f32 startDistanceSquared;
    f32 dot;
    f32 directionLengthSquared;
    f32 crossLengthSquared;
    f32 crossComponent;

    radiusSquared = MTH_fn_xSqrOpt1(xSphereRadius);
    negativeRadius = -xSphereRadius;

    inv_MTH3D_fn_vSubXYZ(&relativeStart, p_stVertexA, p_stSphereCenter);
    inv_MTH3D_fn_vAddXYZ(&relativeEnd, &relativeStart, p_stVectAB);

    endDistanceSquared =
        MTH_fn_xSqrAddSqrAddSqrOpt2(relativeEnd.xX, relativeEnd.xY, relativeEnd.xZ);
    if (endDistanceSquared <= radiusSquared) {
        return 1;
    }
    startDistanceSquared =
        MTH_fn_xSqrAddSqrAddSqrOpt2(relativeStart.xX, relativeStart.xY, relativeStart.xZ);
    if (startDistanceSquared <= radiusSquared) {
        return 1;
    }

    if (p_stVectAB->xX > 0.0f) {
        if (xSphereRadius < relativeStart.xX) {
            return 0;
        }
        if (relativeEnd.xX < negativeRadius) {
            return 0;
        }
    } else {
        if (relativeStart.xX < negativeRadius) {
            return 0;
        }
        if (xSphereRadius < relativeEnd.xX) {
            return 0;
        }
    }
    if (p_stVectAB->xY > 0.0f) {
        if (xSphereRadius < relativeStart.xY) {
            return 0;
        }
        if (relativeEnd.xY < negativeRadius) {
            return 0;
        }
    } else {
        if (relativeStart.xY < negativeRadius) {
            return 0;
        }
        if (xSphereRadius < relativeEnd.xY) {
            return 0;
        }
    }
    if (p_stVectAB->xZ > 0.0f) {
        if (xSphereRadius < relativeStart.xZ) {
            return 0;
        }
        if (relativeEnd.xZ < negativeRadius) {
            return 0;
        }
    } else {
        if (relativeStart.xZ < negativeRadius) {
            return 0;
        }
        if (xSphereRadius < relativeEnd.xZ) {
            return 0;
        }
    }

    dot = inv_MTH_fn_xMulAdd3Grouped(relativeStart.xX, relativeStart.xY, relativeStart.xZ,
                           p_stVectAB->xX, p_stVectAB->xY, p_stVectAB->xZ);
    if (0.0f < dot) {
        return 0;
    }
    directionLengthSquared =
        MTH_fn_xSqrAddSqrAddSqrOpt2(p_stVectAB->xX, p_stVectAB->xY, p_stVectAB->xZ);
    if (directionLengthSquared < -dot) {
        return 0;
    }
    radiusSquared = radiusSquared * directionLengthSquared;
    crossComponent = relativeStart.xX * p_stVectAB->xY;
    crossLengthSquared = crossComponent - relativeStart.xY * p_stVectAB->xX;
    crossLengthSquared = MTH_fn_xSqrOpt1(crossLengthSquared);
    if (radiusSquared <= crossLengthSquared) {
        return 0;
    }
    crossComponent = relativeStart.xX * p_stVectAB->xZ - relativeStart.xZ * p_stVectAB->xX;
    crossLengthSquared += MTH_fn_xSqrOpt1(crossComponent);
    if (radiusSquared <= crossLengthSquared) {
        return 0;
    }
    crossComponent = relativeStart.xY * p_stVectAB->xZ - relativeStart.xZ * p_stVectAB->xY;
    crossLengthSquared += MTH_fn_xSqrOpt1(crossComponent);
    if (radiusSquared <= crossLengthSquared) {
        return 0;
    }
    return 1;
}
