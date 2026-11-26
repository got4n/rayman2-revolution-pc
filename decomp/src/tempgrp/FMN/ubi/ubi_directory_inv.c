#include "common.h"

#include "ps2/libc.h"
#include "FMN/FMN.h"
#include "clib/clib.h"

void freemem(void* block);

UbiDirectory* ubi_directory_Factory(UbiDirectory* parent, char* name)
{
    UbiDirectory* directory = malloc(sizeof(UbiDirectory));

    directory->parent = parent;
    directory->name = name;
    directory->directories = NULL;
    directory->files = NULL;
    return directory;
}

void ubi_directory_destroy(UbiDirectory* directory)
{
    while (directory->files != NULL && directory->files->value != NULL) {
        ubi_file_destroy(directory->files->value);
        directory->files = directory->files->next;
    }
    while (directory->directories != NULL && directory->directories->value != NULL) {
        ubi_directory_destroy(directory->directories->value);
        directory->directories = directory->directories->next;
    }
    if (directory->files != NULL) {
        freemem(directory->files);
    }
    if (directory->directories != NULL) {
        freemem(directory->directories);
    }
    freemem(directory->name);
}

static inline void inv_FMN_fn_vInsertLink(UbiDirectory* directory, void* value, s32 isFile)
{
    if ((isFile ? directory->files : directory->directories) == NULL) {
        UbiLink* link = malloc(sizeof(UbiLink));

        if (isFile) {
            directory->files = link;
        } else {
            directory->directories = link;
        }
        link->value = value;
        link->next = NULL;
    } else {
        UbiLink* link = isFile ? directory->files : directory->directories;

        if (link->value == NULL) {
            link->value = value;
            link->next = NULL;
        } else {
            while (link->next != NULL) {
                link = link->next;
            }
            link->next = malloc(sizeof(UbiLink));
            link->next->value = value;
            link->next->next = NULL;
        }
    }
}

void ubi_directory_insertDirectory(UbiDirectory* directory, UbiDirectory* child)
{
    inv_FMN_fn_vInsertLink(directory, child, 0);
}

void ubi_directory_insertFile(UbiDirectory* directory, UbiFile* file)
{
    inv_FMN_fn_vInsertLink(directory, file, 1);
}

UbiLink* func_00217048(UbiLink* link)
{
    if (link->next == NULL) {
        return link;
    }
    return func_00217048(link->next);
}

UbiFile* ubi_directory_findFile(UbiDirectory* directory, char* path)
{
    UbiLink* link;
    char* slash;
    UbiFile* file = NULL;

    slash = inv_My_strchr(path, '/');
    if (slash == NULL) {
        for (link = directory->files; link != NULL; link = link->next) {
            if (strcmp(((UbiFile*) link->value)->name, path) == 0) {
                file = link->value;
                break;
            }
        }
    } else if (strncmp(directory->name, path, slash - path) == 0) {
        path = slash + 1;
        slash = inv_My_strchr(path, '/');
        if (slash == NULL) {
            file = ubi_directory_findFile(directory, path);
        } else {
            for (link = directory->directories; link != NULL; link = link->next) {
                file = ubi_directory_findFile(link->value, path);
                if (file != NULL) {
                    break;
                }
            }
        }
    }
    return file;
}

UbiDirectory* func_00217158(UbiDirectory* directory, char* path)
{
    char* slash;
    char* name;
    UbiLink* link;
    UbiDirectory* found = NULL;
    s32 length;

    slash = inv_My_strchr(path, '/');
    if (slash == NULL) {
        for (link = directory->directories; link != NULL; link = link->next) {
            if (strcmp(((UbiDirectory*) link->value)->name, path) == 0) {
                found = link->value;
            }
        }
    } else {
        length = slash - path;
        name = malloc(length + 2);
        memset(name, 0, strlen(name));
        strncat(name, path, length);
        for (link = directory->directories; link != NULL; link = link->next) {
            if (strcmp(((UbiDirectory*) link->value)->name, name) == 0) {
                found = func_00217158(link->value, slash + 1);
                if (found != NULL) {
                    break;
                }
            }
        }
    }
    return found;
}
