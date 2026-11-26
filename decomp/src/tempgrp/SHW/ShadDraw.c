#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "PO/PO.h"
#include "SAI/SAI.h"
#include "SCT/SCT.h"

#include "ps2/copy_types.h"
#include "ps2/r5900.h"
#include "ps2/vu0.h"

#include "include_asm.h"
#include "COL/COL.h"
#include "ps2/PS2.h"

extern MTH3D_tdstVector_ inv_SHW_a_stTransformedVertices[];
extern GeometricObjectView* inv_SHW_p_stReceiverGeometry;
extern CollisionEntry* inv_SHW_p_stOctreeEntries;

#define PROJECTION_BUFFER_COUNT 20

extern POS_tdstMatrix_ inv_SHW_stReceiverToProjection;

extern f32 inv_SHW_xFadeStartRatio;

extern f32 inv_SHW_xFadeSlope;

extern f32 inv_SHW_clipBottom;

extern f32 inv_SHW_clipLeft;

extern f32 inv_SHW_clipNear;

extern f32 inv_SHW_clipRight;

extern f32 inv_SHW_clipTop;

extern s32 g_xIndexOfShadow;

extern u32 inv_SHW_vertexCount;

extern void* inv_SHW_p_stReceiverMatrix;

extern volatile s32 D_0039358C;

s32 inv_SHW_fn_bAppendShadowReceiver(s32*, s32, HIE_tdstSuperObject_*);

s32 inv_SHW_fn_bBoundingVolumeIntersectsProjection(void*, HIE_tdstSuperObject_*);

s32 inv_SHW_fn_bSectorReferencesSector(HIE_tdstSuperObject_*, void*);

s32 inv_SHW_fn_bSegmentBoundsIntersectsProjectionVolume(void*, const f32*, const f32*);

void inv_PS2_VU0_vTransformVertexArray(GeometricElement*);

void inv_SHW_fn_vClipTriangleBottomPlane(MTH3D_tdstVector_*, MTH3D_tdstVector_*, MTH3D_tdstVector_*,
                                         s32);

void inv_SHW_fn_vClipTriangleLeftPlane(MTH3D_tdstVector_*, MTH3D_tdstVector_*, MTH3D_tdstVector_*,
                                       s32);

void inv_SHW_fn_vClipTriangleNearPlane(MTH3D_tdstVector_*, MTH3D_tdstVector_*, MTH3D_tdstVector_*,
                                       s32);

void inv_SHW_fn_vClipTriangleRightPlane(MTH3D_tdstVector_*, MTH3D_tdstVector_*, MTH3D_tdstVector_*,
                                        s32);

void inv_SHW_fn_vCollectSectorShadowReceivers(u8*, s32*, void*, HIE_tdstSuperObject_*, void*);

void inv_SHW_fn_vCollectShadowReceiverGeometry(void*, s32*, void*, HIE_tdstSuperObject_*,
                                               HIE_tdstSuperObject_*, void*);

void inv_SHW_fn_vIntersectEdgeWithBottomPlane(f32*, const f32*, const f32*, f32);

void inv_SHW_fn_vIntersectEdgeWithLeftPlane(f32*, const f32*, const f32*, f32);

void inv_SHW_fn_vIntersectEdgeWithNearPlane(f32*, const f32*, const f32*, f32);

void inv_SHW_fn_vIntersectEdgeWithRightPlane(f32*, const f32*, const f32*, f32);

void inv_SHW_fn_vIntersectEdgeWithTopPlane(f32*, const f32*, const f32*, f32);

void inv_SHW_fn_vSetProjectionClipBounds(f32, f32, f32, f32, f32);

extern void inv_POS_fn_vScale(void* m, MTH3D_tdstVector_* scale);

extern s16 inv_SHW_xFaceIndex0;

extern s16 inv_SHW_xFaceIndex1;

extern s16 inv_SHW_xFaceIndex2;

extern f32 D_00392B40;

static inline void inv_SHW_fn_vClearClipScratchW(void)
{
#ifdef PC_PORT
    *(u32*) PC_SCRATCH(0x70003FFC) = 0;
#else
    *(u32*) 0x70003FFC = 0;
#endif
}

static inline MTH3D_tdstVector_* inv_SHW_fn_p_stClipVertex(s32 i)
{
    MTH3D_tdstVector_* p = &inv_SHW_a_stTransformedVertices[i];

    if (*(s32*) p == 0x12345678)
        POS_fn_vMulMatrixVertex(p, &inv_SHW_stReceiverToProjection,
                                &inv_SHW_p_stReceiverGeometry->vertices[i]);
    return p;
}

typedef struct ShadowProjectionMatrixView {
    POS_tdstMatrix_ stInvMatrix;
    POS_tdstMatrix_ stMatrix;
    f32 fInvMinScale;
    f32 fMaxScale;
} ShadowProjectionMatrixView;

typedef struct ProjectedShadowContextView {
    ShadowProjectionMatrixView stProjection;
    HIE_tdstSuperObject_* sectorObject;
    HIE_tdstSuperObject_* superObject;
    f32 weight;
    f32 start;
    f32 end;
    struct MeshView* mesh;
    tdstFogParams_* fogParameters;
} ProjectedShadowContextView;

extern ProjectedShadowContextView inv_SHW_projectionQueue[];
extern ProjectedShadowContextView inv_SHW_stProjectedShadowContext;

s32 inv_SHW_fn_bProjectShadowOntoDynamicGeometry(ProjectedShadowContextView*,
                                                 HIE_tdstSuperObject_*);
s32 inv_SHW_fn_bProjectShadowOntoStaticGeometry(ProjectedShadowContextView* context, void* camera);

#ifdef PC_PORT
/* The PS2 called the inv_SHW_fn_vIntersectEdgeWith* functions through a pointer whose
   parameters are in another order (out, parameter, start, end). MIPS EABI passes floats and
   pointers in separate registers, so the order did not matter there; the Windows x64 ABI
   assigns registers by position, and the functions then read wrong points and parameter.
   The pointer has the functions' own type, and the calls below are put in their order. */
typedef void (*ShadowClipIntersect)(f32*, const f32*, const f32*, f32);
#define intersect(out, parameter, start, end) \
    intersect((f32*) (out), (const f32*) (start), (const f32*) (end), (parameter))
#else
typedef void (*ShadowClipIntersect)(MTH3D_tdstVector_*, f32, const MTH3D_tdstVector_*,
                                    const MTH3D_tdstVector_*);
#endif
typedef void (*ShadowClipNext)(MTH3D_tdstVector_*, MTH3D_tdstVector_*, MTH3D_tdstVector_*, s32);

#define SHADOW_CLIP_TRIANGLE_BODY(distanceExpr0, distanceExpr1, distanceExpr2)                           \
    MTH3D_tdstVector_ intersections[2];                                                                  \
    f32 distance0, distance1, distance2;                                                                 \
    FloatBits bits0, bits1, bits2;                                                                       \
    u32 code;                                                                                            \
    distance1 = distanceExpr1;                                                                           \
    distance0 = distanceExpr0;                                                                           \
    distance2 = distanceExpr2;                                                                           \
    bits0.value = distance0;                                                                             \
    bits1.value = distance1;                                                                             \
    bits2.value = distance2;                                                                             \
    code = (bits0.bits >> 31) | ((bits1.bits & 0x80000000U) >> 30) | ((bits2.bits & 0x80000000U) >> 29); \
    switch (code) {                                                                                      \
    case 0:                                                                                              \
        next(vertex0, vertex1, vertex2, flag);                                                           \
        break;                                                                                           \
    case 1:                                                                                              \
        intersect(&intersections[0], distance0, vertex0, vertex1);                                       \
        next(&intersections[0], vertex1, vertex2, 1);                                                    \
        intersect(&intersections[1], distance0, vertex0, vertex2);                                       \
        next(&intersections[1], &intersections[0], vertex2, 1);                                          \
        break;                                                                                           \
    case 2:                                                                                              \
        intersect(&intersections[0], distance1, vertex1, vertex0);                                       \
        next(&intersections[0], vertex2, vertex0, 1);                                                    \
        intersect(&intersections[1], distance1, vertex1, vertex2);                                       \
        next(&intersections[0], &intersections[1], vertex2, 1);                                          \
        break;                                                                                           \
    case 3:                                                                                              \
        intersect(&intersections[0], distance0, vertex0, vertex2);                                       \
        intersect(&intersections[1], distance1, vertex1, vertex2);                                       \
        next(&intersections[0], &intersections[1], vertex2, 1);                                          \
        break;                                                                                           \
    case 4:                                                                                              \
        intersect(&intersections[0], distance2, vertex2, vertex0);                                       \
        next(&intersections[0], vertex0, vertex1, 1);                                                    \
        intersect(&intersections[1], distance2, vertex2, vertex1);                                       \
        next(&intersections[0], vertex1, &intersections[1], 1);                                          \
        break;                                                                                           \
    case 5:                                                                                              \
        intersect(&intersections[0], distance0, vertex0, vertex1);                                       \
        intersect(&intersections[1], distance2, vertex2, vertex1);                                       \
        next(&intersections[0], vertex1, &intersections[1], 1);                                          \
        break;                                                                                           \
    case 6:                                                                                              \
        intersect(&intersections[0], distance1, vertex1, vertex0);                                       \
        intersect(&intersections[1], distance2, vertex2, vertex0);                                       \
        next(&intersections[0], &intersections[1], vertex0, 1);                                          \
        break;                                                                                           \
    case 7:                                                                                              \
        break;                                                                                           \
    }

static __inline__ void inv_SHW_fn_vClip143BottomImplView(MTH3D_tdstVector_* vertex0,
                                               MTH3D_tdstVector_* vertex1,
                                               MTH3D_tdstVector_* vertex2, s32 flag,
                                               ShadowClipIntersect intersect,
                                               ShadowClipNext next)
{
    SHADOW_CLIP_TRIANGLE_BODY(vertex0->xY - inv_SHW_clipBottom,
                              vertex1->xY - inv_SHW_clipBottom, vertex2->xY - inv_SHW_clipBottom);
}

static __inline__ void inv_SHW_fn_vClip143LeftImplView(MTH3D_tdstVector_* vertex0,
                                                       MTH3D_tdstVector_* vertex1,
                                                       MTH3D_tdstVector_* vertex2, s32 flag,
                                                       ShadowClipIntersect intersect,
                                                       ShadowClipNext next)
{
    SHADOW_CLIP_TRIANGLE_BODY(vertex0->xX - inv_SHW_clipLeft,
                              vertex1->xX - inv_SHW_clipLeft, vertex2->xX - inv_SHW_clipLeft);
}

static __inline__ void inv_SHW_fn_vClip143670ImplView(MTH3D_tdstVector_* vertex0,
                                                      MTH3D_tdstVector_* vertex1,
                                                      MTH3D_tdstVector_* vertex2, s32 flag,
                                                      ShadowClipIntersect intersect,
                                                      ShadowClipNext next)
{
    SHADOW_CLIP_TRIANGLE_BODY(vertex0->xZ - inv_SHW_clipNear,
                              vertex1->xZ - inv_SHW_clipNear, vertex2->xZ - inv_SHW_clipNear);
}

static __inline__ void inv_SHW_fn_vClip143RightImplView(MTH3D_tdstVector_* vertex0,
                                              MTH3D_tdstVector_* vertex1,
                                              MTH3D_tdstVector_* vertex2, s32 flag,
                                              ShadowClipIntersect intersect,
                                              ShadowClipNext next)
{
    SHADOW_CLIP_TRIANGLE_BODY(inv_SHW_clipRight - vertex0->xX,
                              inv_SHW_clipRight - vertex1->xX, inv_SHW_clipRight - vertex2->xX);
}

static __inline__ void inv_SHW_fn_vClip143TopImplView(MTH3D_tdstVector_* vertex0,
                                                      MTH3D_tdstVector_* vertex1,
                                                      MTH3D_tdstVector_* vertex2, s32 flag,
                                                      ShadowClipIntersect intersect,
                                                      ShadowClipNext next)
{
    SHADOW_CLIP_TRIANGLE_BODY(inv_SHW_clipTop - vertex0->xY,
                              inv_SHW_clipTop - vertex1->xY, inv_SHW_clipTop - vertex2->xY);
}

void inv_SHW_fn_vInitProjectionBuffers(void)
{
    s32 i;

    for (i = 0; i < PROJECTION_BUFFER_COUNT; i++) {
        inv_RW3_fn_bCreateMesh(&inv_SHW_projectionQueue[i].mesh, 100);
    }
}

void SHW_fn_vInitShadowCounter(void)
{
    g_xIndexOfShadow = 0;
}

void inv_SHW_fn_vBuildProjectionMatrix(ShadowProjectionMatrixView* out, void* pos,
                                       MTH3D_tdstVector_* dir, f32 scaleZ, f32 scaleX, f32 scaleY)
{
    MTH3D_tdstMatrix_ stRotation;
    MTH3D_tdstVector_ stScale;
    f32 len, sq, n;
    f32 minScale;
    f32 x, y, z;
    f32 cy, cz;

    x = dir->xX;
    y = dir->xY;
    z = dir->xZ;
    sq = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    len = MTH_fn_xSqrtOpt2(sq);
    if (len != 0.0f) {
        f32 xInvLength = 1.0f / len;
        MTH3D_M_vSetXYZ_inv(&stRotation.stCol_2, xInvLength * x, xInvLength * y, xInvLength * z);
    } else {
        stRotation.stCol_2 = *dir;
    }

    cz = stRotation.stCol_2.xZ;
    cy = stRotation.stCol_2.xY;
    if (__builtin_fabsf(cz) > __builtin_fabsf(cy)) {
        MTH3D_M_vSetXYZ_inv(&stRotation.stCol_1, 0.0f, 1.0f, 0.0f);
    } else {
        stRotation.stCol_1.xY = stRotation.stCol_1.xX = 0.0f;
        stRotation.stCol_1.xZ = 1.0f;
    }

    x = inv_MTH_fn_xMulSub(stRotation.stCol_1.xY, stRotation.stCol_2.xZ, stRotation.stCol_1.xZ,
                           stRotation.stCol_2.xY);
    y = inv_MTH_fn_xMulSub(stRotation.stCol_1.xZ, stRotation.stCol_2.xX, stRotation.stCol_1.xX,
                           stRotation.stCol_2.xZ);
    z = inv_MTH_fn_xMulSub(stRotation.stCol_1.xX, stRotation.stCol_2.xY, stRotation.stCol_1.xY,
                           stRotation.stCol_2.xX);
    MTH3D_M_vSetXYZ_inv(&stRotation.stCol_0, x, y, z);

    n = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    n = MTH_fn_xSqrtOpt2(n);
    if (n != 0.0f) {
        f32 xInvLength = 1.0f / n;
        MTH3D_M_vSetXYZ_inv(&stRotation.stCol_0, xInvLength * x, xInvLength * y, xInvLength * z);
    }

    DNM_p_stVectorCrossProduct(&stRotation.stCol_1, &stRotation.stCol_2, &stRotation.stCol_0);

    inv_POS_fn_vInitMatrixFromVectors(&out->stMatrix, 5, &stRotation.stCol_0, &stRotation.stCol_1, &stRotation.stCol_2, pos);

    minScale = inv_MTH_fn_xMinimum(scaleX, scaleY);
    out->fInvMinScale = 1.0f / minScale;
    out->fMaxScale = inv_MTH_fn_xMaximum(scaleX, scaleY);
    MTH3D_M_vSetXYZ_inv(&stScale, scaleX, scaleY, scaleZ);
    inv_POS_fn_vScale(&out->stMatrix, &stScale);
    POS_fn_vInvertMatrix(&out->stInvMatrix, &out->stMatrix);
}

s32 inv_SHW_fn_bQueueProjectedShadow(void* sectorObject, void* superObject, void* source,
                                     GliMaterialData* material, f32 weight, f32 start, f32 end,
                                     tdstFogParams_* fogParameters)
{
    s32 result = 0;
    if ((u32) g_xIndexOfShadow < 20) {
        ProjectedShadowContextView* slot = &inv_SHW_projectionQueue[g_xIndexOfShadow];
        fn_vMSSoundSetMechaCharacter(slot->mesh, material);
        slot->weight = weight;
        slot->stProjection = *(ShadowProjectionMatrixView*) source;
        slot->sectorObject = sectorObject;
        slot->start = start;
        slot->end = end;
        slot->superObject = superObject;
        slot->fogParameters = fogParameters;
        g_xIndexOfShadow++;
        result = 1;
    }
    return result;
}

void inv_SHW_fn_vSetProjectedShadowContext(ProjectedShadowContextView* block)
{
    f32 scale;

    inv_SHW_fn_vSetProjectionClipBounds(-0.5f, -0.5f, 0.5f, 0.5f, 0.0f);

    inv_SHW_stProjectedShadowContext = *block;

    inv_SHW_vertexCount = 0;
    if (inv_SHW_stProjectedShadowContext.end > inv_SHW_stProjectedShadowContext.start) {
        scale = inv_SHW_stProjectedShadowContext.start / inv_SHW_stProjectedShadowContext.end;
        inv_SHW_xFadeStartRatio = scale;
        inv_SHW_xFadeSlope = inv_SHW_stProjectedShadowContext.weight / (1.0f - scale);
    } else {
        f32 zero = 0.0f;
        inv_SHW_xFadeSlope = zero;
        inv_SHW_xFadeStartRatio = zero;
    }
}

void inv_SHW_fn_vRenderQueuedProjectedShadows(void* attributes, u32 ulDisplayMode)
{
    s32 status[20];
    MTH3D_tdstVector_ v;
    s32 index;
    s32 combinedStatus;
    ProjectedShadowContextView* slot;
    void* currentSector;
    f32 lengthSquared;
    f32 length;
    f32 inverseLength;
    HIE_tdstSuperObject_* sys;

    combinedStatus = 0;
    index = 0;
    sys = CAM_fn_hGetActiveCamera(1);
    currentSector = inv_HIE_fn_p_stGetEngineObject(sys)->h_SectInfo->hCurrentSector;
    if (g_xIndexOfShadow != 0) {
        do {
            slot = &inv_SHW_projectionQueue[index];
            v = *(MTH3D_tdstVector_*) &slot->stProjection.stMatrix.stMatrix.stCol_2;
            lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(v.xX, v.xY, v.xZ);
            length = MTH_fn_xSqrtOpt2(lengthSquared);
            if (length != 0.0f) {
                inverseLength = 1.0f / length;
                MTH3D_M_vKTimesXYZ_inv(&v, inverseLength, &v);
            }
            inv_RW3_fn_vBeginMesh(slot->mesh, &v);
            inv_SHW_fn_vSetProjectedShadowContext(slot);
            status[index] = inv_SHW_fn_bProjectShadowOntoStaticGeometry(slot, currentSector);
            status[index] =
                status[index] | inv_SHW_fn_bProjectShadowOntoDynamicGeometry(slot, currentSector);
            combinedStatus = combinedStatus | status[index];
            inv_RW3_fn_vEndMesh(slot->mesh);
            index = index + 1;
        } while ((u32) index < (u32) g_xIndexOfShadow);
    }
    if (combinedStatus != 0) {
        inv_RW3_fn_vSetRenderObject(attributes, ulDisplayMode);
        index = 0;
        if (g_xIndexOfShadow != 0) {
            do {
                if (status[index] != 0) {
                    inv_RW3_fn_vDrawMesh(inv_SHW_projectionQueue[index].mesh,
                                  inv_SHW_projectionQueue[index].weight,
                                  inv_SHW_projectionQueue[index].fogParameters);
                }
                index = index + 1;
            } while ((u32) index < (u32) g_xIndexOfShadow);
        }
    }
    g_xIndexOfShadow = 0;
}

static inline MTH3D_tdstVector_* inv_SHW_fn_p_stShadowVertex(s32 index)
{
    return &inv_SHW_a_stTransformedVertices[index];
}

static inline void inv_SHW_fn_vLoadTriangleToVU0(ScratchVector* scratch,
                                                 const MTH3D_tdstVector_* v0,
                                                 const MTH3D_tdstVector_* v1,
                                                 const MTH3D_tdstVector_* v2)
{
    scratch->xyz = *v0;
    scratch->w = 0;
    VU0_LQC2(1, scratch, 0);
    scratch->xyz = *v1;
    scratch->w = 0;
    VU0_LQC2(2, scratch, 0);
    scratch->xyz = *v2;
    scratch->w = 0;
    VU0_LQC2(3, scratch, 0);
    VU0_VCALLMS(0);
}

static inline s32 inv_SHW_fn_bTriangleOutsideProjection(const MTH3D_tdstVector_* p0,
                                                        const MTH3D_tdstVector_* p1,
                                                        const MTH3D_tdstVector_* p2)
{
    s32 outside;

    outside = p0->xX < -0.5f && p1->xX < -0.5f && p2->xX < -0.5f;
    outside |= p0->xY < -0.5f && p1->xY < -0.5f && p2->xY < -0.5f;
    if (outside)
        return outside;
    outside |= p0->xX > 0.5f && p1->xX > 0.5f && p2->xX > 0.5f;
    outside |= p0->xY > 0.5f && p1->xY > 0.5f && p2->xY > 0.5f;
    outside |= p0->xZ > 1.0f && p1->xZ > 1.0f && p2->xZ > 1.0f;
    return outside;
}

void inv_PS2_VU0_vTransformVertexArray(GeometricElement* element)
{
    MTH3D_tdstVector_ transformed;
    MTH3D_tdstVector_* projected = &transformed;
    GEO_tdstTripledIndex_* triangles = element->d_stListOfFacesTripled;
#ifdef PC_PORT
    ScratchVector* scratch = (ScratchVector*) PC_SCRATCH(0x70003FF0);
#else
    ScratchVector* scratch = (ScratchVector*) 0x70003FF0;
#endif
    s16 index;
    MTH3D_tdstVector_ *p0, *p1, *p2;
    GEO_tdstTripledIndex_* triangle;
    const MTH3D_tdstVector_ *v0, *v1, *v2;

    v0 = inv_SHW_fn_p_stShadowVertex(triangles[0].a3_xIndex[0]);
    v1 = inv_SHW_fn_p_stShadowVertex(triangles[0].a3_xIndex[1]);
    v2 = inv_SHW_fn_p_stShadowVertex(triangles[0].a3_xIndex[2]);
    inv_SHW_fn_vLoadTriangleToVU0(scratch, v0, v1, v2);

    for (index = 0; index < element->xNbFaces && inv_SHW_vertexCount < 298; index++) {
        triangle = &triangles[index];
        inv_SHW_xFaceIndex0 = triangle->a3_xIndex[0];
        inv_SHW_xFaceIndex1 = triangle->a3_xIndex[1];
        inv_SHW_xFaceIndex2 = triangle->a3_xIndex[2];
        p0 = inv_SHW_fn_p_stShadowVertex(triangle->a3_xIndex[0]);
        p1 = inv_SHW_fn_p_stShadowVertex(triangle->a3_xIndex[1]);
        p2 = inv_SHW_fn_p_stShadowVertex(triangle->a3_xIndex[2]);
        while (inv_PS2_fn_bIsVu0Busy())
            ;
        VU0_SQC2(4, scratch, 0);
        *projected = scratch->xyz;
        if ((s16) (index + 1) < element->xNbFaces) {
            triangle = &triangles[(s16) (index + 1)];
            v0 = inv_SHW_fn_p_stShadowVertex(triangle->a3_xIndex[0]);
            v1 = inv_SHW_fn_p_stShadowVertex(triangle->a3_xIndex[1]);
            v2 = inv_SHW_fn_p_stShadowVertex(triangle->a3_xIndex[2]);
            inv_SHW_fn_vLoadTriangleToVU0(scratch, v0, v1, v2);
        }
        if (projected->xZ < D_00392B40 && !inv_SHW_fn_bTriangleOutsideProjection(p0, p1, p2))
            inv_SHW_fn_vClipTriangleNearPlane(p0, p1, p2, 0);
    }
}

void inv_PS2_fn_vTransformVertexArrayByMatrices(void* arg, GeometricObjectView* batch,
                                                void* matrix, u32 flags)
{
    u32 work[24];
    u32 result[4];
    s32 i = 0;
    s32 tick;
    s32 inc;
    s32 one;
    s32 copy;
    _POS_fn_vMulMatrixMatrix((POS_tdstMatrix_*) work, arg, matrix);
    POS_fn_vMulMatrixVertexArray(inv_SHW_a_stTransformedVertices, (POS_tdstMatrix_*) work,
                                 batch->vertices, batch->vertexCount);
    inv_SHW_p_stReceiverMatrix = matrix;
    inv_SHW_p_stReceiverGeometry = batch;
    if (batch->elementCount <= 0)
        return;
    if (inv_SHW_vertexCount >= 298)
        return;
    one = 1;
    tick = 0x10000;
    inc = 0x10000;
    do {
        if (batch->elementTypes[i] == one) {
            inv_GEO_fn_vGetElementVisualMaterial(batch, i, (GliMaterialData**) result);
            if (result[0] == 0 || inv_GLI_bIsMaterialAnimationEnabled((void*) result[0]) != 0) {
                inv_PS2_VU0_vTransformVertexArray(batch->elements[i]);
            }
        }
        copy = tick;
        tick += inc;
        i = (s32) copy >> 16;
        if (i >= batch->elementCount)
            break;
    } while (inv_SHW_vertexCount < 298);
}

s32 inv_SHW_fn_bProjectShadowOntoDynamicGeometry(ProjectedShadowContextView* context,
                                                 HIE_tdstSuperObject_* sector)
{
    typedef struct ShadowObjectQueueEntry {
        HIE_tdstSuperObject_* superObject;
        GeometricObjectView* geometry;
    } ShadowObjectQueueEntry;

    ShadowObjectQueueEntry records[100];
    u32 count;
    u32 initial;
    ShadowObjectQueueEntry* record;
    ShadowObjectQueueEntry* end;

    count = 0;
    initial = inv_SHW_vertexCount;
    inv_SHW_fn_vCollectShadowReceiverGeometry(records, &count, context->superObject,
                                              context->sectorObject, sector, context);
    end = records + count;
    record = records;
    if (record < end && inv_SHW_vertexCount < 0x12A) {
        do {
            HIE_tdstSuperObject_* receiver = record->superObject;
            GeometricObjectView* geometry = record->geometry;
            u32 flags = receiver->gu_lDrawModeMask & 0x80000;
            POS_tdstMatrix_* matrix = receiver->gu_hGlobalMatrix;

            record++;
            inv_PS2_fn_vTransformVertexArrayByMatrices(context, geometry, matrix, flags);
        } while (record < end && inv_SHW_vertexCount < 0x12A);
    }
    return initial < inv_SHW_vertexCount;
}

OctreeHit_inv* inv_PS2_VU0_pTransformAndClipVertices(OctreeHit_inv* pIdx, OctreeHit_inv* pEnd)
{
    MTH3D_tdstVector_ stRes;
    GliMaterialData* hMat;
    MTH3D_tdstVector_* pScratch;
    s16* pTris;
    u32 ulElem;
    MTH3D_tdstVector_* pRes;
    CollisionEntry* pElem;
    s16* pTri;
    MTH3D_tdstVector_* p_stClipped0;
    MTH3D_tdstVector_* p_stClipped1;
    MTH3D_tdstVector_* p_stClipped2;
    MTH3D_tdstVector_* p_stTransformed0;
    MTH3D_tdstVector_* p_stTransformed1;
    MTH3D_tdstVector_* p_stTransformed2;
    s32 lMark;
    OctreeHit_inv* pNext;
    s32 lOut;

#ifdef PC_PORT
    pScratch = (MTH3D_tdstVector_*) PC_SCRATCH(0x70003FF0);
#else
    pScratch = (MTH3D_tdstVector_*) 0x70003FF0;
#endif
    ulElem = pIdx->entry;
    pRes = &stRes;
    pElem = &inv_SHW_p_stOctreeEntries[ulElem];
    inv_GEO_fn_vGetElementVisualMaterial(pElem->geometry, pElem->element, &hMat);
    if (hMat != NULL && !inv_GLI_bIsMaterialAnimationEnabled(hMat)) {
        while (pIdx < pEnd && ulElem == pIdx->entry)
            pIdx++;
        return pIdx;
    }

    pTris = pElem->geometry->elements[pElem->element]->d_stListOfFacesTripled->a3_xIndex;
    pTri = pTris + pIdx->triangle * 3;
    p_stClipped0 = inv_SHW_fn_p_stClipVertex(pTri[0]);
    p_stClipped1 = inv_SHW_fn_p_stClipVertex(pTri[1]);
    p_stClipped2 = inv_SHW_fn_p_stClipVertex(pTri[2]);
    *pScratch = *p_stClipped0;
    inv_SHW_fn_vClearClipScratchW();
    VU0_LQC2(1, pScratch, 0);
    *pScratch = *p_stClipped1;
    inv_SHW_fn_vClearClipScratchW();
    VU0_LQC2(2, pScratch, 0);
    *pScratch = *p_stClipped2;
    inv_SHW_fn_vClearClipScratchW();
    VU0_LQC2(3, pScratch, 0);
    VU0_VCALLMS(0);

    if (pIdx < pEnd && ulElem == pIdx->entry) {
        lMark = 0x12345678;
        while (pIdx < pEnd && ulElem == pIdx->entry) {
            pTri = pTris + pIdx->triangle * 3;
            inv_SHW_xFaceIndex0 = pTri[0];
            inv_SHW_xFaceIndex1 = pTri[1];
            inv_SHW_xFaceIndex2 = pTri[2];
            p_stTransformed0 = &inv_SHW_a_stTransformedVertices[inv_SHW_xFaceIndex0];
            if (*(s32*) p_stTransformed0 == lMark)
                POS_fn_vMulMatrixVertex(
                    p_stTransformed0, &inv_SHW_stReceiverToProjection,
                    &inv_SHW_p_stReceiverGeometry->vertices[inv_SHW_xFaceIndex0]);
            p_stTransformed1 = &inv_SHW_a_stTransformedVertices[inv_SHW_xFaceIndex1];
            if (*(s32*) p_stTransformed1 == lMark)
                POS_fn_vMulMatrixVertex(
                    p_stTransformed1, &inv_SHW_stReceiverToProjection,
                    &inv_SHW_p_stReceiverGeometry->vertices[inv_SHW_xFaceIndex1]);
            p_stTransformed2 = &inv_SHW_a_stTransformedVertices[inv_SHW_xFaceIndex2];
            if (*(s32*) p_stTransformed2 == lMark)
                POS_fn_vMulMatrixVertex(
                    p_stTransformed2, &inv_SHW_stReceiverToProjection,
                    &inv_SHW_p_stReceiverGeometry->vertices[inv_SHW_xFaceIndex2]);
            pNext = pIdx + 1;

            while (inv_PS2_fn_bIsVu0Busy())
                ;
            VU0_SQC2(4, pScratch, 0);
            *pRes = *pScratch;

            if (pIdx + 1 < pEnd && ulElem == pIdx[1].entry) {
                pTri = pTris + pNext->triangle * 3;
                p_stClipped0 = inv_SHW_fn_p_stClipVertex(pTri[0]);
                p_stClipped1 = inv_SHW_fn_p_stClipVertex(pTri[1]);
                p_stClipped2 = inv_SHW_fn_p_stClipVertex(pTri[2]);
                *pScratch = *p_stClipped0;
                inv_SHW_fn_vClearClipScratchW();
                VU0_LQC2(1, pScratch, 0);
                *pScratch = *p_stClipped1;
                inv_SHW_fn_vClearClipScratchW();
                VU0_LQC2(2, pScratch, 0);
                *pScratch = *p_stClipped2;
                inv_SHW_fn_vClearClipScratchW();
                VU0_LQC2(3, pScratch, 0);
                VU0_VCALLMS(0);
            }

            if (pRes->xZ < -0.309017f) {
                lOut = p_stTransformed0->xX < -0.5f && p_stTransformed1->xX < -0.5f && p_stTransformed2->xX < -0.5f;
                lOut |= p_stTransformed0->xX > 0.5f && p_stTransformed1->xX > 0.5f && p_stTransformed2->xX > 0.5f;
                if (!lOut) {
                    lOut |= p_stTransformed0->xY < -0.5f && p_stTransformed1->xY < -0.5f && p_stTransformed2->xY < -0.5f;
                    lOut |= p_stTransformed0->xY > 0.5f && p_stTransformed1->xY > 0.5f && p_stTransformed2->xY > 0.5f;
                    lOut |= p_stTransformed0->xZ > 1.0f && p_stTransformed1->xZ > 1.0f && p_stTransformed2->xZ > 1.0f;
                    if (!lOut) {
                        ((void (*)(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b, MTH3D_tdstVector_* c,
                                   s32 flags)) inv_SHW_fn_vClipTriangleNearPlane)(
                            p_stTransformed0, p_stTransformed1, p_stTransformed2, 0);
                        if (inv_SHW_vertexCount >= 300)
                            pIdx = pEnd;
                        pNext = pIdx + 1;
                    }
                }
            }
            pIdx = pNext;
        }
    }
    return pIdx;
}

OctreeHit_inv* inv_SHW_fn_p_uwProcessStaticGeometryBatch(OctreeHit_inv* cur,
                                                         OctreeHit_inv* end)
{
    CollisionEntry* refs;
    u32 value;
    HIE_tdstSuperObject_* superObject;
    s16 count;
    GeometricObjectView* same;

    refs = inv_SHW_p_stOctreeEntries;
    value = (u32) refs[cur->entry].geometry;
    inv_SHW_p_stReceiverGeometry = (GeometricObjectView*) value;
    superObject = refs[cur->entry].superObject;
    count = ((GeometricObjectView*) value)->vertexCount;
    inv_SHW_p_stReceiverMatrix = superObject->gu_hGlobalMatrix;

    if (!(count > 0x2000)) {
        if (((((s32) inv_SHW_p_stOctreeEntries[cur->entry].superObject->flags >> 7) ^ 1) & 1) != 0)
            goto special;
    }
    if (cur >= end)
        return cur;
    if ((GeometricObjectView*) value != inv_SHW_p_stOctreeEntries[cur->entry].geometry)
        return cur;
    same = (GeometricObjectView*) value;
    value = 20;
    do {
        cur++;
        if (cur >= end)
            break;
    } while (same == *(GeometricObjectView**) ((u8*) inv_SHW_p_stOctreeEntries +
                                               (u32) cur->entry * value + 4));
    return cur;

special: {
    u8* p = (u8*) inv_SHW_a_stTransformedVertices;
    u8* limit = p + count * 12;
    if (p < limit) {
        do {
            *(u32*) p = 0x12345678;
            p += 12;
        } while (p < limit);
    }
}
    _POS_fn_vMulMatrixMatrix(&inv_SHW_stReceiverToProjection,
                             &inv_SHW_stProjectedShadowContext.stProjection.stInvMatrix,
                             inv_SHW_p_stReceiverMatrix);
    do {
        cur = inv_PS2_VU0_pTransformAndClipVertices(cur, end);
        if (cur >= end)
            break;
    } while (inv_SHW_p_stReceiverGeometry == inv_SHW_p_stOctreeEntries[cur->entry].geometry);
    return cur;
}

s32 inv_SHW_fn_bProjectShadowOntoStaticGeometry(ProjectedShadowContextView* context, void* camera)
{
    OctreeBox box;
    OctreeHit_inv elements[512];
    s32 count;
    f32 epsilon;
    u32 initialCount;
    OctreeHit_inv* cursor;
    OctreeHit_inv* end;

    epsilon = 0.001f;
    box.center = *inv_POS_fn_p_stPosMatrixTranslation(&context->stProjection.stMatrix);
    box.end = *(MTH3D_tdstVector_*) &context->stProjection.stMatrix.stMatrix.stCol_2;
    initialCount = inv_SHW_vertexCount;
    box.extent = context->stProjection.fMaxScale;
    inv_MTH3D_fn_vAddXYZ(&box.end, &box.end, &box.center);
    box.axisFlags = 0;
    if (inv_MTH_fn_xAbsoluteTied(box.center.xX - box.end.xX) < epsilon) {
        box.axisFlags = 2;
    }
    if (inv_MTH_fn_xAbsoluteTied(box.center.xY - box.end.xY) < epsilon) {
        box.axisFlags |= 4;
    }
    if (inv_MTH_fn_xAbsoluteTied(box.center.xZ - box.end.xZ) < epsilon) {
        box.axisFlags |= 8;
    }
    count = 0;
    COL_fn_vExploreOctreeWithBox(&box, elements, &count, 0x200, &inv_SHW_p_stOctreeEntries);
    if (count != 0) {
        cursor = elements;
        end = elements + count;
        while (cursor < end) {
            cursor = inv_SHW_fn_p_uwProcessStaticGeometryBatch(cursor, end);
        }
    }
    return initialCount < inv_SHW_vertexCount;
}

static __inline__ void inv_SHW_fn_vEmitTransformedVertex(MTH3D_tdstVector_* vert)
{
    MTH3D_tdstVector_ pos;
    f32 uv[2];
    MTH3D_tdstVector_ stVertex;
    MTH3D_tdstVector_* pIn;
    MTH3D_tdstVector_* pOut;
    u8* p_ucMatrix;
    f32 alpha;
    f32 max;

    uv[0] = vert->xX + 0.5f;
    uv[1] = vert->xY + 0.5f;
    pIn = &stVertex;
    pOut = &pos;
    p_ucMatrix = (u8*) &inv_SHW_stProjectedShadowContext.stProjection.stMatrix;
    if (vert == pOut) {
        stVertex = *vert;
        R5900_TRANSFORM_POINT_inv(pOut, pIn, p_ucMatrix);
    } else {
        R5900_TRANSFORM_POINT_inv(pOut, vert, p_ucMatrix);
    }
    max = inv_SHW_stProjectedShadowContext.weight;
    alpha = max - (vert->xZ - inv_SHW_xFadeStartRatio) * inv_SHW_xFadeSlope;
    if (alpha < 0.0f) {
        alpha = 0.0f;
    } else if (max < alpha) {
        alpha = max;
    }
    inv_RW3_fn_vAddMeshVertex(&pos, uv, alpha);
    inv_SHW_vertexCount++;
}

static __inline__ void inv_SHW_fn_vEmitIndexedVertex(MTH3D_tdstVector_* vert, s16 index)
{
    MTH3D_tdstVector_ pos;
    f32 uv[2];
    f32 alpha;
    f32 max;

    uv[0] = vert->xX + 0.5f;
    uv[1] = vert->xY + 0.5f;
    POS_fn_vMulMatrixVertex(&pos, inv_SHW_p_stReceiverMatrix,
                            &inv_SHW_p_stReceiverGeometry->vertices[index]);
    max = inv_SHW_stProjectedShadowContext.weight;
    alpha = max - (vert->xZ - inv_SHW_xFadeStartRatio) * inv_SHW_xFadeSlope;
    if (alpha < 0.0f) {
        alpha = 0.0f;
    } else if (max < alpha) {
        alpha = max;
    }
    inv_RW3_fn_vAddMeshVertex(&pos, uv, alpha);
    inv_SHW_vertexCount++;
}

void inv_SHW_fn_vEmitClippedTriangle(MTH3D_tdstVector_* a0, MTH3D_tdstVector_* a1,
                                     MTH3D_tdstVector_* a2, s32 transformed)
{
    if (inv_SHW_vertexCount < 300) {
        if (transformed) {
            inv_SHW_fn_vEmitTransformedVertex(a0);
            inv_SHW_fn_vEmitTransformedVertex(a1);
            inv_SHW_fn_vEmitTransformedVertex(a2);
        } else {
            inv_SHW_fn_vEmitIndexedVertex(a0, inv_SHW_xFaceIndex0);
            inv_SHW_fn_vEmitIndexedVertex(a1, inv_SHW_xFaceIndex1);
            inv_SHW_fn_vEmitIndexedVertex(a2, inv_SHW_xFaceIndex2);
        }
    }
}

void inv_SHW_fn_vClipTriangleNearPlane(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b,
                                       MTH3D_tdstVector_* c, s32 flag)
{
    inv_SHW_fn_vClip143670ImplView(a, b, c, flag, inv_SHW_fn_vIntersectEdgeWithNearPlane,
                        (ShadowClipNext) inv_SHW_fn_vEmitClippedTriangle);
}

void inv_SHW_fn_vClipTriangleLeftPlane(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b,
                                       MTH3D_tdstVector_* c, s32 flag)
{
    inv_SHW_fn_vClip143LeftImplView(a, b, c, flag, inv_SHW_fn_vIntersectEdgeWithLeftPlane,
                          inv_SHW_fn_vClipTriangleNearPlane);
}

void inv_SHW_fn_vClipTriangleBottomPlane(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b,
                                         MTH3D_tdstVector_* c, s32 flag)
{
    inv_SHW_fn_vClip143BottomImplView(a, b, c, flag, inv_SHW_fn_vIntersectEdgeWithBottomPlane,
                            inv_SHW_fn_vClipTriangleLeftPlane);
}

void inv_SHW_fn_vClipTriangleRightPlane(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b,
                                        MTH3D_tdstVector_* c, s32 flag)
{
    inv_SHW_fn_vClip143RightImplView(a, b, c, flag, inv_SHW_fn_vIntersectEdgeWithRightPlane,
                           inv_SHW_fn_vClipTriangleBottomPlane);
}

void inv_SHW_fn_vClipTriangleTopPlane(MTH3D_tdstVector_* a, MTH3D_tdstVector_* b,
                                      MTH3D_tdstVector_* c, s32 flag)
{
    inv_SHW_fn_vClip143TopImplView(a, b, c, flag, inv_SHW_fn_vIntersectEdgeWithTopPlane,
                         inv_SHW_fn_vClipTriangleRightPlane);
}

void inv_SHW_fn_vSetProjectionClipBounds(f32 left, f32 bottom, f32 right, f32 top, f32 near)
{
    inv_SHW_clipLeft = left;
    inv_SHW_clipRight = right;
    inv_SHW_clipBottom = bottom;
    inv_SHW_clipTop = top;
    inv_SHW_clipNear = near;
}

void inv_SHW_fn_vIntersectEdgeWithNearPlane(f32* out, const f32* start, const f32* end,
                                            f32 parameter)
{
    f32 ax = start[0];
    f32 planePosition = inv_SHW_clipNear;
    f32 ay;
    f32 by;

    parameter = parameter / (start[2] - end[2]);
    out[0] = ax + parameter * (end[0] - ax);
    ay = start[1];
    by = end[1];
    out[2] = planePosition;
    out[1] = ay + parameter * (by - ay);
}

void inv_SHW_fn_vIntersectEdgeWithLeftPlane(f32* out, const f32* start, const f32* end,
                                            f32 parameter)
{
    parameter = parameter / (start[0] - end[0]);
    out[0] = inv_SHW_clipLeft;
    out[1] = start[1] + parameter * (end[1] - start[1]);
    out[2] = start[2] + parameter * (end[2] - start[2]);
}

void inv_SHW_fn_vIntersectEdgeWithBottomPlane(f32* out, const f32* start, const f32* end,
                                              f32 parameter)
{
    f32 ax = start[0];
    f32 bx = end[0];
    f32 planePosition = inv_SHW_clipBottom;

    parameter = parameter / (start[1] - end[1]);
    out[1] = planePosition;
    out[0] = ax + parameter * (bx - ax);
    out[2] = start[2] + parameter * (end[2] - start[2]);
}

void inv_SHW_fn_vIntersectEdgeWithRightPlane(f32* out, const f32* start, const f32* end,
                                             f32 parameter)
{
    parameter = parameter / (end[0] - start[0]);
    out[0] = inv_SHW_clipRight;
    out[1] = start[1] + parameter * (end[1] - start[1]);
    out[2] = start[2] + parameter * (end[2] - start[2]);
}

void inv_SHW_fn_vIntersectEdgeWithTopPlane(f32* out, const f32* start, const f32* end,
                                           f32 parameter)
{
    f32 ax = start[0];
    f32 bx = end[0];
    f32 planePosition = inv_SHW_clipTop;

    parameter = parameter / (end[1] - start[1]);
    out[1] = planePosition;
    out[0] = ax + parameter * (bx - ax);
    out[2] = start[2] + parameter * (end[2] - start[2]);
}

s32 inv_SHW_fn_bBoundingVolumeIntersectsProjection(void* out, HIE_tdstSuperObject_* object)
{
    extern s32 inv_SHW_fn_bSphereIntersectsProjectionVolume(void* obj, void* pos, f32 scale);

    f32 start[4];
    f32 end[4];
    COL_tdstBoundingSphere_* local;
    s32 result = 0;
    if ((*(u8*) &object->flags & 0x10) != 0) {
        f32* p = object->hVisualBoundingVolume;
        if (p != 0) {
            f32* base = &object->gu_hGlobalMatrix->stMatrix.stCol_3.xX;
            f32* p2 = p + 3;
            start[0] = p[0] + base[0];
            start[1] = p[1] + base[1];
            start[2] = p[2] + base[2];
            end[0] = p2[0] + base[0];
            end[1] = p2[1] + base[1];
            end[2] = p2[2] + base[2];
            result = inv_SHW_fn_bSegmentBoundsIntersectsProjectionVolume(out, start, end) != 0;
        }
    } else {
        object->hVisualBoundingVolume = &local;
        if (local != 0) {
            void* mtx = object->gu_hGlobalMatrix;
            f32 scale;
            f32 factor;
            POS_fn_vMulMatrixVertex((MTH3D_tdstVector_*) start, mtx, &local->stCenterPoint);
            scale = local->xRadius;
            factor = POS_fn_xGetMaxScale(mtx);
            result = inv_SHW_fn_bSphereIntersectsProjectionVolume(out, start, scale * factor) != 0;
        }
    }
    return result;
}

s32 inv_SHW_fn_bAppendShadowReceiver(s32* a0, s32 a1, HIE_tdstSuperObject_* a2)
{
    extern s32 inv_SHW_fn_bSphereIntersectsProjectionVolume(s32 a0, void* dst, f32 scale);

    s32 buf[4];
    s32 result;
    GeometricObjectView* node;
    s32 obj;
    f32 scale;

    result = 0;

    if (a2->lTypeOfLinkedObject == HIE_TYPE_PHYSICAL_OBJECT_inv) {
        node = ((PO_tdstPhysicalObject_*) a2->hLinkedObject)->_hVisualSet;
    } else {
        node = 0;
    }

    if (node != 0) {
        scale = node->boundingSphereRadius;
        obj = (s32) a2->gu_hGlobalMatrix;
        if (0.0f < scale) {
            POS_fn_vMulMatrixVertex((MTH3D_tdstVector_*) buf, (void*) obj, &node->boundingSphereCenter);

            if (inv_SHW_fn_bSphereIntersectsProjectionVolume(
                    a1, buf, scale * POS_fn_xGetMaxScale((void*) obj)) != 0) {
                a0[0] = (s32) a2;
                result = 1;
                a0[1] = (s32) node;
            }
        }
    }

    return result;
}

void inv_SHW_fn_vCollectShadowReceiversRecursive(u8* out, s32* count, void* root,
                                                 HIE_tdstSuperObject_* node, void* arg)
{
    s32 active;
    u32 type;
    HIE_tdstSuperObject_* child;
    s32 child_count;
    s32 selected;

    *count = 0;
    active = 1;
    type = node->lTypeOfLinkedObject;
    switch (type) {
    case 2:
        if (root != node) {
            active = inv_HIE_fn_p_stGetEngineObject(node)->h_3dData->shadow.control.bytes[1] & 1;
        } else {
            active = 0;
        }
        break;
    case 8:
        selected = 0;
        if ((node->flags & 1) != 0) {
            selected = active;
        }
        if (selected == 0) {
            if (inv_SHW_fn_bAppendShadowReceiver(out, arg, node) != 0) {
                *count += 1;
                out += 8;
            }
        }
        break;
    default:
        break;
    }
    if (active != 0) {
        child = node->hFirstElementSta;
        while (child != 0) {
            inv_SHW_fn_vCollectShadowReceiversRecursive(out, &child_count, root, child, arg);
            *count += child_count;
            out += child_count * 8;
            child = child->hNextBrotherSta;
        }
    }
}

void inv_SHW_fn_vCollectSectorShadowReceivers(u8* out, s32* total, void* root,
                                              HIE_tdstSuperObject_* sector, void* arg)
{
    s32 count;
    SECT_tdstSector_* data;
    SECT_tdstListOfSOCharacters_* node;

    *total = 0;
    data = sector->hLinkedObject;
    node = data->stListOfCharacters.hFirstElementDyn;
    while (node != 0) {
        inv_SHW_fn_vCollectShadowReceiversRecursive(out, &count, root, node->hPointerOfCharacter,
                                                    arg);
        *total += count;
        out += count * 8;
        node = node->hNextBrotherDyn;
    }
}

s32 inv_SHW_fn_bSectorReferencesSector(HIE_tdstSuperObject_* sector, void* candidate)
{
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    u32 count;
    SECT_tdstSector_* data;
    if (sector == candidate)
        return 1;
    data = sector->hLinkedObject;
    count = data->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt;
    neighbor = data->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    while (count != 0) {
        if (candidate == neighbor->hPointerOfSectorSO)
            return 1;
        count--;
        neighbor++;
    }
    return 0;
}

void inv_SHW_fn_vCollectShadowReceiverGeometry(void* a0, s32* a1, void* superObject,
                                               HIE_tdstSuperObject_* a3, HIE_tdstSuperObject_* a4,
                                               void* a5)
{
    s32 cnt;
    s32 n;
    SECT_tdstListOfSOSectorsInGraphicInteraction_* p_stNeighbor;
    void* cur;
    u8* p;

    p = a0;
    *a1 = 0;
    if (a3 == a4) {
        SECT_tdstSector_* p_stSector;
        ((void (*)(void* a0, s32* a1, void* a2, void* a3,
                   void* a4)) inv_SHW_fn_vCollectSectorShadowReceivers)(p, &cnt, superObject, a4,
                                                                        a5);
        *a1 = *a1 + cnt;
        p = p + cnt * 8;
        p_stSector = a4->hLinkedObject;
        n = p_stSector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt;
        p_stNeighbor = p_stSector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
        while (n != 0) {
            cur = p_stNeighbor->hPointerOfSectorSO;
            if (inv_SHW_fn_bBoundingVolumeIntersectsProjection(a5, cur) != 0) {
                inv_SHW_fn_vCollectSectorShadowReceivers(p, &cnt, superObject, cur, a5);
                *a1 = *a1 + cnt;
                p = p + cnt * 8;
            }
            n--;
            p_stNeighbor++;
        }
    } else {
        SECT_tdstSector_* p_stSector;
        if (inv_SHW_fn_bSectorReferencesSector(a4, a3) != 0) {
            ((void (*)(void* a0, s32* a1, void* a2, void* a3,
                       void* a4)) inv_SHW_fn_vCollectSectorShadowReceivers)(p, &cnt, superObject,
                                                                            a3, a5);
            *a1 = *a1 + cnt;
            p = p + cnt * 8;
        }
        p_stSector = a3->hLinkedObject;
        n = p_stSector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt;
        p_stNeighbor = p_stSector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
        while (n != 0) {
            cur = p_stNeighbor->hPointerOfSectorSO;
            if (inv_SHW_fn_bBoundingVolumeIntersectsProjection(a5, cur) != 0) {
                if (cur == a3 || inv_SHW_fn_bSectorReferencesSector(a4, cur) != 0) {
                    inv_SHW_fn_vCollectSectorShadowReceivers(p, &cnt, superObject, cur, a5);
                    *a1 = *a1 + cnt;
                    p = p + cnt * 8;
                }
            }
            n--;
            p_stNeighbor++;
        }
    }
}

s32 inv_SHW_fn_bSphereIntersectsProjectionVolume(ShadowProjectionMatrixView* volume,
                                                 MTH3D_tdstVector_* center, f32 radius)
{
    MTH3D_tdstVector_ projected;
    MTH3D_tdstVector_ copy;
    MTH3D_tdstVector_* in = &copy;
    MTH3D_tdstVector_* p_stProjected = &projected;
    f32 squaredLength;
    f32 limit;
    s32 result;

    if (center == p_stProjected) {
        *in = *center;
        R5900_TRANSFORM_POINT_inv(p_stProjected, in, &volume->stInvMatrix);
    } else {
        R5900_TRANSFORM_POINT_inv(p_stProjected, center, &volume->stInvMatrix);
    }
    if (projected.xZ > 1.0f)
        projected.xZ -= 1.0f;
    else if (projected.xZ > 0.0f)
        projected.xZ = 0.0f;
    result = 1;
    limit = radius * volume->fInvMinScale + 1.0f;
    squaredLength = MTH_fn_xSqrAddSqrAddSqrOpt2(projected.xX, projected.xY, projected.xZ);
    if (!(squaredLength < limit * limit))
        result = 0;
    return result;
}

s32 inv_SHW_fn_bSegmentBoundsIntersectsProjectionVolume(void* volume, const f32* start,
                                                        const f32* end)
{
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ center;
    f32 squaredLength;

    MTH3D_M_vSetXYZ_inv(&center, (start[0] + end[0]) * 0.5f, (start[1] + end[1]) * 0.5f,
                        (start[2] + end[2]) * 0.5f);
    MTH3D_M_vSetXYZ_inv(&delta, end[0] - start[0], end[1] - start[1], end[2] - start[2]);
    squaredLength = MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ);
    return inv_SHW_fn_bSphereIntersectsProjectionVolume(volume, &center,
                                                        MTH_fn_xSqrtOpt2(squaredLength) * 0.5f);
}

void inv_SHW_fn_vResetProjectionGlobals(void)
{
    SAI_g_stInternalStructure.p_stFirstEntry = NULL;
    D_0039358C = 0;
}
