#include "common.h"

#include "snd/snd.h"

#include "ps2/kernel_types.h"
#include "ps2/libc.h"

#include "include_asm.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#define ADPCM_RPC_ALLOCATE 0x10002
#define ADPCM_RPC_STOP 0x10003
#define ADPCM_RPC_PAUSE 0x10004
#define ADPCM_RPC_RESUME 0x10005
#define ADPCM_RPC_GET_POSITION 0x10008
#define ADPCM_RPC_PUSH_BLOCK 0x10009

#define ADPCM_RPC_WAIT 1
#define ADPCM_RPC_NO_WAIT 0

typedef struct AdpcmAllocateRequest {
    tdstBACKCall call;
    s32 unk0C;
    SndParam2D param;
    u32 voiceHandle;
    AdpcmDescriptor descriptor;
    AdpcmBlockInfo block;
    AdpcmBlockInfo nextBlock;
    s32 handles[2];
} AdpcmAllocateRequest;

typedef struct AdpcmPlayFlags {
    signed int bParamPitch : 1;
    signed int bParamVolume : 1;
    signed int bParamPan : 1;
    signed int b3 : 1;
    signed int b4 : 1;
    signed int bStream : 1;
    signed int bFromCD : 1;
    signed int bLooping : 1;
    signed int bInterleaved : 1;
} AdpcmPlayFlags;

typedef struct AdpcmPlayDescriptor {
    AdpcmPlayFlags flags;
    u32 lFormat;
    u8 cNbChannels;
    u8 ucBitsPerSample;
    u16 uwRate;
    u32 lVolume;
    u32 lStartPos;
    AdpcmBlockInfo block;
} AdpcmPlayDescriptor;

typedef struct AdpcmPlayRequest {
    s32 handles[2];
    u8 unk08[8];
    SndParam2D param;
    u32 voiceHandle;
    AdpcmPlayDescriptor descriptor;
    AdpcmBlockInfo nextBlock;
    s32 newHandles[2];
} AdpcmPlayRequest;

typedef struct AdpcmPushRequest {
    s32 handles[2];
    s32 unk08[2];
    s32 remoteHandles[2];
    s32 mode;
    AdpcmBlockInfo block;
} AdpcmPushRequest;

s32 DeleteSema(s32 sema);
s32 SignalSema(s32 sema);
s32 WaitSema(s32 sema);

static inline s32 inv_SND_fn_bIsAllocated(AdpcmData* data)
{
    return data->flags.allocated;
}

static inline s32 inv_SND_fn_bIsPlaying(AdpcmData* data)
{
    return data->flags.playing;
}

static inline s32 inv_SND_fn_bIsBlockRequested(AdpcmData* data)
{
    return data->flags.blockRequested;
}

static inline s32 inv_SND_fn_bIsLastBlock(AdpcmData* data)
{
    return data->flags.lastBlock;
}

static inline s32 inv_SND_fn_bIsStopping(AdpcmData* data)
{
    return data->flags.stopping;
}

static inline s32 inv_SND_fn_bIsStream(AdpcmDescriptor* descriptor)
{
    return descriptor->flags.bStream;
}

static inline s32 inv_SND_fn_bIsFromCD(AdpcmDescriptor* descriptor)
{
    return descriptor->flags.bFromCD;
}

#ifdef PC_PORT
s32 fn_iRemoteCallTdstAdpcmData(AdpcmData* data, void* unused)
#else
s32 fn_iRemoteCallTdstAdpcmData(AdpcmData* data)
#endif
{
#ifdef PC_PORT
    pc_sema_wait(data->sema);
#else
    WaitSema(data->sema);
#endif
    if (inv_SND_fn_bIsAllocated(data)) {
        data->active = 1;
    }
#ifdef PC_PORT
    pc_sema_signal(data->sema);
#else
    SignalSema(data->sema);
#endif
    return 0;
}

void fn_vCallTdstAdpcmData(AdpcmData* data)
{
    AdpcmDescriptor* descriptor = NULL;
    AdpcmResource* resource;

    if (data->environment != NULL) {
        descriptor = &data->environment->descriptor;
    }
#ifdef PC_PORT
    pc_sema_wait(data->sema);
#else
    WaitSema(data->sema);
#endif
    if (inv_SND_fn_bIsAllocated(data)) {
        data->active = 0;
        if (!descriptor->flags.bLooping) {
            if (data->callback != NULL && !inv_SND_fn_bIsBlockRequested(data)) {
                resource = data->resource;
                data->resource = NULL;
                data->flags.blockRequested = 1;
                data->environment = resource;
                data->callback(data->callbackArgument);
            }
            if (data->resource == NULL) {
                data->flags.playing = 0;
            }
        }
    }
#ifdef PC_PORT
    pc_sema_signal(data->sema);
#else
    SignalSema(data->sema);
#endif
}

s32 CreateSema(ee_sema_t* sema);

s32 fn_iConstructTdstAdpcmData(AdpcmData* data)
{
    ee_sema_t sema;
    SndParam2D* param;
    s32 result = -1;

    sema.option = (s32) data;
    sema.max_count = 1;
    sema.count = 0;
    sema.init_count = 0;
#ifdef PC_PORT
    data->sema = pc_sema_create(sema.init_count, sema.max_count);
#else
    data->sema = CreateSema(&sema);
#endif
#ifdef PC_PORT
    pc_sema_signal(data->sema);
#else
    SignalSema(data->sema);
#endif
    if (data->sema >= 0)
        result = 0;
    data->flags.allocated = 0;
    data->flags.started = 0;
    data->flags.playing = 0;
    data->flags.blockRequested = 0;
    data->flags.paused = 0;
    data->flags.lastBlock = 0;
    data->flags.stopping = 0;
    data->active = 0;
    data->remoteHandle0 = 0;
    data->remoteHandle1 = 0;
    data->voiceHandle = 0;
    data->environment = NULL;
    data->resource = NULL;
    param = &data->param;
    param->pitch = 0;
    param->volume = 0;
    param->pan = 0;
    param->dolby = 0;
    param->reverb = 0;
    data->callbackArgument = 0;
    data->callback = NULL;
    return result;
}

static inline void inv_SND_fn_vClearParam(SndParam2D* param)
{
    param->pitch = 0;
    param->volume = 0;
    param->pan = 0;
    param->dolby = 0;
    param->reverb = 0;
}

s32 fn_iDestructTdstAdpcmData(AdpcmData* data)
{
    if (data->sema >= 0) {
#ifdef PC_PORT
        pc_sema_delete(data->sema);
#else
        DeleteSema(data->sema);
#endif
    }
    data->flags.allocated = 0;
    data->flags.started = 0;
    data->flags.playing = 0;
    data->flags.blockRequested = 0;
    data->flags.paused = 0;
    data->flags.lastBlock = 0;
    data->flags.stopping = 0;
    data->sema = -1;
    data->active = 0;
    data->remoteHandle0 = 0;
    data->remoteHandle1 = 0;
    data->voiceHandle = 0;
    data->environment = NULL;
    data->resource = NULL;
    inv_SND_fn_vClearParam(&data->param);
    data->callbackArgument = 0;
    data->callback = NULL;
    return 0;
}

static inline void inv_SND_fn_vCopyFormatFlags(AdpcmFormatFlags* destination, AdpcmFormatFlags* source)
{
    destination->bParamPitch = source->bParamPitch;
    destination->bParamVolume = source->bParamVolume;
    destination->bParamPan = source->bParamPan;
    destination->b3 = source->b3;
    destination->b4 = source->b4;
    destination->bStream = source->bStream;
    destination->bFromCD = source->bFromCD;
    destination->bLooping = source->bLooping;
    destination->bInterleaved = source->bInterleaved;
}

static inline void inv_SND_fn_vCopyParam(SndParam2D* destination, SndParam2D* source)
{
    destination->pitch = source->pitch;
    destination->volume = source->volume;
    destination->pan = source->pan;
    destination->dolby = source->dolby;
    destination->reverb = source->reverb;
}

static inline void inv_SND_fn_vCopyBlock(AdpcmBlockInfo* destination, AdpcmBlockInfo* source)
{
    destination->words[0] = source->words[0];
    destination->words[1] = source->words[1];
    destination->words[2] = source->words[2];
    destination->words[3] = source->words[3];
}

u32 fn_u32AllocateTdstAdpcmData(AdpcmData* data, AdpcmResource* resource, AdpcmResource* block,
                                SndParam2D* param)
{
    AdpcmAllocateRequest request;
    AdpcmDescriptor* descriptor = &resource->descriptor;
    u32 result = 0;
    s32 bStream;
    s32 bFromCD;

    if (descriptor->lVolume != 0) {
        do {
            request.call.args[0] = (s32) data;
            request.call.callback = (BACKCallback) fn_iRemoteCallTdstAdpcmData;
            data->flags.allocated = 1;
            request.call.args[1] = (s32) data;
            data->voiceHandle = fn_u32AllocateVoiceBACKEnv(&request.call);
            if (data->voiceHandle == 0) {
                break;
            }
            do {
                inv_SND_fn_vCopyParam(&data->param, param);
                inv_SND_fn_vCopyParam(&request.param, param);
                request.voiceHandle = data->voiceHandle;
                inv_SND_fn_vCopyFormatFlags(&request.descriptor.flags, &descriptor->flags);
                request.descriptor.lFormat = descriptor->lFormat;
                request.descriptor.cNbChannels = descriptor->cNbChannels;
                request.descriptor.ucBitsPerSample = descriptor->ucBitsPerSample;
                request.descriptor.uwRate = descriptor->uwRate;
                request.descriptor.lVolume = descriptor->lVolume;
                request.descriptor.lStartPos = descriptor->lStartPos;
                bFromCD = descriptor->flags.bFromCD;
                bStream = descriptor->flags.bStream;
            } while (0);
            if (bStream && bFromCD) {
                inv_SND_fn_vCopyBlock(&request.block, &resource->block);
            } else {
                do {
                    do {
                        do {
                            request.block.words[0] = resource->block.words[0];
                            request.block.words[1] = resource->block.words[1];
                            request.block.words[2] = resource->block.words[2];
                            request.block.words[3] = resource->block.words[3];
                        } while (0);
                    } while (0);
                } while (0);
            }
            if (block != NULL) {
                request.nextBlock = block->block;
            } else {
                memset(&request.nextBlock, 0, sizeof(request.nextBlock));
            }
            if (fn_iRemoteCall(ADPCM_RPC_ALLOCATE, ADPCM_RPC_WAIT, request.handles,
                               sizeof(request.handles), &request.param, 0x40) < 0) {
                break;
            }
            data->flags.started = 1;
            data->flags.playing = 1;
            data->remoteHandle0 = request.handles[0];
            data->remoteHandle1 = request.handles[1];
            data->environment = resource;
            data->resource = block;
            result = (u32) data;
        } while (0);
    }
    if (result == 0) {
        fn_u32DesallocateTdstAdpcmData(data);
    }
    return result;
}

s32 fn_u32DesallocateTdstAdpcmData(AdpcmData* data)
{
    AdpcmDescriptor* descriptor = data->environment ? &data->environment->descriptor : NULL;
    s32 handles[2];

    if (data->remoteHandle1 != 0) {
        handles[0] = data->remoteHandle0;
        handles[1] = data->remoteHandle1;
        if (inv_SND_fn_bIsFromCD(descriptor)) {
            printf("Ai stop the theme\n");
        }
        fn_iRemoteCall(ADPCM_RPC_STOP, ADPCM_RPC_NO_WAIT, NULL, 0, handles, sizeof(handles));
    }
    if (data->voiceHandle != 0) {
        data->voiceHandle = fn_u32DesallocateVoiceBACKEnv(data->voiceHandle);
    }
    data->flags.allocated = 0;
    data->flags.started = 0;
    data->flags.playing = 0;
    data->flags.blockRequested = 0;
    data->flags.paused = 0;
    data->flags.lastBlock = 0;
    data->flags.stopping = 0;
    data->active = 0;
    data->remoteHandle0 = 0;
    data->remoteHandle1 = 0;
    data->voiceHandle = 0;
    data->environment = NULL;
    data->resource = NULL;
    inv_SND_fn_vClearParam(&data->param);
    data->callbackArgument = 0;
    data->callback = NULL;
    return 0;
}

s32 fn_iStartTdstAdpcmData(AdpcmData* data, AdpcmResource* environment, AdpcmResource* resource,
                           u32 argument, AdpcmDataCallback callback)
{
    data->environment = environment;
    data->resource = resource;
    data->callbackArgument = argument;
    data->callback = callback;
    return 0;
}

s32 fn_iStopTdstAdpcmData(AdpcmData* data)
{
    data->flags.started = 0;
    return 0;
}

static inline void inv_SND_fn_vCopyPlayFlags(AdpcmPlayFlags* destination, AdpcmPlayFlags* source)
{
    destination->bParamPitch = source->bParamPitch;
    destination->bParamVolume = source->bParamVolume;
    destination->bParamPan = source->bParamPan;
    destination->b3 = source->b3;
    destination->b4 = source->b4;
    destination->bStream = source->bStream;
    destination->bFromCD = source->bFromCD;
    destination->bLooping = source->bLooping;
    destination->bInterleaved = source->bInterleaved;
}

static inline s32 inv_SND_fn_bIsPlayStream(AdpcmPlayDescriptor* descriptor)
{
    return descriptor->flags.bStream;
}

static inline s32 inv_SND_fn_bIsPlayFromCD(AdpcmPlayDescriptor* descriptor)
{
    return descriptor->flags.bFromCD;
}

s32 fn_bIsPlayingTdstAdpcmData(AdpcmData* data)
{
    AdpcmPlayDescriptor* descriptor;
    AdpcmPlayRequest request;
    s32 playing = 0;

    if (inv_SND_fn_bIsAllocated(data)) {
        if (inv_SND_fn_bIsPlaying(data)) {
            if (data->active) {
                if (inv_SND_fn_bIsLastBlock(data)) {
                    if (!inv_SND_fn_bIsStopping(data)) {
                        data->flags.stopping = 1;
                        request.handles[0] = data->remoteHandle0;
                        request.handles[1] = data->remoteHandle1;
                        if (fn_iRemoteCall(ADPCM_RPC_STOP, ADPCM_RPC_WAIT, NULL, 0, request.handles,
                                           sizeof(request.handles)) >= 0) {
                            printf("fn_bIsPlayingTdstAdpcmData the FADE is done we stop\n");
                        }
                    } else {
                        descriptor = NULL;
                        if (data->environment != NULL) {
                            descriptor = (AdpcmPlayDescriptor*) &data->resource->descriptor;
                        }
                        inv_SND_fn_vCopyParam(&request.param, &data->param);
                        request.voiceHandle = data->voiceHandle;
                        inv_SND_fn_vCopyPlayFlags(&request.descriptor.flags, &descriptor->flags);
                        request.descriptor.lFormat = descriptor->lFormat;
                        request.descriptor.cNbChannels = descriptor->cNbChannels;
                        request.descriptor.ucBitsPerSample = descriptor->ucBitsPerSample;
                        request.descriptor.uwRate = descriptor->uwRate;
                        request.descriptor.lVolume = descriptor->lVolume;
                        request.descriptor.lStartPos = descriptor->lStartPos;
                        if (inv_SND_fn_bIsPlayStream(descriptor) && inv_SND_fn_bIsPlayFromCD(descriptor)) {
                            inv_SND_fn_vCopyBlock(&request.descriptor.block, &descriptor->block);
                        } else {
                            request.descriptor.block.words[0] = descriptor->block.words[0];
                            request.descriptor.block.words[1] = descriptor->block.words[1];
                            request.descriptor.block.words[2] = descriptor->block.words[2];
                            request.descriptor.block.words[3] = descriptor->block.words[3];
                        }
                        memset(&request.nextBlock, 0, sizeof(request.nextBlock));
                        printf("fn_bIsPlayingTdstAdpcmData the FADE is done we start again\n");
                        if (fn_iRemoteCall(ADPCM_RPC_ALLOCATE, ADPCM_RPC_WAIT, request.newHandles,
                                           sizeof(request.newHandles), &request.param,
                                           0x40) >= 0) {
                            data->remoteHandle0 = request.newHandles[0];
                            data->remoteHandle1 = request.newHandles[1];
                            data->flags.stopping = 0;
                            data->flags.lastBlock = 0;
                        }
                    }
                } else {
                    fn_vCallTdstAdpcmData(data);
                }
            }
            playing = inv_SND_fn_bIsPlaying(data) != 0;
        }
        if (!playing) {
            fn_u32DesallocateTdstAdpcmData(data);
        }
    }
    return playing;
}

typedef struct AdpcmParamPayload {
    s32 remoteHandle0;
    s32 remoteHandle1;
    SndParam2D param;
} AdpcmParamPayload;

s32 fn_bSetParamTdstAdpcmData(AdpcmData* data, SndParam2D* param, void* out)
{
    AdpcmBlockInfo block;
    AdpcmParamPayload payload;
    gu_CommandInput chunk;
    gu_CommandInput input;
    AdpcmDescriptor* descriptor;
    u32 current;
    u32 value;
    s32 result = 0;

    if (fn_bIsPlayingTdstAdpcmData(data)) {
        if (!(*(u32*) &data->flags & 0x30)) {
            current = (u32) &data->param;
            if (memcmp((SndParam2D*) current, param, 8) != 0) {
                value = param->pitch;
                if ((u8) current & 0x100) {
                    chunk.size = chunk.size + input.size + current;
                }
                ((SndParam2D*) current)->pitch = value;
                input.size = 0x10;
                input.data = &payload;
                descriptor = data->environment ? &data->environment->descriptor : NULL;
                chunk = input;
                ((SndParam2D*) current)->volume = param->volume;
                ((SndParam2D*) current)->pan = param->pan;
                ((SndParam2D*) current)->dolby = param->dolby;
                ((SndParam2D*) current)->reverb = param->reverb;
                payload.param.pitch = param->pitch;
                payload.param.volume = param->volume;
                payload.param.pan = param->pan;
                payload.param.dolby = param->dolby;
                payload.param.reverb = param->reverb;
                payload.remoteHandle0 = data->remoteHandle0;
                payload.remoteHandle1 = data->remoteHandle1;
                if (descriptor != NULL && inv_SND_fn_bIsFromCD(descriptor)) {
                    printf("fn_bSetParamTdstAdpcmData %d vol statique %d vol dynamic %d\n", data, descriptor->lVolume, payload.param.volume);
                    if (payload.param.volume == 0) {
                        printf("Stream null\n");
                    }
                }
                result = fn_u32FormatCommandChunk(0x10006, out, &chunk, 8) != 0;
            }
        }
    }
    return result;
}

u32 fn_u32PushNextBlockResourceMemTdstAdpcmData(AdpcmData* data, s32 mode, AdpcmResource* resource)
{
    AdpcmDescriptor* descriptor;
    AdpcmPushRequest request;
    u32 result = 0;

    if (fn_bIsPlayingTdstAdpcmData(data)) {
        descriptor = data->environment ? &data->environment->descriptor : NULL;
        if (inv_SND_fn_bIsStream(descriptor)) {
            if (mode != 0) {
                data->flags.lastBlock = 1;
                printf("fn_u32PushNextBlockResourceMemTdstAdpcmData with FADE\n");
            }
            request.remoteHandles[0] = data->remoteHandle0;
            request.remoteHandles[1] = data->remoteHandle1;
            request.mode = mode;
            if (resource != NULL) {
                request.block = resource->block;
            } else {
                memset(&request.block, 0, sizeof(request.block));
            }
            fn_iRemoteCall(ADPCM_RPC_PUSH_BLOCK, ADPCM_RPC_WAIT, request.handles,
                           sizeof(request.handles), request.remoteHandles, 0x1C);
            if (request.handles[1] == data->remoteHandle1) {
                data->resource = resource;
                data->flags.blockRequested = 0;
                result = (u32) data;
            }
        }
    }
    return result;
}

void fn_vPauseTdstAdpcmData(AdpcmData* data)
{
    s32 handles[2];

    if (fn_bIsPlayingTdstAdpcmData(data) && !inv_SND_fn_bIsLastBlock(data)) {
        data->flags.paused = 1;
        handles[0] = data->remoteHandle0;
        handles[1] = data->remoteHandle1;
        fn_iRemoteCall(ADPCM_RPC_PAUSE, ADPCM_RPC_NO_WAIT, NULL, 0, handles, sizeof(handles));
    }
}

void fn_vResumeTdstAdpcmData(AdpcmData* data)
{
    s32 handles[2];

    if (fn_bIsPlayingTdstAdpcmData(data) && !inv_SND_fn_bIsLastBlock(data)) {
        handles[0] = data->remoteHandle0;
        handles[1] = data->remoteHandle1;
        fn_iRemoteCall(ADPCM_RPC_RESUME, ADPCM_RPC_NO_WAIT, NULL, 0, handles, sizeof(handles));
        data->flags.paused = 0;
    }
}

s32 fn_rGetPosTdstAdpcmData(AdpcmData* data)
{
    volatile s32 handles[2];
    s32 answer[4];
    s32 position = -1;

    if (fn_bIsPlayingTdstAdpcmData(data)) {
        if (!inv_SND_fn_bIsLastBlock(data)) {
            handles[1] = data->remoteHandle1;
            handles[0] = data->remoteHandle0;
            if (fn_iRemoteCall(ADPCM_RPC_GET_POSITION, ADPCM_RPC_NO_WAIT, &answer[0], sizeof(s32),
                               (s32*) handles, sizeof(s32) * 2) >= 0) {
                position = answer[0];
            }
        } else {
            position = 0;
        }
    }
    return position;
}

void fn_vResetTdstAdpcmData(AdpcmData* data)
{
    data->flags.allocated = 0;
    data->flags.started = 0;
    data->flags.playing = 0;
    data->flags.blockRequested = 0;
    data->flags.paused = 0;
    data->flags.lastBlock = 0;
    data->flags.stopping = 0;
    data->active = 0;
    data->remoteHandle0 = 0;
    data->remoteHandle1 = 0;
    data->voiceHandle = 0;
    data->environment = NULL;
    data->resource = NULL;
    inv_SND_fn_vClearParam(&data->param);
    data->callbackArgument = 0;
    data->callback = NULL;
}
