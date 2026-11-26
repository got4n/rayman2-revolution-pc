#include "common.h"

#include "MTH/MTH.h"
#include "AI/AI.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "GLI/GLI.h"

POS_tdstMatrix_* GEO_fn_hCreateMatrix(void)
{
    POS_tdstMatrix_* matrix = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(POS_tdstMatrix_));

    gu_MTH4D_M_vSetIdentityMatrix(matrix);
    return matrix;
}

void HIE_fn_vCreateMatrixStack(void)
{
    HIE_g_a_hMatrixStack[0] = GEO_fn_hCreateMatrix();
    HIE_g_lNbMatrixInStack = 1;
}

void HIE_fn_vInitMatrixStack(void)
{
    gu_MTH4D_M_vSetIdentityMatrix(HIE_g_a_hMatrixStack[0]);
    HIE_g_lNbMatrixInStack = 1;
    g_p_stCurrentMatrix = HIE_g_a_hMatrixStack[0];
}
