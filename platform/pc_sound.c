#include <SDL.h>
#include <string.h>

#include "PC.h"
#include "pc_host.h"

#define MESSAGE_SIZE 1024
#define MESSAGE_COUNT 64

typedef struct Message {
    uint32_t function;
    uint32_t size;
    uint8_t bytes[MESSAGE_SIZE];
} Message;

static SDL_mutex *lock;
static Message messages[MESSAGE_COUNT];
static unsigned first, end;
static int receiver;

static int from_driver(uint32_t function, const void *bytes, uint32_t size)
{
    int queued = 0;
    int wake = 0;

    SDL_LockMutex(lock);
    if (end - first < MESSAGE_COUNT && size <= MESSAGE_SIZE) {
        Message *m = &messages[end++ % MESSAGE_COUNT];

        m->function = function;
        m->size = size;
        if (size != 0) {
            memcpy(m->bytes, bytes, size);
        }
        queued = 1;
        wake = receiver;
    }
    SDL_UnlockMutex(lock);
    if (wake != 0) {
        revo_kernel_wakeup_async(wake);
    }
    if (!queued) {
        static int refused;

        if (refused++ < 8) {
            revo_log("[sound] a message of the driver's refused (function 0x%X, %u bytes: %s)", function, size,
                     size > MESSAGE_SIZE ? "too long" : "the queue is full");
        }
    }
    return queued;
}

int32_t pc_sound_start(void)
{
    lock = SDL_CreateMutex();
    if (lock == NULL) {
        return 0;
    }
    return snd_start(from_driver);
}

void pc_sound_command(uint32_t function, PcAddress request, uint32_t requestSize, PcAddress reply,
                      uint32_t replySize)
{
    snd_command(function, request, requestSize, reply, replySize);
}

int32_t pc_sound_receive(uint32_t *function, void *buffer, uint32_t size, uint32_t *messageSize)
{
    int got = 0;

    SDL_LockMutex(lock);
    if (first != end) {
        const Message *m = &messages[first++ % MESSAGE_COUNT];
        const uint32_t n = m->size < size ? m->size : size;

        *function = m->function;
        *messageSize = m->size;
        memcpy(buffer, m->bytes, n);
        got = 1;
    }
    SDL_UnlockMutex(lock);
    return got;
}

void pc_sound_wait(void)
{
    int empty;

    SDL_LockMutex(lock);
    receiver = revo_thread_self();
    empty = first == end;
    SDL_UnlockMutex(lock);
    if (empty) {
        revo_thread_sleep();
    }
}
