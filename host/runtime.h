#ifndef REVO_RUNTIME_H
#define REVO_RUNTIME_H

#include "rt.h"

#ifdef __cplusplus
extern "C" {
#endif

void revo_user_dir(char *out, size_t size);
void revo_log(const char *fmt, ...);
void revo_fatal(const char *fmt, ...) __attribute__((noreturn));
void revo_dump_ring(void);
const char *revo_name_of(u32 addr);
u8 *revo_ptr(u32 addr);
void revo_ext_ram_lock(void);
void revo_ext_ram_unlock(void);
u64 revo_ticks_ee(void);
void revo_mem_slow_read(u64 addr, void *dst, unsigned n);
void revo_mem_slow_write(u64 addr, const void *src, unsigned n);

const char *revo_str(u32 addr);

void revo_platform_init(void);
void revo_vblank(void);
int revo_platform_should_quit(void);
u32 revo_pad_buttons(void);
void revo_pad_sticks(u8 *s);

int revo_disc_open(const char *source);
u8 *revo_disc_load(const char *name, u32 *size);
int revo_disc_read_file(const char *name, u64 at, u32 size, u8 *dst);
extern char revo_bf_dir_default[1024];
/* Where SOUND's bigfiles were taken out to, a folder each (SNDENG for SNDENG.BF). */
extern char revo_sound_dir_default[1024];
/* The disc's i-th file (SOUND\SNDENG.BF), or NULL past the last. */
const char *revo_disc_file(int i, u32 *size);
#include <stdio.h>
FILE *revo_fopen(const char *path, const char *mode);
int revo_user_dir_pick(char *out, size_t size);
int revo_user_dir_set(const char *to, int *had_saves);
int revo_game_open(int argc, char **argv, const char *dev_default);
void revo_game_choose_again(void);
void revo_sha1_hex(const u8 *data, size_t size, char out[41]);
int revo_load_game(void);
void revo_start_full_program(int argc, char **argv);
int revo_full_program(void);

extern char revo_disc_image[1024];
extern char revo_assets_dir[1024];
extern char revo_save_dir[1024];
extern int revo_trace;

#ifdef __cplusplus
}
#endif

#endif
