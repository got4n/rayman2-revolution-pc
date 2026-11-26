#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/wait.h>
#include <spawn.h>
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#endif
#ifdef __linux__
#include <elf.h>
#endif

#include "native_mem.h"
#include "runtime.h"

// The game's memory is a zone of the program at the PS2's addresses
// (tools/native/zone.py): the player's SLUS_201.38 is copied there as the
// console's loader copied it, and the pointers it holds are right as they are.
// The program as it is built holds none of it. On its first start it writes
// a copy of itself next to it, revolution-full, whose zone holds the game's
// executable already: that one never reads SLUS_201.38, and this one starts
// it instead from then on.

#define RETAIL_SHA1 "33618569b7a1aa23300e9d1139510220189b3abe"
#define ZONE_LO 0x100000u
#define ZONE_SIZE 0x440000u
#define FULL_MARK "REVOFULL" RETAIL_SHA1

extern u8 revo_zone[];
extern const u32 revo_code_count, revo_code_table[];
extern const u32 revo_c_data_count, revo_c_data[];

extern char **environ;

static u32 *g_code;

static u32 le32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }

static int writable(u8 *at, size_t size) {
#ifdef _WIN32
    DWORD old;
    return VirtualProtect(at, size, PAGE_READWRITE, &old) != 0;
#else
    return mprotect(at, size, PROT_READ | PROT_WRITE) == 0;
#endif
}

// The native function at an address of the zone; any other pointer is one.
void *revo_code(void *p) {
    u8 *base = revo_native_base();
    const uintptr_t off = (uintptr_t)((u8 *)p - base) - ZONE_LO;
    if (off >= ZONE_SIZE - ZONE_LO) return p;
    const u32 r = g_code ? g_code[off >> 2] : 0;
    if (!r) revo_fatal("a call to %08X: no native function there", (u32)(off + ZONE_LO));
    return base + r;
}

// The program written from the player's copy: its zone holds the game's executable.
int revo_full_program(void) { return !memcmp(revo_zone, FULL_MARK, sizeof FULL_MARK - 1); }

static int own_path(char *out, size_t size) {
#ifdef _WIN32
    const DWORD n = GetModuleFileNameA(NULL, out, (DWORD)size);
    return n && n < size;
#elif defined(__APPLE__)
    uint32_t n = (uint32_t)size;
    return _NSGetExecutablePath(out, &n) == 0;
#else
    const ssize_t n = readlink("/proc/self/exe", out, size - 1);
    if (n <= 0) return 0;
    out[n] = 0;
    return 1;
#endif
}

// revolution.exe -> revolution-full.exe, revolution -> revolution-full
static int full_path(char *out, size_t size) {
    char self[4096];
    if (!own_path(self, sizeof self)) return 0;
    const char *slash = strrchr(self, '/'), *back = strrchr(self, '\\');
    const char *name = back > slash ? back : slash ? slash : self;
    const char *dot = strrchr(name, '.');
    const size_t stem = dot ? (size_t)(dot - self) : strlen(self);
    return snprintf(out, size, "%.*s-full%s", (int)stem, self, dot ? dot : "") < (int)size;
}

// Where the zone's bytes are in the program's file.
static long zone_in_file(void) {
    u8 *base = revo_native_base();
#ifdef _WIN32
    const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)base;
    const IMAGE_NT_HEADERS *nt = (const IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    const IMAGE_SECTION_HEADER *s = IMAGE_FIRST_SECTION(nt);
    const DWORD rva = (DWORD)(revo_zone - base);
    for (unsigned k = 0; k < nt->FileHeader.NumberOfSections; k++)
        if (s[k].VirtualAddress <= rva && rva + ZONE_SIZE <= s[k].VirtualAddress + s[k].SizeOfRawData)
            return (long)(s[k].PointerToRawData + rva - s[k].VirtualAddress);
    return -1;
#elif defined(__APPLE__)
    extern const struct mach_header_64 _mh_execute_header;
    const struct section_64 *s = getsectbynamefromheader_64(&_mh_execute_header, "__DATA_CONST", "__zone");
    unsigned long size = 0;
    const u8 *at = getsectiondata(&_mh_execute_header, "__DATA_CONST", "__zone", &size);
    if (!s || !at || size < ZONE_SIZE) return -1;
    return (long)(s->offset + (revo_zone - at));
#elif defined(__linux__)
    // The program is mapped from its file at its address: the segment of the zone says where it is in the file.
    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)base;
    const Elf64_Phdr *ph = (const Elf64_Phdr *)(base + eh->e_phoff);
    const uintptr_t at = (uintptr_t)revo_zone;
    for (unsigned k = 0; k < eh->e_phnum; k++)
        if (ph[k].p_type == PT_LOAD && ph[k].p_vaddr <= at && at + ZONE_SIZE <= ph[k].p_vaddr + ph[k].p_filesz)
            return (long)(ph[k].p_offset + at - ph[k].p_vaddr);
    return -1;
#else
    return -1;
#endif
}

static void write_full_program(void) {
    char self[4096], full[4096], part[4200];
    const long at = zone_in_file();
    if (at < 0 || !own_path(self, sizeof self) || !full_path(full, sizeof full)) return;
    FILE *f = fopen(self, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    u8 *file = size > at + (long)ZONE_SIZE ? malloc((size_t)size) : NULL;
    const int read = file && fread(file, 1, (size_t)size, f) == (size_t)size;
    fclose(f);
    if (!read) {
        free(file);
        return;
    }
    memcpy(file + at, revo_zone, ZONE_SIZE);
    memcpy(file + at, FULL_MARK, sizeof FULL_MARK - 1);
    snprintf(part, sizeof part, "%s.part", full);
    f = fopen(part, "wb");
    int ok = f && fwrite(file, 1, (size_t)size, f) == (size_t)size;
    if (f) ok = !fclose(f) && ok;
    free(file);
#ifndef _WIN32
    ok = ok && !chmod(part, 0755);
#endif
#ifdef __APPLE__
    // The copy is another program: macOS runs it only signed again.
    if (ok) {
        pid_t pid;
        int status = 0;
        char *args[] = {"codesign", "--force", "--sign", "-", part, NULL};
        ok = !posix_spawn(&pid, "/usr/bin/codesign", NULL, NULL, args, environ) && waitpid(pid, &status, 0) == pid &&
             WIFEXITED(status) && !WEXITSTATUS(status);
    }
#endif
    remove(full);
    if (ok && !rename(part, full)) {
        revo_log("[game] %s written: it starts without SLUS_201.38", full);
        return;
    }
    remove(part);
    revo_log("[game] %s could not be written: SLUS_201.38 is read at each start", full);
}

static long long modified(const char *path) {
    struct stat st;
    return stat(path, &st) ? -1 : (long long)st.st_mtime;
}

// From the program as it is built: the full one, when it is there and not older.
void revo_start_full_program(int argc, char **argv) {
    (void)argc;
    char self[4096], full[4096];
    const char *env = getenv("REVO_FULL");
    if (revo_full_program() || (env && !strcmp(env, "off")) || !own_path(self, sizeof self) ||
        !full_path(full, sizeof full))
        return;
    const long long a = modified(self), b = modified(full);
    if (b < 0 || b < a) return;
    fflush(NULL);
#ifdef _WIN32
    STARTUPINFOA si = {sizeof si};
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(full, GetCommandLineA(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return;
    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    exit((int)code);
#else
    unsetenv("REVO_NATIVE_NO_ASLR");
    argv[0] = full;
    execv(full, argv);
#endif
}

int revo_load_game(void) {
    u8 *base = revo_native_base();
    const uintptr_t zone = (uintptr_t)(revo_zone - base);
    if (zone > ZONE_LO)
        revo_fatal("the game's zone starts at %llX in the program, after %X", (unsigned long long)zone, ZONE_LO);
    const int full = revo_full_program();
    if (!writable(revo_zone, ZONE_SIZE)) revo_fatal("the game's zone cannot be written");
    if (!full) {
        u32 size = 0;
        u8 *elf = revo_disc_load("SLUS_201.38", &size);
        char sha1[41] = "";
        if (elf) revo_sha1_hex(elf, size, sha1);
        if (strcmp(sha1, RETAIL_SHA1)) {
            free(elf);
            return 0;
        }
        const u32 ph = le32(elf + 0x1C);
        const unsigned entry = elf[0x2A] | elf[0x2B] << 8, count = elf[0x2C] | elf[0x2D] << 8;
        for (unsigned k = 0; k < count; k++) {
            const u8 *p = elf + ph + entry * k;
            if (le32(p) != 1) continue;
            const u32 off = le32(p + 4), va = le32(p + 8), filesz = le32(p + 16), memsz = le32(p + 20);
            if (va < zone + sizeof FULL_MARK || va + memsz > zone + ZONE_SIZE || off + filesz > size)
                revo_fatal("SLUS_201.38's segment at %08X does not fit the game's zone", va);
            memcpy(base + va, elf + off, filesz);
            memset(base + va + filesz, 0, memsz - filesz);
        }
        free(elf);
        const char *env = getenv("REVO_FULL");
        if (!(env && !strcmp(env, "off"))) write_full_program();
    }
    for (u32 k = 0; k < revo_c_data_count; k++) {
        const u32 *e = revo_c_data + 3 * k;
        memcpy(base + e[0], base + e[1], e[2]);
    }
    g_code = calloc((ZONE_SIZE - ZONE_LO) / 4, sizeof *g_code);
    if (!g_code) revo_fatal("no memory for the table of the game's functions");
    for (u32 k = 0; k < revo_code_count; k++)
        g_code[(revo_code_table[2 * k] - ZONE_LO) >> 2] = revo_code_table[2 * k + 1];
    revo_log(full ? "[game] the game's executable is in the program: %u functions by address, %u objects from the C"
                  : "[game] SLUS_201.38 loaded at its addresses: %u functions by address, %u objects from the C",
             revo_code_count, revo_c_data_count);
    return 1;
}
