#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/r5900.h"
#ifdef PC_PORT
f32 fabsf(f32 value);
#endif

extern f32 inv_CAM_xAverageForComputePos[3];
extern f32 inv_CAM_xBaseCutAngle[];

static inline f32 inv_MTH_fn_xSquareRoot(f32 in)
{
    f32 ret;
    ret = MTH_fn_xSqrtOpt2(in);
    return ret;
}

static inline f32 inv_AI_fn_xCameraLength3(f32 y, f32 z, f32 x)
{
    f32 accum;
    accum = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    return inv_MTH_fn_xSquareRoot(accum);
}

static __inline__ s32 inv_AI_fn_lNormalizeVector(MTH3D_tdstVector_* dst, MTH3D_tdstVector_* src)
{
    f32 len2;
    f32 len;
    f32 xInvLength;
    len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(src->xX, src->xY, src->xZ);
    len = inv_MTH_fn_xSquareRoot(len2);
    if (len == 0.0f)
        return 0;
    xInvLength = 1.0f / len;
    inv_MTH3D_fn_vKTimesXYZ(dst, xInvLength, src);
    return 1;
}

static __inline__ f32 inv_AI_fn_xScaledLength3(f32 x, f32 y, f32 z, f32 scale)
{
    f32 dot;
    dot = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    dot = MTH_fn_xSqrtOpt2(dot);
    return dot * scale;
}

static inline f32 inv_AI_fn_xClampedTargetSpeed(f32 n, f32 low, f32 high)
{
    if (n < MTH_fn_xSqrOpt1(low))
        n = low;
    else if (MTH_fn_xSqrOpt1(high) < n)
        n = high;
    else
        n = inv_MTH_fn_xSquareRoot(n);
    if (n <= 0.01f)
        n = 0.01f;
    return n;
}

void CAM_fn_vComputeRefAxis(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stRefAxis)
{
    MTH3D_tdstVector_ zero;
    f32 zero_f;
    f32 len2;
    f32 xInvLength;

    if (cRefAxisIsAlreadyComputed != 0) {
        zero_f = 0.0f;
        MTH3D_M_vSetXYZ_inv(&zero, zero_f, zero_f, zero_f);

        if (CAM_fn_bEqualVectorWithEpsilon(&_p_stStruct->hCineinfo->stForceRefAxisY, &zero,
                                           0.0001f) != 0) {
            ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xLinearDecreaseSpeed = 0.0f;
            ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularSpeed = -1.0f;
            ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularIncreaseSpeed = 0.0f;
            *_p_stRefAxis = _p_stStruct->hCineinfo->stForceRefAxisY;
        } else {
            f32 len2, len, inv;
            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(
                ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xLinearDecreaseSpeed,
                ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularSpeed,
                ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularIncreaseSpeed);
            len = inv_MTH_fn_xSquareRoot(len2);
            if (len != 0.0f) {
                inv = 1.0f / len;
                MTH3D_M_vSetXYZ_inv(
                    _p_stRefAxis, inv * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xLinearDecreaseSpeed,
                    inv * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularSpeed,
                    inv * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularIncreaseSpeed);
            } else {
                *_p_stRefAxis = _p_stStruct->hCineinfo->stForceRefAxisY;
            }
        }

        if (CAM_fn_bEqualVectorWithEpsilon(&_p_stStruct->hCineinfo->stForceRefAxisZ, &zero,
                                           0.0001f) != 0) {
            ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularDecreaseSpeed = 0.0f;
            ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed = 0.0f;
            ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed = 1.0f;
        } else {
            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(
                ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularDecreaseSpeed,
                ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed,
                ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed);
            len2 = inv_MTH_fn_xSquareRoot(len2);
            if (len2 != 0.0f) {
                xInvLength = 1.0f / len2;
                _p_stStruct->stCptPos.stRefAxisZ.xX =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularDecreaseSpeed;
                _p_stStruct->stCptPos.stRefAxisZ.xY =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed;
                _p_stStruct->stCptPos.stRefAxisZ.xZ =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed;
                return;
            }
            _p_stStruct->stCptPos.stRefAxisZ = _p_stStruct->hCineinfo->stForceRefAxisZ;
        }
        return;
    }

    if (_p_stStruct->hCineinfoWork->hSecondSuperObjectTargeted != 0) {
        MTH3D_M_vSubXYZ_inv(_p_stRefAxis, &_p_stStruct->stSecondTargetedPersoPos,
                            &_p_stStruct->stTargetedPersoPos);
        inv_AI_fn_lNormalizeVector(_p_stRefAxis, _p_stRefAxis);
        MTH3D_M_vSetXYZ_inv(&_p_stStruct->stCptPos.stRefAxisZ, 0.0f, 0.0f, 1.0f);
        return;
    }

    CAM_fn_vGetPersoSightAxis(_p_stStruct, _p_stRefAxis, &_p_stStruct->stCptPos.stRefAxisZ);
    MTH3D_M_vSetXYZ_inv(&_p_stStruct->stCptPos.stRefAxisZ, 0.0f, 0.0f, 1.0f);
    MTH3D_M_vSetXYZ_inv(_p_stRefAxis, inv_MTH_fn_xNegateTied(_p_stRefAxis->xX),
                        inv_MTH_fn_xNegateTied(_p_stRefAxis->xY),
                        inv_MTH_fn_xNegateTied(_p_stRefAxis->xZ));
}

static inline HIE_tdstSuperObject_* CAM_hChannelSuperObject(HIE_tdstSuperObject_* so, s32 channel)
{
    tdstAChannel_* records;
    HIE_tdstSuperObject_* record;
    u32 index;

    records = (tdstAChannel_*) ((tdstEngineObject_*) so->hLinkedObject)->h_3dData->hArrayOfChannels;
    index = ((u32) channel & 0xFFFF);
    if (records != NULL) {
        record = records[index].hSupObject;
    } else {
        record = NULL;
    }
    return record;
}

void CAM_fn_vComputeTargetedPersoPos(CAM_tdstUpdateCamera_* _p_stStruct, HIE_tdstSuperObject_* _hPerso,
                                     MTH3D_tdstVector_* _p_stTargetedPersoPos, MTH3D_tdstVector_* _p_stTargetedPersoRealPos)
{
    MTH3D_tdstVector_ offset;
    tdstCineinfo_* cine;
    u8 channel;

    if (_hPerso == NULL) {
        _p_stTargetedPersoPos->xZ = _p_stTargetedPersoPos->xY = _p_stTargetedPersoPos->xX = 0.0f;
        _p_stTargetedPersoRealPos->xZ = _p_stTargetedPersoRealPos->xY = _p_stTargetedPersoRealPos->xX = 0.0f;
        return;
    }

    channel = _p_stStruct->hCineinfoWork->cChannel;
    if (channel != 0xFF) {
        _hPerso = CAM_hChannelSuperObject(_hPerso, channel);
    }
    *(MTH3D_tdstVector_*) _p_stTargetedPersoRealPos = *(MTH3D_tdstVector_*) &_hPerso->gu_hGlobalMatrix->stMatrix.stCol_3;

    cine = _p_stStruct->hCineinfo;
    if (((cine->ucVolIAFlags & 2) != 0) ||
        ((_p_stStruct->hCineinfoWork->uwIAFlags & 2) != 0) || (channel != 0xFF)) {
    } else {
        if (((tdstEngineObject_*) _hPerso->hLinkedObject)->h_Dynam != NULL) {
            goto L_avg;
        }
    }
    *_p_stTargetedPersoPos = *_p_stTargetedPersoRealPos;
    goto L_join;

L_avg:
    CAM_fn_vAveragePosition(cine, _p_stTargetedPersoPos, inv_CAM_xAverageForComputePos[0]);

L_join:
    offset = _p_stStruct->hCineinfoWork->stShiftPos;
    POS_fn_vRotateVector(&offset, _hPerso->gu_hGlobalMatrix, &offset);

    MTH3D_M_vAddXYZ_inv(_p_stTargetedPersoPos, _p_stTargetedPersoPos, &offset);
    MTH3D_M_vAddXYZ_inv(_p_stTargetedPersoRealPos, _p_stTargetedPersoRealPos, &offset);
}

void CAM_fn_vComputeReferencePoint(CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstInternalStructurCineinfo_* work;
    HIE_tdstSuperObject_* second;

    work = _p_stStruct->hCineinfoWork;
    if (work->hSecondSuperObjectTargeted != NULL) {
        work->uwIAFlags |= 2;
        work = _p_stStruct->hCineinfoWork;
    }
    CAM_fn_vComputeTargetedPersoPos(_p_stStruct, work->hSuperObjectTargeted, &_p_stStruct->stTargetedPersoPos,
                                    &_p_stStruct->stTargetedPersoRealPos);

    second = _p_stStruct->hCineinfoWork->hSecondSuperObjectTargeted;
    if (second == NULL) {
        return;
    }
    CAM_fn_vComputeTargetedPersoPos(_p_stStruct, second, &_p_stStruct->stSecondTargetedPersoPos,
                                    &_p_stStruct->stSecondTargetedPersoRealPos);

    _p_stStruct->stTargetedPersoPos.xX +=
        _p_stStruct->hCineinfo->xApexRatio * (_p_stStruct->stSecondTargetedPersoPos.xX - _p_stStruct->stTargetedPersoPos.xX);
    _p_stStruct->stTargetedPersoPos.xY +=
        _p_stStruct->hCineinfo->xApexRatio * (_p_stStruct->stSecondTargetedPersoPos.xY - _p_stStruct->stTargetedPersoPos.xY);
    _p_stStruct->stTargetedPersoPos.xZ +=
        _p_stStruct->hCineinfo->xApexRatio * (_p_stStruct->stSecondTargetedPersoPos.xZ - _p_stStruct->stTargetedPersoPos.xZ);
    _p_stStruct->stTargetedPersoRealPos.xX +=
        _p_stStruct->hCineinfo->xApexRatio *
        (_p_stStruct->stSecondTargetedPersoRealPos.xX - _p_stStruct->stTargetedPersoRealPos.xX);
    _p_stStruct->stTargetedPersoRealPos.xY +=
        _p_stStruct->hCineinfo->xApexRatio *
        (_p_stStruct->stSecondTargetedPersoRealPos.xY - _p_stStruct->stTargetedPersoRealPos.xY);
    _p_stStruct->stTargetedPersoRealPos.xZ +=
        _p_stStruct->hCineinfo->xApexRatio *
        (_p_stStruct->stSecondTargetedPersoRealPos.xZ - _p_stStruct->stTargetedPersoRealPos.xZ);
}

void CAM_fn_vComputeTarget(CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstInternalStructurCineinfo_* work = _p_stStruct->hCineinfoWork;
    HIE_tdstSuperObject_* target;
    u8 channel;
    MTH3D_tdstVector_ offset;

    if ((work->uwIAFlags & 0x1000) != 0) {
        *(MTH3D_tdstVector_*) &_p_stStruct->stCptPos.stTarget =
            *(MTH3D_tdstVector_*) &_p_stStruct->hCineinfo->stForceTarget;
        return;
    }
    target = work->hSuperObjectTargeted;
    if (target == NULL) {
        _p_stStruct->stCptPos.stTarget.xZ = _p_stStruct->stCptPos.stTarget.xY = _p_stStruct->stCptPos.stTarget.xX =
            0.0f;
        return;
    }
    if (work->hSecondSuperObjectTargeted != NULL) {
        _p_stStruct->stCptPos.stTarget = _p_stStruct->stTargetedPersoPos;
        return;
    }
    channel = work->cChannel;
    if (channel != 0xFF) {
        target = CAM_hChannelSuperObject(target, channel);
    }
    *(MTH3D_tdstVector_*) &offset = *(MTH3D_tdstVector_*) work;
    POS_fn_vRotateVector(&offset, target->gu_hGlobalMatrix, &offset);
    MTH3D_M_vAddXYZ_inv(&_p_stStruct->stCptPos.stTarget, &_p_stStruct->stTargetedPersoPos, &offset);
}

void CAM_fn_vBoundAngleWithTolerance(CAM_tdstUpdateCamera_* _p_stStruct, f32* _p_xAngleToBound, f32* _p_xCutAngle,
                                     f32* _p_xDeltaAngle, f32 _xRequestAngle, f32 _xTolerance)
{
    f32 bounded;
    f32 low;
    f32 high;
    f32 xDelta;
    s32 inside;
    (void) _p_stStruct;

    if (MTH_PI_inv <= _xTolerance) {
        *_p_xCutAngle = *_p_xAngleToBound;
        return;
    }

    bounded = _xRequestAngle;
    if (0.0f < _xTolerance) {
        _xTolerance += 0.0001f;
        low = _xRequestAngle - _xTolerance;
        high = _xRequestAngle + _xTolerance;
        if (low < 0.0f)
            low += MTH_TWO_PI_inv;
        if (MTH_TWO_PI_inv < high)
            high -= MTH_TWO_PI_inv;

        if (low < high) {
            inside = (low <= *_p_xAngleToBound) && (*_p_xAngleToBound <= high);
        } else {
            inside = !((high < *_p_xAngleToBound) && (*_p_xAngleToBound < low));
        }

        if (!inside) {
            xDelta = CAM_fn_xSetAngleInIntervalPiPi(*_p_xAngleToBound - _xRequestAngle);
            bounded = low;
            if (0.0f < xDelta)
                bounded = high;
        } else {
            bounded = *_p_xAngleToBound;
        }
    }

    xDelta = CAM_fn_xSetAngleInIntervalPiPi(bounded - *_p_xAngleToBound);
    *_p_xDeltaAngle = xDelta;
    if (*(f32*) inv_CAM_xBaseCutAngle <= __builtin_fabsf(xDelta)) {
        if (0.0f < xDelta)
            *_p_xCutAngle = *_p_xAngleToBound + *(f32*) inv_CAM_xBaseCutAngle;
        else
            *_p_xCutAngle = *_p_xAngleToBound - *(f32*) inv_CAM_xBaseCutAngle;
        *_p_xCutAngle = CAM_fn_xSetAngleInInterval02Pi(*_p_xCutAngle);
    } else {
        *_p_xCutAngle = bounded;
    }
    *_p_xAngleToBound = bounded;
}

void CAM_fn_vComputeCameraAxisAngles(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stCameraAxis,
                                     MTH3D_tdstVector_* _p_stCamTargetAxis, f32* _p_xHorAngle, f32* _p_xVerAngle)
{
#define SLOT_F32(slot, off) (*(f32*) ((u8*) (slot) + (off)))
    f32 xShiftTheta;
    f32 xShiftAlpha;
    f32 xTemp;
    tdstInternalStructurCineinfo_* p;

    CAM_fn_vComputeAnglesBetweenTwoVectors(_p_stStruct->hCineinfoWork, _p_stCameraAxis, _p_stCamTargetAxis, _p_xHorAngle, _p_xVerAngle, &_p_stStruct->stCptPos.stRefAxisZ);
    if ((_p_stStruct->hCineinfo->ucPerIAFlags & 4) != 0 ||
        (_p_stStruct->hCineinfoWork->uwIAFlags & 0x40) != 0) {
        xShiftTheta = 0.0f;
        xShiftAlpha = xShiftTheta;
    } else {
        xShiftAlpha = SLOT_F32(_p_stStruct->hCineinfoWork, 0x2C);
        xShiftTheta = SLOT_F32(_p_stStruct->hCineinfoWork, 0x34);
    }
    _p_stStruct->cHasCutAlpha = 0;
    CAM_fn_vBoundAngleWithTolerance(_p_stStruct, _p_xHorAngle, &_p_stStruct->xAngleAlphaCut,
                                    &_p_stStruct->xAngleDeltaAlphaCut,
                                    SLOT_F32(_p_stStruct->hCineinfoWork, 0x28), xShiftAlpha);
    if (*_p_xHorAngle != _p_stStruct->xAngleAlphaCut)
        _p_stStruct->cHasCutAlpha = 1;
    p = (_p_stStruct->hCineinfo->eState == CAM_e_State_GoToComputedOptimal) ? _p_stStruct->hCineinfoCurrent : _p_stStruct->hCineinfoWork;
    CAM_fn_vBoundAngleWithTolerance(_p_stStruct, _p_xVerAngle, &xTemp, &xTemp, SLOT_F32(p, 0x30), xShiftTheta);
}

void CAM_fn_vRotateCameraAxis(MTH3D_tdstVector_* _p_stCameraAxis, MTH3D_tdstVector_* b, f32 _xHorAngle, f32 _xVerAngle)
{
    MTH_stRotation rotation;
    f32 cx;
    f32 cy;
    f32 cz;
    f32 lengthSquared;
    f32 length;

    rotation.m_xAngle = _xHorAngle;
    rotation.m_stAxis = *b;
    MTH_p_stRotationMulVector(&rotation, _p_stCameraAxis, _p_stCameraAxis);

    cx = inv_MTH_fn_xMulSub(_p_stCameraAxis->xY, b->xZ, _p_stCameraAxis->xZ, b->xY);
    cy = inv_MTH_fn_xMulSub(_p_stCameraAxis->xZ, b->xX, _p_stCameraAxis->xX, b->xZ);
    cz = inv_MTH_fn_xMulSub(_p_stCameraAxis->xX, b->xY, _p_stCameraAxis->xY, b->xX);
    lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(cx, cy, cz);
    length = MTH_fn_xSqrtOpt2(lengthSquared);
    MTH3D_M_vSetXYZ_inv(&rotation.m_stAxis, cx, cy, cz);
    if (length != 0.0f) {
        f32 xInvLength;

        xInvLength = 1.0f / length;
        MTH3D_M_vSetXYZ_inv(&rotation.m_stAxis, xInvLength * cx, xInvLength * cy, xInvLength * cz);
    }
    rotation.m_xAngle = _xVerAngle;
    MTH_p_stRotationMulVector(&rotation, _p_stCameraAxis, _p_stCameraAxis);
}

void CAM_fn_vComputeRealWantedPos(CAM_tdstUpdateCamera_* _p_stStruct)
{
    CAM_tdstUpdateCamera_ local;

    local = *_p_stStruct;
    if ((_p_stStruct->hCineinfo->ucPerIAFlags & 2) != 0) {
        gst_Ideal = *_p_stStruct->hCineinfoVisibility;
        gst_Ideal.xDistMin = _p_stStruct->hCineinfoWork->xDistMin;
        gst_Ideal.xDistMax = _p_stStruct->hCineinfoWork->xDistMax;
    } else {
        gst_Ideal = *_p_stStruct->hCineinfoWork;
    }
    gst_Ideal.uwIAFlags |= 0x40;
    local.hCineinfoWork = &gst_Ideal;
    CAM_fn_vComputeMovePosWithDynTheta(&local);
    _p_stStruct->stRealWantedCameraPos = local.stWantedCameraPos;
}

s32 CAM_fn_cDynChangeTheta(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stTestedPos)
{
    MTH3D_tdstVector_ stProjected;
    MTH3D_tdstVector_ dir;
    MTH3D_tdstVector_ stRefAxis;
    MTH3D_tdstVector_ src;
    f32 len;
    f32 len2;
    f32 xLength2;
    f32 xInvLength2;
    f32 xLength;
    f32 xInvLength;
    f32 angle;
    f32 proj;
    f32 prod;
    f32 zero;
    f32 dot;
    f32 cur;
    f32 xDistMin;
    tdstInternalStructurCineinfo_* slot;

    if (CAM_fn_bPersoOnFace(_p_stStruct->hCineinfoWork->hSuperObjectTargeted, &src) == 0) {
        return 0;
    }

    CAM_fn_vComputeRefAxis(_p_stStruct, &stRefAxis);
    CAM_fn_vComputeProjection(&stProjected, &stRefAxis, &src);

    len = MTH_fn_xSqrAddSqrAddSqrOpt2(stProjected.xX, stProjected.xY, stProjected.xZ);
    xLength = inv_MTH_fn_xSquareRoot(len);
    if (xLength != 0.0f) {
        xInvLength = (f32) 1.0 / xLength;
        inv_MTH3D_fn_vKTimesXYZ(&stProjected, xInvLength, &stProjected);
    }

    MTH3D_M_vSetXYZ_inv(&dir, 0.0f, 0.0f, 1.0f);
    angle = CAM_fn_xComputeRotation(&dir, &stProjected, 0);

    MTH3D_M_vSubXYZ_inv(&dir, &_p_stStruct->stTargetedPersoPos, _p_stTestedPos);
    len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(dir.xX, dir.xY, dir.xZ);
    proj = angle - MTH_HALF_PI_inv;
    xLength2 = inv_MTH_fn_xSquareRoot(len2);
    if (xLength2 != 0.0f) {
        xInvLength2 = (f32) 1.0 / xLength2;
        inv_MTH3D_fn_vKTimesXYZ(&dir, xInvLength2, &dir);
    }

    dot = inv_MTH3D_fn_xDotXYZ(&dir, &stProjected);
    proj = proj * dot;
    if (_p_stStruct->hCineinfo->ucPerIAFlags & 2) {
        slot = _p_stStruct->hCineinfoVisibility;
    } else {
        slot = _p_stStruct->hCineinfoWork;
    }

    cur = slot->xAngleTheta;
    prod = cur * proj;
    R5900_MTC1_ZERO_F32(zero);
    if (prod < zero) {
        xDistMin = cur + proj;
    } else {
        if (__builtin_fabsf(cur) <= __builtin_fabsf(proj)) {
            proj = proj - cur;
            xDistMin = cur + proj;
        } else {
            return 0;
        }
    }

    _p_stStruct->hCineinfoWork->xAngleTheta = xDistMin;
    return 1;
}

void CAM_fn_vTakeCareOfCameraAltitude(tdstCineinfo_* cineinfo, MTH3D_tdstVector_* position)
{
    tdstInternalStructurCineinfo_* work = cineinfo->hWork;
    f32 z = position->xZ;
    f32 maximum = work->xZMax;
    if (maximum < z) {
        position->xZ = maximum;
        work = cineinfo->hWork;
    }
    {
        f32 minimum = work->xZMin;
        if (z < minimum)
            position->xZ = minimum;
    }
}

void CAM_fn_vComputeMovePos(CAM_tdstUpdateCamera_* _p_stStruct)
{
    MTH3D_tdstVector_ axis;
    MTH3D_tdstVector_ offset;
    f32 x;
    f32 y;
    f32 z;
    f32 first;
    f32 second;
    f32 len2;
    f32 len;
    f32 maxDelta;
    f32 speed;
    f32 xZero;

    if (_p_stStruct->hCineinfoWork->uwIAFlags & 0x2000) {
        if (cRefAxisIsAlreadyComputed != 0) {
            z = ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed;
            x = ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularDecreaseSpeed;
            y = ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed;
            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
            z = inv_MTH_fn_xSquareRoot(len2);
            if (z != 0.0f) {
                f32 xInvLength;
                xInvLength = 1.0f / z;
                _p_stStruct->stCptPos.stRefAxisZ.xX = xInvLength * x;
                _p_stStruct->stCptPos.stRefAxisZ.xY =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed;
                _p_stStruct->stCptPos.stRefAxisZ.xZ =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed;
            } else {
                *(MTH3D_tdstVector_*) &_p_stStruct->stCptPos.stRefAxisZ =
                    *(MTH3D_tdstVector_*) &((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)
                         ->xAngularDecreaseSpeed;
            }
        } else {
            MTH3D_M_vSetXYZ_inv(&_p_stStruct->stCptPos.stRefAxisZ, 0.0f, 0.0f, 1.0f);
        }
        cNoDynChangeTheta = 1;
        *(MTH3D_tdstVector_*) &_p_stStruct->stWantedCameraPos =
            *(MTH3D_tdstVector_*) &((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngleShiftTheta;
    } else if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted == 0) {
        if (cRefAxisIsAlreadyComputed != 0) {
            z = ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed;
            x = ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xAngularDecreaseSpeed;
            y = ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed;
            len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
            z = inv_MTH_fn_xSquareRoot(len2);
            if (z != 0.0f) {
                f32 xInvLength;
                xInvLength = 1.0f / z;
                _p_stStruct->stCptPos.stRefAxisZ.xX = xInvLength * x;
                _p_stStruct->stCptPos.stRefAxisZ.xY =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetSpeed;
                _p_stStruct->stCptPos.stRefAxisZ.xZ =
                    xInvLength * ((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)->xTargetIncreaseSpeed;
            } else {
                *(MTH3D_tdstVector_*) &_p_stStruct->stCptPos.stRefAxisZ =
                    *(MTH3D_tdstVector_*) &((tdstInternalStructurCineinfo_*) _p_stStruct->hCineinfo)
                         ->xAngularDecreaseSpeed;
            }
        } else {
            MTH3D_M_vSetXYZ_inv(&_p_stStruct->stCptPos.stRefAxisZ, 0.0f, 0.0f, 1.0f);
        }
        CAM_fn_vComputeRefAxis(_p_stStruct, &axis);
        xZero = 0.0f;
        _p_stStruct->stWantedCameraPos = _p_stStruct->stCurrentCameraPos;
        MTH3D_M_vSetXYZ_inv(&_p_stStruct->stCptPos.stMovePos, xZero, xZero, xZero);
        return;
    } else {
        MTH3D_M_vSubXYZ_inv(&offset, &_p_stStruct->stCurrentCameraPos,
                            &_p_stStruct->stTargetedPersoPos);
        if (offset.xX == _p_stStruct->stCptPos.stRefAxisZ.xX &&
            offset.xY == _p_stStruct->stCptPos.stRefAxisZ.xY) {
            offset.xX += 0.5f;
        }
        CAM_fn_vComputeRefAxis(_p_stStruct, &axis);
        CAM_fn_vComputeCameraAxisAngles(_p_stStruct, &axis, &offset, &first, &second);
        CAM_fn_vRotateCameraAxis(&axis, &_p_stStruct->stCptPos.stRefAxisZ, first, second);
        maxDelta = _p_stStruct->hCineinfoWork->xDistMin;
        len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(offset.xX, offset.xY, offset.xZ);
        speed = _p_stStruct->hCineinfoWork->xDistMax;
        len = inv_AI_fn_xClampedTargetSpeed(len2, maxDelta, speed);
        inv_MTH3D_fn_vKTimesXYZ(&axis, len, &axis);
        MTH3D_M_vAddXYZ_inv(&_p_stStruct->stWantedCameraPos, &_p_stStruct->stTargetedPersoPos,
                            &axis);
        CAM_fn_vTakeCareOfCameraAltitude(_p_stStruct->hCineinfo, &_p_stStruct->stWantedCameraPos);
    }
    MTH3D_M_vSubXYZ_inv(&_p_stStruct->stCptPos.stMovePos, &_p_stStruct->stWantedCameraPos,
                        &_p_stStruct->stCurrentCameraPos);
}

void CAM_fn_vComputeMovePosWithDynTheta(CAM_tdstUpdateCamera_* _p_stStruct)
{
    CAM_fn_vComputeMovePos(_p_stStruct);
    if ((_p_stStruct->hCineinfo->ucVolIAFlags & 2) != 0) {
        return;
    }
    if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted == NULL) {
        return;
    }
    if (cNoDynChangeTheta != 0) {
        return;
    }
    if (CAM_fn_cDynChangeTheta(_p_stStruct, &_p_stStruct->stWantedCameraPos) == 0) {
        return;
    }
    CAM_fn_vComputeMovePos(_p_stStruct);
}

void CAM_fn_vComputeSpeed(CAM_tdstUpdateCamera_* _p_stStruct)
{
    MTH3D_tdstVector_ stTempVector;
    MTH3D_tdstVector_ stTempVector1;
    MTH3D_tdstVector_ saved;
    f32 xValue;
    f32 speed;
    f32 zero;

    {
        tdstInternalStructurCineinfo_* cine;
        u16 flags;

        cine = _p_stStruct->hCineinfoWork;
        flags = cine->uwIAFlags;
        if ((flags & 0x80) != 0) {
            _p_stStruct->stCptPos.xLinearSpeed = cine->xLinearSpeed;
            _p_stStruct->stCptPos.xAngularSpeed = cine->xAngularSpeed;
            goto target_speed_simple;
        }
        if ((flags & 0x2000) != 0 || cine->hSuperObjectTargeted == NULL) {
            _p_stStruct->stCptPos.xLinearSpeed = cine->xLinearSpeed;
            _p_stStruct->stCptPos.xAngularSpeed = cine->xAngularSpeed;
            goto after_angular;
        }

        speed = 0.0f;
        if ((cine->uwDNMFlags & 1) == 0) {
            CAM_tdstCameraConstants_* c;

            c = &CAM_g_stCameraConstants;
            MTH3D_M_vSubXYZ_inv(&stTempVector, &_p_stStruct->stWantedCameraPos,
                                &_p_stStruct->stCurrentCameraPos);
            {
                f32 dot;
                dot = MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector.xX, stTempVector.xY, stTempVector.xZ);
                speed = MTH_fn_xSqrtOpt2(dot);
                speed *= c->xDynSpeed_LinearFactNorm;
            }
            if (c->xDynSpeed_LinearMax < speed) {
                speed = c->xDynSpeed_LinearMax;
            }
            CAM_fn_bPersoIsMoving(_p_stStruct, &stTempVector);
            {
                f32 candidate;
                candidate = inv_AI_fn_xScaledLength3(stTempVector.xX, stTempVector.xY, stTempVector.xZ, c->xDynSpeed_LinearMulPerso);
                if (speed < candidate) {
                    speed = candidate;
                }
            }
        }
    }

    {
        tdstInternalStructurCineinfo_* cine;
        cine = _p_stStruct->hCineinfoWork;
        _p_stStruct->stCptPos.xLinearSpeed = speed;
        if ((cine->uwDNMFlags & 4) != 0) {
            _p_stStruct->stCptPos.xAngularSpeed = speed;
            goto after_angular;
        }
    }

    MTH3D_M_vSubXYZ_inv(&stTempVector, &_p_stStruct->stTargetedPersoPos,
                        &_p_stStruct->stCurrentCameraPos);
    speed = 0.0f;
    if (MTH3D_M_bIsZeroXYZ_inv(&stTempVector)) {
        goto angle_scale;
    }

    if (!MTH3D_M_bIsZeroXYZ_inv(&stTempVector)) {
        f32 dot;
        f32 norm;
        dot = MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector.xX, stTempVector.xY, stTempVector.xZ);
        norm = MTH_fn_xSqrtOpt2(dot);
        if (norm != 0.0f) {
            f32 xInvLength;
            xInvLength = 1.0f / norm;
            inv_MTH3D_fn_vKTimesXYZ(&stTempVector, xInvLength, &stTempVector);
        }
    }

    MTH3D_M_vSubXYZ_inv(&stTempVector1, &_p_stStruct->stTargetedPersoPos,
                        &_p_stStruct->stWantedCameraPos);
    if (!MTH3D_M_bIsZeroXYZ_inv(&stTempVector1)) {
        f32 dot;
        f32 norm;
        dot = MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector1.xX, stTempVector1.xY, stTempVector1.xZ);
        norm = MTH_fn_xSqrtOpt2(dot);
        if (norm != 0.0f) {
            f32 xInvLength;
            xInvLength = 1.0f / norm;
            inv_MTH3D_fn_vKTimesXYZ(&stTempVector1, xInvLength, &stTempVector1);
        }
    }
    speed = CAM_fn_xComputeRotation(&stTempVector, &stTempVector1, 0);

angle_scale: {
    tdstInternalStructurCineinfo_* cine;
    CAM_tdstCameraConstants_* c;

    cine = _p_stStruct->hCineinfoWork;
    c = &CAM_g_stCameraConstants;
    speed *= c->xDynSpeed_AngularFactNorm;
    if (c->xDynSpeed_AngularMax < speed) {
        speed = c->xDynSpeed_AngularMax;
    }
    CAM_fn_bPersoIsTurning(cine->hSuperObjectTargeted, &xValue);
    {
        f32 scaled;
        scaled = xValue * c->xDynSpeed_AngularMulPerso;
        if (speed < scaled) {
            speed = xValue;
        }
    }
}

    {
        tdstInternalStructurCineinfo_* cine;
        f32 dx;
        f32 dy;
        f32 dz;
        f32 dot;
        f32 one;
        f32 look;

        dx = _p_stStruct->stTargetedPersoPos.xX - _p_stStruct->stCurrentCameraPos.xX;
        dy = _p_stStruct->stTargetedPersoPos.xY - _p_stStruct->stCurrentCameraPos.xY;
        dz = _p_stStruct->stTargetedPersoPos.xZ - _p_stStruct->stCurrentCameraPos.xZ;
        cine = _p_stStruct->hCineinfoWork;
        one = 1.0f;
        look = cine->xDistMin;
        dot = MTH_fn_xSqrAddSqrAddSqrOpt2(dx, dy, dz);
        dot = MTH_fn_xSqrtOpt2(dot);
        stTempVector.xX = dx;
        xValue = look - dot;
        stTempVector.xY = dy;
        stTempVector.xZ = dz;

        if (one < xValue) {
            speed /= xValue;
            _p_stStruct->stCptPos.xAngularSpeed = speed;
        } else if (xValue < -one) {
            speed *= __builtin_fabsf(xValue);
            _p_stStruct->stCptPos.xAngularSpeed = speed;
        } else {
            _p_stStruct->stCptPos.xAngularSpeed = speed;
        }
    }

after_angular: {
    tdstInternalStructurCineinfo_* cine;
    cine = _p_stStruct->hCineinfoWork;
    if ((cine->uwDNMFlags & 0x10000) != 0 || cine->hSuperObjectTargeted == NULL) {
        _p_stStruct->stCptPos.xTargetSpeed = cine->xTargetSpeed;
        return;
    }
}

    zero = 0.0f;
    MTH3D_M_vSetXYZ_inv(&saved, zero, zero, zero);
    speed = zero;
    {
        tdstInternalStructurCineinfo_* cine;
        cine = _p_stStruct->hCineinfoWork;
        if ((cine->uwDNMFlags & 0x10) == 0) {
            MTH3D_tdstVector_* pos;

            pos = &inv_HIE_fn_p_stGetEngineObject(_p_stStruct->hSuperObjCamera)->h_Dynam->stParsingDatas.stVector;
            stTempVector = *pos;
            MTH3D_M_vSubXYZ_inv(&stTempVector, &_p_stStruct->stCptPos.stTarget, &stTempVector);
            {
                f32 dot;
                dot = MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector.xX, stTempVector.xY, stTempVector.xZ);
                speed = MTH_fn_xSqrtOpt2(dot);
            }
            saved = stTempVector;
        }
    }
    {
        CAM_tdstCameraConstants_* c;
        c = &CAM_g_stCameraConstants;
        speed *= c->xDynSpeed_TargetFactNorm;
        if (c->xDynSpeed_TargetMax < speed) {
            speed = c->xDynSpeed_TargetMax;
        }
        CAM_fn_bPersoIsMoving(_p_stStruct, &stTempVector);
        {
            f32 candidate;
            candidate = inv_AI_fn_xScaledLength3(stTempVector.xX, stTempVector.xY, stTempVector.xZ, c->xDynSpeed_TargetMulPerso);
            if (speed < candidate) {
                speed = candidate;
            }
        }
    }
    _p_stStruct->stCptPos.xTargetSpeed = speed;

    if (MTH3D_M_bIsZeroXYZ_inv(&saved)) {
        return;
    }
    {
        if (MTH3D_M_bIsZeroXYZ_inv(&_p_stStruct->p_stReport->m_stAbsolutePrevPosition.m_stLinear)) {
            return;
        }
        {
            f32 dot;
            dot = inv_MTH_fn_xMulAdd3(
                saved.xX, _p_stStruct->p_stReport->m_stAbsolutePrevPosition.m_stLinear.xX, saved.xY,
                _p_stStruct->p_stReport->m_stAbsolutePrevPosition.m_stLinear.xY, saved.xZ,
                _p_stStruct->p_stReport->m_stAbsolutePrevPosition.m_stLinear.xZ);
            if (dot < -0.9f) {
                _p_stStruct->hCineinfoWork->xTargetDecreaseSpeed = 40.0f;
                _p_stStruct->hCineinfoWork->xTargetIncreaseSpeed = 40.0f;
            }
        }
    }
    return;

target_speed_simple:
    _p_stStruct->stCptPos.xTargetSpeed = _p_stStruct->hCineinfoWork->xTargetSpeed;
}

void CAM_fn_vConstraintMoveAfterVisFailure(CAM_tdstUpdateCamera_* _p_stStruct)
{
    MTH3D_tdstVector_ stTempVector1;
    MTH3D_tdstVector_ stTempVector;
    if (_p_stStruct->cTgtPersoIsMovingRelative != 0 && (inv_CAM_wFlags[0] & 8) == 0) {
        MTH3D_M_vSubXYZ_inv(&stTempVector1, &_p_stStruct->stWantedCameraPos,
                            &_p_stStruct->hCineinfo->stLastFailedWanted);

        MTH3D_M_vSubXYZ_inv(&stTempVector, &_p_stStruct->stTargetedPersoPos,
                            &_p_stStruct->hCineinfo->stLastFailedWanted);

        if (!MTH3D_M_bIsZeroXYZ_inv(&stTempVector1) &&
            !MTH3D_M_bIsZeroXYZ_inv(&stTempVector)) {
            f32 lengthSquared;
            f32 length;
            f32 inv;
            f32 dot;

            lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(stTempVector.xX, stTempVector.xY, stTempVector.xZ);
            length = MTH_fn_xSqrtOpt2(lengthSquared);
            if (length != 0.0f) {
                inv = 1.0f / length;
                inv_MTH3D_fn_vKTimesXYZ(&stTempVector, inv, &stTempVector);
            }

            dot = inv_MTH_fn_xMulAdd3Grouped(stTempVector1.xX, stTempVector1.xY, stTempVector1.xZ,
                                   stTempVector.xX, stTempVector.xY, stTempVector.xZ);

            inv_MTH3D_fn_vKTimesXYZ(&stTempVector1, dot, &stTempVector);
            MTH3D_M_vAddXYZ_inv(&stTempVector1, &stTempVector1,
                                &_p_stStruct->hCineinfo->stLastFailedWanted);
        } else {
            stTempVector1 = _p_stStruct->hCineinfo->stLastFailedWanted;
        }
    } else {
        stTempVector1 = _p_stStruct->hCineinfo->stLastFailedWanted;
    }

    MTH3D_M_vSubXYZ_inv(&_p_stStruct->stCptPos.stMovePos, &stTempVector1,
                        &_p_stStruct->stCurrentCameraPos);

    _p_stStruct->stWantedCameraPos = stTempVector1;
    CAM_fn_vComputeCineinfoCurrentForAPosition(_p_stStruct, &stTempVector1);
    _p_stStruct->hCineinfo->stLastFailedWanted = _p_stStruct->stWantedCameraPos;
}

s32 CAM_fn_cConstraintMoveCutTestAngle(CAM_tdstUpdateCamera_* _p_stStruct, f32 _xAngle)
{
    tdstInternalStructurCineinfo_ work;
    CAM_tdstUpdateCamera_ local;
    MTH3D_tdstVector_ stToCurrent;
    MTH3D_tdstVector_ stToWanted;
    tdstCineinfo_* cine;
    f32 len0;
    f32 len1;
    s32 mode;

    work = *_p_stStruct->hCineinfoWork;
    work.xAngleAlpha = _xAngle;

    CAM_fn_vInitCameraStructure(_p_stStruct->hSuperObjCamera, &local);
    local.hCineinfoWork = &work;
    CAM_fn_vComputeReferencePoint(&local);
    CAM_fn_vComputeMovePosWithDynTheta(&local);

    if (cJustBetterPos != 0) {
        cine = _p_stStruct->hCineinfo;
        MTH3D_M_vSubXYZ_inv(&stToCurrent, &cine->stLastFailedOrigin,
                            &_p_stStruct->stCurrentCameraPos);
        len0 = MTH_fn_xSqrAddSqrAddSqrOpt2(stToCurrent.xX, stToCurrent.xY, stToCurrent.xZ);
        len0 = inv_MTH_fn_xSquareRoot(len0);

        MTH3D_M_vSubXYZ_inv(&stToWanted, &cine->stLastFailedOrigin, &local.stWantedCameraPos);
        len1 = MTH_fn_xSqrAddSqrAddSqrOpt2(stToWanted.xX, stToWanted.xY, stToWanted.xZ);
        len1 = inv_MTH_fn_xSquareRoot(len1);

        if (len1 < len0) {
            return 0;
        }
    } else {
        cine = _p_stStruct->hCineinfo;
    }
    mode = 0;
    if ((*(u32*) &cine->eState & 0x400FF) == 0x40001) {
        mode = 3;
        if (_p_stStruct->hCineinfoWork->uwIAFlags & 8) {
            mode = 2;
        }
    }

    if (CAM_fn_cIsWantedPosIncorrect(&local, &local.stWantedCameraPos, mode) != 0) {
        return 0;
    }

    _p_stStruct->stWantedCameraPos = local.stWantedCameraPos;
    _p_stStruct->stCptPos.stMovePos = local.stCptPos.stMovePos;
    return 1;
}

f32 CAM_fn_xOppositeAngle(CAM_tdstUpdateCamera_* update)
{
    f32 value;

    value = inv_CAM_xBaseCutAngle[0] + inv_CAM_xBaseCutAngle[0];
    if (update->xAngleDeltaAlphaCut > 0.0f) {
        value = -value;
    }
    return CAM_fn_xSetAngleInInterval02Pi(value + update->xAngleAlphaCut);
}

void CAM_fn_vConstraintMoveCutAngles(CAM_tdstUpdateCamera_* _p_stStruct)
{
    tdstCineinfo_* cine;
    f32 xLastDeltaCut;
    f32 angle;

    if (_p_stStruct->hCineinfo->cLastCutAngleSens == -2) {
        return;
    }

    if ((((u8*) _p_stStruct->p_stReport)[0x168] & 1) && !(inv_CAM_wFlags[0] & 1)) {
        _p_stStruct->hCineinfo->cLastCutAngleSens = -2;
        return;
    }

    cine = _p_stStruct->hCineinfo;
    xLastDeltaCut = cine->xLastDeltaCut;

    if (xLastDeltaCut * _p_stStruct->xAngleDeltaAlphaCut < 0.0f) {
        if (fabsf(_p_stStruct->xAngleDeltaAlphaCut) < (MTH_PI_inv - inv_CAM_xBaseCutAngle[0])) {
            _p_stStruct->hCineinfo->cLastCutAngleSens = 0;
        }
    }

    if (_p_stStruct->hCineinfo->cLastCutAngleSens == 0) {
        _p_stStruct->hCineinfo->xLastDeltaCut = _p_stStruct->xAngleDeltaAlphaCut;
    }

    angle = _p_stStruct->xAngleAlphaCut;
    if (_p_stStruct->hCineinfo->cLastCutAngleSens == -1) {
        if (0.0f < _p_stStruct->xAngleDeltaAlphaCut) {
            angle = CAM_fn_xOppositeAngle(_p_stStruct);
        }
    }

    if (_p_stStruct->hCineinfo->cLastCutAngleSens == 1) {
        if (_p_stStruct->xAngleDeltaAlphaCut < 0.0f) {
            angle = CAM_fn_xOppositeAngle(_p_stStruct);
        }
    }

    if (CAM_fn_cConstraintMoveCutTestAngle(_p_stStruct, angle)) {
        if (_p_stStruct->hCineinfo->cLastCutAngleSens != 0) {
            return;
        }
        if (0.0f < _p_stStruct->xAngleDeltaAlphaCut) {
            _p_stStruct->hCineinfo->cLastCutAngleSens = 1;
        } else {
            _p_stStruct->hCineinfo->cLastCutAngleSens = -1;
        }
        return;
    }

    if (_p_stStruct->hCineinfo->cLastCutAngleSens == 0) {
        if ((MTH_PI_inv - inv_CAM_xBaseCutAngle[0]) <= fabsf(_p_stStruct->xAngleDeltaAlphaCut)) {
            angle = CAM_fn_xOppositeAngle(_p_stStruct);
            if (CAM_fn_cConstraintMoveCutTestAngle(_p_stStruct, angle)) {
                if (0.0f < _p_stStruct->xAngleDeltaAlphaCut) {
                    _p_stStruct->hCineinfo->cLastCutAngleSens = -1;
                } else {
                    _p_stStruct->hCineinfo->cLastCutAngleSens = 1;
                }
                return;
            }
        }
    }

    _p_stStruct->hCineinfo->cLastCutAngleSens = -2;
}

void func_00205DE0(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* wanted)
{
}

void CAM_fn_vConstraintMove(CAM_tdstUpdateCamera_* _p_stStruct)
{
    MTH3D_tdstVector_* wanted;

    if (_p_stStruct->hCineinfo->ucVolIAFlags & 2)
        return;
    if (_p_stStruct->hCineinfoWork->uwIAFlags & 0x2000)
        return;
    if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted == NULL)
        return;
    wanted = &_p_stStruct->stWantedCameraPos;
    if (CAM_fn_cCanResetConstraintMoveVisFailure(_p_stStruct, wanted))
        _p_stStruct->hCineinfo->ucPerIAFlags &= 0xF7;
    if (_p_stStruct->hCineinfo->ucPerIAFlags & 8) {
        CAM_fn_vConstraintMoveAfterVisFailure(_p_stStruct);
        return;
    }
    if (_p_stStruct->cHasCutAlpha != 0) {
        if (_p_stStruct->hCineinfoWork->uwIAFlags & 4) {
            _p_stStruct->hCineinfo->cLastCutAngleSens = 0;
        } else {
            CAM_fn_vConstraintMoveCutAngles(_p_stStruct);
        }
    } else {
        _p_stStruct->hCineinfo->cLastCutAngleSens = 0;
    }
    if (_p_stStruct->hCineinfo->eState == CAM_e_State_GoToComputedOptimal)
        func_00205DE0(_p_stStruct, wanted);
}

void CAM_fn_vTakeCareOfFather(CAM_tdstUpdateCamera_* update)
{
    CAM_fn_hGetFather(update->hCineinfoWork->hSuperObjectTargeted);
}

void CAM_fn_vUpdateGeneralCamera(CAM_tdstUpdateCamera_* _p_stStruct)
{
    __asm__(".p2align 2" : "=m"(cCanTestStatic) : : "memory");
restart:
    {
        MTH3D_tdstVector_ v;
        tdstCineinfo_* cine;
        f32 len;
        f32 vx;
        f32 xThreshold;
        f32 cur;
        u32 dnm;

        cCanTestStatic = 1;
        CAM_fn_vTakeCareOfFather(_p_stStruct);

        v = *(MTH3D_tdstVector_*) &_p_stStruct->p_stReport->m_stAbsolutePrevSpeed.m_stLinear;
        vx = v.xX;
        len = inv_AI_fn_xCameraLength3(v.xY, v.xZ, vx);
        if (len < CAM_g_stCameraConstants.xMinLinearSpeedCamera) {
            _p_stStruct->hCineinfo->ucVolIAFlags |= 4;
            cine = _p_stStruct->hCineinfo;
            len = inv_fn_xGetEngineClockScale();
            xThreshold = (f32) CAM_g_stCameraConstants.xTickFindBetterPosIfNoMove;
            xThreshold *= 1000.0f;
            xThreshold *= len;
            cur = cine->xCounter4;
            if (cur < xThreshold) {
                _p_stStruct->hCineinfo->xCounter4 =
                    _p_stStruct->hCineinfo->xCounter4 + inv_fn_xGetEngineClockDelta() * 1000.0f;
            }
        } else {
            _p_stStruct->hCineinfo->xCounter4 = 0.0f;
        }

        if (_p_stStruct->hCineinfoWork->hSuperObjectTargeted != 0 &&
            _p_stStruct->hCineinfoWork->hSecondSuperObjectTargeted != 0) {
            _p_stStruct->hCineinfo->ucPerIAFlags &= 0xF7;
            _p_stStruct->hCineinfoWork->uwIAFlags |= 8;
            _p_stStruct->hCineinfo->eState = 5;
        }

        cRefAxisIsAlreadyComputed = (_p_stStruct->hCineinfoWork->uwIAFlags >> 14) & 1;
        cNoDynChangeTheta = (_p_stStruct->hCineinfoWork->uwIAFlags >> 5) & 1;
        CAM_fn_vComputeReferencePoint(_p_stStruct);
        CAM_fn_vComputeTarget(_p_stStruct);
        CAM_fn_vComputeRealWantedPos(_p_stStruct);
        CAM_fn_vComputeMovePos(_p_stStruct);

        if (!(_p_stStruct->hCineinfo->ucVolIAFlags & 2) && _p_stStruct->hCineinfoWork->hSuperObjectTargeted != 0 &&
            cNoDynChangeTheta == 0 && !(_p_stStruct->hCineinfo->ucPerIAFlags & 8) &&
            !(_p_stStruct->hCineinfoWork->uwIAFlags & 0x2000)) {
            if (CAM_fn_cDynChangeTheta(_p_stStruct, &_p_stStruct->stWantedCameraPos)) {
                CAM_fn_vComputeMovePos(_p_stStruct);
            }
        }

        if (_p_stStruct->hCineinfo->ucPerIAFlags & 4 ||
            (_p_stStruct->hCineinfoWork->uwIAFlags & 0x40)) {
            dnm = *(u32*) &_p_stStruct->hCineinfoWork->uwDNMFlags;
            if ((dnm & 0x30000000) || (_p_stStruct->hCineinfo->ucVolIAFlags & 2) ||
                CAM_fn_bEqualVectorWithEpsilon(&_p_stStruct->stCurrentCameraPos,
                                               &_p_stStruct->stWantedCameraPos, CAM_g_stCameraConstants.xEpsilonForEqualVectors)) {
                _p_stStruct->hCineinfoCurrent->uwIAFlags &= 0xFFBF;
                _p_stStruct->hCineinfoWork->uwIAFlags &= 0xFFBF;
                _p_stStruct->hCineinfo->ucPerIAFlags &= 0xFB;
                _p_stStruct->hCineinfo->cLastCutAngleSens = 0;
            }
        }

        if (CAM_g_stCameraConstants.wFlags & 0x10) {
            if (_p_stStruct->hCineinfo->eState == 1 && _p_stStruct->cTgtPersoIsMovingRelative != 0) {
                if (CAM_fn_bEqualVectorWithEpsilon(&_p_stStruct->stCurrentCameraPos,
                                                   &_p_stStruct->stWantedCameraPos, CAM_g_stCameraConstants.xEpsilonForEqualVectors)) {
                    if (_p_stStruct->hCineinfo->ucPerIAFlags & 2) {
                        CAM_fn_vSetCineinfoCurrentFromVisibility(_p_stStruct->hCineinfo);
                    }
                    _p_stStruct->hCineinfo->ucPerIAFlags &= 0xF7;
                    _p_stStruct->hCineinfo->ucVolIAFlags &= 0xFE;
                    _p_stStruct->hCineinfo->eState = 0;
                    goto restart;
                }
            }
        }

        switch (_p_stStruct->hCineinfo->eState) {
        case 0:
        case 1:
            CAM_fn_vTestBadVisibility(_p_stStruct);
            CAM_fn_vTestBadDistance(_p_stStruct);
            CAM_fn_vTestOppositeCollision(_p_stStruct);
            break;
        }

        switch (_p_stStruct->hCineinfo->eState) {
        case 1:
            CAM_fn_vCheckForABetterPos(_p_stStruct);

        case 0:
            CAM_fn_vConstraintMove(_p_stStruct);
            break;
        case 2:
            CAM_fn_vComputeFailureVisibility(_p_stStruct);
            break;
        case 3:
            break;
        case 4:
        case 5:
            _p_stStruct->hCineinfo->eState = 0;
            break;
        }

        {
            tdstCineinfo_* endCine;
            CAM_fn_vComputeSpeed(_p_stStruct);
            if (((*(u32*) &_p_stStruct->hCineinfoWork->uwDNMFlags & 0x30000000) ||
                 (_p_stStruct->hCineinfo->ucVolIAFlags & 2) || _p_stStruct->hCineinfo->eState == 0)) {
                f32 zero;
                tdstCineinfo_* p2;
                tdstCineinfo_* p3;
                tdstCineinfo_* p4;
                tdstCineinfo_* p5;
                endCine = _p_stStruct->hCineinfo;
                zero = 0.0f;
                MTH3D_M_vSetXYZ_inv(&endCine->stLastFailedOrigin, zero, zero, zero);
                p2 = _p_stStruct->hCineinfo;
                MTH3D_M_vSetXYZ_inv(&p2->stLastFailedWanted, zero, zero, zero);
                p3 = _p_stStruct->hCineinfo;
                p3->xCounter1 = zero;
                p4 = _p_stStruct->hCineinfo;
                p4->xCounter3 = zero;
                p5 = _p_stStruct->hCineinfo;
                p5->ucPerIAFlags &= 0xF7;
            }
        }
        cJustBetterPos = 0;
        return;
    }
}
