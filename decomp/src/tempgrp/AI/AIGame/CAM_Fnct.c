#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"

f32 CAM_fn_xGetARealParameter(tdstCineinfo_* cineinfo, f32* initValue, f32* currentValue,
                              f32* workValue, f32* visibilityValue, s32 mode)
{
    if (mode == 1) {
        if ((cineinfo->ucVolIAFlags & 1) != 0) {
            return *workValue;
        }
        return *currentValue;
    } else if (mode == -1) {
        return *initValue;
    } else if ((cineinfo->ucPerIAFlags & 2) != 0) {
        return *visibilityValue;
    }
    return *currentValue;
}

MTH3D_tdstVector_* CAM_fn_p_stGetAVectorParameter(tdstCineinfo_* cineinfo,
                                                  MTH3D_tdstVector_* initValue,
                                                  MTH3D_tdstVector_* currentValue,
                                                  MTH3D_tdstVector_* workValue,
                                                  MTH3D_tdstVector_* visibilityValue, s32 mode)
{
    s32 flag;

    if (mode == 1) {
        flag = cineinfo->ucVolIAFlags & 1;
        return flag != 0 ? workValue : currentValue;
    }
    if (mode == -1) {
        return initValue;
    }
    flag = cineinfo->ucPerIAFlags & 2;
    return flag != 0 ? visibilityValue : currentValue;
}

tdstNodeInterpret_* CAM_fn_p_stGetVectorParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                  tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    MTH3D_tdstVector_* p_stVisValue = NULL;
    MTH3D_tdstVector_* p_stWorkValue = NULL;
    MTH3D_tdstVector_* p_stCurrentValue = NULL;
    MTH3D_tdstVector_* p_stInitialValue = NULL;
    u16 operatorId = AI_NODE_ID(p_stTree);
    tdstCineinfo_* hCineInfo;
    s32 mode;
    MTH3D_tdstVector_* value;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    hCineInfo = inv_HIE_fn_p_stGetEngineObject(perso)->h_Cineinfo;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    mode = p_stValue->u.lValue;
    switch (operatorId) {
    case eFct_Cam_GetShiftTarget:
        p_stVisValue = &hCineInfo->hInit->stShiftTarget;
        p_stWorkValue = &hCineInfo->hCurrent->stShiftTarget;
        p_stCurrentValue = &hCineInfo->hWork->stShiftTarget;
        p_stInitialValue = &hCineInfo->hVisibility->stShiftTarget;
        break;
    case eFct_Cam_GetShiftPos:
        p_stVisValue = &hCineInfo->hInit->stShiftPos;
        p_stWorkValue = &hCineInfo->hCurrent->stShiftPos;
        p_stCurrentValue = &hCineInfo->hWork->stShiftPos;
        p_stInitialValue = &hCineInfo->hVisibility->stShiftPos;
        break;
    }
    value = CAM_fn_p_stGetAVectorParameter(hCineInfo, p_stVisValue, p_stWorkValue, p_stCurrentValue,
                                           p_stInitialValue, mode);
    p_stValue->u.stVector = *value;
    p_stValue->ucType = E_vt_Vector;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stGetRealParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    f32* initValue = NULL;
    f32* currentValue = NULL;
    f32* workValue = NULL;
    f32* visibilityValue = NULL;
    u16 operatorId = AI_NODE_ID(p_stTree);
    tdstCineinfo_* cineinfo;
    s32 mode;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    cineinfo = inv_HIE_fn_p_stGetEngineObject(perso)->h_Cineinfo;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    mode = p_stValue->u.lValue;
    switch (operatorId) {
    case eFct_Cam_GetDistMin:
        initValue = &cineinfo->hInit->xDistMin;
        currentValue = &cineinfo->hCurrent->xDistMin;
        workValue = &cineinfo->hWork->xDistMin;
        visibilityValue = &cineinfo->hVisibility->xDistMin;
        break;
    case eFct_Cam_GetDistMax:
        initValue = &cineinfo->hInit->xDistMax;
        currentValue = &cineinfo->hCurrent->xDistMax;
        workValue = &cineinfo->hWork->xDistMax;
        visibilityValue = &cineinfo->hVisibility->xDistMax;
        break;
    case eFct_Cam_GetBoundDistMin:
        initValue = &cineinfo->hInit->xBoundDistMin;
        currentValue = &cineinfo->hCurrent->xBoundDistMin;
        workValue = &cineinfo->hWork->xBoundDistMin;
        visibilityValue = &cineinfo->hVisibility->xBoundDistMin;
        break;
    case eFct_Cam_GetBoundDistMax:
        initValue = &cineinfo->hInit->xBoundDistMax;
        currentValue = &cineinfo->hCurrent->xBoundDistMax;
        workValue = &cineinfo->hWork->xBoundDistMax;
        visibilityValue = &cineinfo->hVisibility->xBoundDistMax;
        break;
    case eFct_Cam_GetAngleAlpha:
        initValue = &cineinfo->hInit->xAngleAlpha;
        currentValue = &cineinfo->hCurrent->xAngleAlpha;
        workValue = &cineinfo->hWork->xAngleAlpha;
        visibilityValue = &cineinfo->hVisibility->xAngleAlpha;
        break;
    case eFct_Cam_GetAngleShiftAlpha:
        initValue = &cineinfo->hInit->xAngleShiftAlpha;
        currentValue = &cineinfo->hCurrent->xAngleShiftAlpha;
        workValue = &cineinfo->hWork->xAngleShiftAlpha;
        visibilityValue = &cineinfo->hVisibility->xAngleShiftAlpha;
        break;
    case eFct_Cam_GetAngleTheta:
        initValue = &cineinfo->hInit->xAngleTheta;
        currentValue = &cineinfo->hCurrent->xAngleTheta;
        workValue = &cineinfo->hWork->xAngleTheta;
        visibilityValue = &cineinfo->hVisibility->xAngleTheta;
        break;
    case eFct_Cam_GetAngleShiftTheta:
        initValue = &cineinfo->hInit->xAngleShiftTheta;
        currentValue = &cineinfo->hCurrent->xAngleShiftTheta;
        workValue = &cineinfo->hWork->xAngleShiftTheta;
        visibilityValue = &cineinfo->hVisibility->xAngleShiftTheta;
        break;
    case eFct_Cam_GetLinearSpeed:
        initValue = &cineinfo->hInit->xLinearSpeed;
        currentValue = &cineinfo->hCurrent->xLinearSpeed;
        workValue = &cineinfo->hWork->xLinearSpeed;
        visibilityValue = &cineinfo->hVisibility->xLinearSpeed;
        break;
    case eFct_Cam_GetLinearIncreaseSpeed:
        initValue = &cineinfo->hInit->xLinearIncreaseSpeed;
        currentValue = &cineinfo->hCurrent->xLinearIncreaseSpeed;
        workValue = &cineinfo->hWork->xLinearIncreaseSpeed;
        visibilityValue = &cineinfo->hVisibility->xLinearIncreaseSpeed;
        break;
    case eFct_Cam_GetLinearDecreaseSpeed:
        initValue = &cineinfo->hInit->xLinearDecreaseSpeed;
        currentValue = &cineinfo->hCurrent->xLinearDecreaseSpeed;
        workValue = &cineinfo->hWork->xLinearDecreaseSpeed;
        visibilityValue = &cineinfo->hVisibility->xLinearDecreaseSpeed;
        break;
    case eFct_Cam_GetAngularSpeed:
        initValue = &cineinfo->hInit->xAngularSpeed;
        currentValue = &cineinfo->hCurrent->xAngularSpeed;
        workValue = &cineinfo->hWork->xAngularSpeed;
        visibilityValue = &cineinfo->hVisibility->xAngularSpeed;
        break;
    case eFct_Cam_GetAngularIncreaseSpeed:
        initValue = &cineinfo->hInit->xAngularIncreaseSpeed;
        currentValue = &cineinfo->hCurrent->xAngularIncreaseSpeed;
        workValue = &cineinfo->hWork->xAngularIncreaseSpeed;
        visibilityValue = &cineinfo->hVisibility->xAngularIncreaseSpeed;
        break;
    case eFct_Cam_GetAngularDecreaseSpeed:
        initValue = &cineinfo->hInit->xAngularDecreaseSpeed;
        currentValue = &cineinfo->hCurrent->xAngularDecreaseSpeed;
        workValue = &cineinfo->hWork->xAngularDecreaseSpeed;
        visibilityValue = &cineinfo->hVisibility->xAngularDecreaseSpeed;
        break;
    case eFct_Cam_GetTargetSpeed:
        initValue = &cineinfo->hInit->xTargetSpeed;
        currentValue = &cineinfo->hCurrent->xTargetSpeed;
        workValue = &cineinfo->hWork->xTargetSpeed;
        visibilityValue = &cineinfo->hVisibility->xTargetSpeed;
        break;
    case eFct_Cam_GetTargetIncreaseSpeed:
        initValue = &cineinfo->hInit->xTargetIncreaseSpeed;
        currentValue = &cineinfo->hCurrent->xTargetIncreaseSpeed;
        workValue = &cineinfo->hWork->xTargetIncreaseSpeed;
        visibilityValue = &cineinfo->hVisibility->xTargetIncreaseSpeed;
        break;
    case eFct_Cam_GetTargetDecreaseSpeed:
        initValue = &cineinfo->hInit->xTargetDecreaseSpeed;
        currentValue = &cineinfo->hCurrent->xTargetDecreaseSpeed;
        workValue = &cineinfo->hWork->xTargetDecreaseSpeed;
        visibilityValue = &cineinfo->hVisibility->xTargetDecreaseSpeed;
        break;
    case eFct_Cam_GetFocal:
        initValue = &cineinfo->hInit->xFocal;
        currentValue = &cineinfo->hCurrent->xFocal;
        workValue = &cineinfo->hWork->xFocal;
        visibilityValue = &cineinfo->hVisibility->xFocal;
        break;
    case eFct_Cam_GetZMin:
        initValue = &cineinfo->hInit->xZMin;
        currentValue = &cineinfo->hCurrent->xZMin;
        workValue = &cineinfo->hWork->xZMin;
        visibilityValue = &cineinfo->hVisibility->xZMin;
        break;
    case eFct_Cam_GetZMax:
        initValue = &cineinfo->hInit->xZMax;
        currentValue = &cineinfo->hCurrent->xZMax;
        workValue = &cineinfo->hWork->xZMax;
        visibilityValue = &cineinfo->hVisibility->xZMax;
        break;
    }
    p_stValue->u.xValue = CAM_fn_xGetARealParameter(cineinfo, initValue, currentValue, workValue,
                                                visibilityValue, mode);
    p_stValue->ucType = E_vt_Float;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stGetTargetedSuperObject(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    tdstCineinfo_* cineinfo;
    HIE_tdstSuperObject_* target;
    s32 mode;
    s32 value;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    cineinfo = inv_HIE_fn_p_stGetEngineObject(perso)->h_Cineinfo;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    mode = p_stValue->u.lValue;
    if (mode == 1) {
        if ((cineinfo->ucVolIAFlags & 1) == 0) {
            target = cineinfo->hCurrent->hSuperObjectTargeted;
        } else {
            target = cineinfo->hWork->hSuperObjectTargeted;
        }
    } else if (mode == -1) {
        target = cineinfo->hInit->hSuperObjectTargeted;
    } else if ((cineinfo->ucPerIAFlags & 2) != 0) {
        target = cineinfo->hVisibility->hSuperObjectTargeted;
    } else {
        target = cineinfo->hCurrent->hSuperObjectTargeted;
    }
    if (target != NULL) {
        if (target->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
            value = 0;
        } else {
            value = (s32) target->hLinkedObject;
        }
    } else {
        value = 0;
    }
    p_stValue->u.lValue = value;
    p_stValue->ucType = E_vt_Perso;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stGetTypeOfViewport(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree,
                                                   tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    tdstCineinfo_* cineinfo;
    s32 lCurrentValue;
    s32 lTypeOfViewport;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    cineinfo = inv_HIE_fn_p_stGetEngineObject(perso)->h_Cineinfo;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
    lCurrentValue = p_stValue->u.lValue;
    if (lCurrentValue == 1) {
        if (cineinfo->ucVolIAFlags & 1)
            lTypeOfViewport = cineinfo->hWork->eTypeOfViewport;
        else
            lTypeOfViewport = cineinfo->hCurrent->eTypeOfViewport;
    } else if (lCurrentValue == -1) {
        lTypeOfViewport = cineinfo->hInit->eTypeOfViewport;
    } else if (cineinfo->ucPerIAFlags & 2) {
        lTypeOfViewport = cineinfo->hVisibility->eTypeOfViewport;
    } else {
        lTypeOfViewport = cineinfo->hCurrent->eTypeOfViewport;
    }
    p_stValue->u.lValue = lTypeOfViewport;
    p_stValue->ucType = E_vt_Integer;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stGetSectorCameraType(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                    tdstNodeInterpret_* p_stTree,
                                                    tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    tdstEngineObject_* object;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    object = perso->hLinkedObject;
    p_stValue->u.lValue = fn_cGetSectorCameraType(object->h_SectInfo->hCurrentSector);
    p_stValue->ucType = E_vt_Integer;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stGetCameraOfViewport(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree,
                                                   tdstGetSetParam_* p_stValue)
{
    s32 eFuncId;

    switch (AI_NODE_ID(p_stTree)) {
    case eFct_Cam_GetCameraOfViewport:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stValue);
        if (CAM_fn_hGetActiveCamera(p_stValue->u.lValue) != NULL &&
            CAM_fn_hGetActiveCamera(p_stValue->u.lValue)->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            eFuncId = (s32) CAM_fn_hGetActiveCamera(p_stValue->u.lValue)->hLinkedObject;
        } else {
            eFuncId = 0;
        }
        break;
    case eFct_Cam_GetMainCamera:
        if (CAM_fn_hGetActiveCamera(1) != NULL &&
            CAM_fn_hGetActiveCamera(1)->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
            eFuncId = (s32) CAM_fn_hGetActiveCamera(1)->hLinkedObject;
        } else {
            eFuncId = 0;
        }
        break;
    default:
        return p_stTree;
    }
    p_stValue->u.lValue = eFuncId;
    p_stValue->ucType = E_vt_Perso;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stComputeTargetWithTgtPerso(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                         tdstNodeInterpret_* p_stTree,
                                                         tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    tdstEngineObject_* object;
    CAM_tdstUpdateCamera_ camera;
    u16 flags;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    object = perso->hLinkedObject;
    CAM_fn_vSetCineinfoWorkFromCurrent(object->h_Cineinfo);
    CAM_fn_vInitCameraStructure(perso, &camera);
    if (camera.hCineinfoCurrent->hSuperObjectTargeted == NULL) {
        f32 zero = 0.0f;

        p_stValue->ucType = E_vt_Vector;
        MTH3D_M_vSetXYZ_inv(&p_stValue->u.stVector, zero, zero, zero);
        return p_stTree;
    }
    flags = camera.hCineinfoWork->uwIAFlags;
    camera.hCineinfoWork->uwIAFlags = flags | 1;
    camera.hCineinfoWork->uwIAFlags &= ~0x1000;
    CAM_fn_vComputeReferencePoint(&camera);
    CAM_fn_vComputeTarget(&camera);
    camera.hCineinfoWork->uwIAFlags = flags;
    p_stValue->u.stVector = camera.stCptPos.stTarget;
    p_stValue->ucType = E_vt_Vector;
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stComputeCurrentTarget(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                      tdstNodeInterpret_* p_stTree,
                                                      tdstGetSetParam_* p_stValue)
{
    HIE_tdstSuperObject_* perso;
    tdstDynam_* p_stDynam;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    p_stDynam = inv_HIE_fn_p_stGetEngineObject(perso)->h_Dynam;
    if (p_stDynam != NULL) {
        p_stValue->u.stVector = p_stDynam->stParsingDatas.stVector;
        p_stValue->ucType = E_vt_Vector;
    }
    return p_stTree;
}

tdstNodeInterpret_* CAM_fn_p_stGetBestPos(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue)
{
    CAM_tdstUpdateCamera_ stStruct;
    MTH3D_tdstVector_ position;
    HIE_tdstSuperObject_* hCamera;
    tdstCineinfo_* cineinfo;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hCamera);
    cineinfo = inv_HIE_fn_p_stGetEngineObject(hCamera)->h_Cineinfo;
    if (cineinfo->bCanDoBestPos == 0) {
        p_stValue->u.stVector = *(MTH3D_tdstVector_*) &hCamera->gu_hGlobalMatrix->stMatrix.stCol_3;
        p_stValue->ucType = E_vt_Vector;
        return p_stTree;
    }
    CAM_fn_vSetCineinfoWorkFromCurrent(cineinfo);
    CAM_fn_vInitCameraStructure(hCamera, &stStruct);
    cRefAxisIsAlreadyComputed = (stStruct.hCineinfoWork->uwIAFlags >> 14) & 1;
    cNoDynChangeTheta = (stStruct.hCineinfoWork->uwIAFlags >> 5) & 1;
    cCanTestStatic = 1;
    CAM_fn_vComputeReferencePoint(&stStruct);
    CAM_fn_vComputeTarget(&stStruct);
    CAM_fn_vComputeRealWantedPos(&stStruct);
    CAM_fn_vComputeMovePosWithDynTheta(&stStruct);
    if (stStruct.hCineinfoCurrent->hSuperObjectTargeted == NULL) {
        return p_stTree;
    }
    CAM_fn_vForceFailureVisibility(&stStruct);
    if (stStruct.hCineinfo->eState == CAM_e_State_FailureVisibility) {
        position = stStruct.hCineinfo->stLastFailedWanted;
        cineinfo->bCanDoBestPos = 0;
    } else {
        f32 zero = 0.0f;

        MTH3D_M_vSetXYZ_inv(&position, zero, zero, zero);
    }
    stStruct.hCineinfo->eState = CAM_e_State_GoToOptimal;
    p_stValue->u.stVector = position;
    p_stValue->ucType = E_vt_Vector;
    return p_stTree;
}
