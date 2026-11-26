#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"

void* CAM_fn_p_stIs(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree, s32* p_lValue)
{
    HIE_tdstSuperObject_* hCamera;
    tdstEngineObject_* object;
    tdstGetSetParam_* param;
    tdstCineinfo_* hCineInfo;
    tdstInternalStructurCineinfo_* hIntCineInfo;
    u16 op;
    s32 lCurrentValue;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hCamera);
    object = hCamera->hLinkedObject;
    hCineInfo = object->h_Cineinfo;

    param = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = param + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    lCurrentValue = param->u.lValue;

    if (lCurrentValue == 1) {
        if (hCineInfo->ucVolIAFlags & 1) {
            hIntCineInfo = hCineInfo->hWork;
        } else {
            hIntCineInfo = hCineInfo->hCurrent;
        }
    } else if (lCurrentValue == -1) {
        hIntCineInfo = hCineInfo->hInit;
    } else if (hCineInfo->ucPerIAFlags & 2) {
        hIntCineInfo = hCineInfo->hVisibility;
    } else {
        hIntCineInfo = hCineInfo->hCurrent;
    }

    switch (op) {
    case eCond_Cam_IsActive:
        *p_lValue = hIntCineInfo->bIsActive;
        break;
    case eCond_Cam_IsViewportOwner:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        *p_lValue = param->u.lValue == hIntCineInfo->eTypeOfViewport;
        break;
    case eCond_Cam_IsFlagNoDynamicTarget:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0001;
        break;
    case eCond_Cam_IsFlagNoAverageMoveTgtPerso:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0002;
        break;
    case eCond_Cam_IsFlagNoParseCutAngle:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0004;
        break;
    case eCond_Cam_IsFlagNoVisibility:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0008;
        break;
    case eCond_Cam_IsFlagNoVisibilityWithDynHie:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0010;
        break;
    case eCond_Cam_IsFlagNoDynChangeTheta:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0020;
        break;
    case eCond_Cam_IsFlagNoShiftUntilPosReached:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0040;
        break;
    case eCond_Cam_IsFlagNoDynSpeed:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x0080;
        break;
    case eCond_Cam_IsFlagNoLinearParsing:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0001;
        break;
    case eCond_Cam_IsFlagNoLinearInertia:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0002;
        break;
    case eCond_Cam_IsFlagNoAngularParsing:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0004;
        break;
    case eCond_Cam_IsFlagNoAngularInertia:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0008;
        break;
    case eCond_Cam_IsFlagNoTargetParsing:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0010;
        break;
    case eCond_Cam_IsFlagNoTargetInertia:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0020;
        break;
    case eCond_Cam_IsFlagNoObstacle:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0040;
        break;
    case eCond_Cam_IsFlagFixedOrientation:
        *p_lValue = hIntCineInfo->uwDNMFlags & 0x0080;
        break;
    case eCond_Cam_IsFlagForcedTarget:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x1000;
        break;
    case eCond_Cam_IsFlagForcedAxis:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x4000;
        break;
    case eCond_Cam_IsFlagForcedPosition:
        *p_lValue = hIntCineInfo->uwIAFlags & 0x2000;
        break;
    }

    inv_AI_parameterPoolCursor = param;
    return p_stTree;
}
