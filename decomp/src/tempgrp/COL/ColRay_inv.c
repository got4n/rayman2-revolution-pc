#include "common.h"

#include "COL/COL.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/dma.h"
#include "ps2/types.h"
#include "ps2/vu0.h"
#include "sdk.h"

#include "include_asm.h"
#include "ps2/PS2.h"

#ifdef PC_PORT
#define SCRATCH_ADDRESS PC_SCRATCH(0x70003FF0)
#else
#define SCRATCH_ADDRESS 0x70003FF0
#endif
#define PACKED_WIDE_MASK 0x7FFF
#define COLLIDE_TRIANGLES 1
#define COLLIDE_SPHERES 7
#define SECTOR_FLAG_TRIANGLES 0x20000
#define SECTOR_FLAG_SPHERES 0x80000

typedef union PackedByte {
    s8 tag;
    u8 value;
} PackedByte;

typedef struct COL_RAY_tdstPacked16 {
    u64 first;
    u64 second;
} __attribute__((packed, aligned(4))) COL_RAY_tdstPacked16;

typedef union VuScratch {
    struct {
        MTH3D_tdstVector_ xyz;
        u32 w;
    } vector;

    COL_RAY_tdstPacked16 row;
    f32 values[4];
} VuScratch;

static inline void inv_COL_fn_vCOLRAYVReadPacked(PackedByte** cursor, s16* out)
{
    if ((*cursor)->tag >= 0) {
        *out = (*cursor)->value;
        *cursor += 1;
    } else {
        u32 high = (*cursor)->value;
        u32 low;

        *cursor += 1;
        low = (*cursor)->value;
        *cursor += 1;
        *out = ((high << 8) + low) & PACKED_WIDE_MASK;
    }
}

extern f32 inv_COL_rayRadius;
extern f32 inv_COL_rayHitTime0;
extern s16 inv_COL_rayParameter2;
extern s16 inv_COL_rayHitType0;
extern volatile s16 inv_COL_rayHitType1;
extern f32 inv_COL_rayHitTime1;
extern GMT_tdstGameMaterial_* inv_COL_rayDynamicMaterial;
extern GMT_tdstGameMaterial_* inv_COL_rayStaticMaterial;
extern void* inv_COL_rayParameter1;
extern s32 inv_COL_rayHitBufferCount;
extern u8 inv_COL_bRaySphereChanged;
extern u8 dword_250EB0[];
extern u8 xmmword_386630[];
extern u8 xmmword_386650[];
extern u8 xmmword_386670[];
extern u8 xmmword_386690[];
extern u8 xmmword_3866B0[];
extern u8 xmmword_386640[];
extern u8 xmmword_386660[];
extern u8 xmmword_386680[];
extern u8 xmmword_3866A0[];
extern u8 xmmword_3866C0[];

void COL_fn_vAddInStaticCollisionTable(f32, void*, void*, void*, GMT_tdstGameMaterial_*,
                                       GMT_tdstGameMaterial_*, s16, s16, void*, f32, s16, void*,
                                       void*);
s32 func_0020CAD8(void);
void inv_COL_RAY_fn_vComputeSegmentLength(COL_tdstGVForCollision_* p_stGV);
void inv_COL_RAY_fn_vTestPortalTriangle(COL_tdstGVForCollision_* p_stGV);
void inv_COL_RAY_fn_lProcessTriangleEdge(COL_tdstGVForCollision_* p_stGV);
s32 inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(
    MTH3D_tdstVector_* origin, MTH3D_tdstVector_* direction, MTH3D_tdstVector_* center,
    f32 radius, f32* outT, MTH3D_tdstVector_* intersection, MTH3D_tdstVector_* outNormal);
void inv_COL_fn_vFindSectorsInBox(COL_tdstOctree_* map, f32* minimum, f32* maximum,
                                  COL_tdstOctreeNode_** sectors, s16* count);

extern f32 D_00393174;

void inv_COL_RAY_fn_vComputeSegmentLength(COL_tdstGVForCollision_* p_stGV)
{
    f32 minimum = D_00393174;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 squaredLength;
    f32 length;
    f32 radius;
    f32 halfSpan;
    f32 scale;
    f32 offset[3];

    p_stGV->xSwapRadius = p_stGV->xDynamicRadius;
    p_stGV->stSwapDinST0Point = p_stGV->stDinST0Point;
    dx = p_stGV->stDinST01Vector.xX;
    dy = p_stGV->stDinST01Vector.xY;
    dz = p_stGV->stDinST01Vector.xZ;
    squaredLength = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    if (squaredLength < minimum) {
        p_stGV->bUseEnlargeSphere = 0;
        return;
    }
    radius = p_stGV->xDynamicRadius;
    length = MTH_fn_xSqrtOpt2(squaredLength);
    halfSpan = (length + (radius + radius)) * 0.5f;
    p_stGV->bUseEnlargeSphere = 1;
    p_stGV->xDynamicRadius = halfSpan;
    scale = (halfSpan - radius) / length;
    offset[0] = scale * dx;
    offset[1] = scale * dy;
    offset[2] = scale * dz;
    p_stGV->stDinST0Point.xX += offset[0];
    p_stGV->stDinST0Point.xY += offset[1];
    p_stGV->stDinST0Point.xZ += offset[2];
}

void inv_COL_RAY_fn_vRestoreSegmentOrder(COL_tdstGVForCollision_* p_stGV)
{
    f32 tmp;
    MTH3D_tdstVector_ tmpv;

    if (p_stGV->bUseEnlargeSphere != 0) {
        tmp = p_stGV->xDynamicRadius;
        p_stGV->xDynamicRadius = p_stGV->xSwapRadius;
        p_stGV->xSwapRadius = tmp;
        tmpv = p_stGV->stDinST0Point;
        p_stGV->stDinST0Point = p_stGV->stSwapDinST0Point;
        p_stGV->stSwapDinST0Point = tmpv;
        p_stGV->bUseEnlargeSphere = 0;
    }
}

void func_0020C5E0(COL_tdstGVForCollision_* p_stGV)
{
    f32 tmp;
    MTH3D_tdstVector_ tmpv;

    if (p_stGV->bUseEnlargeSphere == 0) {
        tmp = p_stGV->xDynamicRadius;
        p_stGV->xDynamicRadius = p_stGV->xSwapRadius;
        p_stGV->xSwapRadius = tmp;
        tmpv = p_stGV->stDinST0Point;
        p_stGV->stDinST0Point = p_stGV->stSwapDinST0Point;
        p_stGV->stSwapDinST0Point = tmpv;
        p_stGV->bUseEnlargeSphere = 1;
    }
}

static inline MTH3D_tdstVector_* inv_COL_fn_p_stPointAt(MTH3D_tdstVector_* points, s32 offset)
{
    void* start = points;
    u8* bytes = start;
    void* point = bytes + offset;

    return point;
}

static inline PackedByte* inv_COL_fn_p_stPackedFaces(COL_tdstOctreeNode_* sector)
{
    void* faces = sector->d_xFaceIndexList + 1;

    return faces;
}

void inv_COL_RAY_fn_vInitHitBuffer(void)
{
#ifdef PC_PORT
    inv_PS2_fn_vSendVif0Dma(dword_250EB0);
#else
    inv_PS2_fn_vInitVif0Dma();
    inv_PS2_fn_vSendVif0Dma(dword_250EB0);
    while (inv_PS2_fn_lIsVif0DmaBusy() != 0) {
    }
#endif
    inv_COL_rayHitBufferCount = 0;
}

void inv_COL_RAY_fn_vFlushHitBuffer(void)
{
    u32 i;

    if (inv_COL_rayHitBufferCount != 0) {
        for (i = 0; i < inv_COL_rayHitBufferCount; i++) {
            COL_fn_vAddInStaticCollisionTable(
                (&inv_COL_rayHitTime0)[i], &xmmword_386670[i * 0x10], &xmmword_386630[i * 0x10],
                &xmmword_386650[i * 0x10], inv_COL_rayDynamicMaterial, inv_COL_rayStaticMaterial, 3,
                (&inv_COL_rayHitType0)[i], inv_COL_rayParameter1, inv_COL_rayRadius,
                inv_COL_rayParameter2, &xmmword_3866B0[i * 0x10], &xmmword_386690[i * 0x10]);
        }
        inv_COL_rayHitBufferCount = 0;
    }
}

void inv_COL_RAY_fn_lProcessTriangleEdge(COL_tdstGVForCollision_* p_stGV)
{
    VuScratch* scratch;
    GMT_tdstGameMaterial_* hDynMat;
    GMT_tdstGameMaterial_* hStatMat;
    void* p_vParameter1;
    u16 uwParameter2;
    f32 xRadius;
    COL_tdstIndexedSphere_* p_stDynamicSphere;

    scratch = (VuScratch*) SCRATCH_ADDRESS;

    scratch->vector.xyz =
        p_stGV->p_stStaticColObj->d_stListOfPoints[p_stGV->p_stStaticElementIndexedTriangles->d_stListOfFacesTripled[p_stGV->xStaticIndexedTriangleIndex].a3_xIndex[0]];
    scratch->vector.w = 0;
    SCE_VU0_LQC2(3, scratch);

    scratch->vector.xyz =
        p_stGV->p_stStaticColObj->d_stListOfPoints[p_stGV->p_stStaticElementIndexedTriangles->d_stListOfFacesTripled[p_stGV->xStaticIndexedTriangleIndex].a3_xIndex[1]];
    scratch->vector.w = 0;
    SCE_VU0_LQC2(4, scratch);

    scratch->vector.xyz =
        p_stGV->p_stStaticColObj->d_stListOfPoints[p_stGV->p_stStaticElementIndexedTriangles->d_stListOfFacesTripled[p_stGV->xStaticIndexedTriangleIndex].a3_xIndex[2]];
    scratch->vector.w = 0;
    SCE_VU0_LQC2(5, scratch);

    if (inv_COL_bRaySphereChanged != 0) {
        if (p_stGV->bUseEnlargeSphere != 0) {
            scratch->vector.xyz = p_stGV->stDinST0Point;
            scratch->vector.w = 0;
            SCE_VU0_LQC2(16, scratch);
            scratch->vector.xyz = p_stGV->stSwapDinST0Point;
            scratch->vector.w = 0;
            SCE_VU0_LQC2(27, scratch);
        } else {
            scratch->vector.xyz = p_stGV->stSwapDinST0Point;
            scratch->vector.w = 0;
            SCE_VU0_LQC2(16, scratch);
            scratch->vector.xyz = p_stGV->stDinST0Point;
            scratch->vector.w = 0;
            SCE_VU0_LQC2(27, scratch);
        }
        scratch->vector.xyz = p_stGV->stDinST1Point;
        scratch->vector.w = 0;
        SCE_VU0_LQC2(17, scratch);
    }

    if (inv_COL_bRayStaticMatrixChanged != 0) {
        scratch->row = *(COL_RAY_tdstPacked16*) &p_stGV->p_stStaticGeomObjMatrix->stMatrix.stCol_0;
        SCE_VU0_LQC2(20, scratch);
        scratch->row = *(COL_RAY_tdstPacked16*) &p_stGV->p_stStaticGeomObjMatrix->stMatrix.stCol_1;
        SCE_VU0_LQC2(21, scratch);
        scratch->row = *(COL_RAY_tdstPacked16*) &p_stGV->p_stStaticGeomObjMatrix->stMatrix.stCol_2;
        SCE_VU0_LQC2(22, scratch);
        scratch->row = *(COL_RAY_tdstPacked16*) &p_stGV->p_stStaticGeomObjMatrix->stMatrix.stCol_3;
        SCE_VU0_LQC2(23, scratch);
    }

    if (inv_COL_bRaySphereChanged != 0 || inv_COL_bRayStaticMatrixChanged != 0) {
        if (p_stGV->bUseEnlargeSphere != 0) {
            scratch->values[0] = p_stGV->xDynamicRadius;
            scratch->values[1] = p_stGV->xSwapRadius;
        } else {
            scratch->values[0] = p_stGV->xSwapRadius;
            scratch->values[1] = p_stGV->xDynamicRadius;
        }
        scratch->values[2] = p_stGV->p_stStaticGeomObjMatrix->stScale.xX;
        SCE_VU0_LQC2(26, scratch);
    }
    inv_COL_bRaySphereChanged = 0;
    inv_COL_bRayStaticMatrixChanged = 0;

    SCE_VU0_CALLMS_IMM(0x0);
    inv_COL_RAY_fn_vFlushHitBuffer();

    p_stDynamicSphere = p_stGV->p_stDynamicIndexedSphere;
    xRadius = p_stDynamicSphere->xRadius;
    hStatMat = p_stGV->hStaticMaterial;
    p_vParameter1 = p_stGV->p_vParameter1;
    uwParameter2 = p_stGV->sParameter2;
    hDynMat = p_stGV->hDynamicMaterial;
    inv_COL_rayDynamicMaterial = hDynMat;
    inv_COL_rayStaticMaterial = hStatMat;
    inv_COL_rayParameter1 = p_vParameter1;
    inv_COL_rayParameter2 = uwParameter2;
    inv_COL_rayRadius = xRadius;

    while (inv_PS2_fn_bIsVu0Busy() != 0) {
    }

    func_0020CAD8();
}

s32 func_0020CAD8(void)
{
    void *vu;
    u32 v5;
    u32 v4;
    u32 v6;
    f32 val;

#ifdef PC_PORT
    vu = PC_SCRATCH(0x70003FF0);
#else
    vu = (void *)0x70003FF0;
#endif
    VU0_CFC2_NI(v5, 5);
    inv_COL_rayHitBufferCount = v5;
    if (v5 == 0)
        return 0;
    VU0_SQC2(15, vu, 0);
#ifdef PC_PORT
    val = *(f32 *)PC_SCRATCH(0x70003FF0);
#else
    val = *(f32 *)0x70003FF0;
#endif
    inv_COL_rayHitTime0 = val;
    VU0_CFC2_I(v4, 4);
    switch (v4) {
    case 1:
        inv_COL_rayHitType0 = v4;
        break;
    case 0:
        inv_COL_rayHitType0 = 7;
        break;
    case 2:
        inv_COL_rayHitType0 = 8;
        break;
    }
    VU0_SQC2(14, xmmword_386630, 0);
    VU0_SQC2(24, xmmword_386650, 0);
    VU0_SQC2(25, xmmword_386690, 0);
    VU0_SQC2(31, xmmword_386670, 0);
    VU0_SQC2(9, xmmword_3866B0, 0);
    if (inv_COL_rayHitBufferCount == 1)
        return 1;
    VU0_SQC2(1, vu, 0);
    inv_COL_rayHitTime1 = val;
    VU0_CFC2_NI(v6, 6);
    switch (v6) {
    case 1:
        inv_COL_rayHitType1 = v6;
        break;
    case 0:
        inv_COL_rayHitType1 = 7;
        break;
    case 2:
        inv_COL_rayHitType1 = 8;
        break;
    }
    VU0_SQC2(2, xmmword_386640, 0);
    VU0_SQC2(3, xmmword_386660, 0);
    VU0_SQC2(4, xmmword_3866A0, 0);
    VU0_SQC2(5, xmmword_386680, 0);
    VU0_SQC2(6, xmmword_3866C0, 0);
    if ((u8)v6 & 0x100) {
        inv_COL_rayHitType1 = v6;
        inv_COL_rayHitBufferCount += (s32)vu;
        inv_COL_rayHitBufferCount += (s32)vu;
        inv_COL_rayHitBufferCount += (s32)vu;
        inv_COL_rayHitBufferCount += (s32)vu;
        inv_COL_rayHitBufferCount += (s32)vu;
        inv_COL_rayHitBufferCount += (s32)vu;
    }
    return 2;
}

void inv_COL_RAY_fn_vTestSectorTriangles(COL_tdstGVForCollision_* p_stGV)
{
    s16 edge;

    for (edge = 0; edge < p_stGV->p_stDynamicElementSpheres->xNbSpheres; edge++) {
        inv_COL_bRaySphereChanged = 1;
        p_stGV->p_stDynamicIndexedSphere =
            &p_stGV->p_stDynamicElementSpheres->d_stListOfSpheres[edge];
        p_stGV->p_stDynamicCenter =
            &p_stGV->p_stDynamicColObj->d_stListOfPoints[p_stGV->p_stDynamicIndexedSphere->xCenterPoint];
        POS_fn_vMulMatrixVertex(&p_stGV->stDinST0Point, &p_stGV->stD2ST0TransformMatrix,
                                p_stGV->p_stDynamicCenter);
        POS_fn_vMulMatrixVertex(&p_stGV->stDinST1Point, &p_stGV->stD2ST1TransformMatrix,
                                p_stGV->p_stDynamicCenter);
        MTH3D_M_vSubXYZ_inv(&p_stGV->stDinST01Vector, &p_stGV->stDinST1Point,
                            &p_stGV->stDinST0Point);
        p_stGV->xDynamicRadius = p_stGV->p_stDynamicIndexedSphere->xRadius / p_stGV->xStaticScale;
        inv_COL_RAY_fn_vComputeSegmentLength(p_stGV);
        for (p_stGV->xStaticIndexedTriangleIndex = 0;
             p_stGV->xStaticIndexedTriangleIndex < p_stGV->p_stStaticElementIndexedTriangles->xNbFaces;
             p_stGV->xStaticIndexedTriangleIndex++) {
            p_stGV->hDynamicMaterial = p_stGV->p_stDynamicIndexedSphere->hMaterial;
            p_stGV->hStaticMaterial = p_stGV->p_stStaticElementIndexedTriangles->hMaterial;
            inv_COL_RAY_fn_lProcessTriangleEdge(p_stGV);
        }
    }
}

void inv_COL_RAY_fn_vTestPortalTriangle(COL_tdstGVForCollision_* p_stGV)
{
    MTH3D_tdstVector_ invDelta;
    MTH3D_tdstVector_ deltaCenter;
    MTH3D_tdstVector_ normal;
    MTH3D_tdstVector_ centerWorld;
    MTH3D_tdstVector_ sep;
    MTH3D_tdstVector_ posWorld;
    MTH3D_tdstVector_ hit;
    MTH3D_tdstVector_ hitNormal;
    MTH3D_tdstVector_ hitWorld;
    MTH3D_tdstVector_ normalWorld;
    MTH3D_tdstVector_ zero;
    MTH3D_tdstVector_ pos14World;
    f32 hitT;
    COL_tdstIndexedSphere_* rec;
    f32 radiusSum;
    f32 len2;
    f32 len;
    f32 invLen;
    f32 scale;
    f32 ratio;
    s32 n;

    n = 12;
    rec = p_stGV->p_stStaticIndexedSphere;
    if (!INT_fn_bGetInclusionPointInSphere(
            &p_stGV->stDinST0Point, (p_stGV->p_stStaticCenter = inv_COL_fn_p_stPointAt(p_stGV->p_stStaticColObj->d_stListOfPoints, rec->xCenterPoint * n)),
            p_stGV->xDynamicRadius + rec->xRadius))
        return;

    MTH3D_M_vSetXYZ_inv(&invDelta, inv_MTH_fn_xNegateTied(p_stGV->stDinST01Vector.xX),
                        inv_MTH_fn_xNegateTied(p_stGV->stDinST01Vector.xY),
                        inv_MTH_fn_xNegateTied(p_stGV->stDinST01Vector.xZ));
    inv_COL_RAY_fn_vRestoreSegmentOrder(p_stGV);

    radiusSum = p_stGV->xDynamicRadius + p_stGV->p_stStaticIndexedSphere->xRadius;

    if (INT_fn_bGetInclusionPointInSphere(&p_stGV->stDinST1Point, p_stGV->p_stStaticCenter, radiusSum)) {
        MTH3D_M_vSubXYZ_inv(&deltaCenter, &p_stGV->stDinST0Point, &(*p_stGV->p_stStaticCenter));
        POS_fn_vApplyMatrixToVector(&deltaCenter, p_stGV->p_stStaticGeomObjMatrix, &deltaCenter);

        len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(deltaCenter.xX, deltaCenter.xY,
                                           deltaCenter.xZ);
        len = MTH_fn_xSqrtOpt2(len2);
        if (len != 0.0f) {
            invLen = 1.0f / len;
            MTH3D_M_vKTimesXYZ_inv(&normal, invLen, &deltaCenter);
        } else {
            normal = deltaCenter;
        }

        POS_fn_vMulMatrixVertex(&centerWorld, p_stGV->p_stStaticGeomObjMatrix,
                                p_stGV->p_stStaticCenter);

        MTH3D_M_vSetXYZ_inv(&centerWorld,
                            p_stGV->p_stStaticIndexedSphere->xRadius * normal.xX + centerWorld.xX,
                            p_stGV->p_stStaticIndexedSphere->xRadius * normal.xY + centerWorld.xY,
                            p_stGV->p_stStaticIndexedSphere->xRadius * normal.xZ + centerWorld.xZ);

        scale = radiusSum + 0.001f;
        MTH3D_M_vSetXYZ_inv(&sep, scale * normal.xX - deltaCenter.xX,
                            scale * normal.xY - deltaCenter.xY, scale * normal.xZ - deltaCenter.xZ);

        POS_fn_vApplyMatrixToVector(&invDelta, p_stGV->p_stStaticGeomObjMatrix, &invDelta);
        POS_fn_vMulMatrixVertex(&posWorld, p_stGV->p_stStaticGeomObjMatrix, &p_stGV->stDinST0Point);

        COL_fn_vAddInStaticCollisionTable(-1.0f, &sep, &centerWorld, &normal,
                                          p_stGV->hDynamicMaterial, p_stGV->hStaticMaterial, 3, 3,
                                          p_stGV->p_vParameter1,
                                          p_stGV->p_stDynamicIndexedSphere->xRadius,
                                          p_stGV->sParameter2, &invDelta, &posWorld);
        return;
    }

    POS_fn_vMulMatrixVertex(&pos14World, p_stGV->p_stStaticGeomObjMatrix, &p_stGV->stDinST0Point);
    if (!inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(
            &p_stGV->stDinST1Point, &invDelta, p_stGV->p_stStaticCenter, radiusSum, &hitT, &hit, &hitNormal))
        return;

    ratio = p_stGV->xDynamicRadius / radiusSum;
    MTH3D_M_vSetXYZ_inv(&hit, hit.xX + ratio * (p_stGV->p_stStaticCenter->xX - hit.xX),
                        hit.xY + ratio * (p_stGV->p_stStaticCenter->xY - hit.xY),
                        hit.xZ + ratio * (p_stGV->p_stStaticCenter->xZ - hit.xZ));

    POS_fn_vMulMatrixVertex(&hitWorld, p_stGV->p_stStaticGeomObjMatrix, &hit);
    POS_fn_vRotateVector(&normalWorld, p_stGV->p_stStaticGeomObjMatrix, &hitNormal);

    zero.xZ = zero.xY = zero.xX = 0.0f;
    POS_fn_vApplyMatrixToVector(&invDelta, p_stGV->p_stStaticGeomObjMatrix, &invDelta);

    COL_fn_vAddInStaticCollisionTable(hitT, &zero, &hitWorld, &normalWorld,
                                      p_stGV->hDynamicMaterial, p_stGV->hStaticMaterial, 3, 3,
                                      p_stGV->p_vParameter1,
                                      p_stGV->p_stDynamicIndexedSphere->xRadius,
                                      p_stGV->sParameter2, &invDelta, &pos14World);
}

void inv_COL_RAY_fn_vTestSectorTriangles(COL_tdstGVForCollision_* p_stGV);

void inv_COL_RAY_fn_vTestSectorPortals(COL_tdstGVForCollision_* p_stGV)
{
    s16 i;
    s16 j;
    COL_tdstElementSpheres_* edges;
    COL_tdstElementSpheres_* spheres;
    COL_tdstIndexedSphere_* entry;
    COL_tdstIndexedSphere_* sphere;

    for (i = 0; i < p_stGV->p_stDynamicElementSpheres->xNbSpheres; i++) {
        edges = p_stGV->p_stDynamicElementSpheres;
        entry = &edges->d_stListOfSpheres[i];
        p_stGV->p_stDynamicIndexedSphere = entry;
        p_stGV->p_stDynamicCenter =
            &p_stGV->p_stDynamicColObj->d_stListOfPoints[entry->xCenterPoint];
        p_stGV->xDynamicRadius = entry->xRadius / p_stGV->xStaticScale;
        POS_fn_vMulMatrixVertex(&p_stGV->stDinST0Point, &p_stGV->stD2ST0TransformMatrix,
                                p_stGV->p_stDynamicCenter);
        POS_fn_vMulMatrixVertex(&p_stGV->stDinST1Point, &p_stGV->stD2ST1TransformMatrix,
                                p_stGV->p_stDynamicCenter);
        MTH3D_M_vSubXYZ_inv(&p_stGV->stDinST01Vector, &p_stGV->stDinST1Point,
                            &p_stGV->stDinST0Point);
        inv_COL_RAY_fn_vComputeSegmentLength(p_stGV);
        spheres = p_stGV->p_stStaticElementSpheres;
        for (j = 0; j < spheres->xNbSpheres; j++) {
            spheres = p_stGV->p_stStaticElementSpheres;
            sphere = &spheres->d_stListOfSpheres[j];
            p_stGV->p_stStaticIndexedSphere = sphere;
            p_stGV->hDynamicMaterial = p_stGV->p_stDynamicIndexedSphere->hMaterial;
            p_stGV->hStaticMaterial = sphere->hMaterial;
            inv_COL_RAY_fn_vTestPortalTriangle(p_stGV);
            spheres = p_stGV->p_stStaticElementSpheres;
        }
    }
}

void inv_COL_RAY_fn_vTraverseSectorGraph(COL_tdstGVForCollision_* _p_stGV)
{
    f32 minBox[4];
    f32 maxBox[4];
    s16* pCount;
    MTH3D_tdstVector_* p14;
    MTH3D_tdstVector_* p28;
    POS_tdstMatrix_* p120;
    POS_tdstMatrix_* pCC;
    COL_tdstOctreeNode_** sectors;
    f32* maxPtr;
    COL_tdstIndexedSphere_* rec;
    COL_tdstOctreeNode_* sect;
    PackedByte* packed;
    s8 tag;
    s16 edge;
    s16 sector;
    s16 pairCount;
    u8 pair;
    u32 high;
    u32 face;
    u32 low;
    s16 kind;
    s32 reserved0;
    s32 reserved1;
    s32 reserved2;
    s32 reserved3;
    s32 reserved4;
    COL_tdstGVForCollision_* p_stGV = _p_stGV;

    for (edge = 0; edge < p_stGV->p_stDynamicElementSpheres->xNbSpheres; edge++) {
        p14 = &p_stGV->stDinST0Point;
        maxPtr = maxBox;
        sectors = p_stGV->aDEF_pstSelectedNode;
        pCC = &p_stGV->stD2ST0TransformMatrix;
        p28 = &p_stGV->stDinST1Point;
        p120 = &p_stGV->stD2ST1TransformMatrix;
        pCount = &p_stGV->xNumberOfSelectedNodes;
        inv_COL_bRaySphereChanged = 1;
        rec = &p_stGV->p_stDynamicElementSpheres->d_stListOfSpheres[edge];
        p_stGV->p_stDynamicIndexedSphere = rec;
        p_stGV->p_stDynamicCenter = &p_stGV->p_stDynamicColObj->d_stListOfPoints[rec->xCenterPoint];
        p_stGV->xDynamicRadius = rec->xRadius / p_stGV->xStaticScale;
        POS_fn_vMulMatrixVertex(p14, pCC, p_stGV->p_stDynamicCenter);
        POS_fn_vMulMatrixVertex(p28, p120, p_stGV->p_stDynamicCenter);
        MTH3D_M_vSubXYZ_inv(&p_stGV->stDinST01Vector, &p_stGV->stDinST1Point,
                            &p_stGV->stDinST0Point);
        inv_COL_RAY_fn_vComputeSegmentLength(p_stGV);
        COL_fn_vComputeBoundingBoxOfSphere((MTH3D_tdstVector_*) minBox, (MTH3D_tdstVector_*) maxPtr, p14, p_stGV->xDynamicRadius);
        inv_COL_fn_vFindSectorsInBox(p_stGV->p_stOctree, minBox, maxPtr, sectors, pCount);
        COL_g_lFacesTagCounter += 1;
        for (sector = 0; sector < p_stGV->xNumberOfSelectedNodes; sector++) {
            sect = p_stGV->aDEF_pstSelectedNode[sector];
            packed = inv_COL_fn_p_stPackedFaces(sect);
            pairCount = sect->d_xFaceIndexList[0];
            for (pair = 0; pair < pairCount; pair = (pair + 1) & 0xFF) {
                inv_COL_fn_vCOLRAYVReadPacked(&packed, &p_stGV->xStaticElementIndex);
                tag = packed->tag;
                if (tag >= 0) {
                    face = packed->value;
                    packed += 1;
                } else {
                    high = packed->value;
                    packed += 1;
                    low = packed->value;
                    packed += 1;
                    face = ((high << 8) + low) & PACKED_WIDE_MASK;
                }
                if (COL_g_d_lTaggedFacesTable
                        [p_stGV->p_stOctree->d_xElementBasesTable[p_stGV->xStaticElementIndex] +
                         face] != COL_g_lFacesTagCounter) {
                    kind =
                        p_stGV->p_stStaticColObj->d_xListOfElementsTypes[p_stGV->xStaticElementIndex];
                    switch (kind) {
                    case COLLIDE_TRIANGLES:
                        p_stGV->p_stStaticElementIndexedTriangles =
                            p_stGV->p_stStaticColObj->d_stListOfElements[p_stGV->xStaticElementIndex];
                        p_stGV->xStaticIndexedTriangleIndex = face;
                        p_stGV->hDynamicMaterial = p_stGV->p_stDynamicIndexedSphere->hMaterial;
                        p_stGV->hStaticMaterial =
                            p_stGV->p_stStaticElementIndexedTriangles->hMaterial;
                        inv_COL_RAY_fn_lProcessTriangleEdge(p_stGV);
                        break;
                    case COLLIDE_SPHERES:
                        if (p_stGV->ulSelectedCollisionCases & SECTOR_FLAG_SPHERES) {
                            p_stGV->p_stStaticElementSpheres =
                                p_stGV->p_stStaticColObj->d_stListOfElements[p_stGV->xStaticElementIndex];
                            p_stGV->hDynamicMaterial = p_stGV->p_stDynamicIndexedSphere->hMaterial;
                            p_stGV->hStaticMaterial = p_stGV->p_stStaticIndexedSphere->hMaterial;
                            inv_COL_RAY_fn_vTestPortalTriangle(p_stGV);
                        }
                        break;
                    }
                    COL_g_d_lTaggedFacesTable
                        [p_stGV->p_stOctree->d_xElementBasesTable[p_stGV->xStaticElementIndex] +
                         face] = COL_g_lFacesTagCounter;
                }
            }
        }
    }
}

void inv_COL_RAY_fn_vCastRayThroughSectors(COL_tdstGVForCollision_* p_stGV)
{
    if (p_stGV->p_stOctree == NULL) {
        for (p_stGV->xStaticElementIndex = 0;
             p_stGV->xStaticElementIndex < p_stGV->p_stStaticColObj->xNbElements;
             p_stGV->xStaticElementIndex++) {
            switch (p_stGV->p_stStaticColObj->d_xListOfElementsTypes[p_stGV->xStaticElementIndex]) {
            case COLLIDE_TRIANGLES:
                if (p_stGV->ulSelectedCollisionCases & SECTOR_FLAG_TRIANGLES) {
                    p_stGV->p_stStaticElementIndexedTriangles =
                        p_stGV->p_stStaticColObj->d_stListOfElements[p_stGV->xStaticElementIndex];
                    inv_COL_RAY_fn_vTestSectorTriangles(p_stGV);
                }
                break;
            case COLLIDE_SPHERES:
                if (p_stGV->ulSelectedCollisionCases & SECTOR_FLAG_SPHERES) {
                    p_stGV->p_stStaticElementSpheres =
                        p_stGV->p_stStaticColObj->d_stListOfElements[p_stGV->xStaticElementIndex];
                    inv_COL_RAY_fn_vTestSectorPortals(p_stGV);
                }
                break;
            }
        }
    } else if (p_stGV->ulSelectedCollisionCases & 0x1E0000) {
        inv_COL_RAY_fn_vTraverseSectorGraph(p_stGV);
    }
}
