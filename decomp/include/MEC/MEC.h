#ifndef REVO_MEC_MEC_H
#define REVO_MEC_MEC_H

#include "ps2/types.h"
#include "MTH/MTH.h"

typedef struct MEC_Influence_ {
    MTH3D_tdstVector_ direction;
    f32 scalar;
    f32 weight;
} MEC_Influence_;

typedef struct MTH_stRotation {
    f32 m_xAngle;
    MTH3D_tdstVector_ m_stAxis;
} MTH_stRotation;

typedef struct MTH_stMove {
    MTH3D_tdstVector_ m_stLinear;
    MTH_stRotation m_stAngular;
} MTH_stMove;

typedef struct DNM_stObstacle {
    f32 m_xRate;
    MTH3D_tdstVector_ m_stNorm;
    MTH3D_tdstVector_ m_stContact;
    struct GMT_tdstGameMaterial_* hMyMaterial;
    struct GMT_tdstGameMaterial_* hCollidedMaterial;
    struct HIE_tdstSuperObject_* mp_stSupObj;
} DNM_stObstacle;

typedef struct DNM_stReport_ {
    u32 m_ulPrevSurfaceState;
    u32 m_ulCurrSurfaceState;
    DNM_stObstacle m_stObstacle;
    DNM_stObstacle m_stGround;
    DNM_stObstacle m_stWall;
    DNM_stObstacle m_stCharacter;
    DNM_stObstacle m_stWater;
    DNM_stObstacle m_stCeil;
    MTH_stMove m_stAbsolutePrevSpeed;
    MTH_stMove m_stAbsoluteCurrSpeed;
    MTH_stMove m_stAbsolutePrevPosition;
    MTH_stMove m_stAbsoluteCurrPosition;
    u8 m_cBitField;
} DNM_stReport_;

typedef struct DNM_stDynamicsBase_ {
    u32 m_lObjectType;
    void* m_pCurrentIdCard;
    u32 ulFlags;
    u32 ulEndFlags;
    f32 m_xGravity;
    f32 m_xSlopeLimit;
    f32 m_xCosSlope;
    f32 m_xSlide;
    f32 m_xRebound;
    MTH3D_tdstVector_ m_stImposeSpeed;
    MTH3D_tdstVector_ m_stProposeSpeed;
    MTH3D_tdstVector_ m_stPreviousSpeed;
    MTH3D_tdstVector_ m_stScale;
    MTH3D_tdstVector_ m_stSpeedAnim;
    u32 m_stSafeTranslation[3];
    MTH3D_tdstVector_ m_stAddTranslation;
    POS_tdstMatrix_ m_stPreviousMatrix;
    POS_tdstMatrix_ m_stCurrentMatrix;
    MTH3D_tdstMatrix_ m_stImposeRotationMatrix;
    u8 temporalCurveType;
    u8 collisionPolicy;
    u8 gu_unk146[2];
    DNM_stReport_* m_pstReport;
} DNM_stDynamicsBase_;

typedef struct DNM_stDynamicsAdvancedBlock {
    f32 m_xInertiaX;
    f32 m_xInertiaY;
    f32 m_xInertiaZ;
    f32 m_xStreamPrority;
    f32 m_xStreamFactor;
    f32 m_xSlideFactorX;
    f32 m_xSlideFactorY;
    f32 m_xSlideFactorZ;
    f32 m_xPreviousSlide;
    MTH3D_tdstVector_ m_stMaxSpeed;
    MTH3D_tdstVector_ m_stStreamSpeed;
    MTH3D_tdstVector_ m_stAddSpeed;
    MTH3D_tdstVector_ m_stLimit;
    MTH3D_tdstVector_ m_stCollisionTranslation;
    MTH3D_tdstVector_ m_stInertiaTranslation;
    MTH3D_tdstVector_ m_stGroundNormal;
    MTH3D_tdstVector_ m_stWallNormal;
    s8 m_cCollideCounter;
} DNM_stDynamicsAdvancedBlock;

typedef struct stMACDPID {
    f32 m_xData0;
    MTH3D_tdstVector_ m_stData1;
    MTH3D_tdstVector_ m_stData2;
    MTH3D_tdstVector_ m_stData3;
    f32 m_xData3;
    f32 m_xData4;
    f32 m_xData5;
    MTH_stRotation m_stData6;
    MTH_stRotation m_stData7;
    s8 m_cData8;
    u16 m_uwData9;
    MTH3D_tdstVector_ m_stData10;
    f32 m_xData11;
    MTH3D_tdstVector_ m_stData12;
    f32 m_xData13;
    u8 m_bData14;
} stMACDPID;

typedef struct DNM_stDynamicsComplexBlock {
    f32 m_xTiltIntensity;
    f32 m_xTiltInertia;
    f32 m_xTiltOrigin;
    f32 m_xTiltAngle;
    f32 m_xHangingLimit;
    MTH3D_tdstVector_ m_stContact;
    MTH3D_tdstVector_ m_stFallTranslation;
    stMACDPID m_stExternalDatas;
    struct HIE_tdstSuperObject_* m_hPlatform;
    POS_tdstMatrix_ m_stAbsolutePreviousMatrix;
    POS_tdstMatrix_ m_stPrevPreviousMatrix;
} DNM_stDynamicsComplexBlock;

typedef struct DNM_stDynamics {
    DNM_stDynamicsBase_ m_tdstDynamicsBaseBlock;
    DNM_stDynamicsAdvancedBlock m_tdstDynamicsAdvancedBlock;
    DNM_stDynamicsComplexBlock m_tdstDynamicsComplexBlock;
} DNM_stDynamics;

typedef struct DNM_stMecMatCharacteristics {
    f32 m_xAdhesionCoef;
    f32 m_xAbsorptionCoef;
    f32 m_xFrictionCoef;
    f32 m_xSlideCoef;
    f32 m_xProgressionCoef;
    f32 m_xPenetrationSpeed;
    f32 m_xPenetrationMax;
} DNM_stMecMatCharacteristics;

typedef enum eDynamicSize_ {
    eDynamicSizeBase = 0,
    eDynamicSizeAdvanced = 1,
    eDynamicSizeComplex = 2
} eDynamicSize_;

typedef struct DNM_stParsingDatas {
    MTH3D_tdstVector_ stPosition;
    f32 xOutAlpha;
    MTH3D_tdstVector_ stVector;
} DNM_stParsingDatas;

typedef struct tdstDynam_ {
    DNM_stParsingDatas stParsingDatas;
    DNM_stDynamics* p_stDynamics;
} tdstDynam_;

static inline DNM_stDynamics* inv_MEC_fn_p_stDynamicsOf(tdstDynam_* dynam)
{
    return dynam->p_stDynamics;
}

typedef struct DNM_stMecObstacleProperties_inv {
    f32 m_xSlide;
    f32 m_xRebound;
} DNM_stMecObstacleProperties_inv;

typedef struct DNM_stMecObstacle {
    DNM_stObstacle contact;
    s32 m_eType;
    s16 m_xMyEntity;
    s16 m_xCollidedEntity;
    MTH3D_tdstVector_ m_stTranslation;
    MTH3D_tdstVector_ m_stZoneMove;
    MTH3D_tdstVector_ m_stZonePosition;
    f32 m_xZoneRadius;
    DNM_stMecObstacleProperties_inv m_stProperties;
    DNM_stMecObstacleProperties_inv m_stMyProperties;
} DNM_stMecObstacle;

typedef struct DNM_stMecReport {
    s32 m_ulCurrSurfaceState;
    DNM_stMecObstacle m_stObstacle;
    DNM_stMecObstacle m_stGround;
    DNM_stMecObstacle m_stWall;
    DNM_stMecObstacle m_stCharacter;
    DNM_stMecObstacle m_stWater;
    DNM_stMecObstacle m_stCeil;
} DNM_stMecReport;

extern DNM_stMecReport g_stReport;
extern u32 inv_MEC_stReportObstacle[];
extern u32 inv_MEC_stReportGround[];

extern struct EffectSystem_inv* inv_MEC_p_stFirstEffectSystem;
extern void* inv_MEC_hParticleSuperObject;

extern void MTH_fn_vVectorPlaneProjection(MTH3D_tdstVector_* _p_stResult, MTH3D_tdstVector_* _p_stSource,
                                          MTH3D_tdstVector_* _p_stPlaneNorm, MTH3D_tdstVector_* _p_stProjDirection);
extern void* MTH_p_stRotationComputeRotationWith2Vectors(MTH_stRotation* _p_stResult,
                                                         MTH3D_tdstVector_* _p_stSrcUnitVector,
                                                         MTH3D_tdstVector_* _p_stImaUnitVector);
extern MTH3D_tdstVector_* MTH_p_stRotationMulVector(MTH_stRotation* _p_stRotation, MTH3D_tdstVector_* _p_stSource, MTH3D_tdstVector_* _p_stResult);

extern void DNM_fn_vSetHangingLimit(DNM_stDynamics* object, f32 value);
extern void MEC_vTurnAbsolute(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stVector);
extern void MEC_fn_vUpdateSpeed(struct HIE_tdstSuperObject_* hSupObj);
#ifdef PC_PORT
extern DNM_stDynamics* MEC_p_stDynamicsBaseMechanics(DNM_stDynamics* p_stDynamic,
                                                     struct HIE_tdstSuperObject_* hSupObj, void* unused2,
                                                     void* unused3, f32 unusedDt);
#else
extern DNM_stDynamics* MEC_p_stDynamicsBaseMechanics(DNM_stDynamics* p_stDynamic,
                                                     struct HIE_tdstSuperObject_* hSupObj);
#endif
extern void MEC_vImposeTranslation(DNM_stDynamics* p_stDynamic,
                                   const MTH3D_tdstVector_* p_stVector);
extern void MEC_vTurnAroundZAxis(DNM_stDynamics* p_stDynamic, f32 xAngle);
extern void MEC_vImposeRotationMatrix(DNM_stDynamics* p_stDynamic, POS_tdstMatrix_* hRotationMatrix);
extern void MEC_vImposeRotationVector(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stX, MTH3D_tdstVector_* p_stY, MTH3D_tdstVector_* p_stZ);
extern void MEC_vInitTranslation(DNM_stDynamics* p_stDynamic, struct HIE_tdstSuperObject_* hSupObj, MTH3D_tdstVector_* p_stVector);
extern void MEC_vImposeRelativeRotation(DNM_stDynamics* p_stDynamic, MTH3D_tdstMatrix_* p_stRotationMatrix);

extern void MEC_fn_vInertia(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* hMatrix,
                            POS_tdstMatrix_* hInvMatrix);
extern void MEC_fn_vAddSpeed(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* hMatrix,
                             POS_tdstMatrix_* hInvMatrix);
extern void MEC_fn_vSpeedLimit(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* hMatrix,
                               POS_tdstMatrix_* hInvMatrix);
extern void MEC_fn_vAdjustScaleData(DNM_stDynamics* p_stDynamic, POS_tdstMatrix_* hMatrix,
                                    POS_tdstMatrix_* hInvMatrix);
extern void MEC_fn_vCancelTilt(DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vGravitySpeed(DNM_stDynamics* p_stDynamic, f32 xDT);
extern void MEC_fn_vPositionLimit(DNM_stDynamics* p_stDynamic,
                                  MTH3D_tdstVector_* p_stSafePreviousPosition);
extern void MEC_fn_vImposeRotation(DNM_stDynamics* dynamics);
extern void MEC_fn_vAdditionnalTranslation(DNM_stDynamics* dynamics);
extern void MEC_fn_vAddStream(DNM_stDynamics* p_stDynamic, f32 xDT, u8 bSlide);
extern void MEC_fn_vImposeSpeed(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* matrix, POS_tdstMatrix_* inverseMatrix);
extern void MEC_fn_vProposeSpeed(DNM_stDynamics* p_stDynamic, f32 xDT, POS_tdstMatrix_* matrix, POS_tdstMatrix_* inverseMatrix);
extern void MEC_fn_vAddTiltSpeed(DNM_stDynamics* p_stDynamic, f32 xDt);

extern void MEC_fn_vUpdatePlatform(DNM_stDynamics* p_stDynamic, struct HIE_tdstSuperObject_* hSupObj,
                                   POS_tdstMatrix_* previous, POS_tdstMatrix_* motion);
extern s32 MEC_fn_vBasePlatformManagement(DNM_stDynamics* p_stDynamic,
                                          struct HIE_tdstSuperObject_* hSupObj);
extern void MEC_fn_vForcePlatformLink(DNM_stDynamics* p_stDynamic, struct HIE_tdstSuperObject_* hSupObj, struct HIE_tdstSuperObject_* hFatherSupObj);
extern void MEC_fn_vFreePlatformLink(DNM_stDynamics* p_stDynamic, struct HIE_tdstSuperObject_* hSupObj);

extern void MEC_fn_vBaseObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT);
extern void MEC_fn_vClimbObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT);
extern void MEC_fn_vGroundObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT);
extern void MEC_fn_vSpiderObstacleManagement(DNM_stDynamics* dynamics, void* context,
                                             f32 deltaTime);
extern void MEC_fn_vGroundGuess(DNM_stDynamics* p_stDynamic, f32 xDT);
extern void MEC_fn_vGiObstacleManagement(DNM_stDynamics* p_stDynamic, void* hSupObj, f32 xDT);
extern void MEC_fn_vSwimObstacleManagement(DNM_stDynamics* obj, void* ctx, f32 dt);

extern f32 CAM_fn_xComputeHorizontalAngle(MTH3D_tdstVector_* _p_stOrgVector,
                                          MTH3D_tdstVector_* _p_stDestVector,
                                          MTH3D_tdstVector_* _p_stAxisZ);
extern f32 CAM_fn_xComputeVerticalAngle(MTH3D_tdstVector_* _p_stOrgVector,
                                        MTH3D_tdstVector_* _p_stDestVector,
                                        MTH3D_tdstVector_* _p_stAxisZ);
extern void CAM_fn_vComputeCameraSystemAxis(void* _p_stDynamics, struct HIE_tdstSuperObject_* _h_SupObj,
                                            MTH3D_tdstVector_* _p_stNewPosition,
                                            MTH3D_tdstVector_* _p_stNewTarget, MTH3D_tdstVector_* _p_stAxisX,
                                            MTH3D_tdstVector_* _p_stAxisY, MTH3D_tdstVector_* _p_stAxisZ);
extern DNM_stDynamics* DNM_p_stDynamicsCameraMechanics(DNM_stDynamics* _p_stDynamics,
                                                       struct HIE_tdstSuperObject_* _hSupObj,
                                                       DNM_stParsingDatas* _p_stParsingDatas,
                                                       f32 _xDT);

extern DNM_stMecObstacle* MEC_fn_pSelectGround(DNM_stDynamics* p_stDynamic, s32 lObstacle,
                                               DNM_stMecObstacle* p_stObstacle,
                                               MTH3D_tdstVector_* p_stTangentMove,
                                               u8* bFirstGround, DNM_stMecObstacle** step,
                                               u8* hasStep);
extern s32 MEC_fn_bNoMove(DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vAdjustScaleVector(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stVector);
extern void MEC_fn_vAdjustSpeed(DNM_stDynamics* p_stDynamic, f32 xDT);
extern void MEC_fn_vComputeSpeed(DNM_stDynamics* p_stDynamic, f32 xDT);
extern void MEC_fn_vDeleteHangingObstacle(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                          DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vDeleteNoCollidedGroundFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                               DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vDeleteNullTranslationObstacle(s32 lObstacle, DNM_stMecObstacle* a_stObstacle);
extern void MEC_fn_vDeleteWallEdge(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                   DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vInitMatrix(DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vSetInitialFrameTranslation(DNM_stDynamics* p_stDynamic,
                                               MTH3D_tdstVector_* p_stTranslation);
extern void MEC_fn_vSlowMode(DNM_stDynamics* p_stDynamic, s32 lMode);
extern void MEC_fn_vTranslateMatrixOff(DNM_stDynamics* p_stDynamic,
                                       MTH3D_tdstVector_* p_stTranslation);
extern f32 MEC_fn_xConvertScale(f32 xValue, u8 bAdjust);
extern void MEC_fn_vDeleteUnderGroundWall(s32 lObstacle, void* p_stObstacle,
                                          DNM_stDynamics* p_stDynamic,
                                          DNM_stMecObstacle* p_stGroundObstacle);
extern f32 MEC_fn_xGetRebound(DNM_stDynamics* p_stDynamic, DNM_stMecObstacle* p_stObstacle);
extern s32 MEC_fn_bDynamicsNeedCollide(DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vTranslateMatrixOn(DNM_stDynamics* p_stDynamic,
                                      MTH3D_tdstVector_* p_stMatrixTranslation);
extern void MEC_fn_vCorrectMatrix(DNM_stDynamics* p_stDynamic, void* hMatrix);
extern void MEC_fn_vNormalMode(DNM_stDynamics* p_stDynamic, s32 lMode);
extern f32 MEC_fn_xGetSlide(DNM_stDynamics* p_stDynamic, DNM_stMecObstacle* p_stObstacle);
extern int MEC_fn_bMainActor(struct HIE_tdstSuperObject_* hSupObj);
extern void MEC_fn_vComputeCollisionTranslation(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                                const MTH3D_tdstVector_* p_stMove);
extern void MEC_fn_vDeleteClimbBackFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                        DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stVector);
extern void MEC_fn_vDeleteOnGroundBackFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                           DNM_stDynamics* p_stDynamic,
                                           MTH3D_tdstVector_* p_stVector,
                                           DNM_stMecObstacle* p_stGroundObstacle);
extern void MEC_fn_vRoundVector(MTH3D_tdstVector_* p_stVector, f32 xEpsilon);
extern void* MEC_fn_hGetFatherActor(struct HIE_tdstSuperObject_* hSupObj);
extern void MEC_fn_vComputeInertiaTranslation(DNM_stDynamics* obj, f32 xDT);
extern void MEC_fn_vComputeStream(MTH3D_tdstVector_* p_stStream, DNM_stDynamics* p_stDynamic,
                                  u8 bSlide, f32 xDT);
extern void MEC_fn_vDeleteGiBackFace(s32 lObstacle, DNM_stMecObstacle* p_stObstacle, DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stVector);
extern s32 MEC_fn_bGroundNormal(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stNormal);
extern s32 MEC_fn_bWallNormal(DNM_stDynamics* p_stDynamic, MTH3D_tdstVector_* p_stNormal);
extern s32 MEC_fn_bCanCrashActor(DNM_stMecObstacle* p_stCollideObstacle);
extern s32 MEC_fn_bCeilObstacle(DNM_stDynamics* p_stDynamic, DNM_stObstacle* p_stObstacle);
extern void func_0013B6E8(DNM_stDynamics* p_stDynamic);
extern void MEC_fn_vDeleteBaseBackFace(s32 lObstacle, DNM_stMecObstacle* base, DNM_stDynamics* p_stDynamic);

struct GLD_tdstViewportAttributes_;
extern void inv_fn_vClearLevelRuntimeList(void);
extern s32 inv_MEC_fn_bIsPointBehindPlane(MTH3D_tdstVector_* p_stNormal, MTH3D_tdstVector_* p_stOrigin,
                         MTH3D_tdstVector_* p_stPoint);
extern void inv_MEC_fn_vUpdateAllEffectSystems(struct GLD_tdstViewportAttributes_* p_stVpt, s32 lDrawModeMask);

extern DNM_stReport_* DNM_p_stReportDefaultInit(DNM_stReport_* _p_stReport);

extern u8 MEC_fn_vComputeAdjustVector(MTH3D_tdstVector_* p_stAdjustVector, void* object,
                                      s32 lObstacle, DNM_stMecObstacle* p_stObstacle,
                                      s32 lObstacleMin);
extern void MEC_fn_vEdgeAnalysis(DNM_stDynamics* p_stDynamic, s32 lObstacle,
                                 DNM_stMecObstacle* p_stObstacle, DNM_stMecObstacle* p_stGround);
extern s32 MEC_fn_bCollision(s32* p_lObstacle, DNM_stMecObstacle* p_stObstacle,
                             DNM_stDynamics* p_stDynamic, struct HIE_tdstSuperObject_* p_stSupObj,
                             POS_tdstMatrix_* start, POS_tdstMatrix_* end, u8 bBackFace);
extern void MEC_fn_vReportCollision(DNM_stDynamics* p_stDynamic,
                                    struct HIE_tdstSuperObject_* hSupObj, s32 lNbObstacle,
                                    DNM_stMecObstacle* a_stObstacle, s32 lObstacle);
extern void MEC_fn_vCrashAnalysis(DNM_stDynamics* p_stDynamic, s32 lObstacle, DNM_stMecObstacle* p_stObstacle);
extern void MEC_fn_vFillCollisionReport(DNM_stDynamics* p_stDynamic);

extern s32 g_lCallCounter;
extern u8 g_bHighSpeed;
extern s32 lZWaterValidity;
extern s32 D_00393E10;
extern f32 g_xZWater;

struct EffectOrientation_inv;
struct GeometricObjectView;

typedef struct CollisionNode {
    u8 type;
    struct CollisionNode* next;
    f32 inv_xBounce;
    void* data;
} CollisionNode;

typedef struct CollisionShape {
    f32 inv_xFriction;
    f32 inv_xRestSpeed;
    MTH3D_tdstVector_ points[5];
} CollisionShape;

typedef struct EffectParticle_inv {
    u8 flags;
    f32 lifetime;
    f32 initialLifetime;
    f32 emitTimer;
    f32 maxStep;
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ velocity;
    MTH3D_tdstVector_ acceleration;
    struct EffectOrientation_inv* orientation;
    CollisionNode* lastCollider;
} EffectParticle_inv;

typedef struct EffectOrientation_inv {
    union {
        struct {
            f32 angle;
            f32 angularSpeed;
        } st2D;
        MTH3D_tdstVector_ stRotation;
    } u;
    POS_tdstMatrix_ matrix;
    MTH3D_tdstMatrix_ rotationSpeed;
    void* object;
} EffectOrientation_inv;

typedef struct EffectBasisRow_inv {
    MTH3D_tdstVector_ xyz;
    f32 w;
} EffectBasisRow_inv;

typedef struct EffectSystem_inv {
    struct GeometricObjectView* geometricObject;
    u16 flags;
    s8 emitterShape;
    f32 emitterWidth;
    f32 emitterHeight;
    f32 emitterRadius;
    u8 pad014[0x04];
    f32 spawnRate;
    s32 spawnLimit;
    s32 particleCount;
    u8 pad024[0x14];
    f32 lifetimeBase;
    f32 lifetimeRange;
    u8 pad040[0x04];
    f32 spreadX;
    f32 spreadY;
    u8 pad04C[0x0C];
    f32 speedBase;
    f32 speedRange;
    f32 directionScale;
    f32 spinBase;
    f32 spinRange;
    MTH3D_tdstVector_ spinAxis;
    f32 spinSpread;
    MTH3D_tdstVector_ acceleration;
    s8 childCountMin;
    u8 pad089[0x03];
    u8 childCountRange;
    u8 pad08D[0x03];
    f32 childLifetimeBase;
    f32 childLifetimeRange;
    u8 pad098[0x1C];
    f32 childSpeedScale;
    f32 childSpeedRange;
    f32 childAngleRange;
    f32 emitPeriod;
    u8 pad0C4[0x18];
    f32 emitLifetimeScale;
    u8 pad0E0[0x04];
    f32 forceStrength;
    f32 forceExponent;
    f32 forceTangentScale;
    CollisionNode* colliders;
    MTH3D_tdstVector_ position;
    MTH3D_tdstVector_ direction;
    EffectBasisRow_inv basis[3];
    u8 pad13C[0x24];
    f32 age;
    u8 runFlags;
    u8 pad165[0x03];
    f32 spawnDelay;
    s32 inv_lActiveParticleCount;
    s32 inv_lFreeParticleCount;
    s32 inv_lUsedParticleCount;
    MTH3D_tdstVector_ boundsMin;
    MTH3D_tdstVector_ boundsMax;
    EffectParticle_inv* particles;
    struct EffectSystem_inv* next;
} EffectSystem_inv;

extern f32 inv_MEC_fn_xGetParticlePlaneDistance(EffectParticle_inv* particle, MTH3D_tdstVector_* planePoint,
                         MTH3D_tdstVector_* planeNormal);
extern s32 inv_MEC_fn_bMovesTowardPlane(EffectParticle_inv* particle, MTH3D_tdstVector_* planeNormal, f32 time);
extern s32 inv_MEC_fn_bIntersectParticleWithPlane(EffectParticle_inv* particle, MTH3D_tdstVector_* segment,
                         CollisionShape* shape, f32* distance, MTH3D_tdstVector_* hit);

typedef struct DNM_stMecBaseIdCard {
    s32 m_eIdentity;
    u32 ulFlags;
    f32 m_xGravity;
    f32 m_xSlide;
    f32 m_xRebound;
    f32 m_xSlopeLimit;
    f32 m_xInertiaX;
    f32 m_xInertiaY;
    f32 m_xInertiaZ;
    f32 m_xTiltIntensity;
    f32 m_xTiltInertia;
    f32 m_xTiltOrigin;
    MTH3D_tdstVector_ m_stMaxSpeed;
} DNM_stMecBaseIdCard;

extern s32 DNM_fn_eIdentityCardGetType(DNM_stMecBaseIdCard* _hCurrent);

extern void DNM_fn_vChangeIdCard(DNM_stDynamics* _p_stDynamics, DNM_stMecBaseIdCard* _hNewIdCard);
extern void DNM_fn_vDynamicsCopyClone(DNM_stDynamics* _p_stDst, DNM_stDynamics* _p_stSrc);
extern void DNM_fn_vDynamicsInitDefault(DNM_stDynamics* p_stDynamic);
extern void DNM_fn_vInitBaseMechanic(DNM_stDynamics* p_stDynamic);

extern f32 MEC_g_xBaseCoeff;
extern f32 MEC_g_xWalkCoeff;

extern DNM_stMecObstacle* g_a_stObstacle;

#endif
