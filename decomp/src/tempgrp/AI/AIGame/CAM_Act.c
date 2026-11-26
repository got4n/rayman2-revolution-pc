#include "common.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "AI/AI.h"
#include "GAM/GAM.h"
#include "CAM/CAM.h"
#include "MEC/MEC.h"

void fnv_MoveAToB(HIE_tdstSuperObject_* superObject, MTH3D_tdstVector_* start,
                   MTH3D_tdstVector_* end, MTH3D_tdstVector_* current, s32 duration, s32 ramp)
{
    MTH3D_tdstVector_ direction;
    f32 dx, dy, dz;
    f32 startX, startY, startZ;
    f32 length;
    f32 speed;
    f32 totalTime;
    s32 elapsed;

    totalTime = duration;

    startX = start->xX;
    startY = start->xY;
    startZ = start->xZ;
    dx = end->xX - startX;
    dy = end->xY - startY;
    dz = end->xZ - startZ;
    direction.xX = current->xX - startX;
    dx = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
    direction.xY = current->xY - startY;
    dx = MTH_fn_xSqrtOpt2(dx);
    speed = dx / totalTime;
    direction.xZ = current->xZ - startZ;

    elapsed = inv_fn_ulGetEngineClock() - fn_lGetActionParamlValue(5, superObject);
    if (elapsed < ramp)
        speed = (speed / (f32) ramp) * (f32) elapsed;
    if (duration - ramp < elapsed) {
        f32 rampTime = ramp;

        speed = (f32) elapsed * -(speed / rampTime) + (totalTime * speed) / rampTime;
    }

    inv_MTH3D_fn_vSubXYZ(&direction, end, start);
    length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(direction.xX, direction.xY, direction.xZ));
    if (length != 0.0f) {
        f32 inverse = 1.0f / length;

        inv_MTH3D_fn_vKTimesXYZ(&direction, inverse, &direction);
    }

    speed *= inv_fn_xGetEngineClockDelta();
    inv_MTH3D_fn_vKTimesXYZ(&direction, speed, &direction);
    inv_MTH3D_fn_vAddXYZ(end, &direction, current);
}

#define CUSTOM_BIT_CINE_CAMERA 0x8000
#define CINE_RUNNING 1
#define CINE_FINISHED 4
#define PREVIOUS_POSITION_SLOT(offset) ((MTH3D_tdstVector_*) ((u8*) __builtin_alloca(0) + (offset)))

static inline void inv_AI_fn_vEndCineCamera(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* engine = superObject->hLinkedObject;

    engine->h_StandardGame->ulCustomBits &= ~CUSTOM_BIT_CINE_CAMERA;
}

void fnv_TurnAWithB(HIE_tdstSuperObject_* p_SuperObjPerso, MTH3D_tdstVector_* _p_stSource,
                    MTH3D_tdstVector_* _p_stPointAxis, MTH3D_tdstVector_* _p_stAxis, MTH3D_tdstVector_* _p_stCurrent,
                    f32 _xAngle, s32 _iTime, s32 _iAccel)
{
    MTH3D_tdstVector_ projected;
    MTH3D_tdstVector_ start;
    MTH_stRotation rotation;
    s32 elapsed;

    inv_MTH3D_fn_vSubXYZ(&projected, _p_stCurrent, _p_stPointAxis);
    CAM_fn_vComputeProjection(&projected, &projected, _p_stAxis);
    _xAngle = _xAngle / (f32) _iTime;

    inv_MTH3D_fn_vSubXYZ(&start, _p_stSource, _p_stPointAxis);
    CAM_fn_vComputeProjection(&start, &start, _p_stAxis);

    CAM_fn_xComputeRotation(&start, &projected, NULL);

    elapsed = inv_fn_ulGetEngineClock() - fn_lGetActionParamlValue(5, p_SuperObjPerso);
    if (elapsed < _iAccel) {
        _xAngle = (_xAngle / (f32) _iAccel) * (f32) elapsed;
    }
    if (_iTime - _iAccel < elapsed) {
        f32 rampTime = (f32) _iAccel;
        _xAngle = (f32) elapsed * -(_xAngle / rampTime) + ((f32) _iTime * _xAngle) / rampTime;
    }

    rotation.m_stAxis = *_p_stAxis;
    _xAngle *= inv_fn_xGetEngineClockDelta();
    rotation.m_xAngle = _xAngle;

    inv_MTH3D_fn_vSubXYZ(&projected, _p_stCurrent, _p_stPointAxis);
    MTH_p_stRotationMulVector(&rotation, &projected, _p_stSource);

    MTH3D_M_vAddXYZ_inv(_p_stSource, _p_stSource, _p_stPointAxis);
}

static inline s32 inv_AI_fn_lLongValueOf(tdstGetSetParam_* param)
{
    s32 value;

    if (param->ucType == E_vt_Float) {
        value = inv_MTH_fn_lToIntegerOrdered(param->u.xValue);
    } else {
        value = param->u.lValue;
    }
    return value;
}

tdstNodeInterpret_* fn_p_stCamCineInit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    tdstEngineObject_* engine;
    MTH3D_tdstVector_ up;
    u16 op;

    param = inv_AI_parameterPoolCursor;
    op = p_stTree[-1].param;
    inv_AI_parameterPoolCursor = param + 1;
    engine = p_SuperObjPerso->hLinkedObject;
    engine->h_StandardGame->ulCustomBits |= CUSTOM_BIT_CINE_CAMERA;
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);

    switch (op) {
    case eMetaAction_CamCineMoveAToBTgtC:
    case eMetaAction_CamCineMoveAToBTgtAC:
    case eMetaAction_CamCinePosAMoveTgtBToC:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(0, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(1, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(2, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamlValue(3, p_SuperObjPerso, inv_AI_fn_lLongValueOf(param));
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamlValue(4, p_SuperObjPerso, inv_AI_fn_lLongValueOf(param));
        fn_vSetActionParamlValue(5, p_SuperObjPerso, inv_fn_ulGetEngineClock());
        CAM_fn_vForceCameraPos(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso));
        if (op == eMetaAction_CamCinePosAMoveTgtBToC) {
            CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso), &up);
            fn_vSetActionParamp_stVertex(0, p_SuperObjPerso,
                                         fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso));
        } else {
            CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso), &up);
        }
        break;

    case eMetaAction_CamCinePosATgtB:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(0, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(1, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamlValue(2, p_SuperObjPerso, inv_AI_fn_lLongValueOf(param));
        fn_vSetActionParamlValue(3, p_SuperObjPerso, inv_fn_ulGetEngineClock());
        CAM_fn_vForceCameraPos(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso));
        CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso), &up);
        break;

    case eMetaAction_CamCinePosATgtBTurnPosH:
    case eMetaAction_CamCinePosATgtBTurnTgtH:
    case eMetaAction_CamCinePosATgtBTurnPosV:
    case eMetaAction_CamCinePosATgtBTurnTgtV:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(0, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamp_stVertex(1, p_SuperObjPerso, &param->u.stVector);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamxValue(2, p_SuperObjPerso, param->u.xValue);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamlValue(3, p_SuperObjPerso, inv_AI_fn_lLongValueOf(param));
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        fn_vSetActionParamlValue(4, p_SuperObjPerso, inv_AI_fn_lLongValueOf(param));
        fn_vSetActionParamlValue(5, p_SuperObjPerso, inv_fn_ulGetEngineClock());
        CAM_fn_vForceCameraPos(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso));
        CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso), &up);
        if (op == eMetaAction_CamCinePosATgtBTurnTgtH || op == eMetaAction_CamCinePosATgtBTurnTgtV) {
            fn_vSetActionParamp_stVertex(0, p_SuperObjPerso,
                                         fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso));
        }
        break;
    }

    inv_AI_parameterPoolCursor = param;
    return p_stTree;
}

s32 fn_ucCamCineMoveAToBTgtC(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ previous;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* start;
    MTH3D_tdstVector_* current;
    u32 now;
    s32 deadline;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= (u32) deadline) {
        CAM_fn_vForceCameraPos(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso));
        MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
        CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso), &up);
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    matrix = p_SuperObjPerso->gu_hGlobalMatrix;
    previous = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    up = *fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    start = fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    current = PREVIOUS_POSITION_SLOT(16);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_MoveAToB(p_SuperObjPerso, start, &up, current, duration, ramp);
    CAM_fn_vForceCameraPos(p_SuperObjPerso, &up);
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso), &up);
    return CINE_RUNNING;
}

s32 fn_ucCamCineMoveAToBTgtAC(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ previous;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* start;
    MTH3D_tdstVector_* current;
    u32 now;
    s32 deadline;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= (u32) deadline) {
        CAM_fn_vForceCameraPos(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso));

        MTH3D_M_vSetXYZ_inv(&target,
                            fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso)->xX -
                                fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso)->xX,
                            fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso)->xY -
                                fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso)->xY,
                            fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso)->xZ -
                                fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso)->xZ);

        target.xX += fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso)->xX;
        target.xY += fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso)->xY;
        target.xZ += fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso)->xZ;

        MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
        CAM_fn_vForceCameraTgt(p_SuperObjPerso, &target, &up);
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    matrix = p_SuperObjPerso->gu_hGlobalMatrix;
    previous = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    up = *fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    start = fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    current = PREVIOUS_POSITION_SLOT(32);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_MoveAToB(p_SuperObjPerso, start, &up, current, duration, ramp);
    CAM_fn_vForceCameraPos(p_SuperObjPerso, &up);

    MTH3D_M_vSetXYZ_inv(&target,
                        fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso)->xX -
                            fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso)->xX,
                        fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso)->xY -
                            fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso)->xY,
                        fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso)->xZ -
                            fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso)->xZ);

    MTH3D_M_vAddXYZ_inv(&target, &target, &up);
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, &target, &up);
    return CINE_RUNNING;
}

s32 fn_ucCamCinePosATgtB(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    u32 now;
    s32 deadline;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(2, p_SuperObjPerso);
    if (now < (u32) deadline) {
        return CINE_RUNNING;
    }
    inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
    return CINE_FINISHED;
}

s32 fn_ucCamCinePosAMoveTgtBToC(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_* vertex;
    u32 now;
    u32 deadline;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= deadline) {
        MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
        vertex = fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso);
        CAM_fn_vForceCameraTgt(p_SuperObjPerso, vertex, &up);
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    position = *fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    target = *fn_p_stGetActionParamp_stVertex(2, p_SuperObjPerso);
    vertex = fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_MoveAToB(p_SuperObjPerso, vertex, &target, &position, duration, ramp);
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, &target, &up);
    fn_vSetActionParamp_stVertex(0, p_SuperObjPerso, &target);
    return CINE_RUNNING;
}

s32 fn_ucCamCinePosATgtBTurnPosH(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ previous;
    MTH3D_tdstVector_ axis;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* center;
    MTH3D_tdstVector_* current;
    u32 now;
    s32 deadline;
    f32 angle;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= (u32) deadline) {
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    matrix = p_SuperObjPerso->gu_hGlobalMatrix;
    previous = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    POS_fn_vGetRotationVectors(matrix, &position, &position, &axis);
    position = *fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    center = fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    current = PREVIOUS_POSITION_SLOT(16);
    angle = fn_xGetActionParamxValue(2, p_SuperObjPerso);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_TurnAWithB(p_SuperObjPerso, &position, center, &axis, current, angle, duration, ramp);
    CAM_fn_vForceCameraPos(p_SuperObjPerso, &position);
    MTH3D_M_vSetXYZ_inv(&position, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso), &position);
    return CINE_RUNNING;
}

s32 fn_ucCamCinePosATgtBTurnPosV(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ previous;
    MTH3D_tdstVector_ axis;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* center;
    MTH3D_tdstVector_* current;
    u32 now;
    s32 deadline;
    f32 angle;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= (u32) deadline) {
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    matrix = p_SuperObjPerso->gu_hGlobalMatrix;
    previous = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    POS_fn_vGetRotationVectors(matrix, &axis, &position, &position);
    position = *fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    center = fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    current = PREVIOUS_POSITION_SLOT(16);
    angle = fn_xGetActionParamxValue(2, p_SuperObjPerso);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_TurnAWithB(p_SuperObjPerso, &position, center, &axis, current, angle, duration, ramp);
    CAM_fn_vForceCameraPos(p_SuperObjPerso, &position);
    MTH3D_M_vSetXYZ_inv(&position, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso), &position);
    return CINE_RUNNING;
}

s32 fn_ucCamCinePosATgtBTurnTgtH(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ previous;
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ axis;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* position;
    u32 now;
    s32 deadline;
    f32 angle;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= (u32) deadline) {
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    matrix = p_SuperObjPerso->gu_hGlobalMatrix;
    previous = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    POS_fn_vGetRotationVectors(matrix, &target, &target, &axis);
    target = *fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    position = fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    angle = fn_xGetActionParamxValue(2, p_SuperObjPerso);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_TurnAWithB(p_SuperObjPerso, &target, &previous, &axis, position, angle, duration, ramp);
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, &target, &up);
    fn_vSetActionParamp_stVertex(0, p_SuperObjPerso, &target);
    return CINE_RUNNING;
}

s32 fn_ucCamCinePosATgtBTurnTgtV(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    MTH3D_tdstVector_ previous;
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ up;
    MTH3D_tdstVector_ axis;
    POS_tdstMatrix_* matrix;
    MTH3D_tdstVector_* position;
    u32 now;
    s32 deadline;
    f32 angle;
    s32 duration;
    s32 ramp;

    now = inv_fn_ulGetEngineClock();
    deadline = fn_lGetActionParamlValue(5, p_SuperObjPerso);
    deadline += fn_lGetActionParamlValue(3, p_SuperObjPerso);
    if (now >= (u32) deadline) {
        inv_AI_fn_vEndCineCamera(p_SuperObjPerso);
        return CINE_FINISHED;
    }

    matrix = p_SuperObjPerso->gu_hGlobalMatrix;
    previous = *inv_POS_fn_p_stPosMatrixTranslation(matrix);
    POS_fn_vGetRotationVectors(matrix, &axis, &target, &target);
    target = *fn_p_stGetActionParamp_stVertex(1, p_SuperObjPerso);
    position = fn_p_stGetActionParamp_stVertex(0, p_SuperObjPerso);
    angle = fn_xGetActionParamxValue(2, p_SuperObjPerso);
    duration = fn_lGetActionParamlValue(3, p_SuperObjPerso);
    ramp = fn_lGetActionParamlValue(4, p_SuperObjPerso);
    fnv_TurnAWithB(p_SuperObjPerso, &target, &previous, &axis, position, angle, duration, ramp);
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    CAM_fn_vForceCameraTgt(p_SuperObjPerso, &target, &up);
    fn_vSetActionParamp_stVertex(0, p_SuperObjPerso, &target);
    return CINE_RUNNING;
}
