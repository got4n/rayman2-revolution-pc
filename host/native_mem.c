#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "native_mem.h"

#define BLOCK_SIZE 0x80000000u
#define GRANULE 0x10000u

static uint8_t g_mapped[BLOCK_SIZE / GRANULE / 8];

static void mark(uintptr_t base, uintptr_t from, uintptr_t to, int on) {
    if (to <= from || from < base) return;
    const uintptr_t last = (to - base + GRANULE - 1) / GRANULE;
    for (uintptr_t g = (from - base) / GRANULE; g < last && g < BLOCK_SIZE / GRANULE; g++) {
        if (on) g_mapped[g >> 3] |= (uint8_t)(1u << (g & 7));
        else g_mapped[g >> 3] &= (uint8_t)~(1u << (g & 7));
    }
}

int revo_guest_readable(uint32_t a, uint32_t n) {
    if (!n) n = 1;
    if (a < GRANULE || (uint64_t)a + n > BLOCK_SIZE) return 0;
    const uint32_t last = (uint32_t)(((uint64_t)a + n - 1) / GRANULE);
    for (uint32_t g = a / GRANULE; g <= last; g++)
        if (!(g_mapped[g >> 3] >> (g & 7) & 1)) return 0;
    return 1;
}

#ifdef _WIN32
#include <windows.h>

extern const IMAGE_DOS_HEADER __ImageBase;

static SRWLOCK g_lock = SRWLOCK_INIT;
static uintptr_t g_next, g_end;

uint8_t *revo_native_base(void) { return (uint8_t *)&__ImageBase; }

uint8_t *const revo_base = (uint8_t *)&__ImageBase;

typedef struct { uintptr_t at, size; } Span;
enum { SPANS = 512 };
static Span g_spare[SPANS];
static int g_nspare;
static Span g_given[SPANS];
static int g_ngiven;
static uintptr_t g_reserved;

static void spare_add(uintptr_t at, uintptr_t size) {
    int i = 0;
    while (i < g_nspare && g_spare[i].at < at) i++;
    if (i > 0 && g_spare[i - 1].at + g_spare[i - 1].size == at) {
        g_spare[i - 1].size += size;
        if (i < g_nspare && at + size == g_spare[i].at) {
            g_spare[i - 1].size += g_spare[i].size;
            for (int k = i; k + 1 < g_nspare; k++) g_spare[k] = g_spare[k + 1];
            g_nspare--;
        }
        return;
    }
    if (i < g_nspare && at + size == g_spare[i].at) {
        g_spare[i].at = at;
        g_spare[i].size += size;
        return;
    }
    if (g_nspare == SPANS) return;
    for (int k = g_nspare; k > i; k--) g_spare[k] = g_spare[k - 1];
    g_spare[i] = (Span){at, size};
    g_nspare++;
}

static void block_init(void) {
    if (g_end) return;
    const uintptr_t base = (uintptr_t)&__ImageBase;
    const IMAGE_NT_HEADERS *nt = (const IMAGE_NT_HEADERS *)(base + __ImageBase.e_lfanew);
    if (base & 0xFFFFFFFFu) {
        fprintf(stderr, "the program is not at an address aligned on 4 GB (%llX)\n", (unsigned long long)base);
        abort();
    }
    g_next = (base + nt->OptionalHeader.SizeOfImage + GRANULE - 1) & ~(uintptr_t)(GRANULE - 1);
    g_end = base + BLOCK_SIZE;
    mark(base, base, base + nt->OptionalHeader.SizeOfImage, 1);
    for (uintptr_t a = g_next; a < g_end;) {
        MEMORY_BASIC_INFORMATION mi;
        if (!VirtualQuery((void *)a, &mi, sizeof mi)) break;
        uintptr_t end = (uintptr_t)mi.BaseAddress + mi.RegionSize;
        if (end > g_end) end = g_end;
        if (mi.State == MEM_FREE) {
            const uintptr_t from = (a + GRANULE - 1) & ~(uintptr_t)(GRANULE - 1), to = end & ~(uintptr_t)(GRANULE - 1);
            if (to > from && VirtualAlloc((void *)from, to - from, MEM_RESERVE, PAGE_NOACCESS)) {
                spare_add(from, to - from);
                g_reserved += to - from;
            }
        }
        a = end;
    }
}

__attribute__((constructor)) static void reserve_at_start(void) {
    AcquireSRWLockExclusive(&g_lock);
    block_init();
    ReleaseSRWLockExclusive(&g_lock);
}

void revo_log(const char *fmt, ...);

static void tell_block(size_t size) {
    const uintptr_t base = (uintptr_t)&__ImageBase;
    revo_log("[mem] no %u KB left in the game's block (%u MB reserved at start, %d spare parts, %d given):",
             (unsigned)(size >> 10), (unsigned)(g_reserved >> 20), g_nspare, g_ngiven);
    int lines = 0;
    for (uintptr_t a = base; a < base + BLOCK_SIZE && lines < 80;) {
        MEMORY_BASIC_INFORMATION mi;
        if (!VirtualQuery((void *)a, &mi, sizeof mi)) break;
        const uintptr_t at = (uintptr_t)mi.BaseAddress, end = at + mi.RegionSize;
        if (mi.State != MEM_FREE) {
            char name[MAX_PATH] = "";
            if (mi.Type == MEM_IMAGE) GetModuleFileNameA((HMODULE)mi.AllocationBase, name, sizeof name);
            revo_log("[mem]   %08llX-%08llX %6llu KB %s %s %s", (unsigned long long)(at - base),
                     (unsigned long long)(end - base), (unsigned long long)(mi.RegionSize >> 10),
                     mi.State == MEM_COMMIT ? "committed" : "reserved",
                     mi.Type == MEM_IMAGE ? "image" : mi.Type == MEM_MAPPED ? "mapped" : "private", name);
            lines++;
        }
        a = end;
    }
}

void *revo_low_alloc(size_t size) {
    size = (size + GRANULE - 1) & ~(size_t)(GRANULE - 1);
    AcquireSRWLockExclusive(&g_lock);
    block_init();
    static int told;
    if (!told) {
        told = 1;
        revo_log("[mem] %u MB of the game's block reserved at start, in %d parts", (unsigned)(g_reserved >> 20),
                 g_nspare);
    }
    void *p = NULL;
    for (int i = 0; i < g_nspare && !p; i++) {
        if (g_spare[i].size < size || !VirtualAlloc((void *)g_spare[i].at, size, MEM_COMMIT, PAGE_READWRITE)) continue;
        p = (void *)g_spare[i].at;
        if (g_ngiven < SPANS) g_given[g_ngiven++] = (Span){g_spare[i].at, size};
        g_spare[i].at += size;
        g_spare[i].size -= size;
        if (!g_spare[i].size) {
            for (int k = i; k + 1 < g_nspare; k++) g_spare[k] = g_spare[k + 1];
            g_nspare--;
        }
    }
    uintptr_t a = g_next;
    while (!p && a + size <= g_end) {
        MEMORY_BASIC_INFORMATION mi;
        if (!VirtualQuery((void *)a, &mi, sizeof mi)) break;
        const uintptr_t end = (uintptr_t)mi.BaseAddress + mi.RegionSize;
        if (mi.State == MEM_FREE && end >= a + size) p = VirtualAlloc((void *)a, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!p) a = (end + GRANULE - 1) & ~(uintptr_t)(GRANULE - 1);
    }
    if (p) mark((uintptr_t)&__ImageBase, (uintptr_t)p, (uintptr_t)p + size, 1);
    else tell_block(size);
    ReleaseSRWLockExclusive(&g_lock);
    return p;
}

void revo_low_free(void *p) {
    if (!p) return;
    AcquireSRWLockExclusive(&g_lock);
    for (int i = 0; i < g_ngiven; i++) {
        if (g_given[i].at != (uintptr_t)p) continue;
        const Span s = g_given[i];
        g_given[i] = g_given[--g_ngiven];
        mark((uintptr_t)&__ImageBase, s.at, s.at + s.size, 0);
        VirtualFree(p, s.size, MEM_DECOMMIT);
        spare_add(s.at, s.size);
        ReleaseSRWLockExclusive(&g_lock);
        return;
    }
    MEMORY_BASIC_INFORMATION mi;
    uintptr_t end = (uintptr_t)p;
    while (VirtualQuery((void *)end, &mi, sizeof mi) && mi.AllocationBase == p)
        end = (uintptr_t)mi.BaseAddress + mi.RegionSize;
    mark((uintptr_t)&__ImageBase, (uintptr_t)p, end, 0);
    VirtualFree(p, 0, MEM_RELEASE);
    ReleaseSRWLockExclusive(&g_lock);
}

#elif defined(__APPLE__)
#include <crt_externs.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <spawn.h>
#include <string.h>

#ifndef _POSIX_SPAWN_DISABLE_ASLR
#define _POSIX_SPAWN_DISABLE_ASLR 0x0100
#endif

extern const struct mach_header_64 _mh_execute_header;
extern char **environ;

uint8_t *revo_native_base(void) { return (uint8_t *)&_mh_execute_header; }
uint8_t *const revo_base = (uint8_t *)&_mh_execute_header;

static void without_aslr(void) {
    char **argv = *_NSGetArgv();
    const uintptr_t base = (uintptr_t)&_mh_execute_header;
    if (!(base & 0xFFFFFFFFu)) return;
    if (getenv("REVO_NATIVE_NO_ASLR")) {
        fprintf(stderr, "the program is not at an address aligned on 4 GB (%llX), even without ASLR\n",
                (unsigned long long)base);
        abort();
    }
    char path[4096];
    uint32_t size = sizeof path;
    if (_NSGetExecutablePath(path, &size) != 0) {
        fprintf(stderr, "the program's path is too long\n");
        abort();
    }
    setenv("REVO_NATIVE_NO_ASLR", "1", 1);
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETEXEC | _POSIX_SPAWN_DISABLE_ASLR);
    const int r = posix_spawn(NULL, path, NULL, &attr, argv, environ);
    fprintf(stderr, "cannot start the program again without ASLR: %s\n", strerror(r));
    abort();
}

__attribute__((constructor)) static void without_aslr_at_start(void) { without_aslr(); }

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static uintptr_t g_next, g_end;

typedef struct { uintptr_t at; size_t size; } Region;
static Region *g_regions;
static size_t g_count, g_cap;

static void block_init(void) {
    if (g_end) return;
    without_aslr();
    const uintptr_t base = (uintptr_t)&_mh_execute_header;
    uintptr_t end = base;
    const struct load_command *lc = (const struct load_command *)(&_mh_execute_header + 1);
    for (uint32_t i = 0; i < _mh_execute_header.ncmds; i++) {
        if (lc->cmd == LC_SEGMENT_64) {
            const struct segment_command_64 *seg = (const struct segment_command_64 *)lc;
            if (seg->vmaddr >= base && seg->vmaddr + seg->vmsize > end) end = seg->vmaddr + seg->vmsize;
        }
        lc = (const struct load_command *)((const uint8_t *)lc + lc->cmdsize);
    }
    g_next = (end + GRANULE - 1) & ~(uintptr_t)(GRANULE - 1);
    g_end = base + BLOCK_SIZE;
    mark(base, base, end, 1);
}

void *revo_low_alloc(size_t size) {
    size = (size + GRANULE - 1) & ~(size_t)(GRANULE - 1);
    pthread_mutex_lock(&g_lock);
    block_init();
    void *p = NULL;
    uintptr_t a = g_next;
    while (!p && a + size <= g_end) {
        mach_vm_address_t at = a;
        if (mach_vm_allocate(mach_task_self(), &at, size, VM_FLAGS_FIXED) == KERN_SUCCESS) {
            p = (void *)(uintptr_t)at;
            g_next = a + size;
            break;
        }
        mach_vm_address_t r = a;
        mach_vm_size_t rs = 0;
        vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t n = VM_REGION_BASIC_INFO_COUNT_64;
        mach_port_t object = MACH_PORT_NULL;
        if (mach_vm_region(mach_task_self(), &r, &rs, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&info, &n,
                           &object) != KERN_SUCCESS)
            break;
        a = ((uintptr_t)(r + rs) + GRANULE - 1) & ~(uintptr_t)(GRANULE - 1);
    }
    if (p) {
        if (g_count == g_cap) {
            g_cap = g_cap ? g_cap * 2 : 64;
            g_regions = realloc(g_regions, g_cap * sizeof *g_regions);
        }
        g_regions[g_count++] = (Region){(uintptr_t)p, size};
        mark((uintptr_t)&_mh_execute_header, (uintptr_t)p, (uintptr_t)p + size, 1);
    }
    pthread_mutex_unlock(&g_lock);
    return p;
}

void revo_low_free(void *p) {
    if (!p) return;
    pthread_mutex_lock(&g_lock);
    for (size_t i = 0; i < g_count; i++) {
        if (g_regions[i].at == (uintptr_t)p) {
            mark((uintptr_t)&_mh_execute_header, g_regions[i].at, g_regions[i].at + g_regions[i].size, 0);
            mach_vm_deallocate(mach_task_self(), g_regions[i].at, g_regions[i].size);
            g_regions[i] = g_regions[--g_count];
            break;
        }
    }
    pthread_mutex_unlock(&g_lock);
}

#elif defined(__linux__)
#include <errno.h>
#include <sys/mman.h>

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif

// The program is not position independent: the kernel maps it where it is
// linked (build.py, at 8 GB), whatever ASLR does to the rest.
extern const char __executable_start[];
extern char _end[];

uint8_t *revo_native_base(void) { return (uint8_t *)__executable_start; }
uint8_t *const revo_base = (uint8_t *)__executable_start;

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static uintptr_t g_next, g_end;

typedef struct { uintptr_t at; size_t size; } Region;
static Region *g_regions;
static size_t g_count, g_cap;

static void block_init(void) {
    if (g_end) return;
    const uintptr_t base = (uintptr_t)__executable_start;
    if (base & 0xFFFFFFFFu) {
        fprintf(stderr, "the program is not at an address aligned on 4 GB (%llX): it was linked as PIE\n",
                (unsigned long long)base);
        abort();
    }
    g_next = ((uintptr_t)_end + GRANULE - 1) & ~(uintptr_t)(GRANULE - 1);
    g_end = base + BLOCK_SIZE;
    mark(base, base, (uintptr_t)_end, 1);
}

void *revo_low_alloc(size_t size) {
    size = (size + GRANULE - 1) & ~(size_t)(GRANULE - 1);
    pthread_mutex_lock(&g_lock);
    block_init();
    void *p = NULL;
    for (uintptr_t a = g_next; !p && a + size <= g_end; a += GRANULE) {
        void *q = mmap((void *)a, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
                       -1, 0);
        if (q == MAP_FAILED) {
            if (errno != EEXIST) break;
            continue;
        }
        if ((uintptr_t)q != a) {  // a kernel older than MAP_FIXED_NOREPLACE: a hint it did not take
            munmap(q, size);
            continue;
        }
        p = q;
        g_next = a + size;
    }
    if (p) {
        if (g_count == g_cap) {
            g_cap = g_cap ? g_cap * 2 : 64;
            g_regions = realloc(g_regions, g_cap * sizeof *g_regions);
        }
        g_regions[g_count++] = (Region){(uintptr_t)p, size};
        mark((uintptr_t)__executable_start, (uintptr_t)p, (uintptr_t)p + size, 1);
    }
    pthread_mutex_unlock(&g_lock);
    return p;
}

void revo_low_free(void *p) {
    if (!p) return;
    pthread_mutex_lock(&g_lock);
    for (size_t i = 0; i < g_count; i++) {
        if (g_regions[i].at == (uintptr_t)p) {
            mark((uintptr_t)__executable_start, g_regions[i].at, g_regions[i].at + g_regions[i].size, 0);
            munmap((void *)g_regions[i].at, g_regions[i].size);
            g_regions[i] = g_regions[--g_count];
            break;
        }
    }
    pthread_mutex_unlock(&g_lock);
}

#else
#error "the native build has Windows, macOS and Linux (host/native_mem.c)"
#endif

__thread void *revo_ssp;
static __thread void *t_stack;
static pthread_key_t g_key;
static pthread_once_t g_once = PTHREAD_ONCE_INIT;

static void stack_release(void *p) { revo_low_free(p); }
static void key_init(void) { pthread_key_create(&g_key, stack_release); }

void revo_shadow_stack(size_t size) {
    if (t_stack) return;
    pthread_once(&g_once, key_init);
    t_stack = revo_low_alloc(size);
    if (!t_stack) {
        fprintf(stderr, "no memory in the game's block for a thread's shadow stack\n");
        abort();
    }
    pthread_setspecific(g_key, t_stack);
    revo_ssp = (uint8_t *)t_stack + size;
}

void revo_shadow_stack_ensure(void) {
    if (!t_stack) revo_shadow_stack(4u << 20);
}
