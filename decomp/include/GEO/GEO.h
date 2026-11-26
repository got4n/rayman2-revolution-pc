#ifndef REVO_GEO_GEO_H
#define REVO_GEO_GEO_H

#include "ps2/types.h"

typedef struct GEO_tdstColor_ {
    f32 xR;
    f32 xG;
    f32 xB;
    f32 xA;
} GEO_tdstColor_;

typedef struct GEO_tdstTripledIndex_ {
    s16 a3_xIndex[3];
} GEO_tdstTripledIndex_;

struct POS_tdstMatrix_;
struct MTH3D_tdstVector_;
struct GLD_tdstViewportAttributes_;
extern f32 GEO_xGetZFarTransparencyLevel(void);
extern void GEO_vSetZFarTransparencyZone(f32 _xZ);
extern void GEO_fn_vComputeCameraParametersForCulling(struct GLD_tdstViewportAttributes_* p_stVpt,
                                                      f32 _xZFar);
extern s32 GEO_lCullingBox(struct GLD_tdstViewportAttributes_* p_stVpt, struct MTH3D_tdstVector_* p_stMin, struct MTH3D_tdstVector_* p_stMax, s32 _lCullingResult);
extern s32 GEO_lCullingSphere(struct GLD_tdstViewportAttributes_* viewport, struct MTH3D_tdstVector_* p_stSphereCenter, f32 xRadius, struct POS_tdstMatrix_* objectMatrix, s32 _lCullingResult);

struct MTH3D_tdstVector_;
struct COL_tdstCollideObject_;
extern struct COL_tdstCollideObject_*
GEO_fn_hTransformTableOfObjectInOneObjectForGAM(struct COL_tdstCollideObject_* target,
                                                struct COL_tdstCollideObject_** objects,
                                                s16 objectCount);
extern f32 inv_GEO_fn_xGetField0C(void* object);
extern void* inv_GEO_fn_pIdentity(void* value);
extern void* COL_fn_pGetMinPointOfParallelBox(void* _p_stParallelBox);
extern s16 GEO_xGetGeometricObjectNumberOfElementsMax(struct COL_tdstCollideObject_* object);
extern void gu__GEO_vGetPointOfObject(struct MTH3D_tdstVector_* point, f32* radius,
                                      struct COL_tdstCollideObject_* object);
extern void* COL_fn_pGetMaxPointOfParallelBox(struct MTH3D_tdstVector_* _p_stParallelBox);
extern s32 GEO_xGetElementType(struct COL_tdstCollideObject_* object, s32 element);
extern void GEO_vSetPointOfObject(struct COL_tdstCollideObject_* hObject, struct MTH3D_tdstVector_* p_stPoint, s16 xIndexOfPoint);
extern void GEO_vGetPointOfObject(struct COL_tdstCollideObject_* hObject, struct MTH3D_tdstVector_* p_stPoint, s16 xIndexOfPoint);
extern void func_0020BF38(struct COL_tdstCollideObject_* object, s32 element, s32 triangle, struct MTH3D_tdstVector_* out);

struct OctreeHit_inv;

struct Octree;
struct OctreeNode;
extern void COL_fn_bGetResultCollision(struct OctreeHit_inv* out, u32* count, u32 max);
extern void COL_fn_vAddInCollisionTable(struct OctreeNode* leaf);
extern void COL_fn_vInitCollisionTable(struct Octree* table);

struct GMT_tdstGameMaterial_;
extern void GEO_vSetRadiusOfIndexedSphere(struct COL_tdstCollideObject_* hObject, s16 hElement,
                                          s16 xNumberOfSphere, f32 xRadius);
extern s32 GEO_lGetIndexedSphereNbSpheres(struct COL_tdstCollideObject_* object, s16 element);
extern s32 GEO_lGetAlignedBoxNbBoxes(struct COL_tdstCollideObject_* object, s16 element);
extern void GEO_vGetMinMaxPointOfIndexedAlignedBox(struct COL_tdstCollideObject_* object, s16 element, s16 box, u16* minimum, u16* maximum);
extern struct GMT_tdstGameMaterial_* GEO_hGetGameMaterialOfIndexedSphere(struct COL_tdstCollideObject_* object, s16 element, s16 sphere);
extern struct GMT_tdstGameMaterial_* GEO_vGetGameMaterialOfIndexedAlignedBox(struct COL_tdstCollideObject_* object, s16 element, s16 box);

struct GeometricObjectView;
struct POS_tdstMatrix_;
extern u8 GEO_bIsLookAt(struct GeometricObjectView* p_stObj);
extern void GEO_vTurnLookAt(struct GeometricObjectView* geometry, struct GLD_tdstViewportAttributes_* viewport, struct POS_tdstMatrix_* matrix);

extern struct Octree* inv_COL_p_stCollisionTable;

extern f32 GEO_g_xZFar;
extern f32 GEO_g_xZFarTransparencyLevel;
extern f32 GEO_g_xZFarTransparencyZone;

#endif
