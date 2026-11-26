#include "common.h"

#include "COL/COL.h"
#include "GAM/GAM.h"
#include "HIE/HIE.h"
#include "MEC/MEC.h"
#include "MTH/MTH.h"
#include "PO/PO.h"
#include "SCT/SCT.h"
#include "FMN/FMN.h"

#define SUPER_OBJECT_BOX_BOUNDS 0x10

void COL_fn_vNewStaticCollisionForCharacter(HIE_tdstSuperObject_* character,
                                             HIE_tdstSuperObject_* sector, void* matrixA,
                                             void* matrixB);
void inv_COL_fn_vNewStaticCollisionRecursive(HIE_tdstSuperObject_* character,
                                             HIE_tdstSuperObject_* node, void* handle,
                                             void* matrixA, void* matrixB, s32 isCamera);

static inline PO_tdstPhysicalObject_* inv_GAM_fn_p_stZdxOwnerOf(HIE_tdstSuperObject_* superObject)
{
    return superObject->hLinkedObject;
}

static inline u8 inv_GAM_fn_ucBoundsFlagsOf(HIE_tdstSuperObject_* superObject)
{
    return superObject->flags;
}

static inline u8 inv_GAM_fn_ucCollisionSetFlag(CollisionSet* collSet, u8 mask)
{
    return collSet->ucCollisionFlag & mask;
}

static inline COL_tdstParallelBox_* inv_GAM_fn_p_stBoundsOf(HIE_tdstSuperObject_* superObject)
{
    return superObject->hCollideBoundingVolume;
}

#define CHARACTER_IS_CAMERA(object) ((object)->h_Cineinfo != NULL)

void fn_vChangeCapabilities(tdstEngineObject_* _p_stObject, u32 _ubf32Capabilities, s32 lValue)
{
    switch (lValue) {
    case 0:
    case 2:
        _p_stObject->h_StandardGame->ubf32Capabilities |= _ubf32Capabilities;
        break;
    case 1:
    case 3:
        _p_stObject->h_StandardGame->ubf32Capabilities = _ubf32Capabilities;
        break;
    case 4:
        _p_stObject->h_StandardGame->ubf32Capabilities =
            _p_stObject->h_StandardGame->ubf32Capabilities -
            (_p_stObject->h_StandardGame->ubf32Capabilities & _ubf32Capabilities);
        break;
    case 5:
        _p_stObject->h_StandardGame->ubf32Capabilities = 0;
        break;
    }
}

s32 inv_COL_fn_lTestCharacterAgainstZdxList(DNM_stMecObstacle* hits,
                                            HIE_tdstSuperObject_* character, void* matrixA,
                                            void* matrixB)
{
    DNM_stMecObstacle* hit;
    s32 i;
    tdstSectInfo_* sectorInfo;
    s32 flags;

    g_NbCollidedEdges = 0;
    if (inv_HIE_fn_p_stGetEngineObject(character)->h_CollSet != NULL && CS_fn_hGetZdxList(1, character) != 0) {
        sectorInfo = inv_HIE_fn_p_stGetEngineObject(character)->h_SectInfo;
        COL_g_stCollisionCaseReal = (COL_tdstCollisionCase_*) hits;
        COL_fn_vNewStaticCollisionForCharacter(character, sectorInfo->hCurrentSector, matrixA,
                                                matrixB);
        inv_COL_RAY_fn_vFlushHitBuffer();
        hit = hits;
        for (i = 0; i < g_NbCollidedEdges; i++, hit++) {
            flags = 1;
            hit->m_stProperties.m_xSlide = 0;
            if (hit->m_eType & 1) {
                flags = 2;
            }
            hit->m_stProperties.m_xRebound = 0;
            if (hit->m_eType & 2) {
                flags |= 4;
            }
            hit->m_eType = flags;
        }
    }
    return g_NbCollidedEdges;
}

void inv_COL_fn_vTriggerCollisionCallback(HIE_tdstSuperObject_* character, void* handle,
                                          HIE_tdstSuperObject_* object, void* matrixA,
                                          void* matrixB)
{
    void* zdx = inv_GAM_fn_p_stZdxOwnerOf(object)->_hCollideSet;
    u8 characterFlags;
    void* matrix;

    if (zdx != NULL) {
        characterFlags = character->flags & 8;
        matrix = object->gu_hGlobalMatrix;
        COL_fn_vCollideDynamicGeomObjWithStaticGeomObj(handle, matrixA, matrixB, zdx, matrix, object, 1, object->flags & 4,
                      characterFlags);
    }
}

void fn_vNewTestStaticCollideFor2Characters(HIE_tdstSuperObject_* hCharacter, void* _hDynamicZdm,
                                            HIE_tdstSuperObject_* hStaticCharacter, void* p_stStartMatrix,
                                            void* p_stEndMatrix, u8 isCamera)
{
    tdstEngineObject_* otherObject = inv_HIE_fn_p_stGetEngineObject(hStaticCharacter);
    s32 test = (otherObject->h_StandardGame->ucMiscFlags >> 2) & 1;
    s32 flags;
    CollisionSet* collSet;
    CollisionSet* otherCollSet;
    s32 level;
    HIE_tdstSuperObject_* mainCharacter;
    s32 setFlags;
    void* bounds;
    void* otherBounds;
    COL_tdstCollideObject_* zdx;
    tdstAChannel_* record;
    HIE_tdstSuperObject_* item;

    if (test != 1) {
        return;
    }
    flags = hStaticCharacter->flags;
    test = (flags & 1) ? test : 0;
    if (test) {
        return;
    }
    if (isCamera != 0) {
        if ((flags >> 18) & 1) {
            return;
        }
    } else if ((flags >> 19) & 1) {
        return;
    }

    collSet = inv_GAM_fn_p_stCollisionSetOf(inv_HIE_fn_p_stGetEngineObject(hCharacter));
    otherCollSet = inv_GAM_fn_p_stCollisionSetOf(otherObject);
    level = collSet->ucCharacterPriority;
    if (otherCollSet != NULL) {
        if (level < otherCollSet->ucCharacterPriority) {
            return;
        }
    } else if (level < 0) {
        return;
    }
    mainCharacter = dword_252618[0];
    if (collSet != NULL) {
        setFlags = collSet->ucCollisionFlag;
        if (setFlags & 2) {
            if (fn_ulStandardGameGetCustomBitsSO(hStaticCharacter) & 8) {
                return;
            }
            setFlags = collSet->ucCollisionFlag;
        }
        if (hStaticCharacter == mainCharacter) {
            if (setFlags & 8) {
                return;
            }
        } else if (setFlags & 4) {
            if (((inv_HIE_fn_p_stGetEngineObject(hStaticCharacter)->h_StandardGame->ucMiscFlags >> 4) & 1) == 0) {
                return;
            }
        }
    }
    if (otherCollSet != NULL) {
        setFlags = otherCollSet->ucCollisionFlag;
        if (setFlags & 2) {
            if (fn_ulStandardGameGetCustomBitsSO(hCharacter) & 8) {
                return;
            }
            setFlags = otherCollSet->ucCollisionFlag;
        }
        if (mainCharacter == hCharacter) {
            if (setFlags & 8) {
                return;
            }
        } else if (setFlags & 4) {
            if (((inv_HIE_fn_p_stGetEngineObject(hCharacter)->h_StandardGame->ucMiscFlags >> 4) & 1) == 0) {
                return;
            }
        }
    }
    bounds = inv_GAM_fn_p_stBoundsOf(hCharacter);
    otherBounds = inv_GAM_fn_p_stBoundsOf(hStaticCharacter);
    if (bounds != NULL && otherBounds != NULL) {
        if (inv_COL_fn_bSphereTouchesBoundingVolume(bounds, p_stEndMatrix, otherBounds, hStaticCharacter->gu_hGlobalMatrix, hStaticCharacter->flags & 4,
                          hCharacter->flags & 8, 0) == 0) {
            return;
        }
    }
    if (otherCollSet != NULL) {
        zdx = fn_hStaticMakeOneObjectFromManyZdx(3, hStaticCharacter);
        if (zdx->xNbPoints > 0) {
            void* matrix = hStaticCharacter->gu_hGlobalMatrix;

            COL_fn_vCollideDynamicGeomObjWithStaticGeomObj(_hDynamicZdm, p_stStartMatrix, p_stEndMatrix, zdx, matrix, hStaticCharacter, 1, hStaticCharacter->flags & 4,
                          hCharacter->flags & 8);
        }
    }
    record = inv_HIE_fn_p_stGetEngineObject(hStaticCharacter)->h_3dData->hFirstActiveChannel;
    if (record == NULL) {
        return;
    }
    do {
        item = record->hZoomSupObject;
        if (item == NULL) {
            item = record->hSupObject;
        }
        if (item->lTypeOfLinkedObject == HIE_TYPE_PHYSICAL_OBJECT_inv && !(item->flags & 2)) {
            inv_COL_fn_vTriggerCollisionCallback(hCharacter, _hDynamicZdm, item, p_stStartMatrix, p_stEndMatrix);
        }
        record = record->p_stNextActiveChannel;
    } while (record != NULL);
}

void COL_fn_vNewStaticCollisionForCharacter(HIE_tdstSuperObject_* _hCharacter,
                                             HIE_tdstSuperObject_* _hSector, void* _p_stStartMatrix,
                                             void* _p_stEndMatrix)
{
    MTH3D_tdstVector_ delta;
    COL_tdstParallelBox_ savedBox;
    void* handle;
    tdstEngineObject_* object;
    CollisionSet* collSet;
    u8 isCamera;
    COL_tdstParallelBox_* box;
    f32 savedRadius;
    f32 dx;
    f32 dy;
    f32 dz;
    f32 length;
    SECT_tdstListOfSOCharacters_* node;
    SECT_tdstSector_* sectorData;
    s32 i;
    HIE_tdstSuperObject_* border;
    SECT_tdstListOfSOSectorsInCollisionInteraction_* borders;

    handle = fn_hMakeOneObjectFromManyZdx(1, _hCharacter);
    savedRadius = 0.0f;
    dx = inv_POS_fn_p_stPosMatrixTranslation(_p_stEndMatrix)->xX - inv_POS_fn_p_stPosMatrixTranslation(_p_stStartMatrix)->xX;
    dy = inv_POS_fn_p_stPosMatrixTranslation(_p_stEndMatrix)->xY - inv_POS_fn_p_stPosMatrixTranslation(_p_stStartMatrix)->xY;
    dz = inv_POS_fn_p_stPosMatrixTranslation(_p_stEndMatrix)->xZ - inv_POS_fn_p_stPosMatrixTranslation(_p_stStartMatrix)->xZ;
    object = inv_HIE_fn_p_stGetEngineObject(_hCharacter);
    collSet = inv_GAM_fn_p_stCollisionSetOf(object);
    isCamera = CHARACTER_IS_CAMERA(object);
    MTH3D_M_vSetXYZ_inv(&delta, dx, dy, dz);
    if (inv_GAM_fn_ucBoundsFlagsOf(_hCharacter) & SUPER_OBJECT_BOX_BOUNDS) {
        box = inv_GAM_fn_p_stBoundsOf(_hCharacter);
        savedBox = *box;
        if (savedRadius < delta.xX) {
            box->stMaxPoint.xX = box->stMaxPoint.xX + delta.xX;
        } else {
            box->stMinPoint.xX = box->stMinPoint.xX + delta.xX;
        }
        if (0.0f < delta.xY) {
            box->stMaxPoint.xY = box->stMaxPoint.xY + delta.xY;
        } else {
            box->stMinPoint.xY = box->stMinPoint.xY + delta.xY;
        }
        if (0.0f < delta.xZ) {
            box->stMaxPoint.xZ = box->stMaxPoint.xZ + delta.xZ;
        } else {
            box->stMinPoint.xZ = box->stMinPoint.xZ + delta.xZ;
        }
    } else {
        box = inv_GAM_fn_p_stBoundsOf(_hCharacter);
        length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(delta.xX, delta.xY, delta.xZ));
        savedRadius = box->stMaxPoint.xX;
        box->stMaxPoint.xX = savedRadius + length;
    }

    if (!inv_GAM_fn_ucCollisionSetFlag(collSet, 2) || !inv_GAM_fn_ucCollisionSetFlag(collSet, 4) ||
        !inv_GAM_fn_ucCollisionSetFlag(collSet, 8)) {
        for (node = inv_HIE_fn_p_stGetSector(_hSector)->stListOfCharacters.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
            if (_hCharacter != node->hPointerOfCharacter) {
                fn_vNewTestStaticCollideFor2Characters(_hCharacter, handle, node->hPointerOfCharacter, _p_stStartMatrix,
                                                       _p_stEndMatrix, isCamera);
            }
        }
        if (!(collSet->ucCollisionFlag & 0x20)) {
            sectorData = inv_HIE_fn_p_stGetSector(_hSector);
            for (i = sectorData->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt, borders = sectorData->stListOfSectorsInCollisionInteraction.hFirstElementOpt; i > 0; i--, borders++) {
                for (node = inv_HIE_fn_p_stGetSector(borders->hPointerOfSectorSO)->stListOfCharacters.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
                    if (_hCharacter != node->hPointerOfCharacter) {
                        fn_vNewTestStaticCollideFor2Characters(_hCharacter, handle, node->hPointerOfCharacter,
                                                               _p_stStartMatrix, _p_stEndMatrix, isCamera);
                    }
                }
            }
        }
    }
    if (!(collSet->ucCollisionFlag & 1) && !SECT_fn_bIsThisSectorVirtual(_hSector)) {
        inv_COL_fn_vNewStaticCollisionRecursive(_hCharacter, _hSector, handle, _p_stStartMatrix, _p_stEndMatrix,
                                                isCamera);
        if (!(collSet->ucCollisionFlag & 0x20)) {
            sectorData = inv_HIE_fn_p_stGetSector(_hSector);
            for (i = sectorData->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt, borders = sectorData->stListOfSectorsInCollisionInteraction.hFirstElementOpt; i > 0; i--, borders++) {
                border = borders->hPointerOfSectorSO;
                if (!SECT_fn_bIsThisSectorVirtual(border)) {
                    inv_COL_fn_vNewStaticCollisionRecursive(_hCharacter, border, handle, _p_stStartMatrix,
                                                            _p_stEndMatrix, isCamera);
                }
            }
        }
    }

    if (inv_GAM_fn_ucBoundsFlagsOf(_hCharacter) & SUPER_OBJECT_BOX_BOUNDS) {
        *inv_GAM_fn_p_stBoundsOf(_hCharacter) = savedBox;
    } else {
        inv_GAM_fn_p_stBoundsOf(_hCharacter)->stMaxPoint.xX = savedRadius;
    }
}

void inv_COL_fn_vNewStaticCollisionRecursive(HIE_tdstSuperObject_* character,
                                             HIE_tdstSuperObject_* node, void* handle,
                                             void* matrixA, void* matrixB, s32 isCamera)
{
    s16 camera = isCamera;
    s32 flags = node->flags;
    u8 nodeFlags;
    COL_tdstParallelBox_* bounds;
    COL_tdstParallelBox_* nodeBounds;
    HIE_tdstSuperObject_* child;

    if (flags & 1) {
        return;
    }
    if (camera != 0) {
        if ((flags >> 18) & 1) {
            return;
        }
    } else if ((flags >> 19) & 1) {
        return;
    }

    nodeFlags = node->flags;
    bounds = inv_GAM_fn_p_stBoundsOf(character);
    if ((((nodeFlags << 4) & 0x100) >> 4) != 0) {
        nodeBounds = inv_GAM_fn_p_stBoundsOf(node);
    } else {
        nodeBounds = inv_GAM_fn_p_stBoundsOf(node);
    }
    if (bounds != NULL && nodeBounds != NULL) {
        if (inv_COL_fn_bSphereTouchesBoundingVolume(bounds, matrixB, nodeBounds, node->gu_hGlobalMatrix, nodeFlags & 4,
                          character->flags & 8, nodeFlags & 0x10) == 0) {
            if (node->flags & 0x200) {
                for (child = node->hFirstElementSta; child != NULL;
                     child = child->hNextBrotherSta) {
                    inv_COL_fn_vNewStaticCollisionRecursive(character, child, handle, matrixA,
                                                            matrixB, camera);
                }
            }
            return;
        }
    }

    if (node->lTypeOfLinkedObject & HIE_TYPE_PHYSICAL_OBJECT_inv) {
        void* zdx = inv_GAM_fn_p_stZdxOwnerOf(node)->_hCollideSet;

        COL_fn_vCollideDynamicGeomObjWithStaticGeomObj(handle, matrixA, matrixB, zdx, node->gu_hGlobalMatrix, node, 0,
                      node->flags & 4, character->flags & 8);
    }
    for (child = node->hFirstElementSta; child != NULL; child = child->hNextBrotherSta) {
        inv_COL_fn_vNewStaticCollisionRecursive(character, child, handle, matrixA, matrixB, camera);
    }
}
