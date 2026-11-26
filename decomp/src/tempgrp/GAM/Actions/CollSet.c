#include "common.h"

#include "COL/COL.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "AI/AI.h"

typedef struct CollisionSetCopy {
    u32 words[0x11];
} CollisionSetCopy;

CollisionSet* fn_h_CollSetRealAlloc(tdstEngineObject_* object);
void fn_vCorrectAllActivationLists(tdstEngineObject_* object);
void* CS_fn_hGetActivationListInCollSet(u8 type, CollisionSet* collisionSet);

static inline CollisionSet* inv_GAM_fn_p_stCollisionSetOfSuperObject(HIE_tdstSuperObject_* superObject)
{
    tdstEngineObject_* object = superObject->hLinkedObject;

    return inv_GAM_fn_p_stCollisionSetOf(object);
}

static inline void inv_GAM_fn_vClearObjectPrivileges(tdstEngineObject_* object)
{
    inv_GAM_fn_p_stCollisionSetOf(object)->stColliderInfo.ucColliderPriority = 0;
    inv_GAM_fn_p_stCollisionSetOf(object)->ulFBZddPriviligedZones = 0;
    inv_GAM_fn_p_stCollisionSetOf(object)->ulFBZdePriviligedZones = 0;
    inv_GAM_fn_p_stCollisionSetOf(object)->ulFBZdmPriviligedZones = 0;
    inv_GAM_fn_p_stCollisionSetOf(object)->ulFBZdrPriviligedZones = 0;
}

void fn_vCollSetAlloc(tdstEngineObject_* p_stObject)
{
    p_stObject->h_CollSet = fn_h_CollSetRealAlloc(p_stObject);
}

CollisionSet* fn_h_CollSetRealAlloc(tdstEngineObject_* object)
{
    CollisionSet* h_CollSet = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(CollisionSet));

    h_CollSet->ucCharacterPriority = 0;
    h_CollSet->ucCollComputeFrequency = 1;
    h_CollSet->hZddList = NULL;
    h_CollSet->hZdeList = NULL;
    h_CollSet->hZdmList = NULL;
    h_CollSet->hZdrList = NULL;
    h_CollSet->hZddActivationList = NULL;
    h_CollSet->hZdeActivationList = NULL;
    h_CollSet->hZdmActivationList = NULL;
    h_CollSet->hZdrActivationList = NULL;
    h_CollSet->stColliderInfo.ucColliderPriority = 0;
    h_CollSet->ulFBZddPriviligedZones = 0;
    h_CollSet->ulFBZdePriviligedZones = 0;
    h_CollSet->ulFBZdmPriviligedZones = 0;
    h_CollSet->ulFBZdrPriviligedZones = 0;
    return h_CollSet;
}

void fn_vCollSetInit(tdstEngineObject_* p_stObject, u32 eObjectInit)
{
    switch (eObjectInit & 0xFF) {
    case OTI_WhenGameStart:
    case OTI_MapLoaded:
    case OTI_LoadSavedGame:
        inv_GAM_fn_vClearObjectPrivileges(p_stObject);
        fn_vCorrectAllActivationLists(p_stObject);
        break;
    case OTI_ReinitTheMap:
    case 4:
        inv_GAM_fn_vClearObjectPrivileges(p_stObject);
        break;
    case OTI_AlwaysCreated:
    default:
        break;
    }
}

void fn_vCollSetDesinit(void)
{
}

void fn_vCollSetCopyClone(tdstEngineObject_* _p_stObjectDest, tdstEngineObject_* _p_stObjectSrc)
{
    CollisionSetCopy* copy = (CollisionSetCopy*) _p_stObjectDest->h_CollSet;
    CollisionSetCopy* original = (CollisionSetCopy*) _p_stObjectSrc->h_CollSet;

    *copy = *original;
}

void* CS_fn_hGetZdxList(u8 ucZoneType, HIE_tdstSuperObject_* hSuperObj)
{
    CollisionSet* hCollSet = inv_GAM_fn_p_stCollisionSetOfSuperObject(hSuperObj);

    if (hCollSet == NULL) {
        return NULL;
    }
    switch (ucZoneType) {
    case 0:
        return hCollSet->hZddList;
    case 2:
        return hCollSet->hZdeList;
    case 1:
        return hCollSet->hZdmList;
    case 3:
        return hCollSet->hZdrList;
    default:
        return NULL;
    }
}

void* CS_fn_hGetActivationList(u8 ucZoneType, HIE_tdstSuperObject_* hSuperObj)
{
    CollisionSet* hCollSet = hSuperObj != NULL ? inv_GAM_fn_p_stCollisionSetOfSuperObject(hSuperObj) : NULL;

    if (hCollSet != NULL) {
        return CS_fn_hGetActivationListInCollSet(ucZoneType, hCollSet);
    }
    return NULL;
}

void* CS_fn_hGetActivationListInCollSet(u8 ucZoneType, CollisionSet* hCollSet)
{
    switch (ucZoneType) {
    case 0:
        return hCollSet->hZddActivationList;
    case 2:
        return hCollSet->hZdeActivationList;
    case 1:
        return hCollSet->hZdmActivationList;
    case 3:
        return hCollSet->hZdrActivationList;
    default:
        return NULL;
    }
}

s32 fn_bGetCharacterForceCollisionWhenNotMoving(HIE_tdstSuperObject_* _hSuperObject)
{
    CollisionSet* collisionSet = _hSuperObject != NULL ? inv_GAM_fn_p_stCollisionSetOfSuperObject(_hSuperObject) : NULL;

    if (collisionSet != NULL) {
        return collisionSet->ucCollisionFlag & 0x10;
    }
    return 0;
}

s32 fn_ucCollSetGetCollComputeFrequencySO(HIE_tdstSuperObject_* hSuperObj)
{
    HIE_tdstSuperObject_* object = hSuperObj;
    CollisionSet* collisionSet = hSuperObj != NULL ? inv_GAM_fn_p_stCollisionSetOfSuperObject(hSuperObj) : NULL;

    if (collisionSet != NULL) {
        return collisionSet->ucCollComputeFrequency;
    }
    return 1 + (object == NULL) - (object == NULL);
}

void fn_vCorrectAllActivationLists(tdstEngineObject_* _p_stObject)
{
    CollisionSet* collisionSet = _p_stObject->h_CollSet;
    s32 context;

    if (collisionSet == NULL) {
        return;
    }
    context = _p_stObject->h_3dData->h_Family->hForStateArray.lNumberOfElementsOpt;
    if (collisionSet->hZddActivationList != NULL) {
        fn_bHadToCorrectActivationList(collisionSet->hZddActivationList, context);
    }
    if (collisionSet->hZdeActivationList != NULL) {
        fn_bHadToCorrectActivationList(collisionSet->hZdeActivationList, context);
    }
    if (collisionSet->hZdmActivationList != NULL) {
        fn_bHadToCorrectActivationList(collisionSet->hZdmActivationList, context);
    }
    if (collisionSet->hZdrActivationList == NULL) {
        return;
    }
    fn_bHadToCorrectActivationList(collisionSet->hZdrActivationList, context);
}

void fn_vSetCurrentZoneSetOfType(u8 _ucZoneType, CollisionSet* _hCollSet, tdstZoneSetList_* _hCurrentZoneSet)
{
    switch (_ucZoneType) {
    case 0:
        _hCollSet->hZddCurrentActivation = _hCurrentZoneSet;
        break;
    case 3:
        _hCollSet->hZdrCurrentActivation = _hCurrentZoneSet;
        break;
    case 2:
        _hCollSet->hZdeCurrentActivation = _hCurrentZoneSet;
        break;
    case 1:
        _hCollSet->hZdmCurrentActivation = _hCurrentZoneSet;
        break;
    }
}

void* fn_hGetCurrentZoneSetOfType(u8 _ucZoneType, CollisionSet* _hCollSet)
{
    switch (_ucZoneType) {
    case 0:
        return _hCollSet->hZddCurrentActivation;
    case 3:
        return _hCollSet->hZdrCurrentActivation;
    case 2:
        return _hCollSet->hZdeCurrentActivation;
    case 1:
        return _hCollSet->hZdmCurrentActivation;
    default:
        return NULL;
    }
}

u32 CS_fn_ulGetPrivilegedActivationZoneInCollSet(u8 _ucZoneType, CollisionSet* _hCollSet)
{
    switch (_ucZoneType) {
    case 0:
        return _hCollSet->ulFBZddPriviligedZones;
    case 3:
        return _hCollSet->ulFBZdrPriviligedZones;
    case 2:
        return _hCollSet->ulFBZdePriviligedZones;
    case 1:
        return _hCollSet->ulFBZdmPriviligedZones;
    default:
        return 0;
    }
}

void CS_fn_vSetPrivilegedActivationZoneInCollSet(u8 type, CollisionSet* collisionSet, u32 bits)
{
    switch (type) {
    case 0:
        collisionSet->ulFBZddPriviligedZones = bits;
        break;
    case 3:
        collisionSet->ulFBZdrPriviligedZones = bits;
        break;
    case 2:
        collisionSet->ulFBZdePriviligedZones = bits;
        break;
    case 1:
        collisionSet->ulFBZdmPriviligedZones = bits;
        break;
    }
}

void CS_fn_vSetPrivilegedActivationZone(u8 _ucZoneType, CollisionSet* _hCollSet, s16 _wIndex,
                                                 u8 _ePrivilege)
{
    s16 shift = _wIndex * 2;
    u32 bits = CS_fn_ulGetPrivilegedActivationZoneInCollSet(_ucZoneType, _hCollSet);
    s32 value = _ePrivilege;

    switch (value) {
    case 0:
        bits &= ~(3 << shift);
        break;
    case 1:
        bits &= ~(3 << shift);
        bits |= value << shift;
        break;
    case 2:
        bits &= ~(3 << shift);
        bits |= value << shift;
        break;
    }
    CS_fn_vSetPrivilegedActivationZoneInCollSet(_ucZoneType, _hCollSet, bits);
}

s32 CS_fn_eGetPrivilegedActivationZone(u8 _ucZoneType, CollisionSet* _hCollSet, s16 _wIndex)
{
    u32 bits = CS_fn_ulGetPrivilegedActivationZoneInCollSet(_ucZoneType, _hCollSet);
    s16 shift;

    if (bits == 0) {
        return 0;
    }
    shift = _wIndex * 2;
    bits &= 3 << shift;
    switch ((u8) (bits >> shift)) {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
        return 2;
    default:
        return 3;
    }
}
