#include "common.h"

#include "ps2/libc.h"
#include "GAM/GAM.h"
#include "FMN/FMN.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif
#include "clib/clib.h"
#include "ps2/PS2.h"

#define FMN_MOUNT_COUNT 4
#define FMN_SOUND_LANGUAGE_COUNT 5
#define FMN_MAX_PATH 0x3F3
#define FMN_SECTOR_SHIFT 11
#define FMN_SECTOR_MASK 0x7FF

#define FMN_KIND_UNUSED 0
#define FMN_KIND_CD 1
#define FMN_KIND_ARCHIVE 5

#define PATH_CHAR_UPPER 0x1
#define PATH_CHAR_LOWER 0x2

typedef char* VaList;

typedef struct SoundLanguageNames {
    char* names[4];
} SoundLanguageNames;

extern Bigfile* inv_FMN_a_hBigfiles[];
extern s32 inv_FMN_a_lOpenedBigfileSlots[3];
extern Bigfile* inv_FMN_hCurrentBigfile;
extern s8 inv_FMN_cMountedSoundLanguage;
extern SoundLanguageNames inv_FMN_stSoundLanguageNames;
extern u8 inv_FMN_virtualPathBuffer[];
extern u8 inv_FMN_realPathBuffer[];
extern u8 inv_FMN_pathCharacterFlags[];
extern char aIncreaseCMaxPa[];
extern char aSoundBnmSBf1[];
extern char aSoundBnmSDsc1[];
extern char aSoundSndSBf1[];
extern char aSoundSndSDsc1[];
extern char aSoundVscBf1[];
extern char aSoundVscDsc1[];
extern char aStartFmnBfLoad[];
extern char aStartFmnMountS[];
extern char aBigfileSNotMou[];

s32 vsprintf(char* buffer, const char* format, VaList arguments);
s32 FMN_SCE_CD_Seek(s32 slot, s64 offset, s32 whence);
s32 gu_FMN_SCE_CD_Read(s32 slot, void* buffer, s64 size, s64 count);
s32 inv_FMN_fn_lRegisterOpenFile(char* bigfileName, char* descriptionName);

s32 inv_FMN_fn_lOpenBFDescPair(char* bigfileName, char* descriptionName);
s32 inv_FMN_fn_lProbeVirtualOpen(char* path, s32 mode);
s32 FMN_SCE_VIRTUAL_OpenReadOnly(char* path);
void* inv_FMN_fn_lFindFileInBigfiles(char* path, FmnAllocFn allocate, FmnErrorFn onError);
void inv_FMN_fn_vPrintWriteStub(s32 slot, void* buffer, s32 size, s32 count);
s32 inv_FMN_fn_lGetBigfileHandleExtent(s32 slot);
s32 inv_FMN_fn_lTellBigfileHandle(s32 slot);
s32 inv_FMN_fn_lCloseBigfileHandle(s32 slot);
s32 inv_FMN_fn_lLengthBigfileHandle(s32 slot);
s32 inv_FMN_fn_lGetBigfileSectorOffset(s32 slot);
s32 inv_FMN_fn_vReadBigfileHandle(s32 slot, void* buffer, s64 size, s64 count);
s32 inv_FMN_fn_vSeekBigfileHandle(s32 slot, s64 offset, s32 whence);
u8* inv_FMN_fn_pFormatVirtualPath(char* path, u8 lowercase);
s32 inv_FMN_fn_bValidatePathLength(char* path);
void FMN_Umount_SOUND(void);

void inv_FMN_fn_lOpenGameArchive(void)
{
    memset(inv_FMN_virtualFileTable, 0, sizeof(inv_FMN_virtualFileTable));
    inv_FMN_fn_lOpenBFDescPair("\\GAME.BF;1", "\\GAME.DSC;1");
}

s32 inv_FMN_fn_lOpenBFDescPair(char* bigfileName, char* descriptionName)
{
    return inv_FMN_fn_lRegisterOpenFile(bigfileName, descriptionName);
}

void* inv_FMN_fn_lOpenFileOrVirtual(char* path, FmnAllocFn allocate, FmnErrorFn onError)
{
    void* buffer = inv_FMN_fn_lFindFileInBigfiles(path, allocate, onError);

    if (buffer == NULL) {
        buffer = inv_FMN_fn_lLoadFileFromCdWithCallback(path, allocate, onError);
    }
    return buffer;
}

s32 inv_FMN_fn_uOpenVirtualOrCD(char* path, s32 mode)
{
    s32 slot = inv_FMN_fn_lProbeVirtualOpen(path, mode);

    if ((u32) slot >= FMN_FILE_COUNT) {
        slot = FMN_SCE_CD_Open(path, mode);
    }
    return slot;
}

u32 inv_FMN_fn_ulOpenVirtualOrFallback(char* path)
{
    u32 slot = FMN_SCE_VIRTUAL_OpenReadOnly(path);

    if (slot >= FMN_FILE_COUNT) {
        slot = inv_FMN_fn_lOpenCdFallback(path);
    }
    return slot;
}

s32 FMN_Read(s32 slot, void* buffer, s64 size, s64 count)
{
    switch (inv_FMN_virtualFileTable[slot].kind) {
    case FMN_KIND_UNUSED:
        printf("FMN_Read Error! File does not exist!\n");
        return -1;
    case FMN_KIND_CD:
        return gu_FMN_SCE_CD_Read(slot, buffer, size, count);
    case FMN_KIND_ARCHIVE:
        return inv_FMN_fn_vReadBigfileHandle(slot, buffer, size, count);
    }
    return -1;
}

void inv_FMN_fn_vLogFormatted(s32 slot, const char* format, ...)
{
#ifdef PC_PORT
#else
    char buffer[512];
    VaList arguments;
    s32 length;

    arguments = __builtin_next_arg(format) - 0x30;
    length = vsprintf(buffer, format, arguments);
    inv_FMN_fn_vPrintWriteStub(slot, buffer, 1, length + 1);
#endif
}

void inv_FMN_fn_vPrintWriteStub(s32 slot, void* buffer, s32 size, s32 count)
{
    if (inv_FMN_virtualFileTable[slot].kind == FMN_KIND_UNUSED) {
        printf("FMN_Write Error! File does not exist!\n");
    } else {
        printf("FMN_Write not implemented! The File may not be writable.\n");
    }
}

s32 FMN_Length(s32 slot)
{
    switch (inv_FMN_virtualFileTable[slot].kind) {
    case FMN_KIND_UNUSED:
        printf("FMN_Tell Error! File does not exist!\n");
        return -1;
    case FMN_KIND_CD:
        return inv_FMN_fn_lGetBigfileHandleSize(slot);
    case FMN_KIND_ARCHIVE:
        return inv_FMN_fn_lGetBigfileHandleExtent(slot);
    }
    return -1;
}

s32 func_00212360(s32 slot)
{
    switch (inv_FMN_virtualFileTable[slot].kind) {
    case FMN_KIND_UNUSED:
        printf("FMN_Tell Error! File does not exist!\n");
        return 0;
    case FMN_KIND_CD:
        return FMN_SCE_CD_Tell(slot);
    case FMN_KIND_ARCHIVE:
        return inv_FMN_fn_lTellBigfileHandle(slot);
    }
    return -1;
}

s32 FMN_Seek(s32 slot, s64 offset, s32 whence)
{
    switch (inv_FMN_virtualFileTable[slot].kind) {
    case FMN_KIND_UNUSED:
        printf("FMN_Seek Error! File does not exist!\n");
        return 0;
    case FMN_KIND_CD:
        return FMN_SCE_CD_Seek(slot, offset, whence);
    case FMN_KIND_ARCHIVE:
        return inv_FMN_fn_vSeekBigfileHandle(slot, offset, whence);
    }
    return -1;
}

s32 FMN_Close(s32 slot)
{
    if ((u32) slot < FMN_FILE_COUNT) {
        switch (inv_FMN_virtualFileTable[slot].kind) {
        case FMN_KIND_UNUSED:
            printf("FMN_Close Error! File does not exist!\n");
            return 0;
        case FMN_KIND_CD:
            return FMN_SCE_CD_Close(slot);
        case FMN_KIND_ARCHIVE:
            return inv_FMN_fn_lCloseBigfileHandle(slot);
        }
    }
    return -1;
}

s32 func_00212528(s32 slot)
{
    switch (inv_FMN_virtualFileTable[slot].kind) {
    case FMN_KIND_UNUSED:
        printf("FMN_Eof Error! File does not exist!\n");
        return 0;
    case FMN_KIND_CD:
        return FMN_SCE_CD_Length(slot);
    case FMN_KIND_ARCHIVE:
        return inv_FMN_fn_lLengthBigfileHandle(slot);
    }
    return -1;
}

s32 FMN_Tell(s32 slot)
{
    switch (inv_FMN_virtualFileTable[slot].kind) {
    case FMN_KIND_CD:
        return inv_FMN_fn_pGetBigfileHandlePtr(slot);
    case FMN_KIND_ARCHIVE:
        return inv_FMN_fn_lGetBigfileSectorOffset(slot);
    }
    return -1;
}

void FMN_Umount_SOUND(void)
{
    printf("START FMN_Umount_SOUND\n");
    printf("unmounting bigfile n\260 %d\n", inv_FMN_a_lOpenedBigfileSlots[2]);
    inv_UBI_FILE_fn_vUnmountBigfileIfNotNull(inv_FMN_a_hBigfiles[inv_FMN_a_lOpenedBigfileSlots[2]]);
    inv_FMN_a_hBigfiles[inv_FMN_a_lOpenedBigfileSlots[2]] = NULL;
    printf("unmounting bigfile n\260 %d\n", inv_FMN_a_lOpenedBigfileSlots[1]);
    inv_UBI_FILE_fn_vUnmountBigfileIfNotNull(inv_FMN_a_hBigfiles[inv_FMN_a_lOpenedBigfileSlots[1]]);
    inv_FMN_a_hBigfiles[inv_FMN_a_lOpenedBigfileSlots[1]] = NULL;
    printf("unmounting bigfile n\260 %d\n", inv_FMN_a_lOpenedBigfileSlots[0]);
    inv_UBI_FILE_fn_vUnmountBigfileIfNotNull(inv_FMN_a_hBigfiles[inv_FMN_a_lOpenedBigfileSlots[0]]);
    inv_FMN_a_hBigfiles[inv_FMN_a_lOpenedBigfileSlots[0]] = NULL;
    printf("END FMN_Umount_SOUND\n");
}

s32 inv_FMN_fn_lMountSoundBanks(void)
{
    SoundLanguageNames languages;
    char bigfileName[0x80];
    char descriptionName[0x80];
    u32 language;
    char** name;

    language = g_stEngineStructure.ucCurrentLanguage;
    languages = inv_FMN_stSoundLanguageNames;
    if (language != (u32) inv_FMN_cMountedSoundLanguage) {
        printf(aStartFmnMountS);
        if (language >= FMN_SOUND_LANGUAGE_COUNT) {
            language = 0;
        } else if (inv_FMN_cMountedSoundLanguage != -1) {
            FMN_Umount_SOUND();
        }
        name = &languages.names[language];
        sprintf(bigfileName, aSoundBnmSBf1, *name);
        sprintf(descriptionName, aSoundBnmSDsc1, *name);
        inv_FMN_a_lOpenedBigfileSlots[0] = inv_FMN_fn_lOpenBFDescPair(bigfileName, descriptionName);
        sprintf(bigfileName, aSoundSndSBf1, *name);
        sprintf(descriptionName, aSoundSndSDsc1, *name);
        inv_FMN_a_lOpenedBigfileSlots[1] = inv_FMN_fn_lOpenBFDescPair(bigfileName, descriptionName);
        inv_FMN_a_lOpenedBigfileSlots[2] = inv_FMN_fn_lOpenBFDescPair(aSoundVscBf1, aSoundVscDsc1);
        inv_FMN_cMountedSoundLanguage = language;
    }
    return 0;
}

u8* inv_FMN_fn_pFormatVirtualPath(char* path, u8 lowercase)
{
    char* source = path;
    u8* destination = inv_FMN_virtualPathBuffer;

    do {
        if (*source == '\\') {
            *destination = '/';
        } else if ((u8) *source == 0xE8 || (u8) *source == 0xE9) {
            *destination = 'e';
        } else if (*source == ' ') {
            *destination = '_';
        } else if (lowercase == 1) {
            s32 lower = *source + ('a' - 'A');

            if (!(inv_FMN_pathCharacterFlags[(s32) *source] & PATH_CHAR_UPPER)) {
                lower = *source;
            }
            *destination = lower;
        }
        destination++;
    } while (*source++ != '\0');
    return inv_FMN_virtualPathBuffer;
}

u8* FMN_ConvertPathUnix2Dos(char* path)
{
    char* source = path;
    u8* destination = inv_FMN_realPathBuffer;

    do {
        if (*source == '/') {
            *destination = '\\';
        } else if ((u8) *source == 0xE8 || (u8) *source == 0xE9) {
            *destination = 'e';
        } else if (*source == ' ') {
            *destination = '_';
        } else {
            s32 upper = *source - ('a' - 'A');

            if (!(inv_FMN_pathCharacterFlags[(s32) *source] & PATH_CHAR_LOWER)) {
                upper = *source;
            }
            *destination = upper;
        }
        destination++;
    } while (*source++ != '\0');
    return inv_FMN_realPathBuffer;
}

s32 inv_FMN_fn_bValidatePathLength(char* path)
{
    u32 length;
    u32 maxPath;

    length = strlen(path);
    maxPath = FMN_MAX_PATH;
    if (maxPath < length) {
        printf(aIncreaseCMaxPa);
        return 0;
    }
    return 1;
}

s32 inv_FMN_fn_lRegisterOpenFile(char* bigfileName, char* descriptionName)
{
    s32 slot = 0;
    Bigfile* bigfile;

    while (inv_FMN_a_hBigfiles[slot] != NULL) {
        slot++;
        if (slot >= FMN_MOUNT_COUNT) {
            printf(aBigfileSNotMou, bigfileName);
            return -1;
        }
    }
    bigfile = inv_FMN_fn_bMountBigfile(bigfileName, descriptionName);
    inv_FMN_a_hBigfiles[slot] = bigfile;
    inv_FMN_hCurrentBigfile = bigfile;
    return bigfile != NULL ? slot : -1;
}

s32 inv_FMN_fn_lProbeVirtualOpen(char* path, s32 mode)
{
    return FMN_SCE_VIRTUAL_OpenReadOnly(path);
}

s32 FMN_SCE_VIRTUAL_OpenReadOnly(char* path)
{
    char fullPath[1040];
    s32 mount = 0;
    s32 slot;

    inv_FMN_hCurrentBigfile = inv_FMN_a_hBigfiles[0];
    if (inv_FMN_hCurrentBigfile == NULL || inv_FMN_fn_bValidatePathLength(path) == 0) {
        return -1;
    }
    slot = gu_FMN_iFindVirtualHandle();
    if (inv_FMN_virtualFileTable[slot].handle.archive == NULL) {
        while (inv_FMN_a_hBigfiles[mount] != NULL) {
            inv_FMN_hCurrentBigfile = inv_FMN_a_hBigfiles[mount];
            if (inv_My_strchr(path, ':') == NULL) {
                sprintf(fullPath, "%s/%s", inv_FMN_hCurrentBigfile->directory->name,
                        inv_FMN_fn_pFormatVirtualPath(path, 1));
            } else {
                strcpy(fullPath, path);
            }
            mount++;
            inv_FMN_virtualFileTable[slot].handle.archive =
                ubi_bigfile_fopen(inv_FMN_hCurrentBigfile, fullPath);
            if (inv_FMN_virtualFileTable[slot].handle.archive != NULL || mount >= FMN_MOUNT_COUNT) {
                break;
            }
        }
    }
    if (inv_FMN_virtualFileTable[slot].handle.archive != NULL) {
        inv_FMN_virtualFileTable[slot].position = 0;
        inv_FMN_virtualFileTable[slot].size = 0;
        inv_FMN_virtualFileTable[slot].kind = FMN_KIND_ARCHIVE;
        return slot;
    }
    inv_FMN_fn_lFreeBigfileSlot(slot);
    return -1;
}

s32 inv_FMN_fn_lCloseBigfileHandle(s32 slot)
{
    FmnFileDescriptor* descriptor = &inv_FMN_virtualFileTable[slot];

    ubi_bigfile_fclose(descriptor->handle.archive);
    descriptor->handle.archive = NULL;
    return inv_FMN_fn_lFreeBigfileSlot(slot);
}

s32 inv_FMN_fn_vReadBigfileHandle(s32 slot, void* buffer, s64 size, s64 count)
{
    s32 result =
        ubi_bigfile_fread(buffer, size, count, inv_FMN_virtualFileTable[slot].handle.archive);

    inv_FMN_virtualFileTable[slot].position =
        inv_FMN_virtualFileTable[slot].handle.archive->position;
    return result;
}

s32 inv_FMN_fn_vSeekBigfileHandle(s32 slot, s64 offset, s32 whence)
{
    s32 result = ubi_bigfile_fseek(inv_FMN_virtualFileTable[slot].handle.archive, offset, whence);

    inv_FMN_virtualFileTable[slot].position =
        inv_FMN_virtualFileTable[slot].handle.archive->position;
    return result;
}

s32 inv_FMN_fn_lTellBigfileHandle(s32 slot)
{
    return ubi_bigfile_ftell(inv_FMN_virtualFileTable[slot].handle.archive);
}

s32 inv_FMN_fn_lLengthBigfileHandle(s32 slot)
{
    return ubi_bigfile_flength(inv_FMN_virtualFileTable[slot].handle.archive);
}

s32 inv_FMN_fn_lGetBigfileHandleExtent(s32 slot)
{
    UbiFile* file = inv_FMN_virtualFileTable[slot].handle.archive;
    s32 length = file->length;
    s32 extent = *(s32*) &file->unk18;

    return length < extent ? extent : length;
}

void* inv_FMN_fn_lFindFileInBigfiles(char* path, FmnAllocFn allocate, FmnErrorFn onError)
{
    char fullPath[1040];
    s32 mount = 0;
    void* buffer = NULL;

    printf(aStartFmnBfLoad, path);
    inv_FMN_hCurrentBigfile = inv_FMN_a_hBigfiles[0];
    if (inv_FMN_hCurrentBigfile != NULL && inv_FMN_fn_bValidatePathLength(path) != 0 &&
        inv_FMN_a_hBigfiles[0] != NULL) {
        do {
            inv_FMN_hCurrentBigfile = inv_FMN_a_hBigfiles[mount];
            mount++;
            sprintf(fullPath, "%s/%s", inv_FMN_hCurrentBigfile->directory->name,
                    inv_FMN_fn_pFormatVirtualPath(path, 1));
            buffer = inv_ubi_bigfile_p_vLoadFile(inv_FMN_hCurrentBigfile, fullPath, allocate, onError);
            if (buffer != NULL) {
                break;
            }
            if (mount >= FMN_MOUNT_COUNT) {
                break;
            }
        } while (inv_FMN_a_hBigfiles[mount] != NULL);
    }
    return buffer;
}

s32 inv_FMN_fn_lGetBigfileSectorOffset(s32 slot)
{
    UbiFile* file = inv_FMN_virtualFileTable[slot].handle.archive;
    u32 offset = *(u32*) &file->offset;
    u32 sectors;

    if ((offset & FMN_SECTOR_MASK) == 0) {
        sectors = offset >> FMN_SECTOR_SHIFT;
    } else {
        sectors = (offset >> FMN_SECTOR_SHIFT) + 1;
    }
    return sectors + inv_FMN_virtualFileTable[file->bigfile->handle].handle.cd->lsn;
}
