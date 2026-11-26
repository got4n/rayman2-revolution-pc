#include "common.h"
#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

tdstNodeInterpret_* fn_p_stExecuteActionInit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree)
{
    tdstState_* state;
    s32 changed;

    p_stTree = fn_p_stDefaultActionInit(p_SuperObjPerso, p_stTree);
    state = fn_xGetActionParamStateValue(0, p_SuperObjPerso);
    inv_HIE_fn_p_stGetEngineObject(p_SuperObjPerso)->h_3dData->h_FirstStateOfAction = NULL;
    changed = PLA_fn_bSetNewState(p_SuperObjPerso, state, 0, 0, 0);
    fn_vSetActionParamlValue(0, p_SuperObjPerso, changed);
    fn_vSetActionParamStateValue(1, p_SuperObjPerso, state);
    return p_stTree;
}

s32 fn_ucExecuteAction(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    tdstState_* action = fn_xGetActionParamStateValue(1, p_SuperObjPerso);

    if (fn_lGetActionParamlValue(0, p_SuperObjPerso) == 0)
        return 4;
    if (PLA_fn_bTestCurrentAction(p_SuperObjPerso, action) == 0)
        return 4;
    return PLA_fn_bTestIfEndOfAction(p_SuperObjPerso) != 0 ? 4 : 1;
}

s32 fn_ucWaitEndOfAction(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    return PLA_fn_bTestIfEndOfAction(p_SuperObjPerso) ? 4 : 1;
}

s32 fn_ucWaitEndOfAnim(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    return PLA_fn_bTestIfEndOfAnim(p_SuperObjPerso) ? 4 : 1;
}

tdstNodeInterpret_* fn_p_stFrozenWaitInit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree)
{
    u32 now;

    p_stTree = fn_p_stDefaultActionInit(p_SuperObjPerso, p_stTree);
    now = inv_fn_ulGetEngineClock();
    fn_vSetActionParamlValue(0, p_SuperObjPerso, now + fn_lGetActionParamlValue(0, p_SuperObjPerso));
    return p_stTree;
}

s32 fn_ucFrozenWait(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    u32 now = inv_fn_ulGetEngineClock();
    u32 deadline = fn_lGetActionParamlValue(0, p_SuperObjPerso);

    return now < deadline ? 1 : 4;
}
