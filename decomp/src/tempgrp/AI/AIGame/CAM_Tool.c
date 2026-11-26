#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#define DYNAMICS_COMPLETE 0x4

typedef struct CameraParameters {
    f32 distMin;
    MTH3D_tdstVector_ target;
    MTH3D_tdstVector_ movePos;
    f32 targetSpeed;
    f32 angleOrientationY;
    f32 angleOrientationZ;
    u8 unk28[0x10];
    f32 linearSpeed;
    f32 boundDistMax;
    f32 angleAlpha;
    u8 unk44[4];
    f32 angularSpeed;
    f32 angleTeta;
    f32 angleShiftTeta;
    u8 unk54[2];
    u16 dnmFlags;
    MTH3D_tdstVector_ targetedPersoPos;
    f32 xCutAngleFactorPos;
    MTH3D_tdstVector_ refAxisZ;
    f32 xCutAngleFactorTgt;
} CameraParameters;

typedef struct CameraAxis {
    MTH3D_tdstVector_ value;
    f32 unused;
} CameraAxis;

void func_001E3F08(tdstInternalStructurCineinfo_* slot, f32* horizontal, f32* vertical);
float acosf(float);

static inline tdstDynam_* inv_AI_fn_p_stComponentOf(HIE_tdstSuperObject_* object)
{
    return inv_HIE_fn_p_stGetEngineObject(object)->h_Dynam;
}

void CAM_fn_vSetResetInternalFlag(tdstCineinfo_* _hCineInfo, u16* _p_uwCurrentValue, u16* _p_uwVisibilityValue, u16* _p_uwWorkValue, s32 _lTypeOfChange, long _lChangingMode, u16 _uwFlag)
{
    u16 bits = _uwFlag;

    if (_lTypeOfChange == 1) {
        CAM_fn_vSetCineinfoWorkFromCurrent(_hCineInfo);
        if (_lChangingMode != 0) {
            *_p_uwWorkValue |= bits;
            return;
        }
        *_p_uwWorkValue &= ~bits;
        return;
    }
    if (_lChangingMode != 0) {
        *_p_uwCurrentValue |= bits;
        *_p_uwVisibilityValue |= bits;
        *_p_uwWorkValue |= bits;
        return;
    }
    *_p_uwCurrentValue &= ~bits;
    *_p_uwVisibilityValue &= ~bits;
    *_p_uwWorkValue &= ~bits;
}

void CAM_fn_vForceCameraPos(HIE_tdstSuperObject_* _hSuperObjCamera, MTH3D_tdstVector_* _p_stPosition)
{
    DNM_stReport_* report = fn_pstGetDNMReport(_hSuperObjCamera);

    inv_POS_fn_vSetTranslation(_hSuperObjCamera->gu_hGlobalMatrix, _p_stPosition);
    inv_POS_fn_vSetTranslation(_hSuperObjCamera->gu_hLocalMatrix, _p_stPosition);
    report->m_stAbsolutePrevSpeed.m_stLinear.xZ = report->m_stAbsolutePrevSpeed.m_stLinear.xY = report->m_stAbsolutePrevSpeed.m_stLinear.xX = 0.0f;
    fn_vSectInfoInitForSingleCharacter(_hSuperObjCamera, _p_stPosition);
}

void CAM_fn_vForceCameraTgt(HIE_tdstSuperObject_* _hSuperObjCamera, MTH3D_tdstVector_* _p_stPosition, MTH3D_tdstVector_* _p_stRefAxisZ)
{
    tdstDynam_* component = inv_AI_fn_p_stComponentOf(_hSuperObjCamera);
    DNM_stReport_* report = fn_pstGetDNMReport(_hSuperObjCamera);
    CameraAxis axisX;
    CameraAxis axisZ;
    CameraAxis axisY;
    MTH3D_tdstVector_ position;
    f32 zero;

    position = *inv_POS_fn_p_stPosMatrixTranslation(_hSuperObjCamera->gu_hGlobalMatrix);
    axisY.value = *_p_stRefAxisZ;
    CAM_fn_vComputeCameraSystemAxis(component->p_stDynamics, _hSuperObjCamera, &position, _p_stPosition, &axisX.value,
                                    &axisZ.value, &axisY.value);
    _POS_fn_vSetRotationMatrix(_hSuperObjCamera->gu_hLocalMatrix, &axisX.value, &axisZ.value, &axisY.value);
    _POS_fn_vSetRotationMatrix(_hSuperObjCamera->gu_hGlobalMatrix, &axisX.value, &axisZ.value, &axisY.value);
    component->stParsingDatas.stVector = *_p_stPosition;
    zero = 0.0f;
    *(volatile f32*) &report->m_stAbsolutePrevPosition.m_stLinear.xZ = zero;
    *(volatile f32*) &report->m_stAbsolutePrevPosition.m_stLinear.xX = zero;
    *(volatile f32*) &report->m_stAbsolutePrevPosition.m_stLinear.xY = zero;
}

s32 CAM_fn_bPersoIsMoving(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stMove)
{
    MTH3D_tdstVector_ speed;
    f32 length;

    CAM_fn_vAverageSpeed(_p_stStruct->hCineinfo, &speed, CAM_g_stCameraConstants.xAverageForComputeLinearSpeed);
    length = MTH_fn_xSqrAddSqrAddSqrOpt2(speed.xX, speed.xY, speed.xZ);
    length = MTH_fn_xSqrtOpt2(length);
    if (CAM_g_stCameraConstants.xMinLinearSpeedTgtPerso < length) {
        if (_p_stMove != NULL) {
            *_p_stMove = speed;
        }
        return 1;
    }
    if (_p_stMove != NULL) {
        f32 zero = 0.0f;
        MTH3D_M_vSetXYZ_inv(_p_stMove, zero, zero, zero);
    }
    return 0;
}

s32 CAM_fn_bPersoIsTurning(HIE_tdstSuperObject_* _h_SuperObjPerso, f32* _p_xAngularSpeed)
{
    tdstDynam_* component;
    DNM_stDynamics* dynamics;
    DNM_stReport_* report;

    if (_p_xAngularSpeed != NULL) {
        *_p_xAngularSpeed = 0;
    }
    component = inv_AI_fn_p_stComponentOf(_h_SuperObjPerso);
    if (component == NULL) {
        return 0;
    }
    dynamics = component->p_stDynamics;
    if (dynamics == NULL) {
        return 0;
    }
    report = dynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    if (report == NULL) {
        return 0;
    }
    if (CAM_g_stCameraConstants.xMinAngularSpeedTgtPerso < report->m_stAbsoluteCurrSpeed.m_stAngular.m_xAngle) {
        if (_p_xAngularSpeed != NULL) {
            *_p_xAngularSpeed = report->m_stAbsoluteCurrSpeed.m_stAngular.m_xAngle;
        }
        return 1;
    }
    return 0;
}

s32 CAM_fn_bPersoOnFace(HIE_tdstSuperObject_* _h_SuperObjPerso, MTH3D_tdstVector_* _p_stNormal)
{
    tdstDynam_* component;
    DNM_stDynamics* dynamics;
    DNM_stReport_* report;

    component = inv_AI_fn_p_stComponentOf(_h_SuperObjPerso);
    if (component == NULL) {
        return 0;
    }
    dynamics = component->p_stDynamics;
    if (dynamics == NULL) {
        return 0;
    }
    report = dynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    if (report == NULL) {
        return 0;
    }
    if ((report->m_ulCurrSurfaceState & 1) == 0) {
        return 0;
    }
    *_p_stNormal = report->m_stGround.m_stNorm;
    return 1;
}

HIE_tdstSuperObject_* CAM_fn_hGetFather(HIE_tdstSuperObject_* _h_SuperObjPerso)
{
    tdstDynam_* p_stDynamics;
    DNM_stDynamics* h_Dynam;

    if (_h_SuperObjPerso == NULL) {
        return NULL;
    }
    p_stDynamics = inv_AI_fn_p_stComponentOf(_h_SuperObjPerso);
    if (p_stDynamics == NULL) {
        return NULL;
    }
    h_Dynam = inv_MEC_fn_p_stDynamicsOf(p_stDynamics);
    if (h_Dynam == NULL) {
        return NULL;
    }
    if ((h_Dynam->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_COMPLETE) == 0) {
        return NULL;
    }
    return h_Dynam->m_tdstDynamicsComplexBlock.m_hPlatform;
}

void CAM_fn_vComputeProjection(MTH3D_tdstVector_* _p_sDestVector, MTH3D_tdstVector_* _p_sSrcVector,
                               MTH3D_tdstVector_* _p_sNormVector)
{
    f32 dot;
    MTH3D_tdstVector_ projection;

    dot = inv_MTH_fn_xMulAdd3Grouped(_p_sSrcVector->xX, _p_sSrcVector->xY, _p_sSrcVector->xZ,
                           _p_sNormVector->xX, _p_sNormVector->xY, _p_sNormVector->xZ);
    if (dot != 0.0f) {
        inv_MTH3D_fn_vKTimesXYZ(&projection, dot, _p_sNormVector);
        inv_MTH3D_fn_vSubXYZ(_p_sDestVector, _p_sSrcVector, &projection);
    } else {
        *_p_sDestVector = *_p_sSrcVector;
    }
}

f32 CAM_fn_xComputeRotation(MTH3D_tdstVector_* _p_stDestVector, MTH3D_tdstVector_* _p_stSrcVector, MTH3D_tdstVector_* _p_stAxisVector)
{
    f32 lengthA;
    f32 lengthB;
    f32 dot;
    f32 cosine;

    if ((MTH3D_M_bIsZeroXYZ_inv(_p_stSrcVector)) ||
        (MTH3D_M_bIsZeroXYZ_inv(_p_stDestVector))) {
        return 0.0f;
    }

    lengthA = MTH_fn_xSqrAddSqrAddSqrOpt2(_p_stDestVector->xX, _p_stDestVector->xY, _p_stDestVector->xZ);
    lengthB = MTH_fn_xSqrAddSqrAddSqrOpt2(_p_stSrcVector->xX, _p_stSrcVector->xY, _p_stSrcVector->xZ);
    dot = inv_MTH_fn_xMulAdd3Grouped(_p_stSrcVector->xX, _p_stSrcVector->xY, _p_stSrcVector->xZ,
                           _p_stDestVector->xX, _p_stDestVector->xY, _p_stDestVector->xZ);
    cosine = dot / MTH_fn_xSqrtOpt2(lengthB * lengthA);

    if (_p_stAxisVector != NULL) {
        if (_p_stAxisVector == _p_stSrcVector || _p_stAxisVector == _p_stDestVector) {
            MTH3D_tdstVector_ cross;
            f32 x;
            f32 y;
            f32 z;
            x = inv_MTH_fn_xMulSub(_p_stSrcVector->xY, _p_stDestVector->xZ, _p_stSrcVector->xZ,
                                   _p_stDestVector->xY);
            y = inv_MTH_fn_xMulSub(_p_stSrcVector->xZ, _p_stDestVector->xX, _p_stSrcVector->xX,
                                   _p_stDestVector->xZ);
            z = inv_MTH_fn_xMulSub(_p_stSrcVector->xX, _p_stDestVector->xY, _p_stSrcVector->xY,
                                   _p_stDestVector->xX);
            MTH3D_M_vSetXYZ_inv(&cross, x, y, z);
            *_p_stAxisVector = cross;
        } else {
            DNM_p_stVectorCrossProduct(_p_stAxisVector, _p_stSrcVector, _p_stDestVector);
        }

        if (!MTH3D_M_bIsZeroXYZ_inv(_p_stAxisVector)) {
            inv_MTH3D_fn_vNormalizeXYZ(_p_stAxisVector);
        }
    }

    if (cosine < -1.0f) {
        cosine = -1.0f;
    } else if (cosine > 1.0f) {
        cosine = 1.0f;
    }
    return acosf(cosine);
}

s32 CAM_fn_bEqualVectorWithEpsilon(MTH3D_tdstVector_* _p_stVector1, MTH3D_tdstVector_* _p_stVector2, f32 _xEpsilon)
{
    MTH3D_tdstVector_ delta;
    f32 distance;

    _xEpsilon = _xEpsilon * _xEpsilon;
    inv_MTH3D_fn_vSubXYZ(&delta, _p_stVector1, _p_stVector2);
    distance = MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ);
    if (_xEpsilon < distance) {
        return 0;
    }
    return 1;
}

void func_001E3F08(tdstInternalStructurCineinfo_* slot, f32* horizontal, f32* vertical)
{
    f32 angle = *vertical;

    if (MTH_HALF_PI_inv < inv_MTH_fn_xAbsolute(angle)) {
        if (0.0f < angle) {
            *vertical = MTH_PI_inv - angle;
        } else {
            *vertical = inv_MTH_fn_xNegate(MTH_PI_inv) - angle;
        }
    }
    *horizontal = CAM_fn_xSetAngleInInterval02Pi(*horizontal);
    *vertical = CAM_fn_xSetAngleInInterval02Pi(*vertical);
}

void CAM_fn_vComputeAnglesBetweenTwoVectors(tdstInternalStructurCineinfo_* slot, MTH3D_tdstVector_* wanted,
                   MTH3D_tdstVector_* current, f32* horizontal, f32* vertical,
                   MTH3D_tdstVector_* reference)
{
    *horizontal = CAM_fn_xComputeHorizontalAngle(wanted, current, reference);
    *vertical = CAM_fn_xComputeVerticalAngle(wanted, current, reference);
    func_001E3F08(slot, horizontal, vertical);
}

f32 CAM_fn_xSetAngleInIntervalPiPi(f32 _xAngle)
{
    f32 limit = MTH_PI_inv;

    if (limit < _xAngle) {
        return _xAngle - MTH_TWO_PI_inv;
    }
    limit = inv_MTH_fn_xNegateTied(limit);
    if (_xAngle <= limit) {
        return _xAngle + MTH_TWO_PI_inv;
    }
    return _xAngle;
}

f32 CAM_fn_xSetAngleInInterval02Pi(f32 _xAngle)
{
    f32 twoPi;

    if (_xAngle < 0.0f) {
        return _xAngle + MTH_TWO_PI_inv;
    }
    twoPi = MTH_TWO_PI_inv;
    if (twoPi <= _xAngle) {
        return _xAngle - twoPi;
    }
    return _xAngle;
}

void CAM_fn_vGetPersoSightAxis(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stAxis,
                   MTH3D_tdstVector_* _p_stAxisZ)
{
    HIE_tdstSuperObject_* target;
    tdstCineinfo_* cineinfo;
    MTH3D_tdstVector_ first;
    MTH3D_tdstVector_ second;
    MTH3D_tdstVector_ x;
    MTH3D_tdstVector_ z;
    tdstAChannel_* records;
    s32 channel;
    u32 index;
    HIE_tdstSuperObject_* channelTarget;

    target = _p_stStruct->hCineinfoWork->hSuperObjectTargeted;
    cineinfo = _p_stStruct->hCineinfo;
    *_p_stAxis = cineinfo->stSightAxisSystem.SightAxis;
    first = cineinfo->stSightAxisSystem.FirstComplementaryAxis;
    second = cineinfo->stSightAxisSystem.SecondComplementaryAxis;
    channel = (u8) _p_stStruct->hCineinfoWork->cChannel;
    if (channel != 0xFF) {
        index = (u16) channel;
        records = inv_HIE_fn_p_stGetEngineObject(target)->h_3dData->hArrayOfChannels;
        if (records != NULL) {
            channelTarget = records[index].hSupObject;
        } else {
            channelTarget = NULL;
        }
        target = channelTarget;
    }
    POS_fn_vGetRotationVectors(target->gu_hGlobalMatrix, &x, _p_stAxis, &z);
    if (_p_stAxisZ != NULL) {
        *_p_stAxisZ = z;
    }
    _p_stAxis->xX = inv_MTH_fn_xNegate(_p_stAxis->xX);
    _p_stAxis->xY = inv_MTH_fn_xNegate(_p_stAxis->xY);
    _p_stAxis->xZ = inv_MTH_fn_xNegate(_p_stAxis->xZ);
}

extern f32 flt_393D54[];

f32 Cam_fn_xComputeProjectionRail(MTH3D_tdstVector_* _p_stTargetedPersoPos, MTH3D_tdstVector_* _p_stWpPosition, MTH3D_tdstVector_* _p_stDestWpPosition)
{
    MTH3D_tdstVector_ segment;
    MTH3D_tdstVector_ offset;
    f32 length2;
    f32 t;

    inv_MTH3D_fn_vSubXYZ(&segment, _p_stDestWpPosition, _p_stWpPosition);
    inv_MTH3D_fn_vSubXYZ(&offset, _p_stTargetedPersoPos, _p_stWpPosition);

    length2 = MTH_fn_xSqrAddSqrAddSqrOpt2(segment.xX, segment.xY, segment.xZ);
    if (length2 == 0.0f) {
        return flt_393D54[0];
    }

    t = inv_MTH3D_fn_xDotXYZGrouped(&segment, &offset) / length2;
    if (t < 0.0f) {
        f32 startDistance;
        f32 endDistance;

        startDistance = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(
            _p_stWpPosition->xX - _p_stTargetedPersoPos->xX, _p_stWpPosition->xY - _p_stTargetedPersoPos->xY, _p_stWpPosition->xZ - _p_stTargetedPersoPos->xZ));
        endDistance = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(
            _p_stDestWpPosition->xX - _p_stTargetedPersoPos->xX, _p_stDestWpPosition->xY - _p_stTargetedPersoPos->xY, _p_stDestWpPosition->xZ - _p_stTargetedPersoPos->xZ));
        if (endDistance < startDistance) {
            return endDistance;
        }
        return startDistance;
    }

    {
        MTH3D_tdstVector_ projected;
        MTH3D_tdstVector_ closest;

        inv_MTH3D_fn_vKTimesXYZ(&projected, t, &segment);
        MTH3D_M_vAddXYZ_inv(&closest, &projected, _p_stWpPosition);
        return MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(
            closest.xX - _p_stTargetedPersoPos->xX, closest.xY - _p_stTargetedPersoPos->xY, closest.xZ - _p_stTargetedPersoPos->xZ));
    }
}

void Cam_fn_vComputeMovingOnRail(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stWantedPosition,
                                 MTH3D_tdstVector_* _p_stWpPosition,
                                 MTH3D_tdstVector_* _p_stPreviousWpPosition, f32 _xDistWanted)
{
    MTH3D_tdstVector_ ray;
    MTH3D_tdstVector_ axis;
    MTH3D_tdstVector_ nearest;
    MTH3D_tdstVector_ perpendicular;
    MTH3D_tdstVector_ projection;
    MTH3D_tdstVector_ direction;
    MTH3D_tdstVector_ foot;
    MTH3D_tdstVector_ point;
    f32 squared;
    f32 parameter;
    f32 epsilon;
    f32 dot;
    f32 radiusSquared;
    f32 perpendicularSquared;
    f32 footDistance;
    f32 distance;
    f32 rayLength;
    s32 swapped = 0;

    CAM_fn_vComputeReferencePoint(_p_stStruct);
    MTH3D_M_vSubXYZ_inv(&direction, _p_stWpPosition, &_p_stStruct->stCurrentCameraPos);
    point = *_p_stWpPosition;
    MTH3D_M_vSubXYZ_inv(&axis, &_p_stStruct->stTargetedPersoPos, &_p_stStruct->stCurrentCameraPos);
    squared = MTH_fn_xSqrAddSqrAddSqrOpt2(direction.xX, direction.xY, direction.xZ);
    if (squared == 0.0f) {
        MTH3D_M_vSubXYZ_inv(&direction, &_p_stStruct->stCurrentCameraPos, _p_stPreviousWpPosition);
        point = *_p_stPreviousWpPosition;
        squared = MTH_fn_xSqrAddSqrAddSqrOpt2(direction.xX, direction.xY, direction.xZ);
        if (squared == 0.0f) {
            *_p_stWantedPosition = _p_stStruct->stCurrentCameraPos;
            return;
        }
        swapped = 1;
    }
    dot = inv_MTH_fn_xMulAdd3Grouped(direction.xX, direction.xY, direction.xZ, axis.xX, axis.xY,
                                     axis.xZ);
    epsilon = 0.0001f;
    parameter = dot / squared;
    if (inv_MTH_fn_xAbsolute(parameter) < epsilon) {
        return;
    }
    if (swapped != 0) {
        MTH3D_M_vSubXYZ_inv(&direction, &_p_stStruct->stCurrentCameraPos, &point);
    } else {
        MTH3D_M_vSubXYZ_inv(&direction, &point, &_p_stStruct->stCurrentCameraPos);
    }
    radiusSquared = _xDistWanted * _xDistWanted;
    inv_MTH3D_fn_vKTimesXYZ(&projection, parameter, &direction);
    MTH3D_M_vAddXYZ_inv(&foot, &_p_stStruct->stCurrentCameraPos, &projection);
    inv_MTH3D_fn_vSubXYZ(&perpendicular, &axis, &projection);
    perpendicularSquared =
        MTH_fn_xSqrAddSqrAddSqrOpt2(perpendicular.xX, perpendicular.xY, perpendicular.xZ);
    {
        f32 x = foot.xX - _p_stStruct->stCurrentCameraPos.xX;
        f32 y = foot.xY - _p_stStruct->stCurrentCameraPos.xY;
        f32 z = foot.xZ - _p_stStruct->stCurrentCameraPos.xZ;
        f32 footSquared;
        footSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
        footDistance = MTH_fn_xSqrtOpt2(footSquared);
    }
    if (!(radiusSquared < perpendicularSquared)) {
        distance = MTH_fn_xSqrtOpt2(radiusSquared - perpendicularSquared);
        if (footDistance < distance) {
            distance = distance - footDistance;
            if (0.0f < parameter) {
                point = *_p_stPreviousWpPosition;
            } else if (swapped != 0) {
                return;
            }
        } else {
            distance = footDistance - distance;
            if (parameter < 0.0f) {
                point = *_p_stPreviousWpPosition;
            } else if (swapped != 0) {
                return;
            }
        }
    } else {
        if (parameter < 0.0f) {
            point = *_p_stPreviousWpPosition;
        } else if (swapped != 0) {
            return;
        }
        distance = footDistance;
    }
    MTH3D_M_vSubXYZ_inv(&ray, &point, &_p_stStruct->stCurrentCameraPos);
    {
        f32 raySquared;
        raySquared = MTH_fn_xSqrAddSqrAddSqrOpt2(ray.xX, ray.xY, ray.xZ);
        rayLength = MTH_fn_xSqrtOpt2(raySquared);
    }
    if (rayLength < distance) {
        *_p_stWantedPosition = point;
        return;
    }
    if (rayLength != 0.0f) {
        f32 inverse = 1.0f / rayLength;
        inv_MTH3D_fn_vKTimesXYZ(&ray, inverse, &ray);
    }
    inv_MTH3D_fn_vKTimesXYZ(&nearest, distance, &ray);
    MTH3D_M_vAddXYZ_inv(_p_stWantedPosition, &_p_stStruct->stCurrentCameraPos, &nearest);
}

void CAM_fn_vInitCameraStructure(HIE_tdstSuperObject_* _hSuperObjPerso, CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstDynam_* component;
    DNM_stDynamics* dynamics;
    DNM_stDynamics* targetDynamics;
    MTH3D_tdstVector_* position;
    f32 length;

    memset(_p_stStruct, 0, sizeof(CAM_tdstUpdateCamera_));

    component = inv_AI_fn_p_stComponentOf(_hSuperObjPerso);
    dynamics = inv_MEC_fn_p_stDynamicsOf(component);
    _p_stStruct->p_stDynamics = dynamics;
    _p_stStruct->p_stReport = dynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    _p_stStruct->hCineinfo = inv_HIE_fn_p_stGetEngineObject(_hSuperObjPerso)->h_Cineinfo;
    _p_stStruct->hCineinfoWork = inv_HIE_fn_p_stGetEngineObject(_hSuperObjPerso)->h_Cineinfo->hWork;
    _p_stStruct->hCineinfoCurrent = inv_HIE_fn_p_stGetEngineObject(_hSuperObjPerso)->h_Cineinfo->hCurrent;
    _p_stStruct->hCineinfoInit = inv_HIE_fn_p_stGetEngineObject(_hSuperObjPerso)->h_Cineinfo->hInit;
    _p_stStruct->hCineinfoVisibility = inv_HIE_fn_p_stGetEngineObject(_hSuperObjPerso)->h_Cineinfo->hVisibility;
    _p_stStruct->hSuperObjCamera = _hSuperObjPerso;
    _p_stStruct->stCurrentCameraPos = *inv_POS_fn_p_stPosMatrixTranslation(_hSuperObjPerso->gu_hGlobalMatrix);

    if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted == NULL) {
        return;
    }

    _p_stStruct->cTgtPersoIsMovingAbsolute =
        CAM_fn_bPersoIsMoving(_p_stStruct, &_p_stStruct->stTgtPersoMoveAbsolute);

    if (CAM_fn_hGetFather(_p_stStruct->hCineinfoWork->hSuperObjectTargeted) == NULL) {
        _p_stStruct->cTgtPersoIsMovingRelative = _p_stStruct->cTgtPersoIsMovingAbsolute;
        _p_stStruct->stTgtPersoMoveRelative = _p_stStruct->stTgtPersoMoveAbsolute;
        return;
    }

    component = inv_AI_fn_p_stComponentOf(_p_stStruct->hCineinfoWork->hSuperObjectTargeted);
    targetDynamics = inv_MEC_fn_p_stDynamicsOf(component);
    _p_stStruct->stTgtPersoMoveRelative = *inv_POS_fn_p_stPosMatrixTranslation(&targetDynamics->m_tdstDynamicsBaseBlock.m_stPreviousMatrix);
    position = inv_POS_fn_p_stPosMatrixTranslation(
        &targetDynamics->m_tdstDynamicsComplexBlock.m_stPrevPreviousMatrix);
    MTH3D_M_vSubXYZ_inv(&_p_stStruct->stTgtPersoMoveRelative, position,
                        &_p_stStruct->stTgtPersoMoveRelative);

    length = MTH_fn_xSqrAddSqrAddSqrOpt2(_p_stStruct->stTgtPersoMoveRelative.xX,
                                         _p_stStruct->stTgtPersoMoveRelative.xY,
                                         _p_stStruct->stTgtPersoMoveRelative.xZ);
    length = MTH_fn_xSqrtOpt2(length);

    if (CAM_g_stCameraConstants.xMinLinearSpeedTgtPerso < length) {
        _p_stStruct->cTgtPersoIsMovingRelative = 1;
    } else {
        _p_stStruct->cTgtPersoIsMovingRelative = 0;
    }
}

#ifdef PC_PORT
static void CAM_fn_vSendParametersToParsing_game(CAM_tdstUpdateCamera_* _p_stStruct);

void CAM_fn_vSendParametersToParsing(CAM_tdstUpdateCamera_* _p_stStruct)
{
    CAM_fn_vSendParametersToParsing_game(_p_stStruct);
    pc_mod_camera_updated((PcAddress) _p_stStruct);
}

#define CAM_fn_vSendParametersToParsing CAM_fn_vSendParametersToParsing_game
#endif
void CAM_fn_vSendParametersToParsing(CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstDynam_* component;
    CameraParameters* out;
    MTH3D_tdstVector_* persoPos;
    tdstCineinfo_* cineinfo;
    u16 flags;

    component = inv_AI_fn_p_stComponentOf(_p_stStruct->hSuperObjCamera);
    if (component == NULL) {
        return;
    }

    out = (CameraParameters*) &inv_MEC_fn_p_stDynamicsOf(component)
              ->m_tdstDynamicsComplexBlock.m_stExternalDatas;
    out->xCutAngleFactorPos = CAM_g_stCameraConstants.xCutAngleFactorPos;
    out->xCutAngleFactorTgt = CAM_g_stCameraConstants.xCutAngleFactorTgt;

    out->distMin = _p_stStruct->hCineinfoWork->xAngleTheta;
    out->target = _p_stStruct->stCptPos.stTarget;
    out->movePos = _p_stStruct->stCptPos.stMovePos;
    out->linearSpeed = _p_stStruct->stCptPos.xLinearSpeed;
    if (_p_stStruct->hCineinfoWork->xLinearIncreaseSpeed == 0.0f) {
        _p_stStruct->hCineinfoWork->xLinearIncreaseSpeed = 0.001f;
    }
    out->boundDistMax = _p_stStruct->hCineinfoWork->xLinearIncreaseSpeed;
    out->angleAlpha = _p_stStruct->hCineinfoWork->xLinearDecreaseSpeed;
    out->angularSpeed = _p_stStruct->stCptPos.xAngularSpeed;
    if (_p_stStruct->hCineinfoWork->xAngularIncreaseSpeed == 0.0f) {
        _p_stStruct->hCineinfoWork->xLinearIncreaseSpeed = 0.001f;
    }
    out->angleTeta = _p_stStruct->hCineinfoWork->xAngularIncreaseSpeed;
    out->angleShiftTeta = _p_stStruct->hCineinfoWork->xAngularDecreaseSpeed;
    out->targetSpeed = _p_stStruct->stCptPos.xTargetSpeed;
    if (_p_stStruct->hCineinfoWork->xAngularIncreaseSpeed == 0.0f) {
        _p_stStruct->hCineinfoWork->xTargetIncreaseSpeed = 0.001f;
    }
    out->angleOrientationY = _p_stStruct->hCineinfoWork->xTargetIncreaseSpeed;
    out->angleOrientationZ = _p_stStruct->hCineinfoWork->xTargetDecreaseSpeed;
    out->targetedPersoPos = _p_stStruct->stTargetedPersoPos;
    out->refAxisZ = _p_stStruct->stCptPos.stRefAxisZ;

    persoPos = &out->targetedPersoPos;
    cineinfo = _p_stStruct->hCineinfo;
    if ((cineinfo->ucVolIAFlags & 2) != 0) {
        _p_stStruct->hCineinfoWork->uwDNMFlags |= 1;
        _p_stStruct->hCineinfoWork->uwDNMFlags |= 4;
        _p_stStruct->hCineinfoWork->uwDNMFlags |= 0x10;
    }

    if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted == NULL) {
        persoPos->xX = 0.0f;
        persoPos->xY = 0.0f;
        out->targetedPersoPos.xZ = 0.0f;
    }

    if ((_p_stStruct->hCineinfoWork->uwIAFlags & 0x2000) != 0) {
        _p_stStruct->hCineinfoWork->uwDNMFlags |= 4;
        persoPos->xX = 0.0f;
        persoPos->xY = 0.0f;
        out->targetedPersoPos.xZ = 0.0f;
    }

    flags = _p_stStruct->hCineinfoWork->uwDNMFlags;
    if ((flags & 0x80) != 0) {
        _p_stStruct->hCineinfoWork->uwDNMFlags = flags | 0x10;
    }
    flags = _p_stStruct->hCineinfoWork->uwDNMFlags;
    if ((flags & 1) != 0) {
        _p_stStruct->hCineinfoWork->uwDNMFlags = flags | 2;
        out->xCutAngleFactorPos = 1.0f;
    }
    flags = _p_stStruct->hCineinfoWork->uwDNMFlags;
    if ((flags & 4) != 0) {
        _p_stStruct->hCineinfoWork->uwDNMFlags = flags | 8;
        out->xCutAngleFactorPos = 1.0f;
    }
    flags = _p_stStruct->hCineinfoWork->uwDNMFlags;
    if ((flags & 0x10) != 0) {
        _p_stStruct->hCineinfoWork->uwDNMFlags = flags | 0x20;
        out->xCutAngleFactorTgt = 1.0f;
    }
    out->dnmFlags = _p_stStruct->hCineinfoWork->uwDNMFlags;

    if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted != NULL &&
        (_p_stStruct->hCineinfoWork->uwDNMFlags & 0x40) != 0) {
        inv_HIE_fn_p_stGetEngineObject(_p_stStruct->hSuperObjCamera)->h_SectInfo->hCurrentSector =
            inv_HIE_fn_p_stGetEngineObject(_p_stStruct->hCineinfoWork->hSuperObjectTargeted)->h_SectInfo->hCurrentSector;
    }

    if (_p_stStruct->cTgtPersoIsMovingAbsolute == 0) {
        out->xCutAngleFactorTgt = 1.0f;
        out->xCutAngleFactorPos = 1.0f;
    }
}
#ifdef PC_PORT
#undef CAM_fn_vSendParametersToParsing
#endif
