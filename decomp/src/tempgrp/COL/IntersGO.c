#include "common.h"

#include "COL/COL.h"
#include "MTH/MTH.h"

#include "AI/AI.h"
#include "GAM/GAM.h"
#include "GEO/GEO.h"

typedef struct IntersectionPair {
    s16 x;
    s16 y;
} IntersectionPair;

extern MTH3D_tdstVector_ qword_390208;

s32 inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(MTH3D_tdstVector_* origin,
                                                         MTH3D_tdstVector_* direction,
                                                         MTH3D_tdstVector_* center, f32* parameter,
                                                         MTH3D_tdstVector_* hit,
                                                         MTH3D_tdstVector_* normal, f32 radius);

f32 func_0020F940(MTH3D_tdstVector_* p_stPoint, MTH3D_tdstVector_* p_stVertexA,
                  MTH3D_tdstVector_* p_stVectAB)
{
    MTH3D_tdstVector_ stVector;
    MTH3D_tdstVector_ stHit;
    f32 xSqrNorm;
    f32 xDot;
    f32 xT;
    f32 xSqrDistance;

    inv_MTH3D_fn_vSubXYZ(&stVector, p_stPoint, p_stVertexA);

    if (!MTH3D_M_bIsZeroXYZ_inv(p_stVectAB)) {
        xSqrNorm = MTH_fn_xSqrAddSqrAddSqrOpt2(p_stVectAB->xX, p_stVectAB->xY, p_stVectAB->xZ);
        xDot = inv_MTH_fn_xMulAdd3Grouped(p_stVectAB->xX, p_stVectAB->xY, p_stVectAB->xZ,
                                stVector.xX, stVector.xY, stVector.xZ);
        xT = xDot / xSqrNorm;
        if (xT < 0.0f) {
            stHit = *p_stVertexA;
        } else if (xT > 1.0f) {
            inv_MTH3D_fn_vAddXYZ(&stHit, p_stVectAB, p_stVertexA);
        } else {
            MTH3D_M_vSetXYZ_inv(&stHit, xT * p_stVectAB->xX + p_stVertexA->xX,
                                xT * p_stVectAB->xY + p_stVertexA->xY,
                                xT * p_stVectAB->xZ + p_stVertexA->xZ);
        }
    } else {
        stHit = *p_stVertexA;
    }

    {
        f32 x = stHit.xX - p_stPoint->xX;
        f32 y = stHit.xY - p_stPoint->xY;
        f32 z = stHit.xZ - p_stPoint->xZ;

        xSqrDistance = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    }
    return xSqrDistance;
}

void inv_INT_fn_vInsertSortedIntersection(s16 xNbMaxElements, u16* p_xNbElements,
                                          GLI_tdstDataOfElement_* d_stDataOfElement,
                                          MTH3D_tdstVector_* p_stVertexA, u8 bBack,
                                          MTH3D_tdstVector_* p_stHit, s16 xElements,
                                          s16 xDataElementIndex)
{
    GLI_tdstDataOfElement_* entry;
    s16 xFaceIndex;
    f32 x;
    f32 y;
    f32 z;
    f32 distance;

    if (bBack) {
        return;
    }

    x = p_stHit->xX - p_stVertexA->xX;
    y = p_stHit->xY - p_stVertexA->xY;
    z = p_stHit->xZ - p_stVertexA->xZ;
    distance = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z));

    xFaceIndex = *p_xNbElements;
    if (xFaceIndex > 0) {
        entry = &d_stDataOfElement[xFaceIndex];
        if (distance < entry[-1].xDistance) {
            do {
                if (xFaceIndex < xNbMaxElements) {
                    entry[0] = entry[-1];
                }
                entry--;
                xFaceIndex--;
            } while (xFaceIndex > 0 && distance < entry[-1].xDistance);
        }
    }
    if (xFaceIndex < xNbMaxElements) {
        GLI_tdstDataOfElement_* inserted = &d_stDataOfElement[xFaceIndex];

        inserted->stHit = *p_stHit;
        inserted->xDistance = distance;
        inserted->xElements = xElements;
        inserted->xIndexOfFace = xDataElementIndex;
        if ((s16) *p_xNbElements < xNbMaxElements) {
            (*p_xNbElements)++;
        }
    }
}

s32 INT_fn_bIntersectSegmentWithFaceOfGeometricObject(MTH3D_tdstVector_* p_stVertexA,
                                                  MTH3D_tdstVector_* p_stVertexB,
                                                  MTH3D_tdstVector_* p_stVectAB,
                                                  COL_tdstCollideObject_* p_stGeomObj,
                                                  s16 xNbMaxElements, s16* p_xNbElements,
                                                  GLI_tdstDataOfElement_* d_stDataOfElement)
{
    MTH3D_tdstVector_ stNormal;
    MTH3D_tdstVector_ stHit;
    COL_tdstOctreeNode_* aDEF_pstSelectedNode[48];
    f32 aDEF_xSelectedT[48];
    s16 xNumberOfSelectedNodes;
    f32 xT;
    s16 xElementIndex;
    s16 xDataElementIndex;
    COL_tdstElementIndexedTriangles_* p_stElementIndexedTriangles;
    COL_tdstElementSpheres_* p_stElementSpheres;
    COL_tdstElementAlignedBoxes_* p_stElementAlignedBoxes;
    COL_tdstIndexedSphere_* p_stIndexedSphere;
    COL_tdstIndexedAlignedBox_* p_stIndexedAlignedBox;
    void* hMaterial;
    void* hCM;
    s16 xIndex;
    s16 xNodeIndex;
    s16 xNbElements;
    u8* p_xListIndex;

    *p_xNbElements = 0;
    if (p_stGeomObj->p_stOctree == NULL) {
        for (xElementIndex = 0; xElementIndex < p_stGeomObj->xNbElements; xElementIndex++) {
            switch (p_stGeomObj->d_xListOfElementsTypes[xElementIndex]) {
            case 1:
                if (g_xMask) {
                    hMaterial = ((COL_tdstElementIndexedTriangles_*)
                                     p_stGeomObj->d_stListOfElements[xElementIndex])
                                    ->hMaterial;
                    if (!hMaterial)
                        break;
                    hCM = GMT_fn_hGetCollideMaterial(hMaterial);
                    if (hCM == (void*) 0xFFFFFFFF)
                        break;
                    if (!(GMT_fn_hGetCollideMaterialIdentifier(hCM) & g_xMask))
                        break;
                }
                p_stElementIndexedTriangles = p_stGeomObj->d_stListOfElements[xElementIndex];
                for (xDataElementIndex = 0;
                     xDataElementIndex < p_stElementIndexedTriangles->xNbFaces;
                     xDataElementIndex++) {
                    if (inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
                            p_stVertexA, p_stVectAB,
                            &p_stGeomObj
                                 ->d_stListOfPoints[p_stElementIndexedTriangles
                                                        ->d_stListOfFacesTripled[xDataElementIndex]
                                                        .a3_xIndex[0]],
                            &p_stGeomObj
                                 ->d_stListOfPoints[p_stElementIndexedTriangles
                                                        ->d_stListOfFacesTripled[xDataElementIndex]
                                                        .a3_xIndex[1]],
                            &p_stGeomObj
                                 ->d_stListOfPoints[p_stElementIndexedTriangles
                                                        ->d_stListOfFacesTripled[xDataElementIndex]
                                                        .a3_xIndex[2]],
                            &xT, &stHit)) {
                        inv_INT_fn_vInsertSortedIntersection(
                            xNbMaxElements, p_xNbElements, d_stDataOfElement, p_stVertexA, 0,
                            &stHit, xElementIndex, xDataElementIndex);
                    }
                }
                break;
            case 7:
                p_stElementSpheres = p_stGeomObj->d_stListOfElements[xElementIndex];
                xDataElementIndex = 0;
                for (;;) {
                    goto test_sphere;
                next_sphere:
                    p_stIndexedSphere = &p_stElementSpheres->d_stListOfSpheres[xDataElementIndex];
                    if (inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(
                            p_stVertexA, p_stVectAB,
                            &p_stGeomObj->d_stListOfPoints[p_stIndexedSphere->xCenterPoint], &xT,
                            &stHit, &stNormal, p_stIndexedSphere->xRadius)) {
                        inv_INT_fn_vInsertSortedIntersection(
                            xNbMaxElements, p_xNbElements, d_stDataOfElement, p_stVertexA, 0,
                            &stHit, xElementIndex, xDataElementIndex);
                    }
                    xDataElementIndex++;
                test_sphere:
                    if (xDataElementIndex >= p_stElementSpheres->xNbSpheres)
                        goto next_primitive;
                    if (g_xMask) {
                        hMaterial = GEO_hGetGameMaterialOfIndexedSphere(p_stGeomObj, xElementIndex,
                                                                        xDataElementIndex);
                        if (!hMaterial)
                            goto next_primitive;
                        hCM = GMT_fn_hGetCollideMaterial(hMaterial);
                        if (hCM == (void*) 0xFFFFFFFF)
                            goto next_primitive;
                        if (!(GMT_fn_hGetCollideMaterialIdentifier(hCM) & g_xMask))
                            goto next_primitive;
                    }
                    goto next_sphere;
                }
                break;
            case 8:
                p_stElementAlignedBoxes = p_stGeomObj->d_stListOfElements[xElementIndex];
                for (xDataElementIndex = 0;
                     xDataElementIndex < p_stElementAlignedBoxes->xNbAlignedBoxes;
                     xDataElementIndex++) {
                    if (g_xMask) {
                        hMaterial = GEO_vGetGameMaterialOfIndexedAlignedBox(
                            p_stGeomObj, xElementIndex, xDataElementIndex);
                        if (!hMaterial)
                            break;
                        hCM = GMT_fn_hGetCollideMaterial(hMaterial);
                        if (hCM == (void*) 0xFFFFFFFF)
                            break;
                        if (!(GMT_fn_hGetCollideMaterialIdentifier(hCM) & g_xMask))
                            break;
                    }
                    p_stIndexedAlignedBox =
                        &p_stElementAlignedBoxes->d_stListOfAlignedBoxes[xDataElementIndex];
                    if (INT_fn_bIntersectSegmentWithBox(
                            p_stVertexA, p_stVertexB, p_stVectAB,
                            &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMinPoint],
                            &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMaxPoint], &xT,
                            &stHit, &stNormal)) {
                        inv_INT_fn_vInsertSortedIntersection(
                            xNbMaxElements, p_xNbElements, d_stDataOfElement, p_stVertexA, 0,
                            &stHit, xElementIndex, xDataElementIndex);
                    }
                }
                break;
            }
        next_primitive:;
        }
    } else {
        _COL_fn_vExploreOctreeWithSegment(p_stGeomObj->p_stOctree, p_stVertexA, p_stVectAB,
                                          aDEF_pstSelectedNode, aDEF_xSelectedT,
                                          &xNumberOfSelectedNodes);
        COL_g_lFacesTagCounter++;
        for (xNodeIndex = 0; xNodeIndex < xNumberOfSelectedNodes; xNodeIndex++) {
            s16* p_xNodeFaces = aDEF_pstSelectedNode[xNodeIndex]->d_xFaceIndexList;

            xNbElements = *p_xNodeFaces;
            p_xListIndex = (u8*) (p_xNodeFaces + 1);
            for (xIndex = 0; xIndex < xNbElements; xIndex++) {
                if (*p_xListIndex < 0x80) {
                    xElementIndex = *p_xListIndex++;
                } else {
                    xElementIndex = *p_xListIndex++ << 8;
                    xElementIndex = (xElementIndex + *p_xListIndex++) & 0x7FFF;
                }
                if (*p_xListIndex < 0x80) {
                    xDataElementIndex = *p_xListIndex++;
                } else {
                    xDataElementIndex = *p_xListIndex++ << 8;
                    xDataElementIndex = (xDataElementIndex + *p_xListIndex++) & 0x7FFF;
                }
                if (COL_g_d_lTaggedFacesTable[p_stGeomObj->p_stOctree
                                                  ->d_xElementBasesTable[xElementIndex] +
                                              xDataElementIndex] == COL_g_lFacesTagCounter)
                    continue;
                switch (p_stGeomObj->d_xListOfElementsTypes[xElementIndex]) {
                case 1:
                    if (g_xMask) {
                        hMaterial = ((COL_tdstElementIndexedTriangles_*)
                                         p_stGeomObj->d_stListOfElements[xElementIndex])
                                        ->hMaterial;
                        if (!hMaterial)
                            break;
                        hCM = GMT_fn_hGetCollideMaterial(hMaterial);
                        if (hCM == (void*) 0xFFFFFFFF)
                            break;
                        if (!(GMT_fn_hGetCollideMaterialIdentifier(hCM) & g_xMask))
                            break;
                    }
                    p_stElementIndexedTriangles = p_stGeomObj->d_stListOfElements[xElementIndex];
                    if (inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
                            p_stVertexA, p_stVectAB,
                            &p_stGeomObj
                                 ->d_stListOfPoints[p_stElementIndexedTriangles
                                                        ->d_stListOfFacesTripled[xDataElementIndex]
                                                        .a3_xIndex[0]],
                            &p_stGeomObj
                                 ->d_stListOfPoints[p_stElementIndexedTriangles
                                                        ->d_stListOfFacesTripled[xDataElementIndex]
                                                        .a3_xIndex[1]],
                            &p_stGeomObj
                                 ->d_stListOfPoints[p_stElementIndexedTriangles
                                                        ->d_stListOfFacesTripled[xDataElementIndex]
                                                        .a3_xIndex[2]],
                            &xT, &stHit)) {
                        inv_INT_fn_vInsertSortedIntersection(
                            xNbMaxElements, p_xNbElements, d_stDataOfElement, p_stVertexA, 0,
                            &stHit, xElementIndex, xDataElementIndex);
                    }
                    break;
                case 7:
                    p_stElementSpheres = p_stGeomObj->d_stListOfElements[xElementIndex];
                    if (g_xMask) {
                        hMaterial = GEO_hGetGameMaterialOfIndexedSphere(p_stGeomObj, xElementIndex,
                                                                        xDataElementIndex);
                        if (!hMaterial)
                            break;
                        hCM = GMT_fn_hGetCollideMaterial(hMaterial);
                        if (hCM == (void*) 0xFFFFFFFF)
                            break;
                        if (!(GMT_fn_hGetCollideMaterialIdentifier(hCM) & g_xMask))
                            break;
                    }
                    p_stIndexedSphere = &p_stElementSpheres->d_stListOfSpheres[xDataElementIndex];
                    if (inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(
                            p_stVertexA, p_stVectAB,
                            &p_stGeomObj->d_stListOfPoints[p_stIndexedSphere->xCenterPoint], &xT,
                            &stHit, &stNormal, p_stIndexedSphere->xRadius)) {
                        inv_INT_fn_vInsertSortedIntersection(
                            xNbMaxElements, p_xNbElements, d_stDataOfElement, p_stVertexA, 0,
                            &stHit, xElementIndex, xDataElementIndex);
                    }
                    break;
                case 8:
                    p_stElementAlignedBoxes = p_stGeomObj->d_stListOfElements[xElementIndex];
                    if (g_xMask) {
                        hMaterial = GEO_vGetGameMaterialOfIndexedAlignedBox(
                            p_stGeomObj, xElementIndex, xDataElementIndex);
                        if (!hMaterial)
                            break;
                        hCM = GMT_fn_hGetCollideMaterial(hMaterial);
                        if (hCM == (void*) 0xFFFFFFFF)
                            break;
                        if (!(GMT_fn_hGetCollideMaterialIdentifier(hCM) & g_xMask))
                            break;
                    }
                    p_stIndexedAlignedBox =
                        &p_stElementAlignedBoxes->d_stListOfAlignedBoxes[xDataElementIndex];
                    if (INT_fn_bIntersectSegmentWithBox(
                            p_stVertexA, p_stVertexB, p_stVectAB,
                            &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMinPoint],
                            &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMaxPoint], &xT,
                            &stHit, &stNormal)) {
                        inv_INT_fn_vInsertSortedIntersection(
                            xNbMaxElements, p_xNbElements, d_stDataOfElement, p_stVertexA, 0,
                            &stHit, xElementIndex, xDataElementIndex);
                    }
                    break;
                }
                COL_g_d_lTaggedFacesTable[p_stGeomObj->p_stOctree
                                              ->d_xElementBasesTable[xElementIndex] +
                                          xDataElementIndex] = COL_g_lFacesTagCounter;
            }
        }
    }
    return *p_xNbElements > 0;
}

u8 INT_fn_bDetectIntersectSegmentWithFaceOfGeometricObject(MTH3D_tdstVector_* p_stVertexA,
                                                            MTH3D_tdstVector_* p_stVectAB,
                                                            COL_tdstCollideObject_* p_stGeomObj)
{
    s16 xElementIndex;
    COL_tdstElementIndexedTriangles_* p_stElementIndexedTriangles;
    COL_tdstElementSpheres_* p_stElementSpheres;
    COL_tdstElementAlignedBoxes_* p_stElementAlignedBoxes;
    s16 xDataElementIndex;
    COL_tdstTripledIndex_* p_stTriangle;
    COL_tdstIndexedSphere_* p_stIndexedSphere;
    COL_tdstIndexedAlignedBox_* p_stIndexedAlignedBox;
    void* hCollidedCollideMaterial;
    s16 xNbElements;
    u8 xIndex;
    u8* p_xListIndex;
    s16 xNumberOfSelectedNodes;
    s32 xNbFaces;
    COL_tdstOctreeNode_* aDEF_pstSelectedNode[48];
    f32 aDEF_xSelectedT[48];

    if (!p_stGeomObj->p_stOctree) {
        for (xElementIndex = 0; xElementIndex < p_stGeomObj->xNbElements; xElementIndex++) {
            switch (p_stGeomObj->d_xListOfElementsTypes[xElementIndex]) {
            case 1:
                p_stElementIndexedTriangles = p_stGeomObj->d_stListOfElements[xElementIndex];
                hCollidedCollideMaterial =
                    GMT_fn_hGetCollideMaterial(p_stElementIndexedTriangles->hMaterial);
                if (hCollidedCollideMaterial == (void*) 0xFFFFFFFF ||
                    !(GMT_fn_hGetCollideMaterialIdentifier(hCollidedCollideMaterial) & 0xC000)) {
                    for (xDataElementIndex = 0;
                         xDataElementIndex < p_stElementIndexedTriangles->xNbFaces;
                         xDataElementIndex++) {
                        if (INT_fn_bDetectIntersectSegmentWithTriangle(
                                p_stVertexA, p_stVectAB,
                                &p_stGeomObj->d_stListOfPoints
                                     [p_stElementIndexedTriangles
                                          ->d_stListOfFacesTripled[xDataElementIndex]
                                          .a3_xIndex[0]],
                                &p_stGeomObj->d_stListOfPoints
                                     [p_stElementIndexedTriangles
                                          ->d_stListOfFacesTripled[xDataElementIndex]
                                          .a3_xIndex[1]],
                                &p_stGeomObj->d_stListOfPoints
                                     [p_stElementIndexedTriangles
                                          ->d_stListOfFacesTripled[xDataElementIndex]
                                          .a3_xIndex[2]])) {
                            return 1;
                        }
                    }
                }
                break;
            case 7:
                p_stElementSpheres = p_stGeomObj->d_stListOfElements[xElementIndex];
                for (xDataElementIndex = 0; xDataElementIndex < p_stElementSpheres->xNbSpheres;
                     xDataElementIndex++) {
                    p_stIndexedSphere = &p_stElementSpheres->d_stListOfSpheres[xDataElementIndex];
                    hCollidedCollideMaterial =
                        GMT_fn_hGetCollideMaterial(p_stIndexedSphere->hMaterial);
                    if (hCollidedCollideMaterial == (void*) 0xFFFFFFFF ||
                        !(GMT_fn_hGetCollideMaterialIdentifier(hCollidedCollideMaterial) &
                          0xC000)) {
                        if (INT_fn_bDetectIntersectSegmentWithSphere(
                                p_stVertexA, p_stVectAB,
                                &p_stGeomObj->d_stListOfPoints[p_stIndexedSphere->xCenterPoint],
                                p_stIndexedSphere->xRadius)) {
                            return 1;
                        }
                    }
                }
                break;
            case 8:
                p_stElementAlignedBoxes = p_stGeomObj->d_stListOfElements[xElementIndex];
                for (xDataElementIndex = 0;
                     xDataElementIndex < p_stElementAlignedBoxes->xNbAlignedBoxes;
                     xDataElementIndex++) {
                    p_stIndexedAlignedBox =
                        &p_stElementAlignedBoxes->d_stListOfAlignedBoxes[xDataElementIndex];
                    hCollidedCollideMaterial =
                        GMT_fn_hGetCollideMaterial(p_stIndexedAlignedBox->hMaterial);
                    if (hCollidedCollideMaterial == (void*) 0xFFFFFFFF ||
                        !(GMT_fn_hGetCollideMaterialIdentifier(hCollidedCollideMaterial) &
                          0xC000)) {
                        if (INT_fn_bDetectIntersectSegmentWithBox(
                                p_stVertexA, p_stVectAB,
                                &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMinPoint],
                                &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMaxPoint],
                                0)) {
                            return 1;
                        }
                    }
                }
                break;
            }
        }
    } else {
        _COL_fn_vExploreOctreeWithSegment(p_stGeomObj->p_stOctree, p_stVertexA, p_stVectAB,
                                          aDEF_pstSelectedNode, aDEF_xSelectedT,
                                          &xNumberOfSelectedNodes);
        COL_g_lFacesTagCounter++;
        for (xNbElements = 0; xNbElements < xNumberOfSelectedNodes; xNbElements++) {
            s16* p_xNodeIndex = aDEF_pstSelectedNode[xNbElements]->d_xFaceIndexList;
            xNbFaces = *p_xNodeIndex++;
            p_xListIndex = (u8*) p_xNodeIndex;
            for (xIndex = 0; xIndex < xNbFaces; xIndex++) {
                if ((s8) *p_xListIndex >= 0) {
                    xElementIndex = *p_xListIndex++;
                } else {
                    xElementIndex = *p_xListIndex++ << 8;
                    xElementIndex = (xElementIndex + *p_xListIndex++) & 0x7FFF;
                }
                if ((s8) *p_xListIndex >= 0) {
                    xDataElementIndex = *p_xListIndex++;
                } else {
                    xDataElementIndex = *p_xListIndex++ << 8;
                    xDataElementIndex = (xDataElementIndex + *p_xListIndex++) & 0x7FFF;
                }
                if (COL_g_d_lTaggedFacesTable[p_stGeomObj->p_stOctree
                                                  ->d_xElementBasesTable[xElementIndex] +
                                              xDataElementIndex] != COL_g_lFacesTagCounter) {
                    switch (p_stGeomObj->d_xListOfElementsTypes[xElementIndex]) {
                    case 1:
                        p_stElementIndexedTriangles =
                            p_stGeomObj->d_stListOfElements[xElementIndex];
                        hCollidedCollideMaterial =
                            GMT_fn_hGetCollideMaterial(p_stElementIndexedTriangles->hMaterial);
                        if (hCollidedCollideMaterial == (void*) 0xFFFFFFFF ||
                            !(GMT_fn_hGetCollideMaterialIdentifier(hCollidedCollideMaterial) &
                              0xC000)) {
                            if (INT_fn_bDetectIntersectSegmentWithTriangle(
                                    p_stVertexA, p_stVectAB,
                                    &p_stGeomObj->d_stListOfPoints
                                         [p_stElementIndexedTriangles
                                              ->d_stListOfFacesTripled[xDataElementIndex]
                                              .a3_xIndex[0]],
                                    &p_stGeomObj->d_stListOfPoints
                                         [p_stElementIndexedTriangles
                                              ->d_stListOfFacesTripled[xDataElementIndex]
                                              .a3_xIndex[1]],
                                    &p_stGeomObj->d_stListOfPoints
                                         [p_stElementIndexedTriangles
                                              ->d_stListOfFacesTripled[xDataElementIndex]
                                              .a3_xIndex[2]])) {
                                return 1;
                            }
                        }
                        break;
                    case 7:
                        p_stElementSpheres = p_stGeomObj->d_stListOfElements[xElementIndex];
                        p_stIndexedSphere =
                            &p_stElementSpheres->d_stListOfSpheres[xDataElementIndex];
                        hCollidedCollideMaterial =
                            GMT_fn_hGetCollideMaterial(p_stIndexedSphere->hMaterial);
                        if (hCollidedCollideMaterial == (void*) 0xFFFFFFFF ||
                            !(GMT_fn_hGetCollideMaterialIdentifier(hCollidedCollideMaterial) &
                              0xC000)) {
                            if (INT_fn_bDetectIntersectSegmentWithSphere(
                                    p_stVertexA, p_stVectAB,
                                    &p_stGeomObj->d_stListOfPoints[p_stIndexedSphere->xCenterPoint],
                                    p_stIndexedSphere->xRadius)) {
                                return 1;
                            }
                        }
                        break;
                    case 8:
                        p_stElementAlignedBoxes = p_stGeomObj->d_stListOfElements[xElementIndex];
                        p_stIndexedAlignedBox =
                            &p_stElementAlignedBoxes->d_stListOfAlignedBoxes[xDataElementIndex];
                        hCollidedCollideMaterial =
                            GMT_fn_hGetCollideMaterial(p_stIndexedAlignedBox->hMaterial);
                        if (hCollidedCollideMaterial == (void*) 0xFFFFFFFF ||
                            !(GMT_fn_hGetCollideMaterialIdentifier(hCollidedCollideMaterial) &
                              0xC000)) {
                            if (INT_fn_bDetectIntersectSegmentWithBox(
                                    p_stVertexA, p_stVectAB,
                                    &p_stGeomObj
                                         ->d_stListOfPoints[p_stIndexedAlignedBox->xMinPoint],
                                    &p_stGeomObj
                                         ->d_stListOfPoints[p_stIndexedAlignedBox->xMaxPoint],
                                    0)) {
                                return 1;
                            }
                        }
                        break;
                    }
                    COL_g_d_lTaggedFacesTable[p_stGeomObj->p_stOctree
                                                  ->d_xElementBasesTable[xElementIndex] +
                                              xDataElementIndex] = COL_g_lFacesTagCounter;
                }
            }
        }
    }
    return 0;
}

void func_00210A28(s16 xNbMaxElements, s16* p_xNbElements, IntersectionPair* pairs, s32 unused,
                   s16 x, s16 y)
{
    s32 xIndex = *p_xNbElements;

    if (xIndex < xNbMaxElements) {
        pairs[xIndex].x = x;
        pairs[xIndex].y = y;
        if (*p_xNbElements < xNbMaxElements) {
            (*p_xNbElements)++;
        }
    }
}

s32 inv_INT_fn_bDetectIntersectPointWithGeomObj(MTH3D_tdstVector_* p_stPoint,
                                                COL_tdstCollideObject_* p_stGeomObj)
{
    MTH3D_tdstVector_ stHit;
    MTH3D_tdstVector_ stZNormal = qword_390208;
    f32 xT;
    s16 xElementIndex;
    s16 xDataElementIndex;

    for (xElementIndex = 0; xElementIndex < p_stGeomObj->xNbElements; xElementIndex++) {
        switch (p_stGeomObj->d_xListOfElementsTypes[xElementIndex]) {
        case 1: {
            COL_tdstElementIndexedTriangles_* p_stElementIndexedTriangles =
                p_stGeomObj->d_stListOfElements[xElementIndex];
            s16 xNbIntersect = 0;

            for (xDataElementIndex = 0; xDataElementIndex < p_stElementIndexedTriangles->xNbFaces;
                 xDataElementIndex++) {
                if (inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
                        p_stPoint, &stZNormal,
                        &p_stGeomObj
                             ->d_stListOfPoints[p_stElementIndexedTriangles
                                                    ->d_stListOfFacesTripled[xDataElementIndex]
                                                    .a3_xIndex[0]],
                        &p_stGeomObj
                             ->d_stListOfPoints[p_stElementIndexedTriangles
                                                    ->d_stListOfFacesTripled[xDataElementIndex]
                                                    .a3_xIndex[1]],
                        &p_stGeomObj
                             ->d_stListOfPoints[p_stElementIndexedTriangles
                                                    ->d_stListOfFacesTripled[xDataElementIndex]
                                                    .a3_xIndex[2]],
                        &xT, &stHit)) {
                    xNbIntersect++;
                }
            }
            if (xNbIntersect & 1) {
                return 1;
            }
            break;
        }
        case 7: {
            COL_tdstElementSpheres_* p_stElementSpheres =
                p_stGeomObj->d_stListOfElements[xElementIndex];

            for (xDataElementIndex = 0; xDataElementIndex < p_stElementSpheres->xNbSpheres;
                 xDataElementIndex++) {
                COL_tdstIndexedSphere_* p_stIndexedSphere =
                    &p_stElementSpheres->d_stListOfSpheres[xDataElementIndex];

                if (INT_fn_bGetInclusionPointInSphere(
                        p_stPoint, &p_stGeomObj->d_stListOfPoints[p_stIndexedSphere->xCenterPoint],
                        p_stIndexedSphere->xRadius)) {
                    return 1;
                }
            }
            break;
        }
        case 8: {
            COL_tdstElementAlignedBoxes_* p_stElementAlignedBoxes =
                p_stGeomObj->d_stListOfElements[xElementIndex];

            for (xDataElementIndex = 0;
                 xDataElementIndex < p_stElementAlignedBoxes->xNbAlignedBoxes;
                 xDataElementIndex++) {
                COL_tdstIndexedAlignedBox_* p_stIndexedAlignedBox =
                    &p_stElementAlignedBoxes->d_stListOfAlignedBoxes[xDataElementIndex];

                if (INT_fn_bGetInclusionPointInBox(
                        &p_stPoint->xX,
                        &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMinPoint].xX,
                        &p_stGeomObj->d_stListOfPoints[p_stIndexedAlignedBox->xMaxPoint].xX)) {
                    return 1;
                }
            }
            break;
        }
        }
    }
    return 0;
}

extern COL_tdstDoubledIndex_ COL_g_a12_stBoxEdges[12];

u8 INT_fn_bDetectIntersectGeomObjWithGeomObj(COL_tdstCollideObject_* p_stGeomObj1,
                                              POS_tdstMatrix_* _p_stMatrix1,
                                              COL_tdstCollideObject_* p_stGeomObj2,
                                              POS_tdstMatrix_* _p_stMatrix2, GMT_tdstGameMaterial_** _p_hMat1,
                                              GMT_tdstGameMaterial_** _p_hMat2)
{
    POS_tdstMatrix_ stInvMatrix2 __attribute__((aligned(16)));
    POS_tdstMatrix_ st21TransformMatrix __attribute__((aligned(16)));
    POS_tdstMatrix_ st12TransformMatrix __attribute__((aligned(16)));
    MTH3D_tdstVector_ stCenter1in2;
    MTH3D_tdstVector_ stCenter2in1;
    MTH3D_tdstVector_ stCenter1;
    MTH3D_tdstVector_ stCenter2;
    MTH3D_tdstVector_ a8_st8VBox2[8];
    MTH3D_tdstVector_ a8_st8VBox2in1[8];
    MTH3D_tdstVector_ a8_st8VBox1[8];
    MTH3D_tdstVector_ a8_st8VBox1in2[8];
    MTH3D_tdstVector_ stEdgeVector;
    s16 xElementIndex1;
    s16 xDataElementIndex1;
    COL_tdstElementSpheres_* p_stElementSpheres1;
    COL_tdstElementAlignedBoxes_* p_stElementAlignedBoxes1;
    COL_tdstIndexedSphere_* p_stIndexedSphere1;
    COL_tdstIndexedAlignedBox_* p_stIndexedAlignedBox1;
    s16 xElementIndex2;
    s16 xDataElementIndex2;
    COL_tdstElementSpheres_* p_stElementSpheres2;
    COL_tdstElementAlignedBoxes_* p_stElementAlignedBoxes2;
    COL_tdstIndexedSphere_* p_stIndexedSphere2;
    COL_tdstIndexedAlignedBox_* p_stIndexedAlignedBox2;
    s16 xVertexIndex;
    s16 xEdgeIndex;
    f32 xScale1;
    f32 xScale2;

    xScale1 = POS_fn_xGetMaxScale(_p_stMatrix1);
    xScale2 = POS_fn_xGetMaxScale(_p_stMatrix2);

    for (xElementIndex1 = 0; xElementIndex1 < p_stGeomObj1->xNbElements; xElementIndex1++) {
        switch (p_stGeomObj1->d_xListOfElementsTypes[xElementIndex1]) {
        case 7:
            p_stElementSpheres1 = p_stGeomObj1->d_stListOfElements[xElementIndex1];
            for (xDataElementIndex1 = 0; xDataElementIndex1 < p_stElementSpheres1->xNbSpheres;
                 xDataElementIndex1++) {
                p_stIndexedSphere1 = &p_stElementSpheres1->d_stListOfSpheres[xDataElementIndex1];
                POS_fn_vMulMatrixVertex(
                    &stCenter1, _p_stMatrix1,
                    &p_stGeomObj1->d_stListOfPoints[p_stIndexedSphere1->xCenterPoint]);
                for (xElementIndex2 = 0; xElementIndex2 < p_stGeomObj2->xNbElements;
                     xElementIndex2++) {
                    switch (p_stGeomObj2->d_xListOfElementsTypes[xElementIndex2]) {
                    case 7:
                        p_stElementSpheres2 = p_stGeomObj2->d_stListOfElements[xElementIndex2];
                        for (xDataElementIndex2 = 0;
                             xDataElementIndex2 < p_stElementSpheres2->xNbSpheres;
                             xDataElementIndex2++) {
                            p_stIndexedSphere2 =
                                &p_stElementSpheres2->d_stListOfSpheres[xDataElementIndex2];
                            POS_fn_vMulMatrixVertex(
                                &stCenter2, _p_stMatrix2,
                                &p_stGeomObj2->d_stListOfPoints[p_stIndexedSphere2->xCenterPoint]);
                            if (INT_fn_bIntersectSphereWithSphere(
                                    &stCenter1, &stCenter2, p_stIndexedSphere1->xRadius * xScale1,
                                    p_stIndexedSphere2->xRadius * xScale2)) {
                                *_p_hMat1 = p_stIndexedSphere1->hMaterial;
                                *_p_hMat2 = p_stIndexedSphere2->hMaterial;
                                return 1;
                            }
                        }
                        break;
                    case 8:
                        POS_fn_vInvertMatrix(&stInvMatrix2, _p_stMatrix2);
                        _POS_fn_vMulMatrixMatrix(&st21TransformMatrix, &stInvMatrix2, _p_stMatrix1);
                        POS_fn_vMulMatrixVertex(
                            &stCenter1in2, &st21TransformMatrix,
                            &p_stGeomObj1->d_stListOfPoints[p_stIndexedSphere1->xCenterPoint]);
                        p_stElementAlignedBoxes2 = p_stGeomObj2->d_stListOfElements[xElementIndex2];
                        for (xDataElementIndex2 = 0;
                             xDataElementIndex2 < p_stElementAlignedBoxes2->xNbAlignedBoxes;
                             xDataElementIndex2++) {
                            p_stIndexedAlignedBox2 =
                                &p_stElementAlignedBoxes2
                                     ->d_stListOfAlignedBoxes[xDataElementIndex2];
                            if (INT_fn_bIntersectSphereWithBox(
                                    &stCenter1in2,
                                    &p_stGeomObj2
                                         ->d_stListOfPoints[p_stIndexedAlignedBox2->xMinPoint],
                                    &p_stGeomObj2
                                         ->d_stListOfPoints[p_stIndexedAlignedBox2->xMaxPoint],
                                    p_stIndexedSphere1->xRadius * xScale1 / xScale2)) {
                                *_p_hMat1 = p_stIndexedSphere1->hMaterial;
                                *_p_hMat2 = p_stIndexedAlignedBox2->hMaterial;
                                return 1;
                            }
                        }
                        break;
                    }
                }
            }
            break;
        case 8:
            POS_fn_vInvertMatrix(&stInvMatrix2, _p_stMatrix2);
            _POS_fn_vMulMatrixMatrix(&st21TransformMatrix, &stInvMatrix2, _p_stMatrix1);
            POS_fn_vInvertMatrix(&st12TransformMatrix, &st21TransformMatrix);
            p_stElementAlignedBoxes1 = p_stGeomObj1->d_stListOfElements[xElementIndex1];
            for (xDataElementIndex1 = 0;
                 xDataElementIndex1 < p_stElementAlignedBoxes1->xNbAlignedBoxes;
                 xDataElementIndex1++) {
                p_stIndexedAlignedBox1 =
                    &p_stElementAlignedBoxes1->d_stListOfAlignedBoxes[xDataElementIndex1];
                for (xElementIndex2 = 0; xElementIndex2 < p_stGeomObj2->xNbElements;
                     xElementIndex2++) {
                    switch (p_stGeomObj2->d_xListOfElementsTypes[xElementIndex2]) {
                    case 7:
                        p_stElementSpheres2 = p_stGeomObj2->d_stListOfElements[xElementIndex2];
                        for (xDataElementIndex2 = 0;
                             xDataElementIndex2 < p_stElementSpheres2->xNbSpheres;
                             xDataElementIndex2++) {
                            p_stIndexedSphere2 =
                                &p_stElementSpheres2->d_stListOfSpheres[xDataElementIndex2];
                            POS_fn_vMulMatrixVertex(
                                &stCenter2in1, &st12TransformMatrix,
                                &p_stGeomObj2->d_stListOfPoints[p_stIndexedSphere2->xCenterPoint]);
                            if (INT_fn_bIntersectSphereWithBox(
                                    &stCenter2in1,
                                    &p_stGeomObj1
                                         ->d_stListOfPoints[p_stIndexedAlignedBox1->xMinPoint],
                                    &p_stGeomObj1
                                         ->d_stListOfPoints[p_stIndexedAlignedBox1->xMaxPoint],
                                    p_stIndexedSphere2->xRadius * xScale2 / xScale1)) {
                                *_p_hMat1 = p_stIndexedAlignedBox1->hMaterial;
                                *_p_hMat2 = p_stIndexedSphere2->hMaterial;
                                return 1;
                            }
                        }
                        break;
                    case 8:
                        p_stElementAlignedBoxes2 = p_stGeomObj2->d_stListOfElements[xElementIndex2];
                        for (xDataElementIndex2 = 0;
                             xDataElementIndex2 < p_stElementAlignedBoxes2->xNbAlignedBoxes;
                             xDataElementIndex2++) {
                            p_stIndexedAlignedBox2 =
                                &p_stElementAlignedBoxes2
                                     ->d_stListOfAlignedBoxes[xDataElementIndex2];
                            COL_fn_vMinMaxBox2VerticesBox(
                                a8_st8VBox2,
                                &p_stGeomObj2->d_stListOfPoints[p_stIndexedAlignedBox2->xMinPoint],
                                &p_stGeomObj2->d_stListOfPoints[p_stIndexedAlignedBox2->xMaxPoint]);
                            for (xVertexIndex = 0; xVertexIndex < 8; xVertexIndex++) {
                                POS_fn_vMulMatrixVertex(&a8_st8VBox2in1[xVertexIndex],
                                                         &st12TransformMatrix,
                                                         &a8_st8VBox2[xVertexIndex]);
                            }
                            for (xEdgeIndex = 0; xEdgeIndex < 12; xEdgeIndex++) {
                                stEdgeVector.xX =
                                    (&a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[1]])
                                        ->xX -
                                    (&a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[0]])
                                        ->xX;
                                stEdgeVector.xY =
                                    (&a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[1]])
                                        ->xY -
                                    (&a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[0]])
                                        ->xY;
                                stEdgeVector.xZ =
                                    (&a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[1]])
                                        ->xZ -
                                    (&a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[0]])
                                        ->xZ;
                                if (INT_fn_bDetectIntersectSegmentWithBox(
                                        &a8_st8VBox2in1[COL_g_a12_stBoxEdges[xEdgeIndex]
                                                            .a2_xIndex[0]],
                                        &stEdgeVector,
                                        &p_stGeomObj1
                                             ->d_stListOfPoints[p_stIndexedAlignedBox1->xMinPoint],
                                        &p_stGeomObj1
                                             ->d_stListOfPoints[p_stIndexedAlignedBox1->xMaxPoint],
                                        0)) {
                                    *_p_hMat1 = p_stIndexedAlignedBox1->hMaterial;
                                    *_p_hMat2 = p_stIndexedAlignedBox2->hMaterial;
                                    return 1;
                                }
                            }
                            COL_fn_vMinMaxBox2VerticesBox(
                                a8_st8VBox1,
                                &p_stGeomObj1->d_stListOfPoints[p_stIndexedAlignedBox1->xMinPoint],
                                &p_stGeomObj1->d_stListOfPoints[p_stIndexedAlignedBox1->xMaxPoint]);
                            for (xVertexIndex = 0; xVertexIndex < 8; xVertexIndex++) {
                                POS_fn_vMulMatrixVertex(&a8_st8VBox1in2[xVertexIndex],
                                                         &st21TransformMatrix,
                                                         &a8_st8VBox1[xVertexIndex]);
                            }
                            for (xEdgeIndex = 0; xEdgeIndex < 12; xEdgeIndex++) {
                                stEdgeVector.xX =
                                    (&a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[1]])
                                        ->xX -
                                    (&a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[0]])
                                        ->xX;
                                stEdgeVector.xY =
                                    (&a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[1]])
                                        ->xY -
                                    (&a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[0]])
                                        ->xY;
                                stEdgeVector.xZ =
                                    (&a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[1]])
                                        ->xZ -
                                    (&a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex].a2_xIndex[0]])
                                        ->xZ;
                                if (INT_fn_bDetectIntersectSegmentWithBox(
                                        &a8_st8VBox1in2[COL_g_a12_stBoxEdges[xEdgeIndex]
                                                            .a2_xIndex[0]],
                                        &stEdgeVector,
                                        &p_stGeomObj2
                                             ->d_stListOfPoints[p_stIndexedAlignedBox2->xMinPoint],
                                        &p_stGeomObj2
                                             ->d_stListOfPoints[p_stIndexedAlignedBox2->xMaxPoint],
                                        0)) {
                                    *_p_hMat1 = p_stIndexedAlignedBox1->hMaterial;
                                    *_p_hMat2 = p_stIndexedAlignedBox2->hMaterial;
                                    return 1;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            break;
        }
    }
    return 0;
}
