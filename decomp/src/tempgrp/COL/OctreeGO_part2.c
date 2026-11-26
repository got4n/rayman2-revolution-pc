#include "common.h"

#include "COL/COL.h"
#include "MTH/MTH.h"

void inv_COL_fn_vExploreOctreeWithBox(COL_tdstOctreeNode_* p_stNodeToBeExplored,
                                      MTH3D_tdstVector_* p_stMinPoint,
                                      MTH3D_tdstVector_* p_stMaxPoint,
                                      COL_tdstOctreeNode_** d_pstSelectedNode,
                                      s16* p_xNumberOfSelectedNodes)
{
    COL_tdstOctreeNode_* p_stNext;

again:
    if (INT_fn_bIntersectBoxWithBox(p_stMinPoint, p_stMaxPoint, &p_stNodeToBeExplored->stMinPart,
                                     &p_stNodeToBeExplored->stMaxPart) == 0) {
        return;
    }
    if (p_stNodeToBeExplored->p_pstChildList != NULL) {
        if (p_stNodeToBeExplored->p_pstChildList[0] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[0], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[1] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[1], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[2] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[2], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[3] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[3], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[4] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[4], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[5] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[5], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[6] != NULL) {
            inv_COL_fn_vExploreOctreeWithBox(p_stNodeToBeExplored->p_pstChildList[6], p_stMinPoint,
                                             p_stMaxPoint, d_pstSelectedNode, p_xNumberOfSelectedNodes);
        }
        p_stNext = p_stNodeToBeExplored->p_pstChildList[7];
        if (p_stNext == NULL) {
            return;
        }
        p_stNodeToBeExplored = p_stNext;
        goto again;
    }
    if (*p_xNumberOfSelectedNodes < OCTREE_MAX_HITS) {
        d_pstSelectedNode[*p_xNumberOfSelectedNodes] = p_stNodeToBeExplored;
        *p_xNumberOfSelectedNodes = *p_xNumberOfSelectedNodes + 1;
    }
}

void inv_COL_fn_vFindSectorsInBox(COL_tdstOctree_* p_stOctree, MTH3D_tdstVector_* p_stMinPoint,
                                  MTH3D_tdstVector_* p_stMaxPoint,
                                  COL_tdstOctreeNode_** d_pstSelectedNode,
                                  s16* p_xNumberOfSelectedNodes)
{
    *p_xNumberOfSelectedNodes = 0;
    if (p_stOctree != NULL && p_stOctree->p_stOctreeRoot != NULL) {
        inv_COL_fn_vExploreOctreeWithBox(p_stOctree->p_stOctreeRoot, p_stMinPoint, p_stMaxPoint,
                                         d_pstSelectedNode, p_xNumberOfSelectedNodes);
    }
}

void inv_INT_fn_vInsertSortedBoxHit(f32 xT, COL_tdstOctreeNode_* p_stNode,
                                    COL_tdstOctreeNode_** d_pstSelectedNode, f32* d_xSelectedT,
                                    s16* p_xNumberOfSelectedNodes)
{
    s16 i = *p_xNumberOfSelectedNodes;

    if (i > 0 && xT < d_xSelectedT[i - 1]) {
        do {
            if (i < OCTREE_MAX_HITS) {
                d_pstSelectedNode[i] = d_pstSelectedNode[i - 1];
                d_xSelectedT[i] = d_xSelectedT[i - 1];
            }
            i--;
            if (i <= 0) {
                break;
            }
        } while (xT < d_xSelectedT[i - 1]);
    }
    if (i < OCTREE_MAX_HITS) {
        d_pstSelectedNode[i] = p_stNode;
        d_xSelectedT[i] = xT;
        if (*p_xNumberOfSelectedNodes < OCTREE_MAX_HITS) {
            *p_xNumberOfSelectedNodes = *p_xNumberOfSelectedNodes + 1;
        }
    }
}

void _COL_fn_vExploreOctreeWithSegmentAndBoxes(COL_tdstOctreeNode_* p_stNodeToBeExplored,
                                               MTH3D_tdstVector_* p_stVertexA,
                                               MTH3D_tdstVector_* p_stVectAB,
                                               COL_tdstOctreeNode_** d_pstSelectedNode,
                                               f32* d_xSelectedT, s16* p_xNumberOfSelectedNodes)
{
    COL_tdstOctreeNode_* p_stNext;
    f32 xT;

again:
    if (INT_fn_bDetectIntersectSegmentWithBox(p_stVertexA, p_stVectAB,
                                               &p_stNodeToBeExplored->stMinPart,
                                               &p_stNodeToBeExplored->stMaxPart, &xT) == 0) {
        return;
    }
    if (p_stNodeToBeExplored->p_pstChildList != NULL) {
        if (p_stNodeToBeExplored->p_pstChildList[0] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[0],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[1] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[1],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[2] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[2],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[3] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[3],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[4] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[4],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[5] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[5],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        if (p_stNodeToBeExplored->p_pstChildList[6] != NULL) {
            _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stNodeToBeExplored->p_pstChildList[6],
                                                      p_stVertexA, p_stVectAB, d_pstSelectedNode,
                                                      d_xSelectedT, p_xNumberOfSelectedNodes);
        }
        p_stNext = p_stNodeToBeExplored->p_pstChildList[7];
        if (p_stNext == NULL) {
            return;
        }
        p_stNodeToBeExplored = p_stNext;
        goto again;
    }
    inv_INT_fn_vInsertSortedBoxHit(xT, p_stNodeToBeExplored, d_pstSelectedNode, d_xSelectedT,
                                   p_xNumberOfSelectedNodes);
}

void _COL_fn_vExploreOctreeWithSegment(COL_tdstOctree_* p_stOctree, MTH3D_tdstVector_* p_stVertexA,
                                       MTH3D_tdstVector_* p_stVectAB,
                                       COL_tdstOctreeNode_** d_pstSelectedNode, f32* d_xSelectedT,
                                       s16* p_xNumberOfSelectedNodes)
{
    *p_xNumberOfSelectedNodes = 0;
    if (p_stOctree != NULL && p_stOctree->p_stOctreeRoot != NULL) {
        _COL_fn_vExploreOctreeWithSegmentAndBoxes(p_stOctree->p_stOctreeRoot, p_stVertexA,
                                                  p_stVectAB, d_pstSelectedNode, d_xSelectedT,
                                                  p_xNumberOfSelectedNodes);
    }
}
