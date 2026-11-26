#include "common.h"

#include "MTH/MTH.h"
#include "GLI/GLI.h"

void func_0011A660(void)
{
    s32 depth = g_lNbMatrixInStack;

    if (depth != 0) {
        depth--;
        g_lNbMatrixInStack = depth;
        g_p_stCurrentMatrix = &g_aDEF_stMatrixStack[depth];
    }
}

void func_0011A690(POS_tdstMatrix_* matrix)
{
    if (g_lNbMatrixInStack != MATRIX_STACK_SIZE) {
        g_aDEF_stMatrixStack[g_lNbMatrixInStack] = *matrix;
        g_p_stCurrentMatrix = &g_aDEF_stMatrixStack[g_lNbMatrixInStack++];
    }
}

void func_0011A7A8(void)
{
    g_lNbMatrixInStack = 0;
    g_p_stCurrentMatrix = NULL;
}
