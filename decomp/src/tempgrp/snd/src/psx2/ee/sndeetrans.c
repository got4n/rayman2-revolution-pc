#include "common.h"

#include "snd/snd.h"
#include "FMN/FMN.h"

#define TRANSITION_BUFFER_SIZE 0x1400
#define TRANSITION_STEREO 2
#define TRANSITION_INVALID -1

typedef union TransitionSample {
    u32 word;

    struct {
        s16 left;
        s16 right;
    } channel;
} TransitionSample;

typedef struct TransitionBuffer {
    TransitionSample* samples;
    s32 size;
    s32 handle;
    struct TransitionBuffer* next;
    struct TransitionBuffer* prev;
    struct TransitionBufferList* list;
} TransitionBuffer;

typedef struct TransitionBufferList {
    TransitionBuffer* first;
    TransitionBuffer* last;
    s32 count;
} TransitionBufferList;

typedef struct Transition Transition;

typedef struct TransitionList {
    Transition* first;
    Transition* last;
    s32 count;
} TransitionList;

struct Transition {
    s32 buffer;
    s32 stopped : 1;
    s32 fading : 1;
    s32 sizeShift;
    s32 file;
    s32 unk10;
    u32 end;
    s32 position;
    TransitionSource* current;
    TransitionSource* next;
    s32 startPosition;
    TransitionSource* pending;
    s32 fade;
    s32 fadeShift;
    TransitionCallbackFn callback;
    void* callbackContext;
    TransitionBufferList buffers;
    Transition* listNext;
    Transition* listPrev;
    TransitionList* list;
};

typedef struct TransitionBufferDesc {
    SndSxdBufferDesc_inv description;
    SndSxdBufferFormat_inv format;
    s32 unk1C;
} TransitionBufferDesc;

extern TransitionList inv_SND_transitionList;

void* memset(void* data, s32 value, u32 size);
void FMN_Seek(s32 file, s32 offset, s32 origin);
s32 FMN_Read(s32 file, void* data, s32 size, u32 count);
void push_databuffer_transition(Transition* transition);
void purge_transition(Transition* transition);
void TransitionCallback(Transition* transition, s32 handle);

static inline Transition* inv_SND_fn_p_stTransitionOf(TransitionHandle handle)
{
    return (Transition*) handle;
}

void purge_databuffer_transition(TransitionBuffer* buffer)
{
    TransitionBufferList* list;

    if (buffer->next != NULL) {
        buffer->next->prev = buffer->prev;
    }
    if (buffer->prev != NULL) {
        buffer->prev->next = buffer->next;
    }
    list = buffer->list;
    if (list != NULL) {
        list->count--;
        list = buffer->list;
        if (list->first == buffer) {
            list->first = buffer->next;
            list = buffer->list;
        }
        if (list->last == buffer) {
            list->last = buffer->prev;
        }
    }
    buffer->next = NULL;
    buffer->prev = NULL;
    buffer->list = NULL;
    SND_fn_vFreeSnd(buffer->samples);
    SND_fn_vFreeSndEx(0, buffer);
}

void push_databuffer_transition(Transition* transition)
{
    s32 readBytes;
    TransitionPushArg pushArg;
    u8 path[0x100];
    TransitionBuffer* buffer;
    TransitionBufferList* list;
    s32 remaining;
    s32 capacity;
    s32 fadeActive = 0;
    u32 toRead;
    s32 i;
    s16 left;
    s16 right;
    s32 fade;

    if (transition->stopped) {
        return;
    }

    buffer = SND_fn_pvMallocSndEx(0, sizeof(TransitionBuffer));
    buffer->next = NULL;
    buffer->prev = NULL;
    buffer->list = NULL;
    buffer->size = TRANSITION_BUFFER_SIZE >> transition->sizeShift;
    buffer->samples = SND_fn_pvMallocSnd(TRANSITION_BUFFER_SIZE);
    capacity = TRANSITION_BUFFER_SIZE;
    remaining = TRANSITION_BUFFER_SIZE;
    list = &transition->buffers;

    do {
        if (transition->file >= 15U) {
            if (transition->next == NULL) {
                memset(buffer->samples, 0, remaining);
                remaining = 0;
                continue;
            }
            transition->current = transition->next;
            transition->next = NULL;
            transition->callback(transition->callbackContext);
            transition->startPosition = SND_fn_lGetPosBufferExSxd(transition->buffer);
            transition->file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
            FMN_Seek(transition->file, transition->position, 0);
        }

        toRead = transition->end - transition->position;
        if (toRead > capacity) {
            toRead = capacity;
        }
        if (transition->fading) {
            fadeActive = 1;
            if (toRead >= transition->fade) {
                toRead = transition->fade;
            }
        }

        readBytes = FMN_Read(transition->file, buffer->samples, 1, toRead);
        if (readBytes != capacity) {
            FMN_Close(transition->file);
            transition->file = -1;
            if (transition->fading) {
                if (readBytes == transition->fade) {
                    transition->next = transition->pending;
                    transition->pending = NULL;
                }
            }
            if (transition->next != NULL) {
                transition->current = transition->next;
                transition->next = NULL;
                transition->file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
                FMN_Seek(transition->file, transition->position, 0);
            } else {
                void* samples = buffer->samples;
                u8* data = samples;

                memset(data + readBytes, 0, remaining - readBytes);
            }
            if (!transition->fading || readBytes == transition->fade) {
                transition->callback(transition->callbackContext);
                transition->startPosition = SND_fn_lGetPosBufferExSxd(transition->buffer);
                transition->fading = 0;
            }
        } else {
            transition->position += readBytes;
        }

        if (fadeActive) {
            for (i = 0; i < (buffer->size << transition->sizeShift) / 4; i++) {
                fade = transition->fade;
                left = buffer->samples[i].channel.left;
                right = buffer->samples[i].channel.right;
                left = (left * fade) >> transition->fadeShift;
                right = (right * fade) >> transition->fadeShift;
                buffer->samples[i].word = right << 16 | (left & 0xFFFF);
                if (transition->current->channels == TRANSITION_STEREO) {
                    transition->fade -= 2;
                } else {
                    transition->fade -= 1;
                }
                if (transition->fade == 0) {
                    break;
                }
            }
        }
        remaining = capacity - readBytes;
    } while (remaining != 0);

    if (buffer->next != NULL) {
        buffer->next->prev = buffer->prev;
    }
    if (buffer->prev != NULL) {
        buffer->prev->next = buffer->next;
    }
    if (buffer->list != NULL) {
        buffer->list->count--;
        if (buffer->list->first == buffer) {
            buffer->list->first = buffer->next;
        }
        if (buffer->list->last == buffer) {
            buffer->list->last = buffer->prev;
        }
    }
    buffer->prev = NULL;
    buffer->list = NULL;
    buffer->next = NULL;

    buffer->prev = transition->buffers.last;
    buffer->list = list;
    if (transition->buffers.first == NULL) {
        transition->buffers.first = buffer;
    } else {
        transition->buffers.last->next = buffer;
    }
    transition->buffers.last = buffer;
    transition->buffers.count++;

    pushArg.size = buffer->size;
    pushArg.data = buffer->samples;
    buffer->handle = SND_fn_lPushBufferExSxd(transition->buffer, &pushArg);
}

void TransitionCallback(Transition* transition, s32 handle)
{
    TransitionBuffer* buffer;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x106);
    for (buffer = transition->buffers.first; buffer != NULL; buffer = buffer->next) {
        if (buffer->handle == handle) {
            purge_databuffer_transition(buffer);
            break;
        }
    }
    push_databuffer_transition(transition);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void purge_transition(Transition* transition)
{
    TransitionBuffer* buffer;
    TransitionBuffer* next;

    SND_fn_vDeleteBufferSxd(transition->buffer);
    transition->buffer = TRANSITION_INVALID;
    FMN_Close(transition->file);
    transition->file = TRANSITION_INVALID;
    for (buffer = transition->buffers.first, next = buffer ? buffer->next : NULL; buffer != NULL;
         buffer = next, next = buffer ? buffer->next : NULL) {
        purge_databuffer_transition(buffer);
    }
    if (transition->listNext != NULL) {
        transition->listNext->listPrev = transition->listPrev;
    }
    if (transition->listPrev != NULL) {
        transition->listPrev->listNext = transition->listNext;
    }
    if (transition->list != NULL) {
        transition->list->count--;
        if (transition->list->first == transition) {
            transition->list->first = transition->listNext;
        }
        if (transition->list->last == transition) {
            transition->list->last = transition->listPrev;
        }
    }
    transition->listNext = NULL;
    transition->listPrev = NULL;
    transition->list = NULL;
    SND_fn_vFreeSndEx(0, transition);
}

TransitionHandle SND_fn_lPlayTransitionSxdWave(TransitionSource* current, TransitionSource* next,
                                               TransitionCallbackFn callback, void* callbackContext,
                                               void* bufferContext)
{
    Transition* transition;
    TransitionList* list;
    TransitionBufferDesc desc;
    u8 path[0x100];
    s32 buffer;
    s32 file;
    s32 i;

    if (!current->positional) {
        return TRANSITION_INVALID;
    }
    if (!next->positional) {
        return TRANSITION_INVALID;
    }

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x14F);

    transition = SND_fn_pvMallocSndEx(0, sizeof(Transition));
    if (transition == NULL) {
        return TRANSITION_INVALID;
    }

    transition->list = NULL;
    transition->listNext = NULL;
    transition->listPrev = NULL;
    list = *(TransitionList* volatile*) &transition->list;
    if (list != NULL) {
        list->count = list->count - 1;
        list = transition->list;
        if (list->first == transition) {
            list->first = transition->listNext;
            list = transition->list;
        }
        if (list->last == transition) {
            list->last = transition->listPrev;
        }
    }
    transition->listPrev = NULL;
    transition->list = NULL;
    transition->listNext = NULL;
    transition->listPrev = (&inv_SND_transitionList)->last;
    transition->list = &inv_SND_transitionList;
    if (inv_SND_transitionList.first == NULL) {
        inv_SND_transitionList.first = transition;
    } else {
        (&inv_SND_transitionList)->last->listNext = transition;
    }
    inv_SND_transitionList.last = transition;
    inv_SND_transitionList.count++;

    transition->buffers.first = NULL;
    transition->buffers.last = NULL;
    transition->buffers.count = 0;

    desc.description.lFormat = 1;
    desc.description.unk04 = 0;
    desc.description.uwBitsPerSample = current->ucBitsPerSample;
    desc.description.uwNbChannels = current->channels;
    desc.description.rate = current->rate;
    desc.format.type = 2;
    desc.format.callback = TransitionCallback;
    desc.format.arg = 0;

    transition->callback = callback;
    transition->callbackContext = callbackContext;
    transition->unk10 = TRANSITION_INVALID;
    buffer = SND_fn_lCreateNewBufferExSxd(&desc.description, &desc.format, bufferContext, transition);
    transition->next = next;
    transition->current = current;
    transition->buffer = buffer;
    transition->startPosition = SND_fn_lGetPosBufferExSxd(buffer);

    transition->fading = 0;
    file = inv_FMN_fn_ulOpenVirtualOrFallback(path);
    transition->file = file;
    transition->stopped = 0;
    transition->sizeShift = (desc.description.uwBitsPerSample >> 3) + desc.description.uwNbChannels - 2;
    FMN_Seek(file, transition->position, 0);

    for (i = 2; i >= 0; i--) {
        push_databuffer_transition(transition);
    }

    SND_fn_vQuitCriticalSectionThreadSnd();

    return (TransitionHandle) transition;
}

s32 gu_SND_fn_rGetPosTransitionSxdWave(Transition* transition)
{
    s32 position;
    f32 played;

    if (transition == NULL) {
        return 0;
    }
    position = SND_fn_lGetPosBufferExSxd(transition->buffer) - transition->startPosition;
    if (position < 0) {
        position = 0;
    }
    if (position != 0) {
        played = position;
        return played / transition->current->rate * 65536.0f;
    }
    return 0;
}

s32 SND_fn_bSetParamTransitionSxdWave(TransitionHandle handle, _SoundParam* parameters)
{
    s32 result;

    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x1A8);
    if (handle == TRANSITION_INVALID) {
        result = 0;
    } else if (inv_SND_fn_p_stTransitionOf(handle)->stopped) {
        purge_transition(inv_SND_fn_p_stTransitionOf(handle));
        result = 0;
    } else {
        SND_fn_vSetParamBufferSxd(inv_SND_fn_p_stTransitionOf(handle)->buffer, parameters);
        result = 1;
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
    return result;
}

s32 SND_fn_bSetNextTransitionSxdWave(Transition* transition, TransitionSource* source)
{
    s32 result;

    if (!source->positional) {
        result = 0;
    } else {
        SND_fn_vEnterCriticalSectionThreadSnd_();
        SND_fn_vMouchardThreadsnd(
            "/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c", 0x1CF);
        transition->next = source;
        SND_fn_vQuitCriticalSectionThreadSnd();
        result = 1;
    }
    return result;
}

s32 SND_fn_bDoTransitionWithFadeSxdWave2(Transition* transition, TransitionSource* source,
                                         s32 duration)
{
    s32 shift;
    u32 bits;

    if (!source->positional) {
        return 0;
    }
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x1E3);
    transition->pending = source;
    if (!transition->fading) {
        transition->fading = 1;
        shift = 0;
        bits = (duration * transition->current->rate) >> 16;
        transition->fade = bits;
        if (bits != 0) {
            do {
                shift++;
                bits >>= 1;
            } while (bits != 0);
            transition->fade = 0;
        }
        transition->fadeShift = shift;
        transition->fade = 1 << shift;
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
    return 0;
}

s32 SND_fn_bDoTransitionWithFadeSxdWave(Transition* transition, TransitionSource* source)
{
    return SND_fn_bDoTransitionWithFadeSxdWave2(transition, source, 0x60000);
}

void SND_fn_vStopTransitionSxdWave(TransitionHandle handle)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x20B);
    if (handle != TRANSITION_INVALID) {
        inv_SND_fn_p_stTransitionOf(handle)->stopped = 1;
        purge_transition(inv_SND_fn_p_stTransitionOf(handle));
    }
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vPauseTransitionSxdWave(Transition* transition)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x21C);
    SND_fn_vPauseBufferSxd(transition->buffer);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void SND_fn_vResumeTransitionSxdWave(Transition* transition)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/psx2/ee/sndeetrans.c",
                              0x229);
    SND_fn_vResumeBufferSxd(transition->buffer);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

s32 SND_fn_iInitTransitionSxdWave(void *a0)
{
    return 3;
}
