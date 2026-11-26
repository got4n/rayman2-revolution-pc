#include "common.h"

#include "AI/AI.h"
#include "GAM/GAM.h"

tdstBrain_* fn_h_BrainRealAlloc(void);
void fn_vFirstInitBrain(tdstBrain_* brain);

static inline tdstBrain_* inv_GAM_fn_p_stBrainOf(tdstEngineObject_* object)
{
    return object->h_Brain;
}

void fn_vBrainAlloc(tdstEngineObject_* object)
{
    object->h_Brain = fn_h_BrainRealAlloc();
}

tdstBrain_* fn_h_BrainRealAlloc(void)
{
    tdstBrain_* brain = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstBrain_));

    fn_vFirstInitBrain(brain);
    return brain;
}

void fn_vFirstInitBrain(tdstBrain_* brain)
{
    brain->p_stMind = fn_pstAllocMind();
    brain->hLastCollidedGoThroughMaterial = (struct GMT_tdstGameMaterial_*) 0xCDCDCDCD;
}

void fn_vBrainInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    inv_GAM_fn_p_stBrainOf(p_stObject)->hLastCollidedGoThroughMaterial = (struct GMT_tdstGameMaterial_*) 0xCDCDCDCD;
    switch (eObjectInit) {
    case OTI_ReinitTheMap:
    case OTI_LoadSavedGame:
    case 4:
        fn_vReinitAIWithFlags(inv_GAM_fn_p_stBrainOf(p_stObject)->p_stMind, eObjectInit);
        return;
    case OTI_WhenGameStart:
    case OTI_MapLoaded:
        break;
    case OTI_AlwaysCreated:
    default:
        return;
    }
    if (((p_stObject->h_StandardGame->ucMiscFlags >> 3) & 1) == 0)
        fn_vSecondPassOfMindLoading(inv_GAM_fn_p_stBrainOf(p_stObject)->p_stMind);
    else
        fn_vReinitAI(inv_GAM_fn_p_stBrainOf(p_stObject)->p_stMind);
    func_001DDFF8(p_stObject);
}

void fn_vBrainDesinit(void)
{
}

void fn_vBrainCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    fn_vSimulateMindAllocationFromAnotherMind(inv_GAM_fn_p_stBrainOf(_p_stObjectSrc)->p_stMind, inv_GAM_fn_p_stBrainOf(_p_stObjectDest)->p_stMind);
}
