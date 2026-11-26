#include "common.h"

#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "renderware.h"
#include "rwps2.h"
#include "GAM/GAM.h"

#define LIGHTMAP_LAYERS 3
#define GEOMETRIC_ELEMENT_ATOMIC 1
#define LIGHT_FOR_ACTORS 0x2
#define LIGHT_FOR_MAP 0x1

typedef struct LevelTextureEntry {
    GliTexture* texture;
    s32 id;
} LevelTextureEntry;

s32 RpClumpGetNumAtomics(void* clump);
void* RwTextureSetAddressing(void* texture, s32 mode);
s32 RwFreeListPurgeAllFreeLists(void);

s32 inv_RW3_fn_bLoadLevelTextures(void)
{
    char name[0x10];
    u32 i = 0;
    LevelTextureEntry* entry = inv_RW3_p_stLevelTextures;
    GliTexture* texture;

    for (i = 0; i < inv_RW3_ulLevelTextureCount; i++, entry++) {
        sprintf(name, "%03d", entry->id);
        if (entry->texture == NULL) {
            continue;
        }
        entry->texture->rwTexture = inv_RW3_fn_p_stFindLevelTexture(name);
        texture = entry->texture;
        if (texture->rwTexture == NULL) {
            printf("ERROR -> Cannot load texture [%s] from TexDictionary.\n", name);
            continue;
        }
        texture->loaded = 1;
        texture = entry->texture;
        switch (texture->addressMode & 3) {
        case 0:
            RwTextureSetAddressing(texture->rwTexture, rwTEXTUREADDRESSCLAMP);
            break;
        case 1:
            RwTextureSetAddressing(texture->rwTexture, rwTEXTUREADDRESSCLAMP);
            RwTextureSetAddressingV(entry->texture->rwTexture, rwTEXTUREADDRESSWRAP);
            break;
        case 2:
            RwTextureSetAddressing(texture->rwTexture, rwTEXTUREADDRESSWRAP);
            RwTextureSetAddressingV(entry->texture->rwTexture, rwTEXTUREADDRESSCLAMP);
            break;
        case 3:
            RwTextureSetAddressing(texture->rwTexture, rwTEXTUREADDRESSWRAP);
            break;
        }
    }
    return 1;
}

s32 inv_RW3_fn_bLoadLightmaps(void)
{
    char name[0x10];
    void* rasters[LIGHTMAP_LAYERS];
    u32 index;
    LightmapSlot* slot = inv_RW3_lightmapSlots;
    u32 layer;
    RwTexture* texture;
    RwRaster* palette;
    void* first;

    for (index = 0; index < inv_RW3_ulLightmapSlotCount;) {
        layer = 0;
        do {
            sprintf(name, "%03d.%d", index, layer);
            texture = inv_RW3_fn_p_stFindLevelTexture(name);
            if (texture == NULL) {
                return 0;
            }
            rasters[layer] = texture->raster;
            layer++;
            RwTextureSetRaster(texture, NULL);
            RwTextureDestroy(texture);
        } while (layer < LIGHTMAP_LAYERS);
        first = rasters[0];
        index++;
        palette = inv_RW3_lightmapPaletteRaster;
        slot->r = first;
        slot->g = rasters[1];
        slot->b = rasters[2];
        inv_RW3_fn_vSetRasterPalette(first, palette->palette);
        inv_RW3_fn_vSetRasterPalette(slot->g, inv_RW3_lightmapPaletteRaster->palette);
        inv_RW3_fn_vSetRasterPalette(slot->b, inv_RW3_lightmapPaletteRaster->palette);
        slot++;
    }
    return 1;
}

s32 inv_RW3_fn_bCreateLevelMaterials(void)
{
    RwRGBA color;
    GliMaterialData** list = inv_RW3_p_pLevelMaterials;
    MaterialLayer* entry;
    RpMaterial* material;
    f32 value;
    f32 zero;
    f32 one;
    f32 scale;
    s32 maxChannel;
    u8 entryFlags;

    if (inv_RW3_fn_bLoadLightmaps() == 0) {
        printf("ERROR -> LightMaps NOT LOADED.\n");
    }
    inv_RW3_fn_bLoadLevelTextures();
    if (*list != NULL) {
        maxChannel = 0xFF;
        zero = 0.0f;
        one = 1.0f;
        scale = 255.0f;
        do {
            entry = (*list)->layers - 1;
            do {
                entry++;
                material = inv_RW3_fn_pCreateDefaultRpMaterial();
                if (!((*list)->type & MATERIAL_COLOR_FROM_TEXTURE)) {
                    value = (*list)->color.xR;
                    if (value < zero) {
                        color.red = 0;
                    } else if (one < value) {
                        color.red = maxChannel;
                    } else {
                        color.red = (s32) (value * scale);
                    }
                    value = (*list)->color.xG;
                    if (value < zero) {
                        color.green = 0;
                    } else if (one < value) {
                        color.green = maxChannel;
                    } else {
                        color.green = (s32) (value * scale);
                    }
                    value = (*list)->color.xB;
                    if (value < zero) {
                        color.blue = 0;
                    } else if (one < value) {
                        color.blue = maxChannel;
                    } else {
                        color.blue = (s32) (value * scale);
                    }
                    color.alpha = 0xFE;
                    RpMaterialSetColor(material, &color);
                } else {
                    inv_RW3_fn_vSetRpMaterialColor(material, &(*list)->color);
                }
                if (entry->texture != NULL) {
                    RpMaterialSetTexture(material, entry->texture->rwTexture);
                }
                entryFlags = entry->flags;
                entry->rpMaterial = material;
            } while (entryFlags & MATERIAL_LAYER_CONTINUES);
            list++;
        } while (*list != NULL);
    }
    return 1;
}

s32 inv_RW3_fn_lCountLevelAtomics(void)
{
    GeometricObjectView** list;
    GeometricObjectView* object;
    s32 atomics = 0;
    s32 count;
    u32 i;

    for (list = inv_SNA_p_hLevelGeometricObjects; *list != NULL; list++) {
        object = *list;
        count = object->elementCount;
        for (i = 0; i < count; i++) {
            if ((*(s16* volatile*) &object->elementTypes)[i] == GEOMETRIC_ELEMENT_ATOMIC) {
                atomics++;
            }
        }
    }

    printf("******************************************************\n");
    if (inv_RW3_levelClump == NULL) {
        printf("ATO_SAVE : Trying to save %d atomics in TheClump.\n", atomics);
    } else {
        printf("ATO_LOAD : Trying to load %d / %d atomics in TheClump.\n", atomics,
               RpClumpGetNumAtomics(inv_RW3_levelClump));
    }
    printf("******************************************************\n");
    for (list = inv_SNA_p_hLevelGeometricObjects; *list != NULL; list++) {
        RW3_CreateRW3ObjectFromGeometric(*list);
    }
    return 1;
}

s32 inv_RW3_fn_bCreateLevelLights(void)
{
    GLI_tdstLight_** list;
    GLI_tdstLight_* light;
    GLI_tdstLight_* next;
    s32 parsed;
    s32 loaded;
    s32 total;
    s32 map;
    s32 flags;
    s32 mapBit;

    list = inv_RW3_p_pLevelLights;
    map = 0;
    total = 0;
    loaded = 0;
    parsed = 0;
    inv_RW3_fn_vResetEnabledLights();
    if (inv_RW3_p_pLevelLights == NULL) {
        return 0;
    }
    light = *list;
    if (light != NULL) {
        do {
            flags = light->ucObjectLighted;
            list++;
            total++;
            loaded++;
            if (flags & LIGHT_FOR_ACTORS) {
                parsed++;
            }
            mapBit = flags & LIGHT_FOR_MAP;
            RW3_Light_CreateFromGLILight(light);
            map += mapBit;
            next = *list;
            light = next;
        } while (next != NULL);
    }
    printf("INFO -> [%d / %d] lights loaded/parsed : %d for MAP and %d for Actors.\n", loaded,
           total, map, parsed);
    return 1;
}

void RW3_fn_vCreateAllRWObjectLEVEL(void)
{
    func_001515C0();
    RwFreeListPurgeAllFreeLists();
    inv_RW3_fn_bCreateLevelMaterials();
    RwFreeListPurgeAllFreeLists();
    inv_RW3_fn_lCountLevelAtomics();
    RwFreeListPurgeAllFreeLists();
    inv_RW3_fn_bCreateLevelLights();
    RwFreeListPurgeAllFreeLists();
    D_00393658 = 1;
}
