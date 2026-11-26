#include "common.h"

#include "GAM/GAM.h"
#include "WP/WP.h"
#include "AI/AI.h"

#define WPG_OK 0
#define WPG_ERROR -1
#define WPG_NO_WAY -1
#define WPG_INFINITE_VALUATION 0x7FFFFFFF

void* memmove(void* destination, const void* source, u32 size);
WP_tdstGraphNode_* WPG_fn_hCreateNode(WP_tdstWayPoint_* wayPoint, s32 type);
void func_001E0CF8(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, u32 capabilities,
                   s32 weight);
s32 WPG_fn_lGetArcCapabilities(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination,
                                u32* capabilities);
s32 WPG_fn_lChangeArcWeight(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, s32 weight);
s32 WPG_fn_lGetArcWeight(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, s32* weight);
void func_001E0CC0(WP_tdstGraphNode_* node);
s32 func_001E0D38(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, u32 flags,
                  u32 operation);

WP_tdstGraph_* WPG_fn_hCreate(char* name, char* referenceSection)
{
    WP_tdstGraph_* graph =
        AI_fn_p_vTrueAlloc(gp_6F74, sizeof(LST2_tdstAnchorToWP_tdhGraphNodeDyn));

    if (graph != NULL) {
        graph->m_hListOfNode.hFirstElementDyn = NULL;
        graph->m_hListOfNode.hLastElementDyn = NULL;
        graph->m_hListOfNode.lNumberOfElementsDyn = 0;
    }
    return graph;
}

void WPG_fn_vDestroy(WP_tdstGraph_* hGraph)
{
    WP_tdstGraphNode_* node;
    WP_tdstGraphNode_* next;

    for (node = hGraph->m_hListOfNode.hFirstElementDyn,
        next = node != NULL ? node->hNextBrotherDyn : NULL;
         node != NULL; node = next, next = node != NULL ? node->hNextBrotherDyn : NULL) {
        inv_LST2_M_vIsolateElement(node);
        func_001E0CC0(node);
    }
    inv_MEM_fn_vFreeInHeap(gp_6F74, hGraph);
    hGraph = NULL;
}

s32 WPG_fn_lAddWayPoint(WP_tdstGraph_* _hGraph, WP_tdstWayPoint_* _hWayPoint, s32 _lTypeOfWP)
{
    WP_tdstGraphNode_* node;

    if (_hGraph != NULL && _hWayPoint != NULL) {
        node = WPG_fn_hCreateNode(_hWayPoint, _lTypeOfWP);
        LST2_M_vAddElementToList(&_hGraph->m_hListOfNode, node);
        return WPG_OK;
    }
    return WPG_ERROR;
}

s32 func_001E0018(WP_tdstGraph_* graph, WP_tdstWayPoint_* wayPoint, s32 type)
{
    WP_tdstGraphNode_* node;

    for (node = graph->m_hListOfNode.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
        if (node->m_hWayPoint == wayPoint) {
            node->m_lTypeOfWP = type;
            node->m_lTypeOfWPInit = type;
            return WPG_OK;
        }
    }
    return WPG_ERROR;
}

s32 WPG_fn_lAddArcToWayPoint(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                              WP_tdstWayPoint_* hDstWayPoint, s32 lWeight, u32 ulCapability)
{
    WP_tdstGraphNode_* first;
    WP_tdstGraphNode_* sourceNode;
    WP_tdstGraphNode_* destinationNode;
    WP_tdstWayPoint_* wayPoint;

    if (hGraph == NULL || hSrcWayPoint == NULL || hDstWayPoint == NULL) {
        return WPG_ERROR;
    }
    first = hGraph->m_hListOfNode.hFirstElementDyn;
    for (destinationNode = first; destinationNode != NULL;
         destinationNode = destinationNode->hNextBrotherDyn) {
        if (destinationNode->m_hWayPoint == hDstWayPoint) {
            break;
        }
    }
    if (destinationNode == NULL) {
        return WPG_ERROR;
    }
    sourceNode = first;
    if (sourceNode == NULL) {
        return WPG_ERROR;
    }
    wayPoint = sourceNode->m_hWayPoint;
    while (1) {
        if (wayPoint == hSrcWayPoint) {
            func_001E0CF8(sourceNode, destinationNode, ulCapability, lWeight);
            return WPG_OK;
        }
        sourceNode = sourceNode->hNextBrotherDyn;
        if (sourceNode == NULL) {
            return WPG_ERROR;
        }
        wayPoint = sourceNode->m_hWayPoint;
    }
}

s32 WPG_fn_lChangeLinkCapabilities(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint, WP_tdstWayPoint_* hDstWayPoint,
                  u32 ulCapability, u32 lValue)
{
    WP_tdstGraphNode_* sourceNode;
    WP_tdstGraphNode_* destinationNode;

    if (hGraph != NULL && hSrcWayPoint != NULL && hDstWayPoint != NULL) {
        for (destinationNode = hGraph->m_hListOfNode.hFirstElementDyn; destinationNode != NULL;
             destinationNode = destinationNode->hNextBrotherDyn) {
            if (destinationNode->m_hWayPoint == hDstWayPoint) {
                break;
            }
        }
        if (destinationNode != NULL) {
            for (sourceNode = hGraph->m_hListOfNode.hFirstElementDyn; sourceNode != NULL;
                 sourceNode = sourceNode->hNextBrotherDyn) {
                if (sourceNode->m_hWayPoint == hSrcWayPoint) {
                    return func_001E0D38(sourceNode, destinationNode, ulCapability, lValue);
                }
            }
        }
    }
    return WPG_ERROR;
}

s32 WPG_fn_lGetLinkCapabilities(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                 WP_tdstWayPoint_* hDstWayPoint, u32* p_ulCapability)
{
    WP_tdstGraphNode_* first;
    WP_tdstGraphNode_* sourceNode;
    WP_tdstGraphNode_* destinationNode;
    WP_tdstWayPoint_* wayPoint;

    if (hGraph == NULL || hSrcWayPoint == NULL || hDstWayPoint == NULL) {
        return WPG_ERROR;
    }
    first = hGraph->m_hListOfNode.hFirstElementDyn;
    for (destinationNode = first; destinationNode != NULL;
         destinationNode = destinationNode->hNextBrotherDyn) {
        if (destinationNode->m_hWayPoint == hDstWayPoint) {
            break;
        }
    }
    if (destinationNode == NULL) {
        return WPG_ERROR;
    }
    sourceNode = first;
    if (sourceNode == NULL) {
        return WPG_ERROR;
    }
    wayPoint = sourceNode->m_hWayPoint;
    while (1) {
        if (wayPoint == hSrcWayPoint) {
            return WPG_fn_lGetArcCapabilities(sourceNode, destinationNode, p_ulCapability);
        }
        sourceNode = sourceNode->hNextBrotherDyn;
        if (sourceNode == NULL) {
            return WPG_ERROR;
        }
        wayPoint = sourceNode->m_hWayPoint;
    }
}

s32 WPG_fn_lChangeLinkWeight(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                              WP_tdstWayPoint_* hDstWayPoint, s32 lWeight)
{
    WP_tdstGraphNode_* first;
    WP_tdstGraphNode_* sourceNode;
    WP_tdstGraphNode_* destinationNode;
    WP_tdstWayPoint_* wayPoint;

    if (hGraph == NULL || hSrcWayPoint == NULL || hDstWayPoint == NULL) {
        return WPG_ERROR;
    }
    first = hGraph->m_hListOfNode.hFirstElementDyn;
    for (destinationNode = first; destinationNode != NULL;
         destinationNode = destinationNode->hNextBrotherDyn) {
        if (destinationNode->m_hWayPoint == hDstWayPoint) {
            break;
        }
    }
    if (destinationNode == NULL) {
        return WPG_ERROR;
    }
    sourceNode = first;
    if (sourceNode == NULL) {
        return WPG_ERROR;
    }
    wayPoint = sourceNode->m_hWayPoint;
    while (1) {
        if (wayPoint == hSrcWayPoint) {
            return WPG_fn_lChangeArcWeight(sourceNode, destinationNode, lWeight);
        }
        sourceNode = sourceNode->hNextBrotherDyn;
        if (sourceNode == NULL) {
            return WPG_ERROR;
        }
        wayPoint = sourceNode->m_hWayPoint;
    }
}

s32 WPG_fn_lGetLinkWeight(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                           WP_tdstWayPoint_* hDstWayPoint, s32* p_lWeight)
{
    WP_tdstGraphNode_* first;
    WP_tdstGraphNode_* sourceNode;
    WP_tdstGraphNode_* destinationNode;
    WP_tdstWayPoint_* wayPoint;

    if (hGraph == NULL || hSrcWayPoint == NULL || hDstWayPoint == NULL) {
        return WPG_ERROR;
    }
    first = hGraph->m_hListOfNode.hFirstElementDyn;
    for (destinationNode = first; destinationNode != NULL;
         destinationNode = destinationNode->hNextBrotherDyn) {
        if (destinationNode->m_hWayPoint == hDstWayPoint) {
            break;
        }
    }
    if (destinationNode == NULL) {
        return WPG_ERROR;
    }
    sourceNode = first;
    if (sourceNode == NULL) {
        return WPG_ERROR;
    }
    wayPoint = sourceNode->m_hWayPoint;
    while (1) {
        if (wayPoint == hSrcWayPoint) {
            return WPG_fn_lGetArcWeight(sourceNode, destinationNode, p_lWeight);
        }
        sourceNode = sourceNode->hNextBrotherDyn;
        if (sourceNode == NULL) {
            return WPG_ERROR;
        }
        wayPoint = sourceNode->m_hWayPoint;
    }
}

void WPG_fn_vReinitGraph(WP_tdstGraph_* hGraph)
{
    WP_tdstGraphNode_* node;
    WPArcNode* arc;

    if (hGraph == NULL) {
        return;
    }
    for (node = hGraph->m_hListOfNode.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
        node->m_lTypeOfWP = node->m_lTypeOfWPInit;
        for (arc = node->m_hListOfArc->head; arc != NULL; arc = arc->next) {
            arc->currentCapabilities = arc->initialCapabilities;
            arc->currentWeight = arc->initialWeight;
        }
    }
}

WP_tdstGraph_* WPG_fn_lShortestPath(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                     WP_tdstWayPoint_* hDstWayPoint, u32 ulCapability)
{
    WP_tdstGraphNode_* node;
    WP_tdstGraphNode_* neighbor;
    WPArcNode* arc;
    WP_tdstGraphNode_** explored;
    WP_tdstGraphNode_** reached;
    s32* wayOfExplored;
    s32* wayOfReached;
    s32* valuationOfExplored;
    s32* valuationOfReached;
    s32 exploredCount;
    s32 currentReached;
    s32 reachedCount;
    s32 currentExplored;
    s32 done;
    s32 index;
    s32 i;
    s32 valuation;
    WP_tdstGraph_* path;
    s32 weight;
    s32 nodeCount;
    s32 arraySize;

    nodeCount = hGraph->m_hListOfNode.lNumberOfElementsDyn;
    arraySize = nodeCount * sizeof(s32);
    explored = AI_fn_p_vTrueAlloc(gp_6F74, (arraySize << 1) + (arraySize << 2));
    reached = explored + nodeCount;
    wayOfExplored = (s32*) (reached + nodeCount);
    valuationOfExplored = wayOfExplored + nodeCount;
    wayOfReached = valuationOfExplored + nodeCount;
    valuationOfReached = wayOfReached + nodeCount;

    currentExplored = 0;
    currentReached = 0;
    exploredCount = 0;
    reachedCount = 1;
    for (node = hGraph->m_hListOfNode.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
        if (node->m_hWayPoint == hSrcWayPoint) {
            reached[0] = node;
            wayOfReached[0] = WPG_NO_WAY;
            break;
        }
    }

    if (reached[0] == NULL) {
        path = NULL;
    } else {
        done = 0;
        while (reachedCount && !done) {
            valuation = WPG_INFINITE_VALUATION;
            for (i = 0; i < reachedCount; i++) {
                if (valuationOfReached[i] < valuation) {
                    currentReached = i;
                    valuation = valuationOfReached[i];
                }
            }

            explored[exploredCount] = reached[currentReached];
            valuationOfExplored[exploredCount] = valuationOfReached[currentReached];
            wayOfExplored[exploredCount] = wayOfReached[currentReached];
            currentExplored = exploredCount;
            exploredCount++;

            memmove(&valuationOfReached[currentReached], &valuationOfReached[currentReached] + 1,
                    (reachedCount - currentReached) * sizeof(s32));
            memmove(&wayOfReached[currentReached], &wayOfReached[currentReached] + 1,
                    (reachedCount - currentReached) * sizeof(s32));
            memmove(&reached[currentReached], &reached[currentReached] + 1,
                    (reachedCount - currentReached) * sizeof(WP_tdstGraphNode_*));
            reachedCount--;

            node = explored[currentExplored];
            if (node->m_hWayPoint == hDstWayPoint) {
                done = 1;
            }

            for (arc = node->m_hListOfArc->head, i = 0; arc != NULL; arc = arc->next, i++) {
                neighbor = arc->destination;
                if (arc->currentCapabilities & ulCapability) {
                    for (index = 0; index < exploredCount; index++) {
                        if (explored[index] == neighbor) {
                            break;
                        }
                    }
                    if (index == exploredCount) {
                        for (index = 0; index < reachedCount; index++) {
                            if (reached[index] == neighbor) {
                                break;
                            }
                        }
                        if (index == reachedCount) {
                            reached[reachedCount] = neighbor;
                            valuationOfReached[reachedCount] =
                                valuationOfExplored[currentExplored] + arc->currentWeight;
                            wayOfReached[reachedCount] = currentExplored;
                            reachedCount++;
                        } else if (valuationOfReached[index] >
                                   valuationOfExplored[currentExplored] + arc->currentWeight) {
                            valuationOfReached[index] =
                                valuationOfExplored[currentExplored] + arc->currentWeight;
                            wayOfReached[index] = currentExplored;
                        }
                    }
                }
            }
        }

        path = NULL;
        if (done) {
            path = WPG_fn_hCreate("Generated by ShortestPath", "@");
            for (index = currentExplored; index != WPG_NO_WAY; index = wayOfExplored[index]) {
                WPG_fn_lAddWayPoint(path, explored[index]->m_hWayPoint,
                                     explored[index]->m_lTypeOfWP);
            }
            for (index = currentExplored; index != 0; index = wayOfExplored[index]) {
                WPG_fn_lGetLinkCapabilities(hGraph, explored[wayOfExplored[index]]->m_hWayPoint,
                                             explored[index]->m_hWayPoint, &ulCapability);
                WPG_fn_lGetLinkWeight(hGraph, explored[wayOfExplored[index]]->m_hWayPoint,
                                       explored[index]->m_hWayPoint, &weight);
                WPG_fn_lAddArcToWayPoint(path, explored[wayOfExplored[index]]->m_hWayPoint,
                                          explored[index]->m_hWayPoint, weight, ulCapability);
            }
        }
    }

    inv_MEM_fn_vFreeInHeap(gp_6F74, explored);
    return path;
}

WP_tdstGraph_* WPG_fn_lBuildOrderedPath(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hStartWP)
{
    u32 capabilities;
    s32 weight;
    WP_tdstGraph_* path;
    WP_tdstGraphNode_* first;
    u32 bestCapabilities;
    WP_tdstWayPoint_** wayPoints;
    s32* types;
    u32* arcCapabilities;
    s32* arcWeights;
    s32 count;
    WP_tdstGraphNode_** nodes;
    WP_tdstGraphNode_* node;
    s32 nodeCount;
    s32 startIndex;
    s32 i;
    s32 k;
    s32 current;
    s32 best;
    s32 minWeight;

    nodeCount = hGraph->m_hListOfNode.lNumberOfElementsDyn;
    bestCapabilities = 0;
    if (nodeCount == 0) {
        return NULL;
    }

    startIndex = -1;
    i = 0;
    nodes = AI_fn_p_vTrueAlloc(gp_6F74, (nodeCount * sizeof(s32) << 2) + nodeCount * sizeof(s32));
    wayPoints = (WP_tdstWayPoint_**) (nodes + nodeCount);
    types = (s32*) (wayPoints + nodeCount);
    arcCapabilities = (u32*) (types + nodeCount);
    arcWeights = (s32*) (arcCapabilities + nodeCount);

    for (node = hGraph->m_hListOfNode.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
        if (node->m_hWayPoint == hStartWP) {
            nodes[0] = node;
            startIndex = i;
            break;
        }
        i++;
    }

    path = NULL;
    if (startIndex != -1) {
        path = WPG_fn_hCreate("Generated by BuildOrderedPath", "@");

        for (i = startIndex + 1; i != -1;) {
            node = hGraph->m_hListOfNode.hFirstElementDyn;
            for (k = 0; k < i % nodeCount; k++) {
                node = node->hNextBrotherDyn;
            }
            nodes[(i - startIndex) % nodeCount] = node;
            i++;
            if (i % nodeCount == startIndex) {
                i = -1;
            }
        }

        first = nodes[0];
        current = 0;
        wayPoints[0] = first->m_hWayPoint;
        types[0] = nodes[0]->m_lTypeOfWP;
        count = 0;

        do {
            best = -1;
            minWeight = WPG_INFINITE_VALUATION;
            for (i = 1; i < nodeCount; i++) {
                if (i != current &&
                    WPG_fn_lGetArcCapabilities(nodes[current], nodes[i], &capabilities) !=
                        WPG_ERROR &&
                    WPG_fn_lGetArcWeight(nodes[current], nodes[i], &weight) != WPG_ERROR &&
                    weight < minWeight) {
                    minWeight = weight;
                    best = i;
                    bestCapabilities = capabilities;
                }
            }
            if (best != -1) {
                count++;
                wayPoints[count] = nodes[best]->m_hWayPoint;
                types[count] = nodes[best]->m_lTypeOfWP;
                arcCapabilities[count] = bestCapabilities;
                arcWeights[count] = minWeight;
                nodes[current] = NULL;
                current = best;
            }
        } while (best != -1);

        arcCapabilities[0] = 0;
        arcWeights[0] = 0;
        if (WPG_fn_lGetArcCapabilities(nodes[current], first, &capabilities) != WPG_ERROR &&
            WPG_fn_lGetArcWeight(nodes[current], first, &weight) != WPG_ERROR) {
            arcCapabilities[0] = capabilities;
            arcWeights[0] = weight;
        }

        WPG_fn_lAddWayPoint(path, wayPoints[count], types[count]);
        for (count--; count >= 0; count--) {
            WPG_fn_lAddWayPoint(path, wayPoints[count], types[count]);
            WPG_fn_lAddArcToWayPoint(path, wayPoints[count], wayPoints[count + 1],
                                      arcWeights[count + 1], arcCapabilities[count + 1]);
        }
        WPG_fn_lAddArcToWayPoint(path, nodes[current]->m_hWayPoint, wayPoints[0], arcWeights[0],
                                  arcCapabilities[0]);
    }

    inv_MEM_fn_vFreeInHeap(gp_6F74, nodes);
    return path;
}

WP_tdstWayPoint_* WPG_fn_hGetWaypointOfGraph(WP_tdstGraph_* hGraph, s32 iIndex, s32* _plTypeOfWP)
{
    WP_tdstGraphNode_* node = hGraph->m_hListOfNode.hFirstElementDyn;

    if (iIndex > 0) {
        do {
            iIndex--;
            node = node->hNextBrotherDyn;
        } while (iIndex != 0);
    }
    *_plTypeOfWP = node->m_lTypeOfWP;
    return node->m_hWayPoint;
}

WP_tdstGraphNode_* WPG_fn_hCreateNode(WP_tdstWayPoint_* wayPoint, s32 type)
{
    WP_tdstGraphNode_* node = NULL;

    if (wayPoint != NULL) {
        node = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(WP_tdstGraphNode_));
        if (node != NULL) {
            node->hNextBrotherDyn = NULL;
            node->hPrevBrotherDyn = NULL;
            node->hFatherDyn = NULL;
            node->m_lTypeOfWP = type;
            node->m_hWayPoint = wayPoint;
            node->m_lTypeOfWPInit = type;
            node->m_hListOfArc = WPARCLST_fn_hInit();
        }
    }
    return node;
}

void func_001E0CC0(WP_tdstGraphNode_* node)
{
    if (node != NULL) {
        WPARCLST_fn_vDestroy(node->m_hListOfArc);
        inv_MEM_fn_vFreeInHeap(gp_6F74, node);
        node = NULL;
    }
}

void func_001E0CF8(WP_tdstGraphNode_* hNode, WP_tdstGraphNode_* destination, u32 ulCapability,
                   s32 weight)
{
    if (hNode != NULL && destination != NULL) {
        WPARCLST_fn_vAdd(hNode->m_hListOfArc, destination, weight, ulCapability);
    }
}

WP_tdstListOfArc_* IPT_fn_bGetActivationFlagOfEntryElement_0(WP_tdstGraphNode_* hNode)
{
    WP_tdstListOfArc_* hListOfArc = NULL;

    if (hNode != NULL) {
        hListOfArc = hNode->m_hListOfArc;
    }
    return hListOfArc;
}

s32 func_001E0D38(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, u32 flags,
                  u32 operation)
{
    if (source != NULL && destination != NULL) {
        return WPARCLST_fn_lChangeCapabilities(source->m_hListOfArc, destination, flags, operation);
    }
    return WPG_ERROR;
}

s32 WPG_fn_lGetArcCapabilities(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination,
                                u32* capabilities)
{
    if (source != NULL && destination != NULL) {
        return WPARCLST_fn_lGetCapabilities(source->m_hListOfArc, destination, capabilities);
    }
    return WPG_ERROR;
}

s32 WPG_fn_lChangeArcWeight(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, s32 weight)
{
    if (source != NULL && destination != NULL) {
        return WPARCLST_fn_lChangeWeight(source->m_hListOfArc, destination, weight);
    }
    return WPG_ERROR;
}

s32 WPG_fn_lGetArcWeight(WP_tdstGraphNode_* source, WP_tdstGraphNode_* destination, s32* weight)
{
    if (source != NULL && destination != NULL) {
        return WPARCLST_fn_lGetWeight(source->m_hListOfArc, destination, weight);
    }
    return WPG_ERROR;
}
