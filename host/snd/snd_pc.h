#ifndef SND_PC_H
#define SND_PC_H
#include <stdint.h>
uint32_t snd_pc_mem_alloc(uint32_t);
void snd_pc_mem_free(uint32_t);
uint32_t snd_pc_file_read(uint32_t, uint32_t, uint32_t, uint32_t);
void snd_pc_task_signal(uint32_t);
uint32_t snd_pc_task_create(uint32_t, uint32_t, uint32_t);
void snd_pc_task_delete(uint32_t);
void snd_pc_log(uint32_t);
uint32_t snd_pc_mem_available(void);
uint32_t snd_pc_mem_largest(void);
void snd_pc_task_every(uint32_t, uint32_t);
void snd_pc_mixer_open(void);
uint32_t snd_pc_game_message(uint32_t, uint32_t, uint32_t);
void snd_pc_serve(uint32_t, uint32_t);
void snd_pc_channel_play(uint32_t, uint32_t);
void snd_pc_channel_volume(uint32_t, uint32_t, uint32_t);
void snd_pc_channel_stop(uint32_t);
uint32_t snd_pc_channel_ended(uint32_t);
void snd_pc_channel_rate(uint32_t, uint32_t);
uint32_t snd_pc_channel_position(uint32_t);
void snd_pc_channel_notify(uint32_t, uint32_t, uint32_t);
#endif
