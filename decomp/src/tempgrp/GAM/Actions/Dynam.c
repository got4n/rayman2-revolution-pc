#include "common.h"

#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"

#include "ps2/libc.h"
#include "AI/AI.h"

#define DYNAMICS_ALWAYS 0x40000
#define DYNAMICS_HAS_REPORT 0x4000
#define DYNAMICS_ADVANCED_SIZE 0x1D4
#define DYNAMICS_COMPLEX_SIZE 0x328

tdstDynam_* fn_h_DynamRealAlloc(void);
void fn_v_DynamicAlloc(tdstDynam_* h_Dynam, s32 eSize, s32 bCollision, s32 bAlways);

static inline s32 inv_GAM_fn_lDynamicsFlags(DNM_stDynamics* dynamics)
{
    return dynamics->m_tdstDynamicsBaseBlock.ulEndFlags;
}

static inline s32 inv_GAM_fn_bHasReport(s32 flags)
{
    return (flags >> 14) & 1;
}

void fn_vDynamAlloc(tdstEngineObject_* p_stObject)
{
    tdstDynam_* component = fn_h_DynamRealAlloc();
    gu_tdstStandardGame* standardGame = p_stObject->h_StandardGame;

    p_stObject->h_Dynam = component;
    fn_v_DynamicAlloc(component, eDynamicSizeBase, 0, (standardGame->ucMiscFlags >> 4) & 1);
}

tdstDynam_* fn_h_DynamRealAlloc(void)
{
    return AI_fn_p_vTrueAlloc(gp_6F74, sizeof(tdstDynam_));
}

void fn_v_DynamicAlloc(tdstDynam_* h_Dynam, s32 ucSize, s32 ucCollision, s32 ucAlways)
{
    u8 eSize = ucSize;
    u8 bCollision = ucCollision;
    u8 bAlways = ucAlways;
    DNM_stDynamics* p_stDynamic = NULL;

    if (bAlways) {
        switch (eSize) {
        case eDynamicSizeBase:
            p_stDynamic = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(DNM_stDynamicsBase_));
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & ~7) | 1;
            break;
        case eDynamicSizeAdvanced:
            p_stDynamic = AI_fn_p_vTrueAlloc(gp_6F74, DYNAMICS_ADVANCED_SIZE);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & ~7) | 3;
            break;
        case eDynamicSizeComplex:
            p_stDynamic = AI_fn_p_vTrueAlloc(gp_6F74, DYNAMICS_COMPLEX_SIZE);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 7;
            break;
        }
    } else {
        switch (eSize) {
        case eDynamicSizeBase:
            p_stDynamic = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(DNM_stDynamicsBase_));
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & ~7) | 1;
            break;
        case eDynamicSizeAdvanced:
            p_stDynamic = AI_fn_p_vTrueAlloc(gp_6F74, DYNAMICS_ADVANCED_SIZE);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & ~7) | 3;
            break;
        case eDynamicSizeComplex:
            p_stDynamic = AI_fn_p_vTrueAlloc(gp_6F74, DYNAMICS_COMPLEX_SIZE);
            p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= 7;
            break;
        }
    }
    h_Dynam->p_stDynamics = p_stDynamic;

    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = bAlways ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= DYNAMICS_ALWAYS)
                                 : (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_ALWAYS);
    if (bCollision) {
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(DNM_stReport_));
    } else {
        p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport = NULL;
    }
    p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags = bCollision ? (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags |= DYNAMICS_HAS_REPORT)
                                   : (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_HAS_REPORT);
}

void fn_v_DynamicFree(tdstDynam_* h_Dynam)
{
    DNM_stDynamics* p_stDynamic = h_Dynam->p_stDynamics;

    if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_ALWAYS) {
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_HAS_REPORT) {
            inv_MEM_fn_vFreeInHeap(gp_6F74, p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport);
            p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport = NULL;
        }
        inv_MEM_fn_vFreeInHeap(gp_6F74, p_stDynamic);
    } else {
        if (p_stDynamic->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_HAS_REPORT) {
            inv_MEM_fn_vFreeInHeap(gp_6F74, p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport);
            p_stDynamic->m_tdstDynamicsBaseBlock.m_pstReport = NULL;
        }
        inv_MEM_fn_vFreeInHeap(gp_6F74, p_stDynamic);
    }
    h_Dynam->p_stDynamics = NULL;
}

void fn_vDynamInit(tdstEngineObject_* p_stObject, u8 eObjectInit)
{
    tdstDynam_* component;
    s32 type = eObjectInit;

    if (type < 0) {
        return;
    }
    if (type >= 5) {
        return;
    }
    component = p_stObject->h_Dynam;
    DNM_fn_vInitBaseMechanic(component->p_stDynamics);
    memset(p_stObject->h_Dynam, 0, sizeof(DNM_stParsingDatas));
}

void fn_vDynamDesinit(tdstEngineObject_* p_stObject, s32 eObjectInit)
{
    tdstDynam_* component;

    eObjectInit &= 0xFF;
    if (eObjectInit == OTI_ReinitTheMap) {
        return;
    }
    if (eObjectInit >= 3) {
        return;
    }
    if (eObjectInit != OTI_MapLoaded) {
        return;
    }
    component = p_stObject->h_Dynam;
    if (!(component->p_stDynamics->m_tdstDynamicsBaseBlock.ulEndFlags & DYNAMICS_ALWAYS)) {
        return;
    }
    fn_v_DynamicFree(component);
}

void fn_vDynamCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    tdstDynam_* hDest = _p_stObjectDest->h_Dynam;
    tdstDynam_* hSrc = _p_stObjectSrc->h_Dynam;
    DNM_stDynamics* p_stDestDynamic = hDest->p_stDynamics;
    DNM_stDynamics* p_stSrcDynamic = hSrc->p_stDynamics;
    DNM_stDynamics* dynamics;
    s32 ulFlags;
    s32 eSize;
    u32 bAlways;
    u32 bAlloc = 1;

    if (p_stSrcDynamic == NULL) {
        return;
    }
    ulFlags = inv_GAM_fn_lDynamicsFlags(p_stSrcDynamic);
    bAlways = (_p_stObjectDest->h_StandardGame->ucMiscFlags >> 4) & 1;
    eSize = eDynamicSizeBase;
    if (ulFlags & 4) {
        eSize = eDynamicSizeComplex;
    } else if (ulFlags & 2) {
        eSize = eDynamicSizeAdvanced;
    }
    if (p_stDestDynamic != NULL) {
        switch (eSize) {
        case eDynamicSizeComplex:
            bAlloc = ((inv_GAM_fn_lDynamicsFlags(p_stDestDynamic) >> 2) ^ 1) & 1;
            break;
        case eDynamicSizeAdvanced:
            bAlloc = ((inv_GAM_fn_lDynamicsFlags(p_stDestDynamic) >> 1) ^ 1) & 1;
            break;
        case eDynamicSizeBase:
            bAlloc = (inv_GAM_fn_lDynamicsFlags(p_stDestDynamic) ^ 1) & 1;
            break;
        }
        if (inv_GAM_fn_bHasReport(ulFlags) != inv_GAM_fn_bHasReport(inv_GAM_fn_lDynamicsFlags(p_stDestDynamic))) {
            bAlloc = 1;
        }
        if (bAlloc) {
            fn_v_DynamicFree(hDest);
        }
    }
    if (bAlloc) {
        fn_v_DynamicAlloc(hDest, eSize, inv_GAM_fn_bHasReport(inv_GAM_fn_lDynamicsFlags(p_stSrcDynamic)), bAlways);
    }
    DNM_fn_vDynamicsCopyClone(hDest->p_stDynamics, hSrc->p_stDynamics);
    hDest->stParsingDatas = hSrc->stParsingDatas;
    dynamics = hDest->p_stDynamics;
    dynamics->m_tdstDynamicsBaseBlock.ulEndFlags = bAlways ? (dynamics->m_tdstDynamicsBaseBlock.ulEndFlags |= DYNAMICS_ALWAYS)
                                 : (dynamics->m_tdstDynamicsBaseBlock.ulEndFlags &= ~DYNAMICS_ALWAYS);
}

void func_00108F48(tdstDynam_* component)
{
    DNM_fn_vDynamicsInitDefault(component->p_stDynamics);
}

DNM_stDynamics* fn_p_stDynamGetDynamicsSO(HIE_tdstSuperObject_* hSupObj)
{
    tdstEngineObject_* object = hSupObj->hLinkedObject;
    tdstDynam_* component;
    DNM_stDynamics* p_stDynamics = NULL;

    if (object != NULL) {
        component = object->h_Dynam;
        if (component != NULL) {
            p_stDynamics = component->p_stDynamics;
        }
    }
    return p_stDynamics;
}
