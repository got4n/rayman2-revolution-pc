#include "common.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

extern f32 flt_3934D8[];
extern f32 flt_3934DC[];
extern f32 unk_394F28[];
extern f32 unk_394F38[];

s32 inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
    MTH3D_tdstVector_* origin, MTH3D_tdstVector_* direction, MTH3D_tdstVector_* v0,
    MTH3D_tdstVector_* v1, MTH3D_tdstVector_* v2, f32* outT, f32* outPoint);

s32 inv_INT_fn_bIntersectSegmentWithTriangleFrontFace(EffectParticle_inv* object,
                                                      MTH3D_tdstVector_* point,
                                                      CollisionShape* shape, f32* outDistance,
                                                      MTH3D_tdstVector_* outNormal)
{
    MTH3D_tdstVector_ direction;
    f32 hitT;
    f32 distance;

    if (inv_MEC_fn_bIsPointBehindPlane(&shape->points[4], &shape->points[0], point) == 0) {
        return 0;
    }
    MTH3D_M_vSubXYZ_inv(&direction, point, &object->position);
    if (inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
            &object->position, &direction, &shape->points[0], &shape->points[1],
            &shape->points[2], &hitT, unk_394F28) == 0 &&
        inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
            &object->position, &direction, &shape->points[0], &shape->points[2],
            &shape->points[3], &hitT, unk_394F28) == 0) {
        return 0;
    }
    distance = inv_MEC_fn_xGetParticlePlaneDistance(object, &shape->points[0], &shape->points[4]);
    if (distance == flt_3934D8[0]) {
        return 0;
    }
    if (inv_MEC_fn_bMovesTowardPlane(object, &shape->points[4], distance) == 0) {
        return 0;
    }
    *outNormal = shape->points[4];
    *outDistance = distance;
    return 1;
}

s32 inv_INT_fn_bIntersectSegmentWithTriangleBackFace(EffectParticle_inv* object,
                                                     MTH3D_tdstVector_* point,
                                                     CollisionShape* shape,
                                                     f32* outDistance,
                                                     MTH3D_tdstVector_* outNormal)
{
    MTH3D_tdstVector_ direction;
    f32 hitT[3];
    f32 distance;

    if (inv_MEC_fn_bIsPointBehindPlane(&shape->points[3], &shape->points[0], point) == 0) {
        return 0;
    }
    MTH3D_M_vSubXYZ_inv(&direction, point, &object->position);
    if (inv_INT_fn_bDetectIntersectSegmentWithTriangleDetailed(
            &object->position, &direction, &shape->points[0], &shape->points[1],
            &shape->points[2], hitT, unk_394F38) == 0) {
        return 0;
    }
    distance = inv_MEC_fn_xGetParticlePlaneDistance(object, &shape->points[0], &shape->points[3]);
    if (distance == flt_3934DC[0]) {
        return 0;
    }
    if (inv_MEC_fn_bMovesTowardPlane(object, &shape->points[3], distance) == 0) {
        return 0;
    }
    *outNormal = shape->points[3];
    *outDistance = distance;
    return 1;
}

typedef struct IntersectionSphere {
    MTH3D_tdstVector_ center;
    f32 radius;
} IntersectionSphere;

extern f32 flt_3934E0[];
extern MTH3D_tdstVector_ inv_INT_stSphereHitPoint;

s32 inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(MTH3D_tdstVector_* origin,
                                                         MTH3D_tdstVector_* direction,
                                                         IntersectionSphere* sphere, f32* outT,
                                                         MTH3D_tdstVector_* hit,
                                                         MTH3D_tdstVector_* normal, f32 radius);

s32 inv_INT_fn_bIntersectSegmentWithSphere(EffectParticle_inv* particle,
                                           MTH3D_tdstVector_* point, IntersectionSphere* sphere,
                                           f32* outDistance, MTH3D_tdstVector_* outVector)
{
    MTH3D_tdstVector_ segment;
    MTH3D_tdstVector_ delta;
    MTH3D_tdstVector_ normal;
    f32 t;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 len;
    f32 distance;
    f32 sx;
    f32 radius;
    f32 ex;
    f32 ey;
    f32 ez;
    f32 sy;
    f32 sz;

    dx = point->xX - sphere->center.xX;
    dy = point->xY - sphere->center.xY;
    dz = point->xZ - sphere->center.xZ;
    radius = sphere->radius;
    dx = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    sx = point->xX - particle->position.xX;
    sy = point->xY - particle->position.xY;
    sz = point->xZ - particle->position.xZ;
    MTH3D_M_vSetXYZ_inv(&delta, sx, sy, sz);
    if (radius + 1e-05f < MTH_fn_xSqrtOpt2(dx)) {
        return 0;
    }
    ex = particle->position.xX - sphere->center.xX;
    ey = particle->position.xY - sphere->center.xY;
    ez = particle->position.xZ - sphere->center.xZ;
    ex = MTH_fn_xSqrAddSqrAddSqrOpt2(ex, ey, ez);
    if (MTH_fn_xSqrtOpt2(ex) <= radius - 1e-05f) {
        return 0;
    }
    MTH3D_M_vSetXYZ_inv(&segment, sx, sy, sz);
    if (!inv_INT_fn_bDetectIntersectSegmentWithSphereDetailed(
            point, &segment, sphere, &t, &inv_INT_stSphereHitPoint, &normal, radius)) {
        return 0;
    }
    inv_MTH3D_fn_vSubXYZ(outVector, &inv_INT_stSphereHitPoint, &sphere->center);
    distance = inv_MEC_fn_xGetParticlePlaneDistance(particle, &inv_INT_stSphereHitPoint, outVector);
    if (distance == flt_3934E0[0]) {
        return 0;
    }
    if (!inv_MEC_fn_bMovesTowardPlane(particle, outVector, distance)) {
        return 0;
    }
    *outDistance = distance;
    return 1;
}
