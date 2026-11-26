#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include "PC.h"
#include "mods.h"
#include "pc_host.h"

#define PATH_SIZE 1200

static int32_t traced(const char *request, const char *path, int32_t result)
{
    (void)request;
    (void)path;
    return result;
}

static int make_folder(const char *path)
{
#ifdef _WIN32
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

static const char *card_folder(void)
{
    static char folder[1024];

    if (folder[0] != '\0') {
        return folder;
    }
    if (getenv("REVO_MEMCARD") != NULL && getenv("REVO_MEMCARD")[0] != '\0') {
        snprintf(folder, sizeof folder, "%s", getenv("REVO_MEMCARD"));
    } else {
        char dir[1024];

        revo_user_dir(dir, sizeof dir);
        snprintf(folder, sizeof folder, "%s/memcard0", dir);
    }
    make_folder(folder);
    revo_log("[save] memory card folder: %s", folder);
    return folder;
}

static void host_name(const char *name, char *out, size_t size)
{
#ifdef _WIN32
    size_t n = 0;

    for (; *name != '\0' && n + 4 < size; name++) {
        if (strchr("\\:*?\"<>|%", *name) != NULL || (unsigned char) *name < 0x20) {
            n += (size_t) snprintf(out + n, size - n, "%%%02X", (unsigned char) *name);
        } else {
            out[n++] = *name;
        }
    }
    out[n] = '\0';
#else
    snprintf(out, size, "%s", name);
#endif
}

static int host_path(const char *path, char *out, size_t size)
{
    char part[256];
    size_t n;
    const char *slash;

    if (strstr(path, "..") != NULL) {
        return 0;
    }
    n = (size_t) snprintf(out, size, "%s", card_folder());
    while (*path != '\0') {
        while (*path == '/') {
            path++;
        }
        if (*path == '\0') {
            break;
        }
        slash = strchr(path, '/');
        snprintf(part, sizeof part, "%.*s", (int) (slash != NULL ? slash - path : (ptrdiff_t) strlen(path)),
                 path);
        n += (size_t) snprintf(out + n, size > n ? size - n : 0, "/");
        host_name(part, out + n, size > n ? size - n : 0);
        n = strlen(out);
        path = slash != NULL ? slash : path + strlen(path);
    }
    return n + 1 < size;
}

static int matches(const char *pattern, const char *name)
{
    for (; *pattern != '\0'; pattern++, name++) {
        if (*pattern == '*') {
            for (; *name != '\0'; name++) {
                if (matches(pattern + 1, name)) {
                    return 1;
                }
            }
            return matches(pattern + 1, name);
        }
        if (*name == '\0' || (*pattern != '?' && *pattern != *name)) {
            return 0;
        }
    }
    return *name == '\0';
}

static void to_date(time_t when, PcDate *date)
{
    struct tm local;

#ifdef _WIN32
    localtime_s(&local, &when);
#else
    localtime_r(&when, &local);
#endif
    date->second = (uint8_t) (local.tm_sec > 59 ? 59 : local.tm_sec);
    date->minute = (uint8_t) local.tm_min;
    date->hour = (uint8_t) local.tm_hour;
    date->day = (uint8_t) local.tm_mday;
    date->month = (uint8_t) (local.tm_mon + 1);
    date->year = (uint8_t) (local.tm_year % 100);
}

static int describe(const char *name, const char *full, PcSaveEntry *entry)
{
    struct stat info;

    if (stat(full, &info) != 0) {
        return 0;
    }
    memset(entry, 0, sizeof *entry);
    snprintf(entry->name, sizeof entry->name, "%s", name);
    entry->isDirectory = (info.st_mode & S_IFMT) == S_IFDIR;
    entry->size = entry->isDirectory ? 0 : (uint32_t) info.st_size;
    to_date(info.st_ctime, &entry->created);
    to_date(info.st_mtime, &entry->modified);
    return 1;
}

static void card_name(const char *name, char *out, size_t size)
{
#ifdef _WIN32
    size_t n = 0;
    unsigned value;

    for (; *name != '\0' && n + 1 < size; name++) {
        if (name[0] == '%' && sscanf(name + 1, "%2x", &value) == 1) {
            out[n++] = (char) value;
            name += 2;
        } else {
            out[n++] = *name;
        }
    }
    out[n] = '\0';
#else
    snprintf(out, size, "%s", name);
#endif
}

static int32_t save_list(const char *path, PcSaveEntry *entries, int32_t maxEntries)
{
    char directory[PATH_SIZE];
    char full[PATH_SIZE + 260];
    char name[260];
    const char *pattern;
    const char *slash;
    int32_t count = 0;

    slash = strrchr(path, '/');
    pattern = slash != NULL ? slash + 1 : path;
    snprintf(name, sizeof name, "%.*s", (int) (slash != NULL ? slash - path : 0), path);
    if (!host_path(name, directory, sizeof directory)) {
        return -1;
    }
    if (*pattern == '\0') {
        pattern = "*";
    }

#ifdef _WIN32
    {
        WIN32_FIND_DATAA found;
        HANDLE search;

        snprintf(full, sizeof full, "%s/*", directory);
        search = FindFirstFileA(full, &found);
        if (search == INVALID_HANDLE_VALUE) {
            return -1;
        }
        do {
            if (found.cFileName[0] == '.') {
                continue;
            }
            card_name(found.cFileName, name, sizeof name);
            snprintf(full, sizeof full, "%s/%s", directory, found.cFileName);
            if (matches(pattern, name) && count < maxEntries && describe(name, full, &entries[count])) {
                count++;
            }
        } while (count < maxEntries && FindNextFileA(search, &found));
        FindClose(search);
    }
#else
    {
        DIR *folder = opendir(directory);
        struct dirent *found;

        if (folder == NULL) {
            return -1;
        }
        while (count < maxEntries && (found = readdir(folder)) != NULL) {
            if (found->d_name[0] == '.') {
                continue;
            }
            card_name(found->d_name, name, sizeof name);
            snprintf(full, sizeof full, "%s/%s", directory, found->d_name);
            if (matches(pattern, name) && describe(name, full, &entries[count])) {
                count++;
            }
        }
        closedir(folder);
    }
#endif
    return count;
}

static int32_t save_make_directory(const char *path)
{
    char full[PATH_SIZE];

    if (!host_path(path, full, sizeof full)) {
        return 0;
    }
#ifdef _WIN32
    return _mkdir(full) == 0;
#else
    return mkdir(full, 0755) == 0;
#endif
}

static int32_t save_delete(const char *path)
{
    char full[PATH_SIZE];
    struct stat info;

    if (!host_path(path, full, sizeof full) || stat(full, &info) != 0) {
        return 0;
    }
    if ((info.st_mode & S_IFMT) == S_IFDIR) {
#ifdef _WIN32
        return _rmdir(full) == 0;
#else
        return rmdir(full) == 0;
#endif
    }
    return remove(full) == 0;
}

static int32_t save_read(const char *path, void *buffer, uint32_t size)
{
    char full[PATH_SIZE];
    struct stat info;
    FILE *file;
    int32_t ok;

    if (!host_path(path, full, sizeof full) || stat(full, &info) != 0 || (uint64_t) info.st_size != size) {
        return 0;
    }
    file = fopen(full, "rb");
    if (file == NULL) {
        return 0;
    }
    ok = fread(buffer, 1, size, file) == size;
    fclose(file);
    return ok;
}

static int on_disk(FILE *file)
{
#ifdef _WIN32
    return _commit(_fileno(file)) == 0;
#else
    return fsync(fileno(file)) == 0;
#endif
}

static int32_t save_write(const char *path, const void *buffer, uint32_t size)
{
    char full[PATH_SIZE];
    char temporary[PATH_SIZE + 8];
    struct stat info;
    FILE *file;
    int ok;

    if (!host_path(path, full, sizeof full)) {
        return 0;
    }
    if (stat(full, &info) == 0) {
        if ((info.st_mode & S_IFMT) == S_IFDIR) {
            return 0;
        }
    }
    snprintf(temporary, sizeof temporary, "%s.new", full);
    file = fopen(temporary, "wb");
    if (file == NULL) {
        return 0;
    }
    ok = fwrite(buffer, 1, size, file) == size && fflush(file) == 0 && on_disk(file);
    if (fclose(file) != 0) {
        ok = 0;
    }
#ifdef _WIN32
    if (ok) {
        ok = MoveFileExA(temporary, full, MOVEFILE_REPLACE_EXISTING) != 0;
    }
#else
    if (ok) {
        ok = rename(temporary, full) == 0;
    }
#endif
    if (!ok) {
        remove(temporary);
    }
    return ok;
}

int32_t pc_save_list(const char *path, PcSaveEntry *entries, int32_t maxEntries)
{
    return traced("list", path, save_list(path, entries, maxEntries));
}

int32_t pc_save_make_directory(const char *path)
{
    return traced("make directory", path, save_make_directory(path));
}

int32_t pc_save_delete(const char *path)
{
    return traced("delete", path, save_delete(path));
}

int32_t pc_save_read(const char *path, void *buffer, uint32_t size)
{
    return traced("read", path, save_read(path, buffer, size));
}

int32_t pc_save_write(const char *path, const void *buffer, uint32_t size)
{
    return traced("write", path, save_write(path, buffer, size));
}
