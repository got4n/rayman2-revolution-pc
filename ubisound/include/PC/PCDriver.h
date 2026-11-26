#ifndef PC_PCDRIVER_H
#define PC_PCDRIVER_H

#ifndef PC_API
#define PC_API
#endif

#include <stdint.h>

typedef uint32_t PcAddress;

PC_API void pc_serve(PcAddress function, void *buffer);

PC_API int32_t pc_game_message(int32_t function, const void *data, uint32_t size);

PC_API int32_t pc_task_create(PcAddress function, uint32_t argument, int32_t priority);

PC_API void pc_task_signal(int32_t task);

PC_API void pc_task_every(int32_t task, uint32_t microseconds);

PC_API void pc_task_delete(int32_t task);

PC_API void *pc_mem_alloc(uint32_t size);
PC_API void pc_mem_free(void *memory);

PC_API uint32_t pc_mem_available(void);
PC_API uint32_t pc_mem_largest(void);

typedef struct PcFile {
    uint32_t sector;
    uint32_t size;
} PcFile;

#define PC_FILE_SECTOR_SIZE 2048

PC_API int32_t pc_file_read(const PcFile *file, uint32_t offset, void *buffer, uint32_t size);

#define PC_SOUND_CHANNELS 48

#define PC_SOUND_FULL 0x3FFF

PC_API void pc_mixer_open(void);

PC_API void pc_channel_play(int32_t channel, const void *sound);
PC_API void pc_channel_stop(int32_t channel);

PC_API void pc_channel_rate(int32_t channel, uint32_t rate);
PC_API void pc_channel_volume(int32_t channel, uint32_t left, uint32_t right);

PC_API int32_t pc_channel_ended(int32_t channel);

PC_API const void *pc_channel_position(int32_t channel);

PC_API void pc_channel_notify(int32_t channel, const void *at, int32_t task);

PC_API void pc_log(const char *text);

#define pc_memcpy(dst, src, n) __builtin_memcpy((dst), (src), (n))
#define pc_memset(dst, c, n) __builtin_memset((dst), (c), (n))

static inline int pc_memcmp(const void *a, const void *b, uint32_t n)
{
    const unsigned char *x = a;
    const unsigned char *y = b;
    uint32_t i;

    for (i = 0; i < n; i++)
        if (x[i] != y[i])
            return x[i] - y[i];
    return 0;
}

#endif
