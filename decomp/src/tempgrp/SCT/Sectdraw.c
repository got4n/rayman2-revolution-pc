#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "HIE/HIE.h"
#include "MTH/MTH.h"
#include "SCT/SCT.h"

#include "ps2/libc.h"
#include "GEO/GEO.h"
#include "AI/AI.h"
#include "SHW/SHW.h"

#define LIGHT_TYPE_FOG 6
#define DRAW_MASK_STATIC_LIGHTS 0x8000
#define DRAW_MASK_CHARACTER 0x200000
#define SUPER_OBJECT_FLAG_ALWAYS_DRAWN 0x200

void SECT_fn_vSendDynamicLightsOfASector(void* _p_stVpt, SECT_tdstSector_* _hSectorObject, s32 unused)
{
    SECT_tdstListOfDNMLights_* node = _hSectorObject->stListOfDNMLights.hFirstElementDyn;

    while (node != 0) {
        inv_GLI_vRecordAddLightToViewport(node->hDNMLights);
        node = node->hNextBrotherDyn;
    }
}

void SECT_fn_vSendDynamicLightsFromWhereIAmToViewport(GLD_tdstViewportAttributes_* _p_stVpt,
                                                      HIE_tdstSuperObject_* _hSprObjSector)
{
    SECT_tdstSector_* sector = inv_HIE_fn_p_stGetSector(_hSprObjSector);
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    s16 index;

    SECT_fn_vSendDynamicLightsOfASector(_p_stVpt, sector, 0);
    neighbor = sector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    for (index = 0; index < sector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt; index++) {
        HIE_tdstSuperObject_* neighborObject = neighbor->hPointerOfSectorSO;

        neighbor++;
        SECT_fn_vSendDynamicLightsOfASector(_p_stVpt, inv_HIE_fn_p_stGetSector(neighborObject), 0);
    }
}

s32 func_00146888(HIE_tdstSuperObject_* superObject, GLI_tdstLight_** lights, u32 capacity,
                  u32* count)
{
    SECT_tdstSector_* sector;
    SECT_tdstListOfStaticLights_* sectorLights;
    s16 index;

    if (superObject->lTypeOfLinkedObject == HIE_TYPE_SECTOR_inv) {
        sector = inv_HIE_fn_p_stGetSector(superObject);
        sectorLights = sector->stListOfStaticLights.hFirstElementOpt;
        for (index = 0; index < sector->stListOfStaticLights.lNumberOfElementsOpt; index++, sectorLights++) {
            GLI_tdstLight_* light = sectorLights->hStaticLights;
            u32 total;
            u32 i;
            s32 found;

            if (GLI_lGetLightType(light) == LIGHT_TYPE_FOG) {
                continue;
            }
            total = *count;
            found = 0;
            for (i = 0; i < total; i++) {
                if (light == lights[i]) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                lights[total] = light;
                (*count)++;
                if (*count >= capacity) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

s32 func_001469B8(HIE_tdstSuperObject_* superObject, GLI_tdstLight_** lights, u32 capacity,
                  u32* count)
{
    SECT_tdstListOfDNMLights_* node;

    for (node = inv_HIE_fn_p_stGetSector(superObject)->stListOfDNMLights.hFirstElementDyn; node != NULL; node = node->hNextBrotherDyn) {
        GLI_tdstLight_* light = node->hDNMLights;
        u32 total;
        u32 i;
        s32 found;

        if (GLI_lGetLightType(light) == LIGHT_TYPE_FOG)
            continue;
        total = *count;
        found = 0;
        for (i = 0; i < total; i++) {
            if (light == lights[i]) {
                found = 1;
                break;
            }
        }
        if (!found) {
            lights[total] = light;
            (*count)++;
            if (*count >= capacity)
                return 0;
        }
    }
    return 1;
}

s32 func_00146AB0(HIE_tdstSuperObject_* superObject, GLI_tdstLight_** lights, u32 capacity,
                  u32* count)
{
    SECT_tdstSector_* sector;
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    s16 index;

    if (superObject->lTypeOfLinkedObject == HIE_TYPE_SECTOR_inv) {
        sector = inv_HIE_fn_p_stGetSector(superObject);
        if (func_001469B8(superObject, lights, capacity, count) == 0)
            return 0;
        neighbor = sector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
        for (index = 0; index < sector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt; index++, neighbor++) {
            if (func_001469B8(neighbor->hPointerOfSectorSO, lights, capacity, count) == 0)
                return 0;
        }
    }
    return 1;
}

s32 SECT_fn_xSendStaticLightsToViewportForChar(void* viewport, HIE_tdstSuperObject_* superObject)
{
    SECT_tdstSector_* sector = inv_HIE_fn_p_stGetSector(superObject);
    SECT_tdstListOfStaticLights_* lights = sector->stListOfStaticLights.hFirstElementOpt;
    s32 result = 0;
    s16 sent = 0;
    s16 index;

    for (index = 0; index < sector->stListOfStaticLights.lNumberOfElementsOpt; index++, lights++) {
        GLI_tdstLight_* light = lights->hStaticLights;

        if (GLI_ucGetObjectLighted(light) & 2) {
            inv_GLI_vRecordAddLightToViewport(light);
            result = ++sent;
        }
    }
    return result;
}

void SECT_fn_vSendFogToViewport(void* viewport, HIE_tdstSuperObject_* _hSprObjSector)
{
    SECT_tdstSector_* sector;
    SECT_tdstListOfStaticLights_* lights;
    s16 index;

    if (GLI_cGlobalFogIsOn != 0) {
        GLI_vFogOn();
        return;
    }
    sector = inv_HIE_fn_p_stGetSector(_hSprObjSector);
    lights = sector->stListOfStaticLights.hFirstElementOpt;
    for (index = 0; index < sector->stListOfStaticLights.lNumberOfElementsOpt; index++, lights++) {
        GLI_tdstLight_* light = lights->hStaticLights;

        if (GLI_lGetLightType(light) == LIGHT_TYPE_FOG) {
            GEO_tdstColor_ color;
            f32 nearBlend;
            f32 nearDistance;
            f32 farBlend;
            f32 farDistance;

            GLI_xGetLightColor(light, &color);
            nearBlend = GLI_fn_xFogGetBlendNear(light);
            nearDistance = GLI_fn_xFogGetNear(light);
            farBlend = GLI_fn_xFogGetBlendFar(light);
            farDistance = GLI_fn_xFogGetFar(light);
            GLI_vSetFog2(&color, nearBlend, nearDistance, farBlend, farDistance,
                          GLI_fn_xFogGetInfinite(light));
            GLI_vFogOn();
            return;
        }
    }
    GLI_vFogOff();
}

s32 func_00146D78(HIE_tdstSuperObject_* superObject, GLI_tdstLight_** lights, u32 capacity,
                  u32* count)
{
    HIE_tdstSuperObject_* child;

    if (superObject->lTypeOfLinkedObject == HIE_TYPE_SECTOR_inv) {
        SECT_tdstSector_* sector = inv_HIE_fn_p_stGetSector(superObject);
        SECT_tdstListOfStaticLights_* sectorLights = sector->stListOfStaticLights.hFirstElementOpt;
        s16 index;

        for (index = 0; index < sector->stListOfStaticLights.lNumberOfElementsOpt; index++, sectorLights++) {
            GLI_tdstLight_* light = sectorLights->hStaticLights;
            u32 i;
            s32 found;

            if (GLI_lGetLightType(light) != LIGHT_TYPE_FOG) {
                continue;
            }
            found = 0;
            for (i = 0; i < *count; i++) {
                if (light == lights[i]) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                lights[*count] = light;
                (*count)++;
                if (*count >= capacity) {
                    return 0;
                }
            }
        }
    }

    for (child = superObject->hFirstElementSta; child != 0; child = child->hNextBrotherSta) {
        if (!func_00146D78(child, lights, capacity, count)) {
            return 0;
        }
    }
    return 1;
}

s32 SCT_fn_lSendSectorToViewportStatic(GLD_tdstViewportAttributes_* _p_stVpt,
                                       HIE_tdstSuperObject_* _hSprObjSector, s32 _lDrawMask)
{
    SECT_tdstSector_* sector = inv_HIE_fn_p_stGetSector(_hSprObjSector);
    void* boundingVolume;
    MTH3D_tdstVector_* sectorMin;
    MTH3D_tdstVector_* sectorMax;
    MTH3D_tdstVector_* boxMin;
    MTH3D_tdstVector_* boxMax;
    MTH3D_tdstVector_ minDelta;
    MTH3D_tdstVector_ maxDelta;
    f32 sectorMinX;
    f32 minX, minY, minZ, maxX, maxY, maxZ;
    s32 result;
    s32 minSide;
    s32 maxSide;

    _GLI_vResetTableOfActiveLightsForChar(_hSprObjSector);
    boundingVolume = _hSprObjSector->hVisualBoundingVolume;
    sectorMin = SECT_fn_p_stGetMinPointInBorder(sector);
    sectorMax = SECT_fn_p_stGetMaxPointInBorder(sector);
    if (boundingVolume != 0) {
        boxMin = COL_fn_pGetMinPointOfParallelBox(boundingVolume);
        boxMax = COL_fn_pGetMaxPointOfParallelBox(boundingVolume);
        result = GEO_lCullingBox(_p_stVpt, boxMin, boxMax, 0);
        if (result != 0 || (_hSprObjSector->flags & SUPER_OBJECT_FLAG_ALWAYS_DRAWN)) {
            HIE_fn_vSendStaticWorldToViewport(_p_stVpt, _hSprObjSector, _lDrawMask, result);
            sectorMinX = sectorMin->xX;
        } else {
            sectorMinX = sectorMin->xX;
        }

        MTH3D_M_vSetXYZ_inv(&minDelta, sectorMinX - boxMin->xX, sectorMin->xY - boxMin->xY,
                            sectorMin->xZ - boxMin->xZ);
        inv_MTH3D_fn_vSubXYZ(&maxDelta, sectorMax, boxMax);
        minX = minDelta.xX;
        minY = minDelta.xY;
        minZ = minDelta.xZ;
        maxX = maxDelta.xX;
        maxY = maxDelta.xY;
        maxZ = maxDelta.xZ;

        if (inv_MTH_fn_xAbsolute(inv_MTH_fn_xAbsolute(minX) + inv_MTH_fn_xAbsolute(minY) +
                                 inv_MTH_fn_xAbsolute(minZ)) < 0.01f &&
            inv_MTH_fn_xAbsolute(inv_MTH_fn_xAbsolute(maxX) + inv_MTH_fn_xAbsolute(maxY) +
                                 inv_MTH_fn_xAbsolute(maxZ)) < 0.01f) {
            return result;
        }
        minSide = (minX < 0.0f ? 1 : -1) + (minY < 0.0f ? 1 : -1) + (minZ < 0.0f ? 1 : -1);
        if (minSide == -3 || minSide == 3) {
            maxSide = (maxX < 0.0f ? -1 : 1) + (maxY < 0.0f ? -1 : 1) + (maxZ < 0.0f ? -1 : 1);
        } else {
            maxSide = 0;
        }
        if (minSide != maxSide || minSide > 0 || result == 0x7F) {
            result = GEO_lCullingBox(_p_stVpt, sectorMin, sectorMax, 0);
        }
    } else {
        result = GEO_lCullingBox(_p_stVpt, sectorMin, sectorMax, 0);
    }
    return result;
}

void inv_SCT_fn_vComputeSectorAndNeighborsCulling(void* viewport, HIE_tdstSuperObject_* superObject,
                                                  s32 mask, s32* results)
{
    SECT_tdstSector_* sector = inv_HIE_fn_p_stGetSector(superObject);
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    s8 mode = g_stEngineStructure.cCameraMode;
    s16 index;

    memset(g_ucTableOfActiveLightsForChar, 1, 0x20);
    *results++ = SCT_fn_lSendSectorToViewportStatic(viewport, superObject, mask);
    neighbor = sector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    for (index = 0; index < sector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt; index++, neighbor++) {
        if (mode == 1 || neighbor->cDisplayMode != 2) {
            *results++ = SCT_fn_lSendSectorToViewportStatic(viewport, neighbor->hPointerOfSectorSO, mask);
        }
    }
}

void SCT_fn_vSendSectorToViewportDynamic(void* _p_stAbsolutePositionOfCamera, void* _p_stVpt,
                                         HIE_tdstSuperObject_* _hSprObjSector, s32 _lDrawMask, s32 _lCullingResult)
{
    SECT_tdstSector_* hSectorObject = inv_HIE_fn_p_stGetSector(_hSprObjSector);
    SECT_tdstListOfSOCharacters_* node;
    s32 pushedLights;

    if (_lCullingResult == 0 && inv_GAM_bMenuCharactersOnly == 0) {
        return;
    }

    memset(g_ucTableOfActiveLightsForChar, 1, 0x20);
    pushedLights = 0;
    _GLI_vResetTableOfActiveLightsForChar(_hSprObjSector);
    SECT_fn_vSendFogToViewport(_p_stVpt, _hSprObjSector);
    if ((_lDrawMask & DRAW_MASK_STATIC_LIGHTS) &&
        (_hSprObjSector->gu_lDrawModeMask & DRAW_MASK_STATIC_LIGHTS)) {
        pushedLights = SECT_fn_xSendStaticLightsToViewportForChar(_p_stVpt, _hSprObjSector);
    }

    node = hSectorObject->stListOfCharacters.hFirstElementDyn;
    GEO_g_xZFarTransparencyZone = 0.0f;
    GEO_g_xZFar = inv_HIE_fn_p_stGetSector(_hSprObjSector)->xZFar;
    for (; node != 0; node = node->hNextBrotherDyn) {
        HIE_tdstSuperObject_* hCharacterSO = node->hPointerOfCharacter;
        s32 lDrawmaskOfCharacter = hCharacterSO->gu_lDrawModeMask;
        u32 ulCustomBits = fn_ulStandardGameGetCustomBitsSO(hCharacterSO);

        if (inv_GAM_bMenuCharactersOnly == 0 || (ulCustomBits & 0x100000)) {
            if (lDrawmaskOfCharacter & DRAW_MASK_CHARACTER) {
                HIE_fn_vSendCharacterToViewport(_p_stVpt, hCharacterSO, lDrawmaskOfCharacter & _lDrawMask, _lCullingResult);
            }
            if (inv_HIE_fn_p_stGetEngineObject(hCharacterSO)->h_3dData->shadow.control.mode != 0) {
                inv_SHW_fn_vQueueCharacterShadow(_p_stVpt, hCharacterSO, _hSprObjSector, _lDrawMask);
            }
        }
    }

    if (pushedLights == 0) {
        return;
    }
    inv_GLI_vRecordPopLightsInViewport(pushedLights);
}

void inv_SCT_fn_vSendSectorAndNeighborsDynamic(void* context, void* viewport,
                                               HIE_tdstSuperObject_* superObject, s32 mask,
                                               s32* results)
{
    SECT_tdstSector_* sector = inv_HIE_fn_p_stGetSector(superObject);
    SECT_tdstListOfSOSectorsInGraphicInteraction_* neighbor;
    s8 mode = g_stEngineStructure.cCameraMode;
    s16 index;

    SCT_fn_vSendSectorToViewportDynamic(context, viewport, superObject, mask, *results++);
    neighbor = sector->stListOfSectorsInGraphicInteraction.hFirstElementOpt;
    for (index = 0; index < sector->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt; index++, results++, neighbor++) {
        if (mode == 1 || neighbor->cDisplayMode != 2 || inv_GAM_bMenuCharactersOnly != 0) {
            SCT_fn_vSendSectorToViewportDynamic(context, viewport, neighbor->hPointerOfSectorSO, mask,
                                                *results);
        }
    }
}

void SCT_fn_vSendSectorWhereIAmToViewport(MTH3D_tdstVector_* _p_stAbsolutePositionOfCamera,
                                          GLD_tdstViewportAttributes_* _p_stVpt,
                                          HIE_tdstSuperObject_* _hSprObjSector, s32 _lDrawMask)
{
    s32* cullingResults;

    inv_SCT_bSendingSectorToViewport = 1;
    cullingResults = __builtin_alloca((inv_HIE_fn_p_stGetSector(_hSprObjSector)->stListOfSectorsInGraphicInteraction.lNumberOfElementsOpt + 1) * sizeof(s32));

    inv_GLI_vResetLightCommands();
    SECT_fn_vSendDynamicLightsFromWhereIAmToViewport(_p_stVpt, _hSprObjSector);
    inv_GLI_vPushLightCommand06();
    if (inv_GAM_bMenuCharactersOnly == 0) {
        inv_SCT_fn_vComputeSectorAndNeighborsCulling(_p_stVpt, _hSprObjSector, _lDrawMask, cullingResults);
    }
    inv_GLI_vPushLightCommandSkip();
    inv_SCT_fn_vSendSectorAndNeighborsDynamic(_p_stAbsolutePositionOfCamera, _p_stVpt, _hSprObjSector, _lDrawMask, cullingResults);
    inv_GLI_vPushLightCommand05();

    inv_SCT_ucDrawPass = 1;
    func_0014B380(_p_stVpt, 1);
    inv_SCT_ucDrawPass = 0;
    func_0014B380(_p_stVpt, 0);
    GLI_vClearListOfLightInViewport(_p_stVpt);
    inv_SCT_bSendingSectorToViewport = 0;
}
