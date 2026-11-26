#include "common.h"

#include "COL/COL.h"
#include "MTH/MTH.h"
#include "GEO/GEO.h"

#define PLANE_SIDE_INVALID 0x1F1

typedef struct SphereQuery {
    MTH3D_tdstVector_ center;
    f32 radius;
} SphereQuery;

void func_0020B640(Octree* table, OctreeSegment* segment)
{
    COL_fn_vInitCollisionTable(table);
    COL_fn_vExploreRecursiveOctreeWithSegment(0, segment);
}

s32 inv_COL_fn_bIntersectSegmentWithNode(u16 index, OctreeSegment* segment)
{
    OctreeNode* node;
    OctreeSegment split;

    if (index == 0xFFFF) {
        return 0;
    }
    node = &inv_COL_p_stCollisionTable->nodes[index];
    if (node->axis == 0) {
        return inv_COL_fn_bIntersectSegmentWithLeaf(node, segment);
    }
    switch (inv_COL_fn_ulClipSegmentWithSplitPlane(segment, node, &split.start)) {
    case 0:
        return inv_COL_fn_bIntersectSegmentWithNode(node->u.split.positiveChild, segment);
    case 1:
        split.end = segment->end;
        if (inv_COL_fn_bIntersectSegmentWithNode(node->u.split.positiveChild, &split) != 0) {
            return 1;
        }
        segment->end = split.start;
        return inv_COL_fn_bIntersectSegmentWithNode(node->u.split.negativeChild, segment);
    case 2:
        split.end = segment->end;
        if (inv_COL_fn_bIntersectSegmentWithNode(node->u.split.negativeChild, &split) != 0) {
            return 1;
        }
        segment->end = split.start;
        return inv_COL_fn_bIntersectSegmentWithNode(node->u.split.positiveChild, segment);
    case 3:
        return inv_COL_fn_bIntersectSegmentWithNode(node->u.split.negativeChild, segment);
    default:
        return 0;
    }
}

static inline s32 inv_COL_fn_lSideOfPlane(OctreeNode* node, SphereQuery* query)
{
    f32 distance;
    f32 reference;
    s32 side;

    switch (node->axis) {
    case 1:
        distance = query->center.xX;
        break;
    case 2:
        distance = query->center.xY;
        break;
    case 3:
        distance = query->center.xZ;
        break;
    default:
        return PLANE_SIDE_INVALID;
    }
    reference = node->u.split.position;
    distance -= reference;
    reference = 0.0f;
    if (distance < reference) {
        distance = -distance;
        side = 1;
    } else {
        side = 0;
    }
    if (distance < query->radius) {
        side = 2;
    }
    return side;
}

void inv_COL_fn_vExploreNodeWithQuery(u16 index, SphereQuery* query)
{
    OctreeNode* node;

    if (index == 0xFFFF) {
        return;
    }
    node = &inv_COL_p_stCollisionTable->nodes[index];
    if (node->axis == 0) {
        COL_fn_vAddInCollisionTable(node);
        return;
    }
    switch (inv_COL_fn_lSideOfPlane(node, query)) {
    case 0:
        inv_COL_fn_vExploreNodeWithQuery(node->u.split.positiveChild, query);
        break;
    case 2:
        inv_COL_fn_vExploreNodeWithQuery(node->u.split.positiveChild, query);
    case 1:
        inv_COL_fn_vExploreNodeWithQuery(node->u.split.negativeChild, query);
        break;
    }
}
