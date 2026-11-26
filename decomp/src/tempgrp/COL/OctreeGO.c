#include "common.h"

#include "COL/COL.h"
#include "GEO/GEO.h"
#include "GLI/GLI.h"
#include "MTH/MTH.h"

#define OCTREE_NO_CHILD 0xFFFF

s32 inv_COL_fn_lSplitBoxX(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive);
s32 inv_COL_fn_lSplitBoxY(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive);
s32 inv_COL_fn_lSplitBoxZ(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive);

typedef void (*OctreeClipLerp)(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from,
                               MTH3D_tdstVector_* to, f32 distance);

void inv_COL_fn_vInterpolateAtX(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from, MTH3D_tdstVector_* to,
                   f32 distance);
void inv_COL_fn_vInterpolateAtY(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from, MTH3D_tdstVector_* to,
                   f32 distance);
void inv_COL_fn_vInterpolateAtZ(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from, MTH3D_tdstVector_* to,
                   f32 distance);
void COL_fn_vExploreRecursiveOctreeWithBox(s32 index, OctreeBox* box);

void COL_fn_vExploreOctreeWithBox(OctreeBox* box, OctreeHit_inv* out, u32* count, u32 max,
                                  CollisionEntry** entries)
{
    Octree* table = &inv_COL_stLevelOctree1;

    COL_fn_vInitCollisionTable(table);
    COL_fn_vExploreRecursiveOctreeWithBox(0, box);
    COL_fn_bGetResultCollision(out, count, max);
    *entries = table->entries;
}

void COL_fn_vExploreOctreeWithSegment(OctreeSegment* segment, OctreeHit_inv* out, u32* count, u32 max,
                                      CollisionEntry** entries)
{
    Octree* table = &inv_COL_stLevelOctree1;

    COL_fn_vInitCollisionTable(table);
    COL_fn_vExploreRecursiveOctreeWithSegment(0, segment);
    COL_fn_bGetResultCollision(out, count, max);
    *entries = table->entries;
}

void inv_COL_fn_vQueryLevelOctreeWithSegment(OctreeSegment* segment, OctreeHit_inv* out, u32* count, u32 max, CollisionEntry** entries)
{
    Octree* table = &inv_COL_stLevelOctree2;

    COL_fn_vInitCollisionTable(table);
    COL_fn_vExploreRecursiveOctreeWithSegment(0, segment);
    COL_fn_bGetResultCollision(out, count, max);
    *entries = table->entries;
}

void COL_fn_vExploreOctreeWithSemiAxe(void* query, OctreeHit_inv* out, u32* count, u32 max,
                                      CollisionEntry** entries)
{
    Octree* table = &inv_COL_stLevelOctree1;

    COL_fn_vInitCollisionTable(table);
    inv_COL_fn_vExploreNodeWithQuery(0, query);
    COL_fn_bGetResultCollision(out, count, max);
    *entries = table->entries;
}

void inv_COL_fn_vQueryLevelOctree(void* query, OctreeHit_inv* out, u32* count, u32 max, CollisionEntry** entries)
{
    Octree* table = &inv_COL_stLevelOctree2;

    COL_fn_vInitCollisionTable(table);
    inv_COL_fn_vExploreNodeWithQuery(0, query);
    COL_fn_bGetResultCollision(out, count, max);
    *entries = table->entries;
}

s32 inv_COL_fn_bIntersectSegmentWithLevel(OctreeSegment* segment)
{
    COL_fn_vInitCollisionTable(&inv_COL_stLevelOctree1);
    return inv_COL_fn_bIntersectSegmentWithNode(0, segment);
}

s32 inv_COL_fn_bIntersectSegmentWithLeaf(OctreeNode* leaf, OctreeSegment* segment)
{
    MTH3D_tdstVector_ direction;
    u16* list;
    u32 entryCount;
    u32 triangleCount;
    CollisionEntry* entry;
    GeometricObjectView* geometry;
    GeometricElement* element;
    u32 triangle;

    MTH3D_M_vSubXYZ_inv(&direction, &segment->end, &segment->start);
    list = leaf->u.elementList;
    entryCount = *list++;
    while (entryCount--) {
        entry = &inv_COL_p_stCollisionTable->entries[list[0]];
        triangleCount = list[1];
        list += 2;
        geometry = entry->geometry;
        element = geometry->elements[entry->element];
        while (triangleCount--) {
            triangle = *list++;
            if (INT_fn_bDetectIntersectSegmentWithTriangle(
                    &segment->start, &direction,
                    &geometry->vertices[element->d_stListOfFacesTripled[triangle].a3_xIndex[0]],
                    &geometry->vertices[element->d_stListOfFacesTripled[triangle].a3_xIndex[1]],
                    &geometry->vertices[element->d_stListOfFacesTripled[triangle].a3_xIndex[2]])) {
                return 1;
            }
            if (INT_fn_bDetectIntersectSegmentWithTriangle(
                    &segment->start, &direction,
                    &geometry->vertices[element->d_stListOfFacesTripled[triangle].a3_xIndex[0]],
                    &geometry->vertices[element->d_stListOfFacesTripled[triangle].a3_xIndex[2]],
                    &geometry->vertices[element->d_stListOfFacesTripled[triangle].a3_xIndex[1]])) {
                return 1;
            }
        }
    }
    return 0;
}

s32 inv_COL_fn_lSplitBoxX(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive)
{
    f32 position;
    f32 dmin;
    f32 dmax;
    f32 rad;
    f32 radS;
    f32 rad2;
    f32 xAbsMax0;
    f32 xAbsMin0;
    f32 rad0;

    f32 xRatio;
    f32 t_dmax;
    f32 t_dmin;
    f32 xAbsMax;
    f32 xAbsMin;
    s32 side;
    u32 code;
    s32 sa;
    s32 sa0;
    s32 sb0;
    s32 sb;
    OctreeClipLerp lerp;
    u8 flags;
    MTH3D_tdstVector_* pA;
    MTH3D_tdstVector_* pB;
    MTH3D_tdstVector_* pOutB;

    side = 1;
    position = node->u.split.position;
    dmin = box->center.xX - position;
    dmax = box->end.xX - position;
    if (!(dmin < 0.0f)) {
        side = 0;
    }
    if (0.0f <= (dmin * dmax)) {
        radS = box->extent;
        if (side != 0) {
            rad = radS;
            do {
                xAbsMax = __builtin_fabsf(dmax);
            } while (0);
            xAbsMin = __builtin_fabsf(dmin);
            do {
            } while (0);
            lerp = inv_COL_fn_vInterpolateAtX;
            t_dmax = xAbsMax - rad;
            t_dmin = xAbsMin - rad;
            sa = *(s32*) &t_dmax;
            sb = *(s32*) &t_dmin;
            code = (u32) sb >> 31;
            code |= ((u32) sa >> 31) << 1;
            switch (code) {
            case 0:
                break;
            case 1:
                positive->axisFlags = box->axisFlags;
                positive->extent = box->extent;
                positive->center = box->center;
                lerp(&positive->end, &box->end, &box->center, t_dmax);
                break;
            case 2:
                positive->axisFlags = box->axisFlags;
                positive->extent = box->extent;
                positive->center = box->end;
                lerp(&positive->end, &box->center, &box->end, t_dmin);
                break;
            case 3:
                *positive = *box;
                break;
            }
            lerp = 0;
            if (code != 0) {
                *negative = *box;
                return 2;
            }
            return 1;
        }
        rad0 = radS;
        do {
            xAbsMax0 = __builtin_fabsf(dmax);
        } while (0);
        xAbsMin0 = __builtin_fabsf(dmin);
        do {
        } while (0);
        lerp = inv_COL_fn_vInterpolateAtX;
        t_dmax = xAbsMax0 - rad0;
        t_dmin = xAbsMin0 - rad0;
        sa0 = *(s32*) &t_dmax;
        sb0 = *(s32*) &t_dmin;
        code = (u32) sb0 >> 31;
        code |= ((u32) sa0 >> 31) << 1;
        switch (code) {
        case 0:
            break;
        case 1:
            negative->axisFlags = box->axisFlags;
            negative->extent = box->extent;
            negative->center = box->center;
            lerp(&negative->end, &box->end, &box->center, t_dmax);
            break;
        case 2:
            negative->axisFlags = box->axisFlags;
            negative->extent = box->extent;
            negative->center = box->end;
            lerp(&negative->end, &box->center, &box->end, t_dmin);
            break;
        case 3:
            *negative = *box;
            break;
        }
        lerp = 0;
        if (code != 0) {
            *positive = *box;
            return 2;
        }
        return 0;
    }
    rad = 0.0f;
    rad0 = 0.0f;
    flags = box->axisFlags;
    positive->axisFlags = flags;
    negative->axisFlags = flags;
    rad2 = box->extent;
    positive->extent = rad2;
    negative->extent = rad2;
    pA = &box->center;
    pB = &box->end;
    pOutB = &negative->end;
    xRatio = __builtin_fabsf(dmin) / inv_MTH_fn_xAbsoluteTied(pA->xX - pB->xX);
    MTH3D_M_vSetXYZ_inv(pOutB, pA->xX + (xRatio * (pB->xX - pA->xX)),
                        pA->xY + (xRatio * (pB->xY - pA->xY)),
                        pA->xZ + (xRatio * (pB->xZ - pA->xZ)));
    positive->end = negative->end;
    if (side != 0) {
        negative->center = box->center;
        positive->center = box->end;
        return 2;
    }
    negative->center = box->end;
    positive->center = box->center;
    return 2;
}

s32 inv_COL_fn_lSplitBoxY(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive)
{
    f32 position;
    f32 dmin;
    f32 dmax;
    f32 rad;
    f32 radS;
    f32 rad2;
    f32 xAbsMax0;
    f32 xAbsMin0;
    f32 rad0;

    f32 xRatio;
    f32 t_dmax;
    f32 t_dmin;
    f32 xAbsMax;
    f32 xAbsMin;
    s32 side;
    u32 code;
    s32 sa;
    s32 sa0;
    s32 sb0;
    s32 sb;
    OctreeClipLerp lerp;
    u8 flags;
    MTH3D_tdstVector_* pA;
    MTH3D_tdstVector_* pB;
    MTH3D_tdstVector_* pOutB;

    side = 1;
    position = node->u.split.position;
    dmin = box->center.xY - position;
    dmax = box->end.xY - position;
    if (!(dmin < 0.0f)) {
        side = 0;
    }
    if (0.0f <= (dmin * dmax)) {
        radS = box->extent;
        if (side != 0) {
            rad = radS;
            do {
                xAbsMax = __builtin_fabsf(dmax);
            } while (0);
            xAbsMin = __builtin_fabsf(dmin);
            do {
            } while (0);
            lerp = inv_COL_fn_vInterpolateAtY;
            t_dmax = xAbsMax - rad;
            t_dmin = xAbsMin - rad;
            sa = *(s32*) &t_dmax;
            sb = *(s32*) &t_dmin;
            code = (u32) sb >> 31;
            code |= ((u32) sa >> 31) << 1;
            switch (code) {
            case 0:
                break;
            case 1:
                positive->axisFlags = box->axisFlags;
                positive->extent = box->extent;
                positive->center = box->center;
                lerp(&positive->end, &box->end, &box->center, t_dmax);
                break;
            case 2:
                positive->axisFlags = box->axisFlags;
                positive->extent = box->extent;
                positive->center = box->end;
                lerp(&positive->end, &box->center, &box->end, t_dmin);
                break;
            case 3:
                *positive = *box;
                break;
            }
            lerp = 0;
            if (code != 0) {
                *negative = *box;
                return 2;
            }
            return 1;
        }
        rad0 = radS;
        do {
            xAbsMax0 = __builtin_fabsf(dmax);
        } while (0);
        xAbsMin0 = __builtin_fabsf(dmin);
        do {
        } while (0);
        lerp = inv_COL_fn_vInterpolateAtY;
        t_dmax = xAbsMax0 - rad0;
        t_dmin = xAbsMin0 - rad0;
        sa0 = *(s32*) &t_dmax;
        sb0 = *(s32*) &t_dmin;
        code = (u32) sb0 >> 31;
        code |= ((u32) sa0 >> 31) << 1;
        switch (code) {
        case 0:
            break;
        case 1:
            negative->axisFlags = box->axisFlags;
            negative->extent = box->extent;
            negative->center = box->center;
            lerp(&negative->end, &box->end, &box->center, t_dmax);
            break;
        case 2:
            negative->axisFlags = box->axisFlags;
            negative->extent = box->extent;
            negative->center = box->end;
            lerp(&negative->end, &box->center, &box->end, t_dmin);
            break;
        case 3:
            *negative = *box;
            break;
        }
        lerp = 0;
        if (code != 0) {
            *positive = *box;
            return 2;
        }
        return 0;
    }
    rad = 0.0f;
    rad0 = 0.0f;
    flags = box->axisFlags;
    positive->axisFlags = flags;
    negative->axisFlags = flags;
    rad2 = box->extent;
    positive->extent = rad2;
    negative->extent = rad2;
    pA = &box->center;
    pB = &box->end;
    pOutB = &negative->end;
    xRatio = inv_MTH_fn_xAbsolute(dmin) / inv_MTH_fn_xAbsoluteTied(pA->xY - pB->xY);
    MTH3D_M_vSetXYZ_inv(pOutB, pA->xX + (xRatio * (pB->xX - pA->xX)),
                        pA->xY + (xRatio * (pB->xY - pA->xY)),
                        pA->xZ + (xRatio * (pB->xZ - pA->xZ)));
    positive->end = negative->end;
    if (side != 0) {
        negative->center = box->center;
        positive->center = box->end;
        return 2;
    }
    negative->center = box->end;
    positive->center = box->center;
    return 2;
}

s32 inv_COL_fn_lSplitBoxZ(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive)
{
    f32 position;
    f32 dmin;
    f32 dmax;
    f32 rad;
    f32 radS;
    f32 rad2;
    f32 xAbsMax0;
    f32 xAbsMin0;
    f32 rad0;

    f32 xRatio;
    f32 t_dmax;
    f32 t_dmin;
    f32 xAbsMax;
    f32 xAbsMin;
    s32 side;
    u32 code;
    s32 sa;
    s32 sa0;
    s32 sb0;
    s32 sb;
    OctreeClipLerp lerp;
    u8 flags;
    MTH3D_tdstVector_* pA;
    MTH3D_tdstVector_* pB;
    MTH3D_tdstVector_* pOutB;

    side = 1;
    position = node->u.split.position;
    dmin = box->center.xZ - position;
    dmax = box->end.xZ - position;
    if (!(dmin < 0.0f)) {
        side = 0;
    }
    if (0.0f <= (dmin * dmax)) {
        radS = box->extent;
        if (side != 0) {
            rad = radS;
            do {
                xAbsMax = __builtin_fabsf(dmax);
            } while (0);
            xAbsMin = __builtin_fabsf(dmin);
            do {
            } while (0);
            lerp = inv_COL_fn_vInterpolateAtZ;
            t_dmax = xAbsMax - rad;
            t_dmin = xAbsMin - rad;
            sa = *(s32*) &t_dmax;
            sb = *(s32*) &t_dmin;
            code = (u32) sb >> 31;
            code |= ((u32) sa >> 31) << 1;
            switch (code) {
            case 0:
                break;
            case 1:
                positive->axisFlags = box->axisFlags;
                positive->extent = box->extent;
                positive->center = box->center;
                lerp(&positive->end, &box->end, &box->center, t_dmax);
                break;
            case 2:
                positive->axisFlags = box->axisFlags;
                positive->extent = box->extent;
                positive->center = box->end;
                lerp(&positive->end, &box->center, &box->end, t_dmin);
                break;
            case 3:
                *positive = *box;
                break;
            }
            lerp = 0;
            if (code != 0) {
                *negative = *box;
                return 2;
            }
            return 1;
        }
        rad0 = radS;
        do {
            xAbsMax0 = __builtin_fabsf(dmax);
        } while (0);
        xAbsMin0 = __builtin_fabsf(dmin);
        do {
        } while (0);
        lerp = inv_COL_fn_vInterpolateAtZ;
        t_dmax = xAbsMax0 - rad0;
        t_dmin = xAbsMin0 - rad0;
        sa0 = *(s32*) &t_dmax;
        sb0 = *(s32*) &t_dmin;
        code = (u32) sb0 >> 31;
        code |= ((u32) sa0 >> 31) << 1;
        switch (code) {
        case 0:
            break;
        case 1:
            negative->axisFlags = box->axisFlags;
            negative->extent = box->extent;
            negative->center = box->center;
            lerp(&negative->end, &box->end, &box->center, t_dmax);
            break;
        case 2:
            negative->axisFlags = box->axisFlags;
            negative->extent = box->extent;
            negative->center = box->end;
            lerp(&negative->end, &box->center, &box->end, t_dmin);
            break;
        case 3:
            *negative = *box;
            break;
        }
        lerp = 0;
        if (code != 0) {
            *positive = *box;
            return 2;
        }
        return 0;
    }
    rad = 0.0f;
    rad0 = 0.0f;
    flags = box->axisFlags;
    positive->axisFlags = flags;
    negative->axisFlags = flags;
    rad2 = box->extent;
    positive->extent = rad2;
    negative->extent = rad2;
    pA = &box->center;
    pB = &box->end;
    pOutB = &negative->end;
    xRatio = inv_MTH_fn_xAbsolute(dmin) / inv_MTH_fn_xAbsoluteTied(pA->xZ - pB->xZ);
    MTH3D_M_vSetXYZ_inv(pOutB, pA->xX + (xRatio * (pB->xX - pA->xX)),
                        pA->xY + (xRatio * (pB->xY - pA->xY)),
                        pA->xZ + (xRatio * (pB->xZ - pA->xZ)));
    positive->end = negative->end;
    if (side != 0) {
        negative->center = box->center;
        positive->center = box->end;
        return 2;
    }
    negative->center = box->end;
    positive->center = box->center;
    return 2;
}

s32 inv_COL_fn_lGetBoxSide(OctreeBox* box, OctreeNode* node)
{
    s32 axis = node->axis;
    f32 delta;
    s32 side;

    switch (axis) {
    case 1:
        delta = box->center.xX - node->u.split.position;
        break;
    case 2:
        delta = box->center.xY - node->u.split.position;
        break;
    case 3:
        delta = box->center.xZ - node->u.split.position;
        break;
    default:
        delta = 0.0f;
        break;
    }

    if (delta < 0.0f) {
        side = 1;
        delta = -delta;
    } else {
        side = 0;
    }
    if (delta - box->extent < 0.0f) {
        side = 3;
    }
    return side;
}

s32 inv_COL_fn_lSplitBox(OctreeBox* box, OctreeNode* node, OctreeBox* negative, OctreeBox* positive)
{
    switch (node->axis) {
    case 1:
        return inv_COL_fn_lSplitBoxX(box, node, negative, positive);
    case 2:
        return inv_COL_fn_lSplitBoxY(box, node, negative, positive);
    case 3:
        return inv_COL_fn_lSplitBoxZ(box, node, negative, positive);
    default:
        return 0x17;
    }
}

void COL_fn_vExploreRecursiveOctreeWithBox(s32 nodeIndex, OctreeBox* box)
{
    OctreeBox negative;
    OctreeBox positive;
    OctreeNode* node;
    u32 index = nodeIndex & 0xFFFF;
    s32 side;

    if (index == OCTREE_NO_CHILD) {
        return;
    }

    node = &inv_COL_p_stCollisionTable->nodes[index];
    if (node->axis == 0) {
        COL_fn_vAddInCollisionTable(node);
        return;
    }

    if ((box->axisFlags >> node->axis) & 1) {
        side = inv_COL_fn_lGetBoxSide(box, node);
    } else {
        side = inv_COL_fn_lSplitBox(box, node, &negative, &positive);
    }

    switch (side) {
    case 0:
        COL_fn_vExploreRecursiveOctreeWithBox(node->u.split.positiveChild, box);
        break;
    case 1:
        COL_fn_vExploreRecursiveOctreeWithBox(node->u.split.negativeChild, box);
        break;
    case 2:
        COL_fn_vExploreRecursiveOctreeWithBox(node->u.split.negativeChild, &negative);
        COL_fn_vExploreRecursiveOctreeWithBox(node->u.split.positiveChild, &positive);
        break;
    case 3:
        COL_fn_vExploreRecursiveOctreeWithBox(node->u.split.negativeChild, box);
        COL_fn_vExploreRecursiveOctreeWithBox(node->u.split.positiveChild, box);
        break;
    }
}

void inv_COL_fn_vInterpolateAtX(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from, MTH3D_tdstVector_* to,
                   f32 distance)
{
    f32 ratio = distance / inv_MTH_fn_xAbsoluteTied(from->xX - to->xX);

    MTH3D_M_vSetXYZ_inv(out, from->xX + ratio * (to->xX - from->xX),
                        from->xY + ratio * (to->xY - from->xY),
                        from->xZ + ratio * (to->xZ - from->xZ));
}

void inv_COL_fn_vInterpolateAtY(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from, MTH3D_tdstVector_* to,
                   f32 distance)
{
    f32 ratio = distance / inv_MTH_fn_xAbsoluteTied(from->xY - to->xY);

    MTH3D_M_vSetXYZ_inv(out, from->xX + ratio * (to->xX - from->xX),
                        from->xY + ratio * (to->xY - from->xY),
                        from->xZ + ratio * (to->xZ - from->xZ));
}

void inv_COL_fn_vInterpolateAtZ(MTH3D_tdstVector_* out, MTH3D_tdstVector_* from, MTH3D_tdstVector_* to,
                   f32 distance)
{
    f32 ratio = distance / inv_MTH_fn_xAbsoluteTied(from->xZ - to->xZ);

    MTH3D_M_vSetXYZ_inv(out, from->xX + ratio * (to->xX - from->xX),
                        from->xY + ratio * (to->xY - from->xY),
                        from->xZ + ratio * (to->xZ - from->xZ));
}

s32 inv_COL_fn_ulClipSegmentWithSplitPlane(OctreeSegment* segment, OctreeNode* node, MTH3D_tdstVector_* hit)
{
    u32 side;

    switch (node->axis) {
    case 1: {
        f32 position = node->u.split.position;
        f32 start = segment->start.xX;
        f32 end;
        f32 parameter;

        side = start <= position;
        end = segment->end.xX;
        if (end <= position) {
            side |= 2;
        }
        if (side != 1 && side != 2) {
            break;
        }
        hit->xX = position;
        parameter = (position - start) / (end - start);
        hit->xY = segment->start.xY + parameter * (segment->end.xY - segment->start.xY);
        parameter = parameter * (segment->end.xZ - segment->start.xZ);
        hit->xZ = segment->start.xZ + parameter;
        break;
    }
    case 2: {
        f32 position = node->u.split.position;
        f32 start = segment->start.xY;
        f32 end;
        f32 parameter;

        side = start <= position;
        end = segment->end.xY;
        if (end <= position) {
            side |= 2;
        }
        if (side != 1 && side != 2) {
            break;
        }
        parameter = (position - start) / (end - start);
        hit->xX = segment->start.xX + parameter * (segment->end.xX - segment->start.xX);
        hit->xY = position;
        parameter = parameter * (segment->end.xZ - segment->start.xZ);
        hit->xZ = segment->start.xZ + parameter;
        break;
    }
    case 3: {
        f32 position = node->u.split.position;
        f32 start = segment->start.xZ;
        f32 end;
        f32 parameter;

        side = start <= position;
        end = segment->end.xZ;
        if (end <= position) {
            side |= 2;
        }
        if (side != 1 && side != 2) {
            break;
        }
        end = end - start;
        parameter = (position - start) / end;
        hit->xX = segment->start.xX + parameter * (segment->end.xX - segment->start.xX);
        parameter = parameter * (segment->end.xY - segment->start.xY);
        hit->xY = segment->start.xY + parameter;
        hit->xZ = position;
        break;
    }
    default:
        side = -1;
        break;
    }
    return side;
}

void COL_fn_vExploreRecursiveOctreeWithSegment(u16 index, OctreeSegment* segment)
{
    OctreeNode* node;
    OctreeSegment part;

    if (index == OCTREE_NO_CHILD) {
        return;
    }
    node = &inv_COL_p_stCollisionTable->nodes[index];
    if (node->axis == 0) {
        COL_fn_vAddInCollisionTable(node);
        return;
    }
    switch (inv_COL_fn_ulClipSegmentWithSplitPlane(segment, node, &part.start)) {
    case 0:
        COL_fn_vExploreRecursiveOctreeWithSegment(node->u.split.positiveChild, segment);
        break;
    case 1:
        part.end = segment->start;
        COL_fn_vExploreRecursiveOctreeWithSegment(node->u.split.negativeChild, &part);
        part.end = segment->end;
        COL_fn_vExploreRecursiveOctreeWithSegment(node->u.split.positiveChild, &part);
        break;
    case 2:
        part.end = segment->end;
        COL_fn_vExploreRecursiveOctreeWithSegment(node->u.split.negativeChild, &part);
        part.end = segment->start;
        COL_fn_vExploreRecursiveOctreeWithSegment(node->u.split.positiveChild, &part);
        break;
    case 3:
        COL_fn_vExploreRecursiveOctreeWithSegment(node->u.split.negativeChild, segment);
        break;
    }
}
