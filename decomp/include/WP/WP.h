#ifndef REVO_WP_WP_H
#define REVO_WP_WP_H

#include "ps2/types.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

typedef struct WPArcList WPArcList;

typedef struct WPArcNode {
    struct WPArcNode* next;
    struct WPArcNode* previous;
    WPArcList* list;
    struct WP_tdstGraphNode_* destination;
    u32 currentCapabilities;
    u32 initialCapabilities;
    s32 currentWeight;
    s32 initialWeight;
} WPArcNode;

struct WPArcList {
    WPArcNode* head;
    WPArcNode* tail;
    s32 count;
};

typedef struct WP_tdstWayPoint_ {
    MTH3D_tdstVector_ m_stVertex;
    f32 m_xRadius;
    HIE_tdstSuperObject_* m_hSuperObject;
} WP_tdstWayPoint_;

typedef WPArcList WP_tdstListOfArc_;
typedef struct WP_tdstGraphNode_ WP_tdstGraphNode_;

typedef struct LST2_tdstAnchorToWP_tdhGraphNodeDyn {
    WP_tdstGraphNode_* hFirstElementDyn;
    WP_tdstGraphNode_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorToWP_tdhGraphNodeDyn;

struct WP_tdstGraphNode_ {
    WP_tdstGraphNode_* hNextBrotherDyn;
    WP_tdstGraphNode_* hPrevBrotherDyn;
    LST2_tdstAnchorToWP_tdhGraphNodeDyn* hFatherDyn;
    WP_tdstWayPoint_* m_hWayPoint;
    s32 m_lTypeOfWP;
    s32 m_lTypeOfWPInit;
    WP_tdstListOfArc_* m_hListOfArc;
};

typedef struct WP_tdstGraph_ {
    LST2_tdstAnchorToWP_tdhGraphNodeDyn m_hListOfNode;
    char* m_szNameOfGraph;
    char* m_szReferenceSection;
} WP_tdstGraph_;

extern WP_tdstListOfArc_* IPT_fn_bGetActivationFlagOfEntryElement_0(WP_tdstGraphNode_* hNode);

#endif
