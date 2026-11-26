#include "common.h"
#include "GAM/GAM.h"

void fn_vLastDesinitFamilyStructure(void)
{
    tdstFamilyListView* param0;
    tdstFamilyListView* p_stNextFamily;

    param0 = g_stEngineStructure.hFamilyList.hFirstElementDyn;
    p_stNextFamily = param0 != NULL ? param0->hNextBrotherDyn : NULL;
    while (param0 != NULL) {
        inv_LST2_M_vIsolateElement(param0);
        param0 = p_stNextFamily;
        p_stNextFamily = param0 != NULL ? param0->hNextBrotherDyn : NULL;
    }
}
