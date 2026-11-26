#include "common.h"

#include "AI/AI.h"

void fn_vReinitComport(tdstIntelligence_* p_stIntelligence)
{
    fn_ucChangeComportIntell(p_stIntelligence, p_stIntelligence->initialBehavior);
    p_stIntelligence->p_stPrevComport = p_stIntelligence->p_stCurrentComport;
}

void fn_ucChangeComportIntell(tdstIntelligence_* p_stIntelligence, tdstComport_* p_stComport)
{
    p_stIntelligence->p_stPrevComport = p_stIntelligence->p_stCurrentComport;
    p_stIntelligence->p_stCurrentComport = p_stComport;
    fn_vInitActionTable(p_stIntelligence);
    p_stIntelligence->p_stCurrentSchedule = p_stComport->p_stSchedule;
}

u8 fn_ucGetTreeNbParam(tdstNodeInterpret_* p_stTree)
{
    u8 ucDepth = p_stTree[-1].ucDepth;
    u8 ucNbParam = 0;

    while (ucDepth < p_stTree->ucDepth && p_stTree->eType != E_ti_EndTree) {
        if (p_stTree->ucDepth == ucDepth + 1)
            ucNbParam++;
        p_stTree++;
    }
    return ucNbParam;
}
