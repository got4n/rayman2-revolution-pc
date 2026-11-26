#include "game_vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"

#define CURRENT_HEAP GAME_VAR(gp_6F74)
#define FILE_HEAP_SLOT 1

u32 file_open(const char *path, CallerFrame frame) {
    GuestPtr base = game_stack_push(frame.size);
    snprintf(GUEST(char, base + frame.path_at), 0x80, "%s", path);
    u32 file = inv_FMN_fn_ulOpenVirtualOrFallback(base + frame.path_at);
    game_stack_pop(frame.size);
    return file;
}

u32 file_length(u32 file) { return (u32)FMN_Length(file); }
void file_close(u32 file) { FMN_Close(file); }

u8 *file_read(u32 file, u32 *length) {
    *length = file_length(file);
    inv_MEM_fn_vCreateLevelSlotHeap(*length + 0x400, FILE_HEAP_SLOT);
    GuestPtr guest_bytes = AI_fn_p_vTrueAlloc(*GUEST(u32, CURRENT_HEAP), *length);
    FMN_Read(file, guest_bytes, 1, *length);
    file_close(file);
    if (!guest_bytes) return NULL;
    u8 *bytes = malloc(*length ? *length : 1);
    memcpy(bytes, GUEST(u8, guest_bytes), *length);
    return bytes;
}

void file_read_done(void) { inv_MEM_fn_vFreeLevelSlotHeap(FILE_HEAP_SLOT); }

void file_read_into(u32 file, GuestPtr buffer, u32 bytes) {
    FMN_Read(file, buffer, 1, bytes);
}

void lower_in_place(GuestPtr string) {
    for (char *c = GUEST(char, string); *c; c++)
        if (*c >= 'A' && *c <= 'Z') *c = (char)(*c + ('a' - 'A'));
}
