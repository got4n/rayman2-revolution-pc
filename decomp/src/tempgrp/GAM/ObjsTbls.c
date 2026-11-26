#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"

#include "ps2/libc.h"
#include "AI/AI.h"

#define OBJECT_TABLE_NO_SELECTOR 0xFFFF

static inline tdstObjectsTablesList_* inv_GAM_fn_p_stCurrentObjectsTable(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return object->h_3dData->h_CurrentObjectsTable;
}

void fn_vExchangeTwoObjectsInCurrentObjectsTable(HIE_tdstSuperObject_* p_stSuperObject, u16 w_ObjectA,
                                                 u16 w_ObjectB)
{
    PackedU64U64U32Aligned4 stObjectsTableElement;

    if (inv_GAM_fn_p_stCurrentObjectsTable(p_stSuperObject) == NULL) {
        return;
    }
    stObjectsTableElement = inv_GAM_fn_p_stCurrentObjectsTable(p_stSuperObject)->d_stObjectsTable[w_ObjectB].words;
    inv_GAM_fn_p_stCurrentObjectsTable(p_stSuperObject)->d_stObjectsTable[w_ObjectB].words =
        inv_GAM_fn_p_stCurrentObjectsTable(p_stSuperObject)->d_stObjectsTable[w_ObjectA].words;
    inv_GAM_fn_p_stCurrentObjectsTable(p_stSuperObject)->d_stObjectsTable[w_ObjectA].words = stObjectsTableElement;
}

void fn_vCopyObjectFromObjectTabletoAnother(tdstObjectsTablesList_* _hTargetTable,
                                            tdstObjectsTablesList_* _hSourceTable, u16 _uwTargetObject,
                                            u16 _uwSourceObject)
{
    _hTargetTable->d_stObjectsTable[_uwTargetObject].words =
        _hSourceTable->d_stObjectsTable[_uwSourceObject].words;
}

u32 fn_uwGetObjectTableNumberOfelements(tdstObjectsTablesList_* _hTargetTable)
{
    return _hTargetTable->wNumberOfElement;
}

void func_00113A20(tdstObjectsTablesList_* param0, u16 count)
{
    u32 i;

    param0->wNumberOfElement = count;
    param0->d_stObjectsTable = AI_fn_p_vTrueAlloc(gp_6F74, count * sizeof(ObjectTableEntry));
    param0->d_stInitObjectsTable = NULL;
    for (i = 0; i < count; i++) {
        param0->d_stObjectsTable[i].stElement.wTypeOfTarget = OBJECT_TABLE_NO_SELECTOR;
    }
}

void fn_vReInitObjectTable(tdstObjectsTablesList_* param0)
{
    memcpy(param0->d_stObjectsTable, param0->d_stInitObjectsTable,
           param0->wNumberOfElement * sizeof(ObjectTableEntry));
}

ObjectTableEntry* fn_d_hGetInitObjectsTable(tdstObjectsTablesList_* h_ObjectsTablesListElement)
{
    return h_ObjectsTablesListElement->d_stInitObjectsTable;
}

tdstTransition_* fn_hIfExistTransitionalWithTargetState(tdstState_* h_State, tdstState_* h_TargetState)
{
    tdstTransition_* entry = h_State->hForTransitionArray.d_stElements;
    s32 index;

    for (index = 0; index < h_State->hForTransitionArray.lNumberOfElements; index++, entry++) {
        if (entry->h_TargetState == h_TargetState) {
            return entry;
        }
    }
    return NULL;
}

s32 fn_hIfExistProhibitedState(tdstState_* h_State, tdstState_* h_TargetState)
{
    tdstState_** id = h_State->hForProhibitArray.d_hElements;
    s32 count = h_State->hForProhibitArray.lNumberOfElements;
    s32 i;

    for (i = 0; id != NULL && i < count; i++) {
        if (*id++ == h_TargetState) {
            return 1;
        }
    }
    return 0;
}
