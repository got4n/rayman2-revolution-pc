#include "common.h"

#include "GAM/GAM.h"
#include "SVG/SVG.h"

#include "ps2/libc.h"
#include "SAI/SAI.h"
#include "AI/AI.h"
#include "ps2/PS2.h"

#define SAVE_DEVICE_FORMATTED 2
#define SAVE_FULL_VOLUME 100
#define NO_LEVEL 0xFF

extern char byte_393598[16];

void inv_fn_lGetCdClockSeconds(u32* seconds);
s32 inv_fn_bSaveGameWithSlotCheck(SaveSlot* slot, void* data, s32 size, s32 neededSize);
s32 inv_SAV_fn_bSaveGameWithCurrentSlot(SaveSlot* slot, void* data, s32 size);
s32 inv_fn_bLoadSaveGameData(SaveSlot* slot, void* data, s32 size);
void inv_fn_vBuildSaveDirectoryName(char* directory, const char* name);
void inv_fn_vBuildSaveDataPath(char* path, const char* name);

void inv_SIF_fn_vInitSoundInfos(void)
{
    inv_SAV_stGameData.recordCount = 0;
    inv_SAV_stGameData.streamSize = 0;
    inv_SAV_stGameData.volumes[0] = SAVE_FULL_VOLUME;
    inv_SAV_stGameData.volumes[1] = SAVE_FULL_VOLUME;
    inv_SAV_stGameData.volumes[2] = SAVE_FULL_VOLUME;
}

u16 inv_SAV_fn_uwMakeSaveKey(s32 value)
{
    char* levelName;
    s32 levelIndex = NO_LEVEL;

    levelName = fn_p_szGetLevelName();
    if (strlen(levelName) != 0 && (value & 0xFE00) == 0) {
        levelIndex = inv_fn_ucFindLevelNameIndex(levelName);
    }
    return (levelIndex << 9) | (value & 0xFFF);
}

u8* inv_SAV_fn_p_ucGetOrCreateSaveRecord(s32 id)
{
    u16 key;
    u8* data;
    SaveRecordEntry* record;

    key = inv_SAV_fn_uwMakeSaveKey(id);
    data = inv_SAV_fn_p_ucFindSaveRecord(id);
    if (data == NULL) {
        record = &inv_SAV_stGameData.records[inv_SAV_stGameData.recordCount];
        record->key = key;
        record->offset = inv_SAV_stGameData.streamSize;
        data = &inv_SAV_stGameData.stream[record->offset];
        inv_SAV_stGameData.recordCount++;
    }
    return data;
}

u8* inv_SAV_fn_p_ucFindSaveRecord(s32 id)
{
    u16 key;
    u16 i;
    SaveRecordEntry* record;

    key = inv_SAV_fn_uwMakeSaveKey(id);
    record = inv_SAV_stGameData.records;
    for (i = 0; i < inv_SAV_stGameData.recordCount; i++, record++) {
        if (record->key == key) {
            return &inv_SAV_stGameData.stream[record->offset];
        }
    }
    return NULL;
}

u8* inv_SAV_fn_p_ucWriteAligned(u8* cursor, void* source, s32 size)
{
    u16 used;

    memcpy(cursor, source, size);
    cursor += size;
    if (size & 3) {
        cursor += 4 - (size & 3);
    }
    used = cursor - inv_SAV_stGameData.stream;
    if (inv_SAV_stGameData.streamSize < used) {
        inv_SAV_stGameData.streamSize = used;
    }
    return cursor;
}

u8* inv_SAV_fn_p_ucReadAligned(u8* cursor, void* destination, u32 length)
{
    memcpy(destination, cursor, length);
    cursor += length;
    if (length & 3) {
        cursor += 4 - (length & 3);
    }
    return cursor;
}

void inv_SAV_fn_vWriteHeader(s32 offset, void* source, u32 size)
{
    memcpy(inv_SAV_stGameData.header + offset, source, size);
}

void inv_SAV_fn_vReadHeader(s32 offset, void* destination, s32 size)
{
    memcpy(destination, inv_SAV_stGameData.header + offset, size);
}

SaveSlot* inv_SAV_fn_p_stGetSlotByIndex(s16 id)
{
    SaveSlot* slot;

    for (slot = inv_SAV_p_stDirectorySlots; slot != NULL; slot = slot->next) {
        if (slot->id == id) {
            return slot;
        }
    }
    return NULL;
}

char* inv_SAV_fn_szGetSlotPlayerName(SaveSlot* slot)
{
    if (slot != NULL) {
        return slot->info.playerName;
    }
    return NULL;
}

s32 inv_SAV_fn_uwGetSlotIndex(SaveSlot* slot)
{
    s32 value = 0;

    if (slot != NULL) {
        value = slot->info.slotIndex;
    }
    return value;
}

char* inv_SAV_fn_szGetSlotComment(SaveSlot* slot)
{
    if (slot != NULL) {
        return slot->info.comment;
    }
    return NULL;
}

u32 inv_SAV_fn_ulGetSlotPlayTime(SaveSlot* slot)
{
    u32 value = 0;

    if (slot != NULL) {
        value = slot->info.gameDuration;
    }
    return value;
}

SaveSlot* inv_SAV_fn_p_stAllocateSaveSlot(s16 id, char* name)
{
    SaveSlot* slot;

    slot = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(SaveSlot));
    if (slot != NULL) {
        slot->id = id;
        slot->info.modificationTime = 0;
        slot->info.valid = 0;
        slot->info.slotIndex = 0;
        slot->next = NULL;
        memset(slot->info.name, 0, sizeof(slot->info.name));
        memset(slot->info.playerName, '-', sizeof(slot->info.playerName));
        memset(slot->info.comment, '.', sizeof(slot->info.comment));
        strcpy(slot->info.name, name);
        slot->info.playerName[0] = byte_393598[0];
        slot->info.comment[0] = byte_393598[0];
        slot->info.gameDuration = 0;
    }
    return slot;
}

s32 inv_SAV_fn_bSaveCurrentGame(SaveSlot* slot)
{
    if (inv_fn_bCanSaveGame(SAI_fn_ucPlayerSaveValues()) != 0) {
        return inv_fn_bSaveGameWithSlotCheck(slot, &inv_SAV_stGameData, sizeof(SaveGameData),
                                             sizeof(SaveGameData));
    }
    return 0;
}

s32 inv_SAV_fn_bSaveSelectedSlot(SaveSlot* slot)
{
    if (inv_fn_bIsSaveSlotValid(&slot->info)) {
        return inv_SAV_fn_bSaveGameWithCurrentSlot(slot, &inv_SAV_stGameData, sizeof(SaveGameData));
    }
    return 0;
}

s32 inv_SAV_fn_bLoadSlotData(SaveSlot* slot)
{
    SaveGameData* buffer;
    s32 loaded;

    if (inv_fn_bIsSaveSlotValid(&slot->info) == 0) {
        return 0;
    }
    buffer = AI_fn_p_vTrueAlloc(gp_6F74, sizeof(SaveGameData));
    loaded = inv_fn_bLoadSaveGameData(slot, buffer, sizeof(SaveGameData));
    if (loaded != 0) {
        inv_SAV_stGameData = *buffer;
    }
    inv_MEM_fn_vFreeInHeap(gp_6F74, buffer);
    return loaded;
}

s32 inv_SAV_fn_bDeleteSlot(SaveSlot* slot)
{
    if (inv_fn_bIsSaveSlotValid(&slot->info)) {
        return inv_fn_bDeleteSaveGame(slot);
    }
    return 0;
}

s32 inv_SAV_fn_bCanSaveCurrentGame(void)
{
    return inv_fn_bCanSaveGame(SAI_fn_ucLoadPlayerValues());
}

void inv_fn_vBuildSaveDirectoryName(char* directory, const char* name)
{
    sprintf(directory, "BASLUS-20138%s", name);
}

void j_sprintf_SaveDirectoryName(char* directory, const char* name)
{
    inv_fn_vBuildSaveDirectoryName(directory, name);
}

void inv_fn_vBuildSaveDataPath(char* path, const char* name)
{
    char directory[0x70];

    inv_fn_vBuildSaveDirectoryName(directory, name);
    sprintf(path, "%s/%s", directory, directory);
}

s32 inv_SAV_fn_lCanSaveGame(s32 size)
{
    s32 canSave = inv_fn_bCanSaveGame(size);
    s32 result = -1;

    if (canSave != 0) {
        result = 0;
    }
    return result;
}

s32 inv_SAV_fn_lIsUnformattedDeviceReady(void)
{
    if (inv_SAV_fn_bIsSaveDeviceReady() != 0 && inv_SAV_fn_bIsSaveDeviceFormatted() == 0) {
        return 0;
    }
    return -1;
}

s32 inv_SAV_fn_bIsSaveDeviceReady(void)
{
    return SVG_fn_s32Query(0, 0, NULL) != 0;
}

s32 inv_SAV_fn_bIsSaveDeviceFormatted(void)
{
    return SVG_fn_s32Query(0, 0, NULL) == SAVE_DEVICE_FORMATTED;
}

s32 inv_fn_bCanSaveGame(s32 size)
{
    s32 freeSpace;
    s32 canSave = 0;

    if (SVG_fn_s32Query(0, 0, &freeSpace) == SAVE_DEVICE_FORMATTED) {
        canSave = freeSpace >= func_00140840(0, 0, size);
    }
    return canSave;
}

s32 inv_fn_bIsSaveSlotValid(SaveSlotInfo* info)
{
    char directory[0x40];
    s32 handle = 0;

    if (info->valid != 0 && strlen(info->name) != 0) {
        j_sprintf_SaveDirectoryName(directory, info->name);
        handle = inv_SVG_fn_lEnumerateSaveDirectory(0, 0, directory);
    }
    if (handle == 0) {
        return 0;
    }
    return handle == info->modificationTime;
}

u8 inv_fn_bSaveGameToSlot(SaveSlot* slot, void* data, s32 size, u8 newName)
{
    char directory[0x80];
    u32 seconds;
    char* name;
    char* c;

    if (newName != 0) {
        inv_fn_lGetCdClockSeconds(&seconds);
        name = slot->info.name;
        c = name;
        sprintf(name, "%08u", seconds);
        for (; *c != '\0'; c++) {
            *c += 0x31;
        }
    } else {
        name = slot->info.name;
    }
    j_sprintf_SaveDirectoryName(directory, name);
    slot->info.modificationTime =
        SVG_fn_u32SaveGame(0, 0, directory, slot->info.playerName, slot->info.comment,
                           slot->info.slotIndex, slot->info.gameDuration, data, size);
    slot->info.valid = slot->info.modificationTime != 0;
    return slot->info.valid;
}

s32 inv_fn_bSaveGameWithSlotCheck(SaveSlot* slot, void* data, s32 size, s32 neededSize)
{
    s32 result;

    if (inv_fn_bIsSaveSlotValid(&slot->info) != 0) {
        result = inv_fn_bSaveGameToSlot(slot, data, size, 0);
    } else if (inv_SAV_fn_lCanSaveGame(neededSize) != -1) {
        result = inv_fn_bSaveGameToSlot(slot, data, size, 1);
    } else {
        result = 0;
    }
    return result;
}

s32 inv_SAV_fn_bSaveGameWithCurrentSlot(SaveSlot* slot, void* data, s32 size)
{
    return inv_fn_bSaveGameWithSlotCheck(slot, data, size, size);
}

s32 inv_fn_bLoadSaveGameData(SaveSlot* slot, void* data, s32 size)
{
    char path[0x100];

    inv_fn_vBuildSaveDataPath(path, slot->info.name);
    return SVG_fn_s32LoadFromFile(0, 0, path, data, size) != 0;
}
