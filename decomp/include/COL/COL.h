#ifndef REVO_COL_COL_H
#define REVO_COL_COL_H

#include "ps2/types.h"
#include "MTH/MTH.h"
#include "MEC/MEC.h"
#include "GMT/GMT.h"
#include "ps2/copy_types.h"

typedef struct COL_tdstBoundingSphere_ {
    MTH3D_tdstVector_ stCenterPoint;
    f32 xRadius;
} COL_tdstBoundingSphere_;

typedef struct COL_tdstParallelBox_ {
    MTH3D_tdstVector_ stMinPoint;
    MTH3D_tdstVector_ stMaxPoint;
} COL_tdstParallelBox_;

typedef struct COL_tdstDoubledIndex_ {
    s16 a2_xIndex[2];
} COL_tdstDoubledIndex_;

typedef struct COL_tdstTripledIndex_ {
    s16 a3_xIndex[3];
} COL_tdstTripledIndex_;

typedef struct COL_tdstElementIndexedTriangles_ {
    GMT_tdstGameMaterial_* hMaterial;
    s16 xNbFaces;
    s16 xIndexOfParallelBox;
    COL_tdstTripledIndex_* d_stListOfFacesTripled;
} COL_tdstElementIndexedTriangles_;

typedef struct COL_tdstIndexedSphere_ {
    f32 xRadius;
    GMT_tdstGameMaterial_* hMaterial;
    s16 xCenterPoint;
} COL_tdstIndexedSphere_;

typedef struct COL_tdstElementSpheres_ {
    COL_tdstIndexedSphere_* d_stListOfSpheres;
    s16 xNbSpheres;
    s16 xIndexOfParallelBox;
} COL_tdstElementSpheres_;

#define COL_ELEMENT_SPHERES_inv 7
#define COL_ELEMENT_ALIGNED_BOXES_inv 8

typedef struct COL_tdstIndexedAlignedBox_ {
    s16 xMinPoint;
    s16 xMaxPoint;
    GMT_tdstGameMaterial_* hMaterial;
} COL_tdstIndexedAlignedBox_;

typedef struct COL_tdstElementAlignedBoxes_ {
    COL_tdstIndexedAlignedBox_* d_stListOfAlignedBoxes;
    s16 xNbAlignedBoxes;
    s16 xIndexOfParallelBox;
} COL_tdstElementAlignedBoxes_;

typedef struct COL_tdstOctreeNode_ COL_tdstOctreeNode_;

struct COL_tdstOctreeNode_ {
    MTH3D_tdstVector_ stMinPart;
    MTH3D_tdstVector_ stMaxPart;
    COL_tdstOctreeNode_** p_pstChildList;
    s16* d_xFaceIndexList;
};

typedef struct COL_tdstOctree_ {
    COL_tdstOctreeNode_* p_stOctreeRoot;
    s16 xNbFaces;
    u16* d_xElementBasesTable;
} COL_tdstOctree_;

typedef struct COL_tdstCollideObject_ {
    s16 xNbPoints;
    s16 xNbElements;
    MTH3D_tdstVector_* d_stListOfPoints;
    s16* d_xListOfElementsTypes;
    void** d_stListOfElements;
    COL_tdstOctree_* p_stOctree;
} COL_tdstCollideObject_;

typedef struct COL_tdstCollisionCase_ {
    f32 xCollisionTime;
    MTH3D_tdstVector_ stCollisionNormal;
    MTH3D_tdstVector_ stCollisionPoint;
    GMT_tdstGameMaterial_* hDynamicMaterial;
    GMT_tdstGameMaterial_* hStaticMaterial;
    void* p_vParameter1;
    s32 sParameter2;
    s16 xDynamicGeomEntity;
    s16 xStaticGeomEntity;
    MTH3D_tdstVector_ stTranslation;
    MTH3D_tdstVector_ stMovement;
    MTH3D_tdstVector_ stEndPosition;
    f32 xSphereRadius;
    f32 m_xSlide1;
    f32 m_xRebound1;
    f32 m_xSlide2;
    f32 m_xRebound2;
} COL_tdstCollisionCase_;

#define OCTREE_MAX_HITS 0x2D

typedef struct COL_tdstGVForCollision_ {
    MTH3D_tdstVector_* p_stVertex1;
    MTH3D_tdstVector_ stEdgeVector;
    MTH3D_tdstVector_* p_stVertex2;
    MTH3D_tdstVector_ stDinST0Point;
    f32 xDynamicRadius;
    POS_tdstMatrix_* p_stStaticGeomObjMatrix;
    MTH3D_tdstVector_ stDinST1Point;
    MTH3D_tdstVector_ stDinST01Vector;
    GMT_tdstGameMaterial_* hDynamicMaterial;
    GMT_tdstGameMaterial_* hStaticMaterial;
    void* p_vParameter1;
    s16 sParameter2;
    COL_tdstCollideObject_* p_stStaticColObj;
    COL_tdstElementIndexedTriangles_* p_stStaticElementIndexedTriangles;
    u32 ulSelectedCollisionCases;
    s16 xStaticElementIndex;
    s16 xStaticIndexedTriangleIndex;
    COL_tdstIndexedSphere_* p_stDynamicIndexedSphere;
    COL_tdstOctree_* p_stOctree;
    u8 bStaticGeomObjHasNoTransformationMatrix;
    u8 bDynamicGeomObjHasZoomInsteadOfScale;
    COL_tdstCollideObject_* p_stDynamicColObj;
    POS_tdstMatrix_* p_stT0DynamicGeomObjMatrix;
    POS_tdstMatrix_* p_stT1DynamicGeomObjMatrix;
    POS_tdstMatrix_ stInvMatrix;
    POS_tdstMatrix_ stD2ST0TransformMatrix;
    POS_tdstMatrix_ stD2ST1TransformMatrix;
    POS_tdstMatrix_ stS2DT0TransformMatrix;
    POS_tdstMatrix_ stS2DT1TransformMatrix;
    f32 xStaticScale;
    COL_tdstElementSpheres_* p_stDynamicElementSpheres;
    COL_tdstElementSpheres_* p_stStaticElementSpheres;
    COL_tdstIndexedSphere_* p_stStaticIndexedSphere;
    MTH3D_tdstVector_ stSwapDinST0Point;
    MTH3D_tdstVector_* p_stDynamicCenter;
    MTH3D_tdstVector_* p_stStaticCenter;
    f32 xSwapRadius;
    u8 bUseEnlargeSphere;
    s16 xNumberOfSelectedNodes;
    COL_tdstOctreeNode_* aDEF_pstSelectedNode[OCTREE_MAX_HITS];
} COL_tdstGVForCollision_;

typedef struct GLI_tdstDataOfElement_ {
    MTH3D_tdstVector_ stHit;
    f32 xDistance;
    s16 xElements;
    s16 xIndexOfFace;
    s16 xIndexOfEdge;
    s16 xIndexOfPoint;
} GLI_tdstDataOfElement_;

typedef struct tdstCollideZdxList_ {
    void* hGeoObj;
} tdstCollideZdxList_;

typedef struct LST2_tdstAnchorToZDX_tdxHandleToGeoZdxListOpt {
    tdstCollideZdxList_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToZDX_tdxHandleToGeoZdxListOpt;

typedef struct tdstZdxList_ {
    LST2_tdstAnchorToZDX_tdxHandleToGeoZdxListOpt hGeoZdxList;
} tdstZdxList_;

typedef struct tdstZdxIndexList_ {
    u16 uwZdxIndex;
} tdstZdxIndexList_;

typedef struct LST2_tdstAnchorToZDX_tdxHandleToZdxIndexListOpt {
    tdstZdxIndexList_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToZDX_tdxHandleToZdxIndexListOpt;

typedef struct tdstZoneSetList_ {
    LST2_tdstAnchorToZDX_tdxHandleToZdxIndexListOpt hZdxIndexList;
} tdstZoneSetList_;

typedef struct tdstZoneSetEncapsulation_ {
    tdstZoneSetList_* p_stZoneSetList;
    u16 uwIndex;
} tdstZoneSetEncapsulation_;

typedef struct LST2_tdstAnchorToZDX_tdxHandleToZoneSetEncapsulationOpt {
    tdstZoneSetEncapsulation_* hFirstElementOpt;
    s32 lNumberOfElementsOpt;
} LST2_tdstAnchorToZDX_tdxHandleToZoneSetEncapsulationOpt;

typedef struct tdstCsaList_ {
    LST2_tdstAnchorToZDX_tdxHandleToZoneSetEncapsulationOpt hZoneSetEncapsulationList;
} tdstCsaList_;

typedef struct stColliderInfo_ {
    void* a_stColliderActors[2];
    MTH3D_tdstVector_ a_stColliderVectors[2];
    f32 a_xColliderReals[2];
    u8 ucColliderType;
    u8 ucColliderPriority;
    u8 a2_ucUnused[2];
} stColliderInfo_;

typedef struct CollisionSet {
    tdstZdxList_* hZddList;
    tdstZdxList_* hZdeList;
    tdstZdxList_* hZdmList;
    tdstZdxList_* hZdrList;
    tdstCsaList_* hZddActivationList;
    tdstCsaList_* hZdeActivationList;
    tdstCsaList_* hZdmActivationList;
    tdstCsaList_* hZdrActivationList;
    tdstZoneSetList_* hZddCurrentActivation;
    tdstZoneSetList_* hZdeCurrentActivation;
    tdstZoneSetList_* hZdrCurrentActivation;
    tdstZoneSetList_* hZdmCurrentActivation;
    u32 ulFBZddPriviligedZones;
    u32 ulFBZdePriviligedZones;
    u32 ulFBZdmPriviligedZones;
    u32 ulFBZdrPriviligedZones;
    u8 ucCollComputeFrequency;
    u8 ucCharacterPriority;
    u8 ucCollisionFlag;
    u8 pad43;
    stColliderInfo_ stColliderInfo;
} CollisionSet;

typedef struct OctreeSegment {
    MTH3D_tdstVector_ start;
    MTH3D_tdstVector_ end;
} OctreeSegment;

typedef struct OctreeNode {
    u8 axis;
    union {
        struct {
            f32 position;
            u16 negativeChild;
            u16 positiveChild;
        } split;
        u16* elementList;
    } u;
} OctreeNode;

typedef struct CollisionEntry {
    struct HIE_tdstSuperObject_* superObject;
    struct GeometricObjectView* geometry;
    u16 element;
    u16 bitCount;
    u32* bits;
    u8 queued;
} CollisionEntry;

typedef struct OctreeHit_inv {
    u16 entry;
    u16 triangle;
} OctreeHit_inv;

typedef struct Octree {
    u8 pad00[0x18];
    OctreeNode* nodes;
    u8 pad1C[8];
    CollisionEntry* entries;
} Octree;

typedef struct ScratchVector {
    MTH3D_tdstVector_ xyz;
    f32 w;
} ScratchVector;

typedef struct OctreeBox {
    u8 axisFlags;
    MTH3D_tdstVector_ center;
    MTH3D_tdstVector_ end;
    f32 extent;
} OctreeBox;

extern Octree inv_COL_stLevelOctree1;
extern Octree inv_COL_stLevelOctree2;
extern s32* COL_g_d_lTaggedFacesTable;

extern void inv_MEC_fn_vSplitAlongNormal(MTH3D_tdstVector_* vector, MTH3D_tdstVector_* tangent, MTH3D_tdstVector_* normalPart,
                          MTH3D_tdstVector_* normal);

extern void COL_fn_vMinMaxBox2VerticesBox(MTH3D_tdstVector_* a8_stBoxDest,
                                          MTH3D_tdstVector_* p_stMinPoint,
                                          MTH3D_tdstVector_* p_stMaxPoint);
extern u8 COL_fn_bDetectCollisionBetweenTwoParallelBoxes(MTH3D_tdstVector_* _p_stFirstBoxMaxPoint, MTH3D_tdstVector_* _p_stFirstBoxMinPoint, MTH3D_tdstVector_* _p_stSecondBoxMaxPoint, MTH3D_tdstVector_* _p_stSecondBoxMinPoint);
extern void COL_fn_vComputeBoundingBoxOfSphere(MTH3D_tdstVector_* p_stMinBounding, MTH3D_tdstVector_* p_stMaxBounding, MTH3D_tdstVector_* p_stSphereCenter, f32 xSphereRadius);

extern void inv_COL_RAY_fn_vFlushHitBuffer(void);
extern void inv_COL_RAY_fn_vInitHitBuffer(void);
extern void inv_COL_RAY_fn_vCastRayThroughSectors(COL_tdstGVForCollision_* p_stGV);

extern s32 INT_fn_bIntersectSegmentWithFaceOfGeometricObject(
    MTH3D_tdstVector_* p_stVertexA, MTH3D_tdstVector_* p_stVertexB, MTH3D_tdstVector_* p_stVectAB,
    COL_tdstCollideObject_* p_stGeomObj, s16 xNbMaxElements, s16* p_xNbElements,
    GLI_tdstDataOfElement_* d_stDataOfElement);
extern u8 INT_fn_bDetectIntersectSegmentWithFaceOfGeometricObject(MTH3D_tdstVector_* p_stVertexA,
                                                                  MTH3D_tdstVector_* p_stVectAB,
                                                                  COL_tdstCollideObject_* p_stGeomObj);
extern u8 INT_fn_bDetectIntersectGeomObjWithGeomObj(COL_tdstCollideObject_* p_stGeomObj1, POS_tdstMatrix_* _p_stMatrix1, COL_tdstCollideObject_* p_stGeomObj2, POS_tdstMatrix_* _p_stMatrix2, GMT_tdstGameMaterial_** _p_hMat1, GMT_tdstGameMaterial_** _p_hMat2);
extern s32 inv_INT_fn_bDetectIntersectPointWithGeomObj(MTH3D_tdstVector_* p_stPoint, COL_tdstCollideObject_* p_stGeomObj);

extern s32 INT_fn_bGetInclusionPointInBox(f32* p_stPoint, f32* p_stMinPoint, f32* p_stMaxPoint);
extern s32 INT_fn_bGetInclusionPointInSphere(MTH3D_tdstVector_* p_stSphereCenter,
                                             MTH3D_tdstVector_* p_stPoint, f32 xSphereRadius);
extern s32 INT_fn_bGetPointInTriangle(MTH3D_tdstVector_* p_stPoint, MTH3D_tdstVector_* p_stVertex0,
                                      MTH3D_tdstVector_* p_stVertex1,
                                      MTH3D_tdstVector_* p_stVertex2,
                                      MTH3D_tdstVector_* p_stNormal);
extern s32 INT_fn_bIntersectSegmentWithBox(MTH3D_tdstVector_* p_stVertex1,
                                           MTH3D_tdstVector_* p_stVertex2,
                                           MTH3D_tdstVector_* p_stVect12,
                                           MTH3D_tdstVector_* p_stMinPoint,
                                           MTH3D_tdstVector_* p_stMaxPoint, f32* p_xT,
                                           MTH3D_tdstVector_* p_stHit,
                                           MTH3D_tdstVector_* p_stNormal);
extern s32 INT_fn_bIntersectBoxWithBox(MTH3D_tdstVector_* p_stMinPoint1,
                                       MTH3D_tdstVector_* p_stMaxPoint1,
                                       MTH3D_tdstVector_* p_stMinPoint2,
                                       MTH3D_tdstVector_* p_stMaxPoint2);
extern s32 inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(MTH3D_tdstVector_* origin,
                                                                  MTH3D_tdstVector_* direction,
                                                                  MTH3D_tdstVector_* a,
                                                                  MTH3D_tdstVector_* b,
                                                                  MTH3D_tdstVector_* c,
                                                                  f32* parameter,
                                                                  MTH3D_tdstVector_* point);
extern s32 INT_fn_bDetectIntersectSegmentWithBox(MTH3D_tdstVector_* p_stVertex1,
                                                 MTH3D_tdstVector_* p_stVect12,
                                                 MTH3D_tdstVector_* p_stMinPoint,
                                                 MTH3D_tdstVector_* p_stMaxPoint, f32* _p_xT);
extern s32 INT_fn_bIntersectSphereWithSphere(MTH3D_tdstVector_* p_stCenter1,
                                             MTH3D_tdstVector_* p_stCenter2, f32 xRadius1,
                                             f32 xRadius2);
extern s32 INT_fn_bDetectIntersectSegmentWithTriangle(MTH3D_tdstVector_* p_stVertexA, MTH3D_tdstVector_* p_stVectAB, MTH3D_tdstVector_* p_stVertex1, MTH3D_tdstVector_* p_stVertex2, MTH3D_tdstVector_* p_stVertex3);
extern u8 INT_fn_bDetectIntersectSegmentWithSphere(MTH3D_tdstVector_* p_stVertexA, MTH3D_tdstVector_* p_stVectAB, MTH3D_tdstVector_* p_stSphereCenter, f32 xSphereRadius);
extern u8 INT_fn_bIntersectSphereWithBox(MTH3D_tdstVector_* p_stCenter, MTH3D_tdstVector_* p_stMinPoint, MTH3D_tdstVector_* p_stMaxPoint, f32 xRadius);

extern s32 inv_COL_fn_bIntersectSegmentWithLevel(OctreeSegment* segment);
extern s32 inv_COL_fn_bIntersectSegmentWithLeaf(OctreeNode* leaf, OctreeSegment* segment);
extern void COL_fn_vExploreRecursiveOctreeWithSegment(u16 index, OctreeSegment* segment);
extern void COL_fn_vExploreOctreeWithBox(OctreeBox* box, OctreeHit_inv* out, u32* count, u32 max,
                                         CollisionEntry** entries);
extern s32 inv_COL_fn_ulClipSegmentWithSplitPlane(OctreeSegment* segment, OctreeNode* node, MTH3D_tdstVector_* hit);

extern void _COL_fn_vExploreOctreeWithSegment(COL_tdstOctree_* p_stOctree,
                                              MTH3D_tdstVector_* p_stVertexA,
                                              MTH3D_tdstVector_* p_stVectAB,
                                              COL_tdstOctreeNode_** d_pstSelectedNode,
                                              f32* d_xSelectedT, s16* p_xNumberOfSelectedNodes);

struct SphereQuery;
extern s32 inv_COL_fn_bIntersectSegmentWithNode(u16 index, OctreeSegment* segment);
extern void inv_COL_fn_vExploreNodeWithQuery(u16 index, struct SphereQuery* query);

extern s32 COL_g_lFacesTagCounter;

extern s32 func_0013F5C0(EffectParticle_inv* particle, EffectSystem_inv* list,
                         CollisionNode** outNode, f32* outDistance, MTH3D_tdstVector_* segment,
                         MTH3D_tdstVector_* outHit);
extern void func_0013F788(EffectParticle_inv* particle, EffectSystem_inv* list);

extern s32 inv_INT_fn_bIntersectSegmentWithTriangleBackFace(EffectParticle_inv* object,
                                                            MTH3D_tdstVector_* point,
                                                            CollisionShape* shape,
                                                            f32* outDistance,
                                                            MTH3D_tdstVector_* outNormal);
extern s32 inv_INT_fn_bIntersectSegmentWithTriangleFrontFace(EffectParticle_inv* object,
                                                             MTH3D_tdstVector_* point,
                                                             CollisionShape* shape,
                                                             f32* outDistance,
                                                             MTH3D_tdstVector_* outNormal);

struct IntersectionSphere;
extern s32 inv_INT_fn_bIntersectSegmentWithSphere(EffectParticle_inv* particle,
                                                  MTH3D_tdstVector_* point,
                                                  struct IntersectionSphere* sphere,
                                                  f32* outDistance, MTH3D_tdstVector_* outVector);

extern u8 inv_COL_bRayStaticMatrixChanged;

extern COL_tdstCollisionCase_* COL_g_stCollisionCaseReal;
extern s32 g_NbCollidedEdges;

#endif
