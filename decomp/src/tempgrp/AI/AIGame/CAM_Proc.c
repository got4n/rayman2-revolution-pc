#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"

extern f32 D_00392FE8;
extern s32 inv_CAM_lPathDirection;

void CAM_fn_vChangeARealParameter(f32, void*, f32*, f32*, f32*, f32*, s32, s32);

extern u8 CAM_g_stCopyCameraConstants[0x80];

void* CAM_fn_p_stUpdatePosition(HIE_tdstSuperObject_* _hSuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    CAM_tdstUpdateCamera_ local;
    tdstCineinfo_* cineinfo;
    HIE_tdstSuperObject_* root;
    DNM_stDynamics* obj;
    tdstDynam_* p_stDynam;
    DNM_stReport_* mec;
    HIE_tdstSuperObject_* q;
    f32 timeDelta;

    root = _hSuperObjPerso;
    cineinfo = ((tdstEngineObject_*) root->hLinkedObject)->h_Cineinfo;
    CAM_fn_vSetCineinfoWorkFromCurrent(cineinfo);
    CAM_fn_vInitCameraStructure(root, &local);

    if (inv_CAM_hPreviousTarget != 0 &&
        inv_CAM_hPreviousTarget != cineinfo->hWork->hSuperObjectTargeted) {
        CAM_fn_vUpdateTargetPosition(cineinfo);
    }

    q = local.hCineinfoWork->hSuperObjectTargeted;
    p_stDynam = ((tdstEngineObject_*) q->hLinkedObject)->h_Dynam;
    if (p_stDynam == 0) {
        return p_stTree;
    }
    obj = p_stDynam->p_stDynamics;
    mec = obj->m_tdstDynamicsBaseBlock.m_pstReport;

    if ((obj->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80) == 0) {
        MEC_fn_vUpdateSpeed(q);
    }

    if (cineinfo->bCanDoBestPos == 0) {
        if (obj->m_tdstDynamicsBaseBlock.ulEndFlags & 0x1000000) {
            cineinfo->bCanDoBestPos = 1;
        }
    }

    if (!MTH3D_M_bIsZeroXYZ_inv(&mec->m_stAbsoluteCurrPosition.m_stLinear)) {
        MTH3D_tdstVector_* p14C;
        MTH3D_tdstVector_* p114 = &mec->m_stAbsoluteCurrSpeed.m_stLinear;
        tdstCineinfo_* p_stChecked;

        memcpy(&cineinfo->AveragePosition.a_CheckedSpeed[0], &cineinfo->AveragePosition.a_CheckedSpeed[1],
               7 * sizeof(MTH3D_tdstVector_));
        p14C = &mec->m_stAbsoluteCurrPosition.m_stLinear;
        cineinfo->AveragePosition.a_CheckedSpeed[7] = *p114;
        p_stChecked = local.hCineinfo;
        memcpy(&p_stChecked->AveragePosition.a_CheckedPos[0], &p_stChecked->AveragePosition.a_CheckedPos[1],
               7 * sizeof(MTH3D_tdstVector_));
        p_stChecked->AveragePosition.a_CheckedPos[7] = *p14C;
        timeDelta = inv_fn_xGetEngineClockDelta();
        {
            tdstCineinfo_* end = local.hCineinfo;
            memcpy(&end->AveragePosition.a_CheckedTime[0], &end->AveragePosition.a_CheckedTime[1], 7 * sizeof(f32));
            end->AveragePosition.a_CheckedTime[7] = timeDelta;
        }
    }

    CAM_fn_vUpdateGeneralCamera(&local);
    CAM_fn_vSendParametersToParsing(&local);
    inv_CAM_hPreviousTarget = cineinfo->hWork->hSuperObjectTargeted;
    cineinfo->ucVolIAFlags = 0;
    return p_stTree;
}

void CAM_fn_vChangeARealParameter(f32 value, void* _p_xInitialValue, f32* _p_xCurrentValue,
                                  f32* _p_xVisibilityValue, f32* _p_xWorkValue, f32* _lTypeOfChange,
                                  s32 _lChangingMode, s32 _xValue)
{
    f32 savedValue;
    f32 result;
    tdstCineinfo_* cineinfo;
    f32* initialValue;
    f32* visibilityValue;
    f32* workValue;
    s32 mode;
    s32 operation;
    s32 lOne;
    f32 xWorkValue;
    u8 flag;

    visibilityValue = _p_xWorkValue;
    mode = _lChangingMode;
    initialValue = _p_xCurrentValue;
    lOne = 1;
    cineinfo = _p_xInitialValue;
    workValue = _lTypeOfChange;
    operation = _xValue;
    savedValue = value;
    result = 0.0f;
    if (mode == lOne) {
        CAM_fn_vSetCineinfoWorkFromCurrent(cineinfo);
        _p_xVisibilityValue = workValue;
    }
    if (operation != 0) {
        if (!(0 < operation)) {
            if (operation == -1) {
                result = *initialValue;
                goto L_store;
            }
            *_p_xVisibilityValue = result;
            goto L_tail;
        }
        if (operation == lOne) {
            result = savedValue;
            goto L_store;
        }
        *_p_xVisibilityValue = result;
        goto L_tail;
    }
    flag = cineinfo->ucVolIAFlags;
    if ((flag & 1) != 0) {
        xWorkValue = *(volatile f32*) workValue;
        result = xWorkValue + savedValue;
    } else {
        xWorkValue = *_p_xVisibilityValue;
        result = xWorkValue + savedValue;
    }
L_store:
    *_p_xVisibilityValue = result;
L_tail:
    if (mode != 1) {
        *visibilityValue = result;
        *workValue = result;
    }
}

void CAM_fn_vActivateCamera(void* _hCineInfo, s32 _lTypeOfChange, s32 _bActive)
{
    tdstCineinfo_* camera;
    s32 mode;
    u8 enabled;
    tdstInternalStructurCineinfo_* hCineInfoTest;
    tdstInternalStructurCineinfo_* param0;

    camera = _hCineInfo;
    mode = _lTypeOfChange;
    enabled = (u8) _bActive;
    if (mode == 1) {
        CAM_fn_vSetCineinfoWorkFromCurrent(camera);
        hCineInfoTest = camera->hWork;
        param0 = hCineInfoTest;
    } else {
        hCineInfoTest = camera->hCurrent;
        param0 = hCineInfoTest;
        if ((camera->ucVolIAFlags & 1) != 0) {
            param0 = camera->hWork;
        }
    }
    if (param0->bIsActive != enabled) {
        hCineInfoTest->bIsActive = enabled;
        if (enabled == 0) {
            hCineInfoTest->eTypeOfViewport = CAM_e_NoViewport;
        }
        if (mode != 1) {
            if (enabled == 0) {
                camera->hVisibility->eTypeOfViewport = CAM_e_NoViewport;
                camera->hWork->eTypeOfViewport = CAM_e_NoViewport;
            }
            camera->hVisibility->bIsActive = enabled;
            camera->hWork->bIsActive = enabled;
        }
    }
}

void CAM_fn_vChangeAVectorParameter(tdstCineinfo_* _hCineInfo, MTH3D_tdstVector_* _p_stInitialValue,
                                    MTH3D_tdstVector_* _p_stCurrentValue, MTH3D_tdstVector_* _p_stVisibilityValue,
                                    MTH3D_tdstVector_* _p_stWorkValue, s32 _lTypeOfChange, s32 _lChangingMode,
                                    MTH3D_tdstVector_* _p_stValue)
{
    MTH3D_tdstVector_ stNewValue;
    f32 zero;

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&stNewValue, zero, zero, zero);
    if (_lTypeOfChange == 1) {
        CAM_fn_vSetCineinfoWorkFromCurrent(_hCineInfo);
        _p_stCurrentValue = _p_stWorkValue;
    }
    switch (_lChangingMode) {
    case -1:
        __asm__ volatile("");
        stNewValue = *(MTH3D_tdstVector_*) _p_stInitialValue;
        break;
    case 0:
        if ((_hCineInfo->ucVolIAFlags & 1) != 0) {
            MTH3D_M_vAddXYZ_inv(&stNewValue, _p_stWorkValue, _p_stValue);
        } else {
            MTH3D_M_vAddXYZ_inv(&stNewValue, _p_stCurrentValue, _p_stValue);
        }
        break;
    case 1:
        stNewValue = *(MTH3D_tdstVector_*) _p_stValue;
        break;
    }
    *(MTH3D_tdstVector_*) _p_stCurrentValue = stNewValue;
    if (_lTypeOfChange != 1) {
        *(MTH3D_tdstVector_*) _p_stVisibilityValue = stNewValue;
        *(MTH3D_tdstVector_*) _p_stWorkValue = stNewValue;
    }
}

void* CAM_fn_p_stChangeVectorParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    HIE_tdstSuperObject_* ultra;
    HIE_tdstSuperObject_** p_hUltraPerso;
    tdstCineinfo_* cineinfo;
    MTH3D_tdstVector_* pInit;
    MTH3D_tdstVector_* pCurrent;
    MTH3D_tdstVector_* pVisibility;
    MTH3D_tdstVector_* pWork;
    u16 cmd;
    s32 mode2;
    s32 mode1;

    param = inv_AI_parameterPoolCursor;
    p_hUltraPerso = &ultra;
    cmd = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = param + 1;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, p_hUltraPerso);
    cineinfo = inv_HIE_fn_p_stGetEngineObject(ultra)->h_Cineinfo;

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    mode1 = param->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    mode2 = param->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);

    switch (cmd) {
    case eProc_Cam_ChangeShiftTarget:
        pInit = &cineinfo->hInit->stShiftTarget;
        pCurrent = &cineinfo->hCurrent->stShiftTarget;
        pVisibility = &cineinfo->hVisibility->stShiftTarget;
        pWork = &cineinfo->hWork->stShiftTarget;
        break;
    case eProc_Cam_ChangeShiftPos:
        pInit = &cineinfo->hInit->stShiftPos;
        pCurrent = &cineinfo->hCurrent->stShiftPos;
        pVisibility = &cineinfo->hVisibility->stShiftPos;
        pWork = &cineinfo->hWork->stShiftPos;
        break;
    default:
        break;
    }

    CAM_fn_vChangeAVectorParameter(cineinfo, pInit, pCurrent, pVisibility, pWork, mode1, mode2,
                                   &param->u.stVector);
    inv_AI_parameterPoolCursor = param;
    return p_stTree;
}

void* CAM_fn_p_stChangeRealParameter(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    void* p_stNext;
    HIE_tdstSuperObject_* ultra;
    HIE_tdstSuperObject_** p_hUltraPerso;
    f32* pInit;
    f32* pCurrent;
    f32* pVisibility;
    f32* pWork;
    HIE_tdstSuperObject_* perso;
    u16 cmd;
    tdstCineinfo_* cineinfo;
    s32 mode2;
    s32 mode1;
    f32 value;

    param = inv_AI_parameterPoolCursor;
    p_stNext = p_stTree;
    p_hUltraPerso = &ultra;
    pInit = (f32*) 0;
    pCurrent = (f32*) 0;
    pVisibility = (f32*) 0;
    perso = p_SuperObjPerso;
    cmd = AI_NODE_ID(p_stNext);
    inv_AI_parameterPoolCursor = param + 1;
    pWork = (f32*) 0;

    fn_vGetUltraOperatorPerso(perso, p_hUltraPerso);
    cineinfo = inv_HIE_fn_p_stGetEngineObject(ultra)->h_Cineinfo;

    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    mode1 = param->u.lValue;
    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    mode2 = param->u.lValue;
    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    value = param->u.xValue;

    switch (cmd) {
    case eProc_Cam_ChangeDistMin:
        pInit = &cineinfo->hInit->xDistMin;
        pCurrent = &cineinfo->hCurrent->xDistMin;
        pVisibility = &cineinfo->hVisibility->xDistMin;
        pWork = &cineinfo->hWork->xDistMin;
        break;
    case eProc_Cam_ChangeDistMax:
        pInit = &cineinfo->hInit->xDistMax;
        pCurrent = &cineinfo->hCurrent->xDistMax;
        pVisibility = &cineinfo->hVisibility->xDistMax;
        pWork = &cineinfo->hWork->xDistMax;
        break;
    case eProc_Cam_ChangeBoundDistMin:
        pInit = &cineinfo->hInit->xBoundDistMin;
        pCurrent = &cineinfo->hCurrent->xBoundDistMin;
        pVisibility = &cineinfo->hVisibility->xBoundDistMin;
        pWork = &cineinfo->hWork->xBoundDistMin;
        break;
    case eProc_Cam_ChangeBoundDistMax:
        pInit = &cineinfo->hInit->xBoundDistMax;
        pCurrent = &cineinfo->hCurrent->xBoundDistMax;
        pVisibility = &cineinfo->hVisibility->xBoundDistMax;
        pWork = &cineinfo->hWork->xBoundDistMax;
        break;
    case eProc_Cam_ChangeAngleAlpha:
        pInit = &cineinfo->hInit->xAngleAlpha;
        pCurrent = &cineinfo->hCurrent->xAngleAlpha;
        pVisibility = &cineinfo->hVisibility->xAngleAlpha;
        pWork = &cineinfo->hWork->xAngleAlpha;
        break;
    case eProc_Cam_ChangeAngleShiftAlpha:
        pInit = &cineinfo->hInit->xAngleShiftAlpha;
        pCurrent = &cineinfo->hCurrent->xAngleShiftAlpha;
        pVisibility = &cineinfo->hVisibility->xAngleShiftAlpha;
        pWork = &cineinfo->hWork->xAngleShiftAlpha;
        break;
    case eProc_Cam_ChangeAngleTheta:
        pInit = &cineinfo->hInit->xAngleTheta;
        pCurrent = &cineinfo->hCurrent->xAngleTheta;
        pVisibility = &cineinfo->hVisibility->xAngleTheta;
        pWork = &cineinfo->hWork->xAngleTheta;
        break;
    case eProc_Cam_ChangeAngleShiftTheta:
        pInit = &cineinfo->hInit->xAngleShiftTheta;
        pCurrent = &cineinfo->hCurrent->xAngleShiftTheta;
        pVisibility = &cineinfo->hVisibility->xAngleShiftTheta;
        pWork = &cineinfo->hWork->xAngleShiftTheta;
        break;
    case eProc_Cam_ChangeLinearSpeed:
        pInit = &cineinfo->hInit->xLinearSpeed;
        pCurrent = &cineinfo->hCurrent->xLinearSpeed;
        pVisibility = &cineinfo->hVisibility->xLinearSpeed;
        pWork = &cineinfo->hWork->xLinearSpeed;
        break;
    case eProc_Cam_ChangeLinearIncreaseSpeed:
        pInit = &cineinfo->hInit->xLinearIncreaseSpeed;
        pCurrent = &cineinfo->hCurrent->xLinearIncreaseSpeed;
        pVisibility = &cineinfo->hVisibility->xLinearIncreaseSpeed;
        pWork = &cineinfo->hWork->xLinearIncreaseSpeed;
        break;
    case eProc_Cam_ChangeLinearDecreaseSpeed:
        pInit = &cineinfo->hInit->xLinearDecreaseSpeed;
        pCurrent = &cineinfo->hCurrent->xLinearDecreaseSpeed;
        pVisibility = &cineinfo->hVisibility->xLinearDecreaseSpeed;
        pWork = &cineinfo->hWork->xLinearDecreaseSpeed;
        break;
    case eProc_Cam_ChangeAngularSpeed:
        pInit = &cineinfo->hInit->xAngularSpeed;
        pCurrent = &cineinfo->hCurrent->xAngularSpeed;
        pVisibility = &cineinfo->hVisibility->xAngularSpeed;
        pWork = &cineinfo->hWork->xAngularSpeed;
        break;
    case eProc_Cam_ChangeAngularIncreaseSpeed:
        pInit = &cineinfo->hInit->xAngularIncreaseSpeed;
        pCurrent = &cineinfo->hCurrent->xAngularIncreaseSpeed;
        pVisibility = &cineinfo->hVisibility->xAngularIncreaseSpeed;
        pWork = &cineinfo->hWork->xAngularIncreaseSpeed;
        break;
    case eProc_Cam_ChangeAngularDecreaseSpeed:
        pInit = &cineinfo->hInit->xAngularDecreaseSpeed;
        pCurrent = &cineinfo->hCurrent->xAngularDecreaseSpeed;
        pVisibility = &cineinfo->hVisibility->xAngularDecreaseSpeed;
        pWork = &cineinfo->hWork->xAngularDecreaseSpeed;
        break;
    case eProc_Cam_ChangeTargetSpeed:
        pInit = &cineinfo->hInit->xTargetSpeed;
        pCurrent = &cineinfo->hCurrent->xTargetSpeed;
        pVisibility = &cineinfo->hVisibility->xTargetSpeed;
        pWork = &cineinfo->hWork->xTargetSpeed;
        break;
    case eProc_Cam_ChangeTargetIncreaseSpeed:
        pInit = &cineinfo->hInit->xTargetIncreaseSpeed;
        pCurrent = &cineinfo->hCurrent->xTargetIncreaseSpeed;
        pVisibility = &cineinfo->hVisibility->xTargetIncreaseSpeed;
        pWork = &cineinfo->hWork->xTargetIncreaseSpeed;
        break;
    case eProc_Cam_ChangeTargetDecreaseSpeed:
        pInit = &cineinfo->hInit->xTargetDecreaseSpeed;
        pCurrent = &cineinfo->hCurrent->xTargetDecreaseSpeed;
        pVisibility = &cineinfo->hVisibility->xTargetDecreaseSpeed;
        pWork = &cineinfo->hWork->xTargetDecreaseSpeed;
        break;
    case eProc_Cam_ChangeFocal:
        pInit = &cineinfo->hInit->xFocal;
        pCurrent = &cineinfo->hCurrent->xFocal;
        pVisibility = &cineinfo->hVisibility->xFocal;
        pWork = &cineinfo->hWork->xFocal;
        break;
    case eProc_Cam_ChangeZMin:
        pInit = &cineinfo->hInit->xZMin;
        pCurrent = &cineinfo->hCurrent->xZMin;
        pVisibility = &cineinfo->hVisibility->xZMin;
        pWork = &cineinfo->hWork->xZMin;
        break;
    case eProc_Cam_ChangeZMax:
        pInit = &cineinfo->hInit->xZMax;
        pCurrent = &cineinfo->hCurrent->xZMax;
        pVisibility = &cineinfo->hVisibility->xZMax;
        pWork = &cineinfo->hWork->xZMax;
        break;
    }

    CAM_fn_vChangeARealParameter(value, cineinfo, pInit, pCurrent, pVisibility, pWork, mode1,
                                 mode2);
    inv_AI_parameterPoolCursor = param;
    return p_stNext;
}

void* CAM_fn_p_stChangeTgtPerso(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    HIE_tdstSuperObject_* resolved;
    void* slot;
    void* engine;
    tdstCineinfo_* cineinfo;
    void* p_stNext = p_stTree;
    HIE_tdstSuperObject_* target = p_SuperObjPerso;
    s32 mode;
    s32 selection;
    s32 one;
    void* stream;

    param = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = param + 1;
    fn_vGetUltraOperatorPerso(target, &resolved);
    slot = resolved;
    engine = ((HIE_tdstSuperObject_*) slot)->hLinkedObject;
    cineinfo = ((tdstEngineObject_*) engine)->h_Cineinfo;
    stream = fn_p_stEvalTree(target, p_stNext, param);
    mode = param->u.lValue;
    stream = fn_p_stEvalTree(target, stream, param);
    selection = param->u.lValue;
    stream = fn_p_stEvalTree(target, stream, param);
    p_stNext = stream;
    one = 1;
    target = ((tdstEngineObject_*) param->u.lValue)->h_StandardGame->h_SuperObject;
    if (mode == one) {
        CAM_fn_vSetCineinfoWorkFromCurrent(cineinfo);
        slot = cineinfo->hWork;
    } else {
        slot = cineinfo->hCurrent;
    }
    if (selection != 0) {
        if (selection > 0) {
        } else if (selection == -1) {
            target = cineinfo->hInit->hSuperObjectTargeted;
        }
    } else {
        target = 0;
    }
    ((tdstInternalStructurCineinfo_*) slot)->hSuperObjectTargeted = target;
    if (mode != 1) {
        tdstInternalStructurCineinfo_* w = cineinfo->hVisibility;
        w->hSuperObjectTargeted = target;
        slot = cineinfo->hWork;
        ((tdstInternalStructurCineinfo_*) slot)->hSuperObjectTargeted = target;
    }
    if (inv_CAM_hPreviousTarget == 0 || inv_CAM_hPreviousTarget == target) {
        inv_AI_parameterPoolCursor = param;
    } else {
        CAM_fn_vUpdateTargetPosition(cineinfo);
        inv_AI_parameterPoolCursor = param;
    }
    return p_stNext;
}

void* CAM_fn_p_stChangeSecondTgtPerso(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    void* p_stNext;
    void* perso;
    HIE_tdstSuperObject_* resolved;
    tdstEngineObject_* node;
    tdstCineinfo_* cineinfo;
    tdstInternalStructurCineinfo_* slot;
    tdstInternalStructurCineinfo_* src;
    tdstInternalStructurCineinfo_* dst;
    tdstEngineObject_* obj;
    HIE_tdstSuperObject_* target;
    s32 mode;
    s32 selector;
    f32 value;

    param = (tdstGetSetParam_*) inv_AI_parameterPoolCursor;
    p_stNext = p_stTree;
    perso = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) ((s32) (param + 1));
    fn_vGetUltraOperatorPerso(perso, &resolved);
    node = resolved->hLinkedObject;
    cineinfo = node->h_Cineinfo;

    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    mode = param->u.lValue;
    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    selector = param->u.lValue;
    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    obj = (void*) param->u.lValue;
    target = obj->h_StandardGame->h_SuperObject;
    p_stNext = fn_p_stEvalTree(perso, p_stNext, param);
    value = param->u.xValue;

    if ((cineinfo->ucPerIAFlags & 2) != 0) {
        CAM_fn_vSetCineinfoCurrentFromVisibility(cineinfo);
        if ((cineinfo->ucVolIAFlags & 1) != 0) {
            dst = cineinfo->hWork;
            src = cineinfo->hVisibility;
            *dst = *src;
        }
    }

    if (mode == 1) {
        CAM_fn_vSetCineinfoWorkFromCurrent(cineinfo);
        slot = cineinfo->hWork;
    } else {
        slot = cineinfo->hCurrent;
    }

    if (selector != 0) {
        if (!(0 < selector)) {
            if (selector == -1) {
                target = cineinfo->hInit->hSuperObjectTargeted;
            }
        }
    } else {
        target = (HIE_tdstSuperObject_*) 0;
    }

    slot->hSecondSuperObjectTargeted = target;
    if (mode != 1) {
        cineinfo->hVisibility->hSecondSuperObjectTargeted = target;
        cineinfo->hWork->hSecondSuperObjectTargeted = target;
    }
    cineinfo->xApexRatio = value;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) param;
    return p_stNext;
}

tdstNodeInterpret_* CAM_fn_p_stChangeChannel(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* p_ucParam = inv_AI_parameterPoolCursor;
    HIE_tdstSuperObject_* entry;
    tdstCineinfo_* info;
    tdstInternalStructurCineinfo_* ptr;
    s32 c1, c2;
    s8 value;
    inv_AI_parameterPoolCursor = p_ucParam + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &entry);
    info = inv_HIE_fn_p_stGetEngineObject(entry)->h_Cineinfo;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_ucParam);
    c1 = p_ucParam->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_ucParam);
    c2 = p_ucParam->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_ucParam);
    value = p_ucParam->u.ucValue;
    if (c1 == 1) {
        CAM_fn_vSetCineinfoWorkFromCurrent(info);
        ptr = info->hWork;
    } else
        ptr = info->hCurrent;
    if (c2 != 0) {
        if (c2 > 0)
            goto final_store;
        if (c2 == -1)
            value = info->hInit->cChannel;
    } else
        value = -1;
final_store:;
    ptr->cChannel = value;
    if (c1 != 1) {
        info->hVisibility->cChannel = value;
        info->hWork->cChannel = value;
    }
    inv_AI_parameterPoolCursor = p_ucParam;
    return p_stTree;
}

void* CAM_fn_p_stActivate(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* saved;
    void* stream;
    s32 first;

    stream = p_stTree;
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);

    p_stTree = ((tdstEngineObject_*) saved->hLinkedObject)->h_Cineinfo;

    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, scratch);
    first = scratch->u.lValue;
    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, scratch);

    CAM_fn_vActivateCamera(p_stTree, first, scratch->u.lValue != 0);

    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* CAM_fn_p_stAssociateViewport(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    s32 first;
    s32 second;
    HIE_tdstSuperObject_* left;
    HIE_tdstSuperObject_* right;
    tdstCineinfo_* left_group;
    tdstCineinfo_* right_group;
    tdstInternalStructurCineinfo_* slot3;
    void* result;

    left_group = 0;
    right_group = 0;
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    first = scratch->u.lValue;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, scratch);
    second = scratch->u.lValue;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, scratch);
    left = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
    right = CAM_fn_hGetActiveCamera(first);
    inv_AI_parameterPoolCursor = scratch;
    if (left == right)
        return result;
    if (first >= 3)
        return result;
    if (first == 0)
        return result;

    if (left != 0)
        left_group = ((tdstEngineObject_*) left->hLinkedObject)->h_Cineinfo;
    if (right != 0)
        right_group = ((tdstEngineObject_*) right->hLinkedObject)->h_Cineinfo;

    if (left != 0) {
        CAM_fn_vActivateCamera(left_group, 0, 1);
        left_group->hWork->eTypeOfViewport = (u8) first;
        CAM_fn_vAssignCameraToAViewport(first, left);
        left_group->hCurrent->eTypeOfViewport = (u8) first;
        slot3 = left_group->hVisibility;
        slot3->eTypeOfViewport = (u8) first;
        GAM_fn_vChangeCharacterForMicro(left);
    }
    if (right != 0 && (*(s32*) &g_stEngineStructure.h_StdCamCaracter) != (s32) right) {
        if ((s32) second == 1)
            CAM_fn_vActivateCamera(right_group, 0, 0);
        right_group->hCurrent->eTypeOfViewport = 0;
        right_group->hVisibility->eTypeOfViewport = 0;
        right_group->hWork->eTypeOfViewport = 0;
    }
    return result;
}

#define CAMERA_FLAG_MASK_CASE(opcode, flagMask, field)                                             \
    case opcode:                                                                                   \
        mask = flagMask;                                                                           \
        if (on == -1) {                                                                            \
            int bits;                                                                              \
            bits = set->hInit->field & flagMask;                                                   \
            on = bits != 0;                                                                        \
        }                                                                                          \
        break

void* CAM_fn_p_stSetIAFlags(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* resolved;
    tdstEngineObject_* node;
    tdstCineinfo_* set;
    void* step;
    s32 mode;
    u64 on;
    s32 mask;
    u16 op;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    mask = 0;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &resolved);
    node = ((HIE_tdstSuperObject_*) resolved)->hLinkedObject;
    set = node->h_Cineinfo;
    step = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    mode = scratch->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, step, scratch);
    on = scratch->u.lValue;

    switch (op) {
    case eProc_Cam_ResetIAFlags:
        mask = set->hInit->uwIAFlags;
        break;
    case eProc_Cam_SetFlagNoDynamicTarget:
        mask = 1;
        if (on == -1) {
            s32 bits = set->hInit->uwIAFlags & 1;
            on = (bits == 0) ? (on + 1) : mask;
        }
        break;
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoAverageMoveTgtPerso, 2, uwIAFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoParseCutAngle, 4, uwIAFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoVisibility, 8, uwIAFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoVisibilityWithDynHie, 0x10, uwIAFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoDynChangeTheta, 0x20, uwIAFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoShiftUntilPosReached, 0x40, uwIAFlags);
    case eProc_Cam_SetFlagNoDynSpeed:
        mask = 0x80;
        if (on == -1) {
            unsigned long bits;
            bits = (u8) set->hInit->uwIAFlags;
            on = bits >> 7;
        }
        break;
    default:
        break;
    }

    CAM_fn_vSetResetInternalFlag(set, &set->hCurrent->uwIAFlags,
                                 &set->hVisibility->uwIAFlags,
                                 &set->hWork->uwIAFlags, mode, on, mask);
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* CAM_fn_p_stSetDNMFlags(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* resolved;
    tdstEngineObject_* node;
    tdstCineinfo_* set;
    void* step;
    s32 mode;
    u64 on;
    s32 mask;
    u16 op;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    mask = 0;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &resolved);
    node = ((HIE_tdstSuperObject_*) resolved)->hLinkedObject;
    set = node->h_Cineinfo;
    step = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    mode = scratch->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, step, scratch);
    on = scratch->u.lValue;

    switch (op) {
    case eProc_Cam_ResetDNMFlags:
        mask = set->hInit->uwDNMFlags;
        break;
    case eProc_Cam_SetFlagNoLinearParsing:
        mask = 1;
        if (on == -1) {
            s32 bits = set->hInit->uwDNMFlags & 1;
            on = (bits == 0) ? (on + 1) : mask;
        }
        break;
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoLinearInertia, 2, uwDNMFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoAngularParsing, 4, uwDNMFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoAngularInertia, 8, uwDNMFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoTargetParsing, 0x10, uwDNMFlags);
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoTargetInertia, 0x20, uwDNMFlags);
    case eProc_Cam_SetFlagFixedOrientation:
        mask = 0x80;
        if (on == -1) {
            unsigned long bits;
            bits = (u8) set->hInit->uwDNMFlags;
            on = bits >> 7;
        }
        break;
        CAMERA_FLAG_MASK_CASE(eProc_Cam_SetFlagNoObstacle, 0x40, uwDNMFlags);
    case eProc_Cam_SetFlagNoCollisionWhenNotMoving:
        mask = 0x100;
        if (on == -1) {
            on = ((u8*) &set->hInit->uwDNMFlags)[1] & 1;
        }
        break;
    default:
        break;
    }

    CAM_fn_vSetResetInternalFlag(set, &set->hCurrent->uwDNMFlags,
                                 &set->hVisibility->uwDNMFlags,
                                 &set->hWork->uwDNMFlags, mode, on, mask);
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* CAM_fn_p_stShowInfo(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* saved;
    tdstCineinfo_* target;
    s32 enabled;
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    target = ((tdstEngineObject_*) saved->hLinkedObject)->h_Cineinfo;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    enabled = scratch->u.lValue;
    switch (enabled) {
    case 0:
        target->ucPerIAFlags &= 0xFE;
        break;
    case 1:
        target->ucPerIAFlags |= 1;
        break;
    }
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* CAM_fn_p_stChangeConstants(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* saved;
    s32 id;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    id = scratch->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    {
        f32 value;
        void* obj;
        value = scratch->u.xValue;
        inv_AI_parameterPoolCursor = scratch;

        if (id == 0x27) {
            s32 bits;
            bits = inv_MTH_fn_lToInteger(value);
            obj = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)->h_Cineinfo;
            if (bits & 2) {
                ((tdstCineinfo_*) obj)->ucVolIAFlags &= 0xFE;
                CAM_g_stCameraConstants.wFlags &= (u16) ~bits;
                return p_stTree;
            } else {
                CAM_g_stCameraConstants.wFlags |= (u16) bits;
                return p_stTree;
            }
        }

        if (id < 0x11) {
            (&CAM_g_stCameraConstants.xRayCameraSphereToGo1)[id - 1] = value;
            return p_stTree;
        }

        if (id < 0x17) {
            (&CAM_g_stCameraConstants.xNumRayCameraCanGo)[id - 0x11] =
                (s16) inv_MTH_fn_lToInteger(value);
            return p_stTree;
        }

        if (id < 0x1E) {
            return p_stTree;
        }

        if (id < 0x27) {
            (&CAM_g_stCameraConstants.xDynSpeed_LinearFactNorm)[id - 0x1E] = value;
            return p_stTree;
        }

        if (id == 0x28) {
            CAM_g_stCameraConstants.wFlags &= (u16) ~inv_MTH_fn_lToInteger(value);
            return p_stTree;
        }

        if (id < 0x2C) {
            (&CAM_g_stCameraConstants.xBaseCutAngle)[id - 0x29] = value;
            return p_stTree;
        }

        if (id == 0x2C) {
            obj = ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)->h_Cineinfo;
            ((tdstCineinfo_*) obj)->bCanDoBestPos = (value != 0.0f);
        }

        return p_stTree;
    }
}

void* CAM_fn_p_stConstants(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    switch (AI_NODE_ID(p_stTree)) {
    case eProc_Cam_SaveConstants:
        memcpy(CAM_g_stCopyCameraConstants, &CAM_g_stCameraConstants, 0x80);
        break;
    case eProc_Cam_RestoreConstants:
        memcpy(&CAM_g_stCameraConstants, CAM_g_stCopyCameraConstants, 0x80);
        break;
    }
    return p_stTree;
}

void* CAM_fn_p_stSetForce(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstCineinfo_* cineinfo;
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* hPerso;
    s32 enabled;
    s32 mode;
    u16 op;
    u32 mask;
    MTH3D_tdstVector_ stForce;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;
    mask = 0;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    cineinfo = ((tdstEngineObject_*) hPerso->hLinkedObject)->h_Cineinfo;

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    mode = scratch->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    enabled = scratch->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);

    switch (op) {
    case eProc_Cam_ForceTarget:
        mask = 0x1000;
        if (enabled == 1) {
            cineinfo->stForceTarget = scratch->u.stVector;
        }
        break;
    case eProc_Cam_ForcePosition:
        mask = 0x2000;
        if (enabled == 1) {
            cineinfo->stForcePosition = scratch->u.stVector;
        }
        break;
    case eProc_Cam_ForceRefAxis:
        stForce = scratch->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        if (enabled == 1) {
            cineinfo->stForceRefAxisY = stForce;
            cineinfo->stForceRefAxisZ = scratch->u.stVector;
        }
        mask = 0x4000;
        break;
    }

    CAM_fn_vSetResetInternalFlag(cineinfo, &cineinfo->hCurrent->uwIAFlags,
                                 &cineinfo->hVisibility->uwIAFlags,
                                 &cineinfo->hWork->uwIAFlags, mode,
                                 enabled, mask);

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* CAM_fn_p_stReset(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    void* stream;
    HIE_tdstSuperObject_* hPerso;
    HIE_tdstSuperObject_* p_stPerso;
    tdstEngineObject_* p_stEngineObject;

    stream = p_stTree;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    p_stPerso = hPerso;
    p_stEngineObject = p_stPerso->hLinkedObject;
    CAM_fn_vInitCompleteCineinfo(p_stEngineObject->h_Cineinfo);
    return stream;
}

tdstNodeInterpret_* CAM_fn_p_stForceBestPos(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    CAM_tdstUpdateCamera_ st;
    HIE_tdstSuperObject_* node;
    tdstCineinfo_* obj;
    tdstInternalStructurCineinfo_* p;
    s32 one;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    obj = ((tdstEngineObject_*) node->hLinkedObject)->h_Cineinfo;
    if (obj->bCanDoBestPos == 0) {
        *(MTH3D_tdstVector_*) &obj->stForcePosition =
            *(MTH3D_tdstVector_*) &node->gu_hGlobalMatrix->stMatrix.stCol_3;
        CAM_fn_vSetResetInternalFlag(obj, &obj->hCurrent->uwIAFlags,
                                     &obj->hVisibility->uwIAFlags, &obj->hWork->uwIAFlags,
                                     1, 1, 0x2000);
        return p_stTree;
    }
    CAM_fn_vSetCineinfoWorkFromCurrent(obj);
    CAM_fn_vInitCameraStructure(node, &st);
    p = st.hCineinfoWork;
    cRefAxisIsAlreadyComputed = (p->uwIAFlags >> 14) & 1;
    cNoDynChangeTheta = (p->uwIAFlags >> 5) & 1;
    one = 1;
    cCanTestStatic = one;
    CAM_fn_vComputeReferencePoint(&st);
    CAM_fn_vComputeTarget(&st);
    CAM_fn_vComputeRealWantedPos(&st);
    CAM_fn_vComputeMovePosWithDynTheta(&st);
    if (st.hCineinfoCurrent->hSuperObjectTargeted == NULL) {
        return p_stTree;
    }
    CAM_fn_vForceFailureVisibility(&st);
    CAM_fn_vComputeFailureVisibility(&st);
    st.hCineinfoWork->uwDNMFlags |= 5;
    obj->bCanDoBestPos = 0;
    return p_stTree;
}

void* CAM_fn_p_stForceNormalState(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hPerso;
    tdstCineinfo_* node;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    node = inv_HIE_fn_p_stGetEngineObject(hPerso)->h_Cineinfo;
    if (node->ucPerIAFlags & 2) {
        CAM_fn_vSetCineinfoCurrentFromVisibility(node);
    }
    node->ucPerIAFlags = node->ucPerIAFlags & 0xF7;
    node->eState = 0;
    return p_stTree;
}

void* CAM_fn_p_stForceMovingOnRail(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    CAM_tdstUpdateCamera_ stStruct;
    MTH3D_tdstVector_ stWpPosition;
    MTH3D_tdstVector_ stPreviousWpPosition;
    MTH3D_tdstVector_ stOldPreviousWpPosition;
    MTH3D_tdstVector_ stNextWpPosition;
    MTH3D_tdstVector_ stWantedPosition;
    MTH3D_tdstVector_* pvec;
    HIE_tdstSuperObject_* resolved;
    tdstEngineObject_* p_stEngineObject;
    void* h_MSWay;
    tdstCineinfo_* hCineinfo;
    tdstInternalStructurCineinfo_* hWork;
    HIE_tdstSuperObject_* hTarget;
    MTH3D_tdstVector_* base;
    f32 xDistMin, xDistMax, dx, dy, dz, len2, dist, tmp, d21, d20, d4, radius;
    s32 same = 0, done = 0, idx;
    void *obj, *prev, *o2, *o3;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &resolved);
    radius = 0.0f;
    p_stEngineObject = ((HIE_tdstSuperObject_*) resolved)->hLinkedObject;
    h_MSWay = p_stEngineObject->h_MSWay;
    hCineinfo = p_stEngineObject->h_Cineinfo;
    if (h_MSWay == 0) {
        return p_stTree;
    }
    CAM_fn_vSetCineinfoWorkFromCurrent(hCineinfo);
    CAM_fn_vInitCameraStructure(resolved, &stStruct);
    hWork = hCineinfo->hWork;
    xDistMin = hWork->xDistMin;
    xDistMax = hWork->xDistMax;
    if (xDistMin == xDistMax) {
        same = 1;
    }
    hTarget = hWork->hSuperObjectTargeted;
    base = inv_POS_fn_p_stPosMatrixTranslation(hTarget->gu_hLocalMatrix);
    dx = stStruct.stCurrentCameraPos.xX - base->xX;
    dy = stStruct.stCurrentCameraPos.xY - base->xY;
    dz = stStruct.stCurrentCameraPos.xZ - base->xZ;
    len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    dist = MTH_fn_xSqrtOpt2(len2);

    if (same != 0) {
        dist = inv_MTH_fn_xAbsoluteTied(xDistMin - dist);

        if (dist < *(volatile f32*) &D_00392FE8) {
            stWantedPosition = stStruct.stCurrentCameraPos;
            done = 1;
        } else {
            radius = xDistMin;
        }
    } else {
        if (dist < xDistMin) {
            radius = xDistMin;
            goto l_merge;
        }
        if (xDistMax < dist) {
            radius = xDistMax;
            goto l_merge;
        }

        radius = xDistMax;
        stWantedPosition = stStruct.stCurrentCameraPos;
        done = 1;
    }

l_merge:
    pvec = &stStruct.stCurrentCameraPos;
    stWantedPosition = stStruct.stCurrentCameraPos;

    if (done == 0) {
        CAM_fn_vComputeReferencePoint(&stStruct);
        idx = MSWay_fn_lGetCurrentIndexInPath(h_MSWay);
        obj = MSWay_fn_hGetWayPointOfPath(h_MSWay, idx);
        MSWay_fn_lGetNumberWayPointInPath(h_MSWay);
        WP_fnv_WayPoint_GetVertex(obj, &stWpPosition);
        if (CAM_fn_bEqualVectorWithEpsilon(&stWpPosition, pvec, WP_fnx_WayPoint_GetRadius(obj)) != 0) {
            prev = MSWay_fn_hGetWayPointOfPath(h_MSWay, idx - 1);
            if (prev != 0) {
                WP_fnv_WayPoint_GetVertex(prev, &stOldPreviousWpPosition);
            } else {
                stOldPreviousWpPosition = stWpPosition;
            }
            obj = MSWay_fn_hGetWayPointOfPath(h_MSWay, idx + 1);
            stPreviousWpPosition = stWpPosition;
            if (obj != 0) {
                WP_fnv_WayPoint_GetVertex(obj, &stWpPosition);
            }
            d21 = Cam_fn_xComputeProjectionRail(&stStruct.stTargetedPersoPos, &stPreviousWpPosition, &stOldPreviousWpPosition);
            d20 = Cam_fn_xComputeProjectionRail(&stStruct.stTargetedPersoPos, &stPreviousWpPosition, &stWpPosition);
            if (d21 <= d20) {
                inv_CAM_lPathDirection = -1;
                MSWay_fn_lIncrementCurrentIndexInPath(h_MSWay, -1);
                Cam_fn_vComputeMovingOnRail(&stStruct, &stWantedPosition, &stPreviousWpPosition, &stOldPreviousWpPosition, radius);
                goto l_end;
            } else {
                inv_CAM_lPathDirection = 1;
                MSWay_fn_lIncrementCurrentIndexInPath(h_MSWay, 1);
                Cam_fn_vComputeMovingOnRail(&stStruct, &stWantedPosition, &stWpPosition, &stPreviousWpPosition, radius);
                goto l_end;
            }
        } else {
            prev = MSWay_fn_hGetWayPointOfPath(h_MSWay, idx - inv_CAM_lPathDirection);
            if (prev != 0) {
                WP_fnv_WayPoint_GetVertex(prev, &stPreviousWpPosition);
            } else {
                stPreviousWpPosition = stWpPosition;
                prev = obj;
            }
            d21 = Cam_fn_xComputeProjectionRail(&stStruct.stTargetedPersoPos, &stPreviousWpPosition, &stWpPosition);
            o2 = MSWay_fn_hGetWayPointOfPath(h_MSWay, idx - 2 * inv_CAM_lPathDirection);
            if (o2 != 0) {
                WP_fnv_WayPoint_GetVertex(o2, &stOldPreviousWpPosition);
            } else {
                stOldPreviousWpPosition = stPreviousWpPosition;
            }
            d20 = Cam_fn_xComputeProjectionRail(&stStruct.stTargetedPersoPos, &stPreviousWpPosition, &stOldPreviousWpPosition);
            o3 = MSWay_fn_hGetWayPointOfPath(h_MSWay, idx + inv_CAM_lPathDirection);
            if (o3 != 0) {
                WP_fnv_WayPoint_GetVertex(o3, &stNextWpPosition);
            } else {
                stNextWpPosition = stWpPosition;
            }
            d4 = Cam_fn_xComputeProjectionRail(&stStruct.stTargetedPersoPos, &stNextWpPosition, &stWpPosition);
            if (d4 <= d21) {
                if (d4 <= d20) {
                    stWantedPosition = stWpPosition;
                    goto l_end;
                }
                if (d21 <= d20) {
                    Cam_fn_vComputeMovingOnRail(&stStruct, &stWantedPosition, &stWpPosition, &stPreviousWpPosition, radius);
                    goto l_end;
                } else {
                    tmp = WP_fnx_WayPoint_GetRadius(prev);
                    if (CAM_fn_bEqualVectorWithEpsilon(&stPreviousWpPosition, pvec, tmp) == 0)
                        goto l_res_tE0;
                    Cam_fn_vComputeMovingOnRail(&stStruct, &stWantedPosition, &stPreviousWpPosition, &stOldPreviousWpPosition, radius);
                    inv_CAM_lPathDirection = -inv_CAM_lPathDirection;
                    MSWay_fn_lIncrementCurrentIndexInPath(h_MSWay, inv_CAM_lPathDirection);
                    goto l_end;
                }
            } else if (d21 <= d20) {
                Cam_fn_vComputeMovingOnRail(&stStruct, &stWantedPosition, &stWpPosition, &stPreviousWpPosition, radius);
                goto l_end;
            } else {
                tmp = WP_fnx_WayPoint_GetRadius(prev);
                if (CAM_fn_bEqualVectorWithEpsilon(&stPreviousWpPosition, pvec, tmp) == 0)
                    goto l_res_tE0;
                inv_CAM_lPathDirection = -inv_CAM_lPathDirection;
                MSWay_fn_lIncrementCurrentIndexInPath(h_MSWay, inv_CAM_lPathDirection);
                Cam_fn_vComputeMovingOnRail(&stStruct, &stWantedPosition, &stPreviousWpPosition, &stOldPreviousWpPosition, radius);
                goto l_end;
            }
        }
    } else
        goto l_end;

l_res_tE0:
    stWantedPosition = stPreviousWpPosition;

l_end:
    hCineinfo->stForcePosition = stWantedPosition;
    CAM_fn_vSetResetInternalFlag(hCineinfo, &hCineinfo->hCurrent->uwIAFlags, &hCineinfo->hVisibility->uwIAFlags,
                                 &hCineinfo->hWork->uwIAFlags, 1, 1, 0x2000);
    return p_stTree;
}

void* CAM_fn_p_stCameraMode(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* p_stNext;
    u8* p_cCameraMode;

    scratch = inv_AI_parameterPoolCursor;
    p_stNext = scratch + 1;
    inv_AI_parameterPoolCursor = p_stNext;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    p_SuperObjPerso = scratch->u.ucValue;
    p_cCameraMode = ((u8*) &g_stEngineStructure.cCameraMode);
    inv_AI_parameterPoolCursor = scratch;
    *p_cCameraMode = (u8) p_SuperObjPerso;
    return p_stNext;
}
