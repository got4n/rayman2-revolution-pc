#include "common.h"

#include "WP/WP.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

#define WP_ARC_OR_CAPABILITIES 0
#define WP_ARC_SET_CAPABILITIES 1
#define WP_ARC_OR_CAPABILITIES_ALT 2
#define WP_ARC_SET_CAPABILITIES_ALT 3
#define WP_ARC_CLEAR_CAPABILITIES 4
#define WP_ARC_RESET_CAPABILITIES 5

WPArcList* WPARCLST_fn_hInit(void)
{
    WPArcList* hList = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(WPArcList));

    if (hList != NULL) {
        hList->head = NULL;
        hList->tail = NULL;
        hList->count = 0;
    }
    return hList;
}

void WPARCLST_fn_vDestroy(WPArcList* hList)
{
    WPArcNode* hElement;
    WPArcNode* hNextElement;

    if (hList != NULL) {
        for (hElement = hList->head, hNextElement = hElement ? hElement->next : NULL; hElement != NULL;
             hElement = hNextElement, hNextElement = hElement ? hElement->next : NULL) {
            if (hElement->next != NULL)
                hElement->next->previous = hElement->previous;
            if (hElement->previous != NULL)
                hElement->previous->next = hElement->next;
            if (hElement->list != NULL) {
                hElement->list->count--;
                if (hElement->list->head == hElement)
                    hElement->list->head = hElement->next;
                if (hElement->list->tail == hElement)
                    hElement->list->tail = hElement->previous;
            }
            hElement->next = NULL;
            hElement->previous = NULL;
            hElement->list = NULL;
            func_00202480(hElement);
        }
        inv_MEM_fn_vFreeInHeap(gp_6F74, hList);
        hList = NULL;
    }
}

void WPARCLST_fn_vAdd(WPArcList* hListOfArc, void* hNode, s32 lWeight, u32 ulCapability)
{
    WPArcNode* arc;
    WPArcNode* added;
    s32 found = 0;

    if (hListOfArc == NULL) {
        return;
    }
    for (arc = hListOfArc->head; arc != NULL; arc = arc->next) {
        if (arc->destination == hNode) {
            arc->initialCapabilities = ulCapability;
            arc->currentCapabilities = ulCapability;
            arc->initialWeight = lWeight;
            arc->currentWeight = lWeight;
            found = 1;
            break;
        }
    }
    if (found) {
        return;
    }

    added = func_00202418(hNode, lWeight, ulCapability);
    if (added == NULL) {
        return;
    }
    if (added->next != NULL) {
        added->next->previous = added->previous;
    }
    if (added->previous != NULL) {
        added->previous->next = added->next;
    }
    if (added->list != NULL) {
        added->list->count--;
        if (added->list->head == added) {
            added->list->head = added->next;
        }
        if (added->list->tail == added) {
            added->list->tail = added->previous;
        }
    }
    added->previous = NULL;
    added->list = NULL;
    added->next = NULL;
    added->previous = hListOfArc->tail;
    added->list = hListOfArc;
    if (hListOfArc->head == NULL) {
        hListOfArc->head = added;
    } else {
        hListOfArc->tail->next = added;
    }
    hListOfArc->tail = added;
    hListOfArc->count++;
}

s32 WPARCLST_fn_lChangeCapabilities(WPArcList* hListOfArc, void* hNode, u32 ulCapability, u32 lValue)
{
    WPArcNode* param0;

    if (hListOfArc != NULL && hNode != NULL) {
        for (param0 = hListOfArc->head; param0 != NULL; param0 = param0->next) {
            if (param0->destination == hNode) {
                switch (lValue) {
                case WP_ARC_OR_CAPABILITIES:
                case WP_ARC_OR_CAPABILITIES_ALT:
                    param0->currentCapabilities |= ulCapability;
                    break;
                case WP_ARC_SET_CAPABILITIES:
                case WP_ARC_SET_CAPABILITIES_ALT:
                    param0->currentCapabilities = ulCapability;
                    break;
                case WP_ARC_CLEAR_CAPABILITIES:
                    param0->currentCapabilities &= ~ulCapability;
                    break;
                case WP_ARC_RESET_CAPABILITIES:
                    param0->currentCapabilities = 0;
                    break;
                }
                return 0;
            }
        }
    }
    return -1;
}

s32 WPARCLST_fn_lGetCapabilities(WPArcList* hListOfArc, void* hNode, u32* p_ulCapability)
{
    WPArcNode* hElement;

    if (hListOfArc != NULL && hNode != NULL) {
        for (hElement = hListOfArc->head; hElement != NULL; hElement = hElement->next) {
            if (hElement->destination == hNode) {
                *p_ulCapability = hElement->currentCapabilities;
                return 0;
            }
        }
    }
    return -1;
}

s32 WPARCLST_fn_lChangeWeight(WPArcList* hListOfArc, void* hNode, s32 lWeight)
{
    WPArcNode* hElement;

    if (hListOfArc != NULL && hNode != NULL) {
        for (hElement = hListOfArc->head; hElement != NULL; hElement = hElement->next) {
            if (hElement->destination == hNode) {
                hElement->currentWeight = lWeight;
                return 0;
            }
        }
    }
    return -1;
}

s32 WPARCLST_fn_lGetWeight(WPArcList* hListOfArc, void* hNode, s32* p_lWeight)
{
    WPArcNode* hElement;

    if (hListOfArc != NULL && hNode != NULL) {
        for (hElement = hListOfArc->head; hElement != NULL; hElement = hElement->next) {
            if (hElement->destination == hNode) {
                *p_lWeight = hElement->currentWeight;
                return 0;
            }
        }
    }
    return -1;
}
