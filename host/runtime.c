#include "game_vars.h"
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "glue_rt.h"
#include "runtime.h"

u8 *RAM;
int revo_trace = 0;

void revo_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void revo_fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "FATAL: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    revo_dump_ring();
    exit(2);
}

void revo_fatal_unresolved(const char *name) { revo_fatal("C called %s, which has no provider", name); }

static pthread_mutex_t g_ext_ram_lock = PTHREAD_MUTEX_INITIALIZER;
void revo_ext_ram_lock(void) { pthread_mutex_lock(&g_ext_ram_lock); }
void revo_ext_ram_unlock(void) { pthread_mutex_unlock(&g_ext_ram_lock); }

u8 *revo_ptr(u32 a) { return a < REVO_FAST_LIMIT ? RAM + a : NULL; }

u32 revo_ring[256];
unsigned revo_ring_i;
void revo_dump_ring(void);
static void unmapped(const char *what, u32 a) {
    static int count;
    if (count++ < 50) revo_log("[mem] unmapped %s 0x%08X (last entry %s)", what, a,
                                revo_name_of(revo_ring[(revo_ring_i - 1) & 255]));
}

#define SLOW_RD(T, name)                                                   \
    T name(u32 a) {                                                       \
        T v = 0;                                                          \
        u8 *p = revo_ptr(a);                                              \
        if (!p) { unmapped("read", a); return 0; }                        \
        memcpy(&v, p, sizeof(T));                                         \
        return v;                                                         \
    }
#define SLOW_WR(T, name)                                                   \
    void name(u32 a, T v) {                                               \
        u8 *p = revo_ptr(a);                                              \
        if (!p) { unmapped("write", a); return; }                         \
        memcpy(p, &v, sizeof(T));                                         \
    }
SLOW_RD(u8, mem_slow_rd8)
SLOW_RD(u16, mem_slow_rd16)
SLOW_RD(u32, mem_slow_rd32)
SLOW_RD(u64, mem_slow_rd64)
SLOW_WR(u8, mem_slow_wr8)
SLOW_WR(u16, mem_slow_wr16)
SLOW_WR(u32, mem_slow_wr32)
SLOW_WR(u64, mem_slow_wr64)

void revo_mem_slow_read(u64 addr, void *dst, unsigned n) {
    if (addr >> 32) { unmapped("read(>4G)", (u32)addr); memset(dst, 0, n); return; }
    u8 *p = revo_ptr((u32)addr);
    if (!p) { unmapped("read", (u32)addr); memset(dst, 0, n); return; }
    memcpy(dst, p, n);
}
void revo_mem_slow_write(u64 addr, const void *src, unsigned n) {
    if (addr >> 32) { unmapped("write(>4G)", (u32)addr); return; }
    u8 *p = revo_ptr((u32)addr);
    if (!p) { unmapped("write", (u32)addr); return; }
    memcpy(p, src, n);
}

extern const u32 revo_entry_addrs[];
extern const unsigned revo_entry_count;
extern const char *const revo_entry_names[];
extern const char revo_entry_kinds[];

static int find_entry(u32 a) {
    unsigned lo = 0, hi = revo_entry_count;
    while (lo < hi) {
        unsigned mid = (lo + hi) / 2;
        if (revo_entry_addrs[mid] < a) lo = mid + 1;
        else hi = mid;
    }
    return (lo < revo_entry_count && revo_entry_addrs[lo] == a) ? (int)lo : -1;
}

const char *revo_name_of(u32 a) {
    int k = find_entry(a);
    if (k >= 0) return revo_entry_names[k];
    unsigned lo = 0, hi = revo_entry_count;
    while (lo < hi) {
        unsigned mid = (lo + hi) / 2;
        if (revo_entry_addrs[mid] <= a) lo = mid + 1;
        else hi = mid;
    }
    return lo ? revo_entry_names[lo - 1] : "?";
}

void revo_dump_ring(void) {
    fprintf(stderr, "  last function entries (oldest first):\n");
    for (unsigned k = 0; k < 256; k++) {
        u32 a = revo_ring[(revo_ring_i + k) & 255];
        if (a) fprintf(stderr, "    %08X %s\n", a, revo_name_of(a));
    }
}

u32 revo_break_addr = 0;
static int g_break_hits;

void revo_break_hit(u32 addr) {
    const int limit = 1000;
    if (++g_break_hits < limit) return;
    fprintf(stderr, "[break] %08X %s hit %d times\n", addr, revo_name_of(addr), g_break_hits);
    revo_dump_ring();
    exit(4);
}

void revo_div0(const char *where) {
    static unsigned n;
    if (n++ < 20)
        revo_log("[div0] integer division by zero in C (after %s, in %s)", revo_name_of(revo_ring[(revo_ring_i - 1) & 255]),
                 where);
}

void revo_nan_store(const char *where, u64 addr) {
    static const char *seen[40];
    static unsigned n;
    for (unsigned i = 0; i < n; i++)
        if (seen[i] == where) return;
    if (n >= 40) return;
    seen[n++] = where;
    revo_log("[nan] %s stores a NaN at %08X (after %s)", where, (u32)addr,
             revo_name_of(revo_ring[(revo_ring_i - 1) & 255]));
}

int revo_count_calls;
static u32 *g_call_counts;
typedef struct { const char *type; u32 target, count; } CiSeen;
static CiSeen g_ci[4096];
void revo_ci_seen(const char *type, u32 target) {
    u32 h = (target * 2654435761u ^ (u32)(uintptr_t)type) & 4095u;
    for (u32 k = 0; k < 4096; k++, h = (h + 1) & 4095u) {
        CiSeen *c = &g_ci[h];
        if (!c->type) { c->type = type; c->target = target; }
        if (c->type == type && c->target == target) { c->count++; return; }
    }
}
static void ci_seen_dump(void) {
    FILE *o = fopen("ci_counts.txt", "w");
    if (!o) return;
    for (u32 k = 0; k < 4096; k++)
        if (g_ci[k].type) fprintf(o, "%u %s %08X %s\n", g_ci[k].count, g_ci[k].type, g_ci[k].target, revo_name_of(g_ci[k].target));
    fclose(o);
}
void revo_count_calls_dump(void) {
    if (!g_call_counts) return;
    ci_seen_dump();
    FILE *o = fopen("call_counts.txt", "w");
    if (!o) return;
    for (u32 i = 0; i < 0xC0000u; i++)
        if (g_call_counts[i]) fprintf(o, "%08X %u %s\n", 0x00100000u + i * 4, g_call_counts[i], revo_name_of(0x00100000u + i * 4));
    fclose(o);
}
static void count_calls_on_signal(int sig) {
    (void)sig;
    revo_count_calls_dump();
    _exit(0);
}
static void *count_calls_writer(void *unused) {
    (void)unused;
    for (;;) {
        struct timespec ten = {10, 0};
        nanosleep(&ten, NULL);
        revo_count_calls_dump();
    }
    return NULL;
}
void revo_count_calls_init(void) {
    revo_count_calls = 1;
    revo_trace = 2;
    g_call_counts = calloc(0xC0000u, sizeof *g_call_counts);
    atexit(revo_count_calls_dump);
    pthread_t writer;
    if (!pthread_create(&writer, NULL, count_calls_writer, NULL)) pthread_detach(writer);
    signal(SIGTERM, count_calls_on_signal);
    signal(SIGINT, count_calls_on_signal);
}

static unsigned long g_heapcheck_every, g_heapcheck_from, g_entries;
#define HEAP_SBRK_BASE GAME_VAR(__malloc_sbrk_base)
#define HEAP_TOP GAME_VAR(off_388808)
#define HEAP_BINS_LO 0x003887F8u
#define HEAP_BINS_HI GAME_VAR(__malloc_trim_threshold)
static u32 heap_word(u32 a) { return a + 4 <= REVO_RAM_SIZE ? rd32(a) : 0xFFFFFFFFu; }
static void heap_check(void) {
    u32 base = heap_word(HEAP_SBRK_BASE), top = heap_word(HEAP_TOP);
    if (base == 0xFFFFFFFFu || !top || top >= REVO_RAM_SIZE) return;
    u32 c = (base + 7) & ~7u, prev = 0;
    const char *what = NULL;
    u32 bad = 0, val = 0;
    while (c < top) {
        u32 sz = heap_word(c + 4) & ~3u;
        if (sz < 16 || (sz & 7) || c + sz > top) { what = "a chunk's size"; bad = c + 4; val = heap_word(c + 4); break; }
        u32 next = c + sz;
        if (!(heap_word(next + 4) & 1)) {
            u32 fd = heap_word(c + 8), bk = heap_word(c + 12);
            bool fd_ok = (fd >= base && fd < top) || (fd >= HEAP_BINS_LO && fd < HEAP_BINS_HI);
            bool bk_ok = (bk >= base && bk < top) || (bk >= HEAP_BINS_LO && bk < HEAP_BINS_HI);
            if (!fd_ok || !bk_ok) { what = "a free chunk's links"; bad = c + 8; val = fd_ok ? bk : fd; break; }
        }
        prev = c;
        c = next;
    }
    if (!what && c != top) { what = "the walk's end (not the top chunk)"; bad = c; val = top; }
    static unsigned long walks;
    if ((walks++ & 255) == 0)
        revo_log("[heap] walk %lu at entry %lu: heap %08X-%08X, top's size %08X", walks, g_entries, base, top,
                 heap_word(top + 4));
    if (!what) return;
    revo_log("[heap] after %lu function entries: %s at %08X is %08X (chunk at %08X, previous chunk at %08X, "
             "its size %08X; heap %08X-%08X)", g_entries, what, bad, val, bad & ~7u, prev,
             prev ? heap_word(prev + 4) : 0, base, top);
    u32 from = (prev ? prev : bad) & ~15u, to = ((bad + 32) & ~15u);
    if (to - from > 0x800) from = to - 0x800;
    for (u32 a = from; a < to; a += 16)
        revo_log("[heap]   %08X: %08X %08X %08X %08X", a, heap_word(a), heap_word(a + 4), heap_word(a + 8),
                 heap_word(a + 12));
    revo_dump_ring();
    exit(5);
}
void revo_heapcheck_init(const char *spec) {
    g_heapcheck_every = strtoul(spec, NULL, 10);
    const char *colon = strchr(spec, ':');
    if (colon) g_heapcheck_from = strtoul(colon + 1, NULL, 10);
    if (!g_heapcheck_every) g_heapcheck_every = 1;
    revo_trace = 2;
    revo_log("[heap] checking newlib's heap every %lu function entries from entry %lu", g_heapcheck_every,
             g_heapcheck_from);
}

void revo_trace_enter(u32 addr) {
    if (addr == revo_break_addr) revo_break_hit(addr);
    if (g_heapcheck_every) {
        bool in_malloc = (addr >= 0x002178B8u && addr < 0x0022EAA0u) || addr == 0x001C13A8u || addr == 0x001C0D40u;
        if (++g_entries >= g_heapcheck_from && g_entries % g_heapcheck_every == 0 && !in_malloc) heap_check();
        else if (g_entries >= g_heapcheck_from && g_entries % g_heapcheck_every == 0) g_entries--;
        return;
    }
    if (revo_count_calls) {
        if (addr >= 0x00100000u && addr < 0x00400000u) g_call_counts[(addr - 0x00100000u) >> 2]++;
        return;
    }
    if (revo_trace < 2) return;
    int k = find_entry(addr);
    fprintf(stderr, "[F] %c %s\n", k >= 0 ? revo_entry_kinds[k] : '?', revo_name_of(addr));
}
