#ifndef REVO_SND_H
#define REVO_SND_H

#include <stdio.h>

#include "../rt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*SndGamePost)(u32 function, const void *data, u32 size);
int snd_start(SndGamePost post);

void snd_command(u32 function, u32 request, u32 request_size, u32 reply, u32 reply_size);

void snd_dump(void);

u8 *snd_mem(u32 addr, u32 len);
void snd_log(const char *fmt, ...);
void snd_task_due(int task);

void snd_mixer_generate(s16 *out, int frames);
int snd_mixer_channels_on(void);
void snd_mixer_dump(void);

void revo_audio_push(const s16 *frames, int n);

#ifdef __cplusplus
}
#endif

#endif
