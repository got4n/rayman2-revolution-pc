#ifndef GLI_HANDLES_H
#define GLI_HANDLES_H

#include "guest.h"

typedef enum {
    OBJECT_CAMERA = 1, OBJECT_WORLD, OBJECT_MATERIAL, OBJECT_TEXTURE, OBJECT_RASTER, OBJECT_FRAME, OBJECT_ATOMIC,
    OBJECT_LIGHT, OBJECT_GEOMETRY, OBJECT_MORPH_TARGET,
} ObjectKind;

#define HANDLE_VIEW 0x40u
#define HANDLE_SPAN 0x00100000u
#define WINDOW_SPAN 0x00800000u
extern u32 revo_gli_window;
#define HANDLE_BASE revo_gli_window
#define HANDLE_LIMIT (HANDLE_BASE + HANDLE_SPAN)
#define WINDOW_LIMIT (HANDLE_BASE + WINDOW_SPAN)

u32 handle_new(ObjectKind kind, void *object);
void handle_free(u32 handle);

void *handle_object(ObjectKind kind, u32 value);

static inline bool is_handle(u32 value) { return value >= HANDLE_BASE && value < HANDLE_LIMIT; }

u32 *handle_view(u32 handle);

GuestPtr window_alloc(u32 size);
void window_free(GuestPtr buffer);

#endif
