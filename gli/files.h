#ifndef GLI_FILES_H
#define GLI_FILES_H

#include "guest.h"

typedef struct CallerFrame { u32 size, path_at; } CallerFrame;

u32 file_open(const char *path, CallerFrame frame);
static inline bool file_opened(u32 file) { return file < 0xFu; }
u32 file_length(u32 file);
void file_close(u32 file);

u8 *file_read(u32 file, u32 *length);
void file_read_done(void);

void file_read_into(u32 file, GuestPtr buffer, u32 bytes);

void lower_in_place(GuestPtr string);

#endif
