#include "common.h"

#include "MTH/MTH.h"
#include "GLI/GLI.h"

void GLI_xInitMatrixStack(void)
{
    s16 xI;

    g_lNbMatrixInStack = 0;
    for (xI = 0; xI < MATRIX_STACK_SIZE; xI++)
        gu_MTH4D_M_vSetIdentityMatrix(&g_aDEF_stMatrixStack[xI]);
    g_p_stCurrentMatrix = &g_aDEF_stMatrixStack[g_lNbMatrixInStack++];
}
