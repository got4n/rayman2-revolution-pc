#include "common.h"

#include "GAM/GAM.h"
#include "GLI/GLI.h"

#include "ps2/libc.h"
#include "renderware.h"
#include "AI/AI.h"
#include "FMN/FMN.h"
#include "clib/clib.h"

#define LEVEL_TEXTURE_DICTIONARY_COUNT 6
#define PURGE_TEXTURE_COUNT 21
#define GEOMETRIC_ELEMENT_MESH 1
#define MAX_POINT_LIGHTS 5
#define FMN_INVALID_FILE 0xF
#define RW_STREAM_MEMORY 3
#define RW_STREAM_READ 1
#define RW_CHUNK_TEXDICTIONARY 0x16
#define RP_LIGHT_LIGHTATOMICS 1
#define RP_LIGHT_DIRECTIONAL 1
#define RP_GEOMETRY_LOCK_TEXCOORDS 8

typedef struct TexCoord {
    f32 u;
    f32 v;
} TexCoord;

extern RwRaster* inv_RW3_p_stEmptyRaster;
extern MemoryHeapView* inv_RW3_texDictionaryHeap;
extern u8 inv_RW3_bUseTexDictionaryHeap;
extern void* inv_RW3_levelTexDictionaries[LEVEL_TEXTURE_DICTIONARY_COUNT];
extern RwRGBA dword_3936A8[];
extern char byte_3936B0[];

double modf(double value, double* integral);
char* strlwr(char* string);

RpLight* RpLightSetColor(RpLight* light, const GEO_tdstColor_* color);
RpLight* RpLightSetFlags(RpLight* light, s32 flags);
s32 RpLightGetFlags(RpLight* light);
const RwRGBA* RpMaterialGetColor(RpMaterial* material);
TexCoord* RpGeometryGetVertexTexCoords(RpGeometry* geometry);
s32 RpGeometryGetNumVertices(RpGeometry* geometry);
RwTexture* RwTextureCreate(RwRaster* raster);
void* RwTexDictionaryCreate(void);
void RwTexDictionaryDestroy(void* dictionary);
void* RwTexDictionarySetCurrent(void* dictionary);
RwTexture* RwTexDictionaryFindNamedTexture(void* dictionary, char* name);
void* RwTexDictionaryStreamRead(RwStream* stream);
void RwFreeListPurgeAllFreeLists(void);
void _sweGarbageCollectChain(void);

void FMN_Read(u32 file, void* buffer, s32 count, s32 size);
void freemem(void* block);
void inv_RW3_fn_vScrollGeometryTexCoords(RpGeometry* geometry, GeometricObjectView* object, u32 index);
void inv_RW3_fn_vUseMallocForTexDictionary(void);
void inv_RW3_fn_vUseHeapForTexDictionary(void);
void inv_RW3_fn_vCreateTexDictionaryHeap(s32 size);
void inv_RW3_fn_vDestroyTexDictionaryHeap(void);

void RW3_Light_AddListOfLightsToWorld(LevelLightList* list)
{
    GLI_tdstLight_** entry;
    s32 remaining;
    RpLight* light;
    s32 type;

    entry = list->lights;
    for (remaining = list->count; remaining > 0; entry++, remaining--) {
        light = (*entry)->backendLight;
        if (light == inv_RW3_singleLight) {
            RpLightSetColor(light, &(*entry)->stColor);
            RpLightSetFlags(light, RP_LIGHT_LIGHTATOMICS);
        } else {
            type = RpLightGetType(light);
            if (RpLightGetFlags(light) != RP_LIGHT_LIGHTATOMICS) {
                if (type == RP_LIGHT_DIRECTIONAL) {
                    if (inv_RW3_enabledDirectionalLights == 0) {
                        RpLightSetFlags(light, RP_LIGHT_LIGHTATOMICS);
                        inv_RW3_enabledDirectionalLights++;
                    }
                } else if (inv_RW3_enabledPointLights < MAX_POINT_LIGHTS) {
                    RpLightSetFlags(light, RP_LIGHT_LIGHTATOMICS);
                    inv_RW3_enabledPointLights++;
                }
            }
        }
    }
}

RpMaterial* inv_RW3_fn_pCreateDefaultRpMaterial(void)
{
    RwRGBA color;
    RpMaterial* material;

    color = dword_3936A8[0];
    material = RpMaterialCreate();
    RpMaterialSetColor(material, &color);
    return material;
}

void inv_RW3_fn_vDestroyRpMaterial(RpMaterial* material)
{
    RpMaterialDestroy(material);
}

void inv_RW3_fn_pCloneRpMaterial(void* unused, RpMaterial* source)
{
    RpMaterial* material;

    material = RpMaterialCreate();
    RpMaterialSetColor(material, RpMaterialGetColor(source));
    RpMaterialSetTexture(material, RpMaterialGetTexture(source));
}

void inv_RW3_fn_vSetRpMaterialColor(RpMaterial* material, GEO_tdstColor_* color)
{
    RwRGBA rgba;
    f32 channel;

    channel = color->xR;
    if (channel < 0.0f) {
        rgba.red = 0;
    } else if (1.0f < channel) {
        rgba.red = 0xFF;
    } else {
        rgba.red = channel * 255.0f;
    }

    channel = color->xG;
    if (channel < 0.0f) {
        rgba.green = 0;
    } else if (1.0f < channel) {
        rgba.green = 0xFF;
    } else {
        rgba.green = channel * 255.0f;
    }

    channel = color->xB;
    if (channel < 0.0f) {
        rgba.blue = 0;
    } else if (1.0f < channel) {
        rgba.blue = 0xFF;
    } else {
        rgba.blue = channel * 255.0f;
    }

    channel = color->xA;
    if (channel < 0.0f) {
        rgba.alpha = 0;
    } else if (1.0f < channel) {
        rgba.alpha = 0xFF;
    } else {
        rgba.alpha = channel * 255.0f;
    }

    RpMaterialSetColor(material, &rgba);
}

void inv_RW3_fn_vSetLayerTexture(MaterialLayer* layer)
{
    void* texture = NULL;

    if (layer != NULL && layer->rpMaterial != NULL) {
        if (layer->texture != NULL) {
            texture = layer->texture->rwTexture;
        }
        RpMaterialSetTexture(layer->rpMaterial, texture);
    }
}

char* inv_RW3_fn_szCopyFileNameFromPath(char* destination, char* path)
{
    char* separator;

    if (strlen(path) < 2) {
        destination[0] = byte_3936B0[0];
        return;
    }
    separator = Strrchr(path, '\\');
    if (separator == NULL) {
        separator = Strrchr(path, '/');
        if (separator == NULL) {
            return;
        }
    }
    return strcpy(destination, separator + 1);
}

void inv_RW3_fn_vScrollGeometryTexCoords(RpGeometry* geometry, GeometricObjectView* object, u32 index)
{
    MaterialLayer* layer;
    TexCoord* texCoords;
    TexCoord* offset;
    f32* offsetV;
    s32 vertex;
    f32 half;
    f32 scrollU;
    f32 scrollV;
    f32 u;
    f32 v;
    double integral;

    layer = object->elements[index]->material->layers;
    integral = 0.0;
    if (layer->flags & MATERIAL_SCROLL_ANY) {
        RpGeometryLock(geometry, RP_GEOMETRY_LOCK_TEXCOORDS);
        half = 0.5f;
        texCoords = RpGeometryGetVertexTexCoords(geometry);
        vertex = RpGeometryGetNumVertices(geometry) - 1;
        scrollU = layer->scrollU * gs_fCoefOfMultiplicationForAnimatedTexture;
        scrollV = layer->scrollV * gs_fCoefOfMultiplicationForAnimatedTexture;
        offset = &object->rw3Object->scrollOffsets[index];
        offsetV = &offset->v;
        u = modf(scrollU * half, &integral);
        u += u;
        v = modf(scrollV * half, &integral);
        v += v;
        for (; vertex >= 0; vertex--) {
            texCoords[vertex].u += u - offset->u;
            texCoords[vertex].v += v - *offsetV;
        }
        offset->u = u;
        *offsetV = v;
        RpGeometryUnlock(geometry);
    }
}

void* inv_RW3_fn_pCreateEmptyTexture(void)
{
    RwTexture* texture;

    if (inv_RW3_p_stEmptyRaster == NULL) {
        inv_RW3_p_stEmptyRaster = RwRasterCreate(0, 0, 0, 4);
    }
    texture = RwTextureCreate(inv_RW3_p_stEmptyRaster);
    RwTextureSetFilterMode(texture, 2);
    return texture;
}

void inv_RW3_fn_vScrollObjectTexCoords(RW3ObjectView* rw3Object, GLI_tdstRenderContext* context)
{
    GeometricObjectView* first;
    GeometricObjectView* object;
    GliMaterialData* material;
    RpGeometry* geometry;
    u32 meshIndex;
    u32 index;
    s16 elementCount;

    meshIndex = 0;
    index = 0;
    first = inv_GLI_renderContext->geometricObject;
    elementCount = first->elementCount;
    if (g_stEngineStructure.bEngineFrozen == 0 && elementCount != 0) {
        do {
            object = inv_GLI_renderContext->geometricObject;
            if (object->elementTypes[index] == GEOMETRIC_ELEMENT_MESH) {
                material = object->elements[index]->material;
                if (material->frame <= gs_lActualRefreshNumberForScolledTextures && material->layers[0].texture != NULL) {
                    if (material->layers[0].flags & MATERIAL_SCROLL_ANY) {
                        geometry = object->rw3Object->atomics[meshIndex]->geometry;
                        if (geometry != NULL) {
                            inv_RW3_fn_vScrollGeometryTexCoords(geometry, object, index);
                        }
                    }
                    GLI_xRefreshAnimatedTexture(material);
                }
                meshIndex++;
                if (g_bTakeCareOfAnimationCoordination == 0) {
                    material->frame = gs_lActualRefreshNumberForScolledTextures;
                }
            }
            index++;
        } while (index < elementCount);
    }
}

s32 inv_fn_bLoadLevelTextureDictionaries(char* name)
{
    RwMemory memory;
    char path[0x80];
    char extension[0x10];
    s32 slot;
    u32 i;
    u32 levelFile;
    s32 levelLength;
    u32 file;
    u32 opened;
    s32 length;
    s32 part;
    void* dictionary;
    RwStream* stream;
    u8 pass;

    slot = -1;
    strlwr(name);
    for (i = 0; i < LEVEL_TEXTURE_DICTIONARY_COUNT; i++) {
        inv_RW3_levelTexDictionaries[i] = RwTexDictionaryCreate();
    }
    for (i = 0; i < LEVEL_TEXTURE_DICTIONARY_COUNT; i++) {
        RwTexDictionaryDestroy(inv_RW3_levelTexDictionaries[i]);
        inv_RW3_levelTexDictionaries[i] = NULL;
    }

    sprintf(path, "%s/%s%s", name, name, ".lv2");
    levelFile = inv_FMN_fn_ulOpenVirtualOrFallback(path);
    levelLength = FMN_Length(levelFile);
    FMN_Close(levelFile);
    inv_RW3_fn_vCreateTexDictionaryHeap(inv_fn_ulGetBiggestFreeBlockSize() - levelLength - 0xCCCCC);

    for (pass = 0; pass < 2; pass++) {
        if (pass != 0) {
            sprintf(extension, "lm3");
        } else {
            sprintf(extension, "rw3");
        }
        part = -1;
        do {
            part++;
            sprintf(path, "%s/%s.%s.%d", name, name, extension, part);
            printf("opening textures : %s\n", path);
            file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
            opened = file < FMN_INVALID_FILE;
            if (opened) {
                length = FMN_Length(file);
                dictionary = NULL;
                inv_MEM_fn_vCreateLevelSlotHeap(length + 0x400, 1);
                memory.start = AI_fn_p_vTrueAlloc(gp_6F74, length);
                memory.length = length;
                FMN_Read(file, memory.start, 1, length);
                FMN_Close(file);
                stream = RwStreamOpen(RW_STREAM_MEMORY, RW_STREAM_READ, &memory);
                if (stream != NULL) {
                    if (RwStreamFindChunk(stream, RW_CHUNK_TEXDICTIONARY, NULL, NULL)) {
                        inv_RW3_fn_vUseHeapForTexDictionary();
                        slot++;
                        dictionary = RwTexDictionaryStreamRead(stream);
                        inv_RW3_fn_vUseMallocForTexDictionary();
                    } else {
                        printf("ERROR -> BAD TexDictionary File : %s\n", path);
                    }
                    RwStreamClose(stream, NULL);
                } else {
                    printf("ERROR -> Cannot open TexDictionary File : %s\n", path);
                }
                inv_RW3_levelTexDictionaries[slot] = dictionary;
                inv_MEM_fn_vFreeLevelSlotHeap(1);
            }
        } while (opened || part == 0);
    }
    return 1;
}

RwTexture* inv_RW3_fn_p_stFindLevelTexture(char* name)
{
    RwTexture* texture = NULL;
    void** dictionary = inv_RW3_levelTexDictionaries;
    u32 i = 0;

    while (i < LEVEL_TEXTURE_DICTIONARY_COUNT && texture == NULL) {
        if (*dictionary != NULL) {
            texture = RwTexDictionaryFindNamedTexture(*dictionary, name);
        }
        i++;
        dictionary++;
    }
    return texture;
}

void inv_RW3_Texture_PurgeLevelResources(void)
{
    void* textures[PURGE_TEXTURE_COUNT];
    u32 i;

    RwTexDictionarySetCurrent(NULL);
    for (i = 0; i < LEVEL_TEXTURE_DICTIONARY_COUNT; i++) {
        if (inv_RW3_levelTexDictionaries[i] != NULL) {
            RwTexDictionaryDestroy(inv_RW3_levelTexDictionaries[i]);
            inv_RW3_levelTexDictionaries[i] = NULL;
        }
    }

    RwFreeListPurgeAllFreeLists();
    for (i = 0; i < PURGE_TEXTURE_COUNT; i++) {
        textures[i] = RwTextureCreate(RwRasterCreate(0x40, 0x40, 0, 0x104));
    }
    for (i = 0; i < PURGE_TEXTURE_COUNT; i++) {
        RwTextureDestroy(textures[i]);
    }

    RwFreeListPurgeAllFreeLists();
    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);
    RwCameraShowRaster(inv_GLI_currentCamera, NULL, 0);
    _sweGarbageCollectChain();
    inv_RW3_fn_vDestroyTexDictionaryHeap();
}

void inv_RW3_fn_vUseMallocForTexDictionary(void)
{
    inv_RW3_bUseTexDictionaryHeap = 0;
}

void inv_RW3_fn_vUseHeapForTexDictionary(void)
{
    inv_RW3_bUseTexDictionaryHeap = 1;
}

void* inv_RW3_fn_p_vTexDictionaryAlloc(s32 size)
{
    void* allocation;

    if (inv_RW3_bUseTexDictionaryHeap != 0) {
        allocation = inv_MEM_fn_p_vAlloc(inv_RW3_texDictionaryHeap, size);
    } else {
        allocation = malloc(size);
    }
    if (allocation == NULL && inv_RW3_bUseTexDictionaryHeap != 0) {
        allocation = malloc(size);
    }
    return allocation;
}

void inv_RW3_fn_vTexDictionaryFree(void* allocation)
{
    s32 insideHeap = 0;

    if (inv_RW3_texDictionaryHeap != NULL) {
        insideHeap = inv_MEM_fn_bIsInHeap(inv_RW3_texDictionaryHeap, allocation) != 0;
    }
    if (insideHeap) {
        inv_MEM_fn_vFreeInHeap(inv_RW3_texDictionaryHeap, allocation);
        return;
    }
    freemem(allocation);
}

void* inv_RW3_fn_p_vTexDictionaryRealloc(void* allocation, s32 size)
{
    s32 insideHeap = 0;
    void* result;

    if (inv_RW3_texDictionaryHeap != NULL)
        insideHeap = inv_MEM_fn_bIsInHeap(inv_RW3_texDictionaryHeap, allocation) != 0;
    if (!insideHeap) {
        result = realloc(allocation, size);
    } else {
        result = inv_MEM_fn_p_vRealloc(inv_RW3_texDictionaryHeap, allocation, size);
        if (result == NULL) {
            result = malloc(size);
            if (result != NULL)
                memcpy(result, allocation, inv_MEM_fn_ulGetBlockSize(inv_RW3_texDictionaryHeap, allocation));
            inv_MEM_fn_vFreeInHeap(inv_RW3_texDictionaryHeap, allocation);
        }
    }
    return result;
}

void func_0014F918(void)
{
    for (;;) {
    }
}

void inv_RW3_fn_vCreateTexDictionaryHeap(s32 size)
{
    inv_RW3_texDictionaryHeap = inv_MEM_fn_p_vAllocAligned(NULL, size, 0x10, 5);
}

void inv_RW3_fn_vDestroyTexDictionaryHeap(void)
{
    inv_MEM_fn_vFree(&inv_RW3_texDictionaryHeap);
    inv_RW3_texDictionaryHeap = NULL;
}
