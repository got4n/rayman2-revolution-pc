#include "common.h"

#include "COL/COL.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "PO/PO.h"
#include "SCT/SCT.h"

#include "include_asm.h"
#include "AI/AI.h"
#include "GEO/GEO.h"
#include "MTH/MTH.h"
#include "GMT/GMT.h"

void* ZDX_fn_hGetGeoObjOfTypeAndMatrixFromZdxInfos(HIE_tdstSuperObject_* hCharacter, u8 ucZoneType,
                                                   u8 ucChannelNumber, POS_tdstMatrix_** p_hMatrix);
s32 fn_bIsAllDefaultPrivilegedActivationZoneOfCharacter(u8, HIE_tdstSuperObject_*);
s32 fn_bIsThisZdeInClashWithCharacterZdeOfTypes(void* hGeoObj, POS_tdstMatrix_* p_stMatrix,
                                                 HIE_tdstSuperObject_* hCharacter,
                                                 u16 wMaterialMask);
COL_tdstCollideObject_* fn_hMakeOneObjectFromManyOthers(u8 ucType, HIE_tdstSuperObject_* hChar, void* hObject);
s32 fn_eGetPrivilegedActivationZoneOfCharacter(u8, HIE_tdstSuperObject_*, s16);
tdstZoneSetList_* fn_p_stGetCurrentZoneSetOfType2(u8 ucZoneType, HIE_tdstSuperObject_* hCharacter);
u16 fn_uwGetIndexFromZdxIndexList(tdstZdxIndexList_* p_stZdxIndex);
void fn_vDetectAllObjectsInThisModule(HIE_tdstSuperObject_* hCharacter,
                                      COL_tdstCollideObject_* hModule, POS_tdstMatrix_* p_stMatrix,
                                      HIE_tdstSuperObject_** p_hSuperObjTable,
                                      s32 lMaxNumberOfObjects, s32* p_lCurrentNumberOfObjects);
void fn_vGetAllCharacterInContactWithThisGeoObj(HIE_tdstSuperObject_* hSuperObj,
                                                COL_tdstCollideObject_* hGeoObj,
                                                POS_tdstMatrix_* p_stMatrix,
                                                HIE_tdstSuperObject_** p_hSuperObjTable,
                                                s32 wMaterialMask, s32 lMaxNumberOfObjects,
                                                s32* p_lCurrentNumberOfObjects);
void* fn_hGetGeometricZdxOfTypeAtIndex(u8 ucZoneType, u16 uwIndex, HIE_tdstSuperObject_* hSuperObj);
s32 fn_bIsCharacterInZdd(void* hGeoObj, POS_tdstMatrix_* p_stMatrix,
                         HIE_tdstSuperObject_* hCharacter);

extern int fn_iCmpZoneSetIndex(void*, void*);
extern void qsort(void*, unsigned int, unsigned int, void*);

static inline HIE_tdstSuperObject_* inv_GAM_fn_p_stChannelPrimaryObject(tdstAChannel_* records,
                                                                        u8 channel)
{
    return records != NULL ? records[channel].hSupObject : NULL;
}

static inline HIE_tdstSuperObject_* inv_GAM_fn_p_stChannelSecondaryObject(tdstAChannel_* records,
                                                                          u8 channel)
{
    return records != NULL ? records[channel].hZoomSupObject : NULL;
}

void* fn_hIsThisZoneActive(HIE_tdstSuperObject_* _hCharacter, s32 _ucZoneType,
                                                 s32 _ucZoneNumber)
{
    u8 zoneType;
    s32 index;
    s32 state;
    tdstZoneSetList_* zoneSet;
    tdstZdxIndexList_* p;
    tdstZdxList_* list;
    s32 count;
    s32 i;
    u32 limit;
    char* slot;

    zoneType = (u8) _ucZoneType;
    if (*(u8*) _ucZoneNumber & 0x100) {
        *(u8*) _ucZoneNumber = 0;
    }
    index = _ucZoneNumber & 0xFF;
    state = fn_eGetPrivilegedActivationZoneOfCharacter(zoneType, _hCharacter, index);
    if (state != 1) {
        if (state != 0) {
            goto fail;
        }
        zoneSet = fn_p_stGetCurrentZoneSetOfType2(zoneType, _hCharacter);
        i = 0;
        if (zoneSet == NULL) {
            goto fail;
        }
        p = zoneSet->hZdxIndexList.hFirstElementOpt;
        count = zoneSet->hZdxIndexList.lNumberOfElementsOpt;
        goto test;
    step:
        p++;
        i++;
    test:
        if (i >= count) {
            goto fail;
        }
        limit = index & 0xFFFF;
        if (fn_uwGetIndexFromZdxIndexList(p) < limit) {
            goto step;
        }
        if (limit != fn_uwGetIndexFromZdxIndexList(p)) {
            goto fail;
        }
    }
    list = CS_fn_hGetZdxList(zoneType, _hCharacter);
    slot = (char*) list->hGeoZdxList.hFirstElementOpt + ((u32) index << 2);
    if (*(u8*) slot & 0x100) {
        *(s32*) slot = 0;
    }
    return *(void**) slot;
fail:
    return NULL;
}

GMT_tdstGameMaterial_* fn_hIsTypeExistsInGeoObj(COL_tdstCollideObject_* _hGeoObj, u16 _wMaterialMask)
{
    s16 elementIndex = 0;
    s32 nbElements;
    s32 shapeCount;
    s16 shapeIndex;
    GMT_tdstGameMaterial_* material;

    nbElements = GEO_xGetGeometricObjectNumberOfElementsMax(_hGeoObj);
    for (; elementIndex < nbElements; elementIndex++) {
        switch (GEO_xGetElementType(_hGeoObj, elementIndex)) {
        case 0:
            break;
        case 1:
            material = *(GMT_tdstGameMaterial_**) _hGeoObj->d_stListOfElements[elementIndex];
            if (!GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(material, _wMaterialMask))
                break;
            return material;
        case 7:
            shapeCount = GEO_lGetIndexedSphereNbSpheres(_hGeoObj, elementIndex);
            for (shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++) {
                material = GEO_hGetGameMaterialOfIndexedSphere(_hGeoObj, elementIndex, shapeIndex);
                if (GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(material, _wMaterialMask) != 0)
                    return material;
            }
            break;
        case 8:
            shapeCount = GEO_lGetAlignedBoxNbBoxes(_hGeoObj, elementIndex);
            for (shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++) {
                material = GEO_vGetGameMaterialOfIndexedAlignedBox(_hGeoObj, elementIndex, shapeIndex);
                if (GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(material, _wMaterialMask) != 0)
                    return material;
            }
            break;
        }
    }
    return 0;
}

s32 fn_bIsCharacterInZdd(void* hGeoObj, POS_tdstMatrix_* p_stMatrix,
                         HIE_tdstSuperObject_* hCharacter)
{
    tdstEngineObject_* engineObject;
    POS_tdstMatrix_* matrix;
    f32 inverse[0x60 / 4];
    f32 vertex[4];
    s32 one;
    s32 bResult;

    matrix = p_stMatrix;
    engineObject = hCharacter->hLinkedObject;
    one = 1;
    bResult = 0;
    if (((engineObject->h_StandardGame->ucMiscFlags >> 2) & 1) != one) {
        return bResult;
    }
    POS_fn_vInvertMatrix((POS_tdstMatrix_*) inverse, matrix);
    POS_fn_vMulMatrixVertex((MTH3D_tdstVector_*) vertex, (POS_tdstMatrix_*) inverse,
                             inv_POS_fn_p_stPosMatrixTranslation(hCharacter->gu_hGlobalMatrix));
    bResult = inv_INT_fn_bDetectIntersectPointWithGeomObj((MTH3D_tdstVector_*) vertex, hGeoObj);
    return bResult;
}

tdstZoneSetList_* fn_p_stGetCurrentZoneSetOfType2(u8 ucZoneType, HIE_tdstSuperObject_* hCharacter)
{
    CollisionSet* collisionSet;
    tdstZoneSetList_* result;

    collisionSet = ((tdstEngineObject_*) hCharacter->hLinkedObject)->h_CollSet;
    if (collisionSet != NULL) {
        result = fn_hGetCurrentZoneSetOfType(ucZoneType, collisionSet);
    } else {
        result = NULL;
    }
    return result;
}

void fn_vUpdateCurrentActivationForCharacter(HIE_tdstSuperObject_* _hCharacter)
{
    tdstEngineObject_* engineObject;
    gu_tdst3dData_* data3d;
    CollisionSet* collisionSet;
    s32 index;
    tdstCsaList_* list;
    tdstZoneSetEncapsulation_* first;

    engineObject = _hCharacter->hLinkedObject;
    collisionSet = engineObject->h_CollSet;
    if (collisionSet == NULL) {
        return;
    }

    data3d = engineObject->h_3dData;
    index = ((s32) data3d->h_CurrentState - (s32) data3d->h_Family->hForStateArray.hFirstElementOpt) *
                0x800 >>
            16;

    list = CS_fn_hGetActivationList(0, _hCharacter);
    if (list != NULL) {
        first = list->hZoneSetEncapsulationList.hFirstElementOpt + index;
        fn_vSetCurrentZoneSetOfType(0, collisionSet, first->p_stZoneSetList);
    } else {
        fn_vSetCurrentZoneSetOfType(0, collisionSet, NULL);
    }

    list = CS_fn_hGetActivationList(2, _hCharacter);
    if (list != NULL) {
        first = list->hZoneSetEncapsulationList.hFirstElementOpt + index;
        fn_vSetCurrentZoneSetOfType(2, collisionSet, first->p_stZoneSetList);
    } else {
        fn_vSetCurrentZoneSetOfType(2, collisionSet, NULL);
    }

    list = CS_fn_hGetActivationList(3, _hCharacter);
    if (list != NULL) {
        first = list->hZoneSetEncapsulationList.hFirstElementOpt + index;
        fn_vSetCurrentZoneSetOfType(3, collisionSet, first->p_stZoneSetList);
    } else {
        fn_vSetCurrentZoneSetOfType(3, collisionSet, NULL);
    }

    list = CS_fn_hGetActivationList(1, _hCharacter);
    if (list != NULL) {
        first = list->hZoneSetEncapsulationList.hFirstElementOpt + index;
        fn_vSetCurrentZoneSetOfType(1, collisionSet, first->p_stZoneSetList);
    } else {
        fn_vSetCurrentZoneSetOfType(1, collisionSet, NULL);
    }
}

s32 fn_iCmpZoneSetIndex(void* p_stZSE1, void* p_stZSE2)
{
    return ((tdstZoneSetEncapsulation_*) p_stZSE1)->uwIndex -
           ((tdstZoneSetEncapsulation_*) p_stZSE2)->uwIndex;
}

void loc_114618(tdstCsaList_* p_stCsaList)
{
    qsort(p_stCsaList->hZoneSetEncapsulationList.hFirstElementOpt,
          p_stCsaList->hZoneSetEncapsulationList.lNumberOfElementsOpt,
          sizeof(tdstZoneSetEncapsulation_), &fn_iCmpZoneSetIndex);
}

void fn_bHadToCorrectActivationList(tdstCsaList_* p_stCsaList, s32 context)
{
    tdstZoneSetEncapsulation_* p_stZSE;
    tdstZoneSetEncapsulation_* p_stZSE2;
    u16 uwPosition;
    s32 bResult;

    bResult = 0;
    p_stZSE = p_stCsaList->hZoneSetEncapsulationList.hFirstElementOpt;
    p_stZSE2 = p_stZSE + p_stCsaList->hZoneSetEncapsulationList.lNumberOfElementsOpt - 1;
    uwPosition = 0;
    if (p_stZSE < p_stZSE2) {
        do {
            if (uwPosition < p_stZSE->uwIndex) {
                p_stZSE2->uwIndex = uwPosition;
                bResult = -1;
                p_stZSE2 = p_stZSE - 1;
            } else {
                p_stZSE++;
            }
            uwPosition = uwPosition + 1;
        } while (p_stZSE < p_stZSE2);
    }
    if (bResult != 0) {
        loc_114618(p_stCsaList);
    }
}

u16 fn_uwGetIndexFromZdxIndexList(tdstZdxIndexList_* p_stZdxIndex)
{
    return p_stZdxIndex->uwZdxIndex;
}

void* fn_hGetGeometricZdxOfTypeAtIndex(u8 ucZoneType, u16 uwIndex, HIE_tdstSuperObject_* hSuperObj)
{
    tdstZdxList_* list;
    tdstCollideZdxList_* first;
    tdstCollideZdxList_* slot;

    list = CS_fn_hGetZdxList(ucZoneType, hSuperObj);
    first = list->hGeoZdxList.hFirstElementOpt;
    slot = &first[uwIndex];
    return slot->hGeoObj;
}

void* func_001146E8(void)
{
    return AI_fn_p_vTrueAlloc(gp_6F74, 0x8);
}

void fn_vGetAllCharactersInContactWithThisZde(HIE_tdstSuperObject_* _hSuperObj, s32 ucChannelNumber,
                                              HIE_tdstSuperObject_** _hSuperObjTable,
                                              u16* _p_uwNumber, u16 _wMaterialMask)
{
    u16 lMaxNumberOfObjects;
    void* hGeoObj;
    s32 lCurrentNumberOfObjects;

    lMaxNumberOfObjects = *_p_uwNumber;
    lCurrentNumberOfObjects = 0;
    hGeoObj =
        fn_hIsThisZoneActive(_hSuperObj, 2, (ucChannelNumber - 1) & 0xFF);
    if (hGeoObj != NULL) {
        fn_vGetAllCharacterInContactWithThisGeoObj(_hSuperObj, hGeoObj, _hSuperObj->gu_hGlobalMatrix,
                                                   _hSuperObjTable, _wMaterialMask,
                                                   lMaxNumberOfObjects, &lCurrentNumberOfObjects);
    }
    *_p_uwNumber = (u16) lCurrentNumberOfObjects;
}

void fn_vGetAllCharacterInContactWithThisGeoObj(HIE_tdstSuperObject_* _hSuperObj,
                                                COL_tdstCollideObject_* _hGeoObj,
                                                POS_tdstMatrix_* p_stMatrix,
                                                HIE_tdstSuperObject_** _hSuperObjTable,
                                                s32 _lMaxNumberOfObjects, s32 lMaxNumberOfObjects,
                                                s32* _p_lCurrentNumberOfObjects)
{
    u16 flags = (u16) _lMaxNumberOfObjects;
    volatile s32* counter = _p_lCurrentNumberOfObjects;
    StandardGameList* list;
    gu_tdstStandardGame* node;
    HIE_tdstSuperObject_* ch;
    tdstEngineObject_* engineObject;
    tdstEngineObject_* info;
    s32 flag40;
    s32 flag8;

    engineObject = inv_HIE_fn_p_stGetEngineObject(_hSuperObj);
    flag8 = engineObject->h_StandardGame->ulCustomBits & 8;
    flag40 = engineObject->h_CollSet->ucCollisionFlag & 0x40;
    list = inv_GAM_currentCharacterList;
    do {
        node = list->first;
        if (node != 0 && *counter < lMaxNumberOfObjects) {
            s32 one;
            one = 1;
            do {
                if (node || ch) {
                    ch = node->h_SuperObject;
                } else {
                    ch = node->h_SuperObject;
                }
                if (ch != _hSuperObj) {
                    info = inv_HIE_fn_p_stGetEngineObject(ch);
                    if (((info->h_StandardGame->ucMiscFlags >> 2) & 1) == one) {
                        if (flag8 == 0 || info->h_CollSet == 0 ||
                            ((info->h_CollSet->ucCollisionFlag & 0x40) == 0)) {
                            if (flag40 == 0 ||
                                (((*(gu_tdstStandardGame* volatile*) &info->h_StandardGame)
                                      ->ulCustomBits &
                                  8) == 0)) {
                                if (fn_bIsThisZdeInClashWithCharacterZdeOfTypes(
                                        _hGeoObj, p_stMatrix, ch, flags) != 0)
                                    _hSuperObjTable[(*counter)++] = ch;
                            }
                        }
                    }
                }
                node = node->listNext;
                if (node == 0)
                    break;
            } while (*counter < lMaxNumberOfObjects);
        }
        if (list == inv_GAM_previousCharacterList)
            break;
        list = inv_GAM_previousCharacterList;
    } while (1);
}

void fn_vGetAllCharactersInContactWithThisModuleZde(HIE_tdstSuperObject_* _hSuperObj, s32 _ucChannelNumber, HIE_tdstSuperObject_** _hSuperObjTable,
                   u16 _wMaterialMask, s32 _lMaxNumberOfObjects, s32* _p_lCurrentNumberofObjects)
{
    HIE_tdstSuperObject_* module;
    void* geometricObject;

    module = inv_GAM_fn_p_stChannelPrimaryObject(
        ((tdstEngineObject_*) _hSuperObj->hLinkedObject)->h_3dData->hArrayOfChannels, _ucChannelNumber);
    if (module == NULL || module->lTypeOfLinkedObject != HIE_TYPE_PHYSICAL_OBJECT_inv) {
        module = inv_GAM_fn_p_stChannelSecondaryObject(
            ((tdstEngineObject_*) _hSuperObj->hLinkedObject)->h_3dData->hArrayOfChannels, _ucChannelNumber);
        if (module == NULL || module->lTypeOfLinkedObject != HIE_TYPE_PHYSICAL_OBJECT_inv) {
            goto done;
        }
    }
    geometricObject = ((PO_tdstPhysicalObject_*) module->hLinkedObject)->_hCollideSet;
    if (geometricObject != NULL) {
        fn_vGetAllCharacterInContactWithThisGeoObj(_hSuperObj, geometricObject,
                                                   module->gu_hGlobalMatrix, _hSuperObjTable, _wMaterialMask,
                                                   _lMaxNumberOfObjects, _p_lCurrentNumberofObjects);
    }
done:;
}

s32 fn_bIsThisZdeInClashWithCharacterZdeOfTypes(void* _hGeoObj, POS_tdstMatrix_* _hMatrix,
                                                 HIE_tdstSuperObject_* _hCharacter, u16 _wMaterialMask)
{
    GMT_tdstGameMaterial_* ulFirstMaterial;
    GMT_tdstGameMaterial_* ulSecondMaterial;
    tdstZoneSetList_* set;
    s32 cur;
    tdstZdxIndexList_* p;
    s32 nextListed;
    s32 first;
    s32 accum;
    s32 accTmp;
    s32 accTmp2;
    s32 step;
    void* geo;
    s32 bit;
    tdstAChannel_* link;
    HIE_tdstSuperObject_* node;

    bit = (((tdstEngineObject_*) _hCharacter->hLinkedObject)->h_StandardGame->ucMiscFlags >> 2) & 1;
    if (bit == 1) {
        ulFirstMaterial = 0;
        ulSecondMaterial = 0;
        cur = 0;
        set = fn_p_stGetCurrentZoneSetOfType2(2, _hCharacter);
        if (fn_bIsAllDefaultPrivilegedActivationZoneOfCharacter(2, _hCharacter)) {
            if (set != 0) {
                p = set->hZdxIndexList.hFirstElementOpt;
                if (set->hZdxIndexList.lNumberOfElementsOpt > 0) {
                    accum = 0x10000;
                    step = 0x10000;
                    do {
                        geo = fn_hGetGeometricZdxOfTypeAtIndex(2, fn_uwGetIndexFromZdxIndexList(p),
                                                               _hCharacter);
                        if (geo != 0 && (_wMaterialMask == 0 || fn_hIsTypeExistsInGeoObj(geo, _wMaterialMask))) {
                            if (INT_fn_bDetectIntersectGeomObjWithGeomObj(
                                    _hGeoObj, _hMatrix, geo, _hCharacter->gu_hGlobalMatrix, &ulFirstMaterial, &ulSecondMaterial)) {
                                if (_wMaterialMask == 0 ||
                                    GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(ulSecondMaterial,
                                                                                           _wMaterialMask)) {
                                    return 1;
                                }
                            }
                        }
                        accTmp = accum;
                        accum += step;
                        cur = accTmp >> 16;
                        p++;
                    } while (cur < set->hZdxIndexList.lNumberOfElementsOpt);
                }
            }
        } else {
            p = 0;
            if (set != 0)
                p = set->hZdxIndexList.hFirstElementOpt;
            if (p != 0)
                first = fn_uwGetIndexFromZdxIndexList(p);
            else
                first = 0xFFFF;
            accum = (cur << 16) + 0x10000;
            step = 0x10000;
            nextListed = first;
            while (cur < fn_lGetZdxListNumberOfElements(2, _hCharacter)) {
                s32 state;
                                state = fn_eGetPrivilegedActivationZoneOfCharacter(2, _hCharacter, cur);
                if (state == 1 || (state == 0 && nextListed == cur)) {
                    geo = fn_hGetGeometricZdxOfTypeAtIndex(2, (u16) cur, _hCharacter);
                    if (geo != 0 && (_wMaterialMask == 0 || fn_hIsTypeExistsInGeoObj(geo, _wMaterialMask))) {
                        if (INT_fn_bDetectIntersectGeomObjWithGeomObj(
                                _hGeoObj, _hMatrix, geo, _hCharacter->gu_hGlobalMatrix, &ulFirstMaterial, &ulSecondMaterial)) {
                            if (_wMaterialMask == 0 || GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(
                                                 ulSecondMaterial, _wMaterialMask)) {
                                return 1;
                            }
                        }
                    }
                }
                accTmp2 = accum;
                accum += step;
                cur = accTmp2 >> 16;
                if (p != 0 && nextListed < cur) {
                    s32 newIndex;
                    p++;
                    if (p != 0)
                        newIndex = fn_uwGetIndexFromZdxIndexList(p);
                    else
                        newIndex = 0xFFFF;
                    nextListed = newIndex;
                }
            }
        }

        link = (tdstAChannel_*) ((tdstEngineObject_*) _hCharacter->hLinkedObject)
                   ->h_3dData->hFirstActiveChannel;
        if (link != 0) {
            do {
                node = link->hZoomSupObject;
                if (node == 0)
                    node = link->hSupObject;
                if ((node->lTypeOfLinkedObject & HIE_TYPE_PHYSICAL_OBJECT_inv) != 0) {
                    geo = ((PO_tdstPhysicalObject_*) node->hLinkedObject)->_hCollideSet;
                    if (geo != 0 && (_wMaterialMask == 0 || fn_hIsTypeExistsInGeoObj(geo, _wMaterialMask))) {
                        if (INT_fn_bDetectIntersectGeomObjWithGeomObj(
                                _hGeoObj, _hMatrix, geo, node->gu_hGlobalMatrix, &ulFirstMaterial, &ulSecondMaterial)) {
                            if (_wMaterialMask == 0 || GMT_fn_xMultipleTestCollideIdentifierFromGameMaterial(
                                                 ulSecondMaterial, _wMaterialMask)) {
                                return 1;
                            }
                        }
                    }
                }
                link = link->p_stNextActiveChannel;
            } while (link != 0);
        }
    }
    return 0;
}

void* ZDX_fn_hGetGeoObjOfTypeAndMatrixFromZdxInfos(HIE_tdstSuperObject_* _hCharacter, u8 _ucZoneType,
                                                   u8 _ucModuleOrCharacter, POS_tdstMatrix_** p_hMatrix)
{
    void* hGeoObj;

    hGeoObj = fn_hIsThisZoneActive(_hCharacter, _ucZoneType,
                                                         (_ucModuleOrCharacter - 1) & 0xFF);
    if (hGeoObj != NULL) {
        *p_hMatrix = _hCharacter->gu_hGlobalMatrix;
        return hGeoObj;
    }
    return NULL;
}

s32 fn_bThisZdeFromOneVsAllZdeFromTwo(HIE_tdstSuperObject_* _hCharacterONE, u8 _ucModuleOrCharacter,
                                      HIE_tdstSuperObject_* _hCharacterTWO, u16 _wMaterialMask)
{
    POS_tdstMatrix_* p_stMatrix;
    void* hGeoObj;

    hGeoObj = ZDX_fn_hGetGeoObjOfTypeAndMatrixFromZdxInfos(_hCharacterONE, 2, _ucModuleOrCharacter,
                                                           &p_stMatrix);
    if (hGeoObj == NULL) {
        return 0;
    }
    return fn_bIsThisZdeInClashWithCharacterZdeOfTypes(hGeoObj, p_stMatrix, _hCharacterTWO,
                                                        _wMaterialMask);
}

s32 fn_bThisZdeFromOneVsThisZdeFromTwo(HIE_tdstSuperObject_* _hCharacterONE,
                                       u8 _ucModuleOrCharacterForONE,
                                       HIE_tdstSuperObject_* _hCharacterTWO,
                                       u8 _ucModuleOrCharacterForTWO)
{
    POS_tdstMatrix_* p_stMatrix1;
    POS_tdstMatrix_* p_stMatrix2;
    GMT_tdstGameMaterial_* ulFirstMaterial;
    GMT_tdstGameMaterial_* ulSecondMaterial;
    void* hGeoObj1;
    s32 geometry2OrResult;
    s32 index2;

    index2 = _ucModuleOrCharacterForTWO;
    hGeoObj1 = ZDX_fn_hGetGeoObjOfTypeAndMatrixFromZdxInfos(_hCharacterONE, 2, _ucModuleOrCharacterForONE,
                                                            &p_stMatrix1);
    geometry2OrResult =
        (s32) ZDX_fn_hGetGeoObjOfTypeAndMatrixFromZdxInfos(_hCharacterTWO, 2, index2, &p_stMatrix2);
    if (hGeoObj1 == NULL) {
        return 0;
    }
    if (geometry2OrResult == 0) {
        return 0;
    }
    geometry2OrResult = INT_fn_bDetectIntersectGeomObjWithGeomObj(
        hGeoObj1, p_stMatrix1, (void*) geometry2OrResult, p_stMatrix2, &ulFirstMaterial, &ulSecondMaterial);
    return (u32) geometry2OrResult > 0u;
}

static inline SECT_tdstSector_* inv_GAM_fn_p_stCharacterSector(HIE_tdstSuperObject_* hCharacter)
{
    return inv_HIE_fn_p_stGetSector(
        inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_SectInfo->hCurrentSector);
}

static inline u32 inv_GAM_fn_bIsCharacterActive(HIE_tdstSuperObject_* hCharacter)
{
    return (inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_StandardGame->ucMiscFlags >> 2) & 1;
}

void fn_vThisTypeVsThisType(HIE_tdstSuperObject_* _hCharacter, u16 _wCharacterMask, u16 _wOthersMask,
                            HIE_tdstSuperObject_** _p_hObjectTable, u16* _p_uwNumber)
{
    HIE_tdstSuperObject_* obj;
    SECT_tdstListOfSOCharacters_* node;
    SECT_tdstListOfSOSectorsInCollisionInteraction_* groups;
    s32 groupCount;
    u16 uwMaxNumber;
    u16 uwCurrentNumber;

    uwMaxNumber = *_p_uwNumber;
    uwCurrentNumber = 0;

    node = inv_GAM_fn_p_stCharacterSector(_hCharacter)->stListOfCharacters.hFirstElementDyn;

    if (node != 0) {
        s32 active = 1;
        do {
            obj = node->hPointerOfCharacter;
            if (_hCharacter != obj) {
                u32 flag = inv_GAM_fn_bIsCharacterActive(obj);
                if (flag == active) {
                    if (uwCurrentNumber >= uwMaxNumber) {
                        *_p_uwNumber = uwMaxNumber;
                        return;
                    }
                    if (fn_bThisTypeFromOneVsThisTypeFromTwo(_hCharacter, _wCharacterMask, obj,
                                                             _wOthersMask)) {
                        _p_hObjectTable[uwCurrentNumber++] = obj;
                    }
                }
            }
            node = node->hNextBrotherDyn;
        } while (node != 0);
    }

    {
        SECT_tdstSector_* sector = inv_GAM_fn_p_stCharacterSector(_hCharacter);
        groupCount = sector->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt;
        groups = sector->stListOfSectorsInCollisionInteraction.hFirstElementOpt;
    }

    if (groupCount != 0) {
        do {
            node = inv_HIE_fn_p_stGetSector(groups->hPointerOfSectorSO)->stListOfCharacters.hFirstElementDyn;
            if (node != 0) {
                s32 active = 1;
                do {
                    if (uwCurrentNumber >= uwMaxNumber) {
                        *_p_uwNumber = uwMaxNumber;
                        return;
                    }
                    obj = node->hPointerOfCharacter;
                    {
                        u32 flag = inv_GAM_fn_bIsCharacterActive(obj);
                        if (flag == active) {
                            if (fn_bThisTypeFromOneVsThisTypeFromTwo(_hCharacter, _wCharacterMask,
                                                                     obj, _wOthersMask)) {
                                _p_hObjectTable[uwCurrentNumber++] = obj;
                            }
                        }
                    }
                    node = node->hNextBrotherDyn;
                } while (node != 0);
            }
            groupCount--;
            groups++;
        } while (groupCount != 0);
    }

    *_p_uwNumber = uwCurrentNumber;
}

s32 fn_bThisTypeFromOneVsThisTypeFromTwo(HIE_tdstSuperObject_* _hCharacterONE, u16 _wMaterialMaskOfONE,
                                         HIE_tdstSuperObject_* _hCharacterTWO, u16 _wMaterialMaskOfTWO)
{
    tdstZoneSetList_* set;
    void* geo;
    tdstZdxIndexList_* p_stZdxIndex;
    s32 cur;
    s32 accum;
    s32 accTmp;
    s32 accTmp2;
    s32 first;
    s32 nextListed;
    s32 newIndex;
    s32 state;
    tdstAChannel_* link;
    HIE_tdstSuperObject_* node;

    node = 0;
    cur = 0;
    set = fn_p_stGetCurrentZoneSetOfType2(2, _hCharacterONE);
    if (fn_bIsAllDefaultPrivilegedActivationZoneOfCharacter(2, _hCharacterONE)) {
        if (set != 0) {
            p_stZdxIndex = set->hZdxIndexList.hFirstElementOpt;
            if (set->hZdxIndexList.lNumberOfElementsOpt > 0) {
                accum = 0x10000;
                do {
                    geo =
                        fn_hGetGeometricZdxOfTypeAtIndex(2, fn_uwGetIndexFromZdxIndexList(p_stZdxIndex), _hCharacterONE);
                    if (geo != 0 && fn_hIsTypeExistsInGeoObj(geo, _wMaterialMaskOfONE) != 0 &&
                        fn_bIsThisZdeInClashWithCharacterZdeOfTypes(geo, _hCharacterONE->gu_hGlobalMatrix,
                                                                     _hCharacterTWO, _wMaterialMaskOfTWO) != 0) {
                        return 1;
                    }
                    p_stZdxIndex++;
                    accTmp = accum;
                    accum += 0x10000;
                    cur = accTmp >> 16;
                } while (cur < set->hZdxIndexList.lNumberOfElementsOpt);
            }
        }
    } else {
        p_stZdxIndex = 0;
        if (set != 0)
            p_stZdxIndex = set->hZdxIndexList.hFirstElementOpt;
        if (p_stZdxIndex != 0)
            first = fn_uwGetIndexFromZdxIndexList(p_stZdxIndex);
        else
            first = 0xFFFF;
        accum = (cur << 16) + 0x10000;
        nextListed = first;
        while (cur < fn_lGetZdxListNumberOfElements(2, _hCharacterONE)) {
            state = fn_eGetPrivilegedActivationZoneOfCharacter(2, _hCharacterONE, (s16) cur);
            if (state == 1 || (state == 0 && nextListed == cur)) {
                geo = fn_hGetGeometricZdxOfTypeAtIndex(2, (u16) cur, _hCharacterONE);
                if (geo != 0 && fn_hIsTypeExistsInGeoObj(geo, _wMaterialMaskOfONE) != 0 &&
                    fn_bIsThisZdeInClashWithCharacterZdeOfTypes(geo, _hCharacterONE->gu_hGlobalMatrix, _hCharacterTWO,
                                                                 _wMaterialMaskOfTWO) != 0) {
                    return 1;
                }
            }
            accTmp2 = accum;
            accum += 0x10000;
            cur = accTmp2 >> 16;
            if (p_stZdxIndex != 0 && nextListed < cur) {
                p_stZdxIndex++;
                if (p_stZdxIndex != 0)
                    newIndex = fn_uwGetIndexFromZdxIndexList(p_stZdxIndex);
                else
                    newIndex = 0xFFFF;
                nextListed = newIndex;
            }
        }
    }

    link =
        (tdstAChannel_*) ((tdstEngineObject_*) _hCharacterONE->hLinkedObject)->h_3dData->hFirstActiveChannel;
    if (link != 0) {
        do {
            if (node != 0) {
                node = link->hZoomSupObject;
            }
            if (node == 0) {
                node = link->hSupObject;
            }
            if ((node->lTypeOfLinkedObject & HIE_TYPE_PHYSICAL_OBJECT_inv) != 0) {
                geo = ((PO_tdstPhysicalObject_*) node->hLinkedObject)->_hCollideSet;
                if (geo != 0 && fn_hIsTypeExistsInGeoObj(geo, _wMaterialMaskOfONE) != 0 &&
                    fn_bIsThisZdeInClashWithCharacterZdeOfTypes(geo, node->gu_hGlobalMatrix, _hCharacterTWO,
                                                                 _wMaterialMaskOfTWO) != 0) {
                    return 1;
                }
            }
            link = link->p_stNextActiveChannel;
        } while (link != 0);
    }
    return 0;
}

s32 fn_bIsCharacterTWODetectedByThisZddOfCharacterOne(HIE_tdstSuperObject_* _hCharacterONE,
                                                      s32 ucChannelNumber,
                                                      HIE_tdstSuperObject_* _hCharacterTWO)
{
    POS_tdstMatrix_* p_stMatrix;
    void* hGeoObj;
    POS_tdstMatrix_* savedMatrix;

    hGeoObj = ZDX_fn_hGetGeoObjOfTypeAndMatrixFromZdxInfos(_hCharacterONE, 0, ucChannelNumber & 0xFF,
                                                           &p_stMatrix);
    savedMatrix = p_stMatrix;
    if (hGeoObj == NULL) {
        return 0;
    }
    return fn_bIsCharacterInZdd(hGeoObj, savedMatrix, _hCharacterTWO);
}

s32 fn_bIsCharacterTWODetectedByCharacterOne(HIE_tdstSuperObject_* _hCharacterONE,
                                             HIE_tdstSuperObject_* _hCharacterTWO)
{
    tdstZoneSetList_* set;
    tdstZdxIndexList_* p;
    s32 cur;
    s32 accum;
    s32 snap;
    s32 first;
    s32 nextListed;
    s32 newIndex;
    s32 state;
    s32 step;
    void* geo;
    s32 yes;

    cur = 0;
    p = 0;
    set = fn_p_stGetCurrentZoneSetOfType2(0, _hCharacterONE);
    if (set != 0)
        p = set->hZdxIndexList.hFirstElementOpt;
    if (p != 0)
        first = fn_uwGetIndexFromZdxIndexList(p);
    else
        first = 0xFFFF;
    accum = (cur << 16) + 0x10000;
    nextListed = first;
    yes = 1;
    step = 0x10000;
    while (cur < fn_lGetZdxListNumberOfElements(0, _hCharacterONE)) {
        state = fn_eGetPrivilegedActivationZoneOfCharacter(0, _hCharacterONE, (s16) cur);
        if (state == yes || (state == 0 && nextListed == cur)) {
            geo = fn_hGetGeometricZdxOfTypeAtIndex(0, (u16) cur, _hCharacterONE);
            if (geo != 0 && fn_bIsCharacterInZdd(geo, _hCharacterONE->gu_hGlobalMatrix, _hCharacterTWO) != 0)
                return 1;
        }
        snap = accum;
        accum += step;
        cur = snap >> 16;
        if (p != 0 && nextListed < cur) {
            p++;
            if (p != 0)
                newIndex = fn_uwGetIndexFromZdxIndexList(p);
            else
                newIndex = 0xFFFF;
            nextListed = newIndex;
        }
    }
    return 0;
}

void fn_vDetectAllObjectsInThisModule(HIE_tdstSuperObject_* _hCharacter,
                                      COL_tdstCollideObject_* _hModule, POS_tdstMatrix_* p_stMatrix,
                                      HIE_tdstSuperObject_** _p_hSuperObjTable,
                                      s32 _lMaxNumberOfObjects, s32* _lCurrentNumberOfObjects)
{
    SECT_tdstListOfSOCharacters_* link;
    SECT_tdstListOfSOSectorsInActivityInteraction_* array;
    HIE_tdstSuperObject_* obj;
    s32 idx;
    s32 n;
    s32 one;

    link = inv_GAM_fn_p_stCharacterSector(_hCharacter)->stListOfCharacters.hFirstElementDyn;
    if (link != NULL) {
        one = 1;
        while (link != NULL) {
            obj = link->hPointerOfCharacter;
            if (_hCharacter != obj && inv_GAM_fn_bIsCharacterActive(obj) == one &&
                fn_bIsCharacterInZdd(_hModule, p_stMatrix, obj) != 0) {
                idx = *_lCurrentNumberOfObjects;
                if (idx >= _lMaxNumberOfObjects)
                    return;
                _p_hSuperObjTable[idx] = obj;
                *_lCurrentNumberOfObjects = idx + 1;
            }
            link = link->hNextBrotherDyn;
        }
    }

    if (*_lCurrentNumberOfObjects < _lMaxNumberOfObjects) {
        n = inv_GAM_fn_p_stCharacterSector(_hCharacter)->stListOfSectorsInActivityInteraction.lNumberOfElementsOpt;
        array = inv_GAM_fn_p_stCharacterSector(_hCharacter)->stListOfSectorsInActivityInteraction.hFirstElementOpt;
        if (n > 0) {
            do {
                if (*_lCurrentNumberOfObjects >= _lMaxNumberOfObjects)
                    return;
                link = inv_HIE_fn_p_stGetSector(array->hPointerOfSectorSO)->stListOfCharacters.hFirstElementDyn;
                if (link != NULL) {
                inner_loop:
                    do {
                        obj = link->hPointerOfCharacter;
                        {
                            s8 detected = inv_GAM_fn_bIsCharacterActive(obj);
                            if (detected == 1 &&
                                fn_bIsCharacterInZdd(_hModule, p_stMatrix, obj) != 0) {
                                idx = *_lCurrentNumberOfObjects;
                                if (idx >= _lMaxNumberOfObjects)
                                    break;
                                _p_hSuperObjTable[idx] = obj;
                                *_lCurrentNumberOfObjects = idx + 1;
                            }
                            link = link->hNextBrotherDyn;
                            if (link != NULL)
                                goto inner_loop;
                        }
                    } while (0);
                }
                n--;
                array++;
            } while (n > 0);
        }
    }
}

void fn_vGetAllObjectsInZdd(HIE_tdstSuperObject_* _hSuperObj, s32 ucChannelNumber,
                            HIE_tdstSuperObject_** _p_hSuperObjTable, u16* _p_uwNumber)
{
    s32 lCurrentNumberOfObjects;
    void* hGeoObj;
    u16 lMaxNumberOfObjects;

    ucChannelNumber = (ucChannelNumber - 1) & 0xFF;
    lMaxNumberOfObjects = *_p_uwNumber;
    lCurrentNumberOfObjects = 0;
    hGeoObj = fn_hIsThisZoneActive(_hSuperObj, 0, (u8) ucChannelNumber);
    if (hGeoObj != NULL) {
        fn_vDetectAllObjectsInThisModule(_hSuperObj, hGeoObj, _hSuperObj->gu_hGlobalMatrix,
                                         _p_hSuperObjTable, lMaxNumberOfObjects,
                                         &lCurrentNumberOfObjects);
    }
    *_p_uwNumber = (u16) lCurrentNumberOfObjects;
}

COL_tdstCollideObject_* fn_hMakeOneObjectFromManyZdx(s32 _ucZoneType, HIE_tdstSuperObject_* _hCharacter)
{
    return fn_hMakeOneObjectFromManyOthers(_ucZoneType & 0xFF, _hCharacter, g_hCollisionGeoObj);
}

COL_tdstCollideObject_* fn_hStaticMakeOneObjectFromManyZdx(s32 _ucZoneType, HIE_tdstSuperObject_* _hCharacter)
{
    return fn_hMakeOneObjectFromManyOthers(_ucZoneType & 0xFF, _hCharacter, g_hStaticCollisionGeoObj);
}

COL_tdstCollideObject_* fn_hMakeOneObjectFromManyOthers(u8 _ucZoneType, HIE_tdstSuperObject_* _hCharacter,
                                    void* _hTargetObject)
{
    COL_tdstCollideObject_* a_hObj[12];
    s16 wNb;
    s16 i;
    tdstZoneSetList_* zs;
    tdstZdxIndexList_* p;
    u16 idx;
    void* hZdx;
    s32 eActivation;

    wNb = 0;
    zs = fn_p_stGetCurrentZoneSetOfType2(_ucZoneType, _hCharacter);
    if (fn_bIsAllDefaultPrivilegedActivationZoneOfCharacter(_ucZoneType, _hCharacter)) {
        if (zs != NULL) {
            p = zs->hZdxIndexList.hFirstElementOpt;
            for (i = 0; i < zs->hZdxIndexList.lNumberOfElementsOpt; i++, p++) {
                hZdx = fn_hGetGeometricZdxOfTypeAtIndex(_ucZoneType, fn_uwGetIndexFromZdxIndexList(p),
                                                     _hCharacter);
                if (hZdx != NULL)
                    a_hObj[wNb++] = hZdx;
            }
        }
    } else {
        i = 0;
        p = NULL;
        if (zs != NULL)
            p = zs->hZdxIndexList.hFirstElementOpt;
        idx = (p != NULL) ? fn_uwGetIndexFromZdxIndexList(p) : 0xFFFF;
        while (i < fn_lGetZdxListNumberOfElements(_ucZoneType, _hCharacter)) {
            eActivation = fn_eGetPrivilegedActivationZoneOfCharacter(_ucZoneType, _hCharacter, i);
            if (eActivation == 1 || (eActivation == 0 && idx == i)) {
                hZdx = fn_hGetGeometricZdxOfTypeAtIndex(_ucZoneType, i, _hCharacter);
                if (hZdx != NULL)
                    a_hObj[wNb++] = hZdx;
            }
            i++;
            if (p != NULL && idx < i) {
                p++;
                idx = (p != NULL) ? fn_uwGetIndexFromZdxIndexList(p) : 0xFFFF;
            }
        }
    }
    GEO_fn_hTransformTableOfObjectInOneObjectForGAM(_hTargetObject, a_hObj, wNb);
    return _hTargetObject;
}

s32 fn_eGetPrivilegedActivationZoneOfCharacter(u8 a0, HIE_tdstSuperObject_* superObject, s16 a2)
{
    CollisionSet* obj;

    obj = ((tdstEngineObject_*) superObject->hLinkedObject)->h_CollSet;
    if (obj != 0) {
        return CS_fn_eGetPrivilegedActivationZone(a0, obj, a2);
    }
    return 3;
}

s32 fn_bIsAllDefaultPrivilegedActivationZoneOfCharacter(u8 a0, HIE_tdstSuperObject_* superObject)
{
    CollisionSet* obj;

    obj = ((tdstEngineObject_*) superObject->hLinkedObject)->h_CollSet;
    if (obj != 0) {
        return (u32) CS_fn_ulGetPrivilegedActivationZoneInCollSet(a0, obj) < 1u;
    }
    return 1;
}

s32 fn_lGetZdxListNumberOfElements(u8 ucZoneType, HIE_tdstSuperObject_* hSuperObj)
{
    tdstZdxList_* hZdxList = CS_fn_hGetZdxList(ucZoneType, hSuperObj);
    if (hZdxList != NULL) {
        return hZdxList->hGeoZdxList.lNumberOfElementsOpt;
    }
    return 0;
}
