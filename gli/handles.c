#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "handles.h"

typedef struct Slot {
    ObjectKind kind;
    void *object;
} Slot;

#define SLOTS (HANDLE_SPAN / HANDLE_VIEW)
static Slot g_slots[SLOTS];

#include "native_mem.h"
u32 revo_gli_window;
__attribute__((constructor)) static void window_reserve(void) {
    void *p = revo_low_alloc(WINDOW_SPAN);
    if (!p) {
        fprintf(stderr, "[gli] no memory in the game's block for the driver's window\n");
        abort();
    }
    revo_gli_window = (u32)(uintptr_t)p;
}

static u32 handle_of(u32 slot) { return HANDLE_BASE + HANDLE_VIEW * slot; }
static u32 slot_of(u32 handle) {
    if (!is_handle(handle) || (handle - HANDLE_BASE) % HANDLE_VIEW) return 0;
    return (handle - HANDLE_BASE) / HANDLE_VIEW;
}

u32 *handle_view(u32 handle) { return GUEST(u32, handle); }

u32 handle_new(ObjectKind kind, void *object) {
    for (u32 s = 1; s < SLOTS; s++)
        if (!g_slots[s].kind) {
            g_slots[s] = (Slot){kind, object};
            memset(handle_view(handle_of(s)), 0, HANDLE_VIEW);
            return handle_of(s);
        }
    revo_log("[gli] out of handles");
    abort();
}

void handle_free(u32 handle) {
    u32 s = slot_of(handle);
    if (!s) return;
    g_slots[s] = (Slot){0, NULL};
    memset(handle_view(handle), 0, HANDLE_VIEW);
}

typedef struct Buffer {
    u32 lo, hi;
} Buffer;

static Buffer *g_buffers;
static int g_nbuffers, g_buffers_cap;

GuestPtr window_alloc(u32 size) {
    size = (size + 15u) & ~15u;
    u32 at = HANDLE_LIMIT;
    int k = 0;
    for (; k < g_nbuffers && g_buffers[k].lo - at < size; k++) at = g_buffers[k].hi;
    if (at + size > WINDOW_LIMIT || at + size < at) return 0;
    if (g_nbuffers == g_buffers_cap) {
        g_buffers_cap = g_buffers_cap ? g_buffers_cap * 2 : 256;
        g_buffers = realloc(g_buffers, sizeof *g_buffers * (size_t)g_buffers_cap);
    }
    memmove(&g_buffers[k + 1], &g_buffers[k], sizeof *g_buffers * (size_t)(g_nbuffers - k));
    g_buffers[k] = (Buffer){at, at + size};
    g_nbuffers++;
    memset(GUEST(u8, at), 0, size);
    return at;
}

static int buffer_at(u32 p) {
    for (int k = 0; k < g_nbuffers; k++)
        if (g_buffers[k].lo == p) return k;
    return -1;
}

void window_free(GuestPtr buffer) {
    int k = buffer_at(buffer);
    if (k < 0) return;
    memmove(&g_buffers[k], &g_buffers[k + 1], sizeof *g_buffers * (size_t)(g_nbuffers - k - 1));
    g_nbuffers--;
}

void *handle_object(ObjectKind kind, u32 value) {
    u32 s = slot_of(value);
    return s && g_slots[s].kind == kind ? g_slots[s].object : NULL;
}
