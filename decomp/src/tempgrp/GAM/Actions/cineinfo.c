#include "common.h"

#include "CAM/CAM.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"

#include "ps2/libc.h"
#include "AI/AI.h"

#define CINEINFO_INITIAL_TRANSPARENCE 128.0f
#define CINEINFO_HISTORY_WEIGHT 0.03f

extern const f32 inv_CAM_xInitZMin[];
extern const f32 inv_CAM_xInitZMax[];

void* CAM_fn_hCineinfoInternalRealAlloc(void);
void CAM_fn_vSetCameraInList(HIE_tdstSuperObject_* superObject);
void fn_vInitAveragePositionDatasIfNecessary(tdstCineinfo_* cineinfo);

void CAM_fn_vLoadCameraConstants(void)
{
    CAM_g_stCameraConstants.xRayCameraSphereToGo1 = 0.35f;
    CAM_g_stCameraConstants.xRayCameraSphereToGo2 = 0.35f;
    CAM_g_stCameraConstants.xRayCameraSphereToSee1 = 0.1f;
    CAM_g_stCameraConstants.xRayCameraSphereToSee2 = 0.1f;
    CAM_g_stCameraConstants.xBaseAngleComputePosNormal = MTH_QUARTER_PI_inv;
    CAM_g_stCameraConstants.xBaseAngleComputePosFailure = MTH_QUARTER_PI_inv;
    CAM_g_stCameraConstants.xBaseCutAngle = 0.3926991f;
    CAM_g_stCameraConstants.xAverageForComputePos = 0.4f;
    CAM_g_stCameraConstants.xAverageForComputeLinearSpeed = 0.4f;
    CAM_g_stCameraConstants.xMinLinearSpeedCamera = 0.005f;
    CAM_g_stCameraConstants.xMinLinearSpeedTgtPerso = 0.01f;
    CAM_g_stCameraConstants.xMinAngularSpeedTgtPerso = 0.01f;
    CAM_g_stCameraConstants.xEpsilonDynTgtAngle = MTH_QUARTER_PI_inv;
    CAM_g_stCameraConstants.xEpsilonDynTgtFocalAngle = 0.19634954f;
    CAM_g_stCameraConstants.xEpsilonForEqualVectors = 0.5f;
    CAM_g_stCameraConstants.xOffsetMinDistPerso = 0.05f;
    CAM_g_stCameraConstants.xDistanceForNoCut = 5.0f;
    CAM_g_stCameraConstants.xNumRayCameraCanGo = 3;
    CAM_g_stCameraConstants.xNumRayCameraCanSee = 1;
    CAM_g_stCameraConstants.xTickTestVisibility = 4;
    CAM_g_stCameraConstants.xTickFindBetterPos = 20;
    CAM_g_stCameraConstants.xTickFindBetterPosIfNoMove = 60;
    CAM_g_stCameraConstants.xTickFailureCantSee = 60;
    CAM_g_stCameraConstants.xDynSpeed_LinearFactNorm = 3.0f;
    CAM_g_stCameraConstants.xDynSpeed_LinearMax = 10.0f;
    CAM_g_stCameraConstants.xDynSpeed_LinearMulPerso = 1.5f;
    CAM_g_stCameraConstants.xDynSpeed_AngularFactNorm = 3.0f;
    CAM_g_stCameraConstants.xDynSpeed_AngularMax = 3.0f;
    CAM_g_stCameraConstants.xDynSpeed_AngularMulPerso = 0.0f;
    CAM_g_stCameraConstants.xDynSpeed_TargetFactNorm = 4.0f;
    CAM_g_stCameraConstants.xDynSpeed_TargetMax = 10.0f;
    CAM_g_stCameraConstants.xDynSpeed_TargetMulPerso = 1.0f;
    CAM_g_stCameraConstants.xCutAngleFactorPos = 1.0f;
    CAM_g_stCameraConstants.xCutAngleFactorTgt = 1.0f;
    CAM_g_stCameraConstants.wFlags = 0;
}

void* CAM_fn_hCineinfoRealAlloc(void)
{
    return AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstCineinfo_));
}

void CAM_fn_vCineinfoAlloc(tdstEngineObject_* _p_stObject)
{
    f32 zero;

    _p_stObject->h_Cineinfo = CAM_fn_hCineinfoRealAlloc();
    _p_stObject->h_Cineinfo->hInit = CAM_fn_hCineinfoInternalRealAlloc();
    _p_stObject->h_Cineinfo->hCurrent = CAM_fn_hCineinfoInternalRealAlloc();
    _p_stObject->h_Cineinfo->hVisibility = CAM_fn_hCineinfoInternalRealAlloc();
    _p_stObject->h_Cineinfo->hWork = CAM_fn_hCineinfoInternalRealAlloc();

    zero = 0.0f;
    _p_stObject->h_Cineinfo->hInit->stShiftTarget.xZ = _p_stObject->h_Cineinfo->hInit->stShiftTarget.xY =
        _p_stObject->h_Cineinfo->hInit->stShiftTarget.xX = zero;
    _p_stObject->h_Cineinfo->hInit->stShiftPos.xZ = _p_stObject->h_Cineinfo->hInit->stShiftPos.xY =
        _p_stObject->h_Cineinfo->hInit->stShiftPos.xX = zero;
    _p_stObject->h_Cineinfo->hInit->xDistMin = 6.0f;
    _p_stObject->h_Cineinfo->hInit->xDistMax = 6.0f;
    _p_stObject->h_Cineinfo->hInit->xBoundDistMin = 2.0f;
    _p_stObject->h_Cineinfo->hInit->xBoundDistMax = 10.0f;
    _p_stObject->h_Cineinfo->hInit->xAngleAlpha = zero;
    _p_stObject->h_Cineinfo->hInit->xAngleShiftAlpha = zero;
    _p_stObject->h_Cineinfo->hInit->xAngleTheta = zero;
    _p_stObject->h_Cineinfo->hInit->xAngleShiftTheta = zero;
    _p_stObject->h_Cineinfo->hInit->xLinearSpeed = 200.0f;
    _p_stObject->h_Cineinfo->hInit->xLinearIncreaseSpeed = 1.0f;
    _p_stObject->h_Cineinfo->hInit->xLinearDecreaseSpeed = zero;
    _p_stObject->h_Cineinfo->hInit->xAngularSpeed = 3.14f;
    _p_stObject->h_Cineinfo->hInit->xAngularIncreaseSpeed = 1.0f;
    _p_stObject->h_Cineinfo->hInit->xAngularDecreaseSpeed = zero;
    _p_stObject->h_Cineinfo->hInit->xTargetSpeed = 200.0f;
    _p_stObject->h_Cineinfo->hInit->xTargetIncreaseSpeed = 1.0f;
    _p_stObject->h_Cineinfo->hInit->xTargetDecreaseSpeed = zero;
    _p_stObject->h_Cineinfo->hInit->xFocal = 1.5f;
    _p_stObject->h_Cineinfo->hInit->xZMin = inv_CAM_xInitZMin[0];
    _p_stObject->h_Cineinfo->hInit->xZMax = inv_CAM_xInitZMax[0];
    _p_stObject->h_Cineinfo->hInit->hSuperObjectTargeted = NULL;
    _p_stObject->h_Cineinfo->hInit->hSecondSuperObjectTargeted = NULL;
    _p_stObject->h_Cineinfo->hInit->uwDNMFlags = 0;
    _p_stObject->h_Cineinfo->hInit->uwIAFlags = 0;
    _p_stObject->h_Cineinfo->hInit->eTypeOfViewport = CAM_e_NoViewport;
    _p_stObject->h_Cineinfo->hInit->cChannel = -1;
    _p_stObject->h_Cineinfo->hInit->bIsActive = 0;

    inv_CAM_fn_vInitTransparentSuperObjectArray(_p_stObject->h_Cineinfo);
    _p_stObject->h_Cineinfo->fTransparency = CINEINFO_INITIAL_TRANSPARENCE;
    _p_stObject->h_Cineinfo->fTransparencySpeed = 1.0f;
}

void* CAM_fn_hCineinfoInternalRealAlloc(void)
{
    return AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstInternalStructurCineinfo_));
}

void CAM_fn_vInitCompleteCineinfo(tdstCineinfo_* _hCineinfo)
{
    f32 zero = 0.0f;

    _hCineinfo->eState = CAM_e_State_GoToOptimal;
    _hCineinfo->ucVolIAFlags = 0;
    _hCineinfo->ucPerIAFlags = 0;
    _hCineinfo->cLastCutAngleSens = 0;
    MTH3D_M_vSetXYZ_inv(&_hCineinfo->stLastFailedOrigin, zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&_hCineinfo->stLastFailedWanted, zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&_hCineinfo->stForceTarget, zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&_hCineinfo->stForcePosition, zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&_hCineinfo->stForceRefAxisY, zero, zero, zero);
    MTH3D_M_vSetXYZ_inv(&_hCineinfo->stForceRefAxisZ, zero, zero, zero);
    _hCineinfo->xApexRatio = zero;
    _hCineinfo->xLastDeltaCut = zero;
    _hCineinfo->xCounter1 = zero;
    _hCineinfo->xCounter2 = zero;
    _hCineinfo->xCounter3 = zero;
    _hCineinfo->xCounter4 = zero;
    _hCineinfo->bCanDoBestPos = 1;

    CAM_fn_vUpdateTargetPosition(_hCineinfo);
    inv_CAM_hPreviousTarget = 0;
    inv_CAM_fn_vInitTransparentSuperObjectArray(_hCineinfo);
    *_hCineinfo->hCurrent = *_hCineinfo->hInit;
    _hCineinfo->ucVolIAFlags |= 2;
}

void CAM_fn_vUpdateTargetPosition(tdstCineinfo_* _hCineinfo)
{
    MTH3D_tdstVector_ Axis;

    if (_hCineinfo->hWork->hSuperObjectTargeted != NULL) {
        memset(_hCineinfo->AveragePosition.a_CheckedSpeed, 0, sizeof(_hCineinfo->AveragePosition.a_CheckedSpeed));
        memset(_hCineinfo->AveragePosition.a_CheckedPos, 0, sizeof(_hCineinfo->AveragePosition.a_CheckedPos));
        memset(_hCineinfo->AveragePosition.a_CheckedTime, 0, sizeof(_hCineinfo->AveragePosition.a_CheckedTime));
        MTH3D_M_vSetXYZ_inv(&Axis, 0.0f, -1.0f, 0.0f);
        _hCineinfo->stSightAxisSystem.SightAxis = Axis;
        MTH3D_M_vSetXYZ_inv(&Axis, -1.0f, 0.0f, 0.0f);
        _hCineinfo->stSightAxisSystem.FirstComplementaryAxis = Axis;
        MTH3D_M_vSetXYZ_inv(&Axis, 0.0f, 0.0f, -1.0f);
        _hCineinfo->stSightAxisSystem.SecondComplementaryAxis = Axis;
    }
}

void CAM_fn_vCineinfoInit(tdstEngineObject_* _p_stObject, u8 eObjectInit)
{
    tdstCineinfo_* cineinfo = _p_stObject->h_Cineinfo;
    HIE_tdstSuperObject_* target;

    switch (eObjectInit) {
    case OTI_WhenGameStart:
        break;
    case OTI_MapLoaded:
        CAM_fn_vSetCameraInList(_p_stObject->h_StandardGame->h_SuperObject);
        target = g_stEngineStructure.activeSuperObject;
        cineinfo->hInit->hSuperObjectTargeted = target;
        CAM_fn_vInitCompleteCineinfo(cineinfo);
        break;
    case OTI_ReinitTheMap:
    case OTI_LoadSavedGame:
    case 4:
    case OTI_AlwaysCreated:
        target = g_stEngineStructure.activeSuperObject;
        cineinfo->hInit->hSuperObjectTargeted = target;
        CAM_fn_vInitCompleteCineinfo(cineinfo);
        break;
    }
}

void func_00107828(void)
{
}

void func_00107830(tdstEngineObject_* destination, tdstEngineObject_* source)
{
    *destination->h_Cineinfo->hInit = *source->h_Cineinfo->hInit;
}

void CAM_fn_vSetCineinfoWorkFromCurrent(tdstCineinfo_* _hCineinfo)
{
    if (_hCineinfo->ucVolIAFlags & 1) {
        return;
    }
    _hCineinfo->ucVolIAFlags |= 1;
    *_hCineinfo->hWork = *_hCineinfo->hCurrent;
}

void CAM_fn_vSetCineinfoCurrentFromVisibility(tdstCineinfo_* _hCineinfo)
{
    *_hCineinfo->hCurrent = *_hCineinfo->hVisibility;
    _hCineinfo->ucPerIAFlags &= ~2;
}

void CAM_fn_vSetCameraInList(HIE_tdstSuperObject_* _hSupObjectCamera)
{
    tdstNodeCamera_* p_stTempNodeCamera = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstNodeCamera_));

    p_stTempNodeCamera->hNextBrotherDyn = NULL;
    p_stTempNodeCamera->hPrevBrotherDyn = NULL;
    p_stTempNodeCamera->hFatherDyn = NULL;
    p_stTempNodeCamera->p_stSuperObjectCamera = _hSupObjectCamera;
    LST2_M_vAddElementToList(&g_stEngineStructure.hCameraList, p_stTempNodeCamera);
}

void fn_vInitAveragePositionDatasIfNecessary(tdstCineinfo_* _hCineinfo)
{
    u32 i;

    for (i = 0; i < CAMERA_AVERAGE_HISTORY_COUNT; i++) {
        if (_hCineinfo->AveragePosition.a_CheckedTime[i] != 0.0f) {
            break;
        }
    }
    if (i < CAMERA_AVERAGE_HISTORY_COUNT) {
        return;
    }
    {
        MTH3D_tdstVector_* speed = _hCineinfo->AveragePosition.a_CheckedSpeed;
        f32* weight = _hCineinfo->AveragePosition.a_CheckedTime;
        POS_tdstMatrix_* matrix = _hCineinfo->hWork->hSuperObjectTargeted->gu_hGlobalMatrix;
        MTH3D_tdstVector_* translation = inv_POS_fn_p_stPosMatrixTranslation(matrix);
        f32 initialWeight = CINEINFO_HISTORY_WEIGHT;

        for (i = 0; i < CAMERA_AVERAGE_HISTORY_COUNT; i++) {
            *weight = initialWeight;
            weight++;
            speed->xZ = speed->xY = speed->xX = 0.0f;
            speed[CAMERA_AVERAGE_HISTORY_COUNT] = *translation;
            speed++;
        }
    }
}

void CAM_fn_vAverageSpeed(tdstCineinfo_* _hCineinfo, MTH3D_tdstVector_* p_result, f32 limit)
{
    f32* weights;
    MTH3D_tdstVector_* p_stTmp;
    f32 tmpTime;
    f32 xCurrentTime;
    u32 i;

    fn_vInitAveragePositionDatasIfNecessary(_hCineinfo);
    tmpTime = 0.0f;
    i = 0;
    p_stTmp = _hCineinfo->AveragePosition.a_CheckedSpeed;
    weights = _hCineinfo->AveragePosition.a_CheckedTime;
    MTH3D_M_vZeroXYZ_inv(p_result);
    do {
        xCurrentTime = *weights;
        if (xCurrentTime != 0.0f && (!MTH3D_M_bIsZeroXYZ_inv(p_stTmp))) {
            tmpTime += xCurrentTime;
            MTH3D_M_vSetXYZ_inv(p_result, xCurrentTime * p_stTmp->xX + p_result->xX,
                                xCurrentTime * p_stTmp->xY + p_result->xY,
                                xCurrentTime * p_stTmp->xZ + p_result->xZ);
            if (limit <= tmpTime) {
                break;
            }
        }
        i++;
        p_stTmp++;
        weights++;
    } while (i < CAMERA_AVERAGE_HISTORY_COUNT);

    if (tmpTime != 0.0f) {
        MTH3D_M_vSetXYZ_inv(p_result, p_result->xX / tmpTime, p_result->xY / tmpTime,
                            p_result->xZ / tmpTime);
    }
}

void CAM_fn_vAveragePosition(tdstCineinfo_* _hCineinfo, MTH3D_tdstVector_* p_result, f32 limit)
{
    f32* weights;
    MTH3D_tdstVector_* sample;
    f32 xSum;
    f32 xTime;
    u32 i;

    fn_vInitAveragePositionDatasIfNecessary(_hCineinfo);
    xSum = 0.0f;
    i = 0;
    sample = _hCineinfo->AveragePosition.a_CheckedPos;
    weights = _hCineinfo->AveragePosition.a_CheckedTime;
    MTH3D_M_vZeroXYZ_inv(p_result);
    do {
        xTime = *weights;
        if (xTime != 0.0f && (!MTH3D_M_bIsZeroXYZ_inv(sample))) {
            xSum += xTime;
            MTH3D_M_vSetXYZ_inv(p_result, xTime * sample->xX + p_result->xX,
                                xTime * sample->xY + p_result->xY,
                                xTime * sample->xZ + p_result->xZ);
            if (limit <= xSum) {
                break;
            }
        }
        i++;
        sample++;
        weights++;
    } while (i < CAMERA_AVERAGE_HISTORY_COUNT);

    if (xSum != 0.0f) {
        MTH3D_M_vSetXYZ_inv(p_result, p_result->xX / xSum, p_result->xY / xSum,
                            p_result->xZ / xSum);
    }
}
