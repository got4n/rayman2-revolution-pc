#include "common.h"

#include "ps2/libc.h"
#include "FMN/FMN.h"

#define BIGFILE_SECTOR_SHIFT 11

typedef s32 (*BigfileCall)(Bigfile* bigfile);

char* strlwr(char* string);
s32 atol(const char* string);
s32 FMN_Read(s32 handle, void* buffer, s32 size, s32 count);
s32 FMN_Seek(s32 handle, s32 offset, s32 origin);
void freemem(void* block);
void ubi_bigfile_umount(Bigfile* bigfile);

void inv_UBI_FILE_fn_vUnmountBigfileIfNotNull(Bigfile* bigfile)
{
    if (bigfile != NULL) {
        ((BigfileCall) ubi_bigfile_umount)(bigfile);
    }
}

void ubi_bigfile_umount(Bigfile* bigfile)
{
    ubi_directory_destroy(bigfile->directory);
    bigfile->current = NULL;
    FMN_Close(bigfile->handle);
    freemem(bigfile);
}

char* inv_ubi_bigfile_parseName(u8** cursor)
{
    u8 buffer[128];
    char* name;
    s32 length = 0;

    (*cursor)++;
    while ((u8) (**cursor - '(') >= 2 && (s8) * *cursor != '[') {
        buffer[length] = **cursor;
        length++;
        (*cursor)++;
    }
    buffer[length] = 0;
    length++;
    name = malloc(length);
    memset(name, 0, length);
    strcpy(name, buffer);
    return name;
}

char* inv_ubi_bigfile_parseField(char** cursor)
{
    char buffer[0x80];
    char* name;
    s32 length;

    memset(buffer, 0, sizeof(buffer));
    (*cursor)++;
    length = 0;
    if (**cursor != ',') {
        do {
            buffer[length] = **cursor;
            (*cursor)++;
            length++;
        } while (**cursor != ',');
    }
    buffer[length] = 0;
    length++;
    name = malloc(length);
    memset(name, 0, length);
    strcpy(name, buffer);
    strlwr(name);
    return name;
}

s32 inv_ubi_bigfile_parseOffset(char** cursor)
{
    char buffer[0x80];
    s32 length;

    (*cursor)++;
    length = 0;
    if (**cursor != ',') {
        do {
            buffer[length] = **cursor;
            (*cursor)++;
            length++;
        } while (**cursor != ',');
    }
    buffer[length] = 0;
    return atol(buffer);
}

s32 inv_ubi_bigfile_parseLength(char** cursor)
{
    char buffer[0x80];
    s32 length;

    (*cursor)++;
    length = 0;
    if (**cursor != ',') {
        do {
            buffer[length] = **cursor;
            (*cursor)++;
            length++;
        } while (**cursor != ',');
    }
    buffer[length] = 0;
    return atol(buffer);
}

s32 inv_ubi_bigfile_parseLastField(char** cursor)
{
    char buffer[0x80];
    s32 length;

    (*cursor)++;
    length = 0;
    if (**cursor != ']') {
        do {
            buffer[length] = **cursor;
            (*cursor)++;
            length++;
        } while (**cursor != ']');
    }
    buffer[length] = 0;
    return atol(buffer);
}

s32 inv_ubi_bigfile_parseDescriptor(Bigfile* bigfile, char* path)
{
    char* input;
    UbiDirectory* directory;
    s32 depth;
    char* name;
    s32 offset;
    s32 length;
    s32 unk18;
    s32 parsed = 0;

    input = inv_FMN_fn_lOpenFileOrVirtual(path, malloc, freemem);
    if (input != NULL) {
        if (*input == '(') {
            name = inv_ubi_bigfile_parseName((u8**) &input);
            bigfile->directory = ubi_directory_Factory(NULL, name);
        }

        directory = bigfile->directory;
        depth = 1;
        do {
            if (*input == '(') {
                UbiDirectory* child;

                depth++;
                name = inv_ubi_bigfile_parseName((u8**) &input);
                child = ubi_directory_Factory(directory, name);
                ubi_directory_insertDirectory(directory, child);
                directory = child;
            }
            if (*input == ')') {
                input++;
                depth--;
                directory = directory->parent;
            }
            if (*input == '[') {
                name = inv_ubi_bigfile_parseField(&input);
                offset = inv_ubi_bigfile_parseOffset(&input);
                length = inv_ubi_bigfile_parseLength(&input);
                unk18 = inv_ubi_bigfile_parseLastField(&input);
                input++;
                ubi_directory_insertFile(directory,
                                         ubi_file_Factory(directory, name, offset, length, unk18));
            }
        } while (depth > 0);
        parsed = 1;
    }
    return parsed;
}

UbiFile* inv_ubi_bigfile_resolvePath(Bigfile* bigfile, char* path)
{
    return ubi_directory_findFile(bigfile->directory, path);
}

UbiFile* ubi_bigfile_fopen(Bigfile* bigfile, char* path)
{
    UbiFile* file = inv_ubi_bigfile_resolvePath(bigfile, path);

    bigfile->current = file;
    if (file != NULL) {
        file->bigfile = bigfile;
        bigfile->current->position = 0;
    }
    return bigfile->current;
}

s32 ubi_bigfile_fclose(UbiFile* file)
{
    file->position = 0;
    return 0;
}

s32 ubi_bigfile_fread(void* buffer, s64 size, s64 count, UbiFile* file)
{
    s64 position;
    s64 remaining;

    if (ubi_bigfile_fseek(file, 0, 1) < 0) {
        return -1;
    }

    position = file->position;
    remaining = file->length - position;
    if (remaining < size * count) {
        size = remaining;
    }
    file->position = position + size * count;
    return FMN_Read(file->bigfile->handle, buffer, size, count);
}

s32 ubi_bigfile_fseek(UbiFile* file, u64 offset, s32 origin)
{
    file->bigfile->current = file;
    if (origin == 0) {
        file->position = offset;
    }
    if (origin == 1) {
        u64 position = file->position + offset;

        file->position = position;
        if (file->length < position) {
            file->position = file->length;
            return -1;
        }
    }
    return FMN_Seek(file->bigfile->handle, file->offset + file->position, 0);
}

s32 ubi_bigfile_ftell(UbiFile* file)
{
    return file->position;
}

s32 ubi_bigfile_flength(UbiFile* file)
{
    return file->length - file->position;
}

void* inv_ubi_bigfile_p_vLoadFile(Bigfile* bigfile, char* path, FmnAllocFn allocate, FmnErrorFn onError)
{
    UbiFile* file;
    void* buffer;

    if (bigfile != NULL) {
        file = ubi_bigfile_fopen(bigfile, path);
        if (file == NULL) {
            return NULL;
        }
        buffer =
            allocate((((file->length >> BIGFILE_SECTOR_SHIFT) + 1) << BIGFILE_SECTOR_SHIFT) + 8);
        if (file->unk18 == 0) {
            ubi_bigfile_fread(buffer, 1, file->length, file);
            ubi_bigfile_fclose(file);
        }
        return buffer;
    }
    return NULL;
}
