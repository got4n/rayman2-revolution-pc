#include "common.h"

#include "GEO/GEO.h"
#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "MTH/MTH.h"

#ifdef PC_PORT

#include "PC/PC.h"

static f32 PC_DrawDistanceScale(f32 distance)
{
    s32 percent = pc_draw_distance();

    return percent == 100 ? distance : distance * 100.0f / (f32) percent;
}
#endif
#include "GAM/GAM.h"

extern MTH3D_tdstVector_ gs_stGlobalNormPlaneLeft;
extern MTH3D_tdstVector_ gs_stGlobalNormPlaneRight;
extern MTH3D_tdstVector_ gs_stGlobalNormPlaneUp;
extern MTH3D_tdstVector_ gs_stGlobalNormPlaneDown;
extern MTH3D_tdstVector_ gs_stGlobalNormPlaneNear;
extern MTH3D_tdstVector_ gs_stCameraPosInGlobal;
extern f32 GEO_g_xOoZFarTransparencyZone;

f32 GEO_xGetZFar(void)
{
    return GEO_g_xZFar;
}

void GEO_vSetZFar(f32 _xZ)
{
    GEO_g_xZFar = _xZ;
}

f32 GEO_xGetZFarTransparencyZone(void)
{
    return GEO_g_xZFarTransparencyZone;
}

void GEO_vSetZFarTransparencyZone(f32 _xZ)
{
    GEO_g_xZFarTransparencyZone = _xZ;
    if (_xZ != 0.0f) {
        GEO_g_xOoZFarTransparencyZone = 1.0f / _xZ;
    }
}

f32 GEO_xGetZFarTransparencyLevel(void)
{
    return GEO_g_xZFarTransparencyLevel;
}

s32 GEO_lMainCullingSphere(GLD_tdstViewportAttributes_* viewport, MTH3D_tdstVector_* p_stShereCenter, f32 xRadius,
                            POS_tdstMatrix_* matrix, POS_tdstMatrix_* objectMatrix, s32 _lCullingResult)
{
    MTH3D_tdstVector_ center;
    GLI_tdstCamera_* camera;
    f32 lower;
    f32 dot;
    f32 depth;

    if (inv_POS_fn_bIsScaleMatrix(matrix)) {
        xRadius *= POS_fn_xGetMaxScale(matrix);
    }
    camera = *(GLI_tdstCamera_**) viewport->p_vSpecificToXD;
    POS_fn_vMulMatrixVertex(&center, matrix, p_stShereCenter);
    _lCullingResult |= 0x40;

    if ((_lCullingResult & 0x10) == 0) {
        depth = center.xZ;
        if (depth + xRadius < camera->xNear) {
            return 0;
        }
        depth -= xRadius;
        if (camera->xNear < depth) {
            _lCullingResult |= 0x10;
        }
    }

    if ((_lCullingResult & 0x20) == 0) {
        if (GEO_g_xZFar != 0.0f) {
            lower = center.xZ - xRadius;
#ifdef PC_PORT
            lower = PC_DrawDistanceScale(lower);
#endif
            if (GEO_g_xZFar < lower) {
                GEO_g_xZFarTransparencyLevel = 0.0f;
                return 0;
            }
            if (GEO_g_xZFar - GEO_g_xZFarTransparencyZone < lower) {
                GEO_g_xZFarTransparencyLevel =
                    1.0f - ((lower - GEO_g_xZFar) + GEO_g_xZFarTransparencyZone) * GEO_g_xOoZFarTransparencyZone;
            } else {
                _lCullingResult |= 0x20;
                GEO_g_xZFarTransparencyLevel = 1.0f;
            }
        } else {
            _lCullingResult |= 0x20;
            GEO_g_xZFarTransparencyLevel = 1.0f;
        }
    } else {
        GEO_g_xZFarTransparencyLevel = 1.0f;
    }

    if ((_lCullingResult & 1) == 0) {
        dot = inv_MTH_fn_xMulAdd3(center.xX, camera->clipPlanes[1].xX, center.xY,
                       camera->clipPlanes[1].xY, center.xZ, camera->clipPlanes[1].xZ);
        if (xRadius + camera->clipPlanes[0].xW < dot) {
            return 0;
        }
        if (dot < camera->clipPlanes[0].xW - xRadius) {
            _lCullingResult |= 1;
        }
    }

    if ((_lCullingResult & 2) == 0) {
        dot = inv_MTH_fn_xMulAdd3(center.xX, camera->clipPlanes[0].xX, center.xY,
                       camera->clipPlanes[0].xY, center.xZ, camera->clipPlanes[0].xZ);
        if (xRadius + camera->clipPlanes[1].xW < dot) {
            return 0;
        }
        if (dot < camera->clipPlanes[1].xW - xRadius) {
            _lCullingResult |= 2;
        }
    }

    if ((_lCullingResult & 4) == 0) {
        dot = inv_MTH_fn_xMulAdd3(center.xX, camera->clipPlanes[2].xX, center.xY,
                       camera->clipPlanes[2].xY, center.xZ, camera->clipPlanes[2].xZ);
        if (xRadius + camera->clipPlanes[2].xW < dot) {
            return 0;
        }
        if (dot < camera->clipPlanes[2].xW - xRadius) {
            _lCullingResult |= 4;
        }
    }

    if ((_lCullingResult & 8) == 0) {
        dot = inv_MTH_fn_xMulAdd3(center.xX, camera->clipPlanes[3].xX, center.xY,
                       camera->clipPlanes[3].xY, center.xZ, camera->clipPlanes[3].xZ);
        if (xRadius + camera->clipPlanes[3].xW < dot) {
            return 0;
        }
        if (dot < camera->clipPlanes[3].xW - xRadius) {
            _lCullingResult |= 8;
        }
    }

    return _lCullingResult;
}

void GEO_fn_vComputeCameraParametersForCulling(GLD_tdstViewportAttributes_* p_stVpt, f32 _xZFar)
{
    POS_tdstMatrix_ inverse;
    GLI_tdstCamera_* camera = *(GLI_tdstCamera_**) p_stVpt->p_vSpecificToXD;

    POS_fn_vInvertIsoMatrix(&inverse, &camera->stMatrix);

    MTH3D_M_vSetXYZ_inv(&gs_stCameraPosInGlobal, 0.0f, 0.0f, -1.0f);
    POS_fn_vApplyMatrixToVector(&gs_stGlobalNormPlaneNear, &inverse, &gs_stCameraPosInGlobal);

    gs_stCameraPosInGlobal = *inv_POS_fn_p_stPosMatrixTranslation(&inverse);
    POS_fn_vApplyMatrixToVector(&gs_stGlobalNormPlaneLeft, &inverse,
                                (MTH3D_tdstVector_*) &camera->clipPlanes[1]);
    POS_fn_vApplyMatrixToVector(&gs_stGlobalNormPlaneRight, &inverse,
                                (MTH3D_tdstVector_*) &camera->clipPlanes[0]);
    POS_fn_vApplyMatrixToVector(&gs_stGlobalNormPlaneUp, &inverse,
                                (MTH3D_tdstVector_*) &camera->clipPlanes[2]);
    POS_fn_vApplyMatrixToVector(&gs_stGlobalNormPlaneDown, &inverse,
                                (MTH3D_tdstVector_*) &camera->clipPlanes[3]);

    GEO_g_xZFar = _xZFar;
}

s32 GEO_lCullingSphere(GLD_tdstViewportAttributes_* viewport, MTH3D_tdstVector_* p_stSphereCenter, f32 xRadius,
                        POS_tdstMatrix_* objectMatrix, s32 _lCullingResult)
{
    POS_tdstMatrix_ matrix;

    _POS_fn_vMulMatrixMatrix(&matrix, &(*(GLI_tdstCamera_**) viewport->p_vSpecificToXD)->stMatrix, objectMatrix);
    return GEO_lMainCullingSphere(viewport, p_stSphereCenter, xRadius, &matrix, objectMatrix, _lCullingResult);
}

s32 GEO_lCullingBox(GLD_tdstViewportAttributes_* p_stVpt, MTH3D_tdstVector_* p_stMin,
                     MTH3D_tdstVector_* p_stMax, s32 _lCullingResult)
{
    MTH3D_tdstVector_ localMin;
    MTH3D_tdstVector_ localMax;
    f32 distance;
    f32 minX;
    f32 minY;
    f32 minZ;
    f32 maxX;
    f32 maxY;
    f32 maxZ;
    GLI_tdstCamera_* camera;
    u8 computed;

    computed = 0;
    distance = 0.0f;
    camera = *(GLI_tdstCamera_**) p_stVpt->p_vSpecificToXD;

    inv_MTH3D_fn_vSubXYZ(&localMin, p_stMin, &gs_stCameraPosInGlobal);
    inv_MTH3D_fn_vSubXYZ(&localMax, p_stMax, &gs_stCameraPosInGlobal);

    _lCullingResult |= 0x40;

    if (!(_lCullingResult & 0x10)) {
        minX = localMin.xX * gs_stGlobalNormPlaneNear.xX;
        minY = localMin.xY * gs_stGlobalNormPlaneNear.xY;
        minZ = localMin.xZ * gs_stGlobalNormPlaneNear.xZ;
        maxX = localMax.xX * gs_stGlobalNormPlaneNear.xX;
        maxY = localMax.xY * gs_stGlobalNormPlaneNear.xY;
        maxZ = localMax.xZ * gs_stGlobalNormPlaneNear.xZ;
        distance = ((minX < maxX) ? minX : maxX) + ((minY < maxY) ? minY : maxY) +
                   ((minZ < maxZ) ? minZ : maxZ);
        if (distance > -camera->xNear) {
            return 0;
        }
        distance = ((minX > maxX) ? minX : maxX) + ((minY > maxY) ? minY : maxY) +
                   ((minZ > maxZ) ? minZ : maxZ);
        if (distance < -camera->xNear) {
            _lCullingResult |= 0x10;
        }
        computed = 1;
    }

    if (!(_lCullingResult & 0x20)) {
        if (GEO_g_xZFar != 0.0f) {
            if (!computed) {
                minX = localMin.xX * gs_stGlobalNormPlaneNear.xX;
                minY = localMin.xY * gs_stGlobalNormPlaneNear.xY;
                minZ = localMin.xZ * gs_stGlobalNormPlaneNear.xZ;
                maxX = localMax.xX * gs_stGlobalNormPlaneNear.xX;
                maxY = localMax.xY * gs_stGlobalNormPlaneNear.xY;
                maxZ = localMax.xZ * gs_stGlobalNormPlaneNear.xZ;
                distance = -(((minX > maxX) ? minX : maxX) + ((minY > maxY) ? minY : maxY) +
                             ((minZ > maxZ) ? minZ : maxZ));
            } else {
                distance = -distance;
            }
#ifdef PC_PORT
            distance = PC_DrawDistanceScale(distance);
#endif
            if (distance > GEO_g_xZFar)
                return 0;
            GEO_g_xZFarTransparencyLevel = GEO_g_xZFar - GEO_g_xZFarTransparencyZone;
            if (distance > GEO_g_xZFarTransparencyLevel)
                GEO_g_xZFarTransparencyLevel = 1.0f - ((distance - GEO_g_xZFarTransparencyLevel) *
                                                       GEO_g_xOoZFarTransparencyZone);
            else {
                GEO_g_xZFarTransparencyLevel = 1.0f;
                _lCullingResult |= 0x20;
            }
        } else {
            GEO_g_xZFarTransparencyLevel = 1.0f;
            _lCullingResult |= 0x20;
        }
    } else {
        GEO_g_xZFarTransparencyLevel = 1.0f;
    }

    if (!(_lCullingResult & 1)) {
        minX = localMin.xX * gs_stGlobalNormPlaneLeft.xX;
        minY = localMin.xY * gs_stGlobalNormPlaneLeft.xY;
        minZ = localMin.xZ * gs_stGlobalNormPlaneLeft.xZ;
        maxX = localMax.xX * gs_stGlobalNormPlaneLeft.xX;
        maxY = localMax.xY * gs_stGlobalNormPlaneLeft.xY;
        maxZ = localMax.xZ * gs_stGlobalNormPlaneLeft.xZ;
        distance = ((minX < maxX) ? minX : maxX) + ((minY < maxY) ? minY : maxY) +
                   ((minZ < maxZ) ? minZ : maxZ);
        if (distance > camera->clipPlanes[0].xW) {
            return 0;
        }
        distance = ((minX > maxX) ? minX : maxX) + ((minY > maxY) ? minY : maxY) +
                   ((minZ > maxZ) ? minZ : maxZ);
        if (distance < camera->clipPlanes[0].xW) {
            _lCullingResult |= 1;
        }
    }

    if (!(_lCullingResult & 2)) {
        minX = localMin.xX * gs_stGlobalNormPlaneRight.xX;
        minY = localMin.xY * gs_stGlobalNormPlaneRight.xY;
        minZ = localMin.xZ * gs_stGlobalNormPlaneRight.xZ;
        maxX = localMax.xX * gs_stGlobalNormPlaneRight.xX;
        maxY = localMax.xY * gs_stGlobalNormPlaneRight.xY;
        maxZ = localMax.xZ * gs_stGlobalNormPlaneRight.xZ;
        distance = ((minX < maxX) ? minX : maxX) + ((minY < maxY) ? minY : maxY) +
                   ((minZ < maxZ) ? minZ : maxZ);
        if (distance > camera->clipPlanes[1].xW) {
            return 0;
        }
        distance = ((minX > maxX) ? minX : maxX) + ((minY > maxY) ? minY : maxY) +
                   ((minZ > maxZ) ? minZ : maxZ);
        if (distance < camera->clipPlanes[1].xW) {
            _lCullingResult |= 2;
        }
    }

    if (!(_lCullingResult & 4)) {
        minX = localMin.xX * gs_stGlobalNormPlaneUp.xX;
        minY = localMin.xY * gs_stGlobalNormPlaneUp.xY;
        minZ = localMin.xZ * gs_stGlobalNormPlaneUp.xZ;
        maxX = localMax.xX * gs_stGlobalNormPlaneUp.xX;
        maxY = localMax.xY * gs_stGlobalNormPlaneUp.xY;
        maxZ = localMax.xZ * gs_stGlobalNormPlaneUp.xZ;
        distance = ((minX < maxX) ? minX : maxX) + ((minY < maxY) ? minY : maxY) +
                   ((minZ < maxZ) ? minZ : maxZ);
        if (distance > camera->clipPlanes[2].xW) {
            return 0;
        }
        distance = ((minX > maxX) ? minX : maxX) + ((minY > maxY) ? minY : maxY) +
                   ((minZ > maxZ) ? minZ : maxZ);
        if (distance < camera->clipPlanes[2].xW) {
            _lCullingResult |= 4;
        }
    }

    if (!(_lCullingResult & 8)) {
        minX = localMin.xX * gs_stGlobalNormPlaneDown.xX;
        minY = localMin.xY * gs_stGlobalNormPlaneDown.xY;
        minZ = localMin.xZ * gs_stGlobalNormPlaneDown.xZ;
        maxX = localMax.xX * gs_stGlobalNormPlaneDown.xX;
        maxY = localMax.xY * gs_stGlobalNormPlaneDown.xY;
        maxZ = localMax.xZ * gs_stGlobalNormPlaneDown.xZ;
        distance = ((minX < maxX) ? minX : maxX) + ((minY < maxY) ? minY : maxY) +
                   ((minZ < maxZ) ? minZ : maxZ);
        if (distance > camera->clipPlanes[3].xW) {
            return 0;
        }
        distance = ((minX > maxX) ? minX : maxX) + ((minY > maxY) ? minY : maxY) +
                   ((minZ > maxZ) ? minZ : maxZ);
        if (distance < camera->clipPlanes[3].xW) {
            _lCullingResult |= 8;
        }
    }

    return _lCullingResult;
}
