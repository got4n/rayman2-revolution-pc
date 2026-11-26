#include "common.h"

#include "GAM/GAM.h"
#include "AI/AI.h"

typedef struct ObjectTableStateView {
    u8 unk00[0x14];
    s32 unk14;
    s32 unk18;
    s32 unk1C;
} ObjectTableStateView;

s32 func_00113870(ObjectTableStateView* state)
{
    return state->unk14;
}

tdstObjectsTablesList_* fn_hGetObjectsTableOfFamilyByIndex(tdstFamilyListView* _hFamily, s16 _xIndex)
{
    return &_hFamily->hForObjectsTablesList[_xIndex];
}

tdstObjectsTablesList_* inv_fn_p_stCreateObjectTable(void)
{
    tdstObjectsTablesList_* table = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstObjectsTablesList_));

    table->wNumberOfElement = 0;
    table->d_stObjectsTable = NULL;
    table->d_stInitObjectsTable = NULL;
    table->wZDxUsed = 0;
    return table;
}

void func_001138D0(void)
{
}

void func_001138D8(ObjectTableStateView* state, s32 value)
{
    state->unk18 = value;
    state->unk1C = 1;
}
