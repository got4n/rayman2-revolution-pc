#include "common.h"

#include "COL/COL.h"
#include "GAM/GAM.h"
#include "GEO/GEO.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "PO/PO.h"

#include "AI/AI.h"
#include "GLI/GLI.h"

typedef struct PIC_tdstPickedObject_ {
    s32 xModePicking;
    s16 xNbElements;
    GLI_tdstDataOfElement_ aDEF_stDataOfElement[3];
} PIC_tdstPickedObject_;

typedef struct HIE_tdstPickInfo_ {
    HIE_tdstSuperObject_* hSprObject;
    void* hGeoObject;
    COL_tdstCollideObject_* hCollideObject;
    PIC_tdstPickedObject_ stPickedObject;
    u8 unk5C[0xC4];
} HIE_tdstPickInfo_;

extern MTH3D_tdstVector_ inv_stPickBoxMin;
extern MTH3D_tdstVector_ inv_stPickBoxMax;
extern MTH3D_tdstVector_ inv_stPickLocalStart;
extern MTH3D_tdstVector_ inv_stPickLocalDirection;
extern POS_tdstMatrix_ inv_stPickInverseMatrix;
extern PIC_tdstPickedObject_ inv_stPickResult;
extern MTH3D_tdstVector_ inv_stPickResultHit;
extern void* inv_hPickSphere;
extern void* inv_hPickBox;
extern void* inv_hPickCollideObject;
extern u8 inv_bPickIsBox;
extern u8 inv_bPickBoundsHit;
extern s32 inv_lPickType;
extern void* inv_hDetectSphere;
extern void* inv_hDetectBox;
extern MTH4D_tdstVector_* inv_p_stDetectTranslation;
extern u8 inv_bDetectIsBox;
extern u8 inv_bDetectBoundsHit;
extern void* inv_hDetectCollideObject;
extern s32 inv_lDetectType;
extern MTH3D_tdstVector_ inv_stDetectBoxMin;
extern MTH3D_tdstVector_ inv_stDetectBoxMax;
extern MTH3D_tdstVector_ inv_stDetectLocalStart;
extern MTH3D_tdstVector_ inv_stDetectLocalDirection;
extern POS_tdstMatrix_ inv_stDetectInverseMatrix;

s32 func_00149988(MTH3D_tdstVector_* start, MTH3D_tdstVector_* direction, void* geometry,
                  PIC_tdstPickedObject_* hit);

static inline MTH4D_tdstVector_* inv_HIE_fn_p_stTranslationOf(HIE_tdstSuperObject_* _hSprObj)
{
    POS_tdstMatrix_* matrix = _hSprObj->gu_hGlobalMatrix;

    return &matrix->stMatrix.stCol_3;
}

s32 fn_bDetectIntersectSegmentWithBoundingSphere(void* matrix, MTH3D_tdstVector_* p_stVertexA,
                                                 MTH3D_tdstVector_* p_stVectAB, void* sphere)
{
    MTH3D_tdstVector_ stSphereCenter;
    f32 xRadius;

    POS_fn_vMulMatrixVertex(&stSphereCenter, matrix, inv_GEO_fn_pIdentity(sphere));
    xRadius = inv_GEO_fn_xGetField0C(sphere);
    return INT_fn_bDetectIntersectSegmentWithSphere(p_stVertexA, p_stVectAB, &stSphereCenter,
                                                    xRadius * POS_fn_xGetMaxScale(matrix));
}

s32 func_00149988(MTH3D_tdstVector_* p_stCameraPos, MTH3D_tdstVector_* p_stVertex12,
                  void* p_stObject, PIC_tdstPickedObject_* p_stPickedObject)
{
    s16 i;
    MTH3D_tdstVector_ stSouris3D;
    s32 bReturn;
    s32 bTest;
    s16 xNbElements;
    f32 len;

    inv_MTH3D_fn_vAddXYZ(&stSouris3D, p_stCameraPos, p_stVertex12);
    bReturn = 0;
    bTest = INT_fn_bIntersectSegmentWithFaceOfGeometricObject(
        p_stCameraPos, &stSouris3D, p_stVertex12, p_stObject, 3, &xNbElements,
        p_stPickedObject->aDEF_stDataOfElement);
    if (bTest) {
        MTH3D_tdstVector_ stTemp;

        POS_fn_vMulMatrixVertex(&stTemp, g_p_stCurrentMatrix, p_stCameraPos);
        for (i = 0; i < xNbElements; i++) {
            POS_fn_vMulMatrixVertex(&p_stPickedObject->aDEF_stDataOfElement[i].stHit,
                                    g_p_stCurrentMatrix,
                                    &p_stPickedObject->aDEF_stDataOfElement[i].stHit);
            {
                f32 dx = stTemp.xX - (&p_stPickedObject->aDEF_stDataOfElement[i].stHit)->xX;
                f32 dy = stTemp.xY - (&p_stPickedObject->aDEF_stDataOfElement[i].stHit)->xY;
                f32 dz = stTemp.xZ - (&p_stPickedObject->aDEF_stDataOfElement[i].stHit)->xZ;
                len = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
                p_stPickedObject->aDEF_stDataOfElement[i].xDistance = MTH_fn_xSqrtOpt2(len);
            }
        }
        p_stPickedObject->xNbElements = xNbElements;
        bReturn = 1;
        p_stPickedObject->xModePicking = 0;
    }
    return bReturn;
}

static inline void inv_HIE_fn_vKeepHit(HIE_tdstPickInfo_* p_stPickInfo,
                                       HIE_tdstSuperObject_* _hSprObj)
{
    p_stPickInfo->stPickedObject = inv_stPickResult;
    p_stPickInfo->hSprObject = _hSprObj;
    p_stPickInfo->hGeoObject = 0;
    p_stPickInfo->hCollideObject = inv_hPickCollideObject;
}

s32 fn_xIntersectSegmentWithRepositioningSuperObject(MTH3D_tdstVector_* p_stVertexA,
                                                     MTH3D_tdstVector_* p_stVectAB,
                                                     HIE_tdstSuperObject_* _hSprObj,
                                                     HIE_tdstPickInfo_* p_stPickInfo,
                                                     s16* p_xNbPickInfo)
{
    u32 ulSpoFlags = _hSprObj->flags;
    HIE_tdstSuperObject_* hChild;

    if (ulSpoFlags & HIE_FLAG_NOT_PICKABLE_inv) {
        return *p_xNbPickInfo;
    }
    inv_lPickType = _hSprObj->lTypeOfLinkedObject;
    if (_hSprObj->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
        if (ulSpoFlags & inv_HIE_ulPickExcludeFlags) {
            return *p_xNbPickInfo;
        }
    } else if (inv_HIE_ulPickExcludeFlags & 0x80) {
        if (!(inv_HIE_fn_p_stGetEngineObject(_hSprObj)->h_3dData->shadow.control.flags & 0x100)) {
            return *p_xNbPickInfo;
        }
    }

    inv_hPickSphere = NULL;
    inv_hPickBox = NULL;
    inv_hPickCollideObject = NULL;
    HIE_fn_vPushSuperObjectMatrix(_hSprObj);
    inv_bPickIsBox = ulSpoFlags & HIE_FLAG_BOX_BOUNDS_inv;
    if (ulSpoFlags & HIE_FLAG_BOX_BOUNDS_inv) {
        inv_hPickBox = _hSprObj->hCollideBoundingVolume;
        if (inv_hPickBox != NULL) {
            MTH3D_tdstVector_* minPoint = COL_fn_pGetMinPointOfParallelBox(inv_hPickBox);
            MTH3D_tdstVector_* maxPoint = COL_fn_pGetMaxPointOfParallelBox(inv_hPickBox);
            MTH4D_tdstVector_* translation = inv_HIE_fn_p_stTranslationOf(_hSprObj);

            MTH3D_M_vAddXYZ_inv(&inv_stPickBoxMin, minPoint, translation);
            MTH3D_M_vAddXYZ_inv(&inv_stPickBoxMax, maxPoint, translation);
        }
    } else {
        inv_hPickSphere = _hSprObj->hCollideBoundingVolume;
    }

    if (inv_hPickSphere == NULL && inv_hPickBox == NULL) {
        inv_bPickBoundsHit = 1;
    } else if (inv_bPickIsBox == 0) {
        inv_bPickBoundsHit = fn_bDetectIntersectSegmentWithBoundingSphere(
            _hSprObj->gu_hGlobalMatrix, p_stVertexA, p_stVectAB, inv_hPickSphere);
    } else {
        inv_bPickBoundsHit = INT_fn_bDetectIntersectSegmentWithBox(
            p_stVertexA, p_stVectAB, &inv_stPickBoxMin, &inv_stPickBoxMax, NULL);
    }

    if (inv_bPickBoundsHit) {
        s32 type = inv_lPickType;

        if (type & 0x600) {
            inv_hPickCollideObject = NULL;
        } else if (type & HIE_TYPE_PHYSICAL_OBJECT_inv) {
            inv_hPickCollideObject =
                ((PO_tdstPhysicalObject_*) _hSprObj->hLinkedObject)->_hCollideSet;
        } else if (type == HIE_TYPE_ACTOR_inv) {
            inv_hPickCollideObject = fn_hMakeOneObjectFromManyZdx(3, _hSprObj);
        }

        if (inv_hPickCollideObject != NULL) {
            if (ulSpoFlags & HIE_FLAG_TRANSLATION_ONLY_inv) {
                MTH3D_M_vSubXYZ_inv(&inv_stPickLocalStart, p_stVertexA,
                                    inv_HIE_fn_p_stTranslationOf(_hSprObj));
                inv_stPickLocalDirection = *p_stVectAB;
            } else {
                POS_fn_vInvertMatrix(&inv_stPickInverseMatrix, _hSprObj->gu_hGlobalMatrix);
                POS_fn_vMulMatrixVertex(&inv_stPickLocalStart, &inv_stPickInverseMatrix,
                                        p_stVertexA);
                POS_fn_vApplyMatrixToVector(&inv_stPickLocalDirection, &inv_stPickInverseMatrix,
                                            p_stVectAB);
            }

            if (func_00149988(&inv_stPickLocalStart, &inv_stPickLocalDirection,
                              inv_hPickCollideObject, &inv_stPickResult)) {
                if (*p_xNbPickInfo == 0) {
                    inv_HIE_fn_vKeepHit(p_stPickInfo, _hSprObj);
                    *p_xNbPickInfo = 1;
                    inv_MTH3D_fn_vSubXYZ(p_stVectAB, &inv_stPickResultHit, p_stVertexA);
                } else if (inv_stPickResult.aDEF_stDataOfElement[0].xDistance <
                           p_stPickInfo->stPickedObject.aDEF_stDataOfElement[0].xDistance) {
                    inv_HIE_fn_vKeepHit(p_stPickInfo, _hSprObj);
                    inv_MTH3D_fn_vSubXYZ(p_stVectAB, &inv_stPickResultHit, p_stVertexA);
                }
            }
        }
    }

    if (inv_bPickBoundsHit || (_hSprObj->flags & HIE_FLAG_ALWAYS_VISIT_CHILDREN_inv)) {
        for (hChild = _hSprObj->hFirstElementSta; hChild != NULL;
             hChild = hChild->hNextBrotherSta) {
            fn_xIntersectSegmentWithRepositioningSuperObject(p_stVertexA, p_stVectAB, hChild,
                                                             p_stPickInfo, p_xNbPickInfo);
        }
    }
    HIE_fn_vPopMatrix();
    return *p_xNbPickInfo;
}

s32 HIE_bIntersectSegmentWithSuperObject(MTH3D_tdstVector_* _p_stVertexA,
                                         MTH3D_tdstVector_* _p_stVectAB,
                                         HIE_tdstSuperObject_* hSprObj,
                                         HIE_tdstPickInfo_* p_stPickInfo)
{
    s16 xNbPickInfo = 0;
    s32 intersects;

    HIE_fn_vPushMatrix();
    intersects = fn_xIntersectSegmentWithRepositioningSuperObject(
        _p_stVertexA, _p_stVectAB, hSprObj, p_stPickInfo, &xNbPickInfo);
    HIE_fn_vPopMatrix();
    return intersects;
}

static inline void inv_HIE_fn_vSetNormalized(MTH3D_tdstVector_* normal, f32 x, f32 y, f32 z)
{
    f32 length;
    f32 scale;

    length = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    length = MTH_fn_xSqrtOpt2(length);
    if (length != 0.0f) {
        scale = 1.0f / length;
        MTH3D_M_vSetXYZ_inv(normal, scale * x, scale * y, scale * z);
    }
}

s32 HIE_bIntersectSegmentWithFirstSuperObject(MTH3D_tdstVector_* p_stVertexA,
                                              MTH3D_tdstVector_* p_stVectAB,
                                              HIE_tdstSuperObject_* _hSprObj,
                                              MTH3D_tdstVector_* _p_stHit,
                                              MTH3D_tdstVector_* _p_stNormal,
                                              HIE_tdstSuperObject_** _p_hSprObjHit)
{
    HIE_tdstPickInfo_ stPickInfo;
    MTH4D_tdstVector_ stNormal;
    MTH3D_tdstVector_ vertex;
    COL_tdstCollideObject_* p_stCollObject;
    GLI_tdstDataOfElement_* p_stDataOfElement;
    s32 element;
    f32 x;
    f32 y;
    f32 z;

    if (HIE_bIntersectSegmentWithSuperObject(p_stVertexA, p_stVectAB, _hSprObj, &stPickInfo) > 0) {
        p_stDataOfElement = &stPickInfo.stPickedObject.aDEF_stDataOfElement[0];
        p_stCollObject = stPickInfo.hCollideObject;
        if (stPickInfo.hSprObject->lTypeOfLinkedObject &
            (HIE_TYPE_GEOMETRIC_OBJECT_inv | HIE_TYPE_PHYSICAL_OBJECT_inv | HIE_TYPE_ACTOR_inv)) {
            *_p_hSprObjHit = stPickInfo.hSprObject;
        } else {
            *_p_hSprObjHit = NULL;
        }
        *_p_stHit = p_stDataOfElement->stHit;
        element = p_stDataOfElement->xElements;
        if (p_stCollObject->d_xListOfElementsTypes[element] == COL_ELEMENT_SPHERES_inv) {
            COL_tdstIndexedSphere_* spheres =
                ((COL_tdstElementSpheres_*) p_stCollObject->d_stListOfElements[element])
                    ->d_stListOfSpheres;

            POS_fn_vMulMatrixVertex(
                &vertex, (*_p_hSprObjHit)->gu_hGlobalMatrix,
                &p_stCollObject
                     ->d_stListOfPoints[spheres[p_stDataOfElement->xIndexOfFace].xCenterPoint]);
            x = _p_stHit->xX - vertex.xX;
            _p_stNormal->xX = x;
            y = _p_stHit->xY - vertex.xY;
            _p_stNormal->xY = y;
            z = _p_stHit->xZ - vertex.xZ;
            _p_stNormal->xZ = z;
            inv_HIE_fn_vSetNormalized(_p_stNormal, x, y, z);
        } else {
            func_0020BF38(p_stCollObject, element, p_stDataOfElement->xIndexOfFace, (MTH3D_tdstVector_*) &stNormal);
            POS_fn_vApplyMatrixToVector(_p_stNormal, (*_p_hSprObjHit)->gu_hGlobalMatrix, (MTH3D_tdstVector_*) &stNormal);
            inv_HIE_fn_vSetNormalized(_p_stNormal, _p_stNormal->xX, _p_stNormal->xY,
                                      _p_stNormal->xZ);
        }
        return 1;
    }
    return 0;
}

s32 HIE_bDetectIntersectSegmentWithSuperObject(MTH3D_tdstVector_* p_stVertexA,
                                               MTH3D_tdstVector_* p_stVectAB,
                                               HIE_tdstSuperObject_* _hSprObj, void* camera)
{
    HIE_tdstSuperObject_* hChild;
    u32 isBox;

    inv_hDetectSphere = NULL;
    inv_hDetectBox = NULL;
    inv_hDetectCollideObject = NULL;
    inv_p_stDetectTranslation = inv_HIE_fn_p_stTranslationOf(_hSprObj);
    isBox = (u8) _hSprObj->flags & HIE_FLAG_BOX_BOUNDS_inv;
    inv_bDetectIsBox = isBox;
    if (isBox) {
        inv_hDetectBox = _hSprObj->hCollideBoundingVolume;
        if (inv_hDetectBox != NULL) {
            MTH3D_tdstVector_* minPoint = COL_fn_pGetMinPointOfParallelBox(inv_hDetectBox);
            MTH3D_tdstVector_* maxPoint = COL_fn_pGetMaxPointOfParallelBox(inv_hDetectBox);
            MTH4D_tdstVector_* translation = inv_p_stDetectTranslation;

            MTH3D_M_vAddXYZ_inv(&inv_stDetectBoxMin, minPoint, translation);
            MTH3D_M_vAddXYZ_inv(&inv_stDetectBoxMax, maxPoint, translation);
        }
    } else {
        inv_hDetectSphere = _hSprObj->hCollideBoundingVolume;
    }

    if (inv_hDetectSphere == NULL && inv_hDetectBox == NULL) {
        inv_bDetectBoundsHit = 1;
    } else if (inv_bDetectIsBox == 0) {
        inv_bDetectBoundsHit = fn_bDetectIntersectSegmentWithBoundingSphere(
            _hSprObj->gu_hGlobalMatrix, p_stVertexA, p_stVectAB, inv_hDetectSphere);
    } else {
        inv_bDetectBoundsHit = INT_fn_bDetectIntersectSegmentWithBox(
            p_stVertexA, p_stVectAB, &inv_stDetectBoxMin, &inv_stDetectBoxMax, NULL);
    }

    if (inv_bDetectBoundsHit) {
        inv_lDetectType = _hSprObj->lTypeOfLinkedObject;
        if (inv_lDetectType & 0x600) {
            inv_hDetectCollideObject = NULL;
        } else if (inv_lDetectType & HIE_TYPE_PHYSICAL_OBJECT_inv) {
            inv_hDetectCollideObject =
                ((PO_tdstPhysicalObject_*) _hSprObj->hLinkedObject)->_hCollideSet;
        } else if (inv_lDetectType == HIE_TYPE_ACTOR_inv) {
            inv_hDetectCollideObject = fn_hMakeOneObjectFromManyZdx(3, _hSprObj);
        }

        if (inv_hDetectCollideObject != NULL) {
            if ((u8) _hSprObj->flags & HIE_FLAG_TRANSLATION_ONLY_inv) {
                MTH4D_tdstVector_* translation = inv_p_stDetectTranslation;

                MTH3D_M_vSubXYZ_inv(&inv_stDetectLocalStart, p_stVertexA, translation);
                inv_stDetectLocalDirection = *p_stVectAB;
            } else {
                POS_fn_vInvertMatrix(&inv_stDetectInverseMatrix, _hSprObj->gu_hGlobalMatrix);
                POS_fn_vMulMatrixVertex(&inv_stDetectLocalStart, &inv_stDetectInverseMatrix,
                                        p_stVertexA);
                POS_fn_vApplyMatrixToVector(&inv_stDetectLocalDirection, &inv_stDetectInverseMatrix,
                                            p_stVectAB);
            }

            if (inv_hDetectCollideObject != NULL &&
                INT_fn_bDetectIntersectSegmentWithFaceOfGeometricObject(&inv_stDetectLocalStart,
                                                                        &inv_stDetectLocalDirection,
                                                                        inv_hDetectCollideObject)) {
                if (camera != NULL && (_hSprObj->flags >> 20 & 1)) {
                    inv_CAM_fn_vMarkSuperObjectForTransparency(camera, _hSprObj);
                } else {
                    return 1;
                }
            }
        }
    }

    if (inv_bDetectBoundsHit || (_hSprObj->flags & HIE_FLAG_ALWAYS_VISIT_CHILDREN_inv)) {
        for (hChild = _hSprObj->hFirstElementSta; hChild != NULL;
             hChild = hChild->hNextBrotherSta) {
            if (hChild->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv &&
                HIE_bDetectIntersectSegmentWithSuperObject(p_stVertexA, p_stVectAB, hChild,
                                                           camera)) {
                return 1;
            }
        }
    }
    return 0;
}
