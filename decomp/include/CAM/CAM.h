#ifndef REVO_CAM_CAM_H
#define REVO_CAM_CAM_H

#include "ps2/types.h"
#include "MTH/MTH.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"

typedef struct tdstInternalStructurCineinfo_ {
    MTH3D_tdstVector_ stShiftTarget;
    MTH3D_tdstVector_ stShiftPos;
    f32 xDistMin;
    f32 xDistMax;
    f32 xBoundDistMin;
    f32 xBoundDistMax;
    f32 xAngleAlpha;
    f32 xAngleShiftAlpha;
    f32 xAngleTheta;
    f32 xAngleShiftTheta;
    f32 xLinearSpeed;
    f32 xLinearIncreaseSpeed;
    f32 xLinearDecreaseSpeed;
    f32 xAngularSpeed;
    f32 xAngularIncreaseSpeed;
    f32 xAngularDecreaseSpeed;
    f32 xTargetSpeed;
    f32 xTargetIncreaseSpeed;
    f32 xTargetDecreaseSpeed;
    f32 xFocal;
    f32 xZMin;
    f32 xZMax;
    struct HIE_tdstSuperObject_* hSuperObjectTargeted;
    struct HIE_tdstSuperObject_* hSecondSuperObjectTargeted;
    u16 uwDNMFlags;
    u16 uwIAFlags;
    u8 eTypeOfViewport;
    s8 cChannel;
    u8 bIsActive;
} tdstInternalStructurCineinfo_;

typedef struct CAM_tdstComputedPosition_ {
    MTH3D_tdstVector_ stTarget;
    MTH3D_tdstVector_ stMovePos;
    MTH3D_tdstVector_ stRefAxisZ;
    f32 xLinearSpeed;
    f32 xAngularSpeed;
    f32 xTargetSpeed;
} CAM_tdstComputedPosition_;

typedef struct tdstCineinfo_ tdstCineinfo_;

typedef struct CAM_tdstUpdateCamera_ {
    DNM_stReport_* p_stReport;
    DNM_stDynamics* p_stDynamics;
    tdstCineinfo_* hCineinfo;
    tdstInternalStructurCineinfo_* hCineinfoWork;
    tdstInternalStructurCineinfo_* hCineinfoCurrent;
    tdstInternalStructurCineinfo_* hCineinfoInit;
    tdstInternalStructurCineinfo_* hCineinfoVisibility;
    struct HIE_tdstSuperObject_* hSuperObjCamera;
    MTH3D_tdstVector_ stCurrentCameraPos;
    MTH3D_tdstVector_ stWantedCameraPos;
    MTH3D_tdstVector_ stRealWantedCameraPos;
    MTH3D_tdstVector_ stTargetedPersoRealPos;
    MTH3D_tdstVector_ stTargetedPersoPos;
    MTH3D_tdstVector_ stSecondTargetedPersoRealPos;
    MTH3D_tdstVector_ stSecondTargetedPersoPos;
    s8 cHasCutAlpha;
    f32 xAngleAlphaCut;
    f32 xAngleDeltaAlphaCut;
    s8 cTgtPersoIsMovingAbsolute;
    MTH3D_tdstVector_ stTgtPersoMoveAbsolute;
    s8 cTgtPersoIsMovingRelative;
    MTH3D_tdstVector_ stTgtPersoMoveRelative;
    CAM_tdstComputedPosition_ stCptPos;
} CAM_tdstUpdateCamera_;

#define CAMERA_TRANSPARENT_OBJECT_COUNT 10
#define CAMERA_AVERAGE_HISTORY_COUNT 8

typedef struct tdstAveragePosition_ {
    MTH3D_tdstVector_ a_CheckedSpeed[CAMERA_AVERAGE_HISTORY_COUNT];
    MTH3D_tdstVector_ a_CheckedPos[CAMERA_AVERAGE_HISTORY_COUNT];
    f32 a_CheckedTime[CAMERA_AVERAGE_HISTORY_COUNT];
} tdstAveragePosition_;

typedef struct tdstTransparentSuperObjectForCamera_ {
    struct HIE_tdstSuperObject_* hSuperObject;
    s32 lInitialDrawMask;
    f32 fInitialTransparenceLevel;
    f32 fCurrentTransparencyLevel;
    u8 bIncrease;
} tdstTransparentSuperObjectForCamera_;

typedef struct tdstSightAxisSystem_ {
    MTH3D_tdstVector_ SightAxis;
    MTH3D_tdstVector_ FirstComplementaryAxis;
    MTH3D_tdstVector_ SecondComplementaryAxis;
} tdstSightAxisSystem_;

typedef enum e_ucListViewport_ {
    CAM_e_NoViewport = 0
} e_ucListViewport_;

typedef enum e_ucCameraStates_ {
    CAM_e_State_GoToOptimal = 0,
    CAM_e_State_GoToComputedOptimal = 1,
    CAM_e_State_FailureVisibility = 2,
    CAM_e_State_FailureVisibilityWithoutCut = 3,
    CAM_e_TempState_GhostMode = 4,
    CAM_e_TempState_ApexMode = 5
} e_ucCameraStates_;

struct tdstCineinfo_ {
    tdstInternalStructurCineinfo_* hInit;
    tdstInternalStructurCineinfo_* hCurrent;
    tdstInternalStructurCineinfo_* hVisibility;
    tdstInternalStructurCineinfo_* hWork;
    MTH3D_tdstVector_ stLastFailedOrigin;
    MTH3D_tdstVector_ stLastFailedWanted;
    MTH3D_tdstVector_ stForceTarget;
    MTH3D_tdstVector_ stForcePosition;
    MTH3D_tdstVector_ stForceRefAxisY;
    MTH3D_tdstVector_ stForceRefAxisZ;
    f32 xApexRatio;
    f32 xLastDeltaCut;
    f32 xCounter1;
    f32 xCounter2;
    f32 xCounter3;
    f32 xCounter4;
    u8 eState;
    u8 ucVolIAFlags;
    u8 ucPerIAFlags;
    s8 cLastCutAngleSens;
    tdstAveragePosition_ AveragePosition;
    tdstSightAxisSystem_ stSightAxisSystem;
    u8 bCanDoBestPos;
    tdstTransparentSuperObjectForCamera_ a10_stTransparentSuperObjectForCamera[CAMERA_TRANSPARENT_OBJECT_COUNT];
    s16 xNumberOfTransparentSuperObjectForCamera;
    f32 fTransparency;
    f32 fTransparencySpeed;
};

typedef struct CAM_tdstCameraConstants_ {
    f32 xRayCameraSphereToGo1;
    f32 xRayCameraSphereToGo2;
    f32 xRayCameraSphereToSee1;
    f32 xRayCameraSphereToSee2;
    f32 xBaseAngleComputePosNormal;
    f32 xBaseAngleComputePosFailure;
    f32 xAverageForComputePos;
    f32 xAverageForComputeLinearSpeed;
    f32 xMinLinearSpeedCamera;
    f32 xMinLinearSpeedTgtPerso;
    f32 xMinAngularSpeedTgtPerso;
    f32 xEpsilonDynTgtAngle;
    f32 xEpsilonDynTgtFocalAngle;
    f32 xEpsilonForEqualVectors;
    f32 xOffsetMinDistPerso;
    f32 xDistanceForNoCut;
    s16 xNumRayCameraCanGo;
    s16 xNumRayCameraCanSee;
    s16 xTickTestVisibility;
    s16 xTickFindBetterPos;
    s16 xTickFindBetterPosIfNoMove;
    s16 xTickFailureCantSee;
    f32 xDynSpeed_LinearFactNorm;
    f32 xDynSpeed_LinearMax;
    f32 xDynSpeed_LinearMulPerso;
    f32 xDynSpeed_AngularFactNorm;
    f32 xDynSpeed_AngularMax;
    f32 xDynSpeed_AngularMulPerso;
    f32 xDynSpeed_TargetFactNorm;
    f32 xDynSpeed_TargetMax;
    f32 xDynSpeed_TargetMulPerso;
    f32 xBaseCutAngle;
    f32 xCutAngleFactorPos;
    f32 xCutAngleFactorTgt;
    s16 wFlags;
    s16 wAlign;
} CAM_tdstCameraConstants_;

extern void CAM_fn_vComputeMovePos(CAM_tdstUpdateCamera_* update);
extern void CAM_fn_vComputeRefAxis(CAM_tdstUpdateCamera_* _p_stStruct,
                                   MTH3D_tdstVector_* _p_stRefAxis);
extern void CAM_fn_vComputeReferencePoint(CAM_tdstUpdateCamera_* _p_stStruct);
extern s32 CAM_fn_cIsWantedPosIncorrect(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stWantedPos, s8 _cMode);
extern void CAM_fn_vUpdateGeneralCamera(CAM_tdstUpdateCamera_* _p_stStruct);

extern void CAM_fn_vInitCompleteCineinfo(tdstCineinfo_* cineinfo);
extern void CAM_fn_vSetCineinfoWorkFromCurrent(tdstCineinfo_* _hCineinfo);

extern void CAM_fn_vSetResetInternalFlag(tdstCineinfo_* _hCineInfo, u16* _p_uwCurrentValue, u16* _p_uwVisibilityValue,
                                         u16* _p_uwWorkValue, s32 _lTypeOfChange, long _lChangingMode, u16 _uwFlag);

extern void CAM_fn_vActualizeAViewport(s32 viewport);
extern HIE_tdstSuperObject_* CAM_fn_hGetActiveCamera(s32 _lViewportNumber);

typedef struct tdstNodeCamera_ tdstNodeCamera_;

typedef struct LST2_tdstAnchorTotdxHandleToNodeCameraDyn {
    tdstNodeCamera_* hFirstElementDyn;
    tdstNodeCamera_* hLastElementDyn;
    s32 lNumberOfElementsDyn;
} LST2_tdstAnchorTotdxHandleToNodeCameraDyn;

struct tdstNodeCamera_ {
    HIE_tdstSuperObject_* p_stSuperObjectCamera;
    struct tdstNodeCamera_* hNextBrotherDyn;
    struct tdstNodeCamera_* hPrevBrotherDyn;
    LST2_tdstAnchorTotdxHandleToNodeCameraDyn* hFatherDyn;
};

extern void* inv_CAM_hPreviousTarget;

extern void CAM_fn_vComputeTarget(CAM_tdstUpdateCamera_* _p_stStruct);
extern void CAM_fn_vComputeRealWantedPos(CAM_tdstUpdateCamera_* _p_stStruct);

extern s32 CAM_fn_bEqualVectorWithEpsilon(MTH3D_tdstVector_* _p_stVector1, MTH3D_tdstVector_* _p_stVector2,
                                          f32 _xEpsilon);
extern s32 CAM_fn_bPersoOnFace(HIE_tdstSuperObject_* _h_SuperObjPerso, MTH3D_tdstVector_* _p_stNormal);
extern void CAM_fn_vComputeProjection(MTH3D_tdstVector_* _p_sDestVector, MTH3D_tdstVector_* _p_sSrcVector,
                                      MTH3D_tdstVector_* _p_sNormVector);
extern void CAM_fn_vComputeAnglesBetweenTwoVectors(tdstInternalStructurCineinfo_* slot, MTH3D_tdstVector_* wanted, MTH3D_tdstVector_* current,
                          f32* horizontal, f32* vertical, MTH3D_tdstVector_* reference);
extern f32 CAM_fn_xSetAngleInIntervalPiPi(f32 _xAngle);
extern f32 Cam_fn_xComputeProjectionRail(MTH3D_tdstVector_* _p_stTargetedPersoPos, MTH3D_tdstVector_* _p_stWpPosition,
                                         MTH3D_tdstVector_* _p_stDestWpPosition);
extern f32 CAM_fn_xSetAngleInInterval02Pi(f32 _xAngle);
extern void CAM_fn_vForceCameraPos(HIE_tdstSuperObject_* _hSuperObjCamera,
                                   MTH3D_tdstVector_* _p_stPosition);
extern void CAM_fn_vForceCameraTgt(HIE_tdstSuperObject_* _hSuperObjCamera,
                                   MTH3D_tdstVector_* _p_stPosition,
                                   MTH3D_tdstVector_* _p_stRefAxisZ);
extern HIE_tdstSuperObject_* CAM_fn_hGetFather(HIE_tdstSuperObject_* _h_SuperObjPerso);
extern f32 CAM_fn_xComputeRotation(MTH3D_tdstVector_* _p_stDestVector,
                                   MTH3D_tdstVector_* _p_stSrcVector,
                                   MTH3D_tdstVector_* _p_stAxisVector);
extern void CAM_fn_vInitCameraStructure(HIE_tdstSuperObject_* _hSuperObjPerso,
                                        CAM_tdstUpdateCamera_* _p_stStruct);
extern void CAM_fn_vSendParametersToParsing(CAM_tdstUpdateCamera_* _p_stStruct);
extern void Cam_fn_vComputeMovingOnRail(CAM_tdstUpdateCamera_* _p_stStruct,
                                        MTH3D_tdstVector_* _p_stWantedPosition,
                                        MTH3D_tdstVector_* _p_stWpPosition,
                                        MTH3D_tdstVector_* _p_stPreviousWpPosition,
                                        f32 _xDistWanted);

extern void CAM_fn_vForceFailureVisibility(CAM_tdstUpdateCamera_* _p_stStruct);
extern void CAM_fn_vComputeFailureVisibility(CAM_tdstUpdateCamera_* _p_stStruct);
extern void CAM_fn_vTestBadDistance(CAM_tdstUpdateCamera_* update);
extern void CAM_fn_vTestBadVisibility(CAM_tdstUpdateCamera_* _p_stStruct);
extern void CAM_fn_vTestOppositeCollision(CAM_tdstUpdateCamera_* update);
extern s32 CAM_fn_cCanResetConstraintMoveVisFailure(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* vector);
extern void CAM_fn_vCheckForABetterPos(CAM_tdstUpdateCamera_* _p_stStruct);
extern void CAM_fn_vComputeCineinfoCurrentForAPosition(CAM_tdstUpdateCamera_* update, MTH3D_tdstVector_* position);

extern void CAM_fn_vComputeMovePosWithDynTheta(CAM_tdstUpdateCamera_* _p_stStruct);

extern s32 CAM_fn_bPersoIsMoving(CAM_tdstUpdateCamera_* _p_stStruct, MTH3D_tdstVector_* _p_stMove);
extern s32 CAM_fn_bPersoIsTurning(HIE_tdstSuperObject_* _h_SuperObjPerso, f32* _p_xAngularSpeed);
extern void CAM_fn_vGetPersoSightAxis(CAM_tdstUpdateCamera_* _p_stStruct,
                                      MTH3D_tdstVector_* _p_stAxis, MTH3D_tdstVector_* _p_stAxisZ);

extern tdstInternalStructurCineinfo_ gst_Ideal;
extern s8 cNoDynChangeTheta;
extern s8 cRefAxisIsAlreadyComputed;
extern s8 cCanTestStatic;
extern s8 cJustBetterPos;

extern CAM_tdstCameraConstants_ CAM_g_stCameraConstants;
extern u16 inv_CAM_wFlags[];

#endif
