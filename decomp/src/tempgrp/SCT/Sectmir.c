#include "common.h"

#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "SAI/SAI.h"
#include "SCT/SCT.h"
#include "AI/AI.h"
#include "GEO/GEO.h"
#include "AGO/AGO.h"

void* fn_hSearchAnElementInList(void* list, void* element, u16 value);

s32 func_00145C48(void* list, void* element, u16 value)
{
    void* found;
    s32 restored = 0;

    found = fn_hSearchAnElementInList(list, element, value);
    if (found != NULL) {
        restored = 1;
        SAI_fn_bRestoreAnElementFromList(found);
    }
    return restored;
}

void SCT_fn_vGetSymMatrixOfCameraInMirror(POS_tdstMatrix_* hMatrixOfCamera, POS_tdstMatrix_* hMatrixOfSymCamera,
                                        MTH3D_tdstVector_* pstGlobalPositionOfSymCamera,
                                        MTH3D_tdstVector_* pstGlobalNormalOfMirror, MTH3D_tdstVector_* pstGlobalPointOfMirror)
{
    POS_tdstMatrix_ inverse;
    POS_tdstMatrix_ reflection;
    POS_tdstMatrix_ product;
    MTH3D_tdstVector_ reflected;
    MTH3D_tdstVector_ origin;
    MTH3D_tdstVector_ rows[3];
    POS_tdstMatrix_* mirror;
    MTH4D_tdstVector_* translation;
    f32 x;
    f32 y;
    f32 z;
    f32 xx;
    f32 yy;
    f32 zz;
    f32 xy;
    f32 xz;
    f32 yz;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 dot;

    gu_MTH4D_M_vSetIdentityMatrix(&inverse);
    POS_fn_vInvertMatrix(&inverse, hMatrixOfCamera);
    mirror = &reflection;
    gu_MTH4D_M_vSetIdentityMatrix(mirror);

    x = pstGlobalNormalOfMirror->xX;
    y = pstGlobalNormalOfMirror->xY;
    z = pstGlobalNormalOfMirror->xZ;
    xx = x * x * 2.0f;
    yy = y * y * 2.0f;
    zz = z * z * 2.0f;
    xy = x * y * -2.0f;
    xz = x * z * -2.0f;
    yz = y * z * -2.0f;
    MTH3D_M_vSetXYZ_inv(&rows[0], 1.0f - xx, xy, xz);
    MTH3D_M_vSetXYZ_inv(&rows[1], xy, 1.0f - yy, yz);
    MTH3D_M_vSetXYZ_inv(&rows[2], xz, yz, 1.0f - zz);
    _POS_fn_vSetRotationMatrix(mirror, &rows[0], &rows[1], &rows[2]);

    origin = *(MTH3D_tdstVector_*) &inverse.stMatrix.stCol_3;
    dx = origin.xX - pstGlobalPointOfMirror->xX;
    dy = origin.xY - pstGlobalPointOfMirror->xY;
    dz = origin.xZ - pstGlobalPointOfMirror->xZ;
    dot = inv_MTH_fn_xMulAdd3Grouped(dx, dy, dz, pstGlobalNormalOfMirror->xX,
                                     pstGlobalNormalOfMirror->xY, pstGlobalNormalOfMirror->xZ);
    dot = dot * -2.0f;
    MTH3D_M_vSetXYZ_inv(&reflected, dot * pstGlobalNormalOfMirror->xX + origin.xX,
                        dot * pstGlobalNormalOfMirror->xY + origin.xY,
                        dot * pstGlobalNormalOfMirror->xZ + origin.xZ);

    product = inverse;
    _POS_fn_vMulMatrixMatrix(&inverse, mirror, &product);
    if (inverse.ucType == MAT_eTransform_Identity) {
        inverse.ucType = MAT_eTransform_Translation;
    }
    *(MTH3D_tdstVector_*) &inverse.stMatrix.stCol_3 = reflected;
    translation = &inverse.stMatrix.stCol_3;
    translation->xW = 1.0f;
    *pstGlobalPositionOfSymCamera = reflected;

    gu_MTH4D_M_vSetIdentityMatrix(hMatrixOfSymCamera);
    POS_fn_vInvertMatrix(hMatrixOfSymCamera, &inverse);
}

#ifdef PC_PORT
extern s32 inv_GAM_bDrawingInBetween;

#endif
void SCT_fn_vSendSectorWhereIAmToViewportWithMirror(MTH3D_tdstVector_* _p_stAbsolutePositionOfCamera, s16 _hDevice, s16 _hViewport,
                                                    GLD_tdstViewportAttributes_* _p_stVpt,
                                                    HIE_tdstSuperObject_* _hSprObjSector, s32 _lDrawMask)
{
    if (GLI_cGlobalFogIsOn != 0) {
        GLI_vSetFog2(&GLI_stFogColor, GLI_xFogBlendNear, GLI_xFogNear,
                     GLI_xFogBlendFar, GLI_xFogFar, GLI_xFogInfinite);
        GLI_vFogOn();
    }
    GEO_fn_vComputeCameraParametersForCulling(_p_stVpt, fn_xGetSectorZFar(_hSprObjSector));
    SCT_fn_vSendSectorWhereIAmToViewport(_p_stAbsolutePositionOfCamera, _p_stVpt, _hSprObjSector, _lDrawMask);
#ifdef PC_PORT
    if (!inv_GAM_bDrawingInBetween) {
        AGO_vProcessAllParts();
    }
#else
    AGO_vProcessAllParts();
#endif
}
