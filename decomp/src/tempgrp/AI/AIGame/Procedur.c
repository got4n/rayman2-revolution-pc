#include "common.h"

#include "AI/AI.h"
#include "CAM/CAM.h"
#include "COL/COL.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "WP/WP.h"
#include "SCT/SCT.h"
#include "snd/snd.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"

#include "include_asm.h"
#ifdef PC_PORT
f32 powf(f32 x, f32 y);
static f32 pc_curve_powf(f32 x, f32 ticks, s32 curve) {
    f32 r = powf(__builtin_fabsf(x), ticks);
    return (x < 0.0f && (curve == 1 || curve == 3 || curve == 5)) ? -r : r;
}
#endif
#include "IPT/IPT.h"
#include "GMT/GMT.h"
#include "GEO/GEO.h"
#include "clib/clib.h"
#include "AGO/AGO.h"
#include "SHW/SHW.h"
#include "ps2/PS2.h"

extern void* fn_hGetGeometricZdxOfTypeAtIndex(s32 type, s32 index, void* target);

extern HIE_tdstSuperObject_* gp_stDynamicWorld;




extern s32 GlobalLoopInit[];

extern s32 GlobalLoopCpt[];

extern s32 GlobalLoopTree[];

extern u8 inv_AI_ucTextSuperImposed;

extern u8 g_ucNewActionReturn;

extern u8 g_ucUseDefaultActionReturn;

extern u8 inv_GLI_savedFogEnabled;

f32 acosf(f32);
f32 acosf(f32);

f32 tanf(f32);
f32 tanf(f32);

s32 inv_SND_fn_lSendStopRequestSound(s32, s32, s32, s32, s32, s32);

s32 fni_CompareFloatLess(f32*, f32*);

s32 fni_CompareIntegerLess(s32*, s32*);

s32 fni_CompareIntegerGreater(s32*, s32*);

s32 fni_CompareFloatGreater(f32*, f32*);

void GLI_xSetLightState(void*, u8);

void atol(char*);

void fn_vAffectZddInList(void*, void*, s16);

void fn_vAffectZdeInList(void*, void*, s16, u16);

void fn_vInternalChangeComportProcedure(tdstIntelligence_*, tdstComport_*, u32);

void inv_POS_fn_vScale(void*, void*);

void inv_SAV_fn_vSetCurrentSlotIndex(u16);

void qsort(void*, s32, s32, void*);

extern s32 HIE_gs_lCurrentFrame;

extern void POS_fn_vGetRotationVector(void* matrix, MTH3D_tdstVector_* vector);

extern void POS_fn_vGetYAxisVector(void* matrix, MTH3D_tdstVector_* axis);

extern f32 cosf(f32);

static const MTH3D_tdstVector_ qword_38FE78;

extern f32 powf(f32, f32);

extern f32 D_00393DDC;

extern f32 D_00393DE0;

static __inline__ f32 inv_fn_xJoystickSign(f32 x)
{
    union {
        f32 f;
        u32 i;
    } u;

    u.f = x;
    if ((u.i & 0x7FFFFFFF) == 0) {
        return 0.0f;
    }
    u.i = (u.i & 0x80000000) | 0x3F800000;
    return u.f;
}

static __inline__ void inv_fn_vJoystickTransformVector(MTH3D_tdstVector_* dst, MTH3D_tdstMatrix_* m,
                                                       MTH3D_tdstVector_* v)
{
    MTH3D_M_vSetXYZ_inv(
        dst, inv_MTH_fn_xMulAdd3(m->stCol_0.xX, v->xX, m->stCol_1.xX, v->xY, m->stCol_2.xX, v->xZ),
        inv_MTH_fn_xMulAdd3(m->stCol_0.xY, v->xX, m->stCol_1.xY, v->xY, m->stCol_2.xY, v->xZ),
        inv_MTH_fn_xMulAdd3(m->stCol_0.xZ, v->xX, m->stCol_1.xZ, v->xY, m->stCol_2.xZ, v->xZ));
}

tdstNodeInterpret_* fn_p_stMiscUltraProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                              tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* handle;
    u16 op;
    s32 bits;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &handle);
    switch (op) {
    case eProc_SetCharacterPriority: {
        tdstEngineObject_* node51;
        s32 raw51;
        s32 bits51;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        node51 = handle->hLinkedObject;
        if (stParam.ucType == E_vt_Float) {
            raw51 = inv_MTH_fn_lToIntegerOrdered(stParam.u.xValue);
            __asm__ volatile("");
            bits51 = raw51;
        } else {
            bits51 = *(u8*) &stParam.u;
        }
        node51->h_CollSet->ucCharacterPriority = bits51;
        return p_stTree;
    }

    case eProc_EraseLastGoThroughMaterial: {
        tdstEngineObject_* nodeE5;
        tdstBrain_* objE5;
        nodeE5 = handle->hLinkedObject;
        objE5 = nodeE5->h_Brain;
        objE5->hLastCollidedGoThroughMaterial = (struct GMT_tdstGameMaterial_*) 0xCDCDCDCD;
        return p_stTree;
    }

    case eProc_FactorAnimationFrameRate: {
        tdstEngineObject_* node50;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        node50 = handle->hLinkedObject;
        if (stParam.ucType == E_vt_Float) {
            bits = inv_MTH_fn_lToIntegerOrdered(stParam.u.xValue);
        } else {
            bits = stParam.u.lValue;
        }
        fn_v3dDataFactorFrameRate(node50->h_3dData, bits);
        return p_stTree;
    }

    case eProc_SetGoThroughMechanicsHandling: {
        tdstEngineObject_* nodeE4;
        tdstDynam_* midE4;
        DNM_stDynamics* dstE4;
        u8 valE4;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        nodeE4 = handle->hLinkedObject;
        midE4 = nodeE4->h_Dynam;
        dstE4 = midE4->p_stDynamics;
        valE4 = *(u8*) &stParam.u;
        dstE4->m_tdstDynamicsBaseBlock.collisionPolicy = valE4;
        return p_stTree;
    }

    default:
        return p_stTree;
    }
}

s32 fn_p_stMiscNoProcedure(struct HIE_tdstSuperObject_* p_SuperObjPerso, s16* p_stTree)
{
    s32 procedure;
    tdstIntelligence_* p_stIntelligence;
    AI_tdstMind_* p_stMind;

    procedure = AI_NODE_ID(p_stTree);
    switch (procedure) {
    case eProc_IgnoreTraceFlagForNextPicking:
        g_ulOverRideRayTraceSpoMask = 0;
        return (s32) p_stTree;
    case eProc_BreakAI:
        p_stMind = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Brain->p_stMind;
        p_stIntelligence =
            p_stMind->bDoingIntel ? p_stMind->p_stIntelligence : p_stMind->p_stReflex;
        {
            tdstComport_* p_stComport = p_stIntelligence->p_stCurrentComport;
            fn_vInitActionTable(p_stIntelligence);
            p_stIntelligence->p_stCurrentSchedule = p_stComport->p_stSchedule;
        }
        ucProcedureActionReturn = 0xB;
        iNumLoop = 0;
        return (s32) p_stTree;
    case eProc_ResetDynamicsWithCard: {
        tdstDynam_* h_Dynam = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Dynam;
        if (h_Dynam != 0) {
            DNM_stDynamics* p_stDynamics = h_Dynam->p_stDynamics;
            void* idCard = fn_h_GetCurrentDNMIdCard(p_SuperObjPerso);
            if (idCard != 0)
                DNM_fn_vChangeIdCard(p_stDynamics, idCard);
        }
        return (s32) p_stTree;
    }
    case eProc_PlayerIsDead:
        fn_vSnifThePlayerIsDead();
        return (s32) p_stTree;
    }
    return (s32) p_stTree;
}

void* fn_p_stKillPersoAndClearVariableProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    u8 variable;
    u8 arrayIndex;
    u8 component;
    u16 field;
    u8 kind;
    HIE_tdstSuperObject_* p_stResultOwnerPerso;
    s32 procedure;
    tdstEngineObject_* hKilledObject;

    procedure = AI_NODE_ID(p_stTree);
    variable = 0xFF;
    field = 0xD;
    p_stResultOwnerPerso = NULL;
    arrayIndex = 0;
    component = 0;

    switch (procedure) {
    case eProc_KillPersoAndClearVariable:
        p_stTree = inv_fn_p_stGetFieldOrDsgVarForAffectOperator(
            p_SuperObjPerso, p_stTree, &stParam, &p_stResultOwnerPerso, &variable, &arrayIndex,
            &component, &field, &kind);
        hKilledObject = stParam.u.pvValue;
        inv_AI_fn_vClearTextLabelsOfObject(hKilledObject->h_StandardGame->h_SuperObject);
        fn_vKillEngineObjectOrAlwaysByPointer(hKilledObject);
        if (p_SuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv
                ? hKilledObject == p_SuperObjPerso->hLinkedObject
                : hKilledObject == NULL) {
            ucProcedureActionReturn = 3;
        }
        stParam.u.pvValue = NULL;
        fn_ucSetDsgVar(
            variable, arrayIndex,
            ((tdstEngineObject_*) p_stResultOwnerPerso->hLinkedObject)->h_Brain
                ->p_stMind,
            &stParam);
        return p_stTree;
    case eProc_KillPerso:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        hKilledObject = stParam.u.pvValue;
        inv_AI_fn_vClearTextLabelsOfObject(hKilledObject->h_StandardGame->h_SuperObject);
        fn_vKillEngineObjectOrAlwaysByPointer(hKilledObject);
        if (p_SuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv
                ? hKilledObject == p_SuperObjPerso->hLinkedObject
                : hKilledObject == NULL) {
            ucProcedureActionReturn = 3;
        }
        break;
    }
    return p_stTree;
}

static inline void inv_AI_fn_vSetIfTypeIsOne(s32* out, s32 type, s32 one)
{
    if (type == 1)
        *out = one;
    else
        *out = 0;
}

tdstNodeInterpret_* fn_p_stMiscProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                         tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stValue;
    u16 procedure;

    procedure = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stValue);

    switch (procedure) {
    case eProc_ForcePersoHandling: {
        tdstEngineObject_* p_stObj;
        s32 count;

        p_stObj = (tdstEngineObject_*) stValue.u.pvValue;
        count = p_stObj->h_StandardGame->ulLastTrame;
        if (count == inv_fn_ulGetEngineTrameNumber()) {
            p_stObj->h_StandardGame->ulLastTrame = inv_fn_ulGetEngineTrameNumber() - 1;
            HIE_fn_vInvalidateHierarchyFrame(p_stObj->h_StandardGame->h_SuperObject);
        }
        return p_stTree;
    }

    case eProc_HierFreezeEngine: {
        u8 value = (u8) stValue.u.lValue;
        if (value != 0) {
            fn_vSaveEngineClock();
        } else if (g_stEngineStructure.bEngineFrozen != 0) {
            fn_vLoadEngineClock();
        }
        g_stEngineStructure.bEngineFrozen = value;
        return p_stTree;
    }

    case eProc_HierSetPlatformType: {
        gu_tdstStandardGame* standardGame;
        HIE_tdstSuperObject_* hChild;
        u8 value;
        s32 characterType;
        s32 one;

        standardGame = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_StandardGame;
        value = (u8) stValue.u.lValue;
        if (standardGame->ucPlatFormType == value)
            break;

        if (standardGame->ucPlatFormType != 0) {
            hChild = p_SuperObjPerso->hFirstElementSta;
            if (hChild != NULL) {
                characterType = 2;
                one = 1;
                do {
                    if (hChild->lTypeOfLinkedObject == characterType) {
                        tdstDynam_* dynam =
                            ((tdstEngineObject_*) hChild->hLinkedObject)->h_Dynam;
                        if (dynam != NULL) {
                            DNM_stDynamics* identity = dynam->p_stDynamics;
                            s32 isTypeOne = 0;
                            if (identity->m_tdstDynamicsBaseBlock.m_pCurrentIdCard != 0)
                                inv_AI_fn_vSetIfTypeIsOne(
                                    &isTypeOne,
                                    DNM_fn_eIdentityCardGetType(identity->m_tdstDynamicsBaseBlock.m_pCurrentIdCard), one);
                            {
                                long isTypeOneWide = (long) isTypeOne;
                                if (isTypeOneWide)
                                    MEC_fn_vFreePlatformLink(identity, hChild);
                            }
                        }
                        fn_vReputCharacterSuperObjectAtTheWorld(hChild);
                    }
                    hChild = hChild->hNextBrotherSta;
                } while (hChild != NULL);
            }
        }

        standardGame->ucPlatFormType = value;
        HIE_fn_vChangeFather(p_SuperObjPerso, p_SuperObjPerso->hFatherSta, value);
        return p_stTree;
    }

    case eProc_SetMainActor:
        (*(s32*) &g_stEngineStructure.pendingSuperObject) =
            (s32) ((tdstEngineObject_*) stValue.u.pvValue)->h_StandardGame->h_SuperObject;
        return p_stTree;

    case eProc_ActivateObject:
        fn_vForceActiveAnObject(stValue.u.pvValue);
        return p_stTree;

    case eProc_DesactivateObject:
        fn_vDesactiveAnObject(stValue.u.pvValue);
        return p_stTree;

    case eProc_ChangeMap:
        fn_bSaveCurrentGameOptions();
    case eProc_ChangeMapNoAutosave:
        GAM_fn_vAskToChangeLevel(fn_szGetStringFromTextOrStringParam(&stValue));
        return p_stTree;

    case eProc_ChangeMapWithTransition:
        if ((s32) stValue.u.lValue < (s32) g_stEngineStructure.ucNumberOfLevels)
            GAM_fn_vAskToChangeLevel(
                g_stEngineStructure.a_szLevelName[stValue.u.lValue]);
        return p_stTree;

    case eProc_SetActionReturn:
        g_ucUseDefaultActionReturn = 1;
        g_ucNewActionReturn = (u8) stValue.u.lValue;
        return p_stTree;
    }

    return p_stTree;
}

u32 fn_p_stMiscMoreProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ out0, out1;
    u16 procedure;
    HIE_tdstSuperObject_* firstValue;
    HIE_tdstSuperObject_* secondValue;

    procedure = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &out0);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &out1);
    if (procedure != eProc_PlayerIsDeadWithPlacement) {
        return (u32) p_stTree;
    }
    firstValue = 0;
    if (out0.u.pvValue != 0) {
        firstValue = ((tdstEngineObject_*) out0.u.pvValue)->h_StandardGame->h_SuperObject;
    }
    secondValue = 0;
    if (out1.u.pvValue != 0) {
        secondValue = ((tdstEngineObject_*) out1.u.pvValue)->h_StandardGame->h_SuperObject;
    }
    fn_vSnifThePlayerIsDeadWithResurrectionAt(firstValue, secondValue);
    return (u32) p_stTree;
}

tdstNodeInterpret_* fn_p_stDynamProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* root;
    tdstCineinfo_* node;
    HIE_tdstSuperObject_* object;
    MTH3D_tdstVector_* entry;
    MTH3D_tdstVector_* slot;
    f32* factor;
    HIE_tdstSuperObject_* perso;
    s32 eProcedureId;
    f32 initialFactor;
    u32 ulIndex;

    eProcedureId = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &perso);
    if (eProcedureId != eProc_ForcePersoAveragePosition) {
        return p_stTree;
    }
    root = g_stEngineStructure.h_StdCamCaracter;
    if (root != 0) {
        node = ((tdstEngineObject_*) root->hLinkedObject)->h_Cineinfo;
        object = node->hCurrent->hSuperObjectTargeted;
        if (object != perso) {
            return p_stTree;
        }
        entry = inv_POS_fn_p_stPosMatrixTranslation(object->gu_hGlobalMatrix);
        ulIndex = 0;
        slot = node->AveragePosition.a_CheckedSpeed;
        factor = node->AveragePosition.a_CheckedTime;
        initialFactor = 0.03f;
        do {
            *factor = initialFactor;
            factor++;
            slot->xZ = slot->xY = slot->xX = 0.0f;
            slot[CAMERA_AVERAGE_HISTORY_COUNT] = *entry;
            slot++;
            ulIndex++;
        } while (ulIndex < 8);
    }
    return p_stTree;
}

u32 func_001F3ED0(void* superObject, void* stream)
{
    tdstGetSetParam_ value;
    tdstEngineObject_* owner;
    void* nextStream;
    u32 result;
    u32 flag;
    u64 tested;

    nextStream = fn_p_stEvalTree(superObject, stream, &value);
    owner = value.u.pvValue;
    result = (u32) fn_p_stEvalTree(superObject, nextStream, &value);
    owner->h_StandardGame->queueKey0C = value.u.ucValue & 0x7F;
    flag = (owner->h_StandardGame->listAnchor == inv_GAM_currentCharacterList) ||
           (owner->h_StandardGame->listAnchor == inv_GAM_previousCharacterList);
    tested = flag;
    if (tested != 0) {
        inv_fn_vQueueEngineObjectByPriority(owner->h_StandardGame);
    }
    return result;
}

#ifdef PC_PORT
tdstNodeInterpret_* fn_p_stHitPointsProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
#else
tdstNodeInterpret_* fn_p_stHitPointsProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                              tdstNodeInterpret_* p_stTree, void* unused)
#endif
{
    enum {
        HP_PROC_SET = 1,
        HP_PROC_SET_INITIAL = 2,
        HP_PROC_RESTORE_INITIAL = 3,
        HP_PROC_FILL_TO_MAXIMUM = 4,
        HP_PROC_ADD = 5,
        HP_PROC_SUBTRACT = 6
    };

    tdstGetSetParam_ stParam;
    u16 operation = AI_NODE_ID(p_stTree);

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    switch (operation) {
    case HP_PROC_SET:
        return inv_fn_p_stSetHitPoints(p_SuperObjPerso, p_stTree, stParam.u.pvValue);
    case HP_PROC_SET_INITIAL: {
        tdstEngineObject_* hPerso1 = stParam.u.pvValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        fn_vSetHitPointsInit(hPerso1, stParam.u.lValue);
        break;
    }
    case HP_PROC_RESTORE_INITIAL:
        fn_vSetHitPointsToInitValue(stParam.u.pvValue);
        break;
    case HP_PROC_FILL_TO_MAXIMUM:
        fn_vSetHitPointsToMaxValue(stParam.u.pvValue);
        break;
    case HP_PROC_ADD:
        return inv_fn_p_stAddHitPoints(p_SuperObjPerso, p_stTree, stParam.u.pvValue);
    case HP_PROC_SUBTRACT:
        return inv_fn_p_stSubHitPoints(p_SuperObjPerso, p_stTree, stParam.u.pvValue);
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stHitPointsMaxProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    u16 operation = AI_NODE_ID(p_stTree);

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    switch (operation) {
    case eProc_SetHitPointsMax:
        return inv_fn_p_stSetHitPointsMax(p_SuperObjPerso, p_stTree, stParam.u.pvValue);
    case eProc_SetHitPointsMaxToInitValue:
        fn_vSetHitPointsMaxToInitValue(stParam.u.pvValue);
        break;
    case eProc_SetHitPointsMaxToMaxValue:
        fn_vSetHitPointsMaxToMaxValue(stParam.u.pvValue);
        break;
    case eProc_AddHitPointsMax:
        return inv_fn_p_stAddHitPointsMax(p_SuperObjPerso, p_stTree, stParam.u.pvValue);
    case eProc_SubHitPointsMax:
        return inv_fn_p_stSubHitPointsMax(p_SuperObjPerso, p_stTree, stParam.u.pvValue);
    }
    return p_stTree;
}

void* fn_p_stListZDDZDEProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stValue;
    s16 value;
    s16 output;
    void* object;
    void* current;
    s32 eProcedureId;
    u16 first;
    u16 second;

    current = p_stTree;
    eProcedureId = AI_NODE_ID(current);
    current = fn_p_stEvalTree(p_SuperObjPerso, current, &stValue);
    object = (void*) stValue.u.lValue;

    if (eProcedureId == eProc_ListAffectTypeZDEWithTypeZDE) {
        current = fn_p_stEvalTree(p_SuperObjPerso, current, &stValue);
        first = *(u16*) &stValue;
        current = fn_p_stEvalTree(p_SuperObjPerso, current, &stValue);
        second = *(u16*) &stValue;
        fn_vAffectTypeZdeWithTypeZdeInList(object, p_SuperObjPerso, first, second);
        return current;
    }

    current = fn_p_stEvalTree(p_SuperObjPerso, current, &stValue);
    value = *(s16*) &stValue;
    if (eProcedureId == 0x13) {
        fn_vAffectZddInList(object, p_SuperObjPerso, value);
        return current;
    }

    if (eProcedureId == 0x14) {
        GMT_fn_vResetEntireMask(&output);
    } else {
        current = fn_p_stEvalTree(p_SuperObjPerso, current, &stValue);
        output = *(u16*) &stValue;
    }

    if ((u16) (eProcedureId - 0x14) < 2u) {
        fn_vAffectZdeInList(object, p_SuperObjPerso, value, (u16) output);
    }
    return current;
}

void* fn_p_stListProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ firstParam;
    tdstGetSetParam_ secondParam;

    tdstList_* list;

    HIE_tdstSuperObject_* outer;
    HIE_tdstSuperObject_* node;
    HIE_tdstSuperObject_* entry;
    tdstBrain_* brain;
    u16 procedure;
    u8 index;
    s32 addKey;
    s32 addModel;
    s32 removeKey;
    s32 removeModel;
    s32 characterType;

    procedure = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
    list = firstParam.u.pvValue;

    switch (procedure) {
    case eProc_AddPersoInList:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        fn_vPutInList(
            list, ((tdstEngineObject_*) firstParam.u.lValue)->h_StandardGame->h_SuperObject);
        break;
    case eProc_AddPersoInListAt:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &secondParam);
        fn_vInsertPersoAtPositionInList(
            list,
            ((tdstEngineObject_*) firstParam.u.lValue)->h_StandardGame->h_SuperObject,
            *(u8*) &secondParam);
        break;
    case eProc_DeletePersoAtInList:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        fn_vDeletePersoAtPositionInList(list, *(u8*) &firstParam);
        break;
    case eProc_FindPersoAndDeleteInList:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        fn_bFindPersoAndDeleteInList(
            list,
            ((tdstEngineObject_*) firstParam.u.lValue)->h_StandardGame->h_SuperObject);
        break;
    case eProc_ListSelect:
        return fn_p_stListSelect(list, 0, p_stTree);
    case eProc_ListUnSelect:
        return fn_p_stListSelect(list, 1, p_stTree);
    case eProc_ListSort:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &secondParam);
        return fn_p_stListSort(list, *(u8*) &firstParam, *(s8*) &secondParam, p_stTree);
    case eProc_ListSortByFamily:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        fn_vSortListByFamily(list, (void*) firstParam.u.lValue);
        break;
    case eProc_ListSortByModel:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &firstParam);
        fn_vSortListByModel(list, (void*) firstParam.u.lValue);
        break;
    case eProc_FillListWithAllPersoOfAFamily:
        list->ucNbElt = 0;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &secondParam);
        addKey = secondParam.u.lValue;
        outer = gp_stDynamicWorld->hFirstElementSta;
        if (outer != 0) {
            characterType = 2;
            node = outer;
            do {
            case20_loop:
                if (node != 0) {
                    if (node->lTypeOfLinkedObject != characterType)
                        goto case20_next;
                    if ((s32) ((tdstEngineObject_*) node->hLinkedObject)->h_3dData->h_Family !=
                        addKey)
                        goto case20_next;
                    if (list->ucNbElt < list->ucMaxSize) {
                        fn_vPutInList(list, node);
                    case20_next:
                        node = node->hNextBrotherSta;
                        goto case20_loop;
                    }
                }
                outer = outer->hFirstElementSta;
                node = outer;
            } while (outer != 0);
        }
        break;
    case eProc_FillListWithAllPersoOfAModel:
        list->ucNbElt = 0;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &secondParam);
        addModel = secondParam.u.lValue;
        outer = gp_stDynamicWorld->hFirstElementSta;
        if (outer != 0) {
            characterType = 2;
            node = outer;
        case21_loop:
            if (node != 0) {
                if (node->lTypeOfLinkedObject != characterType)
                    goto case21_next;
                brain = ((tdstEngineObject_*) node->hLinkedObject)->h_Brain;
                if (brain == 0)
                    goto case21_next;
                if ((s32) brain->p_stMind->p_stAIModel != addModel)
                    goto case21_next;
                if (list->ucNbElt < list->ucMaxSize) {
                    fn_vPutInList(list, node);
                case21_next:
                    node = node->hNextBrotherSta;
                    goto case21_loop;
                }
            }
            outer = outer->hFirstElementSta;
            node = outer;
            if (outer != 0)
                goto case21_loop;
        }
        break;
    case eProc_DeleteFamilyInList:
        index = 0;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &secondParam);
        removeKey = secondParam.u.lValue;
        entry = fn_p_stGetPersoInList(list, 0);
        while (entry != 0) {
            if ((s32) ((tdstEngineObject_*) entry->hLinkedObject)->h_3dData->h_Family ==
                removeKey) {
                fn_vDeletePersoAtPositionInList(list, index);
            } else {
                index++;
            }
            entry = fn_p_stGetPersoInList(list, index);
        }
        break;
    case eProc_DeleteModelInList:
        index = 0;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &secondParam);
        removeModel = secondParam.u.lValue;
        entry = fn_p_stGetPersoInList(list, 0);
        while (entry != 0) {
            brain = ((tdstEngineObject_*) entry->hLinkedObject)->h_Brain;
            if (brain != 0 && (s32) brain->p_stMind->p_stAIModel == removeModel) {
                fn_vDeletePersoAtPositionInList(list, index);
            } else {
                index++;
            }
            entry = fn_p_stGetPersoInList(list, index);
        }
        break;
    default:
        break;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stListEnsembleProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stValue;
    u16 procedure;
    s32 list1;
    s32 list2;
    s32 list3;

    procedure = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stValue);
    list1 = stValue.u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stValue);
    list2 = stValue.u.lValue;
    if (procedure == eProc_ListAdd) {
        fn_vAddList((void*) list1, (void*) list2);
        return p_stTree;
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stValue);
    list3 = stValue.u.lValue;
    switch (procedure) {
    case eProc_ListUnion:
        fn_vUnionList((void*) list1, (void*) list2, (void*) list3);
        break;
    case eProc_ListInter:
        fn_vInterList((void*) list1, (void*) list2, (void*) list3);
        break;
    case eProc_ListDiff:
        fn_vDiffList((void*) list1, (void*) list2, (void*) list3);
        break;
    }
    return p_stTree;
}

void* fn_p_stLinkTableProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    u16 procedure;
    u8 ucObj1;

    procedure = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    switch (procedure) {
    case eProc_SwapLinkTableObjects:
        ucObj1 = scratch.u.ucValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        fn_vExchangeTwoObjectsInCurrentObjectsTable(p_SuperObjPerso, ucObj1, scratch.u.ucValue);
        break;
    case eProc_ChangeCurrentObjectTable:
        ((tdstEngineObject_*) (p_SuperObjPerso)->hLinkedObject)->h_3dData->h_CurrentObjectsTable =
            scratch.u.pvValue;
        return p_stTree;
    default:
        return p_stTree;
    }
    return p_stTree;
}

static inline s32 inv_AI_fn_lObjectTableCharIndex(s8 ch)
{
    s32 code = ch - 0x20;
    if ((inv_FMN_pathCharacterFlags[ch] & 2) == 0) {
        code = ch;
    }
    return code - 0x20;
}

tdstNodeInterpret_*
fn_p_stBuildObjectTableFromTableAndStringProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    tdstEngineObject_* object;
    tdstObjectsTablesList_* dst;
    tdstObjectsTablesList_* src;
    void* result;
    s8* text;
    s16 dst_count;
    s16 src_count;
    s16 index;
    s32 code;
    u16 opcode;
    HIE_tdstSuperObject_* context;

    result = p_stTree;
    context = p_SuperObjPerso;
    opcode = *(u16*) ((u8*) result - 8);
    result = fn_p_stEvalTree(context, result, &stParam);
    if (opcode != 0x56) {
        return result;
    }

    object = context->hLinkedObject;
    dst = object->h_3dData->h_CurrentObjectsTable;
    index = 0;
    src = (tdstObjectsTablesList_*) stParam.u.lValue;
    dst_count = fn_uwGetObjectTableNumberOfelements(dst);
    src_count = fn_uwGetObjectTableNumberOfelements(src);

    result = fn_p_stEvalTree(context, result, &stParam);
    text = fn_szGetStringFromTextOrStringParam(&stParam);
    while (*text && index < dst_count) {
        code = inv_AI_fn_lObjectTableCharIndex(*text);
        if (code < 0 || code >= src_count) {
            code = 0;
        }
        fn_vCopyObjectFromObjectTabletoAnother(dst, src, (u16) index, (u16) code);
        text++;
        index++;
    }
    while (index < dst_count) {
        fn_vCopyObjectFromObjectTabletoAnother(dst, src, (u16) index, 0);
        index = index + 1;
    }
    return result;
}

tdstNodeInterpret_* fn_p_stModuleControlProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    s32 code;
    s32 lChannel;

    code = AI_NODE_ID(p_stTree);
    if (code != eProc_ReleaseAllModulesControl)
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    switch (code) {
    case eProc_TakeManyModulesControl:
        lChannel = stParam.u.lValue;
        lChannel = lChannel - 1;
        while (lChannel >= 0) {
            fn_vTakeControlOfChannel(p_SuperObjPerso->hLinkedObject, (u16) lChannel, 1);
            lChannel--;
        }
        return p_stTree;
    case eProc_TakeModuleControl:
        fn_vTakeControlOfChannel(p_SuperObjPerso->hLinkedObject, stParam.u.lValue, 1);
        return p_stTree;
    case eProc_ReleaseModuleControl:
        fn_vTakeControlOfChannel(p_SuperObjPerso->hLinkedObject, stParam.u.lValue, 0);
        return p_stTree;
    case eProc_ReleaseAllModulesControl:
        fn_vTakeControlOfChannel(p_SuperObjPerso->hLinkedObject, 0xFFFF, 0);
        return p_stTree;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stSaveGameProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    u16 op;
    u32 duration;
    char* text;

    op = AI_NODE_ID(p_stTree);
    switch (op) {
    case eProc_QuitGame:
        fn_vChangeEngineMode(2);
        ucProcedureActionReturn = 3;
        break;
    case 0x5A:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        inv_SAV_fn_vSetCurrentSlotIndex(scratch.u.uwValue);
        break;
    case 0x5B:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        inv_SAV_fn_vSetCurrentSaveComment(fn_szGetStringFromTextOrStringParam(&scratch));
        break;
    case 0x5C:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        func_001DDDD0(scratch.u.pvValue);
        break;
    case 0x5D:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        inv_SAV_fn_vSetCurrentGameDuration(scratch.u.ulValue);
        break;
    case 0x5E:
    case 0x5F:
        if (op == 0x5F) {
            duration = inv_SAV_fn_ulGetCurrentGameDuration();
        } else {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
            duration =
                (u32) inv_SAV_fn_ulGetSlotPlayTime(inv_SAV_fn_p_stGetSlotByIndex(scratch.u.wValue));
        }
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        text = fn_szGetStringFromTextOrStringParam(&scratch);
        {
            u32 hours = duration / 3600;
            u32 minutes = duration / 60;
            u32 seconds = duration % 60;
            sprintf(text, "%d=%02d'%02d*", hours, minutes - hours * 60, seconds);
        }
        break;
    }
    return p_stTree;
}

void* fn_p_stChangeActivationChannelProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                              tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    u16 type;
    u16 code;
    HIE_tdstSuperObject_* hPerso;
    s32 flag;

    type = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    code = stParam.u.uwValue;
    switch (type) {
    case eProc_ActivateChannel:
        hPerso = hNewSuperObjPerso;
        flag = 1;
        break;
    case eProc_DeactivateChannel:
        hPerso = hNewSuperObjPerso;
        flag = 0;
        break;
    default:
        return p_stTree;
    }
    fn_vActivateChannel(hPerso, code, flag);
    return p_stTree;
}

typedef struct {
    f32 f[5];
    GEO_tdstColor_ blk;
} FogCamSaved_;

extern FogCamSaved_ GLI_g_stSaveFog;

void* fn_p_stFogProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    GEO_tdstColor_ color;
    u16 op;
    s32 bits;
    f32 saved0;
    f32 saved1;
    f32 saved2;
    f32 saved3;
    f32 saved4;
    u8 cGlobalFogIsOn;

    op = AI_NODE_ID(p_stTree);

    switch (op) {
    case eProc_FogActivate:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        if (stParam.ucType == E_vt_Float) {
            bits = inv_MTH_fn_lToIntegerOrdered(stParam.u.xValue);
            bits = (u8) bits;
        } else {
            bits = *(u8*) &stParam.u;
        }
        if (bits != 0) {
            GLI_cGlobalFogIsOn = 1;
        } else {
            GLI_cGlobalFogIsOn = 0;
        }
        break;

    case eProc_SetFogColor:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        fn_vConvertParamToColor(&stParam, &color);
        GLI_stFogColor = color;
        return p_stTree;

    case eProc_SetFogNearFarInf:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        GLI_xFogNear =
            (stParam.ucType == E_vt_Float) ? stParam.u.xValue : (f32) stParam.u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        GLI_xFogFar =
            (stParam.ucType == E_vt_Float) ? stParam.u.xValue : (f32) stParam.u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        GLI_xFogInfinite =
            (stParam.ucType == E_vt_Float) ? stParam.u.xValue : (f32) stParam.u.lValue;
        break;

    case eProc_SetFogBlend:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        GLI_xFogBlendNear =
            (stParam.ucType == E_vt_Float) ? stParam.u.xValue : (f32) stParam.u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        GLI_xFogBlendFar =
            (stParam.ucType == E_vt_Float) ? stParam.u.xValue : (f32) stParam.u.lValue;
        break;

    case eProc_RestoreFog:
        if (GLI_g_bSaveFog != 0) {
            cGlobalFogIsOn = inv_GLI_savedFogEnabled;
            saved0 = GLI_g_stSaveFog.f[0];
            saved4 = GLI_g_stSaveFog.f[4];
            saved1 = GLI_g_stSaveFog.f[1];
            saved2 = GLI_g_stSaveFog.f[2];
            saved3 = GLI_g_stSaveFog.f[3];
            GLI_xFogNear = saved0;
            GLI_xFogFar = saved1;
            GLI_xFogInfinite = saved2;
            GLI_xFogBlendNear = saved3;
            GLI_xFogBlendFar = saved4;
            GLI_g_bSaveFog = 0;
            GLI_cGlobalFogIsOn = cGlobalFogIsOn;
            GLI_stFogColor = GLI_g_stSaveFog.blk;
        }
        break;

    case eProc_SaveFog:
        if (GLI_g_bSaveFog != 0) {
            return p_stTree;
        }
        GLI_g_stSaveFog.blk = GLI_stFogColor;
        GLI_g_stSaveFog.f[0] = GLI_xFogNear;
        GLI_g_stSaveFog.f[1] = GLI_xFogFar;
        GLI_g_stSaveFog.f[2] = GLI_xFogInfinite;
        GLI_g_stSaveFog.f[3] = GLI_xFogBlendNear;
        GLI_g_stSaveFog.f[4] = GLI_xFogBlendFar;
        GLI_g_bSaveFog = 1;
        inv_GLI_savedFogEnabled = GLI_cGlobalFogIsOn;
        break;

    default:
        break;
    }
    return p_stTree;
}

void* fn_p_stPersoLightProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                 tdstNodeInterpret_* p_stTree)
{
    void* savedContext;
    void* current;
    void* light;
    u16 operation;
    s32 operationIndex;

    struct {
        tdstGetSetParam_ param;
        GEO_tdstColor_ color;
    } scratch;

    HIE_tdstSuperObject_* p_stPerso;
    tdstEngineObject_* ctx;
    f32 firstValue;
    f32 secondValue;
    f32 value;
    s32 giroEnabled;
    s32 pulseEnabled;

    savedContext = p_SuperObjPerso;
    current = p_stTree;
    operation = AI_NODE_ID(p_stTree);

    fn_vGetUltraOperatorPerso(savedContext, &p_stPerso);
    if (p_stPerso->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
        ctx = 0;
    } else {
        ctx = p_stPerso->hLinkedObject;
    }

    light = ctx->h_MSLight;
    if (!light) {
        return fn_p_stSwapNextParameter2(current);
    }

    operationIndex = (s32) (operation - 0x63);
    if ((u32) operationIndex >= 11u) {
        return current;
    }

    switch (operationIndex) {
    case 0:
        fn_vMSLightSetOnOff(light, 1);
        return current;
    case 1:
        fn_vMSLightSetOnOff(light, 0);
        return current;
    case 3: {
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            firstValue = scratch.param.u.xValue;
        } else {
            firstValue = (f32) scratch.param.u.lValue;
        }
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            secondValue = scratch.param.u.xValue;
        } else {
            secondValue = (f32) scratch.param.u.lValue;
        }
        fn_vMSLightSetNearFar(light, firstValue, secondValue);
        return current;
    }
    case 2: {
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        fn_vConvertParamToColor(&scratch.param, &scratch.color);
        fn_vMSLightSetColorRGBA(light, scratch.color.xR, scratch.color.xG, scratch.color.xB,
                                scratch.color.xA);
        return current;
    }
    case 4: {
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            value = scratch.param.u.xValue;
        } else {
            value = (f32) scratch.param.u.lValue;
        }
        fn_vMSLightSetLittleAlpha(light, value);
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            value = scratch.param.u.xValue;
        } else {
            value = (f32) scratch.param.u.lValue;
        }
        fn_vMSLightSetBigAlpha(light, value);
        return current;
    }
    case 5: {
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        giroEnabled = (*(u8*) &scratch.param.u > 0) ? 1 : 0;
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            value = scratch.param.u.xValue;
        } else {
            value = (f32) scratch.param.u.lValue;
        }
        fn_vMSLightSetGiroPhare(light, giroEnabled, value);
        return current;
    }
    case 6: {
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        pulseEnabled = (*(u8*) &scratch.param.u > 0) ? 1 : 0;
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            firstValue = scratch.param.u.xValue;
        } else {
            firstValue = (f32) scratch.param.u.lValue;
        }
        current = fn_p_stEvalTree(savedContext, current, &scratch.param);
        if (scratch.param.ucType == E_vt_Float) {
            secondValue = scratch.param.u.xValue;
        } else {
            secondValue = (f32) scratch.param.u.lValue;
        }
        fn_vMSLightSetPulse(light, pulseEnabled, firstValue, secondValue);
        return current;
    }
    case 7:
        fn_vMSLightSetTypeOfLight(light, 1);
        return current;
    case 8:
        fn_vMSLightSetTypeOfLight(light, 2);
        return current;
    case 9:
        fn_vMSLightSetTypeOfLight(light, 3);
        return current;
    case 10:
        fn_vMSLightSetTypeOfLight(light, 4);
        return current;
    default:
        return current;
    }
}

void* fn_p_stSoundProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* saved;
    tdstGetSetParam_ scratch;
    void* node;
    void* sound;
    u16 op;
    s32 operationIndex;
    void* base;
    u16 index;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &saved);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    node = scratch.u.pvValue;

    if (node == 0)
        goto no_sound;
    if (node == (void*) -1)
        goto no_sound;
    sound = *(void**) node;
    if (sound == 0)
        goto no_sound;

    operationIndex = (s32) op - 0x6E;
    if ((u32) operationIndex >= 16u)
        return p_stTree;

    switch (operationIndex) {
    no_sound:
        if (op == 0x72 || op == 0x7A || op == 0x75 || op == 0x7D)
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        goto done;
    case 0:
        SND_fn_lSendRequestSound((s32) saved, g_lSoundObjectTypeAnim, (s32) sound,
                                 0, 0);
        return p_stTree;
    case 8:
        printf(" SND_fn_lStopChannelSoundAnim\n ");
        inv_SND_fn_lSendStopRequestSound(-1, (s32) saved, g_lSoundObjectTypeAnim, (s32) sound, 0, 0);
        return p_stTree;
    case 1:
        printf("SendMusicReques \n");
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMusic, (s32) sound, 0, 0);
        return p_stTree;
    case 9:
        printf(" SND_fn_lStopChannelSoundMusic\n ");
        inv_SND_fn_lSendStopRequestSound(-1, -1, g_lSoundObjectTypeMusic, (s32) sound, 0, 0);
        return p_stTree;
    case 2:
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeAnim, (s32) sound, 0, 0);
        return p_stTree;
    case 10:
        inv_SND_fn_lSendStopRequestSound(-1, -1, g_lSoundObjectTypeAnim, (s32) sound, 0, 0);
        return p_stTree;
    case 3:
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMenu, (s32) sound, 0, 0);
        return p_stTree;
    case 11:
        inv_SND_fn_lSendStopRequestSound(-1, -1, g_lSoundObjectTypeMenu, (s32) sound, 0, 0);
        return p_stTree;
    case 4:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        base = (void*) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)
                   ->h_3dData->hArrayOfChannels;
        index = scratch.u.uwValue;
        if (base != 0)
            SND_fn_lSendRequestSound((s32) ((char*) base + ((s32) index << 5)),
                                     inv_SND_moduleObjectSoundType, (s32) sound, 0, 0);
        return p_stTree;
    case 12:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        base = (void*) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)
                   ->h_3dData->hArrayOfChannels;
        index = scratch.u.uwValue;
        if (base != 0)
            inv_SND_fn_lSendStopRequestSound(-1, (s32) ((char*) base + ((s32) index << 5)),
                          inv_SND_moduleObjectSoundType, (s32) sound, 0, 0);
        return p_stTree;
    case 5:
        SND_fn_lSendRequestSound((s32) saved, g_lSoundObjectTypeVoice, (s32) sound, 0, 0);
        return p_stTree;
    case 13:
        inv_SND_fn_lSendStopRequestSound(-1, (s32) saved, g_lSoundObjectTypeVoice, (s32) sound, 0, 0);
        return p_stTree;
    case 6:
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeVoice, (s32) sound, 0, 0);
        return p_stTree;
    case 14:
        inv_SND_fn_lSendStopRequestSound(-1, -1, g_lSoundObjectTypeVoice, (s32) sound, 0, 0);
        return p_stTree;
    case 7:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        base = (void*) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)
                   ->h_3dData->hArrayOfChannels;
        index = scratch.u.uwValue;
        if (base != 0)
            SND_fn_lSendRequestSound((s32) ((char*) base + ((s32) index << 5)),
                                     inv_SND_moduleVoiceObjectSoundType, (s32) sound, 0, 0);
        return p_stTree;
    case 15:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        base = (void*) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) saved)->hLinkedObject)
                   ->h_3dData->hArrayOfChannels;
        index = scratch.u.uwValue;
        if (base != 0)
            inv_SND_fn_lSendStopRequestSound(-1, (s32) ((char*) base + ((s32) index << 5)),
                          inv_SND_moduleVoiceObjectSoundType, (s32) sound, 0, 0);
        return p_stTree;
    }

done:
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stKeyboardProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree)
{
    tdstNodeInterpret_* node;
    IPT_tdstEntryElement_* element;
    IPT_tdstEntryElement_* IPT_xHandle;
    u16 kind;

    node = p_stTree;
    kind = AI_NODE_ID(node);
#ifdef PC_PORT
    {
        tdstGetSetParam_ stElement;
        node = fn_p_stEvalTree(p_SuperObjPerso, node, &stElement);
        element = stElement.u.pvValue;
    }
#else
    node = fn_p_stEvalTree(p_SuperObjPerso, node, (tdstGetSetParam_*) &element);
#endif
    IPT_xHandle = element;
    switch (kind) {
    case eProc_DeactivateBut:
        IPT_fn_vDesactivateEntryElement(IPT_xHandle);
        break;
    case eProc_ActivateBut:
        IPT_fn_vActivateEntryElement(IPT_xHandle);
        break;
    }
    return node;
}

void* fn_p_stDummy(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    (void) p_SuperObjPerso;
    return p_stTree;
}

void fn_vInternalChangeComportProcedure(tdstIntelligence_* p_stAimedPersoIntelligenceOrReflex, tdstComport_* p_stComport, u32 _bContinueExecution)
{
    u8 flag = (u8) _bContinueExecution;
    if (p_stAimedPersoIntelligenceOrReflex->p_stCurrentComport != p_stComport) {
        fn_ucChangeComportIntell(p_stAimedPersoIntelligenceOrReflex, p_stComport);
        if (!flag) {
            iNumLoop = 0;
            ucProcedureActionReturn = 8;
        }
    }
}

void* fn_p_stChangeComportIntelligenceProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ rec;
    void* p_stNext;
    void* p_stEnd;
    HIE_tdstSuperObject_* obj;
    AI_tdstMind_* p_stMind;
    tdstIntelligence_* arg;
    u32 flag;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &rec);
    obj = inv_fn_hGetOwnerSuperObject(rec.u.pvValue);
    p_stMind = inv_HIE_fn_p_stGetEngineObject(obj)->h_Brain->p_stMind;
    p_stEnd = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &rec);
    flag = (obj != p_SuperObjPerso || p_stMind->bDoingIntel == 0);
    arg = p_stMind->p_stIntelligence;
    fn_vInternalChangeComportProcedure(arg, rec.u.pComport, flag);
    return p_stEnd;
}

void* fn_p_stChangeComportReflexProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ rec;
    void* p_stNext;
    void* p_stEnd;
    HIE_tdstSuperObject_* obj;
    AI_tdstMind_* p_stMind;
    tdstIntelligence_* arg;
    u32 flag;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &rec);
    obj = inv_fn_hGetOwnerSuperObject(rec.u.pvValue);
    p_stMind = inv_HIE_fn_p_stGetEngineObject(obj)->h_Brain->p_stMind;
    p_stEnd = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &rec);
    flag = (obj != p_SuperObjPerso || p_stMind->bDoingIntel != 0);
    arg = p_stMind->p_stReflex;
    fn_vInternalChangeComportProcedure(arg, rec.u.pComport, flag);
    return p_stEnd;
}

void* fn_p_stChangeMyComportIntelligenceProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    AI_tdstMind_* p_stMind =
        ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Brain->p_stMind;
    void* result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    fn_vInternalChangeComportProcedure(p_stMind->p_stIntelligence, stParam.u.pComport, p_stMind->bDoingIntel == 0);
    return result;
}

void* fn_p_stChangeMyComportReflexProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                            tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    AI_tdstMind_* p_stMind =
        ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Brain->p_stMind;
    void* result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    fn_vInternalChangeComportProcedure(p_stMind->p_stReflex, stParam.u.pComport, p_stMind->bDoingIntel);
    return result;
}

void* fn_p_stChangeMyComportIntelligenceAndReflexProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                           tdstNodeInterpret_* p_stTree)
{
    tdstBrain_* ppEntry;
    AI_tdstMind_* p_stMind;
    void* pFirst;
    void* pSecond;
    tdstComport_* p_stIntell;
    tdstComport_* p_stReflex;
    tdstGetSetParam_ scratch;

    ppEntry = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Brain;
    p_stMind = ppEntry->p_stMind;
    pFirst = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    p_stIntell = scratch.u.pComport;
    pSecond = fn_p_stEvalTree(p_SuperObjPerso, pFirst, &scratch);
    p_stReflex = scratch.u.pComport;
    fn_vInternalChangeComportProcedure(p_stMind->p_stIntelligence, p_stIntell, 0);
    fn_vInternalChangeComportProcedure(p_stMind->p_stReflex, p_stReflex, 0);
    return pSecond;
}

void* fn_p_stTurnProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    u16 eProcedureId;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstDynam_* hDynam;
    DNM_stDynamics* p_stDynamics;
    tdstGetSetParam_ stParam;
    f32 xAngle;

    eProcedureId = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    xAngle = inv_AI_fn_xAiParamFloat(&stParam);

    hDynam = ((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_Dynam;
    if (hDynam == NULL)
        return p_stTree;
    p_stDynamics = hDynam->p_stDynamics;

    switch (eProcedureId) {
    case eProc_Turn:
        xAngle *= MTH_DEG_TO_RAD_inv;
        xAngle *= inv_fn_xGetEngineClockDelta();
        MEC_vTurnAroundZAxis(p_stDynamics, xAngle);
        break;
    case eProc_Turn2:
        xAngle *= MTH_DEG_TO_RAD_inv;
        MEC_vTurnAroundZAxis(p_stDynamics, xAngle);
        break;
    }
    return p_stTree;
}

void* fn_p_stCustomBitsProc(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    void* ctx;
    gu_tdstStandardGame* obj;
    void* stream;
    tdstGetSetParam_* pool;
    s32 cmd;
    HIE_tdstSuperObject_* node;
    s32 mask;
    s32 lValue;
    s16 xBit;
    s32 bit_mask;
    s32 bits;
    s32 set;

    pool = inv_AI_parameterPoolCursor;
    stream = p_stTree;
    ctx = p_SuperObjPerso;
    cmd = AI_NODE_ID(stream);
    inv_AI_parameterPoolCursor = pool + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    obj = ((tdstEngineObject_*) node->hLinkedObject)->h_StandardGame;
    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, pool);

    if (cmd != eProc_ChangeOneCustomBit) {
        if (cmd < eProc_ChangeOneCustomBit) {
            inv_AI_parameterPoolCursor = pool;
            return stream;
        }
        if (cmd >= 0xA9) {
            inv_AI_parameterPoolCursor = pool;
            return stream;
        }

        mask = pool->u.lValue;
        stream = fn_p_stEvalTree(ctx, stream, pool);
        lValue = pool->u.lValue;
        if (cmd != eProc_ChangeManyCustomBits) {
            goto mask_30;
        }
        bits = obj->ulCustomBits;
        goto mask_common;
    }
    xBit = (s16) (*(u16*) pool - 1);
    stream = fn_p_stEvalTree(ctx, stream, pool);
    set = pool->u.lValue != 0;
    do {
        if (!(xBit >= 0x20)) {
            bit_mask = (s32) (1U << xBit);
            bits = obj->ulCustomBits;
            if (set != 0) {
                bits |= bit_mask;
                goto store_2c;
            }
            bits &= ~bit_mask;
            goto store_2c;
        }
        bit_mask = (s32) (1U << (xBit - 32));
        bits = obj->ulAICustomBits;
        if (set != 0) {
            bits |= bit_mask;
            goto store_30;
        }
        bits &= ~bit_mask;
        goto store_30;

    mask_30:
        bits = obj->ulAICustomBits;

    mask_common:
        lValue &= mask;
        bits &= ~mask;
        bits |= lValue;
        if (cmd != eProc_ChangeManyCustomBits) {
            goto store_30;
        }

    store_2c:
        obj->ulCustomBits = bits;
        goto done;

    store_30:
        obj->ulAICustomBits = bits;
        goto done;
    } while (cmd >= eProc_ChangeOneCustomBit && cmd < 0xA9);

done:
    inv_AI_parameterPoolCursor = pool;
    return stream;
}

void* fn_p_stStringProc(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    s32 kind;
    s32 len;
    s32 pos;
    s32 cChar;
    char* pszString;
    tdstGetSetParam_* base;

    base = inv_AI_parameterPoolCursor;
    kind = AI_NODE_ID(p_stTree);
    len = 0;
    inv_AI_parameterPoolCursor = base + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, base);
    pszString = fn_szGetStringFromTextOrStringParam(base);
    if (pszString != 0) {
        len = (s16) strlen(pszString);
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, base);
    pos = base->u.wValue;
    switch (kind) {
    case eProc_StringAddChar:
    case eProc_StringReplaceChar:
        if (pos >= 0 && pos < len) {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, base);
            pszString += pos;
            cChar = base->u.wValue;
            if (kind == eProc_StringAddChar) {
                memmove(pszString + 1, pszString, (len - pos) + 1);
            }
            *pszString = (char) cChar;
        }
        break;
    case eProc_StringRemoveChar:
        if (pos >= 0 && pos < len) {
            pszString += pos;
            memmove(pszString, pszString + 1, len - pos);
        }
        break;
    }
    inv_AI_parameterPoolCursor = base;
    return p_stTree;
}

void* func_001F5D08(void* superObject, tdstNodeInterpret_* stream)
{
    tdstGetSetParam_ stParam;
    s32 value;
    u16 kind;

    kind = AI_NODE_ID(stream);
    stream = fn_p_stEvalTree(superObject, stream, &stParam);
    value = stParam.u.wValue;
    switch (kind) {
    case 0xE9:
        inv_TXT_fn_vSetLanguage(value & 0xFF);
        break;
    case 0xEA:
        g_stEngineStructure.ucCurrentLanguage = (u8) value;
        break;
    }
    return stream;
}

void* fn_p_stIntToText(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* p_stNext;
    void* str;
    s32 lValue;
    f32 xValue;

    xValue = 0.0f;
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    str = fn_szGetStringFromTextOrStringParam(scratch);
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, scratch);
    if (scratch->ucType == E_vt_Float) {
        xValue = scratch->u.xValue;
        lValue = inv_MTH_fn_lToIntegerOrdered(xValue);
    } else {
        lValue = scratch->u.lValue;
    }
    sprintf(str, "%d", lValue);
    inv_AI_parameterPoolCursor = scratch;
    return p_stNext;
}

void* fn_p_stCollideCommunicationProc(HIE_tdstSuperObject_* p_SuperObjPerso,
                                      tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* stream;
    void* ctx;
    HIE_tdstSuperObject_* node;
    CollisionSet* comm;
    u16 op;
    s16 index;
    u8 ucPriority;

    scratch = inv_AI_parameterPoolCursor;
    stream = p_stTree;
    ctx = p_SuperObjPerso;
    index = -1;
    op = AI_NODE_ID(stream);

    node = ctx;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    comm = inv_HIE_fn_p_stGetEngineObject(node)->h_CollSet;

    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, scratch);
    ucPriority = *(u8*) scratch;
    stream = fn_p_stEvalTree(ctx, stream, scratch);

    switch (op) {
    case eProc_SetColliderType:
        break;
    case eProc_SetColliderActor:
    case eProc_SetColliderVector:
    case eProc_SetColliderReal:
        index = *(s16*) scratch;
        stream = fn_p_stEvalTree(ctx, stream, scratch);
        break;
    default:
        comm = 0;
        break;
    }

    if (comm != 0 && ucPriority >= comm->stColliderInfo.ucColliderPriority) {
        comm->stColliderInfo.ucColliderPriority = ucPriority;
        switch (op) {
        case eProc_SetColliderType:
            comm->stColliderInfo.ucColliderType = *(u8*) scratch;
            break;
        case eProc_SetColliderActor: {
            tdstEngineObject_* ref;
            void* value;
            ref = (tdstEngineObject_*) scratch->u.pvValue;
            value = inv_fn_hGetOwnerSuperObject(ref);
            if ((u16) index < 2) {
                comm->stColliderInfo.a_stColliderActors[index] = value;
            }
            break;
        }
        case eProc_SetColliderVector:
            if ((u16) index < 2) {
                comm->stColliderInfo.a_stColliderVectors[index] = scratch->u.stVector;
            }
            break;
        case eProc_SetColliderReal: {
            f32 value;
            if (scratch->ucType == E_vt_Float) {
                value = scratch->u.xValue;
            } else {
                value = (f32) scratch->u.lValue;
            }
            if ((u16) index < 2) {
                comm->stColliderInfo.a_xColliderReals[index] = value;
            }
            break;
        }
        }
    }

    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void func_001F6020(void* object, void* matrix, void* a2, u8 applyRotation)
{
    u32* p_stEngineObject;
    tdstDynam_* p_stDynam;

    HIE_fn_vComputeNewRelativeMatrix(object);
    if (applyRotation == 0) {
        return;
    }
    if (matrix == 0) {
        return;
    }
    p_stEngineObject = (u32*) ((u32*) object)[1];
    p_stDynam = (tdstDynam_*) p_stEngineObject[2];
    if (p_stDynam == 0) {
        return;
    }
    MEC_vImposeRotationMatrix(p_stDynam->p_stDynamics, matrix);
}

static inline void inv_AI_fn_vNormalizeOrCopy(MTH3D_tdstVector_* out, MTH3D_tdstVector_* in)
{
    f32 lengthSquared;
    f32 length;
    f32 inverse;

    lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(in->xX, in->xY, in->xZ);
    length = MTH_fn_xSqrtOpt2(lengthSquared);
    if (length != 0.0f) {
        inverse = 1.0f / length;
        MTH3D_M_vKTimesXYZ_inv(&(*(MTH3D_tdstVector_*) out), inverse, &(*(MTH3D_tdstVector_*) in));
    } else {
        *(MTH3D_tdstVector_*) out = *(MTH3D_tdstVector_*) in;
    }
}

static inline void inv_AI_fn_vNegate(MTH3D_tdstVector_* vector)
{
    MTH3D_M_vSetXYZ_inv(vector, inv_MTH_fn_xNegateTied(vector->xX),
                        inv_MTH_fn_xNegateTied(vector->xY), inv_MTH_fn_xNegateTied(vector->xZ));
}

static inline HIE_tdstSuperObject_* inv_AI_fn_p_stChannelSuperObject(HIE_tdstSuperObject_* superObject,
                                                       u16 channel)
{
    tdstAChannel_* records =
        ((tdstEngineObject_*) superObject->hLinkedObject)->h_3dData->hArrayOfChannels;

    return records != NULL ? records[channel].hSupObject : NULL;
}

tdstNodeInterpret_* fn_p_st3DProc_ForReorientation(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree)
{
    MTH3D_tdstVector_ primary, secondary, pivot, pivotWorld, axisI, axisJ, axisK, cross;
    HIE_tdstSuperObject_* target;
    tdstGetSetParam_* argument;
    HIE_tdstSuperObject_* channelObject;
    POS_tdstMatrix_* matrix;
    MTH4D_tdstVector_* translation;
    HIE_tdstSuperObject_* flagObject;
    u32 channel;
    u8 worldSpace;
    u8 axis;
    u8 hasPivot;
    s32 opcode;
    u32 negate;
    u32 negatedAxis;

    channelObject = NULL;
    argument = inv_AI_parameterPoolCursor;
    hasPivot = 0;
    worldSpace = 1;
    axis = 0;
    opcode = (u16) p_stTree[-1].param;
    inv_AI_parameterPoolCursor = argument + 1;
    target = p_SuperObjPerso;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &target);
    channel = 0xFF;
    flagObject = target;

    switch (opcode) {
    case eProc_SetFullModuleOrientation:
    case eProc_SetFullModuleOrientationWithOffset:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, argument);
        channel = argument->u.aucValue[0];
        channelObject = inv_AI_fn_p_stChannelSuperObject(target, channel);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, argument);
        worldSpace = argument->u.aucValue[0];
        if (worldSpace & 0x100) {
            flagObject = target;
        }
        hasPivot |= (opcode == eProc_SetFullModuleOrientationWithOffset);
    case eProc_SetFullPersoOrientation:
    case eProc_SetFullPersoOrientationWithOffset:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, argument);
        primary = argument->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, argument);
        secondary = argument->u.stVector;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, argument);
        axis = argument->u.aucValue[0];
        hasPivot |= (opcode == eProc_SetFullPersoOrientationWithOffset);
        if (hasPivot) {
            p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, argument);
            pivot = argument->u.stVector;
        }
        break;
    }

    if (channel != 0xFF) {
        if (!worldSpace) {
            POS_tdstMatrix_* targetMatrix = target->gu_hGlobalMatrix;

            POS_fn_vApplyMatrixToVector(&primary, targetMatrix, &primary);
            POS_fn_vApplyMatrixToVector(&secondary, targetMatrix, &secondary);
        }
        if (channelObject != NULL) {
            target = channelObject;
        }
    }
    matrix = target->gu_hGlobalMatrix;
    if (hasPivot != 0) {
        POS_fn_vApplyMatrixToVector(&pivotWorld, matrix, &pivot);
    }
    POS_fn_vGetRotationVectors(matrix, &axisI, &axisJ, &axisK);

    if (MTH3D_M_bIsZeroXYZ_inv(&primary)) {
        switch (axis) {
        case 0:
        case 1:
            primary = axisJ;
            negate = axis < 2;
            break;
        case 2:
        case 3:
            primary = axisK;
            negate = axis < 2;
            break;
        case 4:
        case 5:
            primary = axisI;
            negate = axis < 2;
            break;
        default:
            goto otherAxis;
        }
    } else {
        f32 lengthSquared, length, inverse;

        lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(primary.xX, primary.xY, primary.xZ);
        length = MTH_fn_xSqrtOpt2(lengthSquared);
        if (length != 0.0f) {
            negate = axis < 2;
            inverse = 1.0f / length;
            MTH3D_M_vKTimesXYZ_inv(&primary, inverse, &primary);
        } else {
            negate = axis < 2;
        }
        if (negate) {
            inv_AI_fn_vNegate(&primary);
        }
    }
    goto primaryDone;
otherAxis:
    negate = axis < 2;
primaryDone:

    if (MTH3D_M_bIsZeroXYZ_inv(&secondary)) {
        switch (axis) {
        case 0:
        case 4:
            secondary = axisK;
            break;
        case 1:
        case 3:
            secondary = axisI;
            break;
        case 2:
        case 5:
            secondary = axisJ;
            break;
        }
    } else {
        f32 lengthSquared, length, inverse;

        lengthSquared = MTH_fn_xSqrAddSqrAddSqrOpt2(secondary.xX, secondary.xY, secondary.xZ);
        length = MTH_fn_xSqrtOpt2(lengthSquared);
        if (length != 0.0f) {
            negatedAxis = 2;
            inverse = 1.0f / length;
            MTH3D_M_vKTimesXYZ_inv(&secondary, inverse, &secondary);
        } else {
            negatedAxis = 2;
        }
        if (axis == negatedAxis || axis == 5) {
            inv_AI_fn_vNegate(&secondary);
        }
    }

    switch (axis) {
    case 5:
        DNM_p_stVectorCrossProduct(&cross, &primary, &secondary);
        if (!MTH3D_M_bIsZeroXYZ_inv(&cross)) {
            inv_AI_fn_vNormalizeOrCopy(&axisK, &cross);
        }
        DNM_p_stVectorCrossProduct(&axisJ, &axisK, &primary);
        break;
    case 4:
        DNM_p_stVectorCrossProduct(&cross, &secondary, &primary);
        if (!MTH3D_M_bIsZeroXYZ_inv(&cross)) {
            inv_AI_fn_vNormalizeOrCopy(&axisJ, &cross);
        }
        DNM_p_stVectorCrossProduct(&axisK, &primary, &axisJ);
        break;
    case 3:
        DNM_p_stVectorCrossProduct(&cross, &primary, &secondary);
        if (!MTH3D_M_bIsZeroXYZ_inv(&cross)) {
            inv_AI_fn_vNormalizeOrCopy(&axisJ, &cross);
        }
        DNM_p_stVectorCrossProduct(&axisI, &axisJ, &primary);
        break;
    case 2:
        DNM_p_stVectorCrossProduct(&cross, &secondary, &primary);
        if (!MTH3D_M_bIsZeroXYZ_inv(&cross)) {
            inv_AI_fn_vNormalizeOrCopy(&axisI, &cross);
        }
        DNM_p_stVectorCrossProduct(&axisJ, &primary, &axisI);
        break;
    case 1:
        DNM_p_stVectorCrossProduct(&cross, &secondary, &primary);
        if (!MTH3D_M_bIsZeroXYZ_inv(&cross)) {
            inv_AI_fn_vNormalizeOrCopy(&axisK, &cross);
        }
        DNM_p_stVectorCrossProduct(&axisI, &primary, &axisK);
        break;
    case 0:
        DNM_p_stVectorCrossProduct(&cross, &primary, &secondary);
        if (!MTH3D_M_bIsZeroXYZ_inv(&cross)) {
            inv_AI_fn_vNormalizeOrCopy(&axisI, &cross);
        }
        DNM_p_stVectorCrossProduct(&axisK, &axisI, &primary);
        break;
    }

    if (negate) {
        _POS_fn_vSetRotationMatrix(matrix, &axisI, &primary, &axisK);
    } else if (axis < 4) {
        _POS_fn_vSetRotationMatrix(matrix, &axisI, &axisJ, &primary);
    } else {
        _POS_fn_vSetRotationMatrix(matrix, &primary, &axisJ, &axisK);
    }

    if (hasPivot != 0) {
        translation = &matrix->stMatrix.stCol_3;
        POS_fn_vApplyMatrixToVector(&cross, matrix, &pivot);
        MTH3D_M_vSubXYZ_inv(&pivotWorld, &pivotWorld, &cross);
        MTH3D_M_vAddXYZ_inv(translation, translation, &pivotWorld);
    }

    func_001F6020(target, matrix, hasPivot != 0 ? &pivot : NULL, channel == 0xFF);
    if (channel != 0xFF) {
        ((tdstEngineObject_*) flagObject->hLinkedObject)->h_3dData->bAnimMatrixChanged = 1;
    }
    inv_AI_parameterPoolCursor = argument;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stChangePersoHorizSighting(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                    tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    MTH3D_tdstVector_ side;
    MTH3D_tdstVector_ up;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    void* globalMatrix;
    void* localMatrix;
    tdstDynam_* dynam;
    MTH3D_tdstVector_* sight;
    u32 ulCustomBits;
    f32 x;
    f32 y;
    f32 z;
    f32 axisX;
    f32 axisY;
    f32 squareLength;
    f32 length;
    f32 inverse;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    sight = &scratch->u.stVector;
    globalMatrix = hNewSuperObjPerso->gu_hGlobalMatrix;
    localMatrix = hNewSuperObjPerso->gu_hLocalMatrix;
    dynam = ((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_Dynam;
    x = sight->xX;
    y = sight->xY;
    sight->xZ = 0.0f;
    z = sight->xZ;
    if (x == z && y == z) {
        POS_fn_vGetYAxisVector(hNewSuperObjPerso->gu_hGlobalMatrix, sight);
        do {
            axisX = sight->xX;
        } while (0);
        axisX = -axisX;
        axisY = inv_MTH_fn_xNegateTied(sight->xY);
        MTH3D_M_vSetXYZ_inv(sight, axisX, axisY, z);
        if (axisX == z && axisY == z) {
            POS_fn_vGetRotationVector(hNewSuperObjPerso->gu_hGlobalMatrix, sight);
            x = sight->xX;
            z = sight->xZ;
            y = sight->xY;
        } else {
            x = axisX;
            y = axisY;
        }
    }
    squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
    length = MTH_fn_xSqrtOpt2(squareLength);
    if (length != 0.0f) {
        inverse = 1.0f / length;
        MTH3D_M_vSetXYZ_inv(sight, inverse * x, inverse * y, inverse * z);
    }
    side.xZ = 0.0f;
    MTH3D_M_vSetXYZ_inv(&up, 0.0f, 0.0f, 1.0f);
    side.xY = sight->xX;
    sight->xX = -sight->xX;
    sight->xY = -sight->xY;
    side.xX = sight->xY;
    _POS_fn_vSetRotationMatrix(globalMatrix, &side, sight, &up);
    hNewSuperObjPerso->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
    if (hNewSuperObjPerso->hFatherSta->lTypeOfLinkedObject & HIE_TYPE_ACTOR_inv) {
        HIE_fn_vComputeNewRelativeMatrix(hNewSuperObjPerso);
    } else {
        _POS_fn_vSetRotationMatrix(localMatrix, &side, sight, &up);
    }
    ulCustomBits = fn_ulStandardGameGetCustomBitsSO(hNewSuperObjPerso);
    if ((ulCustomBits & 2) && (dynam == NULL || (ulCustomBits & 0x8000))) {
        HIE_fn_vRefreshHierarchy(hNewSuperObjPerso);
    } else {
        HIE_fn_vInvalidateHierarchyFrame(hNewSuperObjPerso);
    }
    if (dynam != NULL) {
        MEC_vImposeRotationVector(dynam->p_stDynamics, &side, sight, &up);
    }
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

static inline HIE_tdstSuperObject_* inv_fn_hGetChannelSuperObject(HIE_tdstSuperObject_* superObject,
                                                                 u16 index)
{
    tdstAChannel_* channels =
        ((tdstEngineObject_*) superObject->hLinkedObject)->h_3dData->hArrayOfChannels;

    return channels != NULL ? channels[index].hSupObject : NULL;
}

static inline f32 inv_fn_xSign(f32 value)
{
    union {
        f32 f;
        u32 i;
    } u;

    u.f = value;
    if ((u.i & 0x7FFFFFFF) == 0)
        return 0.0f;
    u.i = (u.i & 0x80000000) | 0x3F800000;
    return u.f;
}

static inline void inv_fn_vSetTranslation(MTH4D_tdstVector_* translation, MTH3D_tdstVector_* vector)
{
    *(MTH3D_tdstVector_*) translation = *vector;
    translation->xW = 1.0f;
}

static inline void inv_fn_vNormalize(MTH3D_tdstVector_* vector)
{
    f32 squareLength;
    f32 length;
    f32 inverse;

    squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(vector->xX, vector->xY, vector->xZ);
    length = MTH_fn_xSqrtOpt2(squareLength);
    if (length != 0.0f) {
        inverse = 1.0f / length;
        MTH3D_M_vKTimesXYZ_inv(vector, inverse, vector);
    }
}

static inline void inv_fn_vNormalizeTo(MTH3D_tdstVector_* dest, MTH3D_tdstVector_* source)
{
    f32 squareLength;
    f32 length;
    f32 inverse;

    squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(source->xX, source->xY, source->xZ);
    length = MTH_fn_xSqrtOpt2(squareLength);
    if (length != 0.0f) {
        inverse = 1.0f / length;
        MTH3D_M_vKTimesXYZ_inv(dest, inverse, source);
    } else {
        *dest = *source;
    }
}

void* fn_p_st3DProc(HIE_tdstSuperObject_* ctx, tdstNodeInterpret_* stream)
{
    POS_tdstMatrix_ savedPos;
    MTH3D_tdstVector_ vec60;
    MTH3D_tdstVector_ vec70;
    MTH3D_tdstVector_ vec80;
    MTH3D_tdstVector_ vec90;
    MTH3D_tdstVector_ axisA;
    MTH3D_tdstVector_ axisB;
    MTH3D_tdstVector_ axisC;
    POS_tdstMatrix_ tempPos;
    MTH3D_tdstVector_ vec130;
    MTH3D_tdstVector_ vec140;
    MTH3D_tdstVector_ vec150;
    HIE_tdstSuperObject_* current;
    s32 opcode;
    HIE_tdstSuperObject_* original;
    s32 usePosition;
    s32 dynamicVector;
    tdstDynam_* dynamics;
    HIE_tdstSuperObject_* selected;
    tdstGetSetParam_* param;
    POS_tdstMatrix_* originalPos;
    POS_tdstMatrix_* pos;
    u8 localMode;
    u8 objectIndex;
    f32 amount;
    f32 length;
    f32 c;
    f32 s;
    f32 sign;
    f32 base;
    f32 wrap;
    f32 rem;
    u32 bits;
    u32 bits2;
    s32 whole;
    s32 signA;
    s32 signB;
    u32 input;
    DNM_stDynamics* dynObj;
    tdstNodeInterpret_* next;
    s32 one_;

    objectIndex = 0xFF;
    localMode = 0;
    usePosition = 0;
    dynamicVector = 0;
    one_ = 1;
    selected = 0;
    amount = 0.0f;
    param = inv_AI_parameterPoolCursor;
    opcode = *(u16 *)((u8 *)stream - 8);
    current = ctx;
    inv_AI_parameterPoolCursor = param + 1;
    fn_vGetUltraOperatorPerso(ctx, &current);
    original = current;
    originalPos = original->gu_hGlobalMatrix;
    dynamics = ((tdstEngineObject_*) original->hLinkedObject)->h_Dynam;
    stream = fn_p_stEvalTree(ctx, stream, param);

    switch (opcode) {
    case 0xD7:
        objectIndex = param->u.lValue;
        selected = inv_fn_hGetChannelSuperObject(current, objectIndex);
        stream = fn_p_stEvalTree(ctx, stream, param);
    case 0x9F:
        vec60 = param->u.stVector;
        stream = fn_p_stEvalTree(ctx, stream, param);
        if (param->ucType == E_vt_Float)
            amount = param->u.xValue;
        else
            amount = (f32)param->u.lValue;
        amount = -(amount * MTH_DEG_TO_RAD_inv);
        __asm__("" : "=r"(usePosition) : "r"(one_), "f"(amount) : "memory");
        stream = fn_p_stEvalTree(ctx, stream, param);
        vec80 = param->u.stVector;
        goto set_local;

    case 0xD6:
        objectIndex = param->u.lValue;
        selected = inv_fn_hGetChannelSuperObject(current, objectIndex);
        stream = fn_p_stEvalTree(ctx, stream, param);
    case 0x9E:
        vec60 = param->u.stVector;
        stream = fn_p_stEvalTree(ctx, stream, param);
        if (param->ucType == E_vt_Float)
            amount = param->u.xValue;
        else
            amount = (f32)param->u.lValue;
        localMode = 1;
        amount = -(amount * MTH_DEG_TO_RAD_inv);
        break;

    case 0x96:
        ctx = ((HIE_tdstSuperObject_**) ((HIE_tdstSuperObject_*) param->u.lValue)->hLinkedObject)[6];
        stream = fn_p_stEvalTree(ctx, stream, param);
        selected = inv_fn_hGetChannelSuperObject(ctx, *(u8 *)param);
        if (selected == 0) {
            inv_AI_parameterPoolCursor = param;
            return fn_p_stSwapNextParameter(stream);
        }
        objectIndex = 0xFF;
        usePosition = 1;
        localMode = 1;
        break;

    case 0xDD:
        objectIndex = param->u.lValue;
        selected = inv_fn_hGetChannelSuperObject(current, objectIndex);
        stream = fn_p_stEvalTree(ctx, stream, param);
    case 0xA5:
        vec60 = param->u.stVector;
        localMode = 1;
        break;

    case 0xD8:
    case 0xD9:
    case 0xDA:
        objectIndex = param->u.lValue;
        selected = inv_fn_hGetChannelSuperObject(current, objectIndex);
        stream = fn_p_stEvalTree(ctx, stream, param);
    case 0xA0:
    case 0xA1:
    case 0xA2:
        if (param->ucType == E_vt_Float)
            amount = param->u.xValue;
        else
            amount = (f32)param->u.lValue;
        amount = amount * MTH_DEG_TO_RAD_inv;
        usePosition = 1;
        localMode = 1;
        next = fn_p_stEvalTree(ctx, stream, param);
        vec60 = param->u.stVector;
        stream = next;
        break;

    case 0xD3:
    case 0xD4:
    case 0xD5:
        objectIndex = param->u.lValue;
        selected = inv_fn_hGetChannelSuperObject(current, objectIndex);
        stream = fn_p_stEvalTree(ctx, stream, param);
    case 0x9B:
    case 0x9C:
    case 0x9D:
        if (param->ucType == E_vt_Float)
            amount = param->u.xValue;
        else
            amount = (f32)param->u.lValue;
        localMode = 1;
        amount = amount * MTH_DEG_TO_RAD_inv;
        break;

    case 0x95:
        usePosition = 1;
        vec60 = param->u.stVector;
        localMode = 1;
        break;

    case 0x99:
    case 0x9A:
        vec60 = param->u.stVector;
        if (opcode != 0x9A) {
            localMode = 1;
            break;
        }
        usePosition = 1;
        next = fn_p_stEvalTree(ctx, stream, param);
        vec80 = param->u.stVector;
        stream = next;
    set_local:
        localMode = 1;
        break;

    case 0x98:
        vec60 = param->u.stVector;
        stream = fn_p_stEvalTree(ctx, stream, param);
        usePosition = 1;
        localMode = param->u.lValue;
        break;

    case 0xCF:
    case 0xD0:
    case 0xD1:
    case 0xD2:
        vec60 = param->u.stVector;
        stream = fn_p_stEvalTree(ctx, stream, param);
        objectIndex = param->u.lValue;
        selected = inv_fn_hGetChannelSuperObject(current, objectIndex);
        stream = fn_p_stEvalTree(ctx, stream, param);
        localMode = param->u.lValue;
        if (opcode == 0xD2) {
            usePosition = 1;
            next = fn_p_stEvalTree(ctx, stream, param);
            vec80 = param->u.stVector;
            stream = next;
        }
        break;
    }

    if (objectIndex != 0xFF) {
        if (selected == 0) {
            inv_AI_parameterPoolCursor = param;
            return stream;
        }
        current = selected;
    }

    pos = current->gu_hGlobalMatrix;
    if (!localMode)
        savedPos = *originalPos;

    switch (opcode) {
    case 0x9F:
    case 0xD7:
        POS_fn_vApplyMatrixToVector(&vec70, pos, &vec80);
    case 0x9E:
    case 0xD6:
        inv_fn_vNormalize(&vec60);
        POS_fn_vRotatePositionAroundAxis(pos, &vec60, amount);
        if (!usePosition)
            break;
        POS_fn_vApplyMatrixToVector(&vec60, pos, &vec80);
        MTH3D_M_vSubXYZ_inv(&vec60, &vec70, &vec60);
        goto add_position;

    case 0x96:
        *pos = *(POS_tdstMatrix_* )selected->gu_hGlobalMatrix;
        break;

    case 0xA5:
        dynamicVector = 1;
    case 0xDD:
        if (current->flags & 8) {
            vec60.xY = vec60.xX;
            vec60.xZ = vec60.xX;
        }
        if (objectIndex != 0xFF || dynamics == 0) {
            MTH3D_M_vCopyXYZ_inv(&vec90, &vec60);
            POS_fn_vSetScaleMatrix(pos, &vec90);
        }
        break;

    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xD8:
    case 0xD9:
    case 0xDA:
        POS_fn_vApplyMatrixToVector(&vec70, pos, &vec60);
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0xD3:
    case 0xD4:
    case 0xD5:
        rem = amount * MTH_INV_TWO_PI_inv;
        wrap = MTH_TWO_PI_inv;
        whole = inv_MTH_fn_lToIntegerOrdered(rem);
        rem = amount - (f32)whole * wrap;
        c = cosf(amount);
        if (__builtin_fabsf(rem) < MTH_PI_inv) {
            sign = inv_MTH_fn_xSign(rem);
        } else {
            bits = *(u32 *)&rem;
            if ((bits & 0x7FFFFFFF) == 0) {
                base = 0.0f;
            } else {
                bits = (bits & 0x80000000) | 0x3F800000;
                base = *(f32 *)&bits;
            }
            sign = -base;
        }
        s = MTH_fn_xSqrtOpt2(1.0f - c * c) * sign;
        switch (opcode) {
        case 0x9B:
        case 0xA0:
        case 0xD3:
        case 0xD8:
            MTH3D_M_vSetXYZ_inv(&axisA, 1.0f, 0.0f, 0.0f);
            MTH3D_M_vSetXYZ_inv(&axisB, 0.0f, c, s);
            MTH3D_M_vSetXYZ_inv(&axisC, 0.0f, -s, c);
            break;
        case 0x9C:
        case 0xA1:
        case 0xD4:
        case 0xD9:
            MTH3D_M_vSetXYZ_inv(&axisA, c, 0.0f, s);
            MTH3D_M_vSetXYZ_inv(&axisB, 0.0f, 1.0f, 0.0f);
            MTH3D_M_vSetXYZ_inv(&axisC, -s, 0.0f, c);
            break;
        case 0x9D:
        case 0xA2:
        case 0xD5:
        case 0xDA:
            MTH3D_M_vSetXYZ_inv(&axisA, c, s, 0.0f);
            MTH3D_M_vSetXYZ_inv(&axisB, -s, c, 0.0f);
            MTH3D_M_vSetXYZ_inv(&axisC, 0.0f, 0.0f, 1.0f);
            break;
        }
        inv_POS_fn_vRemoveScaleFromMatrix(&tempPos, pos);
        POS_fn_vApplyMatrixToVector(&axisA, &tempPos, &axisA);
        POS_fn_vApplyMatrixToVector(&axisB, &tempPos, &axisB);
        POS_fn_vApplyMatrixToVector(&axisC, &tempPos, &axisC);
        _POS_fn_vSetRotationMatrix(pos, &axisA, &axisB, &axisC);
        if (!usePosition)
            break;
        POS_fn_vApplyMatrixToVector(&vec60, pos, &vec60);
        MTH3D_M_vSubXYZ_inv(&vec60, &vec70, &vec60);
        goto add_position;

    case 0x9A:
    case 0xD2:
        POS_fn_vApplyMatrixToVector(&param->u.stVector, pos, &vec80);
    case 0x99:
    case 0xCF:
        POS_fn_vGetRotationVectors(pos, &vec130, &vec70, &vec150);
        if (MTH3D_M_bIsZeroXYZ_inv(&vec60)) {
            vec140 = vec70;
        } else {
            MTH3D_M_vSetXYZ_inv(&vec60, inv_MTH_fn_xNegate(vec60.xX), inv_MTH_fn_xNegate(vec60.xY),
                                inv_MTH_fn_xNegate(vec60.xZ));
            if (!localMode)
                POS_fn_vApplyMatrixToVector(&vec60, &savedPos, &vec60);
            inv_fn_vNormalizeTo(&vec140, &vec60);
        }

        if (__builtin_fabsf(vec140.xX) < 0.001f && __builtin_fabsf(vec140.xY) < 0.001f) {
            if (__builtin_fabsf(vec70.xX) < 0.001f && __builtin_fabsf(vec70.xY) < 0.001f)
                vec70 = vec150;
        } else {
            MTH3D_M_vSetXYZ_inv(&vec70, inv_MTH_fn_xNegate(vec140.xX),
                                inv_MTH_fn_xNegate(vec140.xY), vec140.xZ + 2.0f);
        }

        signA = 0.0f < inv_fn_xSign(vec150.xZ);

        sign = inv_fn_xSign(vec130.xX);

        vec130.xX = inv_MTH_fn_xMulSub(vec70.xY, vec140.xZ, vec70.xZ, vec140.xY);
        vec130.xY = inv_MTH_fn_xMulSub(vec70.xZ, vec140.xX, vec70.xX, vec140.xZ);

        vec150.xX = inv_MTH_fn_xMulSub(vec130.xY, vec140.xZ, 0.0f, vec140.xY);
        vec150.xY = inv_MTH_fn_xMulSub(0.0f, vec140.xX, vec130.xX, vec140.xZ);
        vec130.xZ = 0.0f;
        vec150.xZ = inv_MTH_fn_xMulSub(vec130.xX, vec140.xY, vec130.xY, vec140.xX);

        signB = 0.0f < inv_fn_xSign(vec150.xZ);
        if (signB != signA) {
            MTH3D_M_vSetXYZ_inv(&vec150, inv_MTH_fn_xNegateTied(vec150.xX),
                                inv_MTH_fn_xNegateTied(vec150.xY),
                                inv_MTH_fn_xNegateTied(vec150.xZ));
        }

        MTH3D_M_vSetXYZ_inv(&vec130, inv_MTH_fn_xMulSub(vec140.xY, vec150.xZ, vec140.xZ, vec150.xY),
                            inv_MTH_fn_xMulSub(vec140.xZ, vec150.xX, vec140.xX, vec150.xZ),
                            inv_MTH_fn_xMulSub(vec140.xX, vec150.xY, vec140.xY, vec150.xX));

        inv_fn_vNormalize(&vec130);
        inv_fn_vNormalize(&vec150);
        _POS_fn_vSetRotationMatrix(pos, &vec130, &vec140, &vec150);
        if (!usePosition)
            break;
        POS_fn_vApplyMatrixToVector(&vec80, pos, &vec80);
        MTH3D_M_vSubXYZ_inv(&vec60, &param->u.stVector, &vec80);

    case 0x98:
    case 0xD1:
    add_position:
        if (!localMode)
            POS_fn_vApplyMatrixToVector(&vec60, &savedPos, &vec60);
        vec70 = *inv_POS_fn_p_stPosMatrixTranslation(pos);
        MTH3D_M_vAddXYZ_inv(&vec60, &vec60, &vec70);
        goto store_position;

    case 0xD0:
        if (!localMode)
            POS_fn_vMulMatrixVertex(&vec60, &savedPos, &vec60);
    case 0x95:
    store_position:
        vec80 = *inv_POS_fn_p_stPosMatrixTranslation(pos);
        if (pos->ucType == MAT_eTransform_Identity)
            pos->ucType = MAT_eTransform_Translation;
        inv_fn_vSetTranslation((MTH4D_tdstVector_ *)&pos->stMatrix.stCol_3, &vec60);
        if ((opcode == 0x95 || opcode == 0x98) && dynamics != 0)
            MEC_vImposeTranslation(dynamics->p_stDynamics, &vec60);
        break;

    default:
        goto done;
    }

    if (opcode & 0x10000)
        dynamicVector = one_;
    current->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
    HIE_fn_vComputeNewRelativeMatrix(current);
    if (current->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv ||
        (((input = fn_ulStandardGameGetCustomBitsSO(current)) & 2) &&
         (dynamics == 0 || (input & 0x8000))))
        HIE_fn_vRefreshHierarchy(current);
    else
        HIE_fn_vInvalidateHierarchyFrame(current);

    if (objectIndex == 0xFF && dynamics != 0) {
        if (dynamicVector && inv_MEC_fn_p_stDynamicsOf(dynamics) != 0) {
            dynObj = inv_MEC_fn_p_stDynamicsOf(dynamics);
            MTH3D_M_vCopyXYZ_inv(&dynObj->m_tdstDynamicsBaseBlock.m_stScale, &vec60);
            dynObj->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x1000;
        }
        MEC_vImposeRotationMatrix(inv_MEC_fn_p_stDynamicsOf(dynamics), pos);
    }

done:
    if (objectIndex != 0xFF)
        ((tdstEngineObject_*) original->hLinkedObject)->h_3dData->bAnimMatrixChanged = 1;

    inv_AI_parameterPoolCursor = param;
    return stream;
}

void* fn_p_stWorldMapProc(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* perso;
    tdstGetSetParam_* scratch;
    void* dsgData;
    s32 eProcedureId;
    u8 ucVarId;

    eProcedureId = AI_NODE_ID(p_stTree);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch + 1;

    switch (eProcedureId) {
    case eProc_SetGlobalCounter:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        g_stEngineStructure.ucLevelGlobalCounter = *(u8*) scratch;
        break;

    case eProc_GetUsedExitIdentifier:
        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree + 1, &perso);
        } else {
            perso = p_SuperObjPerso;
        }
        ucVarId = (u8) p_stTree->param;
        dsgData = *(void**) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) perso)->hLinkedObject)
                       ->h_Brain;
        p_stTree++;
        if (g_stEngineStructure.xDemoMode != 0) {
            g_stEngineStructure.ucPreviousLevel = 0;
            scratch->ucType = E_vt_Integer;
            scratch->u.lValue = 0;
        } else {
            scratch->u.lValue = g_stEngineStructure.ucPreviousLevel;
        }
        scratch->ucType = E_vt_Integer;
        fn_ucSetDsgVar(ucVarId, 0, dsgData, scratch);

        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree + 1, &perso);
        } else {
            perso = p_SuperObjPerso;
        }
        ucVarId = (u8) p_stTree->param;
        dsgData = *(void**) ((tdstEngineObject_*) ((HIE_tdstSuperObject_*) perso)->hLinkedObject)
                       ->h_Brain;
        p_stTree++;
        if (g_stEngineStructure.xDemoMode != 0) {
            g_stEngineStructure.ucExitIdToQuitPrevLevel = 0;
            scratch->ucType = E_vt_Integer;
            scratch->u.lValue = 0;
        } else {
            scratch->u.lValue = g_stEngineStructure.ucExitIdToQuitPrevLevel;
        }
        scratch->ucType = E_vt_Integer;
        fn_ucSetDsgVar(ucVarId, 0, dsgData, scratch);
        break;

    case eProc_SetUsedExitIdentifier:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        if (g_stEngineStructure.xDemoMode == 0) {
            g_stEngineStructure.ucPreviousLevel = *(u8*) scratch;
        }
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        if (g_stEngineStructure.xDemoMode == 0) {
            g_stEngineStructure.ucExitIdToQuitPrevLevel = *(u8*) scratch;
        }
        break;

    default:
        break;
    }

    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stOptionChangerProc(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param = (tdstGetSetParam_*) inv_AI_parameterPoolCursor;
    s32 kind = AI_NODE_ID(p_stTree);
    IPT_tdstEntryElement_* first;

    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) param + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    switch (kind) {
    case eProc_OptionConfigureKey:
        first = param->u.pvValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        IPT_fn_bModifyIPTAction(first, param->u.pvValue);
        break;
    case eProc_OptionChangeActionKey:
        IPT_fn_bChangeIPTValue((void*) param->u.lValue, 1);
        break;
    case 0x90:
        fn_vChangeMusicVolumeValue(param->u.lValue);
        break;
    case 0x91:
        fn_vChangeFxVolumeValue(param->u.lValue);
        break;
    case 0x92:
        fn_vChangeVoiceVolumeValue(param->u.lValue);
        break;
    case eProc_OptionChangeDetailsValue:
    default:
        break;
    }
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) param;
    return p_stTree;
}

void* fn_p_stChangeScreen(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    f32 fValue;
    s32 op;
    s32 id;
    s32 scaled;
    s32 lFirstValue;
    s32 lSecondValue;
    s32 lThirdValue;
    s32 lFourthValue;

    op = AI_NODE_ID(p_stTree);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    id = scratch->u.lValue;
    scaled = 0;
    switch (op) {
    case eProc_ChangeScreenClip:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lFirstValue = scratch->u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lSecondValue = scratch->u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lThirdValue = scratch->u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lFourthValue = scratch->u.lValue;
        fn_vChangeViewPortPercent(id, 2, 0, 0, lFirstValue, lSecondValue, lThirdValue, lFourthValue, 0, 0);
        break;

    case eProc_ChangeScreenSize:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lFirstValue = scratch->u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lSecondValue = scratch->u.lValue;
        lFirstValue = (lFirstValue >= 0) ? lFirstValue : 0;
        lSecondValue = (lSecondValue >= 0) ? lSecondValue : 0;
        fn_vChangeViewPortPercent(id, 1, (lFirstValue > 100) ? 100 : lFirstValue, (lSecondValue > 100) ? 100 : lSecondValue, 0, 0, 0, 0,
                                  0, 0);
        break;

    case 0x134:
        scaled = 1;
    case 0x133:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lFirstValue = scratch->u.lValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        lSecondValue = scratch->u.lValue;
        fn_vChangeViewPortPercent(id, scaled + 3, 0, 0, 0, 0, 0, 0, lFirstValue, lSecondValue);
        break;

    case eProc_ChangeScreenRatio:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        if (scratch->ucType == E_vt_Float) {
            fValue = scratch->u.xValue;
        } else {
            fValue = (f32) scratch->u.lValue;
        }
        fn_vChangeViewportRatio(id, fValue);
        break;

    default:
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stFixePositionPerso(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* hPerso;
    tdstDynam_* sub2;
    POS_tdstMatrix_* xf;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    hPerso = ((tdstEngineObject_*) scratch->u.lValue)->h_StandardGame->h_SuperObject;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    sub2 = ((tdstEngineObject_*) hPerso->hLinkedObject)->h_Dynam;
    if (sub2 != 0) {
        MEC_vInitTranslation(sub2->p_stDynamics, hPerso, &scratch->u.stVector);
        inv_AI_parameterPoolCursor = scratch;
    } else {
        xf = hPerso->gu_hGlobalMatrix;
        if (xf->ucType == MAT_eTransform_Identity) {
            xf->ucType = MAT_eTransform_Translation;
        }
        *(MTH3D_tdstVector_*) inv_POS_fn_p_stPosMatrixTranslation(xf) = *(MTH3D_tdstVector_*) scratch;
        xf->stMatrix.stCol_3.xW = 1.0f;
        HIE_fn_vComputeNewRelativeMatrix(hPerso);
        inv_AI_parameterPoolCursor = scratch;
    }
    return p_stTree;
}

void* fn_p_stFixePositionZDx(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* param;
    tdstGetSetParam_ pointA;
    tdstGetSetParam_ pointB;
    tdstGetSetParam_ delta;
    u16 indexA;
    u16 indexB;
    void* target;
    s32 kind;
    u32 value;
    s16 type;
    COL_tdstCollideObject_* node;

    param = (tdstGetSetParam_*) inv_AI_parameterPoolCursor;
    kind = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) param + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    target = (*(tdstEngineObject_**) param)->h_StandardGame->h_SuperObject;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    value = param->u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    switch (kind) {
    case eProc_FixePositionZDM:
        node = fn_hGetGeometricZdxOfTypeAtIndex(1, (value - 1) & 0xFFFF, target);
        break;
    case eProc_FixePositionZDD:
        node = fn_hGetGeometricZdxOfTypeAtIndex(0, (value - 1) & 0xFFFF, target);
        break;
    case eProc_FixePositionZDE:
        node = fn_hGetGeometricZdxOfTypeAtIndex(2, (value - 1) & 0xFFFF, target);
        break;
    default:
        node = (void*) 0;
        break;
    }
    if (node != (void*) 0) {
        type = *node->d_xListOfElementsTypes;
        switch (type) {
        case 7:
            GEO_vSetPointOfObject(node, &param->u.stVector, 0);
            break;
        case 8:
            GEO_vGetMinMaxPointOfIndexedAlignedBox(node, 0, 0, &indexA, &indexB);
            GEO_vGetPointOfObject(node, &pointA.u.stVector, (s16) indexA);
            GEO_vGetPointOfObject(node, &pointB.u.stVector, (s16) indexB);
            inv_MTH3D_fn_vSubXYZ(&delta.u.stVector, &param->u.stVector, &pointA.u.stVector);
            inv_MTH3D_fn_vAddXYZ(&pointA.u.stVector, &pointA.u.stVector, &delta.u.stVector);
            inv_MTH3D_fn_vAddXYZ(&pointB.u.stVector, &pointB.u.stVector, &delta.u.stVector);
            GEO_vSetPointOfObject(node, &pointA.u.stVector, (s16) indexA);
            GEO_vSetPointOfObject(node, &pointB.u.stVector, (s16) indexB);
            break;
        default:
            break;
        }
    }
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) param;
    return p_stTree;
}

void* fn_p_SetMechanicOption(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstEngineObject_* engine;
    tdstDynam_* dynam;
    DNM_stDynamics* pstDynamic;
    u16 op;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    engine = hNewSuperObjPerso->hLinkedObject;
    dynam = engine->h_Dynam;
    pstDynamic = dynam->p_stDynamics;

    switch (op) {
    case eProc_SetMechanicAnimation:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x1)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x1);
        break;
    case eProc_SetMechanicCollide:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x2)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x2);
        if (scratch->u.lValue && !(pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & 0x80)) {
            pstDynamic->m_tdstDynamicsAdvancedBlock.m_stCollisionTranslation =
                *inv_POS_fn_p_stPosMatrixTranslation(
                    &pstDynamic->m_tdstDynamicsBaseBlock.m_stCurrentMatrix);
        }
        break;
    case eProc_SetMechanicGravity:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x4)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x4);
        break;
    case eProc_SetMechanicTilt:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x8)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x8);
        break;
    case eProc_SetMechanicGi:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x10)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x10);
        break;
    case eProc_SetMechanicClimb:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x40)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x40);
        break;
    case eProc_SetMechanicOnGround:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x20)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x20);
        break;
    case eProc_SetMechanicSpider:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x100)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x100);
        break;
    case eProc_SetMechanicShoot:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x200)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x200);
        break;
    case eProc_SetMechanicSwim:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x100000)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x100000);
        break;
    case eProc_SetMechanicNeverFall:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x200000)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x200000);
        break;
    case eProc_SetMechanicCollisionControl:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x80)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x80);
        break;
    case eProc_SetMechanicKeepSpeedZ:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x100)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x100);
        break;
    case eProc_SetMechanicSpeedLimit:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x200)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x200);
        break;
    case eProc_SetMechanicInertia:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x400)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x400);
        break;
    case eProc_SetMechanicStream:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x800)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x800);
        break;
    case eProc_SetMechanicStickOnPlatform:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x1000)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x1000);
        break;
    case eProc_SetMechanicScale:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x2000)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= ~0x2000);
        break;
    case eProc_SetMechanicNewMechanic:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags =
            scratch->u.lValue ? (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x4000000)
                              : (pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~0x4000000);
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stChangeLigthIntensity(HIE_tdstSuperObject_* p_SuperObjPerso,
                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* light;
    f32 red;
    f32 green;
    f32 blue;
    f32 intensity;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    light = (*(tdstEngineObject_**) scratch)->h_MSLight;

    if (light != 0) {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        red = scratch->u.xValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        green = scratch->u.xValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        blue = scratch->u.xValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        intensity = scratch->u.xValue;
        fn_vMSLightSetColorRGBA(light, red * intensity, green * intensity, blue * intensity, 0.0f);
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_SetMechanicRealParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                    tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* target;
    void* param;
    tdstGetSetParam_* p_stParam;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstDynam_* dynam;
    DNM_stDynamics* pstDynamic;
    u32 selector;
    f32 xValue;

    target = p_SuperObjPerso;
    param = p_stTree;
    p_stParam = inv_AI_parameterPoolCursor;
    selector = AI_NODE_ID(param);
    inv_AI_parameterPoolCursor = p_stParam + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    param = fn_p_stEvalTree(target, param, p_stParam);
    xValue = p_stParam->u.xValue;
    dynam = ((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_Dynam;
    pstDynamic = dynam->p_stDynamics;

    switch (selector) {
    case eProc_SetMechanicGravityFactor:
        pstDynamic->m_tdstDynamicsBaseBlock.m_xGravity = xValue;
        break;
    case eProc_SetMechanicSlide:
        pstDynamic->m_tdstDynamicsBaseBlock.m_xSlide = xValue;
        break;
    case eProc_SetMechanicRebound:
        pstDynamic->m_tdstDynamicsBaseBlock.m_xRebound = xValue;
        break;
    case eProc_SetMechanicSlopeLimit:
        pstDynamic->m_tdstDynamicsBaseBlock.m_xSlopeLimit = tanf(xValue * MTH_DEG_TO_RAD_inv);
        break;
    case eProc_SetMechanicInertiaX:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaX = xValue;
        break;
    case eProc_SetMechanicInertiaY:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaY = xValue;
        break;
    case eProc_SetMechanicInertiaZ:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaZ = xValue;
        break;
    case eProc_SetMechanicTiltIntensity:
        pstDynamic->m_tdstDynamicsComplexBlock.m_xTiltIntensity = xValue;
        break;
    case eProc_SetMechanicTiltInertia:
        pstDynamic->m_tdstDynamicsComplexBlock.m_xTiltInertia = xValue;
        break;
    case eProc_SetMechanicTiltOrigin:
        pstDynamic->m_tdstDynamicsComplexBlock.m_xTiltOrigin = xValue;
        break;
    case eProc_SetMechanicStreamPriority:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamPrority = xValue;
        break;
    case eProc_SetMechanicStreamFactor:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xStreamFactor = xValue;
        break;
    case eProc_MecSetSlideFactorX:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorX = xValue;
        break;
    case eProc_MecSetSlideFactorY:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorY = xValue;
        break;
    case eProc_MecSetSlideFactorZ:
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorZ = xValue;
        break;
    case eProc_MecSetHangingLimit:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x400000;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x182;
        pstDynamic->m_tdstDynamicsBaseBlock.m_xRebound = 0;
        pstDynamic->m_tdstDynamicsBaseBlock.m_xSlide = 1.0f;
        DNM_fn_vSetHangingLimit(pstDynamic, xValue);
        break;
    case eProc_MecSetBaseCoeff:
        MEC_g_xBaseCoeff = xValue;
        break;
    case eProc_MecSetWalkCoeff:
        MEC_g_xWalkCoeff = xValue;
        break;
    }

    inv_AI_parameterPoolCursor = p_stParam;
    return param;
}

void* fn_p_SetMechanic3RealParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
{
    void* scratch;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    void* p_stNext;
    f32 xInertiaX;
    f32 xInertiaY;
    f32 xInertiaZ;
    tdstDynam_* obj;
    DNM_stDynamics* pstDynamic;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    xInertiaX = *(f32*) scratch;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, scratch);
    xInertiaY = *(f32*) scratch;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, scratch);
    xInertiaZ = *(f32*) scratch;
    obj = ((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_Dynam;
    pstDynamic = obj->p_stDynamics;
    pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaX = xInertiaX;
    pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaY = xInertiaY;
    pstDynamic->m_tdstDynamicsAdvancedBlock.m_xInertiaZ = xInertiaZ;
    inv_AI_parameterPoolCursor = scratch;
    return p_stNext;
}

void* fn_p_SetMechanicVectorParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                      tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    DNM_stDynamics* mechanic;
    tdstGetSetParam_* p_stParam;
    MTH3D_tdstVector_ stNewVector;
    f32 workValue;
    f32 value;
    u16 operation;
    void* node;

    node = p_stTree;
    p_stParam = inv_AI_parameterPoolCursor;
    operation = AI_NODE_ID(node);
    inv_AI_parameterPoolCursor = p_stParam + 1;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    node = fn_p_stEvalTree(p_SuperObjPerso, node, p_stParam);
    mechanic = ((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)
                           ->h_Dynam
                   ->p_stDynamics;

    switch (operation) {
    case eProc_SetMechanicSpeedMax:
        stNewVector = mechanic->m_tdstDynamicsAdvancedBlock.m_stMaxSpeed;
        workValue = 0.0f;
        value = p_stParam->u.stVector.xX;
        if (workValue < value) {
            stNewVector.xX = value;
        }
        value = p_stParam->u.stVector.xY;
        if (workValue < value) {
            stNewVector.xY = value;
        }
        value = p_stParam->u.stVector.xZ;
        if (workValue < value) {
            stNewVector.xZ = value;
        }
        mechanic->m_tdstDynamicsAdvancedBlock.m_stMaxSpeed = stNewVector;
        inv_AI_parameterPoolCursor = p_stParam;
        break;
    case eProc_SetMechanicStreamSpeed:
        mechanic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed = p_stParam->u.stVector;
        inv_AI_parameterPoolCursor = p_stParam;
        break;
    case eProc_AddMechanicStreamSpeed:
        workValue = p_stParam->u.stVector.xX;
        MTH3D_M_vSetXYZ_inv(
            &stNewVector, mechanic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xX + workValue,
            mechanic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xY + p_stParam->u.stVector.xY,
            mechanic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed.xZ + p_stParam->u.stVector.xZ);
        mechanic->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed = stNewVector;
        inv_AI_parameterPoolCursor = p_stParam;
        break;
    default:
        inv_AI_parameterPoolCursor = p_stParam;
        break;
    }
    return node;
}

void* fn_p_SetMechanicSpeedVector(HIE_tdstSuperObject_* p_SuperObjPerso,
                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstEngineObject_* engine;
    tdstDynam_* dynam;
    DNM_stDynamics* pstDynamic;
    u16 op;

    scratch = inv_AI_parameterPoolCursor;
    op = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    engine = hNewSuperObjPerso->hLinkedObject;
    dynam = engine->h_Dynam;
    pstDynamic = dynam->p_stDynamics;

    switch (op) {
    case eProc_ProposeSpeed:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFF7FFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x100000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x400000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed = scratch->u.stVector;
        break;
    case eProc_ImposeSpeed:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFFBFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x20000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x40000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x80000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed = scratch->u.stVector;
        break;
    case eProc_AddSpeed:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFEFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x1000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x2000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed = scratch->u.stVector;
        break;
    case eProc_ProposeAbsoluteSpeed:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x108000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x400000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed = scratch->u.stVector;
        break;
    case eProc_ImposeAbsoluteSpeed:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x24000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x40000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x80000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed = scratch->u.stVector;
        break;
    case eProc_AddAbsoluteSpeed:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x10000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x1000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x2000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed = scratch->u.stVector;
        break;
    case eProc_MoveLimit:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x4000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x8000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x10000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit = scratch->u.stVector;
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_SetMechanicSpeedReal(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    void* scratch;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstDynam_* dynam;
    DNM_stDynamics* pstDynamic;
    u16 opcode;
    f32 xSpeedX;
    f32 xSpeedY;
    f32 xSpeedZ;

    scratch = inv_AI_parameterPoolCursor;
    opcode = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);

    dynam = ((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_Dynam;
    pstDynamic = dynam->p_stDynamics;

    switch (opcode) {
    case eProc_ImposeSpeedX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFFBFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00020000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = xSpeedX;
        break;

    case eProc_ImposeSpeedY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFFBFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00040000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xY = xSpeedY;
        break;

    case eProc_ImposeSpeedZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFFBFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00080000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xZ = xSpeedZ;
        break;

    case eProc_ImposeSpeedXY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFFBFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00020000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00040000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = xSpeedX;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xY = xSpeedY;
        break;

    case eProc_ImposeSpeedXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFFBFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00020000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00040000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00080000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_ProposeSpeedX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFF7FFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00100000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xX = xSpeedX;
        break;

    case eProc_ProposeSpeedY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFF7FFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xY = xSpeedY;
        break;

    case eProc_ProposeSpeedZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFF7FFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00400000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xZ = xSpeedZ;
        break;

    case eProc_ProposeSpeedXY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFF7FFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00100000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xX = xSpeedX;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xY = xSpeedY;
        break;

    case eProc_ProposeSpeedXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFF7FFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00100000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00400000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_ImposeAbsoluteSpeedX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00024000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = xSpeedX;
        break;

    case eProc_ImposeAbsoluteSpeedY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00044000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xY = xSpeedY;
        break;

    case eProc_ImposeAbsoluteSpeedZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00084000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xZ = xSpeedZ;
        break;

    case eProc_ImposeAbsoluteSpeedXY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00024000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00040000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = xSpeedX;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xY = xSpeedY;
        break;

    case eProc_ImposeAbsoluteSpeedXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00024000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00040000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00080000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_ProposeAbsoluteSpeedX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00108000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xX = xSpeedX;
        break;

    case eProc_ProposeAbsoluteSpeedY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00208000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xY = xSpeedY;
        break;

    case eProc_ProposeAbsoluteSpeedZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00408000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xZ = xSpeedZ;
        break;

    case eProc_ProposeAbsoluteSpeedXY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00108000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xX = xSpeedX;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed.xY = xSpeedY;
        break;

    case eProc_ProposeAbsoluteSpeedXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00108000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00200000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00400000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsBaseBlock.m_stProposeSpeed, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_AddSpeedX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFEFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00800000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xX = xSpeedX;
        break;

    case eProc_AddSpeedY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFEFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x01000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xY = xSpeedY;
        break;

    case eProc_AddSpeedZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFEFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x02000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xZ = xSpeedZ;
        break;

    case eProc_AddSpeedXY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFEFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x01000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xX = xSpeedX;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed.xY = xSpeedY;
        break;

    case eProc_AddSpeedXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFFFEFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x01000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x02000000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsAdvancedBlock.m_stAddSpeed, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_AddAbsoluteSpeedX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00010000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = xSpeedX;
        break;

    case eProc_AddAbsoluteSpeedY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00010000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x01000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xY = xSpeedY;
        break;

    case eProc_AddAbsoluteSpeedZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00010000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x02000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xZ = xSpeedZ;
        break;

    case eProc_AddAbsoluteSpeedXY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00010000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x01000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xX = xSpeedX;
        pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed.xY = xSpeedY;
        break;

    case eProc_AddAbsoluteSpeedXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00010000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x00800000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x01000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x02000000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsBaseBlock.m_stImposeSpeed, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_MoveLimitX:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x04000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit.xX = xSpeedX;
        break;

    case eProc_MoveLimitY:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x08000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit.xY = xSpeedY;
        break;

    case eProc_MoveLimitXYZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedX = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedY = *(f32*) scratch;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x04000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x08000000u;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x10000000u;
        MTH3D_M_vSetXYZ_inv(&pstDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit, xSpeedX, xSpeedY,
                            xSpeedZ);
        break;

    case eProc_MoveLimitZ:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        xSpeedZ = *(f32*) scratch;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags |= 0x10000000u;
        pstDynamic->m_tdstDynamicsAdvancedBlock.m_stLimit.xZ = xSpeedZ;
        break;

    case eProc_StopMoveLimitX:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFBFFFFFFu;
        break;
    case eProc_StopMoveLimitY:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xF7FFFFFFu;
        break;
    case eProc_StopMoveLimitZ:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xEFFFFFFFu;
        break;
    case eProc_StopMoveLimitXYZ:
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xFBFFFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xF7FFFFFFu;
        pstDynamic->m_tdstDynamicsBaseBlock.ulFlags &= 0xEFFFFFFFu;
        break;

    case eProc_MecSetHangingOff:
        pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= 0xFFBFFFFFu;
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_SPO_p_stChangeFlag(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* object;
    s32 bit;
    s32 mode;
    u32 flags;
    u32 mask;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    object = scratch->u.pvValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    p_stTree = fn_p_stEvalTree((bit = scratch->u.lValue, p_SuperObjPerso), p_stTree, scratch);
    mode = scratch->u.lValue;
    flags = object->flags;
    mask = 1U << bit;
    if (mode) {
        flags |= mask;
    } else {
        flags &= ~mask;
    }
    object->flags = flags;
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stChangeActorSpoFlag(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstGetSetParam_* record;
    u32 idx;
    u32 bit;
    u32 flags;

    record = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = record + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, record);
    idx = record->u.ulValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, record);
    flags = hNewSuperObjPerso->flags;
    bit = 1 << idx;
    if (record->u.ulValue != 0) {
        flags |= bit;
    } else {
        flags &= ~bit;
    }
    hNewSuperObjPerso->flags = flags;
    inv_AI_parameterPoolCursor = record;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_PlatformLink(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* hPerso;
    DNM_stDynamics* field;
    HIE_tdstSuperObject_* link;
    u16 type;
    tdstNodeInterpret_* result;

    scratch = inv_AI_parameterPoolCursor;
    type = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    field = ((tdstEngineObject_*) hPerso->hLinkedObject)->h_Dynam->p_stDynamics;
    result = p_stTree;
    switch (type) {
    case eProc_SetPlatformLink:
        result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        link = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        MEC_fn_vForcePlatformLink(field, hPerso, link);
        inv_AI_parameterPoolCursor = scratch;
        break;
    case eProc_FreePlatformLink:
        MEC_fn_vFreePlatformLink(field, hPerso);
        inv_AI_parameterPoolCursor = scratch;
        break;
    default:
        inv_AI_parameterPoolCursor = scratch;
        break;
    }
    return result;
}

void* fn_p_HierFatherLink(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* spo;
    DNM_stDynamics* field;
    u16 type;
    void* result;
    HIE_tdstSuperObject_* link;
    HIE_tdstSuperObject_* node;

    tdstList_* list;

    void* card;
    s32 cond;
    tdstEngineObject_* engine;
    tdstDynam_* dynam;

    scratch = inv_AI_parameterPoolCursor;
    type = AI_NODE_ID(p_stTree);
    field = NULL;
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &spo);

    engine = spo->hLinkedObject;
    dynam = engine->h_Dynam;
    if (dynam != NULL) {
        field = dynam->p_stDynamics;
    }

    result = p_stTree;

    switch (type) {
    case eProc_HierSetFather:
        result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        link = inv_fn_hGetOwnerSuperObject(scratch->u.pvValue);
        if (field != NULL) {
            card = field->m_tdstDynamicsBaseBlock.m_pCurrentIdCard;
            cond = 0;
            if (card != NULL) {
                cond = DNM_fn_eIdentityCardGetType(card) == 1;
            }
            if (cond != 0) {
                MEC_fn_vForcePlatformLink(field, spo, link);
                break;
            }
        }
        HIE_fn_vChangeFather(spo, link, 0);
        break;

    case eProc_HierFreeFather:
        if (field != NULL) {
            card = field->m_tdstDynamicsBaseBlock.m_pCurrentIdCard;
            cond = 0;
            if (card != NULL) {
                cond = DNM_fn_eIdentityCardGetType(card) == 1;
            }
            if (cond != 0) {
                MEC_fn_vFreePlatformLink(field, spo);
                break;
            }
        }
        fn_vReputCharacterSuperObjectAtTheWorld(spo);
        break;

    case eProc_HierListOfSon:
        result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        list = scratch->u.pvValue;
        list->ucNbElt = 0;
        if (list->ucMaxSize != 0) {
            for (node = spo->hFirstElementSta; node != NULL; node = node->hNextBrotherSta) {
                if (node->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
                    fn_vPutInList(list, node);
                    if (list->ucNbElt == list->ucMaxSize) {
                        break;
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return result;
}

void* fn_p_SetMechanicListParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                    tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    tdstList_* list;
    MTH3D_tdstVector_ stVector;
    u32 index;
    u8 count;

    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    list = scratch->u.pvValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    count = list->ucNbElt;
    for (index = 0; index < count; index++) {
        HIE_tdstSuperObject_* perso;
        DNM_stDynamics* dynamics;

        perso = fn_p_stGetPersoInList(list, (u8) index);
        dynamics = inv_HIE_fn_p_stGetEngineObject(perso)->h_Dynam->p_stDynamics;
        MTH3D_M_vAddXYZ_inv(&stVector, &dynamics->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed,
                            &scratch->u.stVector);
        dynamics->m_tdstDynamicsAdvancedBlock.m_stStreamSpeed = stVector;
    }
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stResetLastCollisionActor(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    DNM_stReport_* p_stDynamReport;
    u16 tag;

    tag = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    switch (tag) {
    case eProc_ResetLastCollisionActor:
        if (((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_StandardGame != 0) {
            fn_vSuperObjectSetLastCollidedCharacter(hNewSuperObjPerso, 0);
        }
        break;
    case eProc_ClearCollisionReport:
        p_stDynamReport = fn_pstGetDNMReport(hNewSuperObjPerso);
        if (p_stDynamReport != NULL) {
            p_stDynamReport->m_ulCurrSurfaceState = 0;
        }
        break;
    }
    return p_stTree;
}

void* fn_p_SetMechanic3Real(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* stream;
    HIE_tdstSuperObject_* node;
    DNM_stDynamics* pstDynamic;
    u16 op;
    f32 xX;
    f32 xY;
    f32 xZ;

    scratch = inv_AI_parameterPoolCursor;
    stream = p_stTree;
    op = AI_NODE_ID(stream);
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    pstDynamic = ((tdstEngineObject_*) node->hLinkedObject)->h_Dynam
                     ->p_stDynamics;

    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, scratch);
    xX = scratch->u.xValue;
    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, scratch);
    xY = scratch->u.xValue;
    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, scratch);
    xZ = scratch->u.xValue;

    if (op == eProc_MecSetScale) {
        goto case_11b;
    }
    if (op == eProc_MecSetSlideFactorXYZ) {
        goto case_11c;
    }
    inv_AI_parameterPoolCursor = scratch;
    goto done;

case_11b:
    pstDynamic->m_tdstDynamicsBaseBlock.m_stScale.xY = xY;
    pstDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 0x1000;
    pstDynamic->m_tdstDynamicsBaseBlock.m_stScale.xZ = xZ;
    pstDynamic->m_tdstDynamicsBaseBlock.m_stScale.xX = xX;
    goto join;

case_11c:
    pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorX = xX;
    pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorY = xY;
    pstDynamic->m_tdstDynamicsAdvancedBlock.m_xSlideFactorZ = xZ;
join:
    inv_AI_parameterPoolCursor = scratch;
done:
    return stream;
}

#ifdef PC_PORT
void* fn_p_SetMechanicClimbSpeedLimit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                      tdstNodeInterpret_* p_stTree)
#else
void fn_p_SetMechanicClimbSpeedLimit(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
#endif
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstGetSetParam_* p_stParam;
#ifdef PC_PORT
    void* next;
#endif

    p_stParam = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = p_stParam + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
#ifdef PC_PORT
    next = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
#else
    fn_p_stEvalTree(p_SuperObjPerso, p_stTree, p_stParam);
#endif
    a333 = p_stParam->u.xValue;
    inv_AI_parameterPoolCursor = p_stParam;
#ifdef PC_PORT
    return next;
#endif
}

void* fn_p_stShadow(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstGetSetParam_* param;
    void* obj;
    u16 code;
    f32 xFirstValue;
    f32 xSecondValue;

    param = inv_AI_parameterPoolCursor;
    code = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = param + 1;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    obj = &((tdstEngineObject_*) hNewSuperObjPerso->hLinkedObject)->h_3dData->shadow;

    switch (code) {
    case 0x136:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        inv_SHW_fn_vSetShadowOn(obj, param->u.lValue);
        break;
    case 0x137:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        xFirstValue = param->u.xValue;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        xSecondValue = param->u.xValue;
        inv_SHW_fn_vSetShadowScaleX(obj, xFirstValue);
        inv_SHW_fn_vSetShadowScaleY(obj, xSecondValue);
        break;
    case 0x138:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        inv_SHW_fn_vSetShadowScaleX(obj, param->u.xValue);
        break;
    case 0x139:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        inv_SHW_fn_vSetShadowScaleY(obj, param->u.xValue);
        break;
    case 0x13A:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
        inv_SHW_fn_vSetShadowIncluded(obj, param->u.lValue);
        break;
    }

    inv_AI_parameterPoolCursor = param;
    return p_stTree;
}

void* fn_p_stSetPrivilegeActivationZDx(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    HIE_tdstSuperObject_* node;
    void* obj;
    u16 cmd;
    u32 index;
    s32 raw;

    scratch = inv_AI_parameterPoolCursor;
    cmd = AI_NODE_ID(p_stTree);
    inv_AI_parameterPoolCursor = scratch + 1;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    obj = ((tdstEngineObject_*) node->hLinkedObject)->h_CollSet;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    raw = scratch->u.lValue;
    index = cmd - 0x13Bu;
    raw = raw - 1;
    switch (index) {
    case 0:
        CS_fn_vSetPrivilegedActivationZone(0, obj, (s16) raw, 1);
        break;
    case 1:
        CS_fn_vSetPrivilegedActivationZone(1, obj, (s16) raw, 1);
        break;
    case 3:
        CS_fn_vSetPrivilegedActivationZone(3, obj, (s16) raw, 1);
        break;
    case 2:
        CS_fn_vSetPrivilegedActivationZone(2, obj, (s16) raw, 1);
        break;
    case 6:
        CS_fn_vSetPrivilegedActivationZone(2, obj, (s16) raw, 2);
        break;
    case 4:
        CS_fn_vSetPrivilegedActivationZone(0, obj, (s16) raw, 2);
        break;
    case 5:
        CS_fn_vSetPrivilegedActivationZone(1, obj, (s16) raw, 2);
        break;
    case 7:
        CS_fn_vSetPrivilegedActivationZone(3, obj, (s16) raw, 2);
        break;
    case 8:
        CS_fn_vSetPrivilegedActivationZone(0, obj, (s16) raw, 0);
        break;
    case 11:
        CS_fn_vSetPrivilegedActivationZone(3, obj, (s16) raw, 0);
        break;
    case 9:
        CS_fn_vSetPrivilegedActivationZone(1, obj, (s16) raw, 0);
        break;
    case 10:
        CS_fn_vSetPrivilegedActivationZone(2, obj, (s16) raw, 0);
        break;
    }
    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stSetCollSetCollisionFlag(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* saved;
    CollisionSet* p_stCollSet;
    void* context;
    u16 op;
    u32 index;
    tdstGetSetParam_ scratch;
    u8 bValue;

    context = p_SuperObjPerso;
    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(context, &saved);
    p_stCollSet = ((tdstEngineObject_*) saved->hLinkedObject)->h_CollSet;
    p_stTree = fn_p_stEvalTree(context, p_stTree, &scratch);
    bValue = scratch.u.ucValue;
    index = (u32) op - 0x148;

    switch (index) {
    case 0:
        if (bValue != 0) {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag | 1);
        } else {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag & 0xFE);
        }
        break;
    case 2:
        if (bValue != 0) {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag | 4);
        } else {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag & 0xFB);
        }
        break;
    case 3:
        if (bValue != 0) {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag | 8);
        } else {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag & 0xF7);
        }
        break;
    case 4:
        if (bValue != 0) {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag | 0x20);
        } else {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag & 0xDF);
        }
        break;
    case 1:
        if (bValue != 0) {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag | 2);
        } else {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag & 0xFD);
        }
        break;
    case 5:
        if (bValue != 0) {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag | 0x40);
        } else {
            p_stCollSet->ucCollisionFlag = (u8) (p_stCollSet->ucCollisionFlag & 0xBF);
        }
        break;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stSetComputationFrequency(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree)
{
    s32 eProcedureId;
    tdstNodeInterpret_* pnode;
    tdstGetSetParam_ stParams[2];
    u8 ucFrequency;
    tdstEngineObject_* base;
    CollisionSet* dst;

    pnode = p_stTree;
    eProcedureId = AI_NODE_ID(pnode);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &stParams[1].u.hSuperObject);
    pnode = fn_p_stEvalTree(p_SuperObjPerso, pnode, stParams);
    ucFrequency = stParams[0].u.lValue;
    if (eProcedureId != eProc_SetBrainComputeFrequency) {
        if (eProcedureId < eProc_SetLightComputeFrequency) {
            if (eProcedureId == eProc_SetCollComputeFrequency) {
                goto do_14e;
            }
        } else if (eProcedureId == eProc_SetLightComputeFrequency) {
            goto done;
        }
        return pnode;
    }
    fn_v3dDataSetBrainComputationFrequency(
        ((tdstEngineObject_*) stParams[1].u.hSuperObject->hLinkedObject)->h_3dData,
        ucFrequency);
    return pnode;
do_14e:
    base = stParams[1].u.hSuperObject->hLinkedObject;
    dst = base->h_CollSet;
    dst->ucCollComputeFrequency = ucFrequency;
done:
    return pnode;
}

tdstNodeInterpret_* fn_p_stActivationZDR(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* hNewSuperObjPerso;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    if (stParam.u.lValue != 0) {
        hNewSuperObjPerso->flags &= ~1u;
    } else {
        hNewSuperObjPerso->flags |= 1u;
    }
    return p_stTree;
}

void* fn_p_LinkControl(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    HIE_tdstSuperObject_* hPerso;
    DNM_stDynamics* base;
    u32 flags;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hPerso);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    base = ((tdstEngineObject_*) hPerso->hLinkedObject)->h_Dynam->p_stDynamics;
    if (scratch.u.ucValue != 0) {
        flags = base->m_tdstDynamicsBaseBlock.ulFlags |= 0x40000000;
    } else {
        flags = base->m_tdstDynamicsBaseBlock.ulFlags &= 0xBFFFFFFF;
    }
    base->m_tdstDynamicsBaseBlock.ulFlags = flags;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stReadAnalogJoystickMario(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                   tdstNodeInterpret_* p_stTree)
{
    typedef struct RotTransform {
        f32 m[24];
    } RotTransform;

    tdstGetSetParam_ stParam;
    MTH3D_tdstVector_ stJoyVector;
    MTH3D_tdstVector_ stSightVector;
    MTH3D_tdstVector_ stI;
    MTH3D_tdstVector_ stJ;
    MTH3D_tdstVector_ stK;
    MTH3D_tdstVector_ stTmpV0;
    MTH3D_tdstVector_ stTmpV1;
    MTH3D_tdstVector_ stCamV0;
    MTH3D_tdstVector_ stCamV1;
    MTH3D_tdstVector_ stCamV2;
    MTH3D_tdstVector_ stVectorToTarget;
    MTH3D_tdstVector_ stWorldZ;
    MTH3D_tdstMatrix_ stMatrix;
    MTH3D_tdstVector_ VectTmp;
    RotTransform stTransform;
    HIE_tdstSuperObject_* hCamera;
    IPT_tdstEntryElement_* hJoystickAxeX;
    IPT_tdstEntryElement_* hJoystickAxeY;
    void* hMatrix;
    void* hDynam;
    IPT_tdstPadReadingOutput_* pJoy;
    u16 uwOperator;
    s16 ucPadNumber;
    s32 iXValue;
    s32 iYValue;
    s32 iRotationAsked;
    s32 iMode;
    s32 iDt;
    s32 iComputeSector;
    s32 iCurveType;
    s32 bVectorToTargetIsNull;
    f32 xDeadZone;
    f32 xMinimumAngle;
    f32 xRotationSpeed;
    f32 xInertia;
    f32 xSquare;
    f32 xPow;
    f32 xSign;
    f32 xScale;
    f32 xTrueAnalogForce;
    f32 xOldAnalogForce;
    f32 xAnalogForce;
    f32 xAngleRotation;
    f32 xFactorX;
    f32 xFactorZ;
    f32 xTmp;
    f32 xAbsAngle;

    uwOperator = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hCamera);
    xAngleRotation = 0.0f;

    if (uwOperator == 0x159) {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        {
            s16 sValue;
            if (stParam.ucType == E_vt_Float) {
                s32 lValue;
                lValue = inv_MTH_fn_lToIntegerOrdered(stParam.u.xValue);
                sValue = (s16) lValue;
            } else {
                sValue = stParam.u.lValue;
            }
            ucPadNumber = sValue;
        }
    } else {
        ucPadNumber = 0;
    }

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    xDeadZone = inv_AI_fn_xAiParamFloat(&stParam);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    if (stParam.ucType == E_vt_Float) {
        xMinimumAngle = stParam.u.xValue * MTH_DEG_TO_RAD_inv;
    } else {
        xMinimumAngle = (f32) stParam.u.lValue * MTH_DEG_TO_RAD_inv;
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    xRotationSpeed = inv_AI_fn_xAiParamFloat(&stParam);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    xInertia = inv_AI_fn_xAiParamFloat(&stParam);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    if (stParam.ucType == E_vt_Float) {
        iMode = inv_MTH_fn_lToIntegerOrdered(stParam.u.xValue);
    } else {
        iMode = stParam.u.lValue;
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    iRotationAsked = stParam.u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    iComputeSector = stParam.u.lValue;

    if (iMode != 0) {
        iDt = inv_MTH_fn_lToIntegerOrdered(inv_fn_xGetEngineClockDelta() * 1000.0f);
        iCurveType = fn_lGetTemporalCurveType();
        hJoystickAxeX = inv_IPT_fn_hGetJoystickAxeEntry(ucPadNumber, 0);
        hJoystickAxeY = inv_IPT_fn_hGetJoystickAxeEntry(ucPadNumber, 1);
        if (hJoystickAxeX != 0) {
            iXValue = inv_MTH_fn_lToIntegerOrdered(IPT_fn_xGetAnalogicValue(hJoystickAxeX));
        } else {
            iXValue = 0;
        }
        if (hJoystickAxeY != 0) {
            iYValue = inv_MTH_fn_lToIntegerOrdered(IPT_fn_xGetAnalogicValue(hJoystickAxeY));
        } else {
            iYValue = 0;
        }

        if (iMode == 2) {
            xRotationSpeed = 0.0f;
            iRotationAsked = 0;
        }

        stJoyVector.xX = (f32) -iXValue;
        if (iMode == 2) {
            stJoyVector.xY = 0.0f;
            stJoyVector.xZ = (f32) -iYValue;
        } else {
            stJoyVector.xY = (f32) iYValue;
            stJoyVector.xZ = 0.0f;
        }

        xOldAnalogForce = qword_383BD0[ucPadNumber].left.xAnalogForce;
        xSquare = MTH_fn_xSqrAddSqrAddSqrOpt2(stJoyVector.xX, stJoyVector.xY, stJoyVector.xZ);
        xTrueAnalogForce = MTH_fn_xSqrtOpt2(xSquare);
        if (xTrueAnalogForce <= xDeadZone) {
            xTrueAnalogForce = 0.0f;
            iRotationAsked = 0;
            stJoyVector.xX = stJoyVector.xY = stJoyVector.xZ = 0.0f;
        }

        if (xInertia > 0.0f) {
#ifdef PC_PORT
            if (inv_GAM_fn_xGetEngineClockTicks() != (f32) iCurveType) {
                xTmp = pc_curve_powf(xInertia, inv_GAM_fn_xGetEngineClockTicks(), iCurveType);
            } else
#endif
            switch (iCurveType) {
            case 1:
                xTmp = xInertia;
                break;
            case 2:
                xTmp = xInertia * xInertia;
                break;
            case 3:
                xTmp = xInertia * xInertia * xInertia;
                break;
            default:
                xTmp = xInertia * xInertia;
                xTmp = xTmp * xTmp;
                break;
            }
            xAnalogForce = (xOldAnalogForce - xTrueAnalogForce) * xTmp + xTrueAnalogForce;
            if (xDeadZone < xAnalogForce - xTrueAnalogForce) {
                iRotationAsked = 0;
                stJoyVector.xX = stJoyVector.xY = stJoyVector.xZ = 0.0f;
            }
        } else {
            xAnalogForce = xTrueAnalogForce;
        }

        if (xDeadZone <= xAnalogForce) {
            if (iMode == 2) {
                if (!MTH3D_M_bIsZeroXYZ_inv(&stJoyVector)) {
                    inv_MTH3D_fn_vNormalizeXYZ(&stJoyVector);
                    inv_MTH3D_fn_vKTimesXYZ(&stJoyVector, xAnalogForce, &stJoyVector);
                }

                POS_fn_vGetRotationVectors(p_SuperObjPerso->gu_hGlobalMatrix, &stI, &stJ, &stK);
                POS_fn_vGetRotationVectors(hCamera->gu_hGlobalMatrix, &stCamV0, &stCamV1, &stCamV2);

                xFactorX =
                    inv_MTH_fn_xMulAdd3(stI.xX, stCamV0.xX, stI.xY, stCamV0.xY, stI.xZ, stCamV0.xZ);
                xSign = inv_fn_xJoystickSign(xFactorX);
                xPow = powf(inv_MTH_fn_xAbsolute(xFactorX), 0.3f);
                xFactorZ =
                    inv_MTH_fn_xMulAdd3(stK.xX, stCamV2.xX, stK.xY, stCamV2.xY, stK.xZ, stCamV2.xZ);
                xFactorX = xSign * inv_MTH_fn_xMinimum(1.0f, xPow * 1.5f);
                xSign = inv_fn_xJoystickSign(xFactorZ);
                xAngleRotation = 0.0f;
                xPow = powf(inv_MTH_fn_xAbsolute(xFactorZ), 0.3f);
                xFactorZ = xSign * inv_MTH_fn_xMinimum(1.0f, xPow * 1.5f);

                xScale = xFactorX * stJoyVector.xX;
                inv_MTH3D_fn_vKTimesXYZ(&stTmpV0, xScale, &stI);
                xScale = xFactorZ * stJoyVector.xZ;
                inv_MTH3D_fn_vKTimesXYZ(&stTmpV1, xScale, &stK);
                inv_MTH3D_fn_vAddXYZ(&stJoyVector, &stTmpV0, &stTmpV1);
            } else {
                stWorldZ = qword_38FE78;
                stTmpV0 = *inv_POS_fn_p_stPosMatrixTranslation(hCamera->gu_hGlobalMatrix);
                stVectorToTarget = *inv_POS_fn_p_stPosMatrixTranslation(p_SuperObjPerso->gu_hGlobalMatrix);
                inv_MTH3D_fn_vSubXYZ(&stVectorToTarget, &stTmpV0, &stVectorToTarget);

                if (MTH3D_M_bIsZeroXYZ_inv(&stVectorToTarget)) {
                    bVectorToTargetIsNull = 1;
                } else {
                    bVectorToTargetIsNull = 0;
                    inv_MTH3D_fn_vNormalizeXYZ(&stVectorToTarget);
                }

                if (bVectorToTargetIsNull || iYValue < 0 ||
                    0.8f < __builtin_fabsf(stVectorToTarget.xZ)) {
                    POS_fn_vGetYAxisVector(hCamera->gu_hGlobalMatrix, &stVectorToTarget);
                    stVectorToTarget.xZ = 0.0f;
                    if (stVectorToTarget.xX == 0.0f && stVectorToTarget.xY == 0.0f) {
                        POS_fn_vGetRotationVector(hCamera->gu_hGlobalMatrix, &stVectorToTarget);
                        stVectorToTarget.xZ = 0.0f;
                    }
                    inv_MTH3D_fn_vNormalizeXYZ(&stVectorToTarget);
                    if (&stTmpV0 == &stVectorToTarget || &stTmpV0 == &stWorldZ) {
                        DNM_p_stVectorCrossProduct(&VectTmp, &stVectorToTarget, &stWorldZ);
                        stTmpV0 = VectTmp;
                    } else {
                        DNM_p_stVectorCrossProduct(&stTmpV0, &stVectorToTarget, &stWorldZ);
                    }
                    MTH3D_M_vCopyXYZ_inv(&stMatrix.stCol_0, &stTmpV0);
                    MTH3D_M_vCopyXYZ_inv(&stMatrix.stCol_1, &stVectorToTarget);
                    MTH3D_M_vCopyXYZ_inv(&stMatrix.stCol_2, &stWorldZ);
                    VectTmp = stJoyVector;
                    inv_fn_vJoystickTransformVector(&stJoyVector, &stMatrix, &VectTmp);
                } else {
                    stVectorToTarget.xZ = 0.0f;
                    if (MTH3D_M_bIsZeroXYZ_inv(&stVectorToTarget)) {
                        POS_fn_vGetRotationVector(hCamera->gu_hGlobalMatrix, &stVectorToTarget);
                        stVectorToTarget.xZ = 0.0f;
                        if (MTH3D_M_bIsZeroXYZ_inv(&stVectorToTarget)) {
                            goto store;
                        }
                    }
                    inv_MTH3D_fn_vNormalizeXYZ(&stVectorToTarget);
                    if (&stTmpV0 == &stVectorToTarget || &stTmpV0 == &stWorldZ) {
                        DNM_p_stVectorCrossProduct(&VectTmp, &stVectorToTarget, &stWorldZ);
                        stTmpV0 = VectTmp;
                    } else {
                        DNM_p_stVectorCrossProduct(&stTmpV0, &stVectorToTarget, &stWorldZ);
                    }
                    MTH3D_M_vCopyXYZ_inv(&stMatrix.stCol_0, &stTmpV0);
                    MTH3D_M_vCopyXYZ_inv(&stMatrix.stCol_1, &stVectorToTarget);
                    MTH3D_M_vCopyXYZ_inv(&stMatrix.stCol_2, &stWorldZ);
                    VectTmp = stJoyVector;
                    inv_fn_vJoystickTransformVector(&stJoyVector, &stMatrix, &VectTmp);
                }

                if (MTH3D_M_bIsZeroXYZ_inv(&stJoyVector)) {
                    goto store;
                }
                inv_MTH3D_fn_vNormalizeXYZ(&stJoyVector);

                POS_fn_vGetYAxisVector(p_SuperObjPerso->gu_hGlobalMatrix, &stJ);
                MTH3D_M_vSetXYZ_inv(&stSightVector, inv_MTH_fn_xNegateTied(stJ.xX),
                                    inv_MTH_fn_xNegateTied(stJ.xY), 0.0f);
                if (MTH3D_M_bIsZeroXYZ_inv(&stSightVector)) {
                    goto store;
                }
                inv_MTH3D_fn_vNormalizeXYZ(&stSightVector);

                xTmp = inv_MTH_fn_xMulAdd3(stSightVector.xX, stJoyVector.xX, stSightVector.xY,
                                stJoyVector.xY, stSightVector.xZ, stJoyVector.xZ);
                if (xTmp > 1.0f) {
                    xTmp = 1.0f;
                } else if (xTmp < -1.0f) {
                    xTmp = -1.0f;
                }
                xAngleRotation = acosf(xTmp);
                xTmp = stSightVector.xX * stJoyVector.xY - stSightVector.xY * stJoyVector.xX;
                if (xTmp < 0.0f) {
                    xAngleRotation = inv_MTH_fn_xNegateTied(xAngleRotation);
                }

                inv_MTH3D_fn_vKTimesXYZ(&stJoyVector, xAnalogForce, &stJoyVector);

                if (iRotationAsked != 0 && xRotationSpeed != 0.0f) {
                    f32 xSpeed;
                    f32 xDt;
                    f32 xAbsAngle;

                    xDt = (f32) iDt * 0.0001f;
                    xSpeed = xRotationSpeed * xDt;
                    xTmp = xAngleRotation * xAnalogForce * xSpeed;
                    xAbsAngle = __builtin_fabsf(xAngleRotation);
                    xOldAnalogForce = __builtin_fabsf(xTmp);
                    if (xAbsAngle < xOldAnalogForce) {
                        xOldAnalogForce = xAbsAngle;
                        xTmp = xAngleRotation;
                    }
                    if (xOldAnalogForce <= xMinimumAngle) {
                    } else if (xTrueAnalogForce != 0.0f && iRotationAsked == 1) {
                        hDynam = ((tdstEngineObject_*) p_SuperObjPerso->hLinkedObject)->h_Dynam;
                        hMatrix = p_SuperObjPerso->gu_hGlobalMatrix;
                        if (hMatrix != 0) {
                            POS_fn_vRotateZ((POS_tdstMatrix_*) &stTransform, xTmp);
                            POS_fn_vMulMatrixMatrix(hMatrix, (POS_tdstMatrix_*) &stTransform, hMatrix);
                            HIE_fn_vComputeNewRelativeMatrix(p_SuperObjPerso);
                            if (hDynam != 0) {
                                MEC_vImposeRotationMatrix(((tdstDynam_*) hDynam)->p_stDynamics, hMatrix);
                            }
                        }
                    }
                }
                xAngleRotation = xAngleRotation * MTH_RAD_TO_DEG_inv;
            }
            if (iComputeSector != 0) {
                if (iMode == 2) {
                    if (iYValue < iXValue) {
                        iComputeSector = (-iYValue < iXValue) ? 2 : 1;
                    } else {
                        iComputeSector = (-iYValue < iXValue) ? 3 : 4;
                    }
                } else {
                    xAbsAngle = inv_MTH_fn_xAbsolute(xAngleRotation);
                    if (xAbsAngle <= 45.0f) {
                        iComputeSector = 1;
                    } else if (xAbsAngle >= 135.0f) {
                        iComputeSector = 3;
                    } else if (xAngleRotation > 0.0f) {
                        iComputeSector = 4;
                    } else {
                        iComputeSector = 2;
                    }
                }
            }
        } else {
            iXValue = 0;
            iYValue = 0;
            iComputeSector = 0;
            stJoyVector.xX = stJoyVector.xY = stJoyVector.xZ = 0.0f;
            xAngleRotation = 0.0f;
        }
    } else {
        iXValue = 0;
        iYValue = 0;
        stJoyVector.xX = stJoyVector.xY = stJoyVector.xZ = 0.0f;
        xTrueAnalogForce = 0.0f;
        xAnalogForce = xTrueAnalogForce;
        xAngleRotation = xTrueAnalogForce;
        iComputeSector = 0;
    }

store:
    pJoy = &qword_383BD0[ucPadNumber].left;
    pJoy->wHorizontalAxis = (s16) iXValue;
    pJoy->wVerticalAxis = (s16) iYValue;
    pJoy->stGlobalVector = stJoyVector;
    pJoy->xAnalogForce = xAnalogForce;
    pJoy->xTrueAnalogForce = xTrueAnalogForce;
    pJoy->xRotationAngle = xAngleRotation;
    pJoy->lSector = iComputeSector;
    return p_stTree;
}

typedef struct JoystickRotation {
    f32 m[24];
} JoystickRotation;

extern void POS_fn_vGetRotationVector(void* matrix, MTH3D_tdstVector_* axis);
extern f32 powf(f32 base, f32 exponent);

#define JOYSTICK_CROSS(result, a, b, scratch)                                                      \
    {                                                                                              \
        if (&(result) == &(a) || &(result) == &(b)) {                                              \
            DNM_p_stVectorCrossProduct(&(scratch), &(a), &(b));                                    \
            (result) = (scratch);                                                                  \
        } else {                                                                                   \
            DNM_p_stVectorCrossProduct(&(result), &(a), &(b));                                     \
        }                                                                                          \
    }

static __inline__ void inv_fn_vJoystickSetBasis(MTH3D_tdstMatrix_* basis, MTH3D_tdstVector_* x,
                                                MTH3D_tdstVector_* y, MTH3D_tdstVector_* z)
{
    MTH3D_M_vCopyXYZ_inv(&basis->stCol_0, x);
    MTH3D_M_vCopyXYZ_inv(&basis->stCol_1, y);
    MTH3D_M_vCopyXYZ_inv(&basis->stCol_2, z);
}

void* func_001FBB20(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream)
{
    tdstGetSetParam_ param;
    MTH3D_tdstVector_ stick;
    MTH3D_tdstVector_ sight;
    MTH3D_tdstVector_ xAxis;
    MTH3D_tdstVector_ yAxis;
    MTH3D_tdstVector_ zAxis;
    MTH3D_tdstVector_ side;
    MTH3D_tdstVector_ scaledZ;
    MTH3D_tdstVector_ cameraX;
    MTH3D_tdstVector_ cameraY;
    MTH3D_tdstVector_ cameraZ;
    MTH3D_tdstVector_ toCamera;
    MTH3D_tdstVector_ worldUp;
    MTH3D_tdstMatrix_ basis;
    MTH3D_tdstVector_ copy;
    JoystickRotation rotation;
    HIE_tdstSuperObject_* camera;
    IPT_tdstEntryElement_* axisX;
    IPT_tdstEntryElement_* axisY;
    void* matrix;
    tdstDynam_* dynam;
    JoystickPads* record;
    u16 operatorId;
    s16 pad;
    s32 lStickX;
    s32 lStickY;
    s32 rotationAsked;
    s32 mode;
    s32 deltaMs;
    s32 sector;
    s32 curveType;
    s32 toCameraIsNull;
    f32 deadZone;
    f32 minimumAngle;
    f32 rotationSpeed;
    f32 inertia;
    f32 squareLength;
    f32 power;
    f32 sign;
    f32 scale;
    f32 trueForce;
    f32 previousForce;
    f32 force;
    f32 angle;
    f32 factorX;
    f32 factorZ;
    f32 value;
    f32 absAngle;

    operatorId = AI_NODE_ID(stream);
    deadZone = 0.0f;
    rotationSpeed = 0.0f;
    force = 0.0f;
    fn_vGetUltraOperatorPerso(superObject, &camera);
    angle = 0.0f;

    if (operatorId == 0x15D) {
        stream = fn_p_stEvalTree(superObject, stream, &param);
        {
            s16 padValue;
            if (param.ucType == E_vt_Float) {
                s32 truncated;
                truncated = inv_MTH_fn_lToIntegerOrdered(param.u.xValue);
                padValue = (s16) truncated;
            } else {
                padValue = (s16) param.u.lValue;
            }
            pad = padValue;
        }
    } else {
        pad = 0;
    }

    stream = fn_p_stEvalTree(superObject, stream, &param);
    deadZone = inv_AI_fn_xAiParamFloat(&param);
    stream = fn_p_stEvalTree(superObject, stream, &param);
    if (param.ucType == E_vt_Float) {
        minimumAngle = param.u.xValue * MTH_DEG_TO_RAD_inv;
    } else {
        minimumAngle = (f32) param.u.lValue * MTH_DEG_TO_RAD_inv;
    }
    stream = fn_p_stEvalTree(superObject, stream, &param);
    rotationSpeed = inv_AI_fn_xAiParamFloat(&param);
    stream = fn_p_stEvalTree(superObject, stream, &param);
    inertia = inv_AI_fn_xAiParamFloat(&param);
    stream = fn_p_stEvalTree(superObject, stream, &param);
    if (param.ucType == E_vt_Float) {
        mode = inv_MTH_fn_lToIntegerOrdered(param.u.xValue);
    } else {
        mode = param.u.lValue;
    }
    stream = fn_p_stEvalTree(superObject, stream, &param);
    rotationAsked = param.u.lValue;
    stream = fn_p_stEvalTree(superObject, stream, &param);
    sector = param.u.lValue;

    if (mode != 0) {
        deltaMs = inv_MTH_fn_lToIntegerOrdered(inv_fn_xGetEngineClockDelta() * 1000.0f);
        curveType = fn_lGetTemporalCurveType();
        axisX = inv_IPT_fn_hGetJoystickAxeEntry(pad, 2);
        axisY = inv_IPT_fn_hGetJoystickAxeEntry(pad, 3);
        if (axisX != NULL) {
            lStickX = inv_MTH_fn_lToIntegerOrdered(IPT_fn_xGetAnalogicValue(axisX));
        } else {
            lStickX = 0;
        }
        if (axisY != NULL) {
            lStickY = inv_MTH_fn_lToIntegerOrdered(IPT_fn_xGetAnalogicValue(axisY));
        } else {
            lStickY = 0;
        }

        D_00393DDC = force;
        D_00393DE0 = force;

        if (mode == 2) {
            rotationSpeed = 0.0f;
            rotationAsked = 0;
        }

        stick.xX = (f32) -lStickX;
        if (mode == 2) {
            stick.xY = 0.0f;
            stick.xZ = (f32) -lStickY;
        } else {
            stick.xY = (f32) lStickY;
            stick.xZ = 0.0f;
        }

        previousForce = qword_383BD0[pad].right.xAnalogForce;
        squareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(stick.xX, stick.xY, stick.xZ);
        trueForce = MTH_fn_xSqrtOpt2(squareLength);
        if (trueForce <= deadZone) {
            trueForce = 0.0f;
            rotationAsked = 0;
            stick.xX = stick.xY = stick.xZ = 0.0f;
        }

        if (inertia > 0.0f) {
#ifdef PC_PORT
            if (inv_GAM_fn_xGetEngineClockTicks() != (f32) curveType) {
                value = pc_curve_powf(inertia, inv_GAM_fn_xGetEngineClockTicks(), curveType);
            } else
#endif
            switch (curveType) {
            case 1:
                value = inertia;
                break;
            case 2:
                value = inertia * inertia;
                break;
            case 3:
                value = inertia * inertia * inertia;
                break;
            default:
                value = inertia * inertia;
                value = value * value;
                break;
            }
            force = (previousForce - trueForce) * value + trueForce;
            if (deadZone < force - trueForce) {
                rotationAsked = 0;
                stick.xX = stick.xY = stick.xZ = 0.0f;
            }
        } else {
            force = trueForce;
        }

        if (deadZone <= force) {
            if (mode == 2) {
                if (!MTH3D_M_bIsZeroXYZ_inv(&stick)) {
                    inv_MTH3D_fn_vNormalizeXYZ(&stick);
                    inv_MTH3D_fn_vKTimesXYZ(&stick, force, &stick);
                }

                POS_fn_vGetRotationVectors(superObject->gu_hGlobalMatrix, &xAxis, &yAxis, &zAxis);
                POS_fn_vGetRotationVectors(camera->gu_hGlobalMatrix, &cameraX, &cameraY, &cameraZ);

                factorX = inv_MTH_fn_xMulAdd3(xAxis.xX, cameraX.xX, xAxis.xY,
                                   cameraX.xY, xAxis.xZ, cameraX.xZ);
                sign = inv_fn_xJoystickSign(factorX);
                power = powf(inv_MTH_fn_xAbsolute(factorX), 0.3f);
                factorZ = inv_MTH_fn_xMulAdd3(zAxis.xX, cameraZ.xX, zAxis.xY,
                                   cameraZ.xY, zAxis.xZ, cameraZ.xZ);
                factorX = sign * inv_MTH_fn_xMinimum(1.0f, power * 1.5f);
                sign = inv_fn_xJoystickSign(factorZ);
                angle = 0.0f;
                power = powf(inv_MTH_fn_xAbsolute(factorZ), 0.3f);
                factorZ = sign * inv_MTH_fn_xMinimum(1.0f, power * 1.5f);

                scale = factorX * stick.xX;
                inv_MTH3D_fn_vKTimesXYZ(&side, scale, &xAxis);
                scale = factorZ * stick.xZ;
                inv_MTH3D_fn_vKTimesXYZ(&scaledZ, scale, &zAxis);
                inv_MTH3D_fn_vAddXYZ(&stick, &side, &scaledZ);
            } else {
                worldUp = qword_38FE78;
                side = *inv_POS_fn_p_stPosMatrixTranslation(camera->gu_hGlobalMatrix);
                toCamera = *inv_POS_fn_p_stPosMatrixTranslation(superObject->gu_hGlobalMatrix);
                inv_MTH3D_fn_vSubXYZ(&toCamera, &side, &toCamera);

                if (MTH3D_M_bIsZeroXYZ_inv(&toCamera)) {
                    toCameraIsNull = 1;
                } else {
                    toCameraIsNull = 0;
                    inv_MTH3D_fn_vNormalizeXYZ(&toCamera);
                }

                if (toCameraIsNull || lStickY < 0 || 0.8f < __builtin_fabsf(toCamera.xZ)) {
                    POS_fn_vGetYAxisVector(camera->gu_hGlobalMatrix, &toCamera);
                    toCamera.xZ = 0.0f;
                    if (toCamera.xX == 0.0f && toCamera.xY == 0.0f) {
                        POS_fn_vGetRotationVector(camera->gu_hGlobalMatrix, &toCamera);
                        toCamera.xZ = 0.0f;
                    }
                    inv_MTH3D_fn_vNormalizeXYZ(&toCamera);
                    JOYSTICK_CROSS(side, toCamera, worldUp, copy);
                    inv_fn_vJoystickSetBasis(&basis, &side, &toCamera, &worldUp);
                    copy = stick;
                    inv_fn_vJoystickTransformVector(&stick, &basis, &copy);
                } else {
                    toCamera.xZ = 0.0f;
                    if (MTH3D_M_bIsZeroXYZ_inv(&toCamera)) {
                        POS_fn_vGetRotationVector(camera->gu_hGlobalMatrix, &toCamera);
                        toCamera.xZ = 0.0f;
                        if (MTH3D_M_bIsZeroXYZ_inv(&toCamera)) {
                            goto store;
                        }
                    }
                    inv_MTH3D_fn_vNormalizeXYZ(&toCamera);
                    JOYSTICK_CROSS(side, toCamera, worldUp, copy);
                    inv_fn_vJoystickSetBasis(&basis, &side, &toCamera, &worldUp);
                    copy = stick;
                    inv_fn_vJoystickTransformVector(&stick, &basis, &copy);
                }

                if (MTH3D_M_bIsZeroXYZ_inv(&stick)) {
                    goto store;
                }
                inv_MTH3D_fn_vNormalizeXYZ(&stick);

                POS_fn_vGetYAxisVector(superObject->gu_hGlobalMatrix, &yAxis);
                MTH3D_M_vSetXYZ_inv(&sight, inv_MTH_fn_xNegateTied(yAxis.xX),
                                    inv_MTH_fn_xNegateTied(yAxis.xY), 0.0f);
                if (MTH3D_M_bIsZeroXYZ_inv(&sight)) {
                    goto store;
                }
                inv_MTH3D_fn_vNormalizeXYZ(&sight);

                value =
                    inv_MTH_fn_xMulAdd3(sight.xX, stick.xX, sight.xY, stick.xY, sight.xZ, stick.xZ);
                if (value > 1.0f) {
                    value = 1.0f;
                } else if (value < -1.0f) {
                    value = -1.0f;
                }
                angle = acosf(value);
                value = sight.xX * stick.xY - sight.xY * stick.xX;
                if (value < 0.0f) {
                    angle = inv_MTH_fn_xNegateTied(angle);
                }

                inv_MTH3D_fn_vKTimesXYZ(&stick, force, &stick);

                if (rotationAsked != 0 && rotationSpeed != 0.0f) {
                    f32 speed;
                    f32 dt;
                    f32 absAngle;

                    dt = (f32) deltaMs * 0.0001f;
                    speed = rotationSpeed * dt;
                    value = angle * force * speed;
                    absAngle = __builtin_fabsf(angle);
                    previousForce = __builtin_fabsf(value);
                    if (absAngle < previousForce) {
                        previousForce = absAngle;
                        value = angle;
                    }
                    if (!(previousForce <= minimumAngle) && trueForce != 0.0f &&
                        rotationAsked == 1) {
                        dynam = ((tdstEngineObject_*) superObject->hLinkedObject)->h_Dynam;
                        matrix = superObject->gu_hGlobalMatrix;
                        if (matrix != NULL) {
                            POS_fn_vRotateZ((POS_tdstMatrix_*) &rotation, value);
                            POS_fn_vMulMatrixMatrix(matrix, (POS_tdstMatrix_*) &rotation, matrix);
                            HIE_fn_vComputeNewRelativeMatrix(superObject);
                            if (dynam != NULL) {
                                MEC_vImposeRotationMatrix(dynam->p_stDynamics, matrix);
                            }
                        }
                    }
                }
                angle = angle * MTH_RAD_TO_DEG_inv;
            }
            if (sector != 0) {
                if (mode == 2) {
                    if (lStickY < lStickX) {
                        sector = (-lStickY < lStickX) ? 2 : 1;
                    } else {
                        sector = (-lStickY < lStickX) ? 3 : 4;
                    }
                } else {
                    absAngle = inv_MTH_fn_xAbsolute(angle);
                    if (absAngle <= 45.0f) {
                        sector = 1;
                    } else if (absAngle >= 135.0f) {
                        sector = 3;
                    } else if (angle > 0.0f) {
                        sector = 4;
                    } else {
                        sector = 2;
                    }
                }
            }
        } else {
            lStickX = 0;
            lStickY = 0;
            sector = 0;
            stick.xX = stick.xY = stick.xZ = 0.0f;
            angle = 0.0f;
        }
    } else {
        lStickX = 0;
        lStickY = 0;
        stick.xX = stick.xY = stick.xZ = 0.0f;
        trueForce = 0.0f;
        force = 0.0f;
        angle = 0.0f;
        sector = 0;
    }

store:
    record = &qword_383BD0[pad];
    record->right.wHorizontalAxis = (s16) lStickX;
    record->right.wVerticalAxis = (s16) lStickY;
    record->right.stGlobalVector = stick;
    record->right.xAnalogForce = force;
    record->right.xTrueAnalogForce = trueForce;
    record->right.xRotationAngle = angle;
    record->right.lSector = sector;
    return stream;
}

static const MTH3D_tdstVector_ qword_38FE78 = { 0.0f, 0.0f, 1.0f };

tdstNodeInterpret_* fn_p_stSetAnalogJoystickAxes(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    HIE_tdstSuperObject_* first_result;
    HIE_tdstSuperObject_* second_result;

    switch ((u16) p_stTree[-1].param - 0x155) {
    case 6:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    case 2:
        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = p_stTree + 1;
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree, &first_result);
        } else {
            first_result = p_SuperObjPerso;
        }
        p_stTree = p_stTree + 1;
        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = p_stTree + 1;
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree, &first_result);
        } else {
            first_result = p_SuperObjPerso;
        }
        p_stTree = p_stTree + 1;
        break;

    case 9:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = p_stTree + 1;
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree, &second_result);
        } else {
            second_result = p_SuperObjPerso;
        }
        p_stTree = p_stTree + 1;
        if (p_stTree->eType == E_ti_Operator) {
            p_stTree = p_stTree + 1;
            p_stTree = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stTree, &second_result);
        } else {
            second_result = p_SuperObjPerso;
        }
        p_stTree = p_stTree + 1;
        break;

    case 7:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        goto shared_three_calls;

    case 10:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    case 3:
    shared_three_calls:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        break;

    case 0:
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
        break;
    }

    return p_stTree;
}

void* func_001FCBE8(void* superObject, void* stream)
{
    tdstGetSetParam_ stParam;
    void* obj;
    void* pSecondValue;
    u16 code;

    code = AI_NODE_ID(stream);
    stream = fn_p_stEvalTree(superObject, stream, &stParam);
    obj = stParam.u.pvValue;
    stream = fn_p_stEvalTree(superObject, stream, &stParam);
    pSecondValue = stParam.u.pvValue;
    if (code == 0x160) {
        func_001203D0(obj, pSecondValue);
    }
    return stream;
}

void* fn_p_stSetBooleanInArray(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* p_stResultOwnerPerso;
    tdstNodeInterpret_* stream;
    s32 code;
    tdstBrain_* brain;
    u32 ulValue;
    u32 prev;
    u32 idx;
    s32 lArrayIndex;
    s32 bit;
    AI_tdstMind_* p_stMind;
    s32 handle;
    s32 flag;

    stream = p_stTree;
    code = AI_NODE_ID(stream);
    if (stream->eType == E_ti_Operator) {
        stream = fn_p_stOperatorGetPerso(p_SuperObjPerso, stream + 1, &p_stResultOwnerPerso);
    } else {
        p_stResultOwnerPerso = p_SuperObjPerso;
    }
    brain = ((tdstEngineObject_*) p_stResultOwnerPerso->hLinkedObject)->h_Brain;
    p_stMind = brain->p_stMind;
    handle = (u8) stream->param;
    stream = fn_p_stEvalTree(p_SuperObjPerso, stream + 1, &stParam);
    prev = stParam.u.lValue;
    stream = fn_p_stEvalTree(p_SuperObjPerso, stream, &stParam);
    ulValue = stParam.u.lValue;
    prev = prev - 1;
    idx = prev;
    flag = (ulValue != 0);
    if (code == eProc_SetBooleanInArray) {
        lArrayIndex = (idx >> 5) & 0xFF;
        fn_ucGetDsgVar(handle, lArrayIndex, p_stMind, &stParam);
        idx &= 31;
        ulValue = stParam.u.lValue;
        if (flag != 0) {
            ulValue = ulValue | (1 << idx);
        } else {
            ulValue = ulValue & ~(1 << idx);
        }
        stParam.u.lValue = ulValue;
        stParam.ucType = E_vt_Integer;
        fn_ucSetDsgVar(handle, lArrayIndex, p_stMind, &stParam);
    }
    return stream;
}

void* fn_p_stSetDefaultFormatCharacter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* p_stNext;
    f32 xCharWidth;
    f32 xLineHeight;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    xCharWidth = stParam.u.xValue;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &stParam);
    xLineHeight = stParam.u.xValue;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &stParam);
    TFMT_vSetDefaultValuesCharacter(xCharWidth, xLineHeight, stParam.u.xValue);
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stSelectResolution(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ buf;
    return fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &buf);
}

tdstNodeInterpret_* fn_p_stChangeBrightness(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ buf;
    return fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &buf);
}

tdstNodeInterpret_* fn_p_stSetVideoOptions(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ buf;
    return fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &buf);
}

void* fn_p_stSetInStereoMode(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    j_j_nullsub_103(stParam.u.ucValue);
    return p_stNext;
}

tdstNodeInterpret_* fn_p_stSetPrevMusicFadeOut(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    tdstNodeInterpret_* result;

    result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    SND_fn_vSetMusicFadeOut(stParam.u.xValue);
    return result;
}

void* fn_p_stSetaturationAndBackgroundDistance(void* superObject, void* stream)
{
    HIE_tdstSuperObject_* saved;
    tdstEngineObject_* context;
    void* object;
    tdstGetSetParam_ scratch;

    fn_vGetUltraOperatorPerso(superObject, &saved);
    context = saved->hLinkedObject;
    object = context->h_MSSound;
    if (object != 0) {
        stream = fn_p_stEvalTree(superObject, stream, &scratch);
        fn_vMSSoundSetSaturationDistance(object, (u32) scratch.u.lValue << 16);
        stream = fn_p_stEvalTree(superObject, stream, &scratch);
        fn_vMSSoundSetBackGroundDistance(object, (u32) scratch.u.lValue << 16);
    }
    return stream;
}

void* fn_p_stSaveCurrentSoundRequest(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* p_stNext;
    u32 type;
    u32 s32_Index;
    s32 pst_BlockEvent;

    p_stNext = p_stTree;
    type = AI_NODE_ID(p_stNext);
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    s32_Index = stParam.u.ulValue;
    if (type == eProc_SND_SaveSoundEventInSlotMemory)
        goto case_16f;
    if (type == eProc_SND_RestoreSoundEventInSlotMemory)
        goto case_170;
    return p_stNext;
case_16f:
    GAM_fn_vSaveBlockEventSlotMemory(SND_fn_pstGetLastEventForObjectTypeSound(g_lSoundObjectTypeMusic), s32_Index);
    return p_stNext;
case_170:
    pst_BlockEvent = GAM_p_stGetBlockEventSlotMemory(s32_Index);
    if (pst_BlockEvent != 0) {
        SND_fn_lSendRequestSound(-1, g_lSoundObjectTypeMusic, pst_BlockEvent, 0, 0);
    }
    return p_stNext;
}

void* func_001FD018(void* context, void* stream)
{
    (void) context;
    return stream;
}

tdstNodeInterpret_* fn_p_stTemporalChangeSighting(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stEvalParam;
    MTH3D_tdstVector_ stStart;
    MTH3D_tdstVector_ stDelta;
    MTH3D_tdstVector_ stTarget;
    MTH3D_tdstVector_ stAxisX;
    MTH3D_tdstVector_ stAxisY;
    MTH3D_tdstVector_ stAxisZ;
    tdstGetSetParam_ stYAxisParam;
    HIE_tdstSuperObject_* obj;
    tdstEngineObject_* hEngineObject;
    POS_tdstMatrix_* hMatrix;
    tdstDynam_* hDynam;
    u32 flags;
    f32 angle;
    f32 scale;
    f32 xDeltaX;
    f32 xDeltaY;
    f32 xPower;
    f32 xDirX;
    f32 xDirY;
    f32 xSquareLength;
    f32 xLength;
    f32 xInvLength;
    f32 xUnitX;
    f32 xUnitY;
    f32 xNegUnitX;
    f32 xNegUnitY;
    s32 lCurveType;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &obj);
    hEngineObject = obj->hLinkedObject;
    hMatrix = obj->gu_hGlobalMatrix;
    hDynam = hEngineObject->h_Dynam;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stEvalParam);
    *(MTH3D_tdstVector_*) &stStart = *(MTH3D_tdstVector_*) &stEvalParam;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stEvalParam);
    angle = stEvalParam.u.xValue;

    POS_fn_vGetYAxisVector(hMatrix, &stYAxisParam.u.stVector);
    MTH3D_M_vSetXYZ_inv(&stTarget, inv_MTH_fn_xNegate(stYAxisParam.u.stVector.xX),
                        inv_MTH_fn_xNegate(stYAxisParam.u.stVector.xY),
                        inv_MTH_fn_xNegate(stYAxisParam.u.stVector.xZ));
    stAxisZ.xY = stAxisZ.xX = 0.0f;
    stAxisZ.xZ = 1.0f;
    angle = 1.0f - angle;
    if (angle == 0.0f) {
        scale = angle;
    } else {
        lCurveType = fn_lGetTemporalCurveType();
#ifdef PC_PORT
        if (inv_GAM_fn_xGetEngineClockTicks() != (f32) lCurveType) {
            scale = pc_curve_powf(angle, inv_GAM_fn_xGetEngineClockTicks(), lCurveType);
        } else
#endif
        switch (lCurveType) {
        case 1:
            scale = angle;
            break;
        case 2:
            scale = angle * angle;
            break;
        case 3:
            xPower = angle * angle;
            scale = xPower * angle;
            break;
        case 4:
            angle = angle * angle;
            scale = angle * angle;
            break;
        case 5:
            scale = angle * angle;
            xPower = scale * scale;
            scale = xPower * angle;
            break;
        default:
            xPower = angle * angle;
            scale = xPower * angle;
            scale = scale * scale;
            break;
        }
    }

    xDeltaX = scale * (stTarget.xX - stStart.xX);
    xDeltaY = scale * (stTarget.xY - stStart.xY);
    xDirX = xDeltaX + stStart.xX;
    stDelta.xX = xDeltaX;
    xDirY = xDeltaY + stStart.xY;
    stDelta.xY = xDeltaY;
    stAxisY.xZ = 0.0f;
    xSquareLength = MTH_fn_xSqrAddSqrAddSqrOpt2(xDirX, xDirY, stAxisY.xZ);
    stDelta.xZ = scale * (stTarget.xZ - stStart.xZ);
    xLength = MTH_fn_xSqrtOpt2(xSquareLength);
    stAxisY.xX = xDirX;
    stAxisY.xY = xDirY;
    if (xLength != 0.0f) {
        xInvLength = 1.0f / xLength;
        MTH3D_M_vSetXYZ_inv(&stAxisY, xInvLength * xDirX, xInvLength * xDirY,
                            xInvLength * stAxisY.xZ);
    }

    xUnitX = stAxisY.xX;
    xUnitY = stAxisY.xY;
    xNegUnitX = inv_MTH_fn_xNegate(xUnitX);
    xNegUnitY = inv_MTH_fn_xNegate(xUnitY);
    stAxisX.xZ = 0.0f;
    stAxisY.xX = xNegUnitX;
    stAxisY.xY = xNegUnitY;
    stAxisX.xX = xNegUnitY;
    stAxisX.xY = xUnitX;

    _POS_fn_vSetRotationMatrix(hMatrix, &stAxisX, &stAxisY, &stAxisZ);

    obj->gu_lLastComputeFrame = HIE_gs_lCurrentFrame;
    HIE_fn_vComputeNewRelativeMatrix(obj);
    if (obj->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        flags = fn_ulStandardGameGetCustomBitsSO(obj);
        if ((flags & 2) == 0) {
            goto do_b088;
        }
        if (hDynam == 0) {
            goto do_af58;
        }
        if ((flags & 0x8000) == 0) {
            goto do_b088;
        }
        goto do_af58;
    }
do_af58:
    HIE_fn_vRefreshHierarchy((void*) obj);
    goto after;
do_b088:
    HIE_fn_vInvalidateHierarchyFrame((void*) obj);
after:
    if (hDynam != 0) {
        MEC_vImposeRotationMatrix(hDynam->p_stDynamics, hMatrix);
    }
    return p_stTree;
}

void* func_001FD2D0(void* context, void* stream)
{
    inv_IPT_fn_vInitInputFrameState();
    return stream;
}

#ifdef PC_PORT
void* fn_p_stTextSuperImposed(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* next = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    inv_AI_ucTextSuperImposed = stParam.u.ucValue;
    return next;
}
#else
void fn_p_stTextSuperImposed(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    inv_AI_ucTextSuperImposed = stParam.u.ucValue;
}
#endif

void* fn_p_stTextIntegerParameter(HIE_tdstSuperObject_* p_SuperObjPerso,
                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    s32 index;
    s32 value;
    void* result;

    result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    index = scratch.u.ulValue;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, &scratch);
    if (index >= 10) {
        index = 9;
    }
    value = scratch.u.ulValue;
    TEXT_gai_IntParameters[index] = value;
    return result;
}

void* fn_p_stTextRealParameter(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    s32 l_NumInt;
    void* result;

    result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    l_NumInt = scratch.u.ulValue;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, &scratch);
    if (l_NumInt >= 10) {
        l_NumInt = 9;
    }
    TEXT_gaf_FloatParameters[l_NumInt] = scratch.u.xValue;
    return result;
}

tdstNodeInterpret_* fn_p_stCode4FormateTexte(HIE_tdstSuperObject_* p_SuperObjPerso,
                                             tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    char buf[0x4F0];
    u16 kind;
    char* str1;
    char* str2;
    char* str3;

    kind = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    str1 = fn_szGetStringFromTextOrStringParam(&stParam);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    str2 = fn_szGetStringFromTextOrStringParam(&stParam);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    str3 = fn_szGetStringFromTextOrStringParam(&stParam);
    if (kind == 0x1B0) {
        strlen(str3);
        strcpy(buf, str1);
        Strcat(buf, str2);
        strcpy(str3, buf);
        strlen(str3);
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stCode4AirPointsProcedures(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstExtendDatas4Ray_* points;
    s32 eProcedureId;
    tdstGetSetParam_ stParam;
    f32 xSign;
    f32 xAirPoints;

    eProcedureId = AI_NODE_ID(p_stTree);
    xSign = 1.0f;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    xAirPoints = stParam.u.xValue;
    switch (eProcedureId) {
    case eProc_FixePointsDair:
    case eProc_InitPointsDair:
        stExtendDatas4Ray.xAirPoints4Ray = xAirPoints;
        return p_stTree;
    case eProc_FixePointsDairMax:
    case eProc_InitPointsDairMax:
        stExtendDatas4Ray.xAirPointsMax4Ray = xAirPoints;
        return p_stTree;
    case eProc_EnlevePointsDair:
        xSign = -1.0f;
    case eProc_AjoutePointsDair:
        points = &stExtendDatas4Ray;
        points->xAirPoints4Ray = points->xAirPoints4Ray + xSign * xAirPoints;
        if (points->xAirPointsMax4Ray < points->xAirPoints4Ray) {
            points->xAirPoints4Ray = points->xAirPointsMax4Ray;
        } else if (points->xAirPoints4Ray < 0.0f) {
            points->xAirPoints4Ray = 0.0f;
        }
        break;
    case eProc_EnlevePointsDairMax:
        xSign = -1.0f;
    case eProc_AjoutePointsDairMax:
        points = &stExtendDatas4Ray;
        points->xAirPointsMax4Ray = points->xAirPointsMax4Ray + xSign * xAirPoints;
        if (points->xAirPointsMax4Ray < 0.0f) {
            points->xAirPointsMax4Ray = 0.0f;
        }
        if (points->xAirPoints4Ray > points->xAirPointsMax4Ray) {
            points->xAirPoints4Ray = points->xAirPointsMax4Ray;
        }
        break;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stCode4MagicPointsProcedures(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
#define D_38419C dword_38419C[0]

    tdstGetSetParam_ stParam;

    s32 code;
    s32 iMagicPoints;

    static void* const jpt_1FD63C[] = {&&case0, &&case1, &&case0, &&case1,
                                       &&case2, &&case4, &&case3, &&case5};

    code = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    iMagicPoints = stParam.u.ucValue;
    if (!((u32) (code - eProc_FixePointsDeMagie) >= 8)) {
        goto* jpt_1FD63C[code - eProc_FixePointsDeMagie];
    case0:
        stExtendDatas4Ray.iMagicPoints4Ray = iMagicPoints;
        return p_stTree;
    case1:
        D_38419C = iMagicPoints;
        return p_stTree;
    case2: {
        s32 lNewValue;
        lNewValue = stExtendDatas4Ray.iMagicPoints4Ray + iMagicPoints;
        stExtendDatas4Ray.iMagicPoints4Ray = lNewValue;
        if (lNewValue > stExtendDatas4Ray.iMagicPointsMax4Ray)
            stExtendDatas4Ray.iMagicPoints4Ray = stExtendDatas4Ray.iMagicPointsMax4Ray;
        return p_stTree;
    }
    case3: {
        s32 v;
        v = stExtendDatas4Ray.iMagicPoints4Ray - iMagicPoints;
        stExtendDatas4Ray.iMagicPoints4Ray = v;
        if (v < 0)
            stExtendDatas4Ray.iMagicPoints4Ray = 0;
        return p_stTree;
    }
    case4:
        stExtendDatas4Ray.iMagicPointsMax4Ray = stExtendDatas4Ray.iMagicPointsMax4Ray + iMagicPoints;
        return p_stTree;
    case5: {
        s32 v;
        v = stExtendDatas4Ray.iMagicPointsMax4Ray - iMagicPoints;
        stExtendDatas4Ray.iMagicPointsMax4Ray = v;
        if (v < 0)
            stExtendDatas4Ray.iMagicPointsMax4Ray = 0;
        return p_stTree;
    }
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stSetLocalLight(HIE_tdstSuperObject_* p_SuperObjPerso,
                                         tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* obj;
    void* hMSLight;
    s32 first;
    s32 second;
    void* hSector;

    if (AI_NODE_ID(p_stTree) == eProc_FixeLumierePersoLocal) {
        p_stTree =
            fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        obj = inv_fn_hGetOwnerSuperObject(stParam.u.pvValue);
        p_stTree =
            fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        first = stParam.u.ucValue;
        p_stTree =
            fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        hMSLight = inv_HIE_fn_p_stGetEngineObject(obj)->h_MSLight;
        second = stParam.u.ucValue;
        if (hMSLight != 0) {
            if (first != 0) {
                SECT_fn_vIsolateDNMLightsListNode(fn_hMSLightGetLightNodeInSector(hMSLight));
                if (second != 0) {
                    obj->flags |= 0x20000;
                } else {
                    obj->flags &= ~0x20000;
                }
                fn_vMSLightSetLocalLight(hMSLight, 1);
                fn_vMSLightSetOnlyLocalLight(hMSLight, second);
                return p_stTree;
            } else {
                hSector = GAM_fn_hGetCurrentSector(obj);
                if (second != 0) {
                    obj->flags |= 0x20000;
                } else {
                    obj->flags &= ~0x20000;
                }
                if (hSector != 0) {
                    SECT_fn_vIsolateDNMLightsListNode(fn_hMSLightGetLightNodeInSector(hMSLight));
                    SECT_fn_vAddTailDNMLightsListNode(hSector, fn_hMSLightGetLightNodeInSector(hMSLight));
                }
                fn_vMSLightSetLocalLight(hMSLight, 0);
                fn_vMSLightSetOnlyLocalLight(hMSLight, second);
            }
        }
    }
    return p_stTree;
}

void* fn_p_stSetLight(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    GEO_tdstColor_ color;
    void* current;
    void* savedContext;
    void* light;
    u16 operation;
    f32 nearValue;
    f32 farValue;

    current = p_stTree;
    light = 0;
    savedContext = p_SuperObjPerso;
    operation = AI_NODE_ID(current);

    if (operation != 0x18A) {
        current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        light = stParam.u.pvValue;
    }

    switch (operation) {
    case eProc_FixeLumiereStaticOnOff:
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        GLI_xSetLightState(light, stParam.u.ulValue);
        break;
    case eProc_FixeLumiereStaticNearFar:
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        nearValue = stParam.u.xValue;
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        farValue = stParam.u.xValue;
        GLI_vSetLightNearFar(light, nearValue, farValue);
        break;
    case eProc_FixeLumiereStaticColor:
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        color.xR = stParam.u.xValue;
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        color.xG = stParam.u.xValue;
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        color.xB = stParam.u.xValue;
        current = fn_p_stEvalTree(savedContext, current, &stParam);
        color.xA = stParam.u.xValue;
        GLI_xSetLightColor(light, &color);
        break;
    case 0x18A:
        inv_VID_fn_vAddStaticLightsFromRoot(SECT_hFatherSector);
        break;
    }
    return current;
}

void* fn_p_stCapabilityProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* node;
    void* owner;
    s32 op;
    s32 value;

    owner = 0;
    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &node);
    if (node->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        owner = node->hLinkedObject;
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    value = stParam.u.lValue;
    if (op != eProc_AddCapabilities) {
        if (op < eProc_SubCapabilities) {
            if (op == eProc_SetCapabilities) {
                goto set;
            }
            return p_stTree;
        }
        if (op == eProc_SubCapabilities) {
            goto remove;
        }
        if (op == eProc_ChangeCapabilities) {
            goto mutate;
        }
        return p_stTree;
    set:
        fn_vSetCapabilities(owner, value);
        return p_stTree;
    }
    fn_vAddCapabilities(owner, value);
    return p_stTree;
remove:
    fn_vSubCapabilities(owner, value);
    return p_stTree;
mutate:
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    fn_vChangeCapabilities(owner, value, stParam.u.lValue);
    return p_stTree;
}

void* fn_p_stReinitGraphProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    WPG_fn_vReinitGraph(stParam.u.hGraph);
    return result;
}

void* fn_p_stSetScrollSpeed(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* hVisualMaterial;
    u16 op;
    f32 u;
    f32 v;
    s32 mode;

    op = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    hVisualMaterial = (void*) stParam.u.lValue;

    if (op == eProc_FixeVitesseTexture) {
        f32 scale;
        scale = 100.0f;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        u = stParam.u.xValue;
        u = u / scale;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        v = stParam.u.xValue;
        v = v / scale;
        _GLI_xSetMaterialTextureScrollingCoef(hVisualMaterial, 0, 1, u, v);
        return p_stTree;
    } else if (op == eProc_ScrollingOnOff) {
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        mode = *(u8*) &stParam.u.lValue;
        GLI_xSetMaterialTextureScrollingOnOff(hVisualMaterial, mode);
        return p_stTree;
    } else if (op == eProc_FixeDecalageTexture) {
        f32 scale;
        scale = 100.0f;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        u = stParam.u.xValue;
        u = u / scale;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        v = stParam.u.xValue;
        v = v / scale;
        GLI_xSetMaterialTextureScrollingCoef(hVisualMaterial, 0, 2, u, v);
        return p_stTree;
    } else if (op == eProc_ScrollingPause) {
        u8 flag;
        s32 one;
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        flag = stParam.u.lValue;
        one = 1;
        mode = 3;
        if (flag == 0)
            mode = one;
    } else {
        return p_stTree;
    }
    GLI_xSetMaterialTextureScrollingOnOff(hVisualMaterial, mode);
    return p_stTree;
}

void* fn_p_stManageFrame(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    u16 type;
    s16 code;
    HIE_tdstSuperObject_* hPerso;
    void* p_stNext;
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* hNewSuperObjPerso;

    type = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    switch (type) {
    case eProc_ChangeCurrFrame:
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
        hPerso = hNewSuperObjPerso;
        code = stParam.u.wValue;
        code = (code == (s16) 0x8000) ? (s16) -1 : code;
        p_stTree = p_stNext;
        PLA_fn_vSetCurrFrame(hPerso, code);
        return p_stTree;
    case eProc_ChangeRandomFrame:
        code = (s16) -0x8000;
        hPerso = hNewSuperObjPerso;
        PLA_fn_vSetCurrFrame(hPerso, code);
        return p_stTree;
    default:
        return p_stTree;
    }
    return p_stTree;
}

static inline void inv_AI_fn_vCopyPackedVector(MTH3D_tdstVector_* dst, MTH3D_tdstVector_* src)
{
    *dst = *src;
}

void* fn_p_stSPOSuperimpoed(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    u8 stPosition[16];
    f32 xPosition[4];
    HIE_tdstSuperObject_* p_stPerso;
    tdstGetSetParam_* p_stParam;
    s32* p_lValue;
    void* p_stNext;
    POS_tdstMatrix_* p_stMatrix;
    tdstDynam_* p_stDynam;
    POS_tdstMatrix_* p_stMatrix2;
    tdstDynam_* p_stDynam2;
    tdstEngineObject_* p_stObject;
    tdstEngineObject_* p_stMainObject;
    tdstSectInfo_* p_stSectInfo;
    tdstSectInfo_* p_stMainSectInfo;
    u32* tab;
    u16 op;
    s32 lScreenX;
    s32 lScreenY;
    HIE_tdstSuperObject_* p_stSector;
    f32 d1000;
    f32 d640;
    f32 xScreenX;
    f32 xScreenY;
    f32 xDepth;
    f32 xDistance;
    f32 xViewportValue;

    p_stNext = p_stTree;
    op = AI_NODE_ID(p_stNext);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &p_stPerso);
    p_stParam = (tdstGetSetParam_*) inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) ((s32) (p_stParam + 1));
    p_lValue = (s32*) p_stParam;
    switch (op) {
    case eProc_ReleaseSuperimposed:
        HIE_fn_SO_vClearSuperimposedFlag(p_stPerso);
        inv_AI_fn_vClearTextLabelsOfObject(p_stPerso);
        p_stObject = p_stPerso->hLinkedObject;
        p_stMainObject = ((HIE_tdstSuperObject_*) (*(u32*) &g_stEngineStructure.activeSuperObject))->hLinkedObject;
        p_stSectInfo = p_stObject->h_SectInfo;
        p_stMainSectInfo = p_stMainObject->h_SectInfo;
        p_stSector = p_stMainSectInfo->hCurrentSector;
        p_stSectInfo->hCurrentSector = p_stSector;
        fn_vInsertObjectInSectorList(p_stSector, p_stPerso);
        break;
    case 0x19C:
        p_stDynam = ((tdstEngineObject_*) p_stPerso->hLinkedObject)->h_Dynam;
        HIE_fn_SO_vSetSuperimposedFlag(p_stPerso);
        fn_vRemoveObjectInSectorList(p_stPerso);
        p_stMatrix = p_stPerso->gu_hGlobalMatrix;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        lScreenX = *p_lValue;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        lScreenY = *p_lValue;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        d1000 = 1000.0f;
        xScreenX = (f32) lScreenX / d1000;
        xScreenY = (f32) lScreenY / d1000;
        xDepth = (f32) *p_lValue / d1000;
        tab = ((u32*) &g_stEngineStructure.stFixViewportAttr);
        xViewportValue = (f32) tab[3];
        d640 = 640.0f;
        xDistance = xDepth * xViewportValue / d640;
        GLI_vGet3DVertexFromScreenPos((GLD_tdstViewportAttributes_*) tab, (MTH3D_tdstVector_*) stPosition, xScreenX,
                                      xScreenY, xDistance);
        if (p_stMatrix->ucType == MAT_eTransform_Identity)
            p_stMatrix->ucType = MAT_eTransform_Translation;
        MTH4D_fn_vCopy3DTo4DVector(&p_stMatrix->stMatrix.stCol_3.xX, (MTH3D_tdstVector_*) stPosition);
        if (p_stDynam != 0)
            MEC_vInitTranslation(p_stDynam->p_stDynamics, p_stPerso, (MTH3D_tdstVector_*) stPosition);
        HIE_fn_vComputeNewRelativeMatrix(p_stPerso);
        break;
    case eProc_SetSuperimposed2:
        p_stDynam2 = ((tdstEngineObject_*) p_stPerso->hLinkedObject)->h_Dynam;
        HIE_fn_SO_vSetSuperimposedFlag(p_stPerso);
        fn_vRemoveObjectInSectorList(p_stPerso);
        p_stMatrix2 = p_stPerso->gu_hGlobalMatrix;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        xPosition[0] = *(f32*) p_lValue;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        xPosition[1] = *(f32*) p_lValue;
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        xPosition[2] = *(f32*) p_lValue;
        if (p_stMatrix2->ucType == MAT_eTransform_Identity)
            p_stMatrix2->ucType = MAT_eTransform_Translation;
        inv_AI_fn_vCopyPackedVector((MTH3D_tdstVector_*) &p_stMatrix2->stMatrix.stCol_3.xX,
                         (MTH3D_tdstVector_*) xPosition);
        p_stMatrix2->stMatrix.stCol_3.xW = 1.0f;
        if (p_stDynam2 != 0)
            MEC_vInitTranslation(p_stDynam2->p_stDynamics, p_stPerso, (MTH3D_tdstVector_*) xPosition);
        HIE_fn_vComputeNewRelativeMatrix(p_stPerso);
        break;
    case eProc_SwitchSuperimposedTab:
        p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, p_stParam);
        HIE_fn_SO_SwitchSuperImposedTab((u32) *p_lValue);
        break;
    default:
        break;
    }
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) p_stParam;
    return p_stNext;
}

void* fn_p_stSetSPOCoordinates(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    u16 command;
    HIE_tdstSuperObject_* hSpo;
    POS_tdstMatrix_* object;

    p_stTree = p_stTree;
    command = AI_NODE_ID(p_stTree);
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    hSpo = scratch.u.pvValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    if (command != 0x19F) {
        return p_stTree;
    }
    if (hSpo != 0) {
        object = hSpo->gu_hLocalMatrix;
        if (object->ucType == MAT_eTransform_Identity) {
            object->ucType = MAT_eTransform_Translation;
        }
        MTH4D_fn_vCopy3DTo4DVector(&object->stMatrix.stCol_3.xX, &scratch.u.stVector);
    }
    return p_stTree;
}

void* fn_p_stMiscProcOnTypeOfWP(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* p_stNext;
    WP_tdstGraph_* hGraph;
    WP_tdstGraphNode_* node;
    WP_tdstGraphNode_* found;
    WP_tdstWayPoint_* waypoint;
    s32 mask;
    u8 mode;

    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    hGraph = stParam.u.pvValue;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &stParam);
    waypoint = stParam.u.pvValue;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &stParam);
    mask = stParam.u.lValue;
    p_stNext = fn_p_stEvalTree(p_SuperObjPerso, p_stNext, &stParam);
    mode = *(u8*) &stParam.u;

    node = hGraph->m_hListOfNode.hFirstElementDyn;
    found = 0;
    while (node != 0) {
        if (node->m_hWayPoint == waypoint) {
            found = node;
            break;
        }
        node = node->hNextBrotherDyn;
    }

    if (mode == 2) {
        node = hGraph->m_hListOfNode.hFirstElementDyn;
        while (node != 0) {
            node->m_lTypeOfWP &= ~mask;
            node = node->hNextBrotherDyn;
        }
    }

    if (mode == 3) {
        node = hGraph->m_hListOfNode.hFirstElementDyn;
        while (node != 0) {
            node->m_lTypeOfWP = 0;
            node = node->hNextBrotherDyn;
        }
    }

    if (found == 0 || mode >= 6) {
        return p_stNext;
    }
    switch (mode) {
    case 0:
    case 2:
        found->m_lTypeOfWP |= mask;
        break;
    case 1:
    case 3:
        found->m_lTypeOfWP = mask;
        break;
    case 4:
        found->m_lTypeOfWP &= ~mask;
        break;
    case 5:
        found->m_lTypeOfWP = 0;
        break;
    }
    return p_stNext;
}

void* fn_p_stTractionFactorProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstGetSetParam_ scratch;
    tdstEngineObject_* data = 0;
    u16 command;
    u8 ucTractionFactor;

    command = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    if (hNewSuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        data = hNewSuperObjPerso->hLinkedObject;
    }

    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    ucTractionFactor = scratch.u.ucValue;
    if (command == eProc_SetTractionFactor) {
        data->h_StandardGame->ucTractionFactor = ucTractionFactor;
        return p_stTree;
    }
    return p_stTree;
}

void* fn_p_stTurnToPosition(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    typedef struct Vec16_ {
        MTH3D_tdstVector_ v;
        f32 pad;
    } Vec16_;

    Vec16_ src;
    Vec16_ dir;
    Vec16_ ref;
    Vec16_ cross;
    POS_tdstMatrix_ inverse;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstGetSetParam_* param;
    u16 code;
    f32 limit;
    f32 absLimit;
    f32 len2;
    f32 len;
    f32 angle;

    code = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    param = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = param + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);
    src.v = param->u.stVector;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, param);

    if (param->ucType == E_vt_Float)
        limit = param->u.xValue;
    else
        limit = (f32) param->u.lValue;

    absLimit = inv_MTH_fn_xAbsolute(limit);
    limit = absLimit * MTH_DEG_TO_RAD_inv;
    POS_fn_vInvertMatrix(&inverse, hNewSuperObjPerso->gu_hGlobalMatrix);
    POS_fn_vMulMatrixVertex(&dir.v, &inverse, &src.v);

    if (!(MTH3D_M_bIsZeroXYZ_inv(&dir.v))) {
        MTH3D_M_vSetXYZ_inv(&ref.v, 0.0f, -1.0f, 0.0f);
        dir.v.xZ = 0.0f;
        len2 = MTH_fn_xSqrAddSqrAddSqrOpt2(dir.v.xX, dir.v.xY, ref.v.xX);
        angle =
            inv_MTH_fn_xMulAdd3Grouped(ref.v.xX, ref.v.xY, ref.v.xZ, dir.v.xX, dir.v.xY, ref.v.xX);
        len = MTH_fn_xSqrtOpt2(len2);
        angle = angle / len;
        if (angle < 1.0f && -1.0f < angle)
            angle = acosf(angle);
        else
            angle = 0.0f;

        if (code == eProc_TurnToPositionSpeed)
            limit = limit * inv_fn_xGetEngineClockDelta();
        angle = inv_MTH_fn_xMinimum(angle, limit);

        {
            f32 xRefY = ref.v.xY;
            f32 xDirY = dir.v.xY;
            f32 xDirX = dir.v.xX;
            f32 xRefX = ref.v.xX;

            cross.v.xZ = inv_MTH_fn_xMulSub(xRefX, xDirY, xRefY, xDirX);
        }
        cross.v.xX = inv_MTH_fn_xMulSub(ref.v.xY, dir.v.xZ, ref.v.xZ, dir.v.xY);
        cross.v.xY = inv_MTH_fn_xMulSub(ref.v.xZ, dir.v.xX, ref.v.xX, dir.v.xZ);
        if (cross.v.xZ < 0.0f)
            angle = -angle;

        if (fn_ucInternalTurn(hNewSuperObjPerso, angle) == 0) {
            ucProcedureActionReturn = 1;
        }
    }
    inv_AI_parameterPoolCursor = param;
    return p_stTree;
}

void* fn_p_stFreezeAnim(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    u16 op;

    op = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    switch (op) {
    case eProc_FreezeAnim:
        ANI_fn_vFreeze(hNewSuperObjPerso);
        return p_stTree;
    case eProc_UnFreezeAnim:
        ANI_fn_vUnFreeze(hNewSuperObjPerso);
        return p_stTree;
    default:
        return p_stTree;
    }
}

void* fn_p_stFixePositionFade(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    u8 vecsrc[16];
    MTH3D_tdstVector_ scaleVector;
    void* result;
    HIE_tdstSuperObject_* hSpo;
    POS_tdstMatrix_* matrix;
    s32 screenX;
    s32 screenY;
    f32 d100;
    f32 d1000;
    f32 d640;
    f32 scale;
    f32 norm;

    result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    hSpo = scratch.u.pvValue;
    HIE_fn_SO_vSetSuperimposedFlag(hSpo);
    fn_vRemoveObjectInSectorList(hSpo);
    matrix = hSpo->gu_hLocalMatrix;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, &scratch);
    screenX = scratch.u.lValue;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, &scratch);
    screenY = scratch.u.lValue;
    result = fn_p_stEvalTree(p_SuperObjPerso, result, &scratch);
    d100 = 100.0f;
    d1000 = 1000.0f;
    GLI_vGet3DVertexFromScreenPos(&g_stEngineStructure.stFixViewportAttr, (MTH3D_tdstVector_*) vecsrc, (f32) screenX / d100, (f32) screenY / d100,
                                  (f32) scratch.u.lValue / d1000);
    if (matrix->ucType == MAT_eTransform_Identity)
        matrix->ucType = MAT_eTransform_Translation;
    MTH4D_fn_vCopy3DTo4DVector(&matrix->stMatrix.stCol_3.xX, (MTH3D_tdstVector_*) vecsrc);
    scale = (f32) ((u32*) &g_stEngineStructure.stFixViewportAttr)[3];
    d640 = 640.0f;
    norm = scale / d640;
    MTH3D_M_vSetXYZ_inv(&scaleVector, norm, 1.0f, norm);
    inv_POS_fn_vScale(matrix, &scaleVector);
    return result;
}

s32 fni_CompareIntegerLess(s32* left, s32* right)
{
    s32 rightValue = *right;
    s32 leftValue = *left;

    return (rightValue < leftValue) ? -1 : (rightValue != leftValue);
}

s32 fni_CompareIntegerGreater(s32* left, s32* right)
{
    s32 rightValue = *right;
    s32 leftValue = *left;

    return (leftValue < rightValue) ? -1 : (rightValue != leftValue);
}

s32 fni_CompareFloatGreater(f32* left, f32* right)
{
    f32 rightValue = *right;
    f32 leftValue = *left;
    s32 result;

    if (leftValue < rightValue) {
        result = -1;
    } else {
        result = 1;
        if (rightValue == leftValue)
            result = 0;
    }
    return result;
}

s32 fni_CompareFloatLess(f32* _pv_Elem1, f32* _pv_Elem2)
{
    f32 rightValue = _pv_Elem2[0];
    f32 leftValue = _pv_Elem1[0];
    s32 result;

    if (rightValue < leftValue) {
        return -1;
    }
    result = 1;
    if (rightValue == leftValue) {
        result = 0;
    }
    return result;
}

tdstNodeInterpret_* fn_p_stSortArray(HIE_tdstSuperObject_* p_SuperObjPerso,
                                     tdstNodeInterpret_* p_stTree)
{
    s32 local[4];
    HIE_tdstSuperObject_* p_stPerso;
    tdstDsgMem_* p_stDsgMem;
    void* p_cBuffer;
    void* cmp;
    tdstNodeInterpret_* p_stNode;
    u8* entry;
    s32 lCount;
    s32 bDescending;

    p_stNode = p_stTree;
    if (p_stNode->eType == E_ti_Operator) {
        p_stNode = fn_p_stOperatorGetPerso(p_SuperObjPerso, p_stNode + 1, &p_stPerso);
    } else {
        p_stPerso = p_SuperObjPerso;
    }
    p_stDsgMem = (((tdstEngineObject_*) p_stPerso->hLinkedObject)->h_Brain->p_stMind)->p_stDsgMem;
    p_cBuffer = p_stDsgMem->p_cDsgMemBuffer;
    entry = (u8*) p_cBuffer + (*p_stDsgMem->p_p_stDsgVar)->variables[(u8) p_stNode->param].bufferOffset;
    p_stNode = fn_p_stEvalTree(p_SuperObjPerso, p_stNode + 1, (tdstGetSetParam_*) local);
    lCount = local[0];
    p_stNode = fn_p_stEvalTree(p_SuperObjPerso, p_stNode, (tdstGetSetParam_*) local);
    bDescending = local[0];
    if (entry[5] < lCount) {
        lCount = entry[5];
    }
    switch (entry[4]) {
    case 5:
        cmp = (bDescending != 0) ? fni_CompareIntegerGreater : fni_CompareIntegerLess;
        break;
    case 7:
        cmp = (bDescending != 0) ? fni_CompareFloatGreater : fni_CompareFloatLess;
        break;
    default:
        return p_stNode;
    }
    qsort(entry + 8, lCount, 4, cmp);
    return p_stNode;
}

tdstNodeInterpret_* fn_p_stActiveJoystickAnal(HIE_tdstSuperObject_* unused,
                                              tdstNodeInterpret_* p_stTree)
{
    switch (AI_NODE_ID(p_stTree)) {
    case eProc_ActiveJoystickAnal:
        IPT_fn_vDesactiveJoystickAndPadControl();
        IPT_fn_vActiveJoystickControl();
        break;

    case eProc_UnActiveJoystickAnal:
        IPT_fn_vDesactiveJoystickAndPadControl();
        break;
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stSetZDxSize(HIE_tdstSuperObject_* p_SuperObjPerso,
                                      tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParams[2];
    s32 code;
    tdstNodeInterpret_* p;
    COL_tdstCollideObject_* p_stZdx;
    s32 v;
    s32 flag;
    HIE_tdstSuperObject_* w;

    code = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &stParams[1].u.hSuperObject);
    p = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, stParams);
    v = stParams[0].u.ulValue;
    switch (code) {
    case eProc_SetZDMSizeSphere:
        v = stParams[0].u.ulValue;
        v--;
        v = (u16) v;
        w = stParams[1].u.hSuperObject;
        flag = 1;
        break;
    case eProc_SetZDDSizeCone:
        v = stParams[0].u.ulValue;
        v--;
        v = (u16) v;
        w = stParams[1].u.hSuperObject;
        flag = 0;
        break;
    default:
        return p;
    }
    p_stZdx = fn_hGetGeometricZdxOfTypeAtIndex(flag, v, w);
    if (p_stZdx == 0)
        return p;
    if (p_stZdx) {
        switch (code) {
        case eProc_SetZDDSizeCone:
            return p;
        case eProc_SetZDMSizeSphere: {
            s16 tag;
            tag = *p_stZdx->d_xListOfElementsTypes;
            if (tag != 7)
                return p;
            p = fn_p_stEvalTree(p_SuperObjPerso, p, stParams);
            GEO_vSetRadiusOfIndexedSphere(p_stZdx, 0, 0, stParams[0].u.xValue);
        } break;
        }
    }
    return p;
}

void* fn_p_stSetSoundVolume(void* superObject, void* stream)
{
    s32 op;
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* first;
    void* p_stNext;

    op = AI_NODE_ID(stream);

    if (op == 0x1AA) {
        goto case_aa;
    }
    if (op < 0x1AB) {
        if (op == 0x1A9) {
            goto case_a9;
        }
        return stream;
    }
    if (op == 0x1AB) {
        goto case_ab;
    }
    return stream;

case_a9:
    fn_vGetUltraOperatorPerso(superObject, &first);
    p_stNext = fn_p_stEvalTree(superObject, stream, &stParam);
    SND_fn_vChangeVolumeSound((s32) first, g_lSoundObjectTypeAnim, stParam.u.lValue);
    return p_stNext;
case_aa:
    p_stNext = fn_p_stEvalTree(superObject, stream, &stParam);
    SND_fn_vChangeVolumeSound(-1, g_lSoundObjectTypeMusic, stParam.u.lValue);
    return p_stNext;
case_ab:
    p_stNext = fn_p_stEvalTree(superObject, stream, &stParam);
    SND_fn_vChangeVolumeSound(-1, g_lSoundObjectTypeAnim, stParam.u.lValue);
    return p_stNext;
}

void* fn_p_stSetDopplerEffect(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    SND_fn_vSetDopplerFactorSound((s32) (stParam.u.xValue * 65536.0f));
    return result;
}

void* fn_p_stPauseSound(void* context, void* stream)
{
    u16 tag;

    tag = AI_NODE_ID(stream);
    switch (tag) {
    case 0x1AD:
        SND_fn_vPauseSound();
        break;
    case 0x1AE:
        SND_fn_vResumeSound();
        break;
    case 0x1AF:
        SND_fn_vKillAllObjectTypeSound(g_lSoundObjectTypeMenu);
        break;
    }
    return stream;
}

tdstNodeInterpret_* fn_p_stPlayDemo(HIE_tdstSuperObject_* p_SuperObjPerso,
                                    tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    char szLevelName[0x30];
    tdstNodeInterpret_* node;
    char* s;
    char* p;

    node = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    s = fn_szGetStringFromTextOrStringParam(&stParam);
    fn_lGetSubMapNumber();
    if (*s == '*') {
        strcpy(szLevelName, fn_p_szGetLevelName());
        p = inv_My_strchr(s, '$');
        if (p != 0 && p[1] != '*') {
            atol(p + 1);
        }
    } else {
        strcpy(szLevelName, s);
        p = inv_My_strchr(szLevelName, '$');
        if (p != 0) {
            *p = '\0';
            if (p[1] != '*') {
                atol(p + 1);
            }
        }
    }
    node = fn_p_stEvalTree(p_SuperObjPerso, node, &stParam);
    fn_szGetStringFromTextOrStringParam(&stParam);
    node = fn_p_stEvalTree(p_SuperObjPerso, node, &stParam);
    node = fn_p_stEvalTree(p_SuperObjPerso, node, &stParam);
    fn_szGetStringFromTextOrStringParam(&stParam);
    return node;
}

void* fn_p_stSetTooFarLimit(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstEngineObject_* node;
    gu_tdstStandardGame* next;
    s32 lValue;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    node = 0;
    if (hNewSuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        node = hNewSuperObjPerso->hLinkedObject;
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    lValue = scratch.u.lValue;
    if (node != 0) {
        next = node->h_StandardGame;
        if (next != 0) {
            next->ucTooFarLimit = (u8) lValue;
        }
    }
    return p_stTree;
}

void* fn_p_stSetTransparencyZone(HIE_tdstSuperObject_* p_SuperObjPerso,
                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ scratch;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstEngineObject_* node;
    gu_tdstStandardGame* next;
    s32 first;
    s32 second;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    node = 0;
    if (hNewSuperObjPerso->lTypeOfLinkedObject == HIE_TYPE_ACTOR_inv) {
        node = hNewSuperObjPerso->hLinkedObject;
    }
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    first = scratch.u.lValue;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &scratch);
    second = scratch.u.lValue;
    if (node != 0) {
        next = node->h_StandardGame;
        if (next != 0) {
            next->ucTransparencyZoneMin = (u8) first;
            next = node->h_StandardGame;
            next->ucTransparencyZoneMax = (u8) second;
        }
    }
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stBaseFrequenceForMenu(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    tdstNodeInterpret_* result;

    result = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    GLI_vSetSinEffetBase(stParam.u.xValue);
    return result;
}

tdstNodeInterpret_* fn_p_stSaveSinusEffectContext(HIE_tdstSuperObject_* superObject,
                                                  tdstNodeInterpret_* p_stTree)
{
    if (AI_NODE_ID(p_stTree) == eProc_SaveSinusContext)
        GLI_vSaveSinusContext();
    else
        GLI_vRestoreSinusContext();
    return p_stTree;
}

void* func_001FEFC0(HIE_tdstSuperObject_* superObject, tdstNodeInterpret_* stream)
{
    tdstGetSetParam_ param;
    HIE_tdstSuperObject_* character;
    s32 alwaysActive;
    tdstEngineObject_* object;
    s32 expected;

    fn_vGetUltraOperatorPerso(superObject, &character);
    stream = fn_p_stEvalTree(superObject, stream, &param);
    alwaysActive = param.u.aucValue[0];
    if (fn_bGetIfCharacterIsInAlwaysActiveCharacterList(character) != alwaysActive) {
        object = character->hLinkedObject;
        if (alwaysActive) {
            fn_vAddCharacterInAlwaysActiveCharacterList(character);
            expected = 1;
            if (((object->h_StandardGame->ucMiscFlags >> 2) & 1) != expected) {
                func_00101438(character);
            }
        } else {
            inv_ALWAYS_fn_vRemoveCharacterFromAlwaysActiveCharacterList(character);
            object->h_StandardGame->ucMiscFlags &= ~4;
        }
    }
    return stream;
}

void* func_001FF080(HIE_tdstSuperObject_* ctx, void* stream)
{
    u8 mtx[0xB0];
    tdstGetSetParam_* rec;
    tdstGetSetParam_* p_stDirectionParam;
    tdstGetSetParam_* p_stValueParam;
    HIE_tdstSuperObject_* hCurrentSector;
    GliMaterialData* v;
    f32 x, y, z, xAlpha;

    rec = inv_AI_parameterPoolCursor;
    p_stValueParam = rec + 2;
    p_stDirectionParam = rec + 1;
    hCurrentSector = inv_HIE_fn_p_stGetEngineObject(ctx)->h_SectInfo->hCurrentSector;
    inv_AI_parameterPoolCursor = rec + 3;
    stream = fn_p_stEvalTree(ctx, stream, p_stValueParam);
    v = p_stValueParam->u.pvValue;
    stream = fn_p_stEvalTree(ctx, stream, rec);
    stream = fn_p_stEvalTree(ctx, stream, p_stDirectionParam);
    stream = fn_p_stEvalTree(ctx, stream, p_stValueParam);
    x = p_stValueParam->u.xValue;
    stream = fn_p_stEvalTree(ctx, stream, p_stValueParam);
    y = p_stValueParam->u.xValue;
    stream = fn_p_stEvalTree(ctx, stream, p_stValueParam);
    z = p_stValueParam->u.xValue;
    stream = fn_p_stEvalTree(ctx, stream, p_stValueParam);
    xAlpha = p_stValueParam->u.xValue;
    inv_SHW_fn_vBuildProjectionMatrix((struct ShadowProjectionMatrixView*) mtx, rec, &p_stDirectionParam->u.stVector, x,
                                      y, z);
    inv_SHW_fn_bQueueProjectedShadow(hCurrentSector, ctx, mtx, v, xAlpha / 255.0f, x, x, 0);
    inv_AI_parameterPoolCursor = (tdstGetSetParam_*) rec;
    return stream;
}

void* fn_p_stTurnPersoProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    MTH3D_tdstVector_ to;
    MTH3D_tdstVector_ from;
    HIE_tdstSuperObject_* spo;
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    u16 op;
    tdstEngineObject_* other;
    tdstEngineObject_* engine;
    gu_tdstStandardGame* other_engine;
    tdstDynam_* dynam;
    HIE_tdstSuperObject_* other_obj;
    void* slot;
    f32 x;
    f32 y;
    f32 z;

    context = p_SuperObjPerso;
    current = p_stTree;
    op = AI_NODE_ID(current);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &spo);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    other = (tdstEngineObject_*) scratch->u.lValue;
    engine = spo->hLinkedObject;
    other_engine = other->h_StandardGame;
    dynam = engine->h_Dynam;
    other_obj = other_engine->h_SuperObject;
    if (dynam != 0) {
        slot = dynam->p_stDynamics;
        from = *(MTH3D_tdstVector_*) &spo->gu_hGlobalMatrix->stMatrix.stCol_3.xX;
        if (op != eProc_DeltaTurnPerso) {
            if (op == eProc_TurnPerso) {
                goto op_82;
            }
            goto join;
        }
        current = fn_p_stEvalTree(context, current, scratch);
        if (scratch->ucType == E_vt_Float) {
            x = scratch->u.xValue;
        } else {
            x = (f32) scratch->u.lValue;
        }
        current = fn_p_stEvalTree(context, current, scratch);
        if (scratch->ucType == E_vt_Float) {
            y = scratch->u.xValue;
        } else {
            y = (f32) scratch->u.lValue;
        }
        current = fn_p_stEvalTree(context, current, scratch);
        if (scratch->ucType == E_vt_Float) {
            z = scratch->u.xValue;
        } else {
            z = (f32) scratch->u.lValue;
        }
        MTH3D_M_vSetXYZ_inv(&to, x, y, z);
        POS_fn_vMulMatrixVertex(&to, other_obj->gu_hGlobalMatrix, &to);
        goto join;
    op_82:
        to = *(MTH3D_tdstVector_*) &other_obj->gu_hGlobalMatrix->stMatrix.stCol_3.xX;
    join:
        MTH3D_M_vSubXYZ_inv(&to, &to, &from);
        MEC_vTurnAbsolute(slot, &to);
    }
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

void* fn_p_stChangeActionProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                   tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    void* current;
    void* context;
    u16 op;
    tdstState_* action;
    s32 extra;
    s32 ok;
    tdstEngineObject_* engine;
    tdstDynam_* dynam;
    DNM_stDynamics* mechanic;
    void* card;

    context = p_SuperObjPerso;
    current = p_stTree;
    op = AI_NODE_ID(current);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    action = (void*) stParam.u.lValue;
    if (op == eProc_ChangeActionWithEventsSmoothNL) {
        current = fn_p_stEvalTree(context, current, &stParam);
        extra = *(u8*) &stParam;
    } else {
        extra = 0;
    }
    ok = PLA_fn_bSetNewState(hNewSuperObjPerso, action, op == 0x3F, extra, 0);
    if (ok) {
        engine = hNewSuperObjPerso->hLinkedObject;
        dynam = engine->h_Dynam;
        if (dynam == 0) {
            current = current;
        } else {
            mechanic = dynam->p_stDynamics;
            card = action->h_LinkedMechanicsIdCard;
            if (mechanic->m_tdstDynamicsBaseBlock.m_pCurrentIdCard == card) {
                current = current;
            } else {
                DNM_fn_vChangeIdCard(mechanic, card);
                current = current;
            }
        }
    } else {
        ucProcedureActionReturn = 2;
        current = current;
    }
    return current;
}

void* fn_p_stChangeActionRandomProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                         tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    void* context;
    void* current;
    tdstState_* action;
    s32 ok;
    tdstEngineObject_* engine;
    tdstDynam_* dynam;
    DNM_stDynamics* mechanic;
    void* card;

    context = p_SuperObjPerso;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    action = (void*) stParam.u.lValue;
    ok = PLA_fn_bSetNewStateWithRandom(context, action);
    if (ok) {
        engine = (tdstEngineObject_*) ((HIE_tdstSuperObject_*) context)->hLinkedObject;
        dynam = engine->h_Dynam;
        if (dynam == 0) {
            current = current;
        } else {
            mechanic = dynam->p_stDynamics;
            card = action->h_LinkedMechanicsIdCard;
            if (mechanic->m_tdstDynamicsBaseBlock.m_pCurrentIdCard == card) {
                current = current;
            } else {
                DNM_fn_vChangeIdCard(mechanic, card);
                current = current;
            }
        }
    } else {
        ucProcedureActionReturn = 2;
        current = current;
    }
    return current;
}

void* fn_p_stDisplayProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstEngineObject_* engine;
    gu_tdst3dData_* data3d;
    s32 flags;
    void* current;

    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    current = p_stTree;
    engine = hNewSuperObjPerso->hLinkedObject;
    flags = hNewSuperObjPerso->gu_lDrawModeMask;
    data3d = engine->h_3dData;
    flags = flags | 8;
    data3d->lDrawMask = flags;
    hNewSuperObjPerso->gu_lDrawModeMask = flags;
    return current;
}

tdstNodeInterpret_* fn_p_stTransparenceProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree)
{
    HIE_tdstSuperObject_* spo;
    tdstGetSetParam_* scratch;
    u16 uwChannelNumber;
    s32 has_mod;
    HIE_tdstSuperObject_* hModule;
    HIE_tdstSuperObject_* hNewSuperObjPerso;
    tdstAChannel_* table;
    s32 lValue;
    s32 flag;
    s32 flags;
    f32 xValue;

    has_mod = 0;
    hModule = 0;
    uwChannelNumber = AI_NODE_ID(p_stTree);
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &spo);
    scratch = inv_AI_parameterPoolCursor;
    inv_AI_parameterPoolCursor = scratch + 1;
    p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);

    switch (uwChannelNumber) {
    case eProc_SetModuleTransparency:
    case eProc_SetModuleTransparency2:
        if (uwChannelNumber == eProc_SetModuleTransparency) {
            lValue = *(u16*) scratch;
        } else {
            lValue = *(u16*) scratch;
        }
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        table = ((tdstEngineObject_*) spo->hLinkedObject)->h_3dData->hArrayOfChannels;
        if (table == 0) {
            break;
        }
        spo = table[lValue].hSupObject;
        if (spo == 0) {
            break;
        }
        spo->flags |= 0x10000;
        if (scratch->ucType == E_vt_Float) {
            xValue = scratch->u.xValue;
        } else {
            xValue = (f32) scratch->u.lValue;
        }
        if (xValue < 0.0f) {
            xValue = 0.0f;
        } else if (xValue > 255.0f) {
            xValue = 255.0f;
        }
        spo->gu_fTransparenceLevel = xValue;
        inv_AI_fn_vSetTextLabelScale(spo, xValue);
        if (spo->lTypeOfLinkedObject == HIE_TYPE_EMPTY_inv) {
            for (hNewSuperObjPerso = spo->hFirstElementSta; hNewSuperObjPerso != 0; hNewSuperObjPerso = hNewSuperObjPerso->hNextBrotherSta) {
                hNewSuperObjPerso->gu_fTransparenceLevel = xValue;
                hNewSuperObjPerso->flags |= 0x10000;
                inv_AI_fn_vSetTextLabelScale(hNewSuperObjPerso, xValue);
            }
        }
        break;

    case eProc_SetTransparency:
        if (scratch->ucType == E_vt_Float) {
            xValue = scratch->u.xValue;
        } else {
            xValue = (f32) scratch->u.lValue;
        }
        if (xValue < 0.0f) {
            xValue = 0.0f;
        } else if (xValue > 255.0f) {
            xValue = 255.0f;
        }
        spo->gu_fTransparenceLevel = xValue;
        inv_AI_fn_vSetTextLabelScale(spo, xValue);
        PLA_fn_vUpdateTransparencyForModules(spo);
        break;

    case eProc_ModuleTransparentDisplay:
    case eProc_ModuleTransparentDisplay2:
        if (uwChannelNumber == eProc_ModuleTransparentDisplay) {
            lValue = *(u16*) scratch;
        } else {
            lValue = *(u16*) scratch;
        }
        p_stTree = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
        table = ((tdstEngineObject_*) spo->hLinkedObject)->h_3dData->hArrayOfChannels;
        if (table == 0) {
            break;
        }
        hModule = table[lValue].hSupObject;
        if (hModule == 0) {
            break;
        }
        has_mod = 1;
    case eProc_TransparentDisplay:
        flag = scratch->u.lValue;
        flags = spo->gu_lDrawModeMask;
        inv_AI_fn_vSetTextLabelUseScale(spo, flag & 0xFF);
        if (flag != 0) {
            flags &= ~8;
        } else {
            flags |= 8;
            if (has_mod) {
                hModule->flags &= ~0x10000;
            }
        }
        if (!has_mod) {
            ((tdstEngineObject_*) spo->hLinkedObject)->h_3dData->lDrawMask = flags;
            spo->gu_lDrawModeMask = flags;
        } else {
            hModule->gu_lDrawModeMask = flags;
        }
        break;
    }

    inv_AI_parameterPoolCursor = scratch;
    return p_stTree;
}

void* fn_p_stVisualGameMaterialProcedure(HIE_tdstSuperObject_* superObject,
                                         tdstNodeInterpret_* stream)
{
    tdstGetSetParam_* scratch;
    void* original;
    void* material;
    void* context;
    u16 op;
    GEO_tdstColor_ color;

    scratch = inv_AI_parameterPoolCursor;
    context = superObject;
    op = AI_NODE_ID(stream);
    original = stream;
    inv_AI_parameterPoolCursor = scratch + 1;

    stream = fn_p_stEvalTree(superObject, original, scratch);
    material = (void*) scratch->u.lValue;

    if (material == 0) {
        stream = fn_p_stSwapNextParameter2(original);
    } else {
        switch (op) {
        case eProc_SetVisualGMTSpecularExponent:
            stream = fn_p_stEvalTree(context, stream, scratch);
            break;

        case eProc_SetVisualGMTColor:
        case eProc_SetVisualGMTSpecularCoef:
            stream = fn_p_stEvalTree(context, stream, scratch);
            fn_vConvertParamToColor(scratch, &color);
            break;

        case eProc_SetVisualGMTDiffuseCoef:
            stream = fn_p_stEvalTree(context, stream, scratch);
            fn_vConvertParamToColor(scratch, &color);
            GLI_xSetMaterialColor(material, &color);
            break;

        case eProc_SetVisualGMTAmbientCoef:
            stream = fn_p_stEvalTree(context, stream, scratch);
            fn_vConvertParamToColor(scratch, &color);
            GLI_xSetMaterialAmbientCoef(material, &color);
            break;

        case eProc_SetVisualGMTAsChromed:
            stream = fn_p_stEvalTree(context, stream, scratch);
            inv_GLI_vSetMaterialBackfaceFlag(material, scratch->u.lValue);
            break;

        case eProc_SetVisualGMTTextureScrollingCoef: {
            f32 u;
            f32 v;

            stream = fn_p_stEvalTree(context, stream, scratch);
            u = inv_AI_fn_xAiParamFloat(scratch);
            stream = fn_p_stEvalTree(context, stream, scratch);
            v = inv_AI_fn_xAiParamFloat(scratch);
            _GLI_xSetMaterialTextureScrollingCoef(material, 0, 1, u, v);
            break;
        }

        case eProc_LockVisualGMT:
            stream = fn_p_stEvalTree(context, stream, scratch);
            GLI_vLockAnimatedTexture(material);
            break;

        case eProc_UnlockVisualGMT:
            GLI_vUnLockAnimatedTexture(material);
            break;

        case eProc_SetVisualGMTFrame: {
            s32 index;

            stream = fn_p_stEvalTree(context, stream, scratch);
            if (scratch->ucType == E_vt_Float) {
                index = inv_MTH_fn_lToIntegerOrdered(scratch->u.xValue);
            } else {
                index = scratch->u.lValue;
            }
            GLI_vSetCurrentAnimatedTextureNumber(material, index);
            break;
        }
        }
    }
    inv_AI_parameterPoolCursor = scratch;
    return stream;
}

void* fn_p_st_SinusEffect_SetFreq(HIE_tdstSuperObject_* p_SuperObjPerso,
                                  tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* current;
    void* context;
    u16 op;
    s32 lXFreq;
    s32 lYFreq;
    s32 lZFreq;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    current = p_stTree;
    op = AI_NODE_ID(current);
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    lXFreq = *(s16*) scratch;
    current = fn_p_stEvalTree(context, current, scratch);
    lYFreq = *(s16*) scratch;
    lZFreq = 0;
    if (op == 0x31) {
        current = fn_p_stEvalTree(context, current, scratch);
        lZFreq = *(s16*) scratch;
    }
    GLI_fn_SinusEffect_vSetFreq(lXFreq, lYFreq, lZFreq);
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

void* fn_p_st_SinusEffect_SetAmplitude(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    MTH3D_tdstVector_ scaled;
    f32 scale;
    void* current;

    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    scaled = stParam.u.stVector;
    scale = 0.1f;
    MTH3D_M_vScaleXYZ_inv(&scaled, &scaled, scale);
    GLI_fn_SinusEffect_vSetAmplitude(&scaled);
    return current;
}

void* fn_p_st_SinusEffect_SetRLIParams(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stParam;
    f32 x;
    f32 y;
    f32 z;
    void* current;
    void* context;
    u16 op;

    context = p_SuperObjPerso;
    current = p_stTree;
    op = AI_NODE_ID(current);
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stParam);
    x = stParam.u.xValue;
    current = fn_p_stEvalTree(context, current, &stParam);
    y = stParam.u.xValue;
    current = fn_p_stEvalTree(context, current, &stParam);
    z = stParam.u.xValue;
    if (op == eProc_SinEffect_SetRLIBase) {
        GLI_vSetSinusEffectOnRLIBase(x, y, z);
    } else if (op == eProc_SinEffect_SetRLIAmplitude) {
        GLI_vSetSinusEffectOnRLIAmp(x, y, z);
    }
    return current;
}

#ifdef PC_PORT
tdstNodeInterpret_* fn_p_st_SinusEffect_SetState(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree)
#else
tdstNodeInterpret_* fn_p_st_SinusEffect_SetState(HIE_tdstSuperObject_* p_SuperObjPerso,
                                                 tdstNodeInterpret_* p_stTree, void* unused)
#endif
{
    tdstGetSetParam_ value;
    s8 mode;
    s8 flags;
    void* next =
        fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &value);
    mode = value.u.lValue;
    if (mode < 64) {
        GLI_fn_SinusEffect_vSetState(mode);
    } else {
        flags = GLI_fn_SinusEffect_vGetFlag();
        switch (mode) {
        case 64:
            flags &= ~1;
            break;
        case 65:
            flags |= 1;
            break;
        case 66:
            flags &= ~2;
            break;
        case 67:
            flags |= 2;
            break;
        }
        GLI_fn_SinusEffect_vSetFlag(flags);
    }
    return next;
}

void* fn_p_st_SPO_SetDrawFlag(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    HIE_tdstSuperObject_* hSpo;
    s32 lMask;
    s32 lType;
    s32 lDrawMask;
    s32 lBit;
    s32 one;
    s32 state;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    hSpo = (HIE_tdstSuperObject_*) scratch->u.lValue;
    current = fn_p_stEvalTree(context, current, scratch);
    lMask = scratch->u.lValue;
    current = fn_p_stEvalTree(context, current, scratch);
    lDrawMask = scratch->u.lValue;

    if (hSpo != 0) {
        if (!(lMask < 0)) {
            if (!(lMask >= 0x20)) {
                one = 1;
                lBit = one << lMask;
                lType = hSpo->gu_lDrawModeMask;
                state = lDrawMask;
                if (state == 2) {
                    state = ((lType & lBit) == 0);
                }
                switch (state) {
                case 0:
                    lType = lType & ~lBit;
                    break;
                case 1:
                    lType = lType | lBit;
                    break;
                default:
                    break;
                }
                hSpo->gu_lDrawModeMask = lType;
            }
        }
    }
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

void* fn_p_st_SPO_SetEngineDisplayModeFlag(HIE_tdstSuperObject_* p_SuperObjPerso,
                                           tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    s32 lBit;
    s32 lType;
    u32 lMask;
    u32 one;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    lBit = scratch->u.lValue;
    current = fn_p_stEvalTree(context, current, scratch);
    lType = scratch->u.lValue;

    if ((u32) lBit < 0x20u) {
        one = 1;
        lMask = one << lBit;
        if (lType == 2) {
            lType = (u32) (g_stEngineStructure.ulDisplayMode & lMask) < 1u;
        }
        switch (lType) {
        case 0: {
            tdstEngineStructure_* clearEng;
            lMask = ~lMask;
            clearEng = &g_stEngineStructure;
            lType = clearEng->ulDisplayMode;
            clearEng->ulDisplayMode = lType & lMask;
            break;
        }
        case 1: {
            tdstEngineStructure_* setEng = &g_stEngineStructure;
            s32 lDisplayMode = setEng->ulDisplayMode;
            setEng->ulDisplayMode = lDisplayMode | lMask;
            break;
        }
        }
    }
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

void* fn_p_st_ACT_SetDrawFlag(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    HIE_tdstSuperObject_* hCharact;
    tdstEngineObject_* obj;
    s32 lMask;
    s32 lType;
    s32 lDrawMask;
    s32 lBit;
    s32 one;
    gu_tdstStandardGame* inner;
    tdstEngineObject_* engine;
    gu_tdst3dData_* data3d;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    obj = (tdstEngineObject_*) scratch->u.lValue;
    inner = obj->h_StandardGame;
    hCharact = inner->h_SuperObject;
    current = fn_p_stEvalTree(context, current, scratch);
    lMask = scratch->u.lValue;
    current = fn_p_stEvalTree(context, current, scratch);
    lType = scratch->u.lValue;

    if (hCharact != 0) {
        if (!(lMask < 0)) {
            if (lMask >= 0x20) {
                inv_AI_parameterPoolCursor = scratch;
                return current;
            }
            one = 1;
            engine = hCharact->hLinkedObject;
            lBit = one << lMask;
            data3d = engine->h_3dData;
            lDrawMask = data3d->lDrawMask;
            if (lType == 2) {
                lType = (u32) (lDrawMask & lBit) < 1u;
            }
            if (lType != 0) {
                if (lType == one) {
                    lDrawMask = lDrawMask | lBit;
                    goto store;
                }
                hCharact->gu_lDrawModeMask = lDrawMask;
                goto store_3d;
            }
            lDrawMask = lDrawMask & ~lBit;
        store:
            hCharact->gu_lDrawModeMask = lDrawMask;
        store_3d:
            data3d->lDrawMask = lDrawMask;
        }
    }
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

#ifdef PC_PORT
tdstNodeInterpret_* fn_p_stLoopKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
#else
tdstNodeInterpret_* fn_p_stLoopKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso,
                                       tdstNodeInterpret_* p_stTree, void* unused)
#endif
{
    tdstGetSetParam_ value;
    tdstNodeInterpret_* p_stTreeInit = p_stTree - 1;
    s32 depth;
    s32 count;
    GlobalLoopInit[iNumLoop] = (s32) p_stTreeInit;
    p_stTree =
        fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &value);
    depth = iNumLoop;
    count = value.u.lValue;
    GlobalLoopCpt[depth] = count;
    if (count == 0)
        return p_stTreeInit + p_stTreeInit->uwNodeToSkip;
    iNumLoop = depth + 1;
    GlobalLoopTree[depth] = (s32) p_stTree;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stEndLoopKeyWord(HIE_tdstSuperObject_* p_SuperObjPerso,
                                          tdstNodeInterpret_* p_stTree)
{
    s32 depth;
    s32 index;
    s32 count;

    depth = iNumLoop;
    if (depth == 0) {
        return p_stTree;
    }
    index = depth - 1;
    count = GlobalLoopCpt[index];
    count = count - 1;
    GlobalLoopCpt[index] = count;
    if (count > 0) {
        return (void*) GlobalLoopTree[index];
    }
    iNumLoop = index;
    return p_stTree;
}

void* fn_p_stBreakKeyword(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_ stValue;
    s32 index;
    s32 n;
    s32 previousIndex;
    s32 lt;
    s32* table;
    tdstNodeInterpret_* node;
    u16 skip;

    fn_p_stEvalTree(p_SuperObjPerso, p_stTree, &stValue);
    index = iNumLoop;
    table = GlobalLoopInit;
    n = stValue.u.lValue;
    previousIndex = index - 1;
    lt = previousIndex < n;
    if (lt) {
        n = previousIndex;
    }
    index = index - n;
    previousIndex = previousIndex - n;
    index = index - 1;
    node = (tdstNodeInterpret_*) table[index];
    skip = node->uwNodeToSkip;
    iNumLoop = previousIndex;
    return node + skip;
}

tdstNodeInterpret_* fn_p_stRLIProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                        tdstNodeInterpret_* p_stTree)
{
    u8 lim;
    HIE_tdstSuperObject_* hNewSuperObjPerso;

    lim = p_stTree[-1].ucDepth;
    fn_vGetUltraOperatorPerso(p_SuperObjPerso, &hNewSuperObjPerso);
    while (lim < p_stTree->ucDepth) {
        p_stTree = fn_p_stSkipThisArgument(p_stTree);
    }
    return p_stTree;
}

void* fn_p_stAGOProcedure(HIE_tdstSuperObject_* p_SuperObjPerso, tdstNodeInterpret_* p_stTree)
{
    MTH3D_tdstVector_ pos;
    MTH3D_tdstVector_ dir;
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    s32 type;
    void* owner;
    f32 fRate;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    type = scratch->u.lValue;
    current = fn_p_stEvalTree(context, current, scratch);
    MTH3D_M_vCopyXYZ_inv(&pos, &scratch->u.stVector);
    current = fn_p_stEvalTree(context, current, scratch);
    MTH3D_M_vCopyXYZ_inv(&dir, &scratch->u.stVector);
    current = fn_p_stEvalTree(context, current, scratch);
    owner = (void*) scratch->u.lValue;
    current = fn_p_stEvalTree(context, current, scratch);
    if (scratch->ucType == E_vt_Float) {
        fRate = scratch->u.xValue;
    } else {
        fRate = (f32) scratch->u.lValue;
    }
    AGO_vAddParticle(type, (MTH3D_tdstVector_*) &pos, (MTH3D_tdstVector_*) &dir, owner, fRate);
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

s32 func_002002E8(void* superObject, void* stream)
{
    tdstGetSetParam_ stParam;
    void* result = fn_p_stEvalTree(superObject, stream, &stParam);
    ((u8*) stParam.u.pvValue)[0x164] &= (u8) ~1;
    return (s32) result;
}

void* func_00200318(void* superObject, void* stream)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* spo;
    void* current;
    void* context;
    u8* object;

    context = superObject;
    current = stream;
    fn_vGetUltraOperatorPerso(superObject, &spo);
    current = fn_p_stEvalTree(superObject, stream, &stParam);
    object = (u8*) stParam.u.lValue;
    object[0x164] &= (u8) ~1;
    return current;
}

s32 func_00200370(void* superObject, void* stream)
{
    tdstGetSetParam_ stParam;
    void* result = fn_p_stEvalTree(superObject, stream, &stParam);
    ((u8*) stParam.u.pvValue)[0x164] |= 1;
    return (s32) result;
}

void* func_002003A0(void* superObject, void* stream)
{
    tdstGetSetParam_ stParam;
    HIE_tdstSuperObject_* spo;
    void* current;
    void* context;
    u8* object;

    context = superObject;
    current = stream;
    fn_vGetUltraOperatorPerso(superObject, &spo);
    current = fn_p_stEvalTree(superObject, stream, &stParam);
    object = (u8*) stParam.u.lValue;
    object[0x164] |= 1;
    return current;
}

void* fn_p_stInGameMenuProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                 tdstNodeInterpret_* p_stTree)
{
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    s32 flag;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    inv_AI_parameterPoolCursor = scratch + 1;
    current = fn_p_stEvalTree(p_SuperObjPerso, p_stTree, scratch);
    flag = scratch->u.lValue;
    if (flag) {
        g_bInGameMenu = 1;
        inv_TXT_fn_vHideScriptStrings();
        current = current;
    } else {
        g_bInGameMenu = 0;
        inv_TXT_fn_vShowScriptStrings();
        current = current;
    }
    current = fn_p_stEvalTree(context, current, scratch);
    flag = scratch->u.lValue;
    if (flag) {
        inv_GAM_bMenuCharactersOnly = 1;
    } else {
        inv_GAM_bMenuCharactersOnly = 0;
    }
    inv_AI_parameterPoolCursor = scratch;
    return current;
}

tdstNodeInterpret_* fn_p_stCheatCodeProcedure(HIE_tdstSuperObject_* superObject,
                                              tdstNodeInterpret_* p_stTree)
{
    (void) superObject;
    return p_stTree;
}

tdstNodeInterpret_* fn_p_stJFFTXTProcedure(HIE_tdstSuperObject_* p_SuperObjPerso,
                                           tdstNodeInterpret_* p_stTree)
{
    MTH3D_tdstVector_ pos;
    tdstGetSetParam_* scratch;
    void* context;
    void* current;
    u16 op;
    s32 id;
    void* str;
    f32 scale;
    s32 lValue;

    scratch = inv_AI_parameterPoolCursor;
    context = p_SuperObjPerso;
    current = p_stTree;
    op = *(u16*) ((u8*) current - 8);
    inv_AI_parameterPoolCursor = scratch + 1;

    switch (op) {
    case 0x20B:
        current = fn_p_stEvalTree(p_SuperObjPerso, current, scratch);
        id = scratch->u.lValue;
        current = fn_p_stEvalTree(context, current, scratch);
        pos = *(MTH3D_tdstVector_*) scratch;
        current = fn_p_stEvalTree(context, current, scratch);
        str = fn_szGetStringFromTextOrStringParam(scratch);
        current = fn_p_stEvalTree(context, current, scratch);
        if (scratch->ucType == E_vt_Float) {
            scale = scratch->u.xValue;
        } else {
            scale = (f32) scratch->u.lValue;
        }
        JFFTXT_vAddText(id, &pos, str, scale);
        break;

    case 0x20C: {
        s32 id2;
        current = fn_p_stEvalTree(context, current, scratch);
        id = scratch->u.lValue;
        current = fn_p_stEvalTree(context, current, scratch);
        id2 = scratch->u.lValue;
        current = fn_p_stEvalTree(context, current, scratch);
        if (scratch->ucType == E_vt_Float) {
            scale = scratch->u.xValue;
        } else {
            scale = (f32) scratch->u.lValue;
        }
        JFFTXT_vExtendText(id, id2, scale);
        break;
    }
    case 0x20D: {
        s32 id2;
        current = fn_p_stEvalTree(context, current, scratch);
        id = scratch->u.lValue;
        current = fn_p_stEvalTree(context, current, scratch);
        id2 = scratch->u.lValue;
        current = fn_p_stEvalTree(context, current, scratch);
        lValue = scratch->u.lValue;
        JFFTXT_vModifyCaracter(id, id2, lValue);
        break;
    }
    }

    inv_AI_parameterPoolCursor = scratch;
    return current;
}
