#include "common.h"

#include "GAM/GAM.h"
#include "GEO/GEO.h"
#include "GLI/GLD.h"
#include "GLI/GLI.h"
#include "IPT/IPT.h"
#include "MTH/MTH.h"

#include "ps2/copy_types.h"
#include "ps2/libc.h"
#include "AI/AI.h"
#include "FMN/FMN.h"

#define LIGHT_DIRECTIONAL 1
#define LIGHT_POINT 2
#define MAX_LIGHT_CONTRIBUTIONS 16
#define LEVEL_SECTOR_SIZE 0x800
#define FMN_INVALID_FILE 15U

typedef struct LightContribution_inv {
    MTH3D_tdstVector_ direction;
    f32 intensity;
    f32 weight;
} LightContribution_inv;

typedef union RelocatedWord_inv {
    s32 value;
    u8* address;
} RelocatedWord_inv;

typedef struct Relocation_inv {
    s32 targetBlock;
    s32 offset;
} Relocation_inv;

typedef struct PointerFixup_inv {
    s32 offset;
    s32 block;
    s32 targetOffset;
    s32 targetBlock;
} PointerFixup_inv;

typedef struct SavedWord_inv {
    RelocatedWord_inv* word;
    s32 value;
} SavedWord_inv;

typedef struct SavedWordTable_inv {
    MemoryHeapView* heap;
    SavedWord_inv* words;
    u32 count;
} SavedWordTable_inv;

extern f32 D_00393EAC;
extern u8* inv_SNA_a_pLevelBlocks[];
extern SavedWordTable_inv inv_SNA_a_stSavedFixups[];

char* strlwr(char* string);
s32 FMN_Seek(s32 file, s32 offset, s32 origin);
s32 FMN_Read(s32 file, void* buffer, s32 size, s32 count);
void* memalign(s32 alignment, s32 size);
void freemem(void* block);

static inline void inv_MTH3D_fn_vCopy(void* destination, const void* source)
{
    MTH3D_tdstVector_* to = destination;
    const MTH3D_tdstVector_* from = source;

    *to = *from;
}

static inline void inv_MTH3D_fn_vSetAll(MTH3D_tdstVector_* vector, f32 value)
{
    MTH3D_M_vSetXYZ_inv(vector, value, value, value);
}

static inline void* inv_SNA_fn_p_vBytesAt(u8* block, s32 offset)
{
    return block + offset;
}

static inline s32 inv_SNA_fn_lReadWord(u8* address)
{
    RelocatedWord_inv* word = inv_SNA_fn_p_vBytesAt(address, 0);

    return word->value;
}

static inline PointerFixup_inv* inv_SNA_fn_p_stFixupsOf(u8* data)
{
    return inv_SNA_fn_p_vBytesAt(data, sizeof(s32) + inv_SNA_fn_lReadWord(data) * sizeof(Relocation_inv));
}

s32 inv_GLI_fn_bGetLightContribution(GLI_tdstLight_* light, MTH3D_tdstVector_* position, LightContribution_inv* out)
{
    s32 lit = 0;

    switch (GLI_lGetLightType(light)) {
    case LIGHT_POINT: {
        f32* center = &light->stMatrix.stMatrix.stCol_3.xX;
        f32 nearDistance;
        f32 farDistance;
        f32 falloff;
        f32 x;
        f32 y;
        f32 z;
        f32 distance;
        f32 excess;

        GLI_vGetLightNearFar(light, &nearDistance, &farDistance);
        x = position->xX - center[0];
        out->direction.xX = x;
        nearDistance *= nearDistance;
        farDistance *= farDistance;
        falloff = farDistance - nearDistance;
        farDistance = falloff;
        y = position->xY - center[1];
        out->direction.xY = y;
        z = position->xZ - center[2];
        distance = MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z);
        excess = distance - nearDistance;
        out->direction.xZ = z;
        if (excess < falloff) {
            f32 length;

            lit = 1;
            length = MTH_fn_xSqrtOpt2(distance);
            if (length != 0.0f) {
                f32 inverse = 1.0f / length;

                MTH3D_M_vSetXYZ_inv(&out->direction, inverse * x, inverse * y, inverse * z);
            }
            if (excess < 0.0f) {
                out->intensity = inv_GLI_xGetLightShadowIntensity(light);
                out->weight = 1.0f;
            } else {
                out->weight = 1.0f - excess / falloff;
                out->intensity = inv_GLI_xGetLightShadowIntensity(light);
            }
        }
        break;
    }
    case 3:
        break;
    case LIGHT_DIRECTIONAL:
        inv_MTH3D_fn_vCopy(&out->direction, &inv_GLI_p_stGetLightMatrix(light)->stMatrix.stCol_2.xX);
        out->weight = D_00393EAC;
        lit = 1;
        out->intensity = inv_GLI_xGetLightShadowIntensity(light);
        break;
    }
    return lit;
}

u32 inv_GLI_fn_ulCollectLightContributions(LightContribution_inv* contributions, u32 max, f32* total,
                  MTH3D_tdstVector_* position)
{
    GLI_tdstLight_** light = gs_aDEF_p_CurrentListOfLight;
    GLI_tdstLight_** end = light + gs_CurrentNumberOfLightInViewport;
    u8* enabled = g_ucTableOfActiveLightsForChar;
    LightContribution_inv* contribution = contributions;
    u32 count = 0;

    *total = 0.0f;
    for (; light < end && count < max; light++, enabled++) {
        if (inv_GLI_bIsLightShadowCaster(*light) && (*light)->b_lOnOff && (*light)->bValid && *enabled &&
            inv_GLI_fn_bGetLightContribution(*light, position, contribution)) {
            if (contribution->weight >= 1.0f) {
                *total = contribution->weight;
                *contributions = *contribution;
                count = 1;
                break;
            }
            *total += contribution->weight;
            contribution++;
            count++;
        }
    }
    return count;
}

void inv_GLI_fn_vGetLightDirectionAt(MTH3D_tdstVector_* position, MTH3D_tdstVector_* direction, f32* intensity)
{
    LightContribution_inv contributions[MAX_LIGHT_CONTRIBUTIONS];
    f32 total;
    u32 count = inv_GLI_fn_ulCollectLightContributions(contributions, MAX_LIGHT_CONTRIBUTIONS, &total, position);
    u32 i;
    MTH3D_tdstVector_ weighted;
    f32 value;

    *intensity = 0.0f;
    inv_MTH3D_fn_vSetAll(direction, *intensity);
    for (i = 0; i < count; i++) {
        f32 weight = contributions[i].weight / total;

        inv_MTH3D_fn_vKTimesXYZ(&weighted, weight, &contributions[i].direction);
        inv_MTH3D_fn_vAddXYZ(direction, direction, &weighted);
        *intensity += weight * contributions[i].intensity;
    }

    if (MTH3D_M_bIsZeroXYZ_inv(direction)) {
        MTH3D_M_vSetXYZ_inv(direction, 0.0f, 0.0f, -1.0f);
        value = *intensity;
        if (value == 0.0f) {
            *intensity = 128.0f;
            value = *intensity;
        }
    } else {
        f32 x = direction->xX;
        f32 y = direction->xY;
        f32 z = direction->xZ;
        f32 length = MTH_fn_xSqrtOpt2(MTH_fn_xSqrAddSqrAddSqrOpt2(x, y, z));

        if (length != 0.0f) {
            f32 inverse = 1.0f / length;

            MTH3D_M_vSetXYZ_inv(direction, inverse * x, inverse * y, inverse * z);
        }
        value = *intensity;
    }
    *intensity = value * (1.0f / 255.0f);
}

void func_00214F98(GLD_tdstViewportAttributes_* viewport, void* geometry, void* bounds, s32 drawMask)
{
    if (inv_GLI_renderContext->fogEnabled) {
        MTH3D_tdstVector_ center;
        GLI_tdstSpecificAttributesFor3D_* attributes3D;
        tdstFogParams_* fog;
        f32 depth;

        POS_fn_vMulMatrixVertex(&center, g_p_stCurrentMatrix, inv_GEO_fn_pIdentity(bounds));
        attributes3D = viewport->p_vSpecificToXD;
        POS_fn_vMulMatrixVertex(&center, &attributes3D->p_stCam->stMatrix, &center);
        depth = center.xZ - inv_GEO_fn_xGetField0C(bounds);
        fog = inv_GLI_renderContext->fogParameters;
        if (fog->xInfinite <= depth) {
            return;
        }
    }
    if (geometry != NULL) {
        GLI_xSendObjectToViewportWithLights(viewport, geometry, drawMask);
    }
}

void GLI_lSendVisualSetToViewport2(GLD_tdstViewportAttributes_* viewport, void* geometry, s32 drawMask)
{
    if (geometry != NULL) {
        GLI_xSendObjectToViewportWithLights(viewport, geometry, drawMask);
    }
}

u16 INO_fn_uwJoyPreviousDirections(stDevice* _hDev)
{
    return _hDev->m_p_stLastState->m_uwDirections;
}

u32 INO_fn_ulJoyPreviousButtons(stDevice* _hDev)
{
    return _hDev->m_p_stLastState->m_ulButtons;
}

void inv_SNA_fn_vFreeLevelBlock(void* allocation)
{
    inv_MEM_fn_vFreeInHeap(gp_6F74, allocation);
}

void* inv_SNA_fn_p_vAllocLevelBlock(u32 size)
{
    void* allocation = inv_MEM_fn_p_vAlloc(gp_6F74, size);

    memset(allocation, 0, size);
    return allocation;
}

void inv_SNA_fn_vLoadAndRelocateLevelData(char* name, s32 slot, u8** outBase, void** outEnd)
{
    char path[0x100];
    s32 file;
    u32 length;
    u32 size;
    s32 count;
    s32 savedBytes;
    u8* data;
    u32 relocationCount;
    Relocation_inv* relocations;
    PointerFixup_inv* fixups;
    SavedWord_inv* saved;
    RelocatedWord_inv* word;
    u32 i;

    *outBase = NULL;
    *outEnd = NULL;
    strlwr(name);
    sprintf(path, "%s/%s%s", name, name, ".lv2");
    file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
    length = FMN_Length(file);
    FMN_Close(file);
    size = (length >> 11) + 2;
    size *= LEVEL_SECTOR_SIZE;
    inv_MEM_fn_vCreateLevelSlotHeap(size | 0x400, slot);
    printf("alloc area buffer = %d\n", size);

    inv_SNA_a_pLevelBlocks[slot] =
        inv_FMN_fn_lOpenFileOrVirtual(path, inv_SNA_fn_p_vAllocLevelBlock, inv_SNA_fn_vFreeLevelBlock);
    count = inv_SNA_fn_lReadWord(inv_SNA_a_pLevelBlocks[slot]);
    inv_SNA_a_pLevelBlocks[slot] += sizeof(s32);
    savedBytes = count * sizeof(SavedWord_inv);
    inv_SNA_a_stSavedFixups[slot].heap = savedBytes ? inv_MEM_fn_p_vAllocAlignedHeap(savedBytes) : NULL;
    inv_SNA_a_stSavedFixups[slot].words =
        inv_SNA_a_stSavedFixups[slot].heap
            ? inv_MEM_fn_p_vAllocInHeap(inv_SNA_a_stSavedFixups[slot].heap, savedBytes)
            : NULL;
    inv_SNA_a_stSavedFixups[slot].count = count;

    strlwr(name);
    sprintf(path, "%s/%s%s", name, name, ".pt2");
    file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
    if (file >= FMN_INVALID_FILE) {
        return;
    }
    size = FMN_Length(file);
    inv_fn_vAllocateLevelMemory(size + LEVEL_SECTOR_SIZE);
    data = AI_fn_p_vTrueAlloc(gp_6F74, size);
    if (FMN_Read(file, data, 1, size) != size) {
        return;
    }
    FMN_Close(file);

    relocationCount = inv_SNA_fn_lReadWord(data);
    relocations = inv_SNA_fn_p_vBytesAt(data, sizeof(s32));
    fixups = inv_SNA_fn_p_stFixupsOf(data);
    saved = inv_SNA_a_stSavedFixups[slot].words;
    for (i = 0; i < relocationCount; i++) {
        word = inv_SNA_fn_p_vBytesAt(inv_SNA_a_pLevelBlocks[slot], relocations[i].offset);
        word->address = inv_SNA_a_pLevelBlocks[relocations[i].targetBlock] + word->value;
    }
    for (i = 0; i < count; i++) {
        word = inv_SNA_fn_p_vBytesAt(inv_SNA_a_pLevelBlocks[fixups[i].block], fixups[i].offset);
        saved->word = word;
        saved->value = word->value;
        saved++;
        word->address = inv_SNA_a_pLevelBlocks[fixups[i].targetBlock] + fixups[i].targetOffset;
    }
    *outBase = inv_SNA_a_pLevelBlocks[slot];
    *outEnd = inv_SNA_fn_p_stFixupsOf(data) + count;
}

void inv_SNA_fn_vRestoreSavedFixups(s32 slot)
{
    SavedWord_inv* saved = inv_SNA_a_stSavedFixups[slot].words;
    u32 i;

    for (i = 0; i < inv_SNA_a_stSavedFixups[slot].count; i++) {
        saved->word->value = saved->value;
        saved++;
    }
    inv_MEM_fn_vFreeLevelSlotHeap(slot);
    if (inv_SNA_a_stSavedFixups[slot].heap != NULL) {
        inv_MEM_fn_vFreeHeap(&inv_SNA_a_stSavedFixups[slot].heap);
    }
    inv_SNA_a_pLevelBlocks[slot] = NULL;
}

s32 inv_VID_fn_vBuildMoviePath(char* filename, void** out, s32* size)
{
    s32 file = inv_FMN_fn_ulOpenVirtualOrFallback(filename);

    if (file < 0) {
        printf("Cannot open %s\n", filename);
        return -1;
    }
    *size = FMN_Length(file);
    if (*size < 0) {
        printf("sceLseek() fails (%s)\n", filename);
        FMN_Close(file);
        return -1;
    }
    if (FMN_Seek(file, 0, 0) < 0) {
        printf("sceLseek() fails (%s)\n", filename);
        FMN_Close(file);
        return -1;
    }
    *out = memalign(0x40, *size);
    if (*out == NULL) {
        printf("memalign(%d) fails\n", *size);
        FMN_Close(file);
        return -1;
    }
    if (FMN_Read(file, *out, 1, *size) != *size) {
        printf("Cannot read %s\n", filename);
        FMN_Close(file);
        freemem(*out);
        return -1;
    }
    FMN_Close(file);
    return 0;
}

f32 inv_MTH_fn_xClamp(f32 value, f32 low, f32 high)
{
    f32 result = low;

    if (value < high) {
        high = value;
    }
    if (result < high) {
        result = high;
    }
    return result;
}
