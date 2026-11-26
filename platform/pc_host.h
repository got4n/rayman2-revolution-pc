#ifndef PC_HOST_H
#define PC_HOST_H

#include <stddef.h>
#include <stdint.h>

uint32_t revo_pad_buttons(void);
void revo_pad_sticks(uint8_t sticks[4]);

int revo_disc_find(const char *name, uint32_t *sector, uint32_t *size);
int revo_disc_read_bytes(uint32_t sector, uint32_t offset, uint32_t size, uint8_t *buffer);

int snd_start(int (*post)(uint32_t function, const void *bytes, uint32_t size));
void snd_command(uint32_t function, uint32_t request, uint32_t request_size, uint32_t reply, uint32_t reply_size);

void revo_kernel_wakeup_async(int thread);

uint64_t revo_frame_count(void);
void revo_kernel_poll(void);

int revo_thread_create(uint32_t entry, uint32_t stack, uint32_t stack_size, int priority);
int revo_thread_start(int id, uint32_t argument);
int revo_thread_stop(int id);
int revo_thread_delete(int id);
int revo_thread_self(void);
int revo_thread_priority(int id);
void revo_thread_set_priority(int id, int priority);
void revo_thread_sleep(void);
void revo_thread_wake(int id);
void revo_thread_wait_vblank(void);
void revo_frame_vblank_signal(int sema);
int revo_sema_create(int count, int max);
void revo_sema_delete(int id);
void revo_sema_wait(int id);
void revo_sema_signal(int id);
uint64_t revo_time_ticks(uint32_t frequency);
void revo_time_pause(uint32_t frequency);
void revo_time_pause_until(uint64_t until);
uint64_t revo_now_ns(void);
uint64_t revo_vblank_ns(void);
uint64_t revo_vblank_period_ns(void);
int revo_fast_forwarding(void);

int revo_mod(int mod);

void revo_log(const char *format, ...);
void revo_user_dir(char *out, size_t size);

#endif
