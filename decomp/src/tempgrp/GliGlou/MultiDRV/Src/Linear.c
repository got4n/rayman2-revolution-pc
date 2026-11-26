#include "common.h"

#include "GLI/GLI.h"
#include "MTH/MTH.h"
#include "ps2/r5900.h"

void GLI_xSerialLinearOp(s32 lNbOfVertex, MTH3D_tdstVector_* p_stSource, ProjectedVertex* p_stDest,
                         POS_tdstMatrix_* p_stMatrix)
{
    ProjectedVertex* end = p_stDest + lNbOfVertex;

    while (p_stDest < end) {
        if (p_stSource == (void*) p_stDest) {
            MTH3D_tdstVector_ copy = *p_stSource;

            R5900_TRANSFORM_POINT_inv(p_stDest, &copy, p_stMatrix);
        } else {
            R5900_TRANSFORM_POINT_inv(p_stDest, p_stSource, p_stMatrix);
        }
        p_stDest++;
        p_stSource++;
    }
}
