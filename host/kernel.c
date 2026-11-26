#include "game_vars.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game_api.h"
#include "glue_rt.h"
#include "mods.h"
#include "runtime.h"

#define MAX_THREADS 64
#define MAX_SEMAS 256

enum { TH_FREE, TH_DORMANT, TH_READY, TH_RUN, TH_WAIT, TH_SLEEP, TH_EXT, TH_VBLANK };

typedef struct {
    int status;
    u32 entry, stack, stack_size, gp, prio, arg;
    int wakeups;
    int wait_sema;
    pthread_t th;
    int started;
    int gen;
} Thread;

typedef struct {
    int used;
    int count, max;
    u32 attr, option;
} Sema;

static Thread g_th[MAX_THREADS];
static Sema g_sema[MAX_SEMAS];
static int g_cur = 1;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cv = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_idle_cv = PTHREAD_COND_INITIALIZER;

static u64 g_last_vblank_ns;
static u64 g_frame;
u32 g_heap_end = 0;
static int g_in_irq;

static u64 real_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * 1000000000ull + (u64)ts.tv_nsec;
}

static volatile u32 g_seq;
static u64 g_real0, g_virt0;

static u64 now_ns(void) {
    u32 seq;
    u64 r0, v0;
    do {
        seq = __atomic_load_n(&g_seq, __ATOMIC_ACQUIRE);
        r0 = g_real0;
        v0 = g_virt0;
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
    } while ((seq & 1) || seq != __atomic_load_n(&g_seq, __ATOMIC_ACQUIRE));
    return v0 + (real_ns() - r0);
}

static u64 g_t0_ns;
__attribute__((constructor)) static void revo_note_t0(void) { g_t0_ns = now_ns(); }

static volatile int g_jumping;
int revo_fast_forwarding(void) { return g_jumping; }
static volatile int g_skipping;
int revo_skipping(void) { return g_skipping; }

u64 revo_ee_ns(void) { return now_ns(); }

static void jump_to(u64 due) {
    const u64 real = real_ns(), virt = g_virt0 + (real - g_real0);
    if (due <= virt) return;
    __atomic_add_fetch(&g_seq, 1, __ATOMIC_ACQ_REL);
    g_real0 = real;
    g_virt0 = due;
    __atomic_add_fetch(&g_seq, 1, __ATOMIC_ACQ_REL);
}
u64 revo_ticks_ee(void) { u64 t = now_ns() - g_t0_ns; return t / 1000 * 294912ull / 1000ull + t % 1000 * 294912ull / 1000000ull; }
u64 revo_ticks_bus(void) { return revo_ticks_ee() / 2;  }

static int pick_ready(void) {
    int best = -1;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (g_th[i].status != TH_READY && g_th[i].status != TH_RUN) continue;
        if (best < 0 || g_th[i].prio < g_th[best].prio) best = i;
    }
    return best;
}

static void run_interrupts_if_due(int force);

static __thread int t_gen;

static void wait_turn(int self) {
    for (;;) {
        if (self != 1 && t_gen != g_th[self].gen) {
            pthread_mutex_unlock(&g_lock);
            pthread_exit(NULL);
        }
        if (g_cur == self) return;
        pthread_cond_wait(&g_cv, &g_lock);
    }
}

static void retire_thread(int id) {
    g_th[id].gen++;
    g_th[id].started = 0;
}

#define VBLANK_NS 16666667ull

static u64 vblank_due(void) { return g_last_vblank_ns + VBLANK_NS; }

u64 revo_vblank_ns(void);
u64 revo_vblank_ns(void) { return g_last_vblank_ns; }
u64 revo_vblank_period_ns(void);
u64 revo_vblank_period_ns(void) { return VBLANK_NS; }
u64 revo_now_ns(void);
u64 revo_now_ns(void) { return now_ns(); }

static void reschedule(int self) {
    if (g_in_irq) return;
    for (;;) {
        int next = pick_ready();
        if (next < 0) {
            u64 due = vblank_due();
            u64 t = now_ns();
            int timed_out = 1;
            if (due > t) {
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                u64 ns = (u64)ts.tv_nsec + (due - t);
                ts.tv_sec += (time_t)(ns / 1000000000ull);
                ts.tv_nsec = (long)(ns % 1000000000ull);
                timed_out = pthread_cond_timedwait(&g_idle_cv, &g_lock, &ts) != 0;
            }
            run_interrupts_if_due(timed_out);
            if (revo_platform_should_quit()) exit(0);
            continue;
        }
        if (next == self) {
            g_th[self].status = TH_RUN;
            g_cur = self;
            return;
        }
        if (g_th[self].status == TH_RUN) g_th[self].status = TH_READY;
        g_cur = next;
        g_th[next].status = TH_RUN;
        pthread_cond_broadcast(&g_cv);
        wait_turn(self);
        return;
    }
}

typedef struct { int id, gen; } ThreadStart;

static void thread_body(void *p);

static void *thread_main(void *p) {
    void revo_shadow_stack(size_t size);
    revo_shadow_stack(8u << 20);
    thread_body(p);
    return NULL;
}

static void thread_body(void *p) {
    ThreadStart st = *(ThreadStart *)p;
    free(p);
    int id = st.id;
    t_gen = st.gen;
    pthread_mutex_lock(&g_lock);
    wait_turn(id);
    pthread_mutex_unlock(&g_lock);
    game_fn_void_u32(g_th[id].entry, g_th[id].arg);
    pthread_mutex_lock(&g_lock);
    g_th[id].status = TH_DORMANT;
    retire_thread(id);
    reschedule(id);
    pthread_mutex_unlock(&g_lock);
}

#define GAME_VBLANK_COUNTER GAME_VAR(inv_PS2_vblankCount)

int revo_letterboxed_within(unsigned vblanks);
int revo_skip_pressed(void);
void render_fast_forward(int on);
void revo_audio_mute(int on);
int revo_dialogue_fast(void);
int revo_short_loading_warm_up(void);

static void skip_cutscenes(void) {
    const int on = revo_mod(REVO_MOD_SKIP_CUTSCENES) && revo_letterboxed_within(60) && revo_skip_pressed();
    const int dialogue = revo_dialogue_fast();
    const int fast = on || dialogue;
    g_skipping = fast;
    g_jumping = fast || revo_short_loading_warm_up();
    render_fast_forward(fast);
    revo_audio_mute(fast);
}

void revo_enhance_vblank(void);

static void sixty_fps(void) {
    if (revo_mod(REVO_MOD_60FPS)) wr32(GAME_VBLANK_COUNTER, rd32(GAME_VBLANK_COUNTER) + 1);
}

static int g_vblank_sema;
static void wake_sema_waiters(int s);
static void sema_give(int id, int up_to_max) {
    if (id <= 0 || id >= MAX_SEMAS || !g_sema[id].used) return;
    if (up_to_max && g_sema[id].max > 0 && g_sema[id].count >= g_sema[id].max) return;
    g_sema[id].count++;
    wake_sema_waiters(id);
}
static void vblank_signal(void) { if (g_vblank_sema) sema_give(g_vblank_sema, 1); }

static void run_interrupts_if_due(int force) {
    if (g_in_irq) return;
    u64 t = now_ns();
    const u64 due = vblank_due();
    if (!force && t < due) return;
    g_last_vblank_ns = t >= due && t - due < VBLANK_NS ? due : t;
    g_frame++;
    for (int i = 1; i < MAX_THREADS; i++)
        if (g_th[i].status == TH_VBLANK) g_th[i].status = TH_READY;
    vblank_signal();
    revo_vblank();
    sixty_fps();
    skip_cutscenes();
    revo_enhance_vblank();
}

static volatile int g_preempt;

static void preempt_if_needed(void) {
    if (!__atomic_exchange_n(&g_preempt, 0, __ATOMIC_ACQ_REL)) return;
    pthread_mutex_lock(&g_lock);
    int next = pick_ready();
    if (next >= 0 && next != g_cur && g_th[next].prio < g_th[g_cur].prio) reschedule(g_cur);
    pthread_mutex_unlock(&g_lock);
}

static volatile int g_pause_wanted;
void revo_pause(int on) { __atomic_store_n(&g_pause_wanted, on, __ATOMIC_RELEASE); }

static void pause_here(void) {
    const u64 virt = now_ns();
    revo_audio_mute(1);
    revo_log("[kernel] paused");
    while (__atomic_load_n(&g_pause_wanted, __ATOMIC_ACQUIRE)) {
        struct timespec ts = {0, 10000000};
        nanosleep(&ts, NULL);
    }
    __atomic_add_fetch(&g_seq, 1, __ATOMIC_ACQ_REL);
    g_real0 = real_ns();
    g_virt0 = virt;
    __atomic_add_fetch(&g_seq, 1, __ATOMIC_ACQ_REL);
    revo_audio_mute(0);
    revo_log("[kernel] going on");
}

void revo_kernel_poll(void) {
    if (g_in_irq) return;
    if (__atomic_load_n(&g_pause_wanted, __ATOMIC_ACQUIRE)) pause_here();
    preempt_if_needed();
    if (now_ns() < vblank_due()) return;
    pthread_mutex_lock(&g_lock);
    run_interrupts_if_due(0);
    pthread_mutex_unlock(&g_lock);
}

void revo_wait_vblank(void) {
    pthread_mutex_lock(&g_lock);
    u64 due = vblank_due(), t = now_ns();
    if (due > t) {
        pthread_mutex_unlock(&g_lock);
        struct timespec ts = {0, (long)(due - t)};
        nanosleep(&ts, NULL);
        pthread_mutex_lock(&g_lock);
    }
    run_interrupts_if_due(1);
    pthread_mutex_unlock(&g_lock);
}

u64 revo_frame_count(void) { return g_frame; }

u64 revo_time_ticks(u32 frequency);
int revo_time_jump_next(u32 frequency) {
    if (g_in_irq || !g_jumping) return 0;
    pthread_mutex_lock(&g_lock);
    u64 due = g_last_vblank_ns + 16666667ull;
    if (frequency) {
        const u64 k = revo_time_ticks(frequency) + 1;
        const u64 tick = g_t0_ns + k / frequency * 1000000000ull + (k % frequency * 1000000000ull + frequency - 1) / frequency;
        if (tick < due) due = tick;
    }
    jump_to(due);
    run_interrupts_if_due(0);
    pthread_mutex_unlock(&g_lock);
    return 1;
}

void revo_sleep_ns(u64 ns);
void revo_time_pause_until(u64 until);

void revo_time_pause_until(u64 until) {
    u64 due = vblank_due();
    if (until < due) due = until;
    const u64 t = now_ns();
    u64 wait = due > t ? due - t : 0;
    if (wait > 1000000ull) wait = 1000000ull;
    if (wait < 50000ull) {
        sched_yield();
        return;
    }
    revo_sleep_ns(wait);
}

void revo_time_pause(u32 frequency) {
    u64 until = ~0ull;
    if (frequency) {
        const u64 k = revo_time_ticks(frequency) + 1;
        until = g_t0_ns + k / frequency * 1000000000ull + (k % frequency * 1000000000ull + frequency - 1) / frequency;
    }
    revo_time_pause_until(until);
}

static u32 new_thread(u32 entry, u32 stack, u32 stack_size, u32 gp, u32 prio) {
    for (int i = 2; i < MAX_THREADS; i++) {
        if (g_th[i].status != TH_FREE) continue;
        Thread *t = &g_th[i];
        int gen = t->gen;
        memset(t, 0, sizeof *t);
        t->gen = gen + 1;
        t->status = TH_DORMANT;
        t->entry = entry;
        t->stack = stack;
        t->stack_size = stack_size;
        t->gp = gp;
        t->prio = prio;
        return (u32)i;
    }
    return (u32)-1;
}

static u32 start_thread(int id, u32 arg) {
    if (id <= 0 || id >= MAX_THREADS || g_th[id].status != TH_DORMANT) return (u32)-1;
    Thread *t = &g_th[id];
    t->arg = arg;
    t->status = TH_READY;
    if (!t->started) {
        t->started = 1;
        pthread_attr_t at;
        pthread_attr_init(&at);
        pthread_attr_setstacksize(&at, 8u << 20);
        ThreadStart *st = malloc(sizeof *st);
        st->id = id;
        st->gen = t->gen;
        pthread_create(&t->th, &at, thread_main, st);
        pthread_detach(t->th);
    }
    return (u32)id;
}

static int new_sema(s32 count, s32 max, u32 attr) {
    for (int i = 1; i < MAX_SEMAS; i++) {
        if (g_sema[i].used) continue;
        g_sema[i].used = 1;
        g_sema[i].count = count;
        g_sema[i].max = max;
        g_sema[i].attr = attr;
        return i;
    }
    return -1;
}

static void wake_sema_waiters(int s) {
    while (g_sema[s].count > 0) {
        int best = -1;
        for (int i = 1; i < MAX_THREADS; i++)
            if (g_th[i].status == TH_WAIT && g_th[i].wait_sema == s &&
                (best < 0 || g_th[i].prio < g_th[best].prio))
                best = i;
        if (best < 0) break;
        g_sema[s].count--;
        g_th[best].status = TH_READY;
        g_th[best].wait_sema = 0;
    }
}

static u32 setup_thread(u32 gp, u32 stack, u32 size, u32 args, u32 root) {
    (void)args; (void)root;
    u32 top = stack == 0xFFFFFFFFu ? REVO_RAM_SIZE : stack + size;
    Thread *t = &g_th[1];
    t->status = TH_RUN;
    t->prio = 1;
    t->gp = gp;
    g_cur = 1;
    return (top - 0x20) & ~15u;
}

u32 revo_kernel_setup_main(u32 gp, u32 stack, u32 size) {
    pthread_mutex_lock(&g_lock);
    u32 top = setup_thread(gp, stack, size, 0, 0);
    pthread_mutex_unlock(&g_lock);
    return top;
}

void revo_kernel_setup_heap(u32 start, u32 size) {
    g_heap_end = size == 0xFFFFFFFFu ? 0x01E00000u : start + size;
}

u32 revo_kernel_heap_end(void) { return g_heap_end; }

void revo_kernel_init(void) {
    g_last_vblank_ns = now_ns();
    g_th[1].status = TH_RUN;
    g_th[1].prio = 1;
    g_cur = 1;
    pthread_mutex_lock(&g_lock);
}

void revo_kernel_start(void) { pthread_mutex_unlock(&g_lock); }

static pthread_t g_boot_pthread;
__attribute__((constructor)) static void revo_note_boot_thread(void) { g_boot_pthread = pthread_self(); }
void revo_kernel_set_boot_thread(void) { g_boot_pthread = pthread_self(); }
pthread_t revo_running_thread(void) {
    return g_th[g_cur].started && g_cur != 0 ? g_th[g_cur].th : g_boot_pthread;
}

#define REVO_BUDGET 4096
int revo_budget = REVO_BUDGET;
void revo_preempt(void) {
    revo_budget = REVO_BUDGET;
    if (g_in_irq) return;
    revo_kernel_poll();
    pthread_mutex_lock(&g_lock);
    int best = pick_ready(), self = g_cur;
    if (best >= 0 && best != self && g_th[best].prio < g_th[self].prio) reschedule(self);
    pthread_mutex_unlock(&g_lock);
}

void revo_kernel_dump(void) {
    static const char *names[] = {"free", "dormant", "ready", "run", "wait", "sleep", "ext", "vblank"};
    for (int i = 1; i < MAX_THREADS; i++) {
        if (g_th[i].status == TH_FREE) continue;
        fprintf(stderr, "[ee thread %d] %s prio %u entry %s sema %d wakeups %d%s\n", i, names[g_th[i].status],
                g_th[i].prio, revo_name_of(g_th[i].entry), g_th[i].wait_sema, g_th[i].wakeups, i == g_cur ? " (current)" : "");
    }
}

void revo_kernel_wait_ext(volatile int *flag, int want) {
    pthread_mutex_lock(&g_lock);
    int self = g_cur;
    while (__atomic_load_n(flag, __ATOMIC_ACQUIRE) != want) {
        if (g_in_irq) {
            pthread_mutex_unlock(&g_lock);
            struct timespec ts = {0, 100000};
            nanosleep(&ts, NULL);
            pthread_mutex_lock(&g_lock);
            continue;
        }
        g_th[self].status = TH_EXT;
        reschedule(self);
    }
    pthread_mutex_unlock(&g_lock);
}

void revo_kernel_wake_ext(void) {
    pthread_mutex_lock(&g_lock);
    for (int i = 1; i < MAX_THREADS; i++)
        if (g_th[i].status == TH_EXT) g_th[i].status = TH_READY;
    g_preempt = 1;
    pthread_cond_broadcast(&g_idle_cv);
    pthread_mutex_unlock(&g_lock);
}

void revo_kernel_wakeup_async(int tid) {
    if (tid <= 0 || tid >= MAX_THREADS) return;
    pthread_mutex_lock(&g_lock);
    if (g_th[tid].status == TH_SLEEP) g_th[tid].status = TH_READY;
    else g_th[tid].wakeups++;
    g_preempt = 1;
    pthread_cond_broadcast(&g_idle_cv);
    pthread_mutex_unlock(&g_lock);
}

void revo_kernel_wakeup(int tid) {
    if (tid <= 0 || tid >= MAX_THREADS) return;
    int locked = !g_in_irq;
    if (locked) pthread_mutex_lock(&g_lock);
    if (g_th[tid].status == TH_SLEEP) g_th[tid].status = TH_READY;
    else g_th[tid].wakeups++;
    if (locked) pthread_mutex_unlock(&g_lock);
}

static int sema_trace(void) { return 0; }

static int kernel_enter(void) {
    const int locked = !g_in_irq;
    if (locked) {
        pthread_mutex_lock(&g_lock);
        run_interrupts_if_due(0);
    }
    return locked;
}

static void kernel_leave(int locked) {
    if (locked) pthread_mutex_unlock(&g_lock);
}

static int valid_thread(int id) { return id > 0 && id < MAX_THREADS && g_th[id].status != TH_FREE; }

int revo_thread_create(u32 entry, u32 stack, u32 stack_size, int priority) {
    const int l = kernel_enter();
    const int id = (int)new_thread(entry, stack, stack_size, g_th[1].gp, (u32)priority);
    kernel_leave(l);
    return id;
}

int revo_thread_start(int id, u32 argument) {
    const int l = kernel_enter();
    const int ok = (s32)start_thread(id, argument) == id;
    kernel_leave(l);
    return ok;
}

int revo_thread_stop(int id) {
    const int l = kernel_enter();
    const int ok = valid_thread(id) && id != g_cur;
    if (ok) {
        g_th[id].status = TH_DORMANT;
        retire_thread(id);
        pthread_cond_broadcast(&g_cv);
    }
    kernel_leave(l);
    return ok;
}

int revo_thread_delete(int id) {
    const int l = kernel_enter();
    const int ok = id > 0 && id < MAX_THREADS && id != g_cur && g_th[id].status == TH_DORMANT;
    if (ok) {
        g_th[id].status = TH_FREE;
        retire_thread(id);
    }
    kernel_leave(l);
    return ok;
}

int revo_thread_self(void) { return g_cur; }

int revo_thread_priority(int id) {
    const int l = kernel_enter();
    const int p = valid_thread(id) ? (int)g_th[id].prio : -1;
    kernel_leave(l);
    return p;
}

void revo_thread_set_priority(int id, int priority) {
    const int l = kernel_enter();
    if (valid_thread(id)) {
        g_th[id].prio = (u32)priority;
        reschedule(g_cur);
    }
    kernel_leave(l);
}

void revo_thread_sleep(void) {
    const int l = kernel_enter();
    const int self = g_cur;
    if (g_th[self].wakeups > 0) g_th[self].wakeups--;
    else if (!g_in_irq) {
        g_th[self].status = TH_SLEEP;
        reschedule(self);
    }
    kernel_leave(l);
}

void revo_thread_wake(int id) {
    const int l = kernel_enter();
    if (valid_thread(id)) {
        if (g_th[id].status == TH_SLEEP) g_th[id].status = TH_READY;
        else g_th[id].wakeups++;
        reschedule(g_cur);
    }
    kernel_leave(l);
}

void revo_thread_wait_vblank(void) {
    const int l = kernel_enter();
    if (!g_in_irq) {
        const int self = g_cur;
        g_th[self].status = TH_VBLANK;
        reschedule(self);
    }
    kernel_leave(l);
}

int revo_sema_create(int count, int max) {
    const int l = kernel_enter();
    const int id = new_sema(count, max, 0);
    if (sema_trace()) revo_log("[sema] %d created by %d (%d, max %d)", id, g_cur, count, max);
    kernel_leave(l);
    return id;
}

void revo_sema_delete(int id) {
    const int l = kernel_enter();
    if (sema_trace()) revo_log("[sema] %d deleted by %d", id, g_cur);
    if (id > 0 && id < MAX_SEMAS) g_sema[id].used = 0;
    kernel_leave(l);
}

void revo_sema_wait(int id) {
    const int l = kernel_enter();
    if (id > 0 && id < MAX_SEMAS && g_sema[id].used) {
        if (sema_trace()) revo_log("[sema] %d wait by %d (count %d)", id, g_cur, g_sema[id].count);
        if (g_sema[id].count > 0) g_sema[id].count--;
        else if (!g_in_irq) {
            const int self = g_cur;
            g_th[self].status = TH_WAIT;
            g_th[self].wait_sema = id;
            reschedule(self);
        }
    }
    kernel_leave(l);
}

void revo_sema_signal(int id) {
    const int l = kernel_enter();
    if (id > 0 && id < MAX_SEMAS && g_sema[id].used) {
        if (sema_trace()) revo_log("[sema] %d signal by %d (count %d)", id, g_cur, g_sema[id].count);
        sema_give(id, 0);
        reschedule(g_cur);
    }
    kernel_leave(l);
}

void revo_frame_vblank_signal(int id) {
    const int l = kernel_enter();
    if (sema_trace()) revo_log("[sema] %d given at each vblank (by %d)", id, g_cur);
    g_vblank_sema = id;
    kernel_leave(l);
}

u64 revo_time_ticks(u32 frequency) {
    const u64 t = now_ns() - g_t0_ns;
    return t / 1000000000ull * frequency + t % 1000000000ull * frequency / 1000000000ull;
}
