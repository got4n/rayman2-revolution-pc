#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <stdint.h>
#include "../native_mem.h"
int ubi_start(void);

#include "../runtime.h"
#include "snd.h"

int revo_disc_read_bytes(u32 lsn, u32 offset, u32 size, u8 *dst);
void *revo_code(void *p);

u32 snd_guest_call(void *inst, u32 fn, const u32 *a, int n);

#define RATE 48000
#define BATCH 64
#define NTASK 32
#define NMSG 64
#define MSG_SIZE 0x400

static u8 *g_ram;
static u32 g_memsize;
static int g_trace;

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static void lock(void) { pthread_mutex_lock(&g_lock); }
static void unlock(void);

void snd_log(const char *fmt, ...) {
    char b[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(b, sizeof b, fmt, ap);
    va_end(ap);
    revo_log("[snd] %s", b);
}

u8 *snd_mem(u32 a, u32 len) {
    if (a >= g_memsize || len > g_memsize - a) return NULL;
    return g_ram + a;
}

static u8 *mem(u32 a, u32 len) {
    u8 *p = snd_mem(a, len);
    if (!p) revo_fatal("[snd] driver memory access out of bounds: %08X (%u bytes)", a, len);
    return p;
}

static const char *str(u32 a) {
    u8 *p = mem(a, 1);
    if (!memchr(p, 0, g_memsize - a)) revo_fatal("[snd] unterminated string at %08X", a);
    return (const char *)p;
}

typedef struct { u32 addr, size; int used; } Block;
static Block g_blk[512];
static int g_nblk;

static void heap_init(u32 base, u32 end) {
    base = (base + 15) & ~15u;
    g_blk[0] = (Block){base, end - base, 0};
    g_nblk = 1;
}

static u32 heap_alloc(u32 size) {
    size = (size + 15) & ~15u;
    if (!size) size = 16;
    int pick = -1;
    for (int i = 0; i < g_nblk; i++)
        if (!g_blk[i].used && g_blk[i].size >= size) { pick = i; break; }
    if (pick < 0 || g_nblk + 1 >= 512) return 0;
    Block b = g_blk[pick];
    g_blk[pick] = (Block){b.addr, size, 1};
    if (b.size > size) {
        memmove(&g_blk[pick + 2], &g_blk[pick + 1], sizeof(Block) * (size_t)(g_nblk - pick - 1));
        g_blk[pick + 1] = (Block){b.addr + size, b.size - size, 0};
        g_nblk++;
    }
    memset(g_ram + b.addr, 0, size);
    return b.addr;
}

static void heap_free(u32 addr) {
    for (int i = 0; i < g_nblk; i++) {
        if (g_blk[i].addr != addr || !g_blk[i].used) continue;
        g_blk[i].used = 0;
        if (i + 1 < g_nblk && !g_blk[i + 1].used) {
            g_blk[i].size += g_blk[i + 1].size;
            memmove(&g_blk[i + 1], &g_blk[i + 2], sizeof(Block) * (size_t)(g_nblk - i - 2));
            g_nblk--;
        }
        if (i > 0 && !g_blk[i - 1].used) {
            g_blk[i - 1].size += g_blk[i].size;
            memmove(&g_blk[i], &g_blk[i + 1], sizeof(Block) * (size_t)(g_nblk - i - 1));
            g_nblk--;
        }
        return;
    }
    if (addr) snd_log("pc_mem_free(%08X): not a block of the heap", addr);
}

static u32 heap_free_total(int total) {
    u32 t = 0, m = 0;
    for (int i = 0; i < g_nblk; i++)
        if (!g_blk[i].used) { t += g_blk[i].size; if (g_blk[i].size > m) m = g_blk[i].size; }
    return total ? t : m;
}

u32 snd_pc_mem_alloc(u32 size) { return heap_alloc(size); }
void snd_pc_mem_free(u32 addr) { heap_free(addr); }
u32 snd_pc_mem_available(void) { return heap_free_total(1); }
u32 snd_pc_mem_largest(void) { return heap_free_total(0); }

u32 snd_bad_call(u32 x);
u32 snd_bad_call(u32 x) { revo_fatal("[snd] call to %08X: no driver function there", x); }

u32 snd_guest_call(void *inst, u32 fn, const u32 *a, int n) {
    (void)inst;
    if (fn < 0x10000) return snd_bad_call(fn);
    u32 (*f)(u32, u32, u32, u32) = (u32 (*)(u32, u32, u32, u32))revo_code(revo_native_base() + fn);
    return f(n > 0 ? a[0] : 0, n > 1 ? a[1] : 0, n > 2 ? a[2] : 0, n > 3 ? a[3] : 0);
}
static u32 call(u32 fn, const u32 *a, int n) { return snd_guest_call(NULL, fn, a, n); }

typedef struct {
    int used;
    u32 fn, arg;
    int prio;
    int due;
    int again;
    u64 period, next;
} Task;
static Task g_task[NTASK];
static u64 g_frames;
static int g_running;

static pthread_mutex_t g_wake_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_wake_cv = PTHREAD_COND_INITIALIZER;
static int g_wake;
static volatile int g_kick;

static Task *task(u32 id) { return id > 0 && id < NTASK && g_task[id].used ? &g_task[id] : NULL; }

u32 snd_pc_task_create(u32 fn, u32 arg, u32 prio) {
    for (int i = 1; i < NTASK; i++)
        if (!g_task[i].used) {
            g_task[i] = (Task){.used = 1, .fn = fn, .arg = arg, .prio = (int)prio};
            if (g_trace) snd_log("task %d: function %X (%X), priority %d", i, fn, arg, (int)prio);
            return (u32)i;
        }
    snd_log("no room for a task");
    return 0;
}

void snd_task_due(int id) {
    Task *t = task((u32)id);
    if (!t) return;
    t->due++;
    g_kick = 1;
}

void snd_pc_task_signal(u32 id) { snd_task_due((int)id); }

void snd_pc_task_every(u32 id, u32 usec) {
    Task *t = task(id);
    if (!t) return;
    t->period = (u64)usec * RATE / 1000000ull;
    t->next = g_frames + t->period;
}

void snd_pc_task_delete(u32 id) {
    Task *t = task(id);
    if (t) memset(t, 0, sizeof *t);
}

static void run_tasks(void) {
    for (int i = 1; i < NTASK; i++) {
        Task *t = &g_task[i];
        if (!t->used || !t->period) continue;
        if (g_frames >= t->next + 4 * t->period) t->next = g_frames;
        while (g_frames >= t->next) { t->due++; t->next += t->period; }
    }
    for (int guard = 0; guard < 4096; guard++) {
        int best = 0;
        for (int i = 1; i < NTASK; i++)
            if (g_task[i].used && g_task[i].due > 0 && (!best || g_task[i].prio < g_task[best].prio)) best = i;
        if (!best) break;
        Task *t = &g_task[best];
        t->due--;
        g_running = best;
        const u32 a = t->arg;
        if (g_trace > 1) snd_log("task %d runs", best);
        const u32 r = call(t->fn, &a, 1);
        if (g_trace > 1) snd_log("task %d: %u", best, r);
        g_running = 0;
        if (r && t->used) t->again = 1;
    }
    for (int i = 1; i < NTASK; i++)
        if (g_task[i].again) { g_task[i].again = 0; g_task[i].due++; }
}

static u32 g_serve_fn, g_serve_buf;
static SndGamePost g_post;

void snd_pc_serve(u32 fn, u32 buf) {
    g_serve_fn = fn;
    g_serve_buf = buf;
    snd_log("serving the game's commands");
}

typedef struct { u32 function, size; u8 bytes[MSG_SIZE]; } Msg;
static Msg g_out[NMSG];
static int g_nout;

u32 snd_pc_game_message(u32 function, u32 data, u32 size) {
    if (size > MSG_SIZE || g_nout >= NMSG || !g_post) {
        static int refused;
        if (refused++ < 8) snd_log("a message to the game refused (function 0x%X, %u bytes)", function, size);
        return 0;
    }
    Msg *m = &g_out[g_nout++];
    m->function = function;
    m->size = size;
    if (size) memcpy(m->bytes, mem(data, size), size);
    if (g_trace) snd_log("message to the game 0x%08X (%u bytes)", function, size);
    return 1;
}

static pthread_mutex_t g_post_lock = PTHREAD_MUTEX_INITIALIZER;

static void unlock(void) {
    const int n = g_nout;
    Msg *out = n ? malloc(sizeof(Msg) * (size_t)n) : NULL;
    if (out) memcpy(out, g_out, sizeof(Msg) * (size_t)n);
    g_nout = 0;
    const int kick = g_kick;
    g_kick = 0;
    if (n) pthread_mutex_lock(&g_post_lock);
    pthread_mutex_unlock(&g_lock);
    if (n) {
        for (int i = 0; out && i < n; i++) g_post(out[i].function, out[i].bytes, out[i].size);
        pthread_mutex_unlock(&g_post_lock);
        free(out);
    }
    if (kick) {
        pthread_mutex_lock(&g_wake_lock);
        g_wake = 1;
        pthread_cond_signal(&g_wake_cv);
        pthread_mutex_unlock(&g_wake_lock);
    }
}

void snd_command(u32 function, u32 request, u32 request_size, u32 reply, u32 reply_size) {
    const u8 *src = request_size ? revo_ptr(request) : NULL;
    u8 *dst = reply_size ? revo_ptr(reply) : NULL;
    if (request_size > MSG_SIZE) {
        snd_log("a command of %u bytes: more than the driver's buffer", request_size);
        return;
    }
    if (g_trace > 1) snd_log("command 0x%08X from the game", function);
    lock();
    if (!g_serve_fn) {
        unlock();
        return;
    }
    if (src) memcpy(mem(g_serve_buf, request_size), src, request_size);
    const u32 args[3] = {function, g_serve_buf, request_size};
    const u32 answer = call(g_serve_fn, args, 3);
    if (dst && answer) memcpy(dst, mem(answer, reply_size), reply_size);
    if (g_trace) snd_log("command 0x%08X (%u bytes): answer %08X", function, request_size, answer);
    unlock();
}

u32 snd_pc_file_read(u32 file, u32 offset, u32 buf, u32 size) {
    u32 sector;
    memcpy(&sector, mem(file, 8), 4);
    if (g_trace) snd_log("pc_file_read(sector %u +%u, %u bytes -> %06X)", sector, offset, size, buf);
    return revo_disc_read_bytes(sector, offset, size, mem(buf, size)) == 1;
}

void snd_pc_log(u32 text) { snd_log("%s", str(text)); }

u64 revo_ee_ns(void);
int revo_fast_forwarding(void);

static void audio_body(void *arg) {
    (void)arg;
    {
        static s16 silence[2048 * 2];
        revo_audio_push(silence, 2048);
    }
    const u64 t0 = revo_ee_ns();
    u64 frames = 0;
    int catching_up = 0;
    for (;;) {
        u64 target = (revo_ee_ns() - t0) * RATE / 1000000000ull;
        if (revo_fast_forwarding()) catching_up = 1;
        else if (catching_up && frames + RATE / 100 >= target) catching_up = 0;
        if (!catching_up && target > frames + RATE / 10) frames = target - BATCH;
        lock();
        g_frames = frames;
        run_tasks();
        unlock();
        while (frames + BATCH <= target) {
            s16 buf[BATCH * 2];
            lock();
            g_frames = frames;
            snd_mixer_generate(buf, BATCH);
            run_tasks();
            g_kick = 0;
            unlock();
            revo_audio_push(buf, BATCH);
            frames += BATCH;
        }
        const u64 next = t0 + (frames + BATCH) * 1000000000ull / RATE;
        pthread_mutex_lock(&g_wake_lock);
        const u64 now = revo_ee_ns();
        if (!g_wake && next > now) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            const u64 ns = (u64)ts.tv_nsec + (next - now);
            ts.tv_sec += (time_t)(ns / 1000000000ull);
            ts.tv_nsec = (long)(ns % 1000000000ull);
            pthread_cond_timedwait(&g_wake_cv, &g_wake_lock, &ts);
        }
        g_wake = 0;
        pthread_mutex_unlock(&g_wake_lock);
    }
}

static void *audio_main(void *arg) {
    revo_shadow_stack(1u << 20);
    audio_body(arg);
    return NULL;
}

int snd_start(SndGamePost post) {
    static int started;
    if (started) return 1;
    started = 1;
    g_post = post;
    {
        const u32 size = 6040u << 10;
        u8 *heap = revo_low_alloc(size);
        if (!heap) revo_fatal("[snd] no memory for the driver's heap");
        g_ram = revo_native_base();
        g_memsize = 0x80000000u;
        const u32 at = (u32)(heap - g_ram);
        heap_init(at, at + size);
    }
    lock();
    const int r = ubi_start();
    unlock();
    snd_log("driver started (%d): %u KB of heap", r, heap_free_total(1) >> 10);
    if (r < 0 || !g_serve_fn) return 0;
    pthread_attr_t at;
    pthread_attr_init(&at);
    pthread_attr_setstacksize(&at, 1u << 20);
    pthread_t th;
    const int e = pthread_create(&th, &at, audio_main, NULL);
    pthread_attr_destroy(&at);
    if (e) {
        snd_log("no audio thread");
        return 0;
    }
    return 1;
}

void snd_dump(void) {
    fprintf(stderr, "[snd] frames %llu, task running %d, channels on %d\n", (unsigned long long)g_frames, g_running,
            snd_mixer_channels_on());
    for (int i = 1; i < NTASK; i++)
        if (g_task[i].used)
            fprintf(stderr, "[snd task %d] function %X priority %d due %d every %llu frames\n", i, g_task[i].fn,
                    g_task[i].prio, g_task[i].due, (unsigned long long)g_task[i].period);
    snd_mixer_dump();
}
