#include "common.h"

#include "ps2/libc.h"

#include "include_asm.h"
#include "FMN/FMN.h"
#include "clib/clib.h"

#define CD_MAX_PATH 0x3F3
#define CD_SECTOR_SHIFT 11
#define CD_SECTOR_MASK 0x7FF
#define CD_VIRTUAL_FILES 15
#define CD_DISK_READY 2
#define CD_TRACE_READ_ERRORS 0

void freemem(void* pointer);
void FMN_SCE_CD_ISOify(char** path);
void inv_FMN_fn_lWaitCdReady(s32 mode);
#ifdef PC_PORT
#include "PC/PC.h"

static s32 inv_FMN_fn_bFindCdFile(CdFileLoc* file, const char* name)
{
    PcFile found;

    if (!pc_file_find(name, &found)) {
        return 0;
    }
    file->lsn = found.sector;
    file->size = found.size;
    return 1;
}

static s32 inv_FMN_fn_bReadCd(u32 lsn, u32 offset, void* buffer, u32 size)
{
    PcFile at;

    at.sector = lsn;
    at.size = size;
    return pc_file_read(&at, offset, buffer, size);
}
#else
s32 sceCdSearchFile(CdFileLoc* file, const char* name);
s32 sceCdDiskReady(s32 mode);
s32 sceCdGetError(void);
s32 sceCdRead(u32 lsn, u32 sectors, void* buffer, u8* mode);
s32 sceCdSync(s32 mode);
#endif
s32 gu_FMN_SCE_CD_ReadSectors(u32 lsn, u32 sectors, void* buffer);
char* index(char* string, s32 character);
char* rindex(char* string, s32 character);

s32 FMN_SCE_CD_Open(char* path, s32 mode)
{
    char* name;
    CdFileLoc* file;
    s32 slot;
    u32 size;
    FmnFileDescriptor* entry;
    u32 length;
    u32 maxPath;

    printf("FMN_SCE_CD_Open(%s)\n", path);
    length = strlen(path);
    maxPath = CD_MAX_PATH;
    if (maxPath < length) {
        printf("Increase C_MAX_PATH\n!!!");
        return -1;
    }

    FMN_SCE_CD_ISOify(&path);
    name = malloc(strlen(path) + 2);
    strcpy(name, path);
    file = malloc(0x20);
    inv_FMN_fn_lWaitCdReady(0);
    printf("CD OPENS : %s\n", name);
#ifdef PC_PORT
    if (!inv_FMN_fn_bFindCdFile(file, name)) {
#else
    if (sceCdSearchFile(file, name) == 0) {
#endif
        freemem(file);
        printf("While loading %s (%d), sceCdSearchFile cmd fail :%d\n", path, strlen(path), 0);
        printf("While loading %s (%d), sceCdSearchFile cmd fail :%d\n", name, strlen(name), 0);
        return -1;
    }

#ifndef PC_PORT
    sceCdDiskReady(0);
#endif
    printf("sceCdOpen (sector = %d \n", file->lsn);
    slot = gu_FMN_iFindVirtualHandle();
    if (slot < CD_VIRTUAL_FILES) {
        size = file->size;
        entry = &inv_FMN_virtualFileTable[slot];
        entry->kind = 1;
        entry->size = size;
        entry->handle.cd = file;
        entry->position = 0;
    }
    return slot;
}

#ifdef PC_PORT
s32 inv_FMN_fn_lLogCdError(void)
{
    return 0;
}

void inv_FMN_fn_lWaitCdReady(s32 mode)
{
}

s32 inv_FMN_fn_lReadCdSectorRetry(u32 lsn, void* buffer)
{
    return inv_FMN_fn_bReadCd(lsn, 0, buffer, 1 << CD_SECTOR_SHIFT);
}

s32 gu_FMN_SCE_CD_ReadSectors(u32 lsn, u32 sectors, void* buffer)
{
    return inv_FMN_fn_bReadCd(lsn, 0, buffer, sectors << CD_SECTOR_SHIFT);
}

s32 gu_FMN_SCE_CD_Read(s32 slot, u8* destination, s32 size, s32 count)
{
    FmnFileDescriptor* file = &inv_FMN_virtualFileTable[slot];
    u32 total = size * count;

    inv_FMN_fn_bReadCd(file->handle.cd->lsn, file->position, destination, total);
    file->position += total;
    return total;
}
#else
s32 inv_FMN_fn_lLogCdError(void)
{
    s32 error = sceCdGetError();

    switch (error) {
    case -1:
        printf("CD Read problem : sceCdGetError() function issue failed\n");
        break;
    case 50:
        printf("CD Read problem : Outermost track reached during playback\n");
        break;
    case 49:
        printf("CD Read problem : Cover opened during playback\n");
        break;
    case 48:
        printf("CD Read problem : Problem occurred during read\n");
        break;
    case 20:
        printf("CD Read problem : Not appropriate for disc in drive\n");
        break;
    case 19:
        printf("CD Read problem : Processing command\n");
        break;
    case 1:
        printf("CD Read problem : Abort command received\n");
        break;
    case 0:
        break;
    default:
        printf("CD Read problem : Unhandled error...\n");
        break;
    }
    return error;
}

void inv_FMN_fn_lWaitCdReady(s32 mode)
{
    sceCdSync(mode);
    sceCdDiskReady(0);
}

s32 inv_FMN_fn_lReadCdSectorRetry(u32 lsn, void* buffer)
{
    u8 mode[3];

    mode[0] = 0;
    mode[1] = 1;
    mode[2] = 0;
    while (sceCdDiskReady(0) != CD_DISK_READY) {
    }
    inv_FMN_bCdReadRetrying = 0;
    while (sceCdRead(lsn, 1, buffer, mode) != 1 || sceCdSync(0) != 0 || sceCdGetError() != 0) {
        printf("retrying cd read\n");
        inv_FMN_bCdReadRetrying = 1;
        while (sceCdDiskReady(0) != CD_DISK_READY) {
        }
    }
    inv_FMN_bCdReadRetrying = 0;
    return 1;
}

s32 gu_FMN_SCE_CD_ReadSectors(u32 lsn, u32 sectors, void* buffer)
{
    u8 mode[3];

    mode[0] = 0;
    mode[1] = 1;
    mode[2] = 0;
    while (sceCdDiskReady(0) != CD_DISK_READY) {
    }
    inv_FMN_bCdReadRetrying = 0;
    while (sceCdRead(lsn, sectors, buffer, mode) != 1 || sceCdSync(0) != 0 ||
           sceCdGetError() != 0) {
        if (CD_TRACE_READ_ERRORS)
            printf("Read cmd fail :%d\n", sceCdGetError());
        printf("retrying cd read\n");
        inv_FMN_bCdReadRetrying = 1;
        while (sceCdDiskReady(0) != CD_DISK_READY) {
        }
    }
    inv_FMN_bCdReadRetrying = 0;
    return 1;
}

extern u8 inv_FMN_a_ucCdSectorBuffer[];

s32 gu_FMN_SCE_CD_Read(s32 slot, u8* destination, s32 size, s32 count)
{
    FmnFileDescriptor* file = &inv_FMN_virtualFileTable[slot];
    u8* sectorBuffer = inv_FMN_a_ucCdSectorBuffer;
    u32 total = size * count;
    u8* out = destination;
    u32 position = file->position;
    u32 end = position + total;
    u32 offset = position & CD_SECTOR_MASK;
    u32 lsn = file->handle.cd->lsn;
    u32 sector = (position >> CD_SECTOR_SHIFT) + lsn;
    u32 firstSector;
    u32 lastSector = (end >> CD_SECTOR_SHIFT) + lsn;
    u32 headEnd;
    u32 head;
    u32 rest;
    u32 tail;
    u32 middleSize;
    u32 middleSectors;

    if (offset == 0) {
        headEnd = position;
        firstSector = sector;
    } else {
        firstSector = sector + 1;
        headEnd = position + (0x800 - offset);
        if (end < headEnd)
            headEnd = end;
    }
    head = headEnd - position;
    rest = total - head;
    tail = rest & CD_SECTOR_MASK;
    middleSize = rest - tail;
    middleSectors = middleSize >> CD_SECTOR_SHIFT;
    if (offset != 0 || total == head) {
        memset(sectorBuffer, 0, 0x800);
        out = destination + head;
        inv_FMN_fn_lReadCdSectorRetry(sector, sectorBuffer);
        memcpy(destination, sectorBuffer + offset, head);
    }
    if (middleSectors != 0) {
        gu_FMN_SCE_CD_ReadSectors(firstSector, middleSectors, out);
        out += middleSize;
    }
    if (tail != 0) {
        memset(sectorBuffer, 0, 0x800);
        inv_FMN_fn_lReadCdSectorRetry(lastSector, sectorBuffer);
        memcpy(out, sectorBuffer, tail);
    }
    file->position += total;
    return total;
}
#endif

s32 FMN_SCE_CD_Tell(s32 slot)
{
    return inv_FMN_virtualFileTable[slot].position;
}

s32 FMN_SCE_CD_Seek(s32 slot, s32 offset, s32 whence)
{
    switch (whence) {
    case 0:
        inv_FMN_virtualFileTable[slot].position = offset;
        break;
    case 1:
        inv_FMN_virtualFileTable[slot].position += offset;
        break;
    case 2:
        inv_FMN_virtualFileTable[slot].position = inv_FMN_virtualFileTable[slot].size - offset;
        break;
    }
    return inv_FMN_virtualFileTable[slot].position;
}

s32 FMN_SCE_CD_Close(s32 slot)
{
#ifndef PC_PORT
    sceCdDiskReady(0);
#endif
    freemem(inv_FMN_virtualFileTable[slot].handle.cd);
    inv_FMN_fn_lFreeBigfileSlot(slot);
    return 0;
}

s32 FMN_SCE_CD_Length(s32 slot)
{
    return FMN_SCE_CD_Seek(slot, 0, 2);
}

s32 inv_FMN_fn_lOpenCdFallback(char* path)
{
    return FMN_SCE_CD_Open(path, 1);
}

s32 inv_FMN_fn_lGetBigfileHandleSize(s32 slot)
{
    FmnFileDescriptor* entry = &inv_FMN_virtualFileTable[slot];

    if (entry == NULL) {
        printf("ERROR in FMN_SCE_Length!\n");
    }
    return entry->handle.cd->size;
}

void* inv_FMN_fn_lLoadFileFromCdWithCallback(char* path, FmnAllocFn allocate, FmnErrorFn onError)
{
    CdFileLoc* file;
    u32 size;
    u32 sectors;
    void* buffer;
    u32 length;
    u32 maxPath;

    length = strlen(path);
    maxPath = CD_MAX_PATH;
    if (maxPath < length) {
        printf("Increase C_MAX_PATH\n!!!");
        return NULL;
    }

    FMN_SCE_CD_ISOify(&path);
    file = malloc(0x20);
#ifdef PC_PORT
    if (!inv_FMN_fn_bFindCdFile(file, path)) {
#else
    sceCdDiskReady(0);
    if (sceCdSearchFile(file, path) == 0) {
#endif
        freemem(file);
        printf("while reading %s, sceCdSearchFile failed :%d\n", path, 0);
        return NULL;
    }

    size = file->size;
    sectors = size >> CD_SECTOR_SHIFT;
    if (size & CD_SECTOR_MASK) {
        sectors++;
    }
#ifndef PC_PORT
    sceCdDiskReady(0);
#endif
    buffer = allocate(sectors << CD_SECTOR_SHIFT);
    if (gu_FMN_SCE_CD_ReadSectors(file->lsn, sectors, buffer) == 0) {
        printf("error while loading file %s !\n", path);
        onError(buffer);
    }
    freemem(file);
    return buffer;
}

void FMN_SCE_CD_ISOify(char** path)
{
    char buffer[0x410];
    char extension[0x10];
    char* separator;
    char* cursor;
    char* dot;
    char* shortName;
    s32 length;
    s32 shorten;

    if (strstr(*path, ";1") != NULL) {
        return;
    }
    sprintf(buffer, "\\GAME\\%s", FMN_ConvertPathUnix2Dos(*path));
    Strcat(buffer, ";1");
    separator = rindex(buffer, '\\');
    cursor = separator + 1;
    dot = rindex(buffer, '.');
    strcpy(extension, dot);
    length = dot - separator;
    shorten = length >= 10;
    while (cursor != dot) {
        if (*cursor == '.') {
            strcpy(cursor, cursor + 1);
            dot--;
            shorten = 1;
        } else {
            cursor++;
        }
    }
    if (shorten == 1) {
        length = cursor - separator;
        if (length >= 8) {
            length = 7;
        }
        shortName = separator + length;
        shortName[0] = '~';
        shortName[1] = '1';
        strcpy(shortName + 2, extension);
    }

    cursor = buffer;
    do {
        separator = cursor + 1;
        dot = index(separator, '\\');
        if (cursor != NULL) {
            dot = index(separator, '\\');
            if (dot == NULL) {
                break;
            }
            if (dot - cursor >= 10) {
                cursor[6] = '~';
                cursor[7] = '1';
                strcpy(cursor + 8, dot);
                cursor += 8;
            } else {
                cursor = dot + 1;
            }
        }
    } while (dot != NULL);
    strcpy(*path, buffer);
}

u32 inv_FMN_fn_pGetBigfileHandlePtr(s32 slot)
{
    return inv_FMN_virtualFileTable[slot].handle.cd->lsn;
}

s32 gu_FMN_iFindVirtualHandle(void)
{
    s32 i = 0;
    FmnFileDescriptor* entry = inv_FMN_virtualFileTable;

    while (entry->kind != 0 && i < CD_VIRTUAL_FILES) {
        i++;
        entry++;
    }
    return i >= CD_VIRTUAL_FILES ? -1 : i;
}

s32 inv_FMN_fn_lFreeBigfileSlot(u32 slot)
{
    FmnFileDescriptor* entry;
    s32 used;

    if (slot < CD_VIRTUAL_FILES) {
        inv_FMN_virtualFileTable[slot].position = 0;
        inv_FMN_virtualFileTable[slot].size = 0;
        inv_FMN_virtualFileTable[slot].kind = 0;
        entry = &inv_FMN_virtualFileTable[slot];
        entry->handle.cd = NULL;
        switch (used = inv_FMN_virtualFileTable[slot].kind) {
        default:
            if ((u8) used & 0x100) {
                entry->position = 0;
            }
        case 5:
            __asm__("");
        case 1:
            return 0;
        }
    }
    return -1;
}
