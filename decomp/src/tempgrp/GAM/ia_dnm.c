#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "AI/AI.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

void fn_vMakeCharacterMechanicallyReact(HIE_tdstSuperObject_* superObject);

static inline u16 inv_GAM_fn_uwBrainComputationCount(gu_tdst3dData_* data3d)
{
    return data3d != NULL ? data3d->uwBrainMainCounter : 0;
}

void func_001018B8(void* savedContext, void* vector, void* outParam)
{
}

void fn_vSuperObjectSetLastCollidedCharacter(HIE_tdstSuperObject_* superObject,
                                             HIE_tdstSuperObject_* character)
{
    tdstDynam_* component = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;
    DNM_stReport_* report;

    if (component == NULL) {
        return;
    }
    report = component->p_stDynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    if (report == NULL) {
        return;
    }
    report->m_stCharacter.mp_stSupObj = character;
    if (character == NULL) {
        report->m_ulCurrSurfaceState &= ~0x10000;
    }
}

HIE_tdstSuperObject_* fn_hSuperObjectGetLastCollidedCharacter(HIE_tdstSuperObject_* superObject)
{
    tdstDynam_* component = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;
    DNM_stReport_* report;
    HIE_tdstSuperObject_* character;

    if (component == NULL) {
        return NULL;
    }
    report = component->p_stDynamics->m_tdstDynamicsBaseBlock.m_pstReport;
    if (report == NULL) {
        return NULL;
    }
    character = report->m_stCharacter.mp_stSupObj;
    while (character != NULL && character->lTypeOfLinkedObject != HIE_TYPE_ACTOR_inv) {
        character = character->hFatherSta;
    }
    if (character != NULL && inv_HIE_fn_p_stGetEngineObject(character)->h_StandardGame == NULL) {
        report->m_stCharacter.mp_stSupObj = NULL;
        character = NULL;
    }
    return character;
}

void InitArrayOfDynamicFunction(MechanicsCallback* p_fnDynamicFunction)
{
    p_fnDynamicFunction[0] = (MechanicsCallback) DNM_p_stDynamicsCameraMechanics;
    p_fnDynamicFunction[1] = (MechanicsCallback) MEC_p_stDynamicsBaseMechanics;
}

void* fn_h_GetCurrentDNMIdCard(HIE_tdstSuperObject_* p_SuperObjPerso)
{
    tdstState_* h_State = inv_HIE_fn_p_stGetEngineObject(p_SuperObjPerso)->h_3dData->h_CurrentState;

    if (h_State == NULL) {
        return NULL;
    }
    return h_State->h_LinkedMechanicsIdCard;
}

void fn_vTransferSpeedInformationPLA_DNM(HIE_tdstSuperObject_* superObject, s16 frame)
{
    tdstDynam_* component = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;
    DNM_stDynamics* dynamics;
    s32 hasLinearSpeed;
    s32 hasAngularSpeed;
    MTH3D_tdstVector_ linearSpeed;
    MTH3D_tdstMatrix_ angularSpeed;

    if (component == NULL) {
        return;
    }
    dynamics = component->p_stDynamics;
    hasLinearSpeed = PLA_fn_stGetLinearSpeedVector(superObject, frame, &linearSpeed);
    hasAngularSpeed = PLA_fn_stGetAngularSpeedMatrix(superObject, frame, &angularSpeed);
    if (hasLinearSpeed) {
        dynamics->m_tdstDynamicsBaseBlock.m_stSpeedAnim = linearSpeed;
    }
    if (hasAngularSpeed) {
        MEC_vImposeRelativeRotation(dynamics, &angularSpeed);
    }
}

void fn_vMakeCharacterThink(HIE_tdstSuperObject_* p_stSuperObj)
{
    gu_tdst3dData_* data3d = inv_HIE_fn_p_stGetEngineObject(p_stSuperObj)->h_3dData;
    tdstEngineObject_* object;

    fn_v3dDataUpdateBrainCounter(p_stSuperObj);
    if (!fn_b3dDataCanComputeBrain(p_stSuperObj)) {
        return;
    }
    object = inv_HIE_fn_p_stGetEngineObject(p_stSuperObj);
    if (object->h_Brain == NULL) {
        return;
    }
    if (data3d != NULL) {
        data3d->uwBrainMainCounter++;
    }
    gcGlobAleat = inv_GAM_fn_uwBrainComputationCount(data3d);
    if (fn_bDoReflexPerso(p_stSuperObj)) {
        return;
    }
    object = inv_HIE_fn_p_stGetEngineObject(p_stSuperObj);
    if (object->h_Brain == NULL) {
        return;
    }
    fn_bDoIntelligencePerso(p_stSuperObj);
}

void fn_vMakeCharacterMechanicallyReact(HIE_tdstSuperObject_* superObject)
{
    tdstDynam_* component = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;
    tdstDynam_* current;
    DNM_stDynamics* dynamics;
    MechanicsCallback* slot;
    MechanicsCallback mechanics;
    void* idCard;
    f32 delta;

    if (component == NULL) {
        return;
    }
    dynamics = component->p_stDynamics;
    idCard = fn_h_GetCurrentDNMIdCard(superObject);
    if (dynamics->m_tdstDynamicsBaseBlock.m_pCurrentIdCard != idCard) {
        DNM_fn_vChangeIdCard(dynamics, idCard);
    }
    if (idCard == NULL) {
        return;
    }
    slot = &g_a_pfnDynamicFunction[DNM_fn_eIdentityCardGetType(idCard)];
    current = inv_HIE_fn_p_stGetEngineObject(superObject)->h_Dynam;
    delta = inv_fn_xGetEngineClockDelta();
    mechanics = *slot;
    mechanics(dynamics, superObject, current, idCard, delta);
    if (dynamics->m_tdstDynamicsBaseBlock.ulEndFlags & 0x4000) {
        dynamics->m_tdstDynamicsBaseBlock.m_pstReport->m_ulPrevSurfaceState = dynamics->m_tdstDynamicsBaseBlock.m_pstReport->m_ulCurrSurfaceState;
    }
}

void fn_vMakeCharacterReact(HIE_tdstSuperObject_* p_stSuperObj)
{
    gu_tdstStandardGame* hStdGame = inv_HIE_fn_p_stGetEngineObject(p_stSuperObj)->h_StandardGame;
    u32 ulCurrentBits = hStdGame->ulCustomBits;

    if ((ulCurrentBits & 0x1800) == 0x1800) {
        return;
    }
    if (!(ulCurrentBits & 0x10000) && !((ulCurrentBits & 0x80000) && (hStdGame->ucMiscFlags >> 7))) {
#ifdef PC_PORT
        /* In noclip, Rayman's AI is left out: its actions (Cross jumps) would change his pose. */
        if (!pc_mod_noclip_active((PcAddress) p_stSuperObj)) {
            fn_vMakeCharacterThink(p_stSuperObj);
        }
#else
        fn_vMakeCharacterThink(p_stSuperObj);
#endif
        hStdGame = inv_HIE_fn_p_stGetEngineObject(p_stSuperObj)->h_StandardGame;
        if (hStdGame == NULL) {
            return;
        }
        ulCurrentBits = hStdGame->ulCustomBits;
    }
    if (hStdGame == NULL || (ulCurrentBits & 0x8000)) {
        return;
    }
    if ((ulCurrentBits & 0x400000) && (hStdGame->ucMiscFlags >> 7)) {
        return;
    }
    fn_vMakeCharacterMechanicallyReact(p_stSuperObj);
}
