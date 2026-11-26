#include "common.h"

#include "GEO/GEO.h"
#include "MTH/MTH.h"

#include "ps2/vu0.h"
#include "COL/COL.h"
#include "ps2/PS2.h"

#ifdef PC_PORT
#define SCRATCH_VECTOR ((ScratchVector*) PC_SCRATCH(0x70003FF0))
#else
#define SCRATCH_VECTOR ((ScratchVector*) 0x70003FF0)
#endif

extern u8 D_FD0[];
extern u8 D_BASE[] __asm__("0x0");

s32 GEO_xGetElementType(COL_tdstCollideObject_* object, s32 element)
{
    s16 index = element;

    return object->d_xListOfElementsTypes[index];
}

void GEO_vSetPointOfObject(COL_tdstCollideObject_* hObject, MTH3D_tdstVector_* p_stPoint, s16 xIndexOfPoint)
{
    hObject->d_stListOfPoints[xIndexOfPoint] = *p_stPoint;
}

void GEO_vGetPointOfObject(COL_tdstCollideObject_* hObject, MTH3D_tdstVector_* p_stPoint, s16 xIndexOfPoint)
{
    *p_stPoint = hObject->d_stListOfPoints[xIndexOfPoint];
}

s16 GEO_xGetGeometricObjectNumberOfElementsMax(COL_tdstCollideObject_* object)
{
    return object->xNbElements;
}

void func_0020BF38(COL_tdstCollideObject_* object, s32 element, s32 triangle, MTH3D_tdstVector_* out)
{
    ScratchVector* scratch = SCRATCH_VECTOR;
    s16 elementIndex = element;
    s16 triangleIndex = triangle;
    COL_tdstElementIndexedTriangles_* set;
    f32 zero = 0.0f;

    MTH3D_M_vSetXYZ_inv(out, zero, zero, zero);
    if (object->d_xListOfElementsTypes[elementIndex] != 1) {
        return;
    }

    set = object->d_stListOfElements[elementIndex];
    scratch->xyz = object->d_stListOfPoints[set->d_stListOfFacesTripled[triangleIndex].a3_xIndex[0]];
    SCRATCH_VECTOR->w = zero;
    VU0_LQC2(3, scratch, 0);

    scratch->xyz = object->d_stListOfPoints[set->d_stListOfFacesTripled[triangleIndex].a3_xIndex[1]];
    SCRATCH_VECTOR->w = zero;
    VU0_LQC2(4, scratch, 0);

    scratch->xyz = object->d_stListOfPoints[set->d_stListOfFacesTripled[triangleIndex].a3_xIndex[2]];
    SCRATCH_VECTOR->w = zero;
    VU0_LQC2(5, scratch, 0);

    inv_PS2_fn_vCallVu0Microprogram((u32) (D_FD0 - D_BASE) >> 3);
    while (inv_PS2_fn_bIsVu0Busy() != 0) {
    }

    VU0_SQC2(12, scratch, 0);
    *out = scratch->xyz;
}

void* COL_fn_pGetMinPointOfParallelBox(void* _p_stParallelBox)
{
    return _p_stParallelBox;
}

void* COL_fn_pGetMaxPointOfParallelBox(MTH3D_tdstVector_* _p_stParallelBox)
{
    return &_p_stParallelBox[1];
}

void gu__GEO_vGetPointOfObject(MTH3D_tdstVector_* point, f32* radius, COL_tdstCollideObject_* object)
{
    COL_tdstElementSpheres_* set = object->d_stListOfElements[0];
    COL_tdstIndexedSphere_* sphere = set->d_stListOfSpheres;
    MTH3D_tdstVector_* points = object->d_stListOfPoints;

    points += sphere->xCenterPoint;
    *point = *points;
    *radius = sphere->xRadius;
}

void* inv_GEO_fn_pIdentity(void* element)
{
    return element;
}

f32 inv_GEO_fn_xGetField0C(void* element)
{
    COL_tdstBoundingSphere_* sphere = element;

    return sphere->xRadius;
}

void func_0020C128(MTH3D_tdstVector_* point, f32* radius, MTH3D_tdstVector_* from,
                   MTH3D_tdstVector_* to, f32 limit)
{
    MTH3D_tdstVector_ delta;
    f32 distance;
    f32 factor;

    inv_MTH3D_fn_vSubXYZ(&delta, from, to);
    distance = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ));
    if (distance <= limit) {
        *point = *from;
        *radius = limit;
        return;
    }
    factor = (1.0f - limit / distance) * 0.5f;
    MTH3D_M_vSetXYZ_inv(point, from->xX + factor * (to->xX - from->xX),
                        from->xY + factor * (to->xY - from->xY),
                        from->xZ + factor * (to->xZ - from->xZ));
    *radius = (distance + limit) * 0.5f;
}

COL_tdstCollideObject_*
GEO_fn_hTransformTableOfObjectInOneObjectForGAM(COL_tdstCollideObject_* target,
                                                COL_tdstCollideObject_** objects, s16 objectCount)
{
    COL_tdstCollideObject_** objectsEnd;
    COL_tdstCollideObject_** object;
    void** element;
    void** elementsEnd;
    s16* elementType;
    COL_tdstElementSpheres_* sphereElement;
    COL_tdstElementAlignedBoxes_* boxElement;
    COL_tdstElementSpheres_* sourceSpheres;
    COL_tdstElementAlignedBoxes_* sourceBoxes;
    COL_tdstIndexedSphere_* nextSphere;
    COL_tdstIndexedSphere_* copied;
    COL_tdstIndexedSphere_* copiedSpheresEnd;
    COL_tdstIndexedAlignedBox_* nextBox;
    COL_tdstIndexedAlignedBox_* copiedBoxesEnd;
    s32 pointCount;
    s16 count;
    s32 boxSize;
    s32 size12;
    s16 mergedPoints;
    s32 pointBytes;

    boxSize = sizeof(COL_tdstIndexedAlignedBox_);
    size12 = 12;
    mergedPoints = 0;
    object = objects;
    element = target->d_stListOfElements;
    sphereElement = element[0];
    objectsEnd = objects + objectCount;
    boxElement = element[1];
    nextBox = boxElement->d_stListOfAlignedBoxes;
    sphereElement->xNbSpheres = 0;
    nextSphere = sphereElement->d_stListOfSpheres;
    boxElement->xNbAlignedBoxes = 0;
    while (object < objectsEnd) {
        pointCount = (*object)->xNbPoints;
        pointBytes = size12 * pointCount;
        if (pointBytes != 0) {
            memcpy(&target->d_stListOfPoints[mergedPoints], (*object)->d_stListOfPoints,
                   pointBytes);
            element = (*object)->d_stListOfElements;
            elementType = (*object)->d_xListOfElementsTypes;
            elementsEnd = element + (*object)->xNbElements;
            while (element < elementsEnd) {
                switch (*elementType) {
                case COL_ELEMENT_SPHERES_inv:
                    sourceSpheres = *element;
                    count = sourceSpheres->xNbSpheres;
                    memcpy(nextSphere, sourceSpheres->d_stListOfSpheres, size12 * count);
                    if (mergedPoints != 0) {
                        copiedSpheresEnd = nextSphere + count;
                        for (copied = nextSphere; copied < copiedSpheresEnd; copied++) {
                            copied->xCenterPoint += mergedPoints;
                        }
                    }
                    sphereElement->xNbSpheres += count;
                    nextSphere += count;
                    break;
                case COL_ELEMENT_ALIGNED_BOXES_inv:
                    sourceBoxes = *element;
                    count = sourceBoxes->xNbAlignedBoxes;
                    memcpy(nextBox, sourceBoxes->d_stListOfAlignedBoxes, boxSize * count);
                    if (mergedPoints != 0) {
                        copiedBoxesEnd = nextBox + count;
                        while (nextBox < copiedBoxesEnd) {
                            nextBox->xMinPoint += mergedPoints;
                            nextBox->xMaxPoint += mergedPoints;
                            nextBox++;
                        }
                    } else {
                        nextBox += count;
                    }
                    boxElement->xNbAlignedBoxes += count;
                    break;
                }
                element++;
                elementType++;
            }
            mergedPoints += pointCount;
        }
        object++;
    }
    target->xNbPoints = mergedPoints;
    return target;
}
