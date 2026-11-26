#include "common.h"

#include "GLI/GLI.h"

void GLI_DRV_vSetZClip(GLI_tdstCamera_* camera, f32 value)
{
    GLI_tdstRenderContext* driver = inv_GLI_renderContext;

    driver->inv_xNear = value;
    driver->inv_xOoNear = 1.0f / value;
}

void GLI_DRV_vSetClipWindow(f32 clip0, f32 clip1, f32 clip2, f32 clip3)
{
    GLI_tdstRenderContext* driver = inv_GLI_renderContext;

    driver->clipWindow[0] = clip0;
    driver->clipWindow[1] = clip1;
    driver->clipWindow[2] = clip2;
    driver->clipWindow[3] = clip3;
}
