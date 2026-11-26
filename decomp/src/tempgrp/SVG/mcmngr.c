#include "common.h"

#include "SVG/SVG.h"

#include "ps2/libc.h"
#include "TMR/TMR.h"
#include "SAI/SAI.h"
#include "GAM/GAM.h"

#define MC_TRIES 3
#define MC_ALIGNMENT 0x40
#define MC_ATTRIBUTE_SUBDIRECTORY 0x20
#define SAVE_BLOCK_SIZE 1024
#define SAVE_DATA_SIZE 0x9D7C
#define ICON_SYS_SIZE 0x3C4
#define ICON_COLOR_FULL 0x80
#define ICON_TITLE_LINE_BREAK 0x20
#define ICON_ENTRY_AT(table, offset) ((SaveIconEntry*) ((u8*) (table) + (offset)))
#define ICON_ENTRY_OF_DATA(data) ((SaveIconEntry*) ((u8*) (data) - 8))

#define ALIGNED_STACK_BUFFER(size)                                                                 \
    ((void*) (((u32) __builtin_alloca((size) + MC_ALIGNMENT - 1) + MC_ALIGNMENT - 1) &             \
              ~(MC_ALIGNMENT - 1)))

typedef struct IconSysPayloadFields {
    u32 BgColor[4][4];
    u64 LightDir[6];
    u64 LightColor[6];
    u64 Ambient[2];
} __attribute__((packed)) IconSysPayloadFields;

typedef union IconSysPayload {
    IconSysPayloadFields fields;
    u8 bytes[0xB0];
} __attribute__((packed)) IconSysPayload;

typedef struct IconSysFile {
    char Head[4];
    u16 Reserv1;
    u16 OffsLF;
    u32 Reserv2;
    u32 TransRate;
    IconSysPayload payload;
    char TitleName[68];
    char FnameView[0x40];
    char FnameCopy[0x40];
    char FnameDel[0x40];
    u8 Reserve3[0x200];
} IconSysFile;

typedef struct SaveDescription {
    char comment[0x20];
    char playerName[0x20];
    u32 gameDuration;
    u16 completion;
    u8 unk46[2];
} __attribute__((packed)) SaveDescription;

extern u64 inv_SVG_a_xIconLightDir[6];
extern u64 inv_SVG_a_xIconLightColor[6];
extern u64 inv_SVG_a_xIconAmbient[2];

char* strlwr(char* string);
char* strupr(char* string);
#ifdef PC_PORT
#include "PC/PC.h"

#define MC_ATTRIBUTES_DIRECTORY 0x8427
#define MC_ATTRIBUTES_FILE 0x8497
#define MC_LIST_MAX 0x63

static void inv_SVG_fn_vWriteCardDate(u8* out, PcDate* date)
{
    u16 year = 2000 + date->year;

    out[0] = 0;
    out[1] = date->second;
    out[2] = date->minute;
    out[3] = date->hour;
    out[4] = date->day;
    out[5] = date->month;
    out[6] = year & 0xFF;
    out[7] = year >> 8;
}

s32 inv_SVG_fn_lListCard(char* path, s32 maxEntries, void* table)
{
    PcSaveEntry found[MC_LIST_MAX];
    sceMcTblGetDir* entry = table;
    char* last = path;
    char* c;
    s32 inDirectory = 0;
    s32 wildcards = 0;
    s32 count;
    s32 total = 0;
    s32 i;

    for (c = path; *c != '\0'; c++) {
        if (*c == '/') {
            inDirectory = 1;
            last = c + 1;
        }
    }
    for (c = last; *c != '\0'; c++) {
        if (*c == '*' || *c == '?') {
            wildcards = 1;
        }
    }

    if (inDirectory && wildcards) {
        for (i = 0; i < 2 && total < maxEntries; i++, total++, entry++) {
            memset(entry, 0, sizeof(sceMcTblGetDir));
            entry->AttrFile = MC_ATTRIBUTES_DIRECTORY;
            strcpy(entry->EntryName, i == 0 ? "." : "..");
        }
    }

    count = pc_save_list(path, found, maxEntries - total < MC_LIST_MAX ? maxEntries - total : MC_LIST_MAX);
    if (count < 0) {
        return -1;
    }
    for (i = 0; i < count; i++, total++, entry++) {
        memset(entry, 0, sizeof(sceMcTblGetDir));
        inv_SVG_fn_vWriteCardDate((u8*) &entry->_Create, &found[i].created);
        inv_SVG_fn_vWriteCardDate((u8*) &entry->_Modify, &found[i].modified);
        entry->FileSizeByte = found[i].size;
        entry->AttrFile = found[i].isDirectory ? MC_ATTRIBUTES_DIRECTORY : MC_ATTRIBUTES_FILE;
        strncpy(entry->EntryName, found[i].name, sizeof(entry->EntryName) - 1);
    }
    return total;
}
#else
s32 sceMcDelete(s32 port, s32 slot, char* path);
s32 sceMcGetDir(s32 port, s32 slot, char* path, s32 mode, s32 maxEntries, sceMcTblGetDir* entries);
s32 sceMcMkdir(s32 port, s32 slot, char* path);
s32 sceMcSync(s32 mode, s32* command, s32* result);
#endif
s32 SVG_fn_s32MakeDir(s32 port, s32 slot, char* directory);
char* inv_SVG_fn_szGetSaveNameSuffix(char* name);

s32 SVG_fn_s32CreateIconFilesForSave(s32 _iPort, s32 _iSlot, char* _szSonyCompliantDirName, char* _szTitleName,
                                     char* comment, u16 completion, u32 gameDuration, u8 writeIcons)
{
    IconSysFile* icon;
    SaveDescription description;
    char path[0x40];
    char title[0x20];
    char time[0x20];
    IconSysPayload payload;
    SaveIconEntry* table;
    s32 offset;

    icon = ALIGNED_STACK_BUFFER(sizeof(IconSysFile));

    memset(payload.bytes, 0, 0x40);
    payload.fields.BgColor[0][0] = ICON_COLOR_FULL;
    payload.fields.BgColor[1][1] = ICON_COLOR_FULL;
    payload.fields.BgColor[2][2] = ICON_COLOR_FULL;
    payload.fields.BgColor[3][0] = ICON_COLOR_FULL;
    payload.fields.BgColor[3][1] = ICON_COLOR_FULL;
    payload.fields.BgColor[3][2] = ICON_COLOR_FULL;
    payload.fields.LightDir[0] = inv_SVG_a_xIconLightDir[0];
    payload.fields.LightDir[1] = inv_SVG_a_xIconLightDir[1];
    payload.fields.LightDir[2] = inv_SVG_a_xIconLightDir[2];
    payload.fields.LightDir[3] = inv_SVG_a_xIconLightDir[3];
    payload.fields.LightDir[4] = inv_SVG_a_xIconLightDir[4];
    payload.fields.LightDir[5] = inv_SVG_a_xIconLightDir[5];
    payload.fields.LightColor[0] = inv_SVG_a_xIconLightColor[0];
    payload.fields.LightColor[1] = inv_SVG_a_xIconLightColor[1];
    payload.fields.LightColor[2] = inv_SVG_a_xIconLightColor[2];
    payload.fields.LightColor[3] = inv_SVG_a_xIconLightColor[3];
    payload.fields.LightColor[4] = inv_SVG_a_xIconLightColor[4];
    payload.fields.LightColor[5] = inv_SVG_a_xIconLightColor[5];
    payload.fields.Ambient[0] = inv_SVG_a_xIconAmbient[0];
    payload.fields.Ambient[1] = inv_SVG_a_xIconAmbient[1];

    memset(icon, 0, sizeof(IconSysFile));
    memset(&description, 0, sizeof(description));

    __builtin_memcpy(icon, "PS2D", 5);
    sprintf(title, "%-11s %3d%%", "Rayman-Rev.", completion);
    SVG_fn_vAsciiToSJIS(icon->TitleName, 0x10, title);
    sprintf(time, "%d:%02d:%02d", gameDuration / 3600, gameDuration / 60 % 60, gameDuration % 60);
    sprintf(title, "%-6s %9s", _szTitleName, time);
    strupr(title);
    SVG_fn_vAsciiToSJIS(icon->TitleName + ICON_TITLE_LINE_BREAK, 0x10, title);
    icon->OffsLF = ICON_TITLE_LINE_BREAK;
    strcpy(description.playerName, _szTitleName);
    strcpy(description.comment, comment);
    description.completion = completion;
    description.gameDuration = gameDuration;
    icon->TransRate = 0x60;
    memcpy(&icon->payload.bytes[0x00], &payload.bytes[0x00], 0x40);
    memcpy(&icon->payload.bytes[0x40], &payload.bytes[0x40], 0x30);
    memcpy(&icon->payload.bytes[0x70], &payload.bytes[0x70], 0x30);
    memcpy(&icon->payload.bytes[0xA0], &payload.bytes[0xA0], 0x10);

    table = off_3313D8;
    strcpy(icon->FnameView, table[0].path);
    offset = sizeof(SaveIconEntry);
    if (table[1].size == 0) {
        offset = 0;
    }
    strcpy(icon->FnameCopy, ICON_ENTRY_AT(table, offset)->path);
    offset = 2 * sizeof(SaveIconEntry);
    if (table[2].size == 0) {
        offset = 0;
    }
    strcpy(icon->FnameDel, ICON_ENTRY_AT(table, offset)->path);

    sprintf(path, "%s/icon.sys", _szSonyCompliantDirName);
    if (SVG_fn_s32WriteToFile(_iPort, _iSlot, path, icon, sizeof(IconSysFile)) != 1) {
        return 0;
    }

    if (writeIcons != 0) {
        void** data;
        s32 i;

        data = &table[0].data;
        for (i = 0; i < SAVE_ICON_COUNT; i++, data += 3) {
            if (ICON_ENTRY_OF_DATA(data)->size == 0 || *data == NULL) {
                continue;
            }
            sprintf(path, "%s/%s", _szSonyCompliantDirName, ICON_ENTRY_OF_DATA(data)->path);
            if (SVG_fn_s32WriteToFile(_iPort, _iSlot, path, *data, ICON_ENTRY_OF_DATA(data)->size) !=
                1) {
                return 0;
            }
        }
    }

    sprintf(path, "%s/data.add", _szSonyCompliantDirName);
    return SVG_fn_s32WriteToFile(_iPort, _iSlot, path, &description, sizeof(description)) == 1;
}

s32 func_00140840(s32 unused0, s32 unused1, s32 saveBytes)
{
    s32 blocks;
    s32 i;

    blocks = (func_0013F998() + 4) / 2;
    blocks += 2;
    for (i = 0; i < SAVE_ICON_COUNT; i++) {
        blocks += (off_3313D8[i].size + (SAVE_BLOCK_SIZE - 1)) / SAVE_BLOCK_SIZE;
    }
    blocks += (saveBytes + (SAVE_BLOCK_SIZE - 1)) / SAVE_BLOCK_SIZE;
    return blocks + 3;
}

#ifdef PC_PORT
s32 SVG_fn_s32MakeDir(s32 _iPort, s32 _iSlot, char* _szSonyCompliantDirName)
{
    return pc_save_make_directory(_szSonyCompliantDirName);
}
#else
s32 SVG_fn_s32MakeDir(s32 _iPort, s32 _iSlot, char* _szSonyCompliantDirName)
{
    s32 u32RetryCount;
    s32 iResult;

    gs_bSceMcOpIsSuccessful = 0;
    u32RetryCount = MC_TRIES;
    while (u32RetryCount != 0) {
        gs_bSceMcOpIsSuccessful = sceMcMkdir(_iPort, _iSlot, _szSonyCompliantDirName) == 0;
        if (gs_bSceMcOpIsSuccessful) {
            break;
        }
        if (--u32RetryCount) {
            fn_vTimerWait(1);
        }
    }
    if (gs_bSceMcOpIsSuccessful != 0) {
        sceMcSync(0, NULL, &iResult);
        return iResult == 0;
    }
    return 0;
}
#endif

#ifdef PC_PORT
s32 SVG_fn_s32DestroySaveGame(s32 port, s32 slot, char* directory)
{
    char path[0x40];
    sceMcTblGetDir* entry;
    s32 count;
    s32 i;

    entry = ALIGNED_STACK_BUFFER(0x12 * sizeof(sceMcTblGetDir));
    sprintf(path, "%s/*", directory);
    count = inv_SVG_fn_lListCard(path, 0x12, entry);
    if (count < 0) {
        return 0;
    }
    for (i = 0; i < count; i++, entry++) {
        if (entry->AttrFile & MC_ATTRIBUTE_SUBDIRECTORY) {
            continue;
        }
        sprintf(path, "%s/%s", directory, entry->EntryName);
        if (!pc_save_delete(path)) {
            return 0;
        }
    }
    return pc_save_delete(directory);
}
#else
s32 SVG_fn_s32DestroySaveGame(s32 _iPort, s32 _iSlot, char* _szSonyCompliantDirName)
{
    char path[0x40];
    s32 result;
    sceMcTblGetDir* entry;
    s32 tries;
    s32 i;
    s32 count;
    char* format;

    entry = ALIGNED_STACK_BUFFER(0x12 * sizeof(sceMcTblGetDir));
    sprintf(path, "%s/*", _szSonyCompliantDirName);
    tries = MC_TRIES;
    gs_bSceMcOpIsSuccessful = 0;
    while (tries != 0) {
        gs_bSceMcOpIsSuccessful = sceMcGetDir(_iPort, _iSlot, path, 0, 0x12, entry) == 0;
        if (gs_bSceMcOpIsSuccessful) {
            break;
        }
        if (--tries) {
            fn_vTimerWait(1);
        }
    }
    if (gs_bSceMcOpIsSuccessful == 0) {
        goto failed;
    }
    sceMcSync(0, NULL, &result);
    count = result;
    for (i = 0; i < count; i++, entry++) {
        if (entry->AttrFile & MC_ATTRIBUTE_SUBDIRECTORY) {
            continue;
        }
        if ((u32) path & 1) {
            format = "%s/%s";
        } else {
            format = "%s/%s";
        }
        sprintf(path, format, _szSonyCompliantDirName, entry->EntryName);
        tries = MC_TRIES;
        gs_bSceMcOpIsSuccessful = 0;
        while (tries != 0) {
            gs_bSceMcOpIsSuccessful = sceMcDelete(_iPort, _iSlot, path) == 0;
            if (gs_bSceMcOpIsSuccessful) {
                break;
            }
            if (--tries) {
                fn_vTimerWait(1);
            }
        }
        if (gs_bSceMcOpIsSuccessful != 0) {
            sceMcSync(0, NULL, &result);
            if (result == 0) {
                continue;
            }
        }
    failed:
        return 0;
    }

    tries = MC_TRIES;
    gs_bSceMcOpIsSuccessful = 0;
    while (tries != 0) {
        gs_bSceMcOpIsSuccessful = sceMcDelete(_iPort, _iSlot, _szSonyCompliantDirName) == 0;
        if (gs_bSceMcOpIsSuccessful) {
            break;
        }
        if (--tries) {
            fn_vTimerWait(1);
        }
    }
    if (gs_bSceMcOpIsSuccessful == 0) {
        goto failed;
    }
    sceMcSync(0, NULL, &result);
    if (result != 0) {
        goto failed;
    }
    return 1;
}
#endif

s32 SVG_fn_u32DoesSavegameExist(s32 port, s32 slot, char* directory, u32 saveSize, u8 deleteInvalid)
{
    char path[0x40];
    s32 count;
    s32 tries;
    sceMcTblGetDir* entries;
    sceMcTblGetDir* entry;
    SaveIconEntry* icons;
    u16 remaining;
    u32 size;
    char* name;

    entries = ALIGNED_STACK_BUFFER(10 * sizeof(sceMcTblGetDir));
    sprintf(path, "%s/*", directory);

#ifdef PC_PORT
    count = inv_SVG_fn_lListCard(path, 10, entries);
    if (count < 0) {
        return 0;
    }
    entry = &entries[2];
#else
    tries = MC_TRIES;
    gs_bSceMcOpIsSuccessful = 0;
    while (tries != 0) {
        gs_bSceMcOpIsSuccessful = sceMcGetDir(port, slot, path, 0, 10, entries) == 0;
        if (gs_bSceMcOpIsSuccessful) {
            break;
        }
        if (--tries) {
            fn_vTimerWait(1);
        }
    }
    if (gs_bSceMcOpIsSuccessful == 0) {
        return 0;
    }

    entry = &entries[2];
    sceMcSync(0, NULL, &count);
#endif
    count -= 2;
    remaining = func_0013F998() + 3;

    if (count != remaining) {
        if (deleteInvalid != 0) {
            SVG_fn_s32DestroySaveGame(port, slot, directory);
        }
        return 0;
    }

    count--;
    if (count >= 0) {
        icons = off_3313D8;
        for (; count >= 0; count--, entry++) {
            size = entry->FileSizeByte;
            name = entry->EntryName;

            if (size == sizeof(SaveDescription) && strcmp(name, "data.add") == 0) {
                remaining--;
                continue;
            }
            if (size == saveSize && strcmp(name, directory) == 0) {
                remaining--;
                continue;
            }
            if (size == ICON_SYS_SIZE && strcmp(name, "icon.sys") == 0) {
                remaining--;
                continue;
            }
            strlwr(name);
            if (strstr(name, ".ico") == NULL) {
                continue;
            }
            if (icons[0].size != 0 && size == icons[0].size) {
                remaining--;
            } else if (icons[1].size != 0 && size == icons[1].size) {
                remaining--;
            } else if (icons[2].size != 0 && size == icons[2].size) {
                remaining--;
            }
        }
    }

    if (remaining == 0) {
        return 1;
    }
    if (deleteInvalid != 0) {
        SVG_fn_s32DestroySaveGame(port, slot, directory);
    }
    return 0;
}

u32 SVG_fn_u32SaveGame(s32 port, s32 slot, char* directory, char* playerName, char* comment,
                       u16 completion, u32 gameDuration, void* saveData, s32 saveSize)
{
    s32 count;
    s32 result;
    s32 created;
    s32 tries;
    sceMcTblGetDir* entry;
    char path[0x40];

    entry = ALIGNED_STACK_BUFFER(sizeof(sceMcTblGetDir));
#ifdef PC_PORT
    count = inv_SVG_fn_lListCard(directory, 1, entry);
    if (count < 0) {
        return 0;
    }
    created = count != 1;
    if (created) {
        if (SVG_fn_s32MakeDir(port, slot, directory) != 1) {
            return 0;
        }
        if (inv_SVG_fn_lListCard(directory, 1, entry) < 0) {
            return 0;
        }
    }
#else
    tries = MC_TRIES;
    gs_bSceMcOpIsSuccessful = 0;
    while (tries != 0) {
        gs_bSceMcOpIsSuccessful = sceMcGetDir(port, slot, directory, 0, 1, entry) == 0;
        if (gs_bSceMcOpIsSuccessful) {
            break;
        }
        if (--tries) {
            fn_vTimerWait(1);
        }
    }
    if (gs_bSceMcOpIsSuccessful == 0) {
        return 0;
    }

    sceMcSync(0, NULL, &count);
    created = count != 1;
    if (created) {
        if (SVG_fn_s32MakeDir(port, slot, directory) != 1) {
            return 0;
        }
        tries = MC_TRIES;
        gs_bSceMcOpIsSuccessful = 0;
        while (tries != 0) {
            gs_bSceMcOpIsSuccessful = sceMcGetDir(port, slot, directory, 0, 1, entry) == 0;
            if (gs_bSceMcOpIsSuccessful) {
                break;
            }
            if (--tries) {
                fn_vTimerWait(1);
            }
        }
        if (gs_bSceMcOpIsSuccessful == 0) {
            return 0;
        }
        sceMcSync(0, NULL, &count);
    }
#endif

    result = SVG_fn_s32CreateIconFilesForSave(port, slot, directory, playerName, comment,
                                              completion, gameDuration, created);
    if (result != 1) {
        SVG_fn_s32DestroySaveGame(port, slot, directory);
        return 0;
    }

    sprintf(path, "%s/%s", directory, directory);
    if (SVG_fn_s32WriteToFile(port, slot, path, saveData, saveSize) != result) {
        SVG_fn_s32DestroySaveGame(port, slot, directory);
        return 0;
    }

#ifdef PC_PORT
    inv_SVG_fn_lListCard(directory, 1, entry);
#endif
    return inv_SVG_fn_lGetModificationTimeInSeconds(entry);
}

char* inv_SVG_fn_szGetSaveNameSuffix(char* name)
{
    return name + 0xC;
}

SaveSlot* inv_SVG_fn_p_stScanAndLoadSaveDirectory(void)
{
    SaveDescription description;
    char path[0x40];
    s32 count;
    sceMcTblGetDir* entries;
    SaveSlot* list;
    SaveSlot* slot;
    char* name;
    s32 tries;
    s32 loaded;
    s32 i;
    u16 id;

    list = NULL;
    id = 0;
    if (SVG_fn_s32Query(0, 0, 0) == 2) {
#ifdef PC_PORT
        entries = ALIGNED_STACK_BUFFER(0x63 * sizeof(sceMcTblGetDir));
        count = inv_SVG_fn_lListCard("BASLUS-20138*", 0x63, entries);
        {
#else
        tries = MC_TRIES;
        entries = ALIGNED_STACK_BUFFER(0x63 * sizeof(sceMcTblGetDir));
        gs_bSceMcOpIsSuccessful = 0;
        while (tries != 0) {
            gs_bSceMcOpIsSuccessful = sceMcGetDir(0, 0, "BASLUS-20138*", 0, 0x63, entries) == 0;
            if (gs_bSceMcOpIsSuccessful) {
                break;
            }
            if (--tries) {
                fn_vTimerWait(1);
            }
        }
        if (gs_bSceMcOpIsSuccessful != 0) {
            sceMcSync(0, NULL, &count);
#endif
            if (count >= 0) {
                for (i = 0; i < count; i++) {
                    name = entries[i].EntryName;
                    if (SVG_fn_u32DoesSavegameExist(0, 0, name, SAVE_DATA_SIZE, 1) != 1) {
                        continue;
                    }
                    sprintf(path, "%s/data.add", name);
                    loaded = SVG_fn_s32LoadFromFile(0, 0, path, &description, sizeof(description));
                    if (loaded != 1) {
                        continue;
                    }
                    slot = inv_SAV_fn_p_stAllocateSaveSlot(id++, inv_SVG_fn_szGetSaveNameSuffix(name));
                    if (slot == NULL) {
                        continue;
                    }
                    slot->info.valid = loaded;
                    slot->info.modificationTime = inv_SVG_fn_lGetModificationTimeInSeconds(&entries[i]);
                    strcpy(slot->info.comment, description.comment);
                    strcpy(slot->info.playerName, description.playerName);
                    slot->info.slotIndex = description.completion;
                    slot->info.gameDuration = description.gameDuration;
                    list = inv_SAV_fn_p_stInsertSlotSortedByDate(list, slot);
                }
            }
        }
        inv_SVG_bDirectoryValid = 1;
    }
    return func_00145878(list);
}
