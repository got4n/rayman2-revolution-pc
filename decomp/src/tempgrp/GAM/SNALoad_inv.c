#include "common.h"

#include "CAM/CAM.h"
#include "COL/COL.h"
#include "GAM/GAM.h"
#include "GLI/GLI.h"
#include "IPT/IPT.h"
#include "snd/snd.h"

#include "ps2/libc.h"
#include "FMN/FMN.h"
#include "clib/clib.h"
#include "ps2/PS2.h"
#include "VIG/VIG.h"
#include "SAI/SAI.h"
#include "SCT/SCT.h"

#define LEVEL_NAME_LENGTH 30

extern char inv_GAM_szVersionString[];
extern char inv_SND_bankFilenameBuffer[];
extern u32 D_0039322C;
extern u32 D_00393678;
extern u32 dword_252490[];
extern u32 dword_2525EC[];
extern u32 dword_2542AC[];
extern LST2_tdstAnchorTotdxHandleToNodeCameraDyn dword_2525F0;
extern LST2_tdstAnchorTotdxHandleToFamilyListDyn dword_2525FC[];
extern u8 unk_2538B0[];
extern tdstEngineStructure_ g_stEngineStructure;

s32 strcasecmp(const char* a, const char* b);
void inv_SNA_fn_vLoadAndRelocateLevelData(const char* name, s32 slot, void* base, void* end);

void inv_SNA_fn_vAlignMemoryReadPointer(void* cursor, u32 alignment)
{
    u32* address = cursor;
    u32 remainder = *address % alignment;

    if (remainder != 0) {
        *address += alignment - remainder;
    }
}

void inv_SNA_fn_pvReadMemoryArray(s32 count, s32 size, void* cursor, void* destination)
{
    u8** source = cursor;
    s32 bytes = count * size;

    memcpy(destination, *source, bytes);
    *source += bytes;
}

void inv_SNA_fn_pvReadAlignedMemory(void* cursor, void* destination, s64 size, s64 alignment)
{
    u8** source = cursor;

    inv_SNA_fn_vAlignMemoryReadPointer(source, alignment);
    memcpy(destination, *source, size);
    *source += size;
}

u16* inv_SNA_fn_pwReadFieldSizeTable(void* cursor)
{
    u8** source = cursor;
    void* start = *source;
    u16* sizes = start;

    *source += (sizes[0] + 1) * sizeof(u16);
    return sizes + 1;
}

char* inv_SND_fn_szGetBankFileName(s32 bank)
{
    sprintf(inv_SND_bankFilenameBuffer, "bnk_%d.bnm", bank);
    return inv_SND_bankFilenameBuffer;
}

void GAM_fn_vReadGameDescriptionBinaryFile(void)
{
    void* data;
    void* relocations;
    u16* sizes;
    u32 i = 0;

    inv_SNA_fn_vLoadAndRelocateLevelData("Fix", 0, &data, &relocations);
    sizes = inv_SNA_fn_pwReadFieldSizeTable(&relocations);
    inv_SNA_fn_vAlignMemoryReadPointer(&data, 4);
    inv_SNA_fn_pvReadMemoryArray(sizes[0x4B], 1, &data, unk_2538B0);
    inv_SNA_fn_vAlignMemoryReadPointer(&data, 4);
    inv_SNA_fn_pvReadMemoryArray(sizes[0xC3], 0x10, &data, inv_IPT_a_pEntryElementRecords);
    inv_SNA_fn_pvReadMemoryArray(sizes[0x4C], 1, &data, &IPT_g_hInputStructure);
    inv_SNA_fn_pvReadMemoryArray(sizes[0x9A], 1, &data, &inv_TXT_textTableState);
    inv_SNA_fn_pvReadAlignedMemory(&relocations, &g_stEngineStructure.ucNumberOfLevels, 1, 1);
    for (; i < g_stEngineStructure.ucNumberOfLevels; i++) {
        inv_SNA_fn_pvReadAlignedMemory(&relocations, g_stEngineStructure.a_szLevelName[i],
                                       LEVEL_NAME_LENGTH, 1);
    }
    g_stEngineStructure.ucCurrentLevel = 0;
    fn_vSetLevelName(g_stEngineStructure.a_szLevelName[0]);
    fn_vSetNextLevelName(g_stEngineStructure.a_szLevelName[0]);
    fn_vFreeLevelAllowedMemory();
}

void inv_fn_vLoadLevelDataFiles(const char* levelName, const char* variant1, const char* variant2)
{
    char labelSoundBuffer[256];
    char path[256];
    char versionStr[32];
    u16* fieldSizeTable;
    u32* ptr_234;
    u32* ptr_238;
    u32* ptr_240;
    u32* ptr_244;
    u32* ptr_248;
    u32* dataPtr;
    u32* relocPtr;
    u32 var_228;
    u32 var_22C;
    u32 var_230;
    u32 var_234;
    u32 var_238;
    u32 var_23C;
    u32 var_240;
    u32 var_244;
    u32 var_248;
    u32* basePtr;
    u32* readBase1;
    u32* scanBase;
    u32 headerWord0, headerWord1;
    tdstNodeCamera_* node1;
    tdstFamilyListView* node2;
    tdstAlwaysActiveCharacter_* node3;
    u32 i;
    ShadowQueue_inv* shadowQueue;
    void* queueBase;
    char *s0, *secondCapacityLabel;
    tdstEngineStructure_* p_stEngine;

    strcpy(path, levelName);
    if (strcasecmp(variant1, "default") != 0) {
        Strcat(path, "_");
        Strcat(path, variant1);
    }
    if (strcasecmp(variant2, "default") != 0) {
        Strcat(path, "__");
        Strcat(path, variant2);
    }

    if (D_0039378C == 0) {
        inv_VIG_fn_vDisplayStartupImage(levelName, g_lVIDEO_WantedWidth, g_lVIDEO_WantedHeight, 1);
    }

    inv_FMN_fn_lMountSoundBanks();
    inv_fn_bLoadLevelTextureDictionaries(path);
    inv_ATO_fn_bLoadLevelAtomicClump(path);
    inv_SNA_fn_vLoadAndRelocateLevelData(path, 1, &dataPtr, &relocPtr);

    fieldSizeTable = inv_SNA_fn_pwReadFieldSizeTable(&relocPtr);
    basePtr = dataPtr;
    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0x4D], 0x20, &dataPtr, versionStr);

    sprintf(inv_GAM_szVersionString, "%s V%dp%d", versionStr, 31, 6);
    Strcat(inv_GAM_szVersionString, " ALT");
    Strcat(inv_GAM_szVersionString, " NTSC");
    printf("\nFound version : %s\n", inv_GAM_szVersionString);

    headerWord0 = *dataPtr++;
    headerWord1 = *dataPtr++;

    if (headerWord0 != 0x1C2FFFFF || headerWord1 != 0x0C000000) {
        printf("Wrong binarized data version :\n");
        printf("\tData %08x/%08x -- Engine %08x/%08x : this can not work !!\n", headerWord0,
               headerWord1, 0x1C2FFFFF, 0x0C000000);
        printf("Stopping the program...\n");
        while (1) {
        }
    }

    printf("ControlVersion OK\n\n");

    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &var_228, 4, 4);
    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &var_22C, 4, 4);
    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &var_230, 4, 4);

    readBase1 = dataPtr;
    dword_2542AC[0] = readBase1[0];
    dataPtr = readBase1 + 1;
    g_hCollisionGeoObj = (void*) readBase1[1];
    dataPtr = readBase1 + 2;
    g_hStaticCollisionGeoObj = (void*) readBase1[2];
    dataPtr = readBase1 + 3;
    inv_SNA_fn_vAlignMemoryReadPointer(&dataPtr, 4);

    {
        u32* a20 = dataPtr;
        u32* a21 = a20 + 1;
        u32* a22 = a21 + 1;
        u32* a23 = a22 + 1;
        gp_stActualWorld = (struct HIE_tdstSuperObject_*) a20[0];
        dataPtr = a21;
        gp_stDynamicWorld = (HIE_tdstSuperObject_*) a21[0];
        dataPtr = a22;
        SECT_hFatherSector = (HIE_tdstSuperObject_*) a22[0];
        dataPtr = a23;
    }
    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0x96], 1, &dataPtr, dword_252490);

    {
        u32* a30 = dataPtr;
        u32* a31 = a30 + 1;
        u32* a32 = a31 + 1;
        u32* a33 = a32 + 1;
        s0 = (char*) dword_2525EC;
        D_0039322C = a30[0];
        dataPtr = a31;
        g_hCharacterLauchingSoundEvents = (HIE_tdstSuperObject_*) a31[0];
        dataPtr = a32;
        *(u32*) s0 = a32[0];
        dataPtr = a33;
    }
    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0xA1], 1, &dataPtr, s0 + 4);

    node1 = *(tdstNodeCamera_**) (s0 + 4);
    while (node1 != NULL) {
        node1->hFatherDyn = &dword_2525F0;
        node1 = node1->hNextBrotherDyn;
    }

    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0x33], 1, &dataPtr, dword_2525FC);
    node2 = dword_2525FC[0].hFirstElementDyn;
    while (node2 != NULL) {
        node2->hFatherDyn = dword_2525FC;
        node2 = node2->hNextBrotherDyn;
    }

    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0xA3], 1, &dataPtr, inv_GAM_hAlwaysActiveCharactersList);
    node3 = inv_GAM_hAlwaysActiveCharactersList[0].hFirstElementDyn;
    while (node3 != NULL) {
        node3->hFatherDyn = inv_GAM_hAlwaysActiveCharactersList;
        node3 = node3->hNextBrotherDyn;
    }

    ptr_234 = &var_234;
    ptr_238 = &var_238;

    p_stEngine = (tdstEngineStructure_*) dword_252618;
    secondCapacityLabel = (char*) dword_355570;
    queueBase = secondCapacityLabel - 0x10F0;
    shadowQueue = queueBase;

    dword_252618[0] = (HIE_tdstSuperObject_*) *dataPtr;
    p_stEngine = (tdstEngineStructure_*) ((char*) p_stEngine - 0x168);
    p_stEngine->initialSuperObject = p_stEngine->activeSuperObject;
    dataPtr++;
    p_stEngine->pendingSuperObject = (HIE_tdstSuperObject_*) *dataPtr++;
    p_stEngine->h_StdCamCaracter = (HIE_tdstSuperObject_*) *dataPtr++;
    inv_SNA_fn_vAlignMemoryReadPointer(&dataPtr, 4);

    COL_g_d_lTaggedFacesTable = (s32*) *dataPtr++;
    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0xA9], 1, &dataPtr, &SAI_g_stInternalStructure);

    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, ptr_234, 4, 4);
    inv_SNA_p_hLevelGeometricObjects = dataPtr;
    dataPtr += var_234;

    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, ptr_238, 4, 4);
    inv_RW3_p_pLevelMaterials = (GliMaterialData**) dataPtr;
    dataPtr += var_238;

    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &var_23C, 4, 4);
    inv_RW3_p_stLevelTextures = dataPtr;
    dataPtr += var_23C * 2;
    inv_RW3_ulLevelTextureCount = var_23C - 1;

    ptr_240 = &var_240;
    ptr_244 = &var_244;
    ptr_248 = &var_248;

    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, ptr_240, 4, 4);

    {
        u32* c0 = dataPtr;
        u32* c1 = c0 + 1;
        u32* c2 = c1 + 1;
        u32* c3 = c2 + 1;
        u32* c4 = c3 + 1;
        u32* c5 = c4 + 1;
        inv_RW3_p_pLevelLights = (GLI_tdstLight_**) c0[0];
        dataPtr = c1;
        inv_MEC_p_stFirstEffectSystem = (EffectSystem_inv*) c1[0];
        dataPtr = c2;
        inv_MEC_hParticleSuperObject = (void*) c2[0];
        dataPtr = c3;
        g_AlphabetCharacterPointer = (tdstEngineObject_*) c3[0];
        dataPtr = c4;
        dword_355570[0] = c4[0];
        dataPtr = c5;
    }
    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0x16], shadowQueue->secondCapacity, &dataPtr,
                                 (u32*) shadowQueue->secondMaterials);

    {
        shadowQueue->firstCapacity = *dataPtr++;
        inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0x16], shadowQueue->firstCapacity, &dataPtr,
                                     (u32*) shadowQueue->firstMaterials);
    }

    p_stEngine->ucNbLanguages = 4;
    g_bBeginMapSoundEventFlag = 0;
    {
        u32* d0 = dataPtr;
        u32* d1 = d0 + 1;
        g_stBeginMapSoundEvent = (u32*) d0[0];
        dataPtr = d1;
        if (g_stBeginMapSoundEvent != 0) {
            g_bBeginMapSoundEventFlag = 1;
        }
    }

    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, ptr_244, 4, 4);
    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &inv_SND_ulLevelBank, 4, 4);

    stop_son_all();
    SND_fn_vGetTargetLabelSound(labelSoundBuffer, 0x100);
    j_j_j_j_nullsub_22(0);
    SND_fn_vInitBankSnd(inv_SND_fn_szGetBankFileName(inv_SND_ulLevelBank));
    if (SND_fn_bStartLoadBanks2(-1) == 1) {
        if (SND_fn_bInitBank2(inv_SND_ulLevelBank, -1) == 1) {
            SND_fn_bEndLoadBanks();
        }
    }

    {
        char* latePtr;
        i = 0;
        latePtr = "----------------------------------------------------------\n";
        printf(latePtr);
        printf("compteur de blocs debut de maps %d\n", inv_SND_ulIopBlockCount);
        printf(latePtr);

        inv_SNA_fn_pvReadAlignedMemory(&relocPtr, ptr_248, 4, 4);

        scanBase = relocPtr;
        relocPtr = scanBase + var_248;
        if (var_248 != 0) {
            latePtr = (char*) scanBase;
            do {
                i++;
                *(u32*) ((u8*) basePtr + *(u32*) latePtr) =
                    (u32) SND_fn_pGetBinEvent(*(u32*) ((u8*) basePtr + *(u32*) latePtr));
                latePtr += 4;
            } while (i < var_248);
        }
    }

    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0xC2], 1, &dataPtr, &inv_COL_stLevelOctree1);
    inv_SNA_fn_pvReadMemoryArray(fieldSizeTable[0xC2], 1, &dataPtr, &inv_COL_stLevelOctree2);

    D_00393678 = *dataPtr++;
    if (D_00393678 != 0) {
        inv_RW3_p_stLightHalos = (struct LightHaloEntry*) dataPtr;
        dataPtr += D_00393678 * 8;
    } else {
        inv_RW3_p_stLightHalos = NULL;
    }

    if (D_00393678 != 0) {
        inv_RW3_p_ucQueuedHalos = (u8*) dataPtr;
        dataPtr = (void*) ((u8*) dataPtr + D_00393678);
        inv_SNA_fn_vAlignMemoryReadPointer(&dataPtr, 4);
    } else {
        inv_RW3_p_ucQueuedHalos = NULL;
    }

    {
        u32* d84p = dataPtr;
        u32 d84v = d84p[0];
        dataPtr = d84p + 1;
        inv_GLI_p_stHaloMaterial = (GliMaterialData*) d84v;
    }
    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &inv_GAM_textIdBase, 4, 4);
    inv_SNA_fn_pvReadAlignedMemory(&relocPtr, &inv_RW3_ulLightmapSlotCount, 4, 4);
    if (inv_RW3_ulLightmapSlotCount != 0) {
        inv_RW3_lightmapSlots = (LightmapSlot*) dataPtr;
        dataPtr += inv_RW3_ulLightmapSlotCount * 4;
    } else {
        inv_RW3_lightmapSlots = NULL;
    }

    fn_vFreeLevelAllowedMemory();
}

void inv_SNA_fn_vDelocateMemoryBlocks(void)
{
    inv_SNA_fn_vRestoreSavedFixups(1);
}
