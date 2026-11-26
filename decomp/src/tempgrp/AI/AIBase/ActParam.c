#include "common.h"

#include "AI/AI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

static inline tdstIntelligence_* inv_AI_fn_p_stCurrentIntelligence(AI_tdstMind_* mind)
{
    if (mind->bDoingIntel != 0) {
        return mind->p_stIntelligence;
    }
    return mind->p_stReflex;
}

static inline gu_tduGetSetParamValue_* inv_AI_fn_p_stActionParam(HIE_tdstSuperObject_* superObject, s32 index)
{
    AI_tdstMind_* mind = inv_AI_fn_p_stGetMindSO(superObject);
    tdstActionTable_* table;

    index &= 0xFF;
    table = inv_AI_fn_p_stCurrentIntelligence(mind)->p_stActionTable;
    return (gu_tduGetSetParamValue_*) (index * sizeof(gu_tduGetSetParamValue_) +
                                       (table->ucCurrentEntry * sizeof(tdstActionTableEntry_) +
                                        (u32) table->p_stEntry));
}

void fn_vSetActionParamxValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso, f32 xValue)
{
    inv_AI_fn_p_stActionParam(p_SuperObjPerso, ucN)->xValue = xValue;
}

f32 fn_xGetActionParamxValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso)
{
    return inv_AI_fn_p_stActionParam(p_SuperObjPerso, ucN)->xValue;
}

void fn_vSetActionParamlValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso, s32 lValue)
{
    inv_AI_fn_p_stActionParam(p_SuperObjPerso, ucN)->lValue = lValue;
}

s32 fn_lGetActionParamlValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso)
{
    return inv_AI_fn_p_stActionParam(p_SuperObjPerso, ucN)->lValue;
}

void fn_vSetActionParamStateValue(s32 index, HIE_tdstSuperObject_* superObject, tdstState_* hState)
{
    inv_AI_fn_p_stActionParam(superObject, index)->tdxstHandleToState = hState;
}

tdstState_* fn_xGetActionParamStateValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso)
{
    return inv_AI_fn_p_stActionParam(p_SuperObjPerso, ucN)->tdxstHandleToState;
}

void fn_vSetActionParamp_stVertex(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso,
                                  MTH3D_tdstVector_* p_stVertex)
{
    inv_AI_fn_p_stActionParam(p_SuperObjPerso, ucN)->stVector = *p_stVertex;
}

MTH3D_tdstVector_* fn_p_stGetActionParamp_stVertex(u8 ucN, HIE_tdstSuperObject_* p_SuperObjPerso)
{
    tdstActionTable_* table = inv_AI_fn_p_stCurrentIntelligence(inv_AI_fn_p_stGetMindSO(p_SuperObjPerso))->p_stActionTable;

    return &table->p_stEntry[table->ucCurrentEntry].stActionParam.a_uParam[ucN].stVector;
}
