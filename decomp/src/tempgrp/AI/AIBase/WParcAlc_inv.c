#include "common.h"

#include "WP/WP.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

WPArcNode* func_00202418(void* destination, s32 weight, u32 capabilities)
{
    WPArcNode* arc = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(WPArcNode));

    if (arc != NULL) {
        arc->next = NULL;
        arc->previous = NULL;
        arc->list = NULL;
        arc->destination = destination;
        arc->initialCapabilities = capabilities;
        arc->currentCapabilities = capabilities;
        arc->initialWeight = weight;
        arc->currentWeight = weight;
    }
    return arc;
}

s32 inv_MEM_fn_lFreeIntView_inv(MemoryHeapView* heap, void* block);

s32 func_00202480(WPArcNode* arc)
{
    if (arc != NULL)
        return inv_MEM_fn_lFreeIntView_inv(gp_6F74, arc);
}

void func_002024A8(void)
{
}
