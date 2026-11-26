#include "common.h"

#include "GLI/GLI.h"

#define CAMERA_TYPE_ORTHO_0 0x64
#define CAMERA_TYPE_ORTHO_1 0x65
#define CAMERA_TYPE_PERSPECTIVE_0 0x66
#define CAMERA_TYPE_PERSPECTIVE_1 0x67
#define SIGN_BIT 0x80000000

void GLI_xSerialProjection(GLI_tdstCamera_* p_stCam, s32 lNbOfVertex, ProjectedVertex* p_stSource,
                           ProjectedVertex* p_stDest)
{
    ProjectedVertex* end = p_stDest + lNbOfVertex;
    s32 farBits;
    s32 i;
    f32 scale;

    *(f32*) &farBits = 8000.0f;
    switch (p_stCam->lCameraMode) {
    case CAMERA_TYPE_PERSPECTIVE_0:
    case CAMERA_TYPE_PERSPECTIVE_1:
        for (; p_stDest < end; p_stDest++, p_stSource++) {
            if (*(s32*) &p_stSource->z < *(s32*) &p_stCam->xNear) {
                *(u32*) &p_stDest->z |= SIGN_BIT;
                *(u32*) &p_stDest->x = farBits | ((*(u32*) &p_stSource->x & SIGN_BIT) ^ SIGN_BIT);
                *(u32*) &p_stDest->y = farBits | ((*(u32*) &p_stSource->y & SIGN_BIT) ^ SIGN_BIT);
            } else {
                p_stDest->z = 1.0f / p_stSource->z;
                scale = p_stCam->xScreen * p_stDest->z;
                p_stDest->z *= 0.25f;
                p_stDest->x = p_stCam->stTrans.xX + p_stSource->x * p_stCam->stScale.xX * scale;
                p_stDest->y = p_stCam->stTrans.xY + p_stSource->y * p_stCam->stScale.xY * scale;
            }
        }
        break;
    case CAMERA_TYPE_ORTHO_0:
    case CAMERA_TYPE_ORTHO_1:
        for (i = 0; i < lNbOfVertex; i++) {
            if (!(p_stSource[i].z < p_stCam->xNear)) {
                p_stDest[i].z = 1.0f;
                p_stDest[i].x = p_stCam->stTrans.xX + p_stSource[i].x * p_stCam->stScale.xX;
                p_stDest[i].y = p_stCam->stTrans.xY + p_stSource[i].y * p_stCam->stScale.xY;
            }
        }
        break;
    }
}
