#include "common.h"

#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "AI/AI.h"
#include "GLI/GLI.h"

#define CAMERA_VIEWPORT_COUNT 3

tdstViewportManagement_* CAM_fn_p_stGetViewPortManagement(s32 index)
{
    if (index >= CAMERA_VIEWPORT_COUNT || index == 0) {
        return NULL;
    }
    return &g_stEngineStructure.a_hViewportArray[index - 1];
}

HIE_tdstSuperObject_* CAM_fn_hGetActiveCamera(s32 _lViewportNumber)
{
    return CAM_fn_p_stGetViewPortManagement(_lViewportNumber)->hCamera;
}

void CAM_fn_vAssignCameraToAViewport(s32 _lViewportNumber, HIE_tdstSuperObject_* _hCamera)
{
    tdstViewportManagement_* hViewport = CAM_fn_p_stGetViewPortManagement(_lViewportNumber);

    hViewport->hCamera = _hCamera;
    hViewport->bValid = 1;
}

void CAM_fn_vReInitCameras(void)
{
    tdstNodeCamera_* node;
    tdstNodeCamera_* next;
    s32 i;

    for (i = 1; i < CAMERA_VIEWPORT_COUNT; i++) {
        tdstViewportManagement_* viewport = CAM_fn_p_stGetViewPortManagement(i);

        viewport->hTempCamera = NULL;
        viewport->bValid = 0;
    }
    for (node = (&g_stEngineStructure.hCameraList)->hFirstElementDyn, next = node ? node->hNextBrotherDyn : NULL; node != NULL;
         node = next, next = node ? node->hNextBrotherDyn : NULL) {
        tdstEngineObject_* object = node->p_stSuperObjectCamera->hLinkedObject;
        tdstInternalStructurCineinfo_* slot = object->h_Cineinfo->hInit;

        if (slot->bIsActive && slot->eTypeOfViewport != CAM_e_NoViewport)
            CAM_fn_vAssignCameraToAViewport(slot->eTypeOfViewport, node->p_stSuperObjectCamera);
    }
}

void CAM_fn_vInitCameras(void)
{
    s32 i;

    g_stEngineStructure.a_hViewportArray =
        AI_fn_p_vTrueAlloc(gp_6F74, (CAMERA_VIEWPORT_COUNT - 1) * sizeof(tdstViewportManagement_));
    CAM_fn_vReInitCameras();
    for (i = 1; i < CAMERA_VIEWPORT_COUNT; i++) {
        tdstViewportManagement_* viewport = CAM_fn_p_stGetViewPortManagement(i);

        GLI_xCreateCamera(&viewport->p_stCamera, 0x67);
        GLI_xSetCameraAspectAndRatio(viewport->p_stCamera, 1.5f, 0.75f);
    }
    CAM_fn_vLoadCameraConstants();
}

s32 CAM_fn_bSuperObjectIsACamera(HIE_tdstSuperObject_* _h_SprObj)
{
    tdstEngineObject_* object;

    if (_h_SprObj->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
        return 0;
    }
    object = _h_SprObj->hLinkedObject;
    return object->h_Cineinfo != NULL;
}

void CAM_fn_vActualizeAViewport(s32 _lViewport)
{
    POS_tdstMatrix_ local;
    MTH3D_tdstVector_ x;
    MTH3D_tdstVector_ y;
    MTH3D_tdstVector_ z;
    f32 aspect;
    f32 ratio;
    tdstViewportManagement_* view;
    HIE_tdstSuperObject_* superObject;
    POS_tdstMatrix_* matrix;
    tdstEngineObject_* object;
    void* camera;
    tdstCineinfo_* cineinfo;
    f32 wanted;

    view = CAM_fn_p_stGetViewPortManagement(_lViewport);
    superObject = view->hCamera;
    matrix = superObject->gu_hGlobalMatrix;
    POS_fn_vGetRotationVectors(matrix, &x, &y, &z);
    local = *matrix;

    MTH3D_M_vSetXYZ_inv(&y, inv_MTH_fn_xNegateTied(y.xX), inv_MTH_fn_xNegateTied(y.xY),
                        inv_MTH_fn_xNegateTied(y.xZ));
    _POS_fn_vSetRotationMatrix(&local, &x, &z, &y);
    POS_fn_vInvertIsoMatrix(&local, &local);

    camera = view->p_stCamera;
    GLI_xSetCameraMatrix(camera, &local);
    GLI_xGetCameraAspectAndRatio(camera, &aspect, &ratio);

    object = superObject->hLinkedObject;
    cineinfo = object->h_Cineinfo;
    wanted = cineinfo->hWork->xFocal;
    if (wanted == 0.0f) {
        wanted = cineinfo->hCurrent->xFocal;
    }

    if (wanted != aspect) {
        GLI_xSetCameraAspectAndRatio(camera, wanted, ratio);
        GLI_xSetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport, camera);
        GLI_xAdjustCameraToViewport(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport,
                                    camera);
    } else {
        GLI_xSetViewportCamera(g_stEngineStructure.hGLDDevice, g_stEngineStructure.hGLDViewport, camera);
    }
}

void CAM_fn_vCameraManagement(void)
{
    tdstNodeCamera_* node;
    s32 index;

    for (node = (&g_stEngineStructure.hCameraList)->hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
        tdstEngineObject_* object = node->p_stSuperObjectCamera->hLinkedObject;
        tdstCineinfo_* cineinfo = object->h_Cineinfo;
        tdstInternalStructurCineinfo_* state;

        if (cineinfo->ucVolIAFlags & 1) {
            state = cineinfo->hWork;
        } else {
            state = cineinfo->hCurrent;
        }
        if (state->eTypeOfViewport != CAM_e_NoViewport || state->bIsActive != 0) {
            if (state->eTypeOfViewport != CAM_e_NoViewport) {
                fn_vTreatDynamicHierarchy(node->p_stSuperObjectCamera, 1);
            } else {
                fn_vTreatDynamicHierarchy(node->p_stSuperObjectCamera, 0);
            }
        }
    }

    index = CAMERA_VIEWPORT_COUNT - 1;
    do {
        if (CAM_fn_p_stGetViewPortManagement(index)->bValid != 0) {
            CAM_fn_vActualizeAViewport(index);
        }
        index--;
    } while (index > 0);
}
