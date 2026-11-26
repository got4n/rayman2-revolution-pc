#include "common.h"

#include "COL/COL.h"
#include "GEO/GEO.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "FMN/FMN.h"
#include "GAM/GAM.h"

extern COL_tdstGVForCollision_ COL_g_stGVForCol;

void* func_00217048(void* links);
void freemem(void* block);
void inv_COL_fn_vPrepareGVForCollision(COL_tdstGVForCollision_* query);

UbiFile* ubi_file_Factory(UbiDirectory* directory, char* name, u64 offset, u64 length, u64 unk18)
{
    UbiFile* record = malloc(sizeof(UbiFile));

    record->directory = directory;
    record->name = name;
    record->offset = offset;
    record->length = length;
    record->unk18 = unk18;
    record->position = 0;
    record->bigfile = NULL;
    return record;
}

void ubi_file_destroy(UbiFile* record)
{
    freemem(record->name);
    freemem(record);
}

void* func_00217320(UbiFile* record)
{
    return func_00217048(record->directory);
}

UbiFile* inv_UBI_FILE_fn_p_stMatchName(UbiFile* record, char* name)
{
    if (strcmp(record->name, name) != 0) {
        return NULL;
    }
    return record;
}

s32 inv_COL_fn_bSphereTouchesBoundingVolume(void* sphere, POS_tdstMatrix_* sphereMatrix, void* other,
                  POS_tdstMatrix_* otherMatrix, u8 translationOnly, u8 unused, u8 otherIsBox)
{
    MTH3D_tdstVector_ center;
    MTH3D_tdstVector_ otherCenter;
    MTH3D_tdstVector_ sphereMaximum;
    MTH3D_tdstVector_ sphereMinimum;
    MTH3D_tdstVector_ boxMaximum;
    MTH3D_tdstVector_ boxMinimum;
    f32 radius;
    f32 scaledRadius;
    f32 reachSquared;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 distanceSquared;
    f32 x;
    f32 y;
    f32 z;
    f32 tx;
    f32 ty;
    f32 reach;
    f32 boxX;
    f32 boxY;
    f32 boxZ;
    MTH4D_tdstVector_* translation;
    MTH4D_tdstVector_* boxTranslation;
    MTH3D_tdstVector_* position;
    MTH3D_tdstVector_* minimum;
    MTH3D_tdstVector_* maximum;

    POS_fn_vMulMatrixVertex(&center, sphereMatrix, inv_GEO_fn_pIdentity(sphere));

    if (!otherIsBox) {
        translation = &otherMatrix->stMatrix.stCol_3;
        if (translationOnly) {
            position = other;
            x = *(volatile f32*) &position->xX;
            tx = otherMatrix->stMatrix.stCol_3.xX;
            y = position->xY;
            ty = translation->xY;
            reach = *(volatile f32*) &translation->xZ;
            z = position->xZ;
            MTH3D_M_vSetXYZ_inv(&otherCenter, x + tx, y + ty, z + reach);
        } else {
            POS_fn_vMulMatrixVertex(&otherCenter, otherMatrix, other);
        }

        radius = inv_GEO_fn_xGetField0C(sphere);
        scaledRadius = radius * POS_fn_xGetMaxScale(sphereMatrix);
        if (translationOnly) {
            reach = scaledRadius + inv_GEO_fn_xGetField0C(other);
        } else {
            radius = inv_GEO_fn_xGetField0C(other);
            radius *= POS_fn_xGetMaxScale(otherMatrix);
            reach = scaledRadius + radius;
        }
        reachSquared = MTH_fn_xSqrOpt1(reach);

        dx = center.xX - otherCenter.xX;
        dy = center.xY - otherCenter.xY;
        dz = center.xZ - otherCenter.xZ;
        distanceSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
        return distanceSquared <= reachSquared;
    }

    maximum = COL_fn_pGetMaxPointOfParallelBox(other);
    minimum = COL_fn_pGetMinPointOfParallelBox(other);
    boxTranslation = &otherMatrix->stMatrix.stCol_3;
    boxZ = boxTranslation->xZ;
    boxY = boxTranslation->xY;
    boxX = otherMatrix->stMatrix.stCol_3.xX;
    MTH3D_M_vSetXYZ_inv(&boxMaximum, boxX + maximum->xX, boxY + maximum->xY, boxZ + maximum->xZ);
    MTH3D_M_vSetXYZ_inv(&boxMinimum, boxX + minimum->xX, boxY + minimum->xY, boxZ + minimum->xZ);

    radius = inv_GEO_fn_xGetField0C(sphere);
    scaledRadius = radius * POS_fn_xGetMaxScale(sphereMatrix);
    COL_fn_vComputeBoundingBoxOfSphere(&sphereMinimum, &sphereMaximum, &center,
                                       scaledRadius);
    return COL_fn_bDetectCollisionBetweenTwoParallelBoxes(&boxMaximum, &boxMinimum,
                                                          &sphereMaximum, &sphereMinimum);
}

s32 inv_COL_fn_bHasMeshElements(COL_tdstCollideObject_* object)
{
    s16 i = 0;

    if (object->xNbElements > 0) {
        for (; i < object->xNbElements; i++) {
            s16 type = object->d_xListOfElementsTypes[i];

            if (type == 1) {
                return 1;
            }
            if (type > 0) {
                if (type < 9) {
                    if (type >= 7) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void inv_COL_fn_vInitGVMatrices(COL_tdstGVForCollision_* p_stGV)
{
    u32 flags;

    gu_MTH4D_M_vSetIdentityMatrix(&p_stGV->stInvMatrix);
    gu_MTH4D_M_vSetIdentityMatrix(&p_stGV->stD2ST0TransformMatrix);
    gu_MTH4D_M_vSetIdentityMatrix(&p_stGV->stD2ST1TransformMatrix);
    gu_MTH4D_M_vSetIdentityMatrix(&p_stGV->stS2DT0TransformMatrix);
    gu_MTH4D_M_vSetIdentityMatrix(&p_stGV->stS2DT1TransformMatrix);
    POS_fn_vInvertMatrix(&p_stGV->stInvMatrix, p_stGV->p_stStaticGeomObjMatrix);
    _POS_fn_vMulMatrixMatrix(&p_stGV->stD2ST0TransformMatrix, &p_stGV->stInvMatrix,
                             p_stGV->p_stT0DynamicGeomObjMatrix);
    _POS_fn_vMulMatrixMatrix(&p_stGV->stD2ST1TransformMatrix, &p_stGV->stInvMatrix,
                             p_stGV->p_stT1DynamicGeomObjMatrix);
    if (!inv_COL_fn_bHasMeshElements(p_stGV->p_stDynamicColObj)) {
        flags = p_stGV->ulSelectedCollisionCases;
        flags &= ~0xF0;
        flags &= ~0x100;
        flags &= ~0x600;
        flags &= ~0x1800;
        flags &= ~0x1FFE000;
        p_stGV->ulSelectedCollisionCases = flags;
    }
    p_stGV->p_stOctree = p_stGV->p_stStaticColObj->p_stOctree;
    if (p_stGV->bStaticGeomObjHasNoTransformationMatrix) {
        p_stGV->xStaticScale = 1.0f;
    } else {
        p_stGV->xStaticScale = POS_fn_xGetMaxScale(p_stGV->p_stStaticGeomObjMatrix);
    }
    p_stGV->xStaticScale =
        p_stGV->xStaticScale / POS_fn_xGetMaxScale(p_stGV->p_stT0DynamicGeomObjMatrix);
}

void inv_COL_fn_vPrepareGVForCollision(COL_tdstGVForCollision_* p_stGV)
{
    s16 i;
    COL_tdstElementSpheres_* p_stElementSpheres;

    inv_COL_fn_vInitGVMatrices(p_stGV);
    if ((p_stGV->ulSelectedCollisionCases & 0x1FFE000) == 0) {
        return;
    }
    for (i = 0; i < p_stGV->p_stDynamicColObj->xNbElements; i++) {
        if (p_stGV->p_stDynamicColObj->d_xListOfElementsTypes[i] == 7 &&
            (p_stGV->ulSelectedCollisionCases & 0x1E0000)) {
            p_stElementSpheres = p_stGV->p_stDynamicColObj->d_stListOfElements[i];
            p_stGV->p_stDynamicElementSpheres = p_stElementSpheres;
            if (p_stElementSpheres->xNbSpheres != 0) {
                inv_COL_RAY_fn_vCastRayThroughSectors(p_stGV);
            }
        }
    }
}

void COL_fn_vCollideDynamicGeomObjWithStaticGeomObj(COL_tdstCollideObject_* _hGeometricObject1,
                   POS_tdstMatrix_* _p_stT1DynamicGeomObjMatrix,
                   POS_tdstMatrix_* _p_stT0DynamicGeomObjMatrix,
                   COL_tdstCollideObject_* _hGeometricObject2,
                   POS_tdstMatrix_* _p_stGeometricObjectMatrix2, void* _p_vParameter1, s16 _sParameter2,
                   u8 _bStaticGeomObjHasNoTransformationMatrix,
                   u8 _bDynamicGeomObjHasZoomInsteadOfScale)
{
    COL_tdstGVForCollision_* p_stGV = &COL_g_stGVForCol;

    inv_COL_bRayStaticMatrixChanged = 1;
    p_stGV->bStaticGeomObjHasNoTransformationMatrix = _bStaticGeomObjHasNoTransformationMatrix;
    p_stGV->bDynamicGeomObjHasZoomInsteadOfScale = _bDynamicGeomObjHasZoomInsteadOfScale;
    p_stGV->p_stT0DynamicGeomObjMatrix = _p_stT0DynamicGeomObjMatrix;
    p_stGV->p_stT1DynamicGeomObjMatrix = _p_stT1DynamicGeomObjMatrix;
    p_stGV->p_stStaticGeomObjMatrix = _p_stGeometricObjectMatrix2;
    p_stGV->ulSelectedCollisionCases = 0x1FFFFFF;
    p_stGV->p_vParameter1 = _p_vParameter1;
    p_stGV->sParameter2 = _sParameter2;
    p_stGV->p_stDynamicColObj = _hGeometricObject1;
    p_stGV->p_stStaticColObj = _hGeometricObject2;
    if (_hGeometricObject1 != NULL && _hGeometricObject2 != NULL && _hGeometricObject1->xNbPoints != 0 && _hGeometricObject2->xNbPoints != 0 &&
        _hGeometricObject1->xNbElements != 0 && _hGeometricObject2->xNbElements != 0) {
        inv_COL_fn_vPrepareGVForCollision(p_stGV);
    }
}
