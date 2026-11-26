#include "common.h"

#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"

void HIE_fn_vPushMatrix(void)
{
    s32 depth = HIE_g_lNbMatrixInStack;

    if (depth != HIE_MATRIX_STACK_SIZE_inv) {
        POS_tdstMatrix_* matrix = HIE_g_a_hMatrixStack[0];

        HIE_g_a_hMatrixStack[depth] = matrix;
        g_p_stCurrentMatrix = matrix;
        HIE_g_lNbMatrixInStack = depth + 1;
    }
}

void HIE_fn_vPopMatrix(void)
{
    s32 depth = HIE_g_lNbMatrixInStack;

    if (depth != 0) {
        s32 top = depth - 2;
        POS_tdstMatrix_* matrix = HIE_g_a_hMatrixStack[top];

        HIE_g_lNbMatrixInStack = depth - 1;
        g_p_stCurrentMatrix = matrix;
    }
}

void HIE_fn_vLoadMatrix(POS_tdstMatrix_* matrix)
{
    s32 depth = HIE_g_lNbMatrixInStack;

    if (depth != HIE_MATRIX_STACK_SIZE_inv) {
        HIE_g_a_hMatrixStack[depth] = matrix;
        g_p_stCurrentMatrix = matrix;
        HIE_g_lNbMatrixInStack = depth + 1;
    }
}

s32 inv_MEM_fn_lFreeIntView_inv(MemoryHeapView* heap, void* block);

s32 func_001498B8(POS_tdstMatrix_* matrix)
{
    return inv_MEM_fn_lFreeIntView_inv(gp_6F74, matrix);
}

void func_001498E0(void)
{
    func_001498B8(HIE_g_a_hMatrixStack[0]);
}

void func_001498F0(POS_tdstMatrix_* matrix)
{
    HIE_fn_vLoadMatrix(matrix);
}
