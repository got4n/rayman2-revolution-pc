#ifndef REVO_AI_AI_H
#define REVO_AI_AI_H

#include "ps2/types.h"
#include "MTH/MTH.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "IPT/IPT.h"

typedef enum tdeTypeInterpret_ {
    E_ti_KeyWord = 0x000,
    E_ti_Condition = 0x001,
    E_ti_Operator = 0x002,
    E_ti_Function = 0x003,
    E_ti_Procedure = 0x004,
    E_ti_MetaAction = 0x005,
    E_ti_BeginMacro = 0x006,
    E_ti_EndMacro = 0x007,
    E_ti_EndTree = 0x008,
    E_ti_Field = 0x009,
    E_ti_DsgVar = 0x00A,
    E_ti_DsgVarRef = 0x00B,
    E_ti_Constant = 0x00C,
    E_ti_Real = 0x00D,
    E_ti_Button = 0x00E,
    E_ti_ConstantVector = 0x00F,
    E_ti_Vector = 0x010,
    E_ti_Mask = 0x011,
    E_ti_Module = 0x012,
    E_ti_DsgVarId = 0x013,
    E_ti_String = 0x014,
    E_ti_FamilyRef = 0x016,
    E_ti_PersoRef = 0x017,
    E_ti_ActionRef = 0x018,
    E_ti_SuperObjectRef = 0x019,
    E_ti_WayPointRef = 0x01A,
    E_ti_TextRef = 0x01B,
    E_ti_ComportRef = 0x01C,
    E_ti_ModuleRef = 0x01D,
    E_ti_SoundEventRef = 0x01E,
    E_ti_ObjectTableRef = 0x01F,
    E_ti_GameMaterialRef = 0x020,
    E_ti_VisualMaterialRef = 0x021,
    E_ti_SOLinksRef = 0x022,
    E_ti_Color = 0x023,
    E_ti_ModelRef = 0x024,
    E_ti_Light = 0x025,
    E_ti_Caps = 0x026,
    E_ti_Graph = 0x027,
    E_ti_MacroRef = 0x028,
    E_ti_Unknown = 0x029,
    E_ti_NbTypes = 0x02A
} tdeTypeInterpret_;

typedef enum tdeVariableType_ {
    E_vt_None = 0x000,
    E_vt_Boolean = 0x001,
    E_vt__128To127 = 0x002,
    E_vt_0To255 = 0x003,
    E_vt__32768To32767 = 0x004,
    E_vt_0To65535 = 0x005,
    E_vt_Integer = 0x006,
    E_vt_PositiveInteger = 0x007,
    E_vt_Float = 0x008,
    E_vt_String = 0x009,
    E_vt_WayPoint = 0x00A,
    E_vt_Perso = 0x00B,
    E_vt_List = 0x00C,
    E_vt_MetaActionReturn = 0x014,
    E_vt_Vector = 0x015,
    E_vt_Module = 0x016,
    E_vt_DsgVarId = 0x017,
    E_vt_Action = 0x018,
    E_vt_Button = 0x019,
    E_vt_Mask = 0x01A,
    E_vt_ObjectTable = 0x01D,
    E_vt_SuperObject = 0x01E,
    E_vt_Family = 0x020,
    E_vt_SoundEvent = 0x021,
    E_vt_Comport = 0x022,
    E_vt_Text = 0x024,
    E_vt_GameMaterial = 0x026,
    E_vt_VisualMaterial = 0x027,
    E_vt_Color = 0x028,
    E_vt_Model = 0x029,
    E_vt_Array = 0x02A,
    E_vt_Light = 0x02B,
    E_vt_Caps = 0x02C,
    E_vt_Graph = 0x02D,
    E_vt_SOLinks = 0x02E
} tdeVariableType_;

typedef enum tdeKeyWordId_ {
    eKeyWord_IfNot = 0x000,
    eKeyWord_If = 0x001,
    eKeyWord_IfNot2 = 0x002,
    eKeyWord_If2 = 0x003,
    eKeyWord_IfNot4 = 0x004,
    eKeyWord_If4 = 0x005,
    eKeyWord_IfNot8 = 0x006,
    eKeyWord_If8 = 0x007,
    eKeyWord_IfNot16 = 0x008,
    eKeyWord_If16 = 0x009,
    eKeyWord_Then = 0x00A,
    eKeyWord_Else = 0x00B,
    eKeyWord_EngineGoto = 0x00C,
    eKeyWord_Me = 0x00D,
    eKeyWord_MainActor = 0x00E,
    eKeyWord_Nobody = 0x00F,
    eKeyWord_NoSuperObject = 0x010,
    eKeyWord_Nowhere = 0x011,
    eKeyWord_EmptyText = 0x012,
    eKeyWord_CapsNull = 0x013,
    eKeyWord_NoGraph = 0x014,
    eKeyWord_NoAction = 0x015,
    eKeyWord_NoGameMaterial = 0x016,
    eKeyWord_NoVisualMaterial = 0x017,
    eKeyWord_NoSoundEvent = 0x018,
    eKeyWord_While = 0x01A,
    eKeyWord_BeginWhile = 0x01B,
    eKeyWord_EndWhile = 0x01C,
    eKeyWord_NoInput = 0x01D,
    eNbKeyWord = 0x01E
} tdeKeyWordId_;

typedef enum tdeCondId_ {
    eCond_Et = 0x000,
    eCond_Ou = 0x001,
    eCond_Not = 0x002,
    eCond_XOr = 0x003,
    eCond_Equal = 0x004,
    eCond_Different = 0x005,
    eCond_Lesser = 0x006,
    eCond_Greater = 0x007,
    eCond_LesserOrEqual = 0x008,
    eCond_GreaterOrEqual = 0x009,
    eCond_CollidePersoZDDNoWithPerso = 0x00A,
    eCond_CollidePersoAllZDDWithPerso = 0x00B,
    eCond_CollidePersoZDDWithAnyPerso = 0x00C,
    eCond_CollidePersoZDENoWithPersoZDENo = 0x00D,
    eCond_CollidePersoZDENoWithPersoTypeZDE = 0x00E,
    eCond_CollidePersoTypeZDEWithPersoTypeZDE = 0x00F,
    eCond_CollidePersoAllZDEWithPersoAllZDE = 0x010,
    eCond_CollidePersoTypeZDEWithPersoAllZDE = 0x011,
    eCond_CollidePersoAllZDEWithPersoTypeZDE = 0x012,
    eCond_CollidePersoZDENoWithTypeZDE = 0x013,
    eCond_CollideWithGround = 0x014,
    eCond_CollideWithWall = 0x015,
    eCond_CollideWithNothing = 0x016,
    eCond_CollideWithCeiling = 0x017,
    eCond_CollideWithPerso = 0x018,
    eCond_CollideWithWater = 0x019,
    eCond_CollideWithThisPerso = 0x01A,
    eCond_ZDMCollideWithGround = 0x01B,
    eCond_ZDMCollideWithWall = 0x01C,
    eCond_ZDMCollideWithNothing = 0x01D,
    eCond_ZDMCollideWithCeiling = 0x01E,
    eCond_IsPersoInList = 0x01F,
    eCond_IsModelInList = 0x020,
    eCond_IsFamilyInList = 0x021,
    eCond_ListEmptyTest = 0x022,
    eCond_UserEvent_IsSet = 0x023,
    eCond_UserEvent_IsSet2 = 0x024,
    eCond_PressedBut = 0x025,
    eCond_JustPressedBut = 0x026,
    eCond_ReleasedBut = 0x027,
    eCond_JustReleasedBut = 0x028,
    eCond_IsTimeElapsed = 0x029,
    eCond_IsValidObject = 0x02A,
    eCond_IsValidWayPoint = 0x02B,
    eCond_IsValidGMT = 0x02C,
    eCond_IsValidVMT = 0x02D,
    eCond_IsValidAction = 0x02E,
    eCond_IsValidText = 0x02F,
    eCond_LSY_IsSpeechOver = 0x030,
    eCond_SeePerso = 0x031,
    eCond_IsActivable = 0x032,
    eCond_IsAlreadyHandled = 0x033,
    eCond_Alw_IsMine = 0x034,
    eCond_IsPersoLightOn = 0x035,
    eCond_IsPersoLightPulseOn = 0x036,
    eCond_IsPersoLightGyroPhareOn = 0x037,
    eCond_IsZDMCollideWithObstacle = 0x038,
    eCond_IsZDMCollideWithWall = 0x039,
    eCond_IsZDMCollideWithGround = 0x03A,
    eCond_IsZDMCollideWithCeiling = 0x03B,
    eCond_CmtIdentifierContainsMask = 0x03C,
    eCond_HitByCollider = 0x03D,
    eCond_IsTypeOfGMTCollide = 0x03E,
    eCond_IsInComport = 0x03F,
    eCond_IsInReflexComport = 0x040,
    eCond_IsInAction = 0x041,
    eCond_ChangeActionEnable = 0x042,
    eCond_EngineReinitRequested = 0x043,
    eCond_IsThereMechEvent = 0x044,
    eCond_CollisionWP = 0x045,
    eCond_IsCustomBitSet = 0x046,
    eCond_IsPersoActive = 0x047,
    eCond_CheckActionEnd = 0x048,
    eCond_IsCurrentStateCustomBitSet = 0x049,
    eCond_IsGiBlock = 0x04A,
    eCond_IsMechanicBlock = 0x04B,
    eCond_IsMechanicAnimation = 0x04C,
    eCond_IsMechanicCollide = 0x04D,
    eCond_IsMechanicGravity = 0x04E,
    eCond_IsMechanicTilt = 0x04F,
    eCond_IsMechanicGi = 0x050,
    eCond_IsMechanicClimb = 0x051,
    eCond_IsMechanicOnGround = 0x052,
    eCond_IsMechanicSpider = 0x053,
    eCond_IsMechanicShoot = 0x054,
    eCond_IsMechanicSwim = 0x055,
    eCond_IsMechanicNeverFall = 0x056,
    eCond_IsMechanicCollisionControl = 0x057,
    eCond_IsMechanicKeepSpeedZ = 0x058,
    eCond_IsMechanicSpeedLimit = 0x059,
    eCond_IsMechanicInertia = 0x05A,
    eCond_IsMechanicStream = 0x05B,
    eCond_IsMechanicStickOnPlatform = 0x05C,
    eCond_IsMechanicPatformCrash = 0x05D,
    eCond_IsMechanicScale = 0x05E,
    eCond_IsMechanicExec = 0x05F,
    eCond_CanFall = 0x060,
    eCond_IsMechanicCrash = 0x061,
    eCond_IsNullVector = 0x062,
    eCond_HierIsSonOfActor = 0x063,
    eCond_CheckAnimEnd = 0x064,
    eCond_HasTheCapability = 0x065,
    eCond_HasOneOfTheCapabilities = 0x066,
    eCond_HasTheCapabilityNumber = 0x067,
    eCond_PersoHasTheCapability = 0x068,
    eCond_PersoHasOneOfTheCapabilities = 0x069,
    eCond_PersoHasTheCapabilityNumber = 0x06A,
    eCond_NEstPasEnCollisionAvecMap = 0x06B,
    eCond_NEstPasEnCollisionAvecProjectile = 0x06C,
    eCond_NEstPasEnCollisionAvecSecondCharact = 0x06D,
    eCond_NEstPasEnCollisionAvecMainCharact = 0x06E,
    eCond_NEstPasEnCollisionAvecAutresSecteurs = 0x06F,
    eCond_IsInFamily = 0x070,
    eCond_IsInModel = 0x071,
    eCond_IsInDemoMode = 0x075,
    eCond_IsTooFar = 0x07A,
    eCond_Cam_IsActive = 0x07F,
    eCond_Cam_IsViewportOwner = 0x080,
    eCond_Cam_IsFlagNoDynamicTarget = 0x081,
    eCond_Cam_IsFlagNoAverageMoveTgtPerso = 0x082,
    eCond_Cam_IsFlagNoParseCutAngle = 0x083,
    eCond_Cam_IsFlagNoVisibility = 0x084,
    eCond_Cam_IsFlagNoVisibilityWithDynHie = 0x085,
    eCond_Cam_IsFlagNoDynChangeTheta = 0x086,
    eCond_Cam_IsFlagNoShiftUntilPosReached = 0x087,
    eCond_Cam_IsFlagNoDynSpeed = 0x088,
    eCond_Cam_IsFlagNoLinearParsing = 0x089,
    eCond_Cam_IsFlagNoLinearInertia = 0x08A,
    eCond_Cam_IsFlagNoAngularParsing = 0x08B,
    eCond_Cam_IsFlagNoAngularInertia = 0x08C,
    eCond_Cam_IsFlagNoTargetParsing = 0x08D,
    eCond_Cam_IsFlagNoTargetInertia = 0x08E,
    eCond_Cam_IsFlagNoObstacle = 0x08F,
    eCond_Cam_IsFlagFixedOrientation = 0x090,
    eCond_Cam_IsFlagForcedPosition = 0x091,
    eCond_Cam_IsFlagForcedTarget = 0x092,
    eCond_Cam_IsFlagForcedAxis = 0x093,
    eNbCond = 0x094
} tdeCondId_;

typedef enum tdeOperatorId_ {
    eOperator_ScalarPlusScalar = 0x000,
    eOperator_ScalarMinusScalar = 0x001,
    eOperator_ScalarMulScalar = 0x002,
    eOperator_ScalarDivScalar = 0x003,
    eOperator_ScalarUnaryMinus = 0x004,
    eOperator_PlusAffect = 0x005,
    eOperator_MinusAffect = 0x006,
    eOperator_MulAffect = 0x007,
    eOperator_DivAffect = 0x008,
    eOperator_PlusPlusAffect = 0x009,
    eOperator_MinusMinusAffect = 0x00A,
    eOperator_Affect = 0x00B,
    eOperator_Dot = 0x00C,
    eOperator_GetVectorX = 0x00D,
    eOperator_GetVectorY = 0x00E,
    eOperator_GetVectorZ = 0x00F,
    eOperator_VectorPlusVector = 0x010,
    eOperator_VectorMinusVector = 0x011,
    eOperator_VectorMulScalar = 0x012,
    eOperator_VectorDivScalar = 0x013,
    eOperator_VectorUnaryMinus = 0x014,
    eOperator_SetVectorX = 0x015,
    eOperator_SetVectorY = 0x016,
    eOperator_SetVectorZ = 0x017,
    eOperator_Ultra = 0x018,
    eOperator_ModelCast = 0x019,
    eOperator_Array = 0x01A,
    eOperator_AffectArray = 0x01B,
    eNbOperator = 0x01C
} tdeOperatorId_;

typedef enum tdeFuncId_ {
    eFunc_GetPersoAbsolutePosition = 0x000,
    eFunc_GetAngleAroundZToPerso = 0x002,
    eFunc_DistanceXToPerso = 0x004,
    eFunc_DistanceYToPerso = 0x005,
    eFunc_DistanceZToPerso = 0x006,
    eFunc_DistanceXYToPerso = 0x007,
    eFunc_DistanceXZToPerso = 0x008,
    eFunc_DistanceYZToPerso = 0x009,
    eFunc_DistanceXToPersoCenter = 0x00B,
    eFunc_DistanceYToPersoCenter = 0x00C,
    eFunc_DistanceZToPersoCenter = 0x00D,
    eFunc_DistanceXYToPersoCenter = 0x00E,
    eFunc_DistanceXZToPersoCenter = 0x00F,
    eFunc_DistanceYZToPersoCenter = 0x010,
    eFunc_DistanceToWP = 0x011,
    eFunc_GetWPAbsolutePosition = 0x012,
    eFunc_Int = 0x013,
    eFunc_RandomInt = 0x014,
    eFunc_Real = 0x015,
    eFunc_Sinus = 0x016,
    eFunc_Cosinus = 0x017,
    eFunc_Square = 0x018,
    eFunc_SquareRoot = 0x019,
    eFunc_RandomReal = 0x01A,
    eFunc_MinimumReal = 0x01B,
    eFunc_MaximumReal = 0x01C,
    eFunc_DegreeToRadian = 0x01D,
    eFunc_RadianToDegree = 0x01E,
    eFunc_AbsoluteValue = 0x01F,
    eFunc_LimitRealInRange = 0x020,
    eFunc_Sign = 0x021,
    eFunc_Cube = 0x022,
    eFunc_Modulo = 0x023,
    eFunc_TernInf = 0x024,
    eFunc_TernSup = 0x025,
    eFunc_TernEq = 0x026,
    eFunc_TernInfEq = 0x027,
    eFunc_TernSupEq = 0x028,
    eFunc_TernOp = 0x029,
    eFunc_TemporalRealCombination = 0x02A,
    eFunc_GetHitPoints = 0x02C,
    eFunc_AddAndGetHitPoints = 0x02D,
    eFunc_SubAndGetHitPoints = 0x02E,
    eFunc_GetHitPointsMax = 0x02F,
    eFunc_AddAndGetHitPointsMax = 0x030,
    eFunc_SubAndGetHitPointsMax = 0x031,
    eFunc_AbsoluteVector = 0x034,
    eFunc_RelativeVector = 0x035,
    eFunc_VecteurLocalToGlobal = 0x036,
    eFunc_VecteurGlobalToLocal = 0x037,
    eFunc_SPO_GetDrawFlag = 0x038,
    eFunc_GetTime = 0x039,
    eFunc_GetElapsedTime = 0x03A,
    eFunc_VitessePadAnalogique = 0x03E,
    eFunc_GenerateObject = 0x03F,
    eFunc_CountGeneratedObjects = 0x040,
    eFunc_GetGlobalCounter = 0x041,
    eFunc_GetSubMapId = 0x042,
    eFunc_AddColor = 0x043,
    eFunc_AddRed = 0x044,
    eFunc_AddGreen = 0x045,
    eFunc_AddBlue = 0x046,
    eFunc_AddAlpha = 0x047,
    eFunc_ColorRedGreenBlueAlpha = 0x048,
    eFunc_ColorRedGreenBlue = 0x049,
    eFunc_ColorRed = 0x04A,
    eFunc_ColorGreen = 0x04B,
    eFunc_ColorBlue = 0x04C,
    eFunc_ColorAlpha = 0x04D,
    eFunc_GetVisualGMTColor = 0x04E,
    eFunc_GetVisualGMTSpecularCoef = 0x04F,
    eFunc_GetVisualGMTSpecularExponent = 0x050,
    eFunc_GetVisualGMTDiffuseCoef = 0x051,
    eFunc_GetVisualGMTAmbientCoef = 0x052,
    eFunc_GetVisualGMTTextureScrollingCoefU = 0x053,
    eFunc_GetVisualGMTTextureScrollingCoefV = 0x054,
    eFunc_GetVisualGMTFrame = 0x055,
    eFunc_GetVisualGMTNumberOfFrames = 0x056,
    eFunc_LitPointsDeMagie = 0x068,
    eFunc_LitPointsDeMagieMax = 0x069,
    eFunc_AjouteEtLitPointsDeMagie = 0x06A,
    eFunc_AjouteEtLitPointsDeMagieMax = 0x06B,
    eFunc_EnleveEtLitPointsDeMagie = 0x06C,
    eFunc_EnleveEtLitPointsDeMagieMax = 0x06D,
    eFunc_LitPointsDair = 0x06E,
    eFunc_LitPointsDairMax = 0x06F,
    eFunc_AjouteEtLitPointsDair = 0x070,
    eFunc_AjouteEtLitPointsDairMax = 0x071,
    eFunc_EnleveEtLitPointsDair = 0x072,
    eFunc_EnleveEtLitPointsDairMax = 0x073,
    eFunc_PersoLePlusProche = 0x074,
    eFunc_PersoLePlusProcheDansSecteurCourant = 0x075,
    eFunc_NearerActorInFieldOfVision = 0x076,
    eFunc_NearerActorOfFamilyInFieldOfVision = 0x077,
    eFunc_NearerActorInFieldOfVisionWithOffset = 0x078,
    eFunc_NearerActorOfFamilyInFOVWithOffset = 0x079,
    eFunc_GetNbActivePerso = 0x07A,
    eFunc_CibleLaPlusProche = 0x07B,
    eFunc_CibleLaPlusProcheAvecAngles = 0x07C,
    eFunc_NetworkAffectTypeOfConnectedWP = 0x082,
    eFunc_NetworkAffectTypeOfConnectedWPWithCapa = 0x083,
    eFunc_ReseauCheminLePlusCourt = 0x084,
    eFunc_NetworkBuildOrderedPath = 0x085,
    eFunc_NetworkBuildOrderedPathCircular = 0x086,
    eFunc_NetworkAllocateGraphToMSWay = 0x087,
    eFunc_NetworkAllocateGraphToMSWayCircular = 0x088,
    eFunc_ReseauLitIndexCourant = 0x089,
    eFunc_ReseauForceIndexCourant = 0x08A,
    eFunc_ReseauLitPremierIndex = 0x08B,
    eFunc_ReseauLitDernierIndex = 0x08C,
    eFunc_ReseauIncrementIndex = 0x08D,
    eFunc_ReseauDecrementIndex = 0x08E,
    eFunc_ReseauLitWPAIndex = 0x08F,
    eFunc_ReseauLitCapaciteLiaisonAIndex = 0x090,
    eFunc_ReseauChangeCapaciteLiaisonAIndex = 0x091,
    eFunc_ReseauLitPoidsLiaisonAIndex = 0x092,
    eFunc_ReseauChangePoidsLiaisonAIndex = 0x093,
    eFunc_NetworkGetIndexOfWPInMSWay = 0x094,
    eFunc_NetworkForceWPToCurrent = 0x095,
    eFunc_NetworkTestTheEnds = 0x096,
    eFunc_ReseauLitCapaciteLiaisonDansGraph = 0x097,
    eFunc_ReseauChangeCapaciteLiaisonDansGraph = 0x098,
    eFunc_ReseauLitPoidsLiaisonDansGraph = 0x099,
    eFunc_ReseauChangePoidsLiaisonDansGraph = 0x09A,
    eFunc_NetworkGetTypeOfWP = 0x09B,
    eFunc_GetCapabilities = 0x09D,
    eFunc_CapabilityAtBitNumber = 0x09E,
    eFunc_GetScrollSpeed = 0x09F,
    eFunc_GetNbFrame = 0x0A0,
    eFunc_DotProduct = 0x0A1,
    eFunc_CrossProduct = 0x0A2,
    eFunc_Normalize = 0x0A3,
    eFunc_GetTractionFactor = 0x0A5,
    eFunc_GetCenterZDEType = 0x0A6,
    eFunc_GetCenterZDMType = 0x0A7,
    eFunc_GetCenterZDRType = 0x0A8,
    eFunc_GetCenterZDDType = 0x0A9,
    eFunc_GetCPUCounter = 0x0AC,
    eFunc_TurnToPosition = 0x0AD,
    eFunc_VitesseHorizontaleDuPerso = 0x0BD,
    eFunc_VitesseVerticaleDuPerso = 0x0BE,
    eFunc_GetPersoZoomFactor = 0x0BF,
    eFunc_GetPersoSighting = 0x0C0,
    eFunc_GetPersoHorizon = 0x0C1,
    eFunc_GetPersoBanking = 0x0C2,
    eFunc_LitPositionZDM = 0x0C3,
    eFunc_LitPositionZDE = 0x0C4,
    eFunc_LitPositionZDD = 0x0C5,
    eFunc_LitCentreZDM = 0x0C6,
    eFunc_LitCentreZDE = 0x0C7,
    eFunc_LitCentreZDD = 0x0C8,
    eFunc_LitAxeZDM = 0x0C9,
    eFunc_LitAxeZDE = 0x0CA,
    eFunc_LitAxeZDD = 0x0CB,
    eFunc_LitDimensionZDM = 0x0CC,
    eFunc_LitDimensionZDE = 0x0CD,
    eFunc_LitDimensionZDD = 0x0CE,
    eFunc_VecteurPointAxe = 0x0CF,
    eFunc_VecteurPointSegment = 0x0D0,
    eFunc_VectorContribution = 0x0D1,
    eFunc_VectorCombination = 0x0D2,
    eFunc_TemporalVectorCombination = 0x0D3,
    eFunc_ScaledVector = 0x0D4,
    eFunc_GetVectorNorm = 0x0D5,
    eFunc_RotateVector = 0x0D6,
    eFunc_VectorAngle = 0x0D7,
    eFunc_VectorCos = 0x0D8,
    eFunc_VectorSin = 0x0D9,
    eFunc_GetCollidePoint = 0x0DC,
    eFunc_GetCollidePoint2 = 0x0DD,
    eFunc_GetHandsCollidePoint = 0x0DE,
    eFunc_GetCollideRate = 0x0DF,
    eFunc_GetCollideRate2 = 0x0E0,
    eFunc_GetCollideMaterialType2 = 0x0E2,
    eFunc_GetCollisionPoint = 0x0E3,
    eFunc_GetCollisionVector = 0x0E4,
    eFunc_GetCollisionPerso = 0x0E5,
    eFunc_GetLastTraversedMaterialType = 0x0E7,
    eFunc_GetLastTraversedMaterial = 0x0E8,
    eFunc_GetCurrentCollidedGMT = 0x0E9,
    eFunc_GetColliderType = 0x0EA,
    eFunc_GetColliderActor = 0x0EB,
    eFunc_GetColliderVector = 0x0EC,
    eFunc_GetColliderReal = 0x0ED,
    eFunc_GetLastCollisionActor = 0x0EE,
    eFunc_ComputeRebondVector = 0x0EF,
    eFunc_GetModuleAbsolutePosition = 0x0F0,
    eFunc_GetModuleRelativePosition = 0x0F1,
    eFunc_GetModuleZoomFactor = 0x0F2,
    eFunc_CastIntegerToChannel = 0x0F4,
    eFunc_GetMechanicGravityFactor = 0x0F6,
    eFunc_GetMechanicSlide = 0x0F7,
    eFunc_GetMechanicRebound = 0x0F8,
    eFunc_GetMechanicSlopeLimit = 0x0F9,
    eFunc_GetMechanicInertiaX = 0x0FA,
    eFunc_GetMechanicInertiaY = 0x0FB,
    eFunc_GetMechanicInertiaZ = 0x0FC,
    eFunc_GetMechanicTiltIntensity = 0x0FD,
    eFunc_GetMechanicTiltInertia = 0x0FE,
    eFunc_GetMechanicTiltOrigin = 0x0FF,
    eFunc_GetMechanicMaxSpeed = 0x100,
    eFunc_GetMechanicStreamPriority = 0x101,
    eFunc_GetMechanicStreamSpeed = 0x102,
    eFunc_GetMechanicStreamFactor = 0x103,
    eFunc_GetSlideFactorX = 0x104,
    eFunc_GetSlideFactorY = 0x105,
    eFunc_GetSlideFactorZ = 0x106,
    eFunc_JumpImpulsion = 0x107,
    eFunc_GetSpeedAnim = 0x108,
    eFunc_GetActivationZDD = 0x10A,
    eFunc_GetActivationZDM = 0x10B,
    eFunc_GetActivationZDE = 0x10C,
    eFunc_GetActivationZDR = 0x10D,
    eFunc_GetCollisionFrequency = 0x10E,
    eFunc_GetBooleanInArray = 0x111,
    eFunc_GetNumberOfBooleanInArray = 0x112,
    eFunc_GetButtonName = 0x113,
    eFunc_GetDriversAvailable = 0x115,
    eFunc_GetCurrentLanguageId = 0x116,
    eFunc_GetNbLanguages = 0x117,
    eFunc_GetLanguageText = 0x118,
    eFunc_TextToInt = 0x119,
    eFunc_NameResolution = 0x122,
    eFunc_GetSaturationDistance = 0x125,
    eFunc_GetBackgroundDistance = 0x126,
    eFunc_Xor = 0x12B,
    eFunc_And = 0x12C,
    eFunc_Or = 0x12D,
    eFunc_Not = 0x12E,
    eFunc_DivUnsigned = 0x12F,
    eFunc_MulUnsigned = 0x130,
    eFunc_AddUnsigned = 0x131,
    eFunc_SubUnsigned = 0x132,
    eFct_Cam_GetShiftTarget = 0x135,
    eFct_Cam_GetShiftPos = 0x136,
    eFct_Cam_GetDistMin = 0x137,
    eFct_Cam_GetDistMax = 0x138,
    eFct_Cam_GetBoundDistMin = 0x139,
    eFct_Cam_GetBoundDistMax = 0x13A,
    eFct_Cam_GetAngleAlpha = 0x13B,
    eFct_Cam_GetAngleShiftAlpha = 0x13C,
    eFct_Cam_GetAngleTheta = 0x13D,
    eFct_Cam_GetAngleShiftTheta = 0x13E,
    eFct_Cam_GetLinearSpeed = 0x13F,
    eFct_Cam_GetLinearIncreaseSpeed = 0x140,
    eFct_Cam_GetLinearDecreaseSpeed = 0x141,
    eFct_Cam_GetAngularSpeed = 0x142,
    eFct_Cam_GetAngularIncreaseSpeed = 0x143,
    eFct_Cam_GetAngularDecreaseSpeed = 0x144,
    eFct_Cam_GetTargetSpeed = 0x145,
    eFct_Cam_GetTargetIncreaseSpeed = 0x146,
    eFct_Cam_GetTargetDecreaseSpeed = 0x147,
    eFct_Cam_GetFocal = 0x148,
    eFct_Cam_GetZMin = 0x149,
    eFct_Cam_GetZMax = 0x14A,
    eFct_Cam_GetTypeOfViewport = 0x14B,
    eFct_Cam_GetCameraOfViewport = 0x14D,
    eFct_Cam_GetMainCamera = 0x14E,
    eFct_Cam_ComputeTargetWithTgtPerso = 0x14F,
    eFct_Cam_GetCurrentTargetPosition = 0x151,
    eFct_Cam_GetBestPos = 0x152,
    eNbFunc = 0x153
} tdeFuncId_;

typedef enum tdeProcedureId_ {
    eProc_SetHitPoints = 0x001,
    eProc_SetHitPointsInit = 0x002,
    eProc_SetHitPointsToInitValue = 0x003,
    eProc_SetHitPointsToMaxValue = 0x004,
    eProc_AddHitPoints = 0x005,
    eProc_SubHitPoints = 0x006,
    eProc_SetHitPointsMax = 0x007,
    eProc_SetHitPointsMaxToInitValue = 0x008,
    eProc_SetHitPointsMaxToMaxValue = 0x009,
    eProc_AddHitPointsMax = 0x00A,
    eProc_SubHitPointsMax = 0x00B,
    eProc_TransparentDisplay = 0x00C,
    eProc_SetTransparency = 0x00D,
    eProc_ACT_SetDrawFlag = 0x00E,
    eProc_ModuleTransparentDisplay = 0x00F,
    eProc_ModuleTransparentDisplay2 = 0x010,
    eProc_SetModuleTransparency = 0x011,
    eProc_SetModuleTransparency2 = 0x012,
    eProc_ListAffectTypeZDEWithTypeZDE = 0x016,
    eProc_AddPersoInList = 0x017,
    eProc_AddPersoInListAt = 0x018,
    eProc_DeletePersoAtInList = 0x019,
    eProc_FindPersoAndDeleteInList = 0x01A,
    eProc_ListSelect = 0x01B,
    eProc_ListUnSelect = 0x01C,
    eProc_ListSort = 0x01D,
    eProc_ListSortByFamily = 0x01E,
    eProc_ListSortByModel = 0x01F,
    eProc_FillListWithAllPersoOfAFamily = 0x020,
    eProc_FillListWithAllPersoOfAModel = 0x021,
    eProc_DeleteFamilyInList = 0x022,
    eProc_DeleteModelInList = 0x023,
    eProc_ListUnion = 0x024,
    eProc_ListInter = 0x025,
    eProc_ListDiff = 0x026,
    eProc_ListAdd = 0x027,
    eProc_FogActivate = 0x028,
    eProc_SetFogColor = 0x029,
    eProc_SetFogNearFarInf = 0x02A,
    eProc_SetFogBlend = 0x02B,
    eProc_RestoreFog = 0x02C,
    eProc_SaveFog = 0x02D,
    eProc_SinEffect_SetFreq = 0x02E,
    eProc_SinEffect_SetAmplitude = 0x02F,
    eProc_SinEffect_SetState = 0x030,
    eProc_SinEffect_SetRLIAmplitude = 0x032,
    eProc_SinEffect_SetRLIBase = 0x033,
    eProc_SPO_SetDrawFlag = 0x034,
    eProc_SPO_SetEngineDisplayModeFlag = 0x035,
    eProc_DeactivateBut = 0x036,
    eProc_ActivateBut = 0x037,
    eProc_ChangeComport = 0x039,
    eProc_ChangeComportReflex = 0x03A,
    eProc_ChangeMyComport = 0x03B,
    eProc_ChangeMyComportReflex = 0x03C,
    eProc_ChangeMyComportAndMyReflex = 0x03D,
    eProc_ChangeActionRandomSmoothNL = 0x040,
    eProc_ChangeActionWithEventsSmoothNL = 0x041,
    eKeyWord_Boucle = 0x042,
    eKeyWord_FinBoucle = 0x043,
    eKeyWord_Break = 0x044,
    eProc_PlayerIsDead = 0x045,
    eProc_ResetDynamicsWithCard = 0x046,
    eProc_BreakAI = 0x047,
    eProc_IgnoreTraceFlagForNextPicking = 0x048,
    eProc_SetMainActor = 0x049,
    eProc_ActivateObject = 0x04A,
    eProc_DesactivateObject = 0x04B,
    eProc_ChangeMap = 0x04C,
    eProc_ChangeMapWithTransition = 0x04D,
    eProc_ChangeMapNoAutosave = 0x04E,
    eProc_SetActionReturn = 0x04F,
    eProc_FactorAnimationFrameRate = 0x050,
    eProc_SetCharacterPriority = 0x051,
    eProc_ForcePersoHandling = 0x052,
    eProc_PlayerIsDeadWithPlacement = 0x053,
    eProc_SwapLinkTableObjects = 0x054,
    eProc_ChangeCurrentObjectTable = 0x055,
    eProc_BuildObjectTableFromTableAndString = 0x056,
    eProc_TakeModuleControl = 0x057,
    eProc_TakeManyModulesControl = 0x058,
    eProc_ReleaseModuleControl = 0x059,
    eProc_QuitGame = 0x060,
    eProc_ActivateChannel = 0x061,
    eProc_DeactivateChannel = 0x062,
    eProc_SetPersoLightColor = 0x065,
    eProc_SetPersoLightNearFar = 0x066,
    eProc_SetPersoLightLittleBigAlpha = 0x067,
    eProc_SetPersoLightGyrophare = 0x068,
    eProc_SetPersoLightPulse = 0x069,
    eProc_SetPersoLightParalleleType = 0x06A,
    eProc_SetPersoLightSphericalType = 0x06B,
    eProc_SetPersoLightHotSpotType = 0x06C,
    eProc_SetPersoLightAmbientType = 0x06D,
    eProc_DefaultDisplay = 0x07E,
    eProc_Turn = 0x07F,
    eProc_Turn2 = 0x080,
    eProc_DeltaTurnPerso = 0x081,
    eProc_TurnPerso = 0x082,
    eProc_KillPerso = 0x083,
    eProc_KillPersoAndClearVariable = 0x084,
    eProc_SetVisualGMTColor = 0x085,
    eProc_SetVisualGMTSpecularExponent = 0x086,
    eProc_SetVisualGMTSpecularCoef = 0x087,
    eProc_SetVisualGMTDiffuseCoef = 0x088,
    eProc_SetVisualGMTAmbientCoef = 0x089,
    eProc_SetVisualGMTAsChromed = 0x08A,
    eProc_SetVisualGMTTextureScrollingCoef = 0x08B,
    eProc_LockVisualGMT = 0x08C,
    eProc_UnlockVisualGMT = 0x08D,
    eProc_SetVisualGMTFrame = 0x08E,
    eProc_OptionChangeDetailsValue = 0x08F,
    eProc_OptionChangeActionKey = 0x093,
    eProc_OptionConfigureKey = 0x094,
    eProc_ForcePersoAveragePosition = 0x097,
    eProc_SetFullPersoOrientation = 0x0A3,
    eProc_SetFullPersoOrientationWithOffset = 0x0A4,
    eProc_ZoomPerso = 0x0A5,
    eProc_ChangeOneCustomBit = 0x0A6,
    eProc_ChangeManyCustomBits = 0x0A7,
    eProc_ChangeManyCustomBitsExtended = 0x0A8,
    eProc_ImposeSpeed = 0x0A9,
    eProc_ImposeSpeedX = 0x0AA,
    eProc_ImposeSpeedY = 0x0AB,
    eProc_ImposeSpeedZ = 0x0AC,
    eProc_ImposeSpeedXY = 0x0AD,
    eProc_ImposeSpeedXYZ = 0x0AE,
    eProc_ImposeAbsoluteSpeed = 0x0AF,
    eProc_ImposeAbsoluteSpeedX = 0x0B0,
    eProc_ImposeAbsoluteSpeedY = 0x0B1,
    eProc_ImposeAbsoluteSpeedZ = 0x0B2,
    eProc_ImposeAbsoluteSpeedXY = 0x0B3,
    eProc_ImposeAbsoluteSpeedXYZ = 0x0B4,
    eProc_ProposeSpeed = 0x0B5,
    eProc_ProposeSpeedX = 0x0B6,
    eProc_ProposeSpeedY = 0x0B7,
    eProc_ProposeSpeedZ = 0x0B8,
    eProc_ProposeSpeedXY = 0x0B9,
    eProc_ProposeSpeedXYZ = 0x0BA,
    eProc_ProposeAbsoluteSpeed = 0x0BB,
    eProc_ProposeAbsoluteSpeedX = 0x0BC,
    eProc_ProposeAbsoluteSpeedY = 0x0BD,
    eProc_ProposeAbsoluteSpeedZ = 0x0BE,
    eProc_ProposeAbsoluteSpeedXY = 0x0BF,
    eProc_ProposeAbsoluteSpeedXYZ = 0x0C0,
    eProc_AddSpeed = 0x0C2,
    eProc_AddSpeedX = 0x0C3,
    eProc_AddSpeedY = 0x0C4,
    eProc_AddSpeedZ = 0x0C5,
    eProc_AddSpeedXY = 0x0C6,
    eProc_AddSpeedXYZ = 0x0C7,
    eProc_AddAbsoluteSpeed = 0x0C8,
    eProc_AddAbsoluteSpeedX = 0x0C9,
    eProc_AddAbsoluteSpeedY = 0x0CA,
    eProc_AddAbsoluteSpeedZ = 0x0CB,
    eProc_AddAbsoluteSpeedXY = 0x0CC,
    eProc_AddAbsoluteSpeedXYZ = 0x0CD,
    eProc_ReleaseAllModulesControl = 0x0CE,
    eProc_SetFullModuleOrientation = 0x0DB,
    eProc_SetFullModuleOrientationWithOffset = 0x0DC,
    eProc_ZoomModule = 0x0DD,
    eProc_SetColliderType = 0x0DE,
    eProc_SetColliderActor = 0x0DF,
    eProc_SetColliderVector = 0x0E0,
    eProc_SetColliderReal = 0x0E1,
    eProc_ResetLastCollisionActor = 0x0E2,
    eProc_ClearCollisionReport = 0x0E3,
    eProc_SetGoThroughMechanicsHandling = 0x0E4,
    eProc_EraseLastGoThroughMaterial = 0x0E5,
    eProc_StringAddChar = 0x0E6,
    eProc_StringReplaceChar = 0x0E7,
    eProc_StringRemoveChar = 0x0E8,
    eProc_IntToText = 0x0EB,
    eProc_SetMechanicAnimation = 0x0EC,
    eProc_SetMechanicCollide = 0x0ED,
    eProc_SetMechanicGravity = 0x0EE,
    eProc_SetMechanicTilt = 0x0EF,
    eProc_SetMechanicGi = 0x0F0,
    eProc_SetMechanicClimb = 0x0F1,
    eProc_SetMechanicOnGround = 0x0F2,
    eProc_SetMechanicSpider = 0x0F3,
    eProc_SetMechanicShoot = 0x0F4,
    eProc_SetMechanicSwim = 0x0F5,
    eProc_SetMechanicNeverFall = 0x0F6,
    eProc_SetMechanicNewMechanic = 0x0F7,
    eProc_SetMechanicCollisionControl = 0x0F8,
    eProc_SetMechanicKeepSpeedZ = 0x0F9,
    eProc_SetMechanicSpeedLimit = 0x0FA,
    eProc_SetMechanicInertia = 0x0FB,
    eProc_SetMechanicStream = 0x0FC,
    eProc_SetMechanicStickOnPlatform = 0x0FD,
    eProc_SetMechanicScale = 0x0FE,
    eProc_SetMechanicGravityFactor = 0x0FF,
    eProc_SetMechanicSlide = 0x100,
    eProc_SetMechanicRebound = 0x101,
    eProc_SetMechanicSlopeLimit = 0x102,
    eProc_SetMechanicInertiaX = 0x103,
    eProc_SetMechanicInertiaY = 0x104,
    eProc_SetMechanicInertiaZ = 0x105,
    eProc_SetMechanicInertiaXYZ = 0x106,
    eProc_SetMechanicTiltIntensity = 0x107,
    eProc_SetMechanicTiltInertia = 0x108,
    eProc_SetMechanicTiltOrigin = 0x109,
    eProc_SetMechanicSpeedMax = 0x10A,
    eProc_SetMechanicStreamPriority = 0x10B,
    eProc_SetMechanicStreamSpeed = 0x10C,
    eProc_SetMechanicStreamFactor = 0x10D,
    eProc_AddMechanicStreamSpeed = 0x10E,
    eProc_AddMechanicStreamSpeedList = 0x10F,
    eProc_MoveLimit = 0x110,
    eProc_MoveLimitX = 0x111,
    eProc_MoveLimitY = 0x112,
    eProc_MoveLimitZ = 0x113,
    eProc_MoveLimitXYZ = 0x114,
    eProc_StopMoveLimitX = 0x115,
    eProc_StopMoveLimitY = 0x116,
    eProc_StopMoveLimitZ = 0x117,
    eProc_StopMoveLimitXYZ = 0x118,
    eProc_SetPlatformLink = 0x119,
    eProc_FreePlatformLink = 0x11A,
    eProc_MecSetScale = 0x11B,
    eProc_MecSetSlideFactorXYZ = 0x11C,
    eProc_MecSetSlideFactorX = 0x11D,
    eProc_MecSetSlideFactorY = 0x11E,
    eProc_MecSetSlideFactorZ = 0x11F,
    eProc_MecSetClimbSpeedLimit = 0x120,
    eProc_MecSetHangingLimit = 0x121,
    eProc_MecSetHangingOff = 0x122,
    eProc_MecSetBaseCoeff = 0x123,
    eProc_MecSetWalkCoeff = 0x124,
    eProc_HierSetFather = 0x125,
    eProc_HierFreeFather = 0x126,
    eProc_HierListOfSon = 0x127,
    eProc_HierSetPlatformType = 0x128,
    eProc_HierControlLink = 0x129,
    eProc_HierFreezeEngine = 0x12A,
    eProc_FixePositionZDM = 0x12B,
    eProc_FixePositionZDD = 0x12C,
    eProc_FixePositionZDE = 0x12D,
    eProc_ChangeLightIntensity = 0x12E,
    eProc_ActChangeSpoFlag = 0x130,
    eProc_ChangeScreenSize = 0x131,
    eProc_ChangeScreenClip = 0x132,
    eProc_ChangeScreenRatio = 0x135,
    eProc_ForceDesactivationZDR = 0x142,
    eProc_ActiveZDR = 0x147,
    eProc_SetCollComputeFrequency = 0x14E,
    eProc_SetBrainComputeFrequency = 0x14F,
    eProc_SetLightComputeFrequency = 0x150,
    eProc_SetUsedExitIdentifier = 0x151,
    eProc_GetUsedExitIdentifier = 0x152,
    eProc_SetGlobalCounter = 0x153,
    eProc_ReadAnalogJoystick = 0x154,
    eProc_SetBooleanInArray = 0x161,
    eProc_SetDefaultFormatCharacter = 0x162,
    eProc_SelectResolution = 0x166,
    eProc_ChangeBrightness = 0x167,
    eProc_TexteEntier = 0x168,
    eProc_TexteReel = 0x169,
    eProc_SND_SaveSoundEventInSlotMemory = 0x16F,
    eProc_SND_RestoreSoundEventInSlotMemory = 0x170,
    eProc_ChangeSightingAccordingTime = 0x173,
    eProc_FixePointsDeMagie = 0x175,
    eProc_FixePointsDeMagieMax = 0x176,
    eProc_InitPointsDeMagie = 0x177,
    eProc_InitPointsDeMagieMax = 0x178,
    eProc_AjoutePointsDeMagie = 0x179,
    eProc_AjoutePointsDeMagieMax = 0x17A,
    eProc_EnlevePointsDeMagie = 0x17B,
    eProc_EnlevePointsDeMagieMax = 0x17C,
    eProc_FixePointsDair = 0x17D,
    eProc_FixePointsDairMax = 0x17E,
    eProc_InitPointsDair = 0x17F,
    eProc_InitPointsDairMax = 0x180,
    eProc_AjoutePointsDair = 0x181,
    eProc_AjoutePointsDairMax = 0x182,
    eProc_EnlevePointsDair = 0x183,
    eProc_EnlevePointsDairMax = 0x184,
    eProc_FixePositionFade = 0x185,
    eProc_FixeLumierePersoLocal = 0x186,
    eProc_FixeLumiereStaticOnOff = 0x187,
    eProc_FixeLumiereStaticNearFar = 0x188,
    eProc_FixeLumiereStaticColor = 0x189,
    eProc_ChangeTypeOfWP = 0x18E,
    eProc_SetCapabilities = 0x18F,
    eProc_AddCapabilities = 0x190,
    eProc_SubCapabilities = 0x191,
    eProc_ChangeCapabilities = 0x192,
    eProc_ReinitGraph = 0x193,
    eProc_FixeVitesseTexture = 0x194,
    eProc_ScrollingOnOff = 0x195,
    eProc_FixeDecalageTexture = 0x196,
    eProc_ScrollingPause = 0x197,
    eProc_ChangeCurrFrame = 0x198,
    eProc_ChangeRandomFrame = 0x199,
    eProc_FreezeAnim = 0x19A,
    eProc_UnFreezeAnim = 0x19B,
    eProc_SetSuperimposed2 = 0x19D,
    eProc_ReleaseSuperimposed = 0x19E,
    eProc_SwitchSuperimposedTab = 0x1A0,
    eProc_SetTractionFactor = 0x1A1,
    eProc_TurnToPositionSpeed = 0x1A3,
    eProc_SortArray = 0x1A4,
    eProc_ActiveJoystickAnal = 0x1A5,
    eProc_UnActiveJoystickAnal = 0x1A6,
    eProc_SetZDMSizeSphere = 0x1A7,
    eProc_SetZDDSizeCone = 0x1A8,
    eProc_ChangePersoSighting = 0x1B1,
    eProc_PlayDemo = 0x1B2,
    eProc_ChangeTooFarLimit = 0x1BA,
    eProc_ChangeTransparencyZone = 0x1BB,
    eProc_SetBaseFrequenceForMenu = 0x1BC,
    eProc_SaveSinusContext = 0x1BD,
    eProc_TextSuperImposed = 0x1BF,
    eProc_Cam_UpdatePosition = 0x1CA,
    eProc_Cam_ChangeShiftTarget = 0x1CB,
    eProc_Cam_ChangeShiftPos = 0x1CC,
    eProc_Cam_ChangeDistMin = 0x1CD,
    eProc_Cam_ChangeDistMax = 0x1CE,
    eProc_Cam_ChangeBoundDistMin = 0x1CF,
    eProc_Cam_ChangeBoundDistMax = 0x1D0,
    eProc_Cam_ChangeAngleAlpha = 0x1D1,
    eProc_Cam_ChangeAngleShiftAlpha = 0x1D2,
    eProc_Cam_ChangeAngleTheta = 0x1D3,
    eProc_Cam_ChangeAngleShiftTheta = 0x1D4,
    eProc_Cam_ChangeLinearSpeed = 0x1D5,
    eProc_Cam_ChangeLinearIncreaseSpeed = 0x1D6,
    eProc_Cam_ChangeLinearDecreaseSpeed = 0x1D7,
    eProc_Cam_ChangeAngularSpeed = 0x1D8,
    eProc_Cam_ChangeAngularIncreaseSpeed = 0x1D9,
    eProc_Cam_ChangeAngularDecreaseSpeed = 0x1DA,
    eProc_Cam_ChangeTargetSpeed = 0x1DB,
    eProc_Cam_ChangeTargetIncreaseSpeed = 0x1DC,
    eProc_Cam_ChangeTargetDecreaseSpeed = 0x1DD,
    eProc_Cam_ChangeFocal = 0x1DE,
    eProc_Cam_ChangeZMin = 0x1DF,
    eProc_Cam_ChangeZMax = 0x1E0,
    eProc_Cam_ChangeTgtPerso = 0x1E1,
    eProc_Cam_ChangeSecondTgtPerso = 0x1E2,
    eProc_Cam_ChangeChannel = 0x1E3,
    eProc_Cam_Activate = 0x1E4,
    eProc_Cam_AssociateViewport = 0x1E5,
    eProc_Cam_ResetIAFlags = 0x1E6,
    eProc_Cam_SetFlagNoDynamicTarget = 0x1E7,
    eProc_Cam_SetFlagNoAverageMoveTgtPerso = 0x1E8,
    eProc_Cam_SetFlagNoParseCutAngle = 0x1E9,
    eProc_Cam_SetFlagNoVisibility = 0x1EA,
    eProc_Cam_SetFlagNoVisibilityWithDynHie = 0x1EB,
    eProc_Cam_SetFlagNoDynChangeTheta = 0x1EC,
    eProc_Cam_SetFlagNoShiftUntilPosReached = 0x1ED,
    eProc_Cam_SetFlagNoDynSpeed = 0x1EE,
    eProc_Cam_ResetDNMFlags = 0x1EF,
    eProc_Cam_SetFlagNoLinearParsing = 0x1F0,
    eProc_Cam_SetFlagNoLinearInertia = 0x1F1,
    eProc_Cam_SetFlagNoAngularParsing = 0x1F2,
    eProc_Cam_SetFlagNoAngularInertia = 0x1F3,
    eProc_Cam_SetFlagNoTargetParsing = 0x1F4,
    eProc_Cam_SetFlagNoTargetInertia = 0x1F5,
    eProc_Cam_SetFlagFixedOrientation = 0x1F6,
    eProc_Cam_SetFlagNoObstacle = 0x1F7,
    eProc_Cam_SetFlagNoCollisionWhenNotMoving = 0x1F8,
    eProc_Cam_ChangeConstants = 0x1F9,
    eProc_Cam_SaveConstants = 0x1FA,
    eProc_Cam_RestoreConstants = 0x1FB,
    eProc_Cam_ShowInfo = 0x1FC,
    eProc_Cam_ForceTarget = 0x1FD,
    eProc_Cam_ForcePosition = 0x1FE,
    eProc_Cam_ForceRefAxis = 0x1FF,
    eProc_Cam_Reset = 0x200,
    eProc_Cam_ForceBestPos = 0x201,
    eProc_Cam_ForceNormalState = 0x202,
    eProc_Cam_ForceMovingOnRail = 0x203,
    eProc_Cam_SetCameraModeForEngine = 0x204,
    eProc_LaunchAGO = 0x205,
    eNbProcedure = 0x214
} tdeProcedureId_;

typedef enum tdeMetaActionId_ {
    eMetaAction_FrozenWait = 0x000,
    eMetaAction_ExecuteAction = 0x001,
    eMetaAction_WaitEndOfAction = 0x002,
    eMetaAction_WaitEndOfAnim = 0x003,
    eMetaAction_CamCineMoveAToBTgtC = 0x004,
    eMetaAction_CamCineMoveAToBTgtAC = 0x005,
    eMetaAction_CamCinePosATgtB = 0x006,
    eMetaAction_CamCinePosAMoveTgtBToC = 0x007,
    eMetaAction_CamCinePosATgtBTurnPosH = 0x008,
    eMetaAction_CamCinePosATgtBTurnTgtH = 0x009,
    eMetaAction_CamCinePosATgtBTurnPosV = 0x00A,
    eMetaAction_CamCinePosATgtBTurnTgtV = 0x00B,
    eNbMetaAction = 0x00C
} tdeMetaActionId_;

typedef enum tdeFieldId_ {
    eField_Position = 0x000,
    eField_Orientation = 0x001,
    eField_Speed = 0x002,
    eField_NormSpeed = 0x003,
    eField_PrevComportIntell = 0x004,
    eField_PrevComportReflex = 0x005,
    eField_PadGlobalVector = 0x006,
    eField_PadHorizontalAxis = 0x007,
    eField_PadVerticalAxis = 0x008,
    eField_PadAnalogForce = 0x009,
    eField_PadTrueAnalogForce = 0x00A,
    eField_PadRotationAngle = 0x00B,
    eField_PadSector = 0x00C,
    eNbField = 0x00D
} tdeFieldId_;

typedef enum tdeFieldTypeId_ {
    eFieldType_Integer = 0x000,
    eFieldType_Float = 0x001,
    eFieldType_Vector = 0x002,
    eFieldType_Comport = 0x003,
    eFieldType_0To255 = 0x004,
    eNbFieldType = 0x005
} tdeFieldTypeId_;

typedef struct RevoDsgVarInfo {
    u16 bufferOffset;
    u16 policyFlags;
    u8 typeIndex;
    u8 reinitThreshold;
} RevoDsgVarInfo;

typedef struct RevoDsgDescription {
    void* defaultValues;
    RevoDsgVarInfo* variables;
    u16 nominalBufferSize;
    u8 variableCount;
    u8 unknown0B;
} RevoDsgDescription;

typedef struct tdstScriptAI_ {
    void* p_stComport;
    u32 ulNbComport;
    void* ulNoComportInitDefault;
    u8 ucActionTableNbEntry;
} tdstScriptAI_;

typedef struct AI_tdstAIModel_ {
    tdstScriptAI_* p_stScriptAIIntell;
    tdstScriptAI_* p_stScriptAIReflex;
    RevoDsgDescription* p_stDsgVar;
    void* p_stListOfMacro;
    struct tdstComport_* inv_p_stInitComport;
    struct tdstComport_* inv_p_stDesinitComport;
} AI_tdstAIModel_;

typedef struct tdstDsgMem_ {
    RevoDsgDescription** p_p_stDsgVar;
    void* p_cDsgMemBufferInit;
    void* p_cDsgMemBuffer;
} tdstDsgMem_;

typedef void (*DsgVarAccessFn)(void* variable, u8 arrayIndex, void* param);
#ifdef PC_PORT
typedef s32 (*DsgVarGetFn)(void* variable, u8 arrayIndex, void* param);
#endif

typedef struct DsgTypeEntry {
    u8 typeCode;
#ifdef PC_PORT
     DsgVarGetFn get;
#else
    DsgVarAccessFn get;
#endif
    DsgVarAccessFn set;
} DsgTypeEntry;

typedef enum tdeDsgVarTypeId_ {
    eDsgVarType_Boolean = 0x00,
    eDsgVarType__128To127 = 0x01,
    eDsgVarType_0To255 = 0x02,
    eDsgVarType__32768To32767 = 0x03,
    eDsgVarType_0To65535 = 0x04,
    eDsgVarType_Integer = 0x05,
    eDsgVarType_PositiveInteger = 0x06,
    eDsgVarType_Float = 0x07,
    eDsgVarType_WayPoint = 0x08,
    eDsgVarType_Perso = 0x09,
    eDsgVarType_List = 0x0A,
    eDsgVarType_Vector = 0x0B,
    eDsgVarType_Comport = 0x0C,
    eDsgVarType_Action = 0x0D,
    eDsgVarType_Text = 0x0E,
    eDsgVarType_GameMaterial = 0x0F,
    eDsgVarType_VisualMaterial = 0x10,
    eDsgVarType_SoundEvent = 0x11,
    eDsgVarType_Caps = 0x12,
    eDsgVarType_Graph = 0x13,
    eDsgVarType_PersoArray = 0x14,
    eDsgVarType_VectorArray = 0x15,
    eDsgVarType_FloatArray = 0x16,
    eDsgVarType_IntegerArray = 0x17,
    eDsgVarType_WayPointArray = 0x18,
    eDsgVarType_TextArray = 0x19,
    eDsgVarType_SoundEventArray = 0x1A,
    eDsgVarType_SOLinks = 0x1B,
    inv_eDsgVarType_Module = 0x1C,
    inv_eDsgVarType_Array1D = 0x1D,
    eDsgVarType_SuperObject = 0x1E,
    eDsgVarType_Input = 0x1F,
    eNbDsgVarType = 0x20
} tdeDsgVarTypeId_;

#define LIST_MAX_VALUES 31

typedef struct tdstList_ {
    u8 ucNbElt;
    u8 ucMaxSize;
    HIE_tdstSuperObject_* values[LIST_MAX_VALUES];
} tdstList_;

typedef struct tdstIntelligence_ tdstIntelligence_;

typedef struct AI_tdstMind_ {
    void* p_stAIModel;
    tdstIntelligence_* p_stIntelligence;
    tdstIntelligence_* p_stReflex;
    tdstDsgMem_* p_stDsgMem;
    u8 bDoingIntel;
} AI_tdstMind_;

typedef struct tdstBrain_ {
    AI_tdstMind_* p_stMind;
    struct GMT_tdstGameMaterial_* hLastCollidedGoThroughMaterial;
} tdstBrain_;

typedef struct tdstActionTableEntry_ tdstActionTableEntry_;

typedef struct tdstActionTable_ {
    tdstActionTableEntry_* p_stEntry;
    u8 ucNbEntries;
    u8 ucNbEntriesUsed;
    u8 ucCurrentEntry;
} tdstActionTable_;

struct tdstIntelligence_ {
    void** p_p_stScriptAI;
    void* p_stCurrentSchedule;
    void* p_stCurrentComport;
    void* p_stPrevComport;
    tdstActionTable_* p_stActionTable;
    void* initialBehavior;
};

typedef struct tdstNodeInterpret_ {
    u32 param;
    u16 uwNodeToSkip;
    u8 ucDepth;
    u8 eType;
} tdstNodeInterpret_;

typedef struct tdstComport_ {
    tdstNodeInterpret_** p_stRules;
    tdstNodeInterpret_* p_stSchedule;
    u8 ucNbRules;
} tdstComport_;

typedef union gu_tduGetSetParamValue_ {
    s32 lValue;
    u32 ulValue;
    s16 wValue;
    u16 uwValue;
    s8 cValue;
    u8 ucValue;
    f32 xValue;
    MTH3D_tdstVector_ stVector;
    struct HIE_tdstSuperObject_* hSuperObject;
    struct WP_tdstWayPoint_* m_hWayPoint;
    struct WP_tdstGraph_* hGraph;
    tdstComport_* pComport;
    tdstState_* tdxstHandleToState;
    void* pvValue;
    u8 aucValue[4];
} gu_tduGetSetParamValue_;

typedef struct tdstActionParam_ {
    gu_tduGetSetParamValue_ a_uParam[8];
} tdstActionParam_;

struct tdstActionTableEntry_ {
    tdstActionParam_ stActionParam;
    tdstNodeInterpret_* p_stNode;
    u8 bUsed;
    u8 ucNumRule;
    u8 bUseDefaultActionReturn;
    u8 ucNewActionReturn;
};

typedef struct tdstGetSetParam_ {
    gu_tduGetSetParamValue_ u;
    u8 ucType;
} tdstGetSetParam_;

extern tdstGetSetParam_* inv_AI_parameterPoolCursor;

struct HIE_tdstSuperObject_;

typedef void (*AIFieldAccessFn)(struct HIE_tdstSuperObject_* superObject, tdstGetSetParam_* param);

typedef struct AIFieldDescriptor {
    u8 valueType;
    AIFieldAccessFn get;
    AIFieldAccessFn set;
} AIFieldDescriptor;

static inline f32 inv_AI_fn_xAiParamFloat(const tdstGetSetParam_* param)
{
    if (param->ucType == E_vt_Float)
        return param->u.xValue;
    return (f32) param->u.lValue;
}

static inline AI_tdstAIModel_* inv_AI_fn_p_stModelOf(AI_tdstMind_* mind)
{
    return mind->p_stAIModel;
}

static inline AI_tdstMind_* fn_p_stBrainGetMind(tdstBrain_* brain)
{
    return brain->p_stMind;
}

static inline AI_tdstMind_* inv_AI_fn_p_stGetMindSO(HIE_tdstSuperObject_* hSupObj)
{
    tdstBrain_* brain = inv_HIE_fn_p_stGetEngineObject(hSupObj)->h_Brain;

    return fn_p_stBrainGetMind(brain);
}

extern void* fn_p_stEvalTree(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                             tdstGetSetParam_* out);

extern tdstNodeInterpret_* fn_p_stSkipThisArgument(tdstNodeInterpret_* p_stTree);
extern s32 fn_bIntelligenceEngine(HIE_tdstSuperObject_* p_SuperObjPerso,
                                  tdstIntelligence_* p_stIntelligence);
extern void inv_AI_fn_vEvalInitComport(HIE_tdstSuperObject_* superObject);
struct tdstEngineObject_;
extern void func_001DEE58(HIE_tdstSuperObject_* superObject, struct tdstEngineObject_* unused);
extern tdstNodeInterpret_* fn_p_stSwapNextParameter(tdstNodeInterpret_* p_stTree);
extern tdstNodeInterpret_* fn_p_stSwapNextParameter2(tdstNodeInterpret_* p_stTree);
extern tdstNodeInterpret_* fn_p_stIntelligenceEvalTreeEngine(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree, tdstGetSetParam_* p_stValue);

#define AI_NODE_ID(stream) (*(u16*) ((u8*) (stream) - 8))

struct tdstEngineObject_;
extern void func_001DDDD0(struct tdstEngineObject_* object);
extern AI_tdstMind_* fn_pstAllocMind(void);
extern s32 fn_bDoIntelligencePerso(HIE_tdstSuperObject_* p_SuperObjPerso);
extern s32 fn_bDoReflexPerso(HIE_tdstSuperObject_* p_SuperObjPerso);
extern void fn_vReinitAI(AI_tdstMind_* p_stMind);
extern void fn_vSimulateMindAllocationFromAnotherMind(AI_tdstMind_* p_stMindSrc,
                                                      AI_tdstMind_* p_stMindDst);
extern void func_001DDFF8(tdstEngineObject_* object);
extern void fn_vInitAI(s32 unused);
extern void fn_vReinitAIWithFlags(AI_tdstMind_* p_stMind, u8 eObjIniType);
extern void fn_vSecondPassOfMindLoading(AI_tdstMind_* p_stMind);

extern void fn_ucGetDsgVar(s32 variableIndex, s32 arrayIndex, AI_tdstMind_* mind,
                           tdstGetSetParam_* param);
extern void fn_ucSetDsgVar(s32 variableIndex, s32 arrayIndex, AI_tdstMind_* mind, void* param);

extern HIE_tdstSuperObject_* g_hSuperObjectHit;

struct DNM_stReport_;
extern struct DNM_stReport_* fn_pstGetDNMReport(HIE_tdstSuperObject_* p_SuperObjPerso);
extern f32 fn_xComputeBestEcartDirectionAngle(f32 xAngleFollowed, f32 xAngleToFollow);
extern f32 fn_xRealModuloAround(f32 xFrom, f32 xModulus, f32 xValue);
extern u8 fn_ucAreThereFloatIfYesConvertThem(tdstGetSetParam_* p_stParam1,
                                             tdstGetSetParam_* p_stParam2);
struct GEO_tdstColor_;
extern void fn_vConvertParamToColor(tdstGetSetParam_* p_stParam, struct GEO_tdstColor_* p_stColor);
extern char* fn_szGetStringFromTextOrStringParam(tdstGetSetParam_* p_stValue);
extern f32 fn_xDistanceBetweenPersoAndWP(HIE_tdstSuperObject_* p_SuperObjPerso, void* hWP);
extern void fn_vConvertColorToParam(struct GEO_tdstColor_* p_stColor, tdstGetSetParam_* p_stParam);
extern s32 fn_ucInternalTurn(HIE_tdstSuperObject_* object, f32 xAngle);
extern void fn_ucIsThereFloatIfYesConvertItToInteger(tdstGetSetParam_* p_stGetSetParam);
extern f32 fn_xComputeDirectionAngle(MTH3D_tdstVector_* p_stVector);
extern void fn_vGetCenterOfBoundingVolume(HIE_tdstSuperObject_* hPerso, MTH3D_tdstVector_* p_stVertex);

extern void* fn_p_stOperatorGetPerso(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree,
                                     HIE_tdstSuperObject_** hSuperObjPersoFound);
extern void fn_vGetUltraOperatorPerso(HIE_tdstSuperObject_* hCurrentSuperObjPerso,
                                      HIE_tdstSuperObject_** hNewSuperObjPerso);
extern void* inv_fn_p_stSetHitPoints(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                     tdstEngineObject_* object);
extern void* inv_fn_p_stAddHitPoints(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                     tdstEngineObject_* object);
extern void* inv_fn_p_stSubHitPoints(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                     tdstEngineObject_* object);
extern void* inv_fn_p_stSetHitPointsMax(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                        tdstEngineObject_* object);
extern void* inv_fn_p_stAddHitPointsMax(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                        tdstEngineObject_* object);
extern void* inv_fn_p_stSubHitPointsMax(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream,
                                        tdstEngineObject_* object);
extern void fn_vSetUltraOperatorPerso(HIE_tdstSuperObject_* hCurrentSuperObjPerso,
                                      HIE_tdstSuperObject_* hNewCurrentSuperObjPerso);
extern tdstNodeInterpret_* inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
    HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream, tdstGetSetParam_* param,
    HIE_tdstSuperObject_** perso, u8* index, u8* subIndex, u8* component, u16* field, u8* kind);

extern void fn_ucSetDsgVar(s32 ucId, s32 ucIndex, AI_tdstMind_* p_stPerso, void* p_stGetSetParam);
extern s32 fn_ulSizeOfDsgVar(u8 eDsgVarTypeId, u8 ucNbElt);

extern MTH3D_tdstVector_* fn_p_stGetActionParamp_stVertex(u8 ucN,
                                                          HIE_tdstSuperObject_* p_SuperObjPerso);
extern f32 fn_xGetActionParamxValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso);
extern tdstState_* fn_xGetActionParamStateValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso);
extern void fn_vSetActionParamStateValue(s32 index, HIE_tdstSuperObject_* superObject,
                                         tdstState_* hState);
extern void fn_vSetActionParamlValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso, s32 lValue);
extern void fn_vSetActionParamp_stVertex(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso,
                                         MTH3D_tdstVector_* p_stVertex);
extern void fn_vSetActionParamxValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso, f32 xValue);
extern s32 fn_lGetActionParamlValue(s32 ucN, HIE_tdstSuperObject_* p_SuperObjPerso);

extern s32 fn_ucIsRuleInActionTable(tdstIntelligence_* p_stIntelligence, u8 ucNumRule);
extern void fn_vUnuseAllRulesFromSchedule(tdstIntelligence_* p_stIntelligence);
extern void fn_vUnuseAllRulesWhichHaveGreaterNumRule(tdstIntelligence_* p_stIntelligence,
                                                     u8 ucNumRule);
extern void fn_vUnuseCurrentActionTableEntry(tdstIntelligence_* p_stIntelligence);
extern void fn_vUseCurrentActionTableEntry(tdstIntelligence_* p_stIntelligence, u8 ucNRule,
                                           tdstNodeInterpret_* p_stNode);
extern void fn_vInitActionTable(tdstIntelligence_* p_stIntelligence);
extern u8 fn_ucFindPlaceInActionTable(tdstIntelligence_* p_stIntelligence);
extern u8 fn_ucIsRuleAndNodeInActionTable(tdstIntelligence_* p_stIntelligence, u8 ucNumRule, tdstNodeInterpret_* p_stNode);

extern tdstNodeInterpret_* fn_p_stDefaultActionInit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                    tdstNodeInterpret_* p_stTree);

extern void fn_ucGetFieldValue(u16 eFieldId, HIE_tdstSuperObject_* hSuperObjPerso,
                               tdstGetSetParam_* p_stGetSetParam);
extern void fn_ucSetFieldValue(u16 eFieldId, HIE_tdstSuperObject_* hSuperObjPerso,
                               tdstGetSetParam_* p_stGetSetParam);

extern void fn_vReinitComport(tdstIntelligence_* p_stIntelligence);
extern u8 fn_ucGetTreeNbParam(tdstNodeInterpret_* p_stTree);
extern void fn_ucChangeComportIntell(tdstIntelligence_* p_stIntelligence,
                                     tdstComport_* p_stComport);

extern WPArcList* WPARCLST_fn_hInit(void);
extern s32 WPARCLST_fn_lChangeCapabilities(WPArcList* hListOfArc, void* hNode, u32 ulCapability,
                                           u32 lValue);
extern s32 WPARCLST_fn_lChangeWeight(WPArcList* hListOfArc, void* hNode, s32 lWeight);
extern s32 WPARCLST_fn_lGetCapabilities(WPArcList* hListOfArc, void* hNode, u32* p_ulCapability);
extern s32 WPARCLST_fn_lGetWeight(WPArcList* hListOfArc, void* hNode, s32* p_lWeight);
extern void WPARCLST_fn_vAdd(WPArcList* hListOfArc, void* hNode, s32 lWeight, u32 ulCapability);
extern void WPARCLST_fn_vDestroy(WPArcList* hList);

extern WPArcNode* func_00202418(void* destination, s32 weight, u32 capabilities);
extern s32 func_00202480(WPArcNode* arc);

extern WP_tdstGraph_* WPG_fn_lBuildOrderedPath(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hStartWP);
extern WP_tdstGraph_* WPG_fn_lShortestPath(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                           WP_tdstWayPoint_* hDstWayPoint, u32 ulCapability);
extern WP_tdstWayPoint_* WPG_fn_hGetWaypointOfGraph(WP_tdstGraph_* hGraph, s32 iIndex,
                                                    s32* _plTypeOfWP);
extern void WPG_fn_vDestroy(WP_tdstGraph_* hGraph);
extern s32 WPG_fn_lChangeLinkCapabilities(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                          WP_tdstWayPoint_* hDstWayPoint, u32 ulCapability,
                                          u32 lValue);
extern s32 WPG_fn_lChangeLinkWeight(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                    WP_tdstWayPoint_* hDstWayPoint, s32 lWeight);
extern s32 WPG_fn_lGetLinkCapabilities(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                       WP_tdstWayPoint_* hDstWayPoint, u32* p_ulCapability);
extern s32 WPG_fn_lGetLinkWeight(WP_tdstGraph_* hGraph, WP_tdstWayPoint_* hSrcWayPoint,
                                 WP_tdstWayPoint_* hDstWayPoint, s32* p_lWeight);
extern void WPG_fn_vReinitGraph(WP_tdstGraph_* hGraph);

extern void fnv_MoveAToB(HIE_tdstSuperObject_* superObject, MTH3D_tdstVector_* start,
                          MTH3D_tdstVector_* end, MTH3D_tdstVector_* current, s32 duration,
                          s32 ramp);

extern AIFieldAccessFn fn_p_fn_pGetFieldGetFunctionPtr(u16 eFieldId);
extern AIFieldAccessFn fn_p_fn_pGetFieldSetFunctionPtr(u16 eFieldId);
extern s8* func_001E6F40(u16 index);

extern u8 fn_GetDsgVarType(u8 type);

extern void fn_vComputeCollideResult(HIE_tdstSuperObject_* superObject,
                                     HIE_tdstSuperObject_* sectorObject);

extern void* AI_fn_p_vTrueAlloc(void* heap, s32 size);

extern void inv_AI_fn_vClearTextLabels(u32 a0);
extern void inv_AI_fn_vClearTextLabelsOfObject(HIE_tdstSuperObject_* key);
extern void inv_AI_fn_vSetTextLabelUseScale(HIE_tdstSuperObject_* key, s32 enabled);
extern void inv_AI_fn_vSetTextLabelScale(HIE_tdstSuperObject_* key, f32 value);

extern s32 fn_ucAtLeastOnePersoInZdd(HIE_tdstSuperObject_* p_stPerso, u8 ucModulOrCharact);
extern s32 fn_ucAtLeastOnePersoInZde(HIE_tdstSuperObject_* p_stPerso, u8 ucModulOrCharact,
                                     u16 xMask);
extern s32 fn_bFindFamilyInList(tdstList_* p_List, tdstFamilyListView* hFamily);
extern s32 fn_bFindModelInList(tdstList_* p_List, void* _p_stModel);
extern s32 fn_bFindPersoInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso);
extern void fn_vAddList(tdstList_* p_ListA, tdstList_* p_ListB);
extern void fn_vAffectTypeZdeWithTypeZdeInList(tdstList_* p_List, HIE_tdstSuperObject_* p_stPerso,
                                               u16 xPersoMask, u16 xOthersMask);
extern void fn_vDiffList(tdstList_* p_ListA, tdstList_* p_ListB, tdstList_* p_ListC);
extern void fn_vInterList(tdstList_* p_ListA, tdstList_* p_ListB, tdstList_* p_ListC);
extern void fn_vUnionList(tdstList_* p_ListA, tdstList_* p_ListB, tdstList_* p_ListC);
extern s32 fn_bFindPersoAndDeleteInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso);
extern tdstNodeInterpret_* fn_p_stListSelect(tdstList_* p_List, s8 b_cInverse, tdstNodeInterpret_* p_stTree);
extern tdstNodeInterpret_* fn_p_stListSort(tdstList_* p_List, u8 ucWantedNbElt, s8 b_cIncrease, tdstNodeInterpret_* p_stTree);
extern void fn_vDeletePersoAtPositionInList(tdstList_* p_List, u8 ucPosition);
extern void fn_vInsertPersoAtPositionInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso, u8 ucPosition);
extern void fn_vPutInList(tdstList_* p_List, HIE_tdstSuperObject_* p_Perso);
extern void fn_vSortListByFamily(tdstList_* p_List, tdstFamilyListView* hFamily);
extern void fn_vSortListByModel(tdstList_* p_stList, void* _p_stModel);
extern HIE_tdstSuperObject_* fn_p_stGetPersoInList(tdstList_* p_List, s32 ucPosition);

extern f32 WP_fnx_WayPoint_GetRadius(WP_tdstWayPoint_* _hWayPoint);
extern void WP_fnv_WayPoint_GetVertex(WP_tdstWayPoint_* _hWayPoint, MTH3D_tdstVector_* _pstVector);
extern void WP_fnv_WayPoint_ComputeLocation(WP_tdstWayPoint_* _hWayPoint,
                                            MTH3D_tdstVector_* _p_stVertex);

extern u8 g_ucNewActionReturn;
extern u8 g_ucUseDefaultActionReturn;
extern MTH3D_tdstVector_ g_stCollideResultPoint[];
extern MTH3D_tdstVector_ g_stCollideResultVector[];
extern s32 inv_HIE_ulPickExcludeFlags;
extern MTH3D_tdstVector_ g_stCollideParameterPoint[];
extern MTH3D_tdstVector_ g_stCollideParameterVector[];
extern s32 g_xMask;
extern s32 g_ulMode;
extern s32 g_ulOverRideRayTraceSpoMask;
extern u8 ucProcedureActionReturn;
extern f32 GLI_xFogNear;
extern f32 GLI_xFogFar;
extern f32 GLI_xFogInfinite;
extern f32 GLI_xFogBlendNear;
extern f32 GLI_xFogBlendFar;
extern f32 TEXT_gaf_FloatParameters[];
extern f32 a333;
extern s32 TEXT_gai_IntParameters[];
extern u8 g_bInGameMenu;
extern struct GEO_tdstColor_ GLI_stFogColor;
extern s8 GLI_cGlobalFogIsOn;

typedef struct tdstExtendDatas4Ray_ {
    s32 iMagicPoints4Ray;
    s32 iMagicPointsMax4Ray;
    f32 xAirPoints4Ray;
    f32 xAirPointsMax4Ray;
} tdstExtendDatas4Ray_;

extern tdstExtendDatas4Ray_ stExtendDatas4Ray;

extern JoystickPads qword_383BD0[];

extern void func_001E7348(HIE_tdstSuperObject_* sender);

extern DsgTypeEntry a_stDsgVarTypeTable[];

extern AIFieldDescriptor a_stFieldTable[];

extern s32 iNumLoop;
extern u8 inv_GAM_bMenuCharactersOnly;

extern s32 dword_38419C[];

#endif
