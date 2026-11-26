#include "common.h"

#include "renderware.h"
#include "GLI/GLI.h"
#include "GAM/GAM.h"

#define RW3_LIGHT_TYPE_SINGLE 2

RpWorld* RpClumpGetWorld(RpClump* clump);
RpWorld* RpLightGetWorld(RpLight* light);
void RpClumpDestroy(RpClump* clump);
void RpWorldRemoveClump(RpWorld* world, RpClump* clump);
void RpWorldRemoveLight(RpWorld* world, RpLight* light);

s32 inv_RW3_Light_DestroyAllLevelLights(void)
{
    GLI_tdstLight_** list = inv_RW3_p_pLevelLights;
    s32 singleDestroyed = 0;

    if (list == NULL) {
        return 0;
    }
    for (; *list != NULL; list++) {
        RpLight* light = (*list)->backendLight;
        RpWorld* world;
        RwFrame* frame;

        if (light == NULL) {
            continue;
        }
        if (RpLightGetType(light) == RW3_LIGHT_TYPE_SINGLE && singleDestroyed == 1) {
            continue;
        }
        world = RpLightGetWorld(light);
        if (world != NULL) {
            RpWorldRemoveLight(world, light);
        }
        frame = RpLightGetFrame(light);
        if (frame != NULL) {
            RwFrameDestroy(frame);
        }
        if (RpLightGetType(light) == RW3_LIGHT_TYPE_SINGLE) {
            inv_RW3_singleLight = NULL;
            singleDestroyed = 1;
        }
        RpLightDestroy(light);
    }
    return 1;
}

s32 inv_RW3_DestroyAllLevelObjects(void)
{
    GeometricObjectView** list = inv_SNA_p_hLevelGeometricObjects;
    RpClump* clump;
    RwFrame* frame;
    RpWorld* world;

    if (list == NULL) {
        return 0;
    }
    for (; *list != NULL; list++) {
        RW3_DestroyRW3Object((*list)->rw3Object);
    }
    inv_RW3_fn_vDestroyLightmapTemplates();
    clump = inv_RW3_levelClump;
    D_0039371C = 0;
    frame = clump->object.parent;
    world = RpClumpGetWorld(clump);
    if (frame != NULL) {
        RwFrameDestroy(frame);
    }
    if (world != NULL) {
        RpWorldRemoveClump(world, inv_RW3_levelClump);
    }
    RpClumpDestroy(inv_RW3_levelClump);
    inv_RW3_levelClump = NULL;
    return 1;
}

s32 inv_RW3_Material_DestroyAllLevelMaterials(void)
{
    s32* v1;
    MaterialLayer* entry;
    s32* nextList;
    s32 a0;
    s32 v0;
    v1 = (s32*) inv_RW3_p_pLevelMaterials;
    if (v1 == 0) {
        return 0;
    }
    a0 = *v1;
    if (a0 == 0) {
        return 1;
    }
    do {
        entry = ((GliMaterialData*) a0)->layers - 1;
        nextList = v1 + 1;
        a0 = (s32) nextList + (a0 ^ a0);
        entry++;
        do {
            RpMaterialDestroy(entry->rpMaterial);
            v1 = (s32*) (s32) entry->flags;
            v1 = (s32*) ((s32) v1 & MATERIAL_LAYER_CONTINUES);
            entry++;
        } while ((s32) v1 != 0);
        v1 = nextList;
        v0 = *v1;
        a0 = v0;
    } while (v0 != 0);
    return 1;
}

void RW3_fn_vDestroyAllRWObjectLEVEL(void)
{
    if (D_00393658) {
        inv_RW3_Light_DestroyAllLevelLights();
        inv_RW3_DestroyAllLevelObjects();
        inv_RW3_Material_DestroyAllLevelMaterials();
        RwResourcesEmptyArena();
        inv_RW3_Texture_PurgeLevelResources();
        D_00393658 = 0;
    }
}
