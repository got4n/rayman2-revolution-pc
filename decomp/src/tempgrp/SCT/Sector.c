#include "common.h"

#include "SCT/SCT.h"
#include "AI/AI.h"
#include "GAM/GAM.h"

extern f32 flt_3935B0[];
extern s32 D_003935B4;

void SECT_fn_vGetCenterOfBorder(SECT_tdstSector_* sector, MTH3D_tdstVector_* center);

static inline void inv_SCT_fn_vIsolateCharacterNode(SECT_tdstListOfSOCharacters_* node)
{
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstCharacterDyn* anchor;

    if (node->hNextBrotherDyn != NULL) {
        node->hNextBrotherDyn->hPrevBrotherDyn = node->hPrevBrotherDyn;
    }
    if (node->hPrevBrotherDyn != NULL) {
        node->hPrevBrotherDyn->hNextBrotherDyn = node->hNextBrotherDyn;
    }
    anchor = node->hFatherDyn;
    if (anchor != NULL) {
        anchor->lNumberOfElementsDyn--;
        anchor = node->hFatherDyn;
        if (anchor->hFirstElementDyn == node) {
            anchor->hFirstElementDyn = node->hNextBrotherDyn;
            anchor = node->hFatherDyn;
        }
        if (anchor->hLastElementDyn == node) {
            anchor->hLastElementDyn = node->hPrevBrotherDyn;
        }
    }
}

static inline void inv_SCT_fn_vIsolateLightNode(SECT_tdstListOfDNMLights_* node)
{
    LST2_tdstAnchorToSECT_tdxHandleOfElementLstDNMLightsDyn* anchor;

    if (node->hNextBrotherDyn != NULL) {
        node->hNextBrotherDyn->hPrevBrotherDyn = node->hPrevBrotherDyn;
    }
    if (node->hPrevBrotherDyn != NULL) {
        node->hPrevBrotherDyn->hNextBrotherDyn = node->hNextBrotherDyn;
    }
    anchor = node->hFatherDyn;
    if (anchor != NULL) {
        anchor->lNumberOfElementsDyn--;
        anchor = node->hFatherDyn;
        if (anchor->hFirstElementDyn == node) {
            anchor->hFirstElementDyn = node->hNextBrotherDyn;
            anchor = node->hFatherDyn;
        }
        if (anchor->hLastElementDyn == node) {
            anchor->hLastElementDyn = node->hPrevBrotherDyn;
        }
    }
}

void SECT_fn_vGetMinPointInBorder(const SECT_tdstSector_* _hSector, MTH3D_tdstVector_* _p_stMinPoint)
{
    *_p_stMinPoint = _hSector->stMinMaxPoints[0];
}

void SECT_fn_vGetMaxPointInBorder(const SECT_tdstSector_* _hSector, MTH3D_tdstVector_* _p_stMaxPoint)
{
    *_p_stMaxPoint = _hSector->stMinMaxPoints[1];
}

void SECT_fn_vGetCenterOfBorder(SECT_tdstSector_* sector, MTH3D_tdstVector_* center)
{
    MTH3D_M_vAddXYZ_inv(center, &sector->stMinMaxPoints[0], &sector->stMinMaxPoints[1]);
    MTH3D_M_vScaleXYZ_inv(center, center, 0.5f);
}

SECT_tdstListOfSOCharacters_* SECT_fn_hCreateElementLstCharacter(void)
{
    SECT_tdstListOfSOCharacters_* hCharacterElement = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(SECT_tdstListOfSOCharacters_));

    hCharacterElement->hNextBrotherDyn = NULL;
    hCharacterElement->hPrevBrotherDyn = NULL;
    hCharacterElement->hFatherDyn = NULL;
    return hCharacterElement;
}

void SECT_fn_vSetCharacterPointerInCharacterList(SECT_tdstListOfSOCharacters_* hCharacterElement,
                                                 HIE_tdstSuperObject_* hSectorSuperObject)
{
    hCharacterElement->hPointerOfCharacter = hSectorSuperObject;
}

SECT_tdstListOfDNMLights_* SECT_fn_hCreateElementLstDNMLights(void)
{
    SECT_tdstListOfDNMLights_* hDNMLightsElement =
        AI_fn_p_vTrueAlloc(gp_6F74, sizeof(SECT_tdstListOfDNMLights_));

    hDNMLightsElement->hNextBrotherDyn = NULL;
    hDNMLightsElement->hPrevBrotherDyn = NULL;
    hDNMLightsElement->hFatherDyn = NULL;
    return hDNMLightsElement;
}

void SECT_fn_vSetHandleOfLightsInDNMLightsList(SECT_tdstListOfDNMLights_* hDNMLightsElement, void* hDNMLights)
{
    hDNMLightsElement->hDNMLights = hDNMLights;
}

s32 SECT_fn_bIsInBorderBox(MTH3D_tdstVector_* _p_stPosition, SECT_tdstSector_* _hSector)
{
    MTH3D_tdstVector_ min;
    MTH3D_tdstVector_ max;

    SECT_fn_vGetMinPointInBorder(_hSector, &min);
    SECT_fn_vGetMaxPointInBorder(_hSector, &max);
    if (_p_stPosition->xX < min.xX || max.xX < _p_stPosition->xX || _p_stPosition->xY < min.xY || max.xY < _p_stPosition->xY ||
        _p_stPosition->xZ < min.xZ || max.xZ < _p_stPosition->xZ)
        return 0;
    return 1;
}

HIE_tdstSuperObject_* SECT_fn_hResearchInWhatSectorIAm(HIE_tdstSuperObject_* SECT_hFatherSector,
                                                           MTH3D_tdstVector_* p_stAbsolutePosition)
{
    HIE_tdstSuperObject_* bestOther = NULL;
    s32 priorityMain = 0;
    s32 priorityOther = 0;
    HIE_tdstSuperObject_* bestMain = NULL;
    HIE_tdstSuperObject_* last = NULL;
    HIE_tdstSuperObject_* node;
    SECT_tdstSector_* sector;
    f32 distanceOther = flt_3935B0[0];
    f32 distanceMain = distanceOther;
    MTH3D_tdstVector_ center;
    f32 distance;

    for (node = SECT_hFatherSector->hFirstElementSta; node != NULL; node = node->hNextBrotherSta) {
        sector = inv_HIE_fn_p_stGetSector(node);
        last = node;
        if (SECT_fn_bIsInBorderBox(p_stAbsolutePosition, sector)) {
            SECT_fn_vGetCenterOfBorder(sector, &center);
            MTH3D_M_vSubXYZ_inv(&center, &center, p_stAbsolutePosition);
            distance = MTH_fn_xSqrAddSqrAddSqrOpt2(center.xX, center.xY, center.xZ);
            distance = MTH_fn_xSqrtOpt2(distance);
            if (sector->xVirtual == 0) {
                if (priorityMain < sector->cPriority) {
                    distanceMain = distance;
                    priorityMain = sector->cPriority;
                    bestMain = last;
                } else if (sector->cPriority == priorityMain && distance < distanceMain) {
                    distanceMain = distance;
                    bestMain = last;
                }
            } else {
                if (priorityOther < sector->cPriority) {
                    distanceOther = distance;
                    priorityOther = sector->cPriority;
                    bestOther = last;
                } else if (sector->cPriority == priorityOther && distance < distanceOther) {
                    distanceOther = distance;
                    bestOther = last;
                }
            }
        }
    }
    if (bestMain == NULL) {
        bestMain = bestOther;
    }
    return bestMain != NULL ? bestMain : last;
}

void inv_SpatialTree_vFindChildAtPoint(void* unused, MTH3D_tdstVector_* point,
                                       HIE_tdstSuperObject_** found)
{
    SECT_tdstSector_* current;
    HIE_tdstSuperObject_* root;
    SECT_tdstListOfSOSectorsInCollisionInteraction_* child;
    HIE_tdstSuperObject_* item;
    s32 count;

    root = *found;
    current = inv_HIE_fn_p_stGetSector(root);
    if (SECT_fn_bIsInBorderBox(point, current) != 0 && current->xVirtual == 0) {
        count = inv_HIE_fn_p_stGetSector(root)->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt;
        child = inv_HIE_fn_p_stGetSector(root)->stListOfSectorsInCollisionInteraction.hFirstElementOpt;
        while (count > 0) {
            item = child->hPointerOfSectorSO;
            if (current->cPriority >= inv_HIE_fn_p_stGetSector(item)->cPriority) {
                return;
            }
            if (inv_HIE_fn_p_stGetSector(item)->xVirtual == 0 &&
                SECT_fn_bIsInBorderBox(point, inv_HIE_fn_p_stGetSector(item)) != 0) {
                *found = item;
                return;
            }
            count--;
            child++;
        }
    } else {
        count = inv_HIE_fn_p_stGetSector(root)->stListOfSectorsInCollisionInteraction.lNumberOfElementsOpt;
        child = inv_HIE_fn_p_stGetSector(root)->stListOfSectorsInCollisionInteraction.hFirstElementOpt;
        while (count > 0) {
            item = child->hPointerOfSectorSO;
            if (inv_HIE_fn_p_stGetSector(item)->xVirtual == 0 &&
                SECT_fn_bIsInBorderBox(point, inv_HIE_fn_p_stGetSector(item)) != 0) {
                *found = item;
                return;
            }
            count--;
            child++;
        }
        *found = SECT_fn_hResearchInWhatSectorIAm(root->hFatherSta, point);
    }
}

void SECT_fn_vIsolateCharListNode(SECT_tdstListOfSOCharacters_* hNode)
{
    inv_SCT_fn_vIsolateCharacterNode(hNode);
    hNode->hNextBrotherDyn = NULL;
    hNode->hPrevBrotherDyn = NULL;
    hNode->hFatherDyn = NULL;
}

void SECT_fn_vAddTailCharListNode(HIE_tdstSuperObject_* hSector, SECT_tdstListOfSOCharacters_* hNode)
{
    inv_SCT_fn_vIsolateCharacterNode(hNode);
    hNode->hPrevBrotherDyn = NULL;
    hNode->hFatherDyn = NULL;
    hNode->hNextBrotherDyn = NULL;
    hNode->hPrevBrotherDyn = inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hLastElementDyn;
    hNode->hFatherDyn = &inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters;
    if (inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hFirstElementDyn == NULL) {
        inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hFirstElementDyn = hNode;
    } else {
        inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hLastElementDyn->hNextBrotherDyn = hNode;
    }
    inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hLastElementDyn = hNode;
    inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.lNumberOfElementsDyn++;
}

void SECT_fn_vAddHeadCharListNode(HIE_tdstSuperObject_* hSector, SECT_tdstListOfSOCharacters_* hNode)
{
    inv_SCT_fn_vIsolateCharacterNode(hNode);
    hNode->hNextBrotherDyn = NULL;
    hNode->hFatherDyn = NULL;
    hNode->hPrevBrotherDyn = NULL;
    hNode->hNextBrotherDyn = inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hFirstElementDyn;
    hNode->hFatherDyn = &inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters;
    if (inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hLastElementDyn == NULL) {
        inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hLastElementDyn = hNode;
    } else {
        inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hFirstElementDyn->hPrevBrotherDyn = hNode;
    }
    inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.hFirstElementDyn = hNode;
    inv_HIE_fn_p_stGetSector(hSector)->stListOfCharacters.lNumberOfElementsDyn++;
}

void SECT_fn_vIsolateDNMLightsListNode(SECT_tdstListOfDNMLights_* hNode)
{
    inv_SCT_fn_vIsolateLightNode(hNode);
    hNode->hNextBrotherDyn = NULL;
    hNode->hPrevBrotherDyn = NULL;
    hNode->hFatherDyn = NULL;
}

void SECT_fn_vAddTailDNMLightsListNode(HIE_tdstSuperObject_* hSector,
                                        SECT_tdstListOfDNMLights_* hNode)
{
    inv_SCT_fn_vIsolateLightNode(hNode);
    hNode->hPrevBrotherDyn = NULL;
    hNode->hFatherDyn = NULL;
    hNode->hNextBrotherDyn = NULL;
    hNode->hPrevBrotherDyn = inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights.hLastElementDyn;
    hNode->hFatherDyn = &inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights;
    if (inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights.hFirstElementDyn == NULL) {
        inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights.hFirstElementDyn = hNode;
    } else {
        inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights.hLastElementDyn->hNextBrotherDyn = hNode;
    }
    inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights.hLastElementDyn = hNode;
    inv_HIE_fn_p_stGetSector(hSector)->stListOfDNMLights.lNumberOfElementsDyn++;
}

u8 SECT_fn_bIsThisSectorVirtual(HIE_tdstSuperObject_* _hSprObjSector)
{
    return inv_HIE_fn_p_stGetSector(_hSprObjSector)->xVirtual;
}

s8 fn_cGetSectorCameraType(HIE_tdstSuperObject_* _hSprObjSector)
{
    return inv_HIE_fn_p_stGetSector(_hSprObjSector)->cCameraType;
}

f32 fn_xGetSectorZFar(HIE_tdstSuperObject_* _hSprObjSector)
{
    return inv_HIE_fn_p_stGetSector(_hSprObjSector)->xZFar;
}

MTH3D_tdstVector_* SECT_fn_p_stGetMinPointInBorder(SECT_tdstSector_* _hSector)
{
    return &_hSector->stMinMaxPoints[0];
}

MTH3D_tdstVector_* SECT_fn_p_stGetMaxPointInBorder(SECT_tdstSector_* _hSector)
{
    return &_hSector->stMinMaxPoints[1];
}

void func_001467A8(void)
{
    D_003935B4 = 1;
}
