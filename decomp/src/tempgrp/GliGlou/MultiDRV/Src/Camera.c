#include "common.h"

#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "MTH/MTH.h"

#include "ps2/r5900.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

#define CAMERA_TYPE_PERSPECTIVE 0x67
#define CAMERA_DEFAULT_NEAR 0.25f
#define CAMERA_DEFAULT_FAR 500.0f
#define CAMERA_UNLIMITED_FAR 100000.0f

#define BOUNDS_CORNER_COUNT 4
#define SINUS_EFFECT_FIRST_VIEWPORT 1
#define SINUS_EFFECT_SECOND_VIEWPORT 2
#define SINUS_EFFECT_ALL_VIEWPORTS 3
#define BOUNDS_SECOND_VIEWPORT 0x80000000

typedef struct BoundsGeometryView {
    u8 unk00[0x20];
    s16 vertexCount;
    MTH3D_tdstVector_* vertices;
} BoundsGeometryView;

typedef struct ScreenPoint {
    f32 x;
    f32 y;
} ScreenPoint;

extern s32 GLI_gs_lBackGroundColor;
extern GLI_tdstCamera_* p_stCamAntiBug;

f32 tanf(f32 angle);
void GLI_xAdjustCameraToViewport2(GLD_tdstDeviceAttributes_* device,
                                  GLD_tdstViewportAttributes_* attributes, GLI_tdstCamera_* camera);
void GLI_xGetCameraMatrix(GLI_tdstCamera_* camera, POS_tdstMatrix_* matrix);

s32 inv_GLI_fn_bSetBackGroundColor(s32 unused, s32 value)
{
    GLI_gs_lBackGroundColor = value;
    return 1;
}

void GLI_xCreateCamera(GLI_tdstCamera_** h_stCam, s32 lCameraMode)
{
    f32 aspect;

    *h_stCam = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(GLI_tdstCamera_));
    if (*h_stCam == NULL) {
        return;
    }
    (*h_stCam)->lCameraMode = lCameraMode;
    switch ((*h_stCam)->lCameraMode) {
    case 100:
    case 101:
        aspect = 1.0f;
        (*h_stCam)->xAlphaX = aspect;
        (*h_stCam)->xAlphaY = aspect;
        break;
    case 102:
    case 103:
        aspect = MTH_QUARTER_PI_inv;
        (*h_stCam)->xAlphaX = aspect;
        (*h_stCam)->xAlphaY = aspect;
        break;
    }
    (*h_stCam)->xNear = CAMERA_DEFAULT_NEAR;
    (*h_stCam)->xFar = CAMERA_DEFAULT_FAR;
    (*h_stCam)->xRatio = (*h_stCam)->xAlphaY / (*h_stCam)->xAlphaX;
}

void GLI_xAdjustCameraToViewport(s16 viewportIndex, s16 cameraIndex, GLI_tdstCamera_* camera)
{
    GLD_tdstDeviceAttributes_ device;
    GLD_tdstViewportAttributes_ attributes;

    if (GLD_bGetDeviceAttributes(viewportIndex, &device) == 0) {
        return;
    }
    if (GLD_bGetViewportAttributes(viewportIndex, cameraIndex, &attributes) == 0) {
        return;
    }
    if (camera == NULL) {
        camera = p_stCamAntiBug;
    } else {
        p_stCamAntiBug = camera;
    }
    GLI_xAdjustCameraToViewport2(&device, &attributes, camera);
}

void GLI_vGet3DVertexFromScreenPos(GLD_tdstViewportAttributes_* viewport, MTH3D_tdstVector_* position,
                                   f32 ratioX, f32 ratioY, f32 size)
{
    GLI_tdstSpecificAttributesFor3D_* cameraCell;
    GLI_tdstCamera_* camera;
    f32 depth;
    f32 zero;
    ScreenPoint screen;
    ScreenPoint center;
    MTH3D_tdstVector_ vertex;
    POS_tdstMatrix_ matrix;
    POS_tdstMatrix_ inverse;

    screen.x = (f32) viewport->values.dwWidth * ratioX;
    screen.y = (f32) viewport->values.dwHeight * ratioY;

    cameraCell = viewport->p_vSpecificToXD;
    camera = cameraCell->p_stCam;
    switch (camera->lCameraMode) {
    case 100:
    case 101:
        depth = 0.1f;
        break;
    case 102:
    case 103:
        depth = camera->xScreen;
        break;
    default:
        zero = 0.0f;
        position->xZ = zero;
        position->xY = zero;
        position->xX = zero;
        return;
    }

    center.x = camera->stTrans.xX;
    center.y = camera->stTrans.xY;
    if (size != 0.0f) {
        MTH3D_M_vSetXYZ_inv(&vertex, ((screen.x - center.x) / camera->stScale.xX) / size,
                            ((screen.y - center.y) / camera->stScale.xY) / size,
                            depth * ((size + 1.0f) / size));
    } else {
        MTH3D_M_vSetXYZ_inv(&vertex, (screen.x - center.x) / camera->stScale.xX,
                            (screen.y - center.y) / camera->stScale.xY, 0.0f);
    }

    GLI_xGetCameraMatrix(camera, &matrix);
    POS_fn_vInvertMatrix(&inverse, &matrix);
    POS_fn_vMulMatrixVertex(position, &inverse, &vertex);
}

void GLI_vAdjustCameraToMirror(GLD_tdstViewportAttributes_* viewport, MTH3D_tdstVector_* result,
                               MTH3D_tdstVector_* screen)
{
    POS_tdstMatrix_ inverse;
    MTH3D_tdstVector_ vertex;
    GLI_tdstSpecificAttributesFor3D_* cameraCell;
    GLI_tdstCamera_* camera;
    f32 nearClip;
    f32 depth;
    f32 doubleNear;
    f32 scaleX;
    f32 scaleY;

    cameraCell = viewport->p_vSpecificToXD;
    camera = cameraCell->p_stCam;
    if (camera->lCameraMode != CAMERA_TYPE_PERSPECTIVE) {
        f32 zero = 0.0f;

        MTH3D_M_vSetXYZ_inv(result, zero, zero, zero);
        return;
    }
    nearClip = camera->xNear;
    depth = screen->xZ;
    doubleNear = nearClip + nearClip;
    scaleX = doubleNear / (depth * camera->xXProjectionRatio);
    scaleY = doubleNear / (depth * camera->xYProjectionRatio);
    vertex.xZ = nearClip / depth;
    vertex.xX = (camera->stTrans.xX - screen->xX) * scaleX / (f32) viewport->values.dwWidth;
    vertex.xY = (camera->stTrans.xY - screen->xY) * scaleY / (f32) viewport->values.dwHeight;
    POS_fn_vInvertMatrix(&inverse, &camera->stMatrix);
    POS_fn_vMulMatrixVertex(result, &inverse, &vertex);
}

void GLI_xAdjustCameraToViewport2(GLD_tdstDeviceAttributes_* device,
                                  GLD_tdstViewportAttributes_* attributes, GLI_tdstCamera_* camera)
{
    GLDCameraAttributesPrefix* values = &attributes->values;
    f32 scaledWidth;
    f32 half;
    f32 height;
    f32 width;
    f32 one;
    f32 offset;
    f32 distance;
    f32 inverseLength;

    width = (f32) values->dwWidth;
    scaledWidth = width * device->xPixelDimensionX;
    height = (f32) values->dwHeight;
    half = 0.5f;

    camera->stTrans.xX = (f32) values->iOffsetPosX + width * half;
    camera->stTrans.xY = (f32) values->iOffsetPosY + height * half;

    if (camera->lCameraMode != CAMERA_TYPE_PERSPECTIVE) {
        return;
    }

    one = 1.0f;
    camera->xXProjectionRatio = one / tanf(camera->xAlphaX * half);
    camera->xYProjectionRatio = one / tanf(camera->xAlphaY * half);

    camera->xScreen = scaledWidth * half * camera->xXProjectionRatio;
    one = one / device->xPixelDimensionX;
    one = inv_MTH_fn_xNegateTied(one);
    camera->stScale.xX = one;
    camera->stScale.xY = -(height / ((camera->xScreen + camera->xScreen) / camera->xYProjectionRatio));

    offset = (f32) (s32) (values->dwClipRightInPix - values->iOffsetPosX - (values->dwWidth >> 1));
    distance = (f32) (values->dwWidth >> 1) / tanf(camera->xAlphaX * 0.5f);
    inverseLength = distance * distance + offset * offset;
    inverseLength = inv_MTH_fn_xDivideBySqrt(1.0f, inverseLength);
    MTH3D_M_vSetXYZ_inv(&camera->clipPlanes[0], -distance * inverseLength, 0,
                        -offset * inverseLength);

    offset = (f32) (s32) ((values->dwWidth >> 1) - (values->dwClipLeftInPix - values->iOffsetPosX));
    inverseLength = distance * distance + offset * offset;
    inverseLength = inv_MTH_fn_xDivideBySqrt(1.0f, inverseLength);
    MTH3D_M_vSetXYZ_inv(&camera->clipPlanes[1], distance * inverseLength, 0,
                        -offset * inverseLength);

    offset = (f32) (s32) (values->dwClipBottomInPix - values->iOffsetPosY - (values->dwHeight >> 1));
    distance = (f32) (values->dwHeight >> 1) / tanf(camera->xAlphaY * 0.5f);
    inverseLength = distance * distance + offset * offset;
    inverseLength = inv_MTH_fn_xDivideBySqrt(1.0f, inverseLength);
    MTH3D_M_vSetXYZ_inv(&camera->clipPlanes[3], 0, -distance * inverseLength,
                        -offset * inverseLength);

    offset = (f32) (s32) ((values->dwHeight >> 1) - (values->dwClipTopInPix - values->iOffsetPosY));
    inverseLength = distance * distance + offset * offset;
    inverseLength = inv_MTH_fn_xDivideBySqrt(1.0f, inverseLength);
    camera->clipPlanes[2].xX = 0;
    camera->clipPlanes[2].xW = 0;
    camera->clipPlanes[3].xW = 0;
    camera->clipPlanes[1].xW = 0;
    camera->clipPlanes[0].xW = 0;
    camera->clipPlanes[2].xY = distance * inverseLength;
    camera->clipPlanes[2].xZ = -offset * inverseLength;

    inv_GLI_RW_vSetCameraFOV(camera);
}

s32 func_00118390(BoundsGeometryView* geometry, POS_tdstMatrix_* objectMatrix,
                  POS_tdstMatrix_* viewMatrix, GLI_tdstCamera_* camera,
                  GLDCameraAttributesPrefix* rectangle, u32 flags)
{
    MTH3D_tdstVector_ corners[BOUNDS_CORNER_COUNT];
    ProjectedVertex view[BOUNDS_CORNER_COUNT];
    ProjectedVertex clipped[BOUNDS_CORNER_COUNT + 1];
    ProjectedVertex projected[BOUNDS_CORNER_COUNT + 1];
    POS_tdstMatrix_ matrix;
    u8 behind[BOUNDS_CORNER_COUNT];
    s32 allInFront;
    f32 left, right, top, bottom;
    f32 xMin, xMax, yMin, yMax;
    f32 zMin;
    s16 i, j, count;
    MTH3D_tdstVector_* corner;
    s32 offset;

    allInFront = 1;
    left = rectangle->dwClipLeftInPix;
    right = rectangle->dwClipRightInPix;
    top = rectangle->dwClipTopInPix;
    bottom = rectangle->dwClipBottomInPix;

    offset = geometry->vertexCount * sizeof(MTH3D_tdstVector_);
    for (i = 0; i < BOUNDS_CORNER_COUNT; i++) {
        offset -= sizeof(MTH3D_tdstVector_);
        corner = (MTH3D_tdstVector_*) (i * sizeof(MTH3D_tdstVector_));
        corner = (MTH3D_tdstVector_*) ((u8*) corner + (s32) corners);
        *corner = *(MTH3D_tdstVector_*) (offset + (s32) geometry->vertices);
    }

    _POS_fn_vMulMatrixMatrix(&matrix, viewMatrix, objectMatrix);
    GLI_xSerialLinearOp(BOUNDS_CORNER_COUNT, corners, view, &matrix);

    zMin = camera->xFar;
    for (i = 0; i < BOUNDS_CORNER_COUNT; i++) {
        if ((g_cSinusEffectState == SINUS_EFFECT_FIRST_VIEWPORT && !(flags & BOUNDS_SECOND_VIEWPORT)) ||
            (g_cSinusEffectState == SINUS_EFFECT_SECOND_VIEWPORT && (flags & BOUNDS_SECOND_VIEWPORT)) ||
            g_cSinusEffectState == SINUS_EFFECT_ALL_VIEWPORTS) {
            GLI_vSinusEffetOn3DVertex((MTH3D_tdstVector_*) &view[i]);
        }
        zMin = inv_MTH_fn_xMinimum(zMin, view[i].z);
        if (view[i].z < camera->xNear) {
            behind[i] = 1;
        } else {
            behind[i] = 0;
        }
    }
    yMax = inv_MTH_fn_xMaximum(zMin, camera->xNear);
    camera->xNear = yMax;

    if (behind[0] || behind[1] || behind[2] || behind[3]) {
        allInFront = 0;
    }

    if (allInFront) {
        GLI_xSerialProjection(camera, BOUNDS_CORNER_COUNT, view, projected);
        xMin = right;
        xMax = left;
        yMin = bottom;
        yMax = xMax;
        for (i = 0; i < BOUNDS_CORNER_COUNT; i++) {
            if (projected[i].x < xMin) {
                xMin = projected[i].x;
            }
            if (xMax < projected[i].x) {
                xMax = projected[i].x;
            }
            if (projected[i].y < yMin) {
                yMin = projected[i].y;
            }
            if (yMax < projected[i].y) {
                yMax = projected[i].y;
            }
        }
        if (xMax < xMin) {
            return 0;
        }
        if (yMax < yMin) {
            return 0;
        }
        xMin -= 1.0f;
        xMax += 1.0f;
        yMin -= 1.0f;
        yMax += 1.0f;
        if (xMin < left) {
            xMin = left;
        }
        if (right < xMax) {
            xMax = right;
        }
        if (yMin < top) {
            yMin = top;
        }
        if (bottom < yMax) {
            yMax = bottom;
        }
        {
            u32 rightEdge, leftEdge, topEdge, bottomEdge;

            bottomEdge = inv_MTH_fn_lToInteger(yMax);
            leftEdge = inv_MTH_fn_lToInteger(xMin);
            rightEdge = inv_MTH_fn_lToInteger(xMax);
            topEdge = inv_MTH_fn_lToInteger(yMin);
            rectangle->dwClipLeftInPix = leftEdge;
            rectangle->dwClipRightInPix = rightEdge;
            rectangle->dwClipTopInPix = topEdge;
            rectangle->dwClipBottomInPix = bottomEdge;
        }
        return 1;
    }

    if (behind[0] && behind[1] && behind[2] && behind[3]) {
        return 0;
    }

    count = 0;
    for (i = 0; i < BOUNDS_CORNER_COUNT; i++) {
        if (!behind[i]) {
            clipped[count].x = view[i].x;
            clipped[count].y = view[i].y;
            clipped[count].z = view[i].z;
            count++;
        } else {
            f32 t;

            j = (i + 1) % BOUNDS_CORNER_COUNT;
            if (!behind[j]) {
                t = (yMax - view[i].z) / view[j].z;
                clipped[count].x = view[i].x + t * view[j].x;
                clipped[count].y = view[i].y + t * view[j].y;
                clipped[count].z = yMax;
                count++;
            }
            j = (i + 3) % BOUNDS_CORNER_COUNT;
            if (!behind[j]) {
                t = (yMax - view[i].z) / view[j].z;
                clipped[count].x = view[i].x + t * view[j].x;
                clipped[count].y = view[i].y + t * view[j].y;
                clipped[count].z = yMax;
                count++;
            }
        }
    }

    GLI_xSerialProjection(camera, count, clipped, projected);
    xMin = right;
    xMax = left;
    yMin = bottom;
    yMax = xMax;
    for (i = 0; i < count; i++) {
        if (projected[i].x < xMin) {
            xMin = projected[i].x;
        }
        if (xMax < projected[i].x) {
            xMax = projected[i].x;
        }
        if (projected[i].y < yMin) {
            yMin = projected[i].y;
        }
        if (yMax < projected[i].y) {
            yMax = projected[i].y;
        }
    }
    if (xMax < xMin) {
        return 0;
    }
    if (yMax < yMin) {
        return 0;
    }
    xMin -= 1.0f;
    xMax += 1.0f;
    yMin -= 1.0f;
    yMax += 1.0f;
    if (xMin < left) {
        xMin = left;
    }
    if (right < xMax) {
        xMax = right;
    }
    if (yMin < top) {
        yMin = top;
    }
    if (bottom < yMax) {
        yMax = bottom;
    }
    {
        u32 rightEdge, leftEdge, topEdge, bottomEdge;

        bottomEdge = inv_MTH_fn_lToInteger(yMax);
        leftEdge = inv_MTH_fn_lToInteger(xMin);
        rightEdge = inv_MTH_fn_lToInteger(xMax);
        topEdge = inv_MTH_fn_lToInteger(yMin);
        rectangle->dwClipLeftInPix = leftEdge;
        rectangle->dwClipRightInPix = rightEdge;
        rectangle->dwClipTopInPix = topEdge;
        rectangle->dwClipBottomInPix = bottomEdge;
    }
    return 1;
}

void func_00118AF0(GLI_tdstCamera_* destination, GLI_tdstCamera_* source)
{
    *destination = *source;
}

void GLI_xSetCameraNearFar(GLI_tdstCamera_* p_stCam, f32 xNear, f32 xFar)
{
    p_stCam->xNear = CAMERA_DEFAULT_NEAR;
    if (xFar == 0.0f) {
        p_stCam->xFar = CAMERA_UNLIMITED_FAR;
    } else {
        p_stCam->xFar = xFar;
    }
}

void GLI_xGetCameraMatrix(GLI_tdstCamera_* camera, POS_tdstMatrix_* matrix)
{
    *matrix = camera->stMatrix;
}

void GLI_xSetCameraMatrix(GLI_tdstCamera_* camera, POS_tdstMatrix_* matrix)
{
    camera->stMatrix = *matrix;
}

void GLI_xSetCameraAspectAndRatio(GLI_tdstCamera_* p_stCam, f32 xAlphaX, f32 xRatio)
{
    f32 product = xAlphaX * xRatio;

    p_stCam->xAlphaX = xAlphaX;
    p_stCam->xRatio = xRatio;
    p_stCam->xAlphaY = product;
}

void GLI_xGetCameraAspectAndRatio(const GLI_tdstCamera_* p_stCam, f32* p_xAlphaX, f32* p_xRatio)
{
    *p_xAlphaX = p_stCam->xAlphaX;
    *p_xRatio = p_stCam->xRatio;
}

void GLI_xSetCameraRatio(GLI_tdstCamera_* p_stCam, f32 xRatio)
{
    f32 product = p_stCam->xAlphaX * xRatio;

    p_stCam->xRatio = xRatio;
    p_stCam->xAlphaY = product;
}

void GLI_xSetViewportCamera(s16 hDevice, s16 hViewport, GLI_tdstCamera_* p_stCam)
{
    GLD_tdstViewportAttributes_ attributes;
    GLI_tdstSpecificAttributesFor3D_* cameraCell;

    if (GLD_bGetViewportAttributes(hDevice, hViewport, &attributes) != 0) {
        cameraCell = attributes.p_vSpecificToXD;
        if (p_stCam != cameraCell->p_stCam) {
            cameraCell->p_stCam = p_stCam;
            GLI_xAdjustCameraToViewport(hDevice, hViewport, p_stCam);
        }
    }
}

void GLI_xGetViewportCamera(s16 hDevice, s16 hViewport, GLI_tdstCamera_** p_stCam)
{
    GLD_tdstViewportAttributes_ attributes;
    GLI_tdstSpecificAttributesFor3D_* cameraCell;

    if (GLD_bGetViewportAttributes(hDevice, hViewport, &attributes) != 0) {
        cameraCell = attributes.p_vSpecificToXD;
        *p_stCam = cameraCell->p_stCam;
    }
}
