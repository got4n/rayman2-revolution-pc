#include "common.h"
#include "MTH/MTH.h"
#include "ps2/copy_types.h"
#include "COL/COL.h"

extern f32 flt_3934E4[];
extern MTH3D_tdstVector_ inv_COL_stParticleHitVector;

s32 func_0013F5C0(EffectParticle_inv* particle, EffectSystem_inv* list, CollisionNode** outNode,
                  f32* outDistance, MTH3D_tdstVector_* segment, MTH3D_tdstVector_* outHit)
{
    CollisionNode* node;
    CollisionNode* best = NULL;
    f32 distance;
    f32 bestDistance = flt_3934E4[0];

    if ((particle->flags >> 2) & 1) {
        return 0;
    }
    node = list->colliders;
    while (node != NULL) {
        if (node != particle->lastCollider) {
            s32 hit = 0;

            switch (node->type) {
            case 1:
                hit = inv_MEC_fn_bIntersectParticleWithPlane(particle, segment, node->data, &distance,
                                    &inv_COL_stParticleHitVector);
                break;
            case 8:
                hit = inv_INT_fn_bIntersectSegmentWithTriangleFrontFace(
                    particle, segment, node->data, &distance, &inv_COL_stParticleHitVector);
                break;
            case 4:
                hit = inv_INT_fn_bIntersectSegmentWithTriangleBackFace(
                    particle, segment, node->data, &distance, &inv_COL_stParticleHitVector);
                break;
            case 2:
                hit = inv_INT_fn_bIntersectSegmentWithSphere(
                    particle, segment, node->data, &distance, &inv_COL_stParticleHitVector);
                break;
            }
            if (hit != 0 && distance < bestDistance) {
                *outHit = inv_COL_stParticleHitVector;
                bestDistance = distance;
                best = node;
            }
        }
        node = node->next;
    }

    if (best != NULL) {
        *outNode = best;
        *outDistance = bestDistance;
        return 1;
    }
    return 0;
}

void func_0013F788(EffectParticle_inv* particle, EffectSystem_inv* list)
{
    CollisionNode* node = particle->lastCollider;
    s32 type = node->type;

    if (type == 8) {
        CollisionShape* shape = node->data;

        if (INT_fn_bGetPointInTriangle(&particle->position, &shape->points[0],
                                                 &shape->points[1], &shape->points[2],
                                                 &shape->points[4])) {
            return;
        }
        if (INT_fn_bGetPointInTriangle(&particle->position, &shape->points[0],
                                                 &shape->points[2], &shape->points[3],
                                                 &shape->points[4])) {
            return;
        }
    } else if (type == 4) {
        CollisionShape* shape = node->data;

        if (INT_fn_bGetPointInTriangle(&particle->position, &shape->points[0],
                                                 &shape->points[1], &shape->points[2],
                                                 &shape->points[3])) {
            return;
        }
    } else {
        return;
    }
    particle->lastCollider = NULL;
    particle->acceleration = list->acceleration;
}

void inv_MEC_fn_vSplitAlongNormal(MTH3D_tdstVector_* vector, MTH3D_tdstVector_* tangent, MTH3D_tdstVector_* normalPart,
                  MTH3D_tdstVector_* normal)
{
    MTH3D_M_vSetXYZ_inv(normalPart,
                        inv_MTH3D_fn_xDotXYZ(normal, vector) * normal->xX,
                        inv_MTH3D_fn_xDotXYZ(normal, vector) * normal->xY,
                        inv_MTH3D_fn_xDotXYZ(normal, vector) * normal->xZ);
    inv_MTH3D_fn_vSubXYZ(tangent, vector, normalPart);
}
