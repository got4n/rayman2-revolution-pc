#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"
#include "ps2/r5900.h"
#ifdef PC_PORT
#include "PC/PC.h"
#endif

#define ADPCM_RPC_SET_STEREO 0x1000A
#define ADPCM_RPC_GET_STEREO 0x1000B

extern s32 gst_Env_2[ADPCM_VOICE_COUNT];
extern s32 inv_SND_lastFreeAdpcmVoiceCount;

void func_001687B0(AdpcmManager* manager, AdpcmData* voice, s32 playing);
s32 fn_iGetNumFreeVoiceTdstAdpcmMngr(AdpcmManager* manager);

AdpcmData* fn_pstParseTdstAdpcmMngr(AdpcmManager* manager, void* context,
                                     AdpcmVoiceCallback callback)
{
    AdpcmData* voice;
    AdpcmData* match = NULL;

    for (voice = manager->voices; voice < &manager->voices[ADPCM_VOICE_COUNT]; voice++) {
        if (callback(voice, context) != 0) {
            match = voice;
            break;
        }
    }
    return match;
}

AdpcmData* fn_pstIsValidVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* result = NULL;

    if (voice >= manager->voices && voice < &manager->voices[ADPCM_VOICE_COUNT]) {
        result = NULL;
        if (((u32) voice - (u32) manager->voices) % sizeof(AdpcmData) == 0) {
            result = voice;
        }
    }
    return result;
}

static inline s32 inv_SND_fn_bIsFree(AdpcmData* voice)
{
    return (voice->flags.allocated & 1) ^ 1;
}

s32 fn_iGetNumFreeVoiceCallbackTdstAdpcmMngr(AdpcmData* voice, void* context)
{
    s32* count = context;

    if (inv_SND_fn_bIsFree(voice)) {
        (*count)++;
    }
    return 0;
}

s32 fn_iAllocateVoiceCallbackTdstAdpcmMngr(AdpcmData* voice, void* context)
{
    return inv_SND_fn_bIsFree(voice);
}

s32 fn_iConstructCallbackTdstAdpcmMngr(AdpcmData* voice, void* context)
{
    u32* ok = context;
    s32 result = 0;

    if (fn_iConstructTdstAdpcmData(voice) >= 0) {
        result = *ok != 0;
    }
    *ok = result;
    return 0;
}

s32 fn_iDestructCallbackTdstAdpcmMngr(AdpcmData* voice, void* context)
{
    u32* ok = context;
    s32 result = 0;

    if (fn_iDestructTdstAdpcmData(voice) >= 0) {
        result = *ok != 0;
    }
    *ok = result;
    return 0;
}

s32 fn_iConstructTdstAdpcmMngr(AdpcmManager* manager)
{
    s32 result = -1;
    u32 ok;

    if (manager->constructed == 0) {
        ok = 1;
        fn_pstParseTdstAdpcmMngr(manager, &ok, fn_iConstructCallbackTdstAdpcmMngr);
        if (ok != 0) {
            manager->bytesRemaining = ADPCM_COMMAND_BUFFER_SIZE;
            manager->writeCursor = manager->commandBuffer;
            manager->constructed = 1;
            result = 0;
            manager->commandCount = 0;
        }
    }
    if (result < 0) {
        fn_iDestructTdstAdpcmMngr(manager);
    }
    return result;
}

s32 fn_iDestructTdstAdpcmMngr(AdpcmManager* manager)
{
    s32 result = -1;
    u32 ok = 1;

    fn_pstParseTdstAdpcmMngr(manager, &ok, fn_iDestructCallbackTdstAdpcmMngr);
    if (ok != 0) {
        manager->constructed = 0;
        result = 0;
    }
    return result;
}

s32 fn_iGetNumFreeVoiceTdstAdpcmMngr(AdpcmManager* manager)
{
    s32 count = 0;

    fn_pstParseTdstAdpcmMngr(manager, &count, fn_iGetNumFreeVoiceCallbackTdstAdpcmMngr);
    return count;
}

u32 fn_u32AllocateVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmResource* resource, AdpcmResource* block,
                                     SndParam2D* extra)
{
    u32 result = 0;
    AdpcmData* voice;

    voice = fn_pstParseTdstAdpcmMngr(manager, NULL, fn_iAllocateVoiceCallbackTdstAdpcmMngr);
    if (voice != NULL) {
        result = fn_u32AllocateTdstAdpcmData(voice, resource, block, extra);
    }
    inv_SND_lastFreeAdpcmVoiceCount = fn_iGetNumFreeVoiceTdstAdpcmMngr(manager);
    return result;
}

u32 fn_u32DesallocateVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid;
    u32 result = 0xFFFFFFFF;
    u32 index;

    valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    if (valid != NULL) {
        result = fn_u32DesallocateTdstAdpcmData(valid);
        if (result != 0xFFFFFFFF) {
            index = valid - manager->voices;
            if (index < ADPCM_VOICE_COUNT) {
                gst_Env_2[index] = 0;
            }
        }
    }
    inv_SND_lastFreeAdpcmVoiceCount = fn_iGetNumFreeVoiceTdstAdpcmMngr(manager);
    return result;
}

s32 fn_bIsAllocateVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    s32 result = -1;

    if (valid != NULL) {
        result = valid->flags.allocated & 1;
    }
    return result;
}

s32 fn_iStartVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice, AdpcmResource* environment,
                                AdpcmResource* resource, u32 value28, AdpcmDataCallback value2C)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    s32 result = -1;

    if (valid != NULL) {
        result = fn_iStartTdstAdpcmData(valid, environment, resource, value28, value2C);
    }
    return result;
}

s32 fn_iStopVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    s32 result = -1;

    if (valid != NULL) {
        result = fn_iStopTdstAdpcmData(valid);
    }
    return result;
}

s32 TMR_fn_ulFastGetInternalCounter(void)
{
#ifdef PC_PORT
    return (s32) pc_time_ticks(294912000);
#else
    s32 value;

    R5900_MFC0(value, 9);
    return value;
#endif
}

void func_00168798(void)
{
    memset(gst_Env_2, 0, sizeof(gst_Env_2));
}

void func_001687B0(AdpcmManager* manager, AdpcmData* voice, s32 playing)
{
    s32 index = voice - manager->voices;
    s32* started;
    AdpcmResource* state;
    f32 duration;
    f32 timeout;
    s32 elapsed;

    if ((u32) index >= ADPCM_VOICE_COUNT)
        return;
    if (playing == 0) {
        gst_Env_2[index] = 0;
        return;
    }
    started = &gst_Env_2[index];
    if (*started == 0) {
        *started = TMR_fn_ulFastGetInternalCounter();
        return;
    }
    state = voice->environment;
    if (state != NULL && state->status == 1 && !state->descriptor.flags.bStream && !state->descriptor.flags.bLooping) {
        duration = (f32) (state->block.words[0] / 16) * 28.0f / (f32) state->descriptor.uwRate;
        if (duration < 5.0f) {
            timeout = (duration + 0.2f) * 300000000.0f;
            elapsed = TMR_fn_ulFastGetInternalCounter() - *started;
            if (timeout < (f32) (u32) elapsed) {
                voice->flags.playing = 0;
                gst_Env_2[index] = 0;
            }
        }
    }
}

s32 fn_bIsPlayingVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid;
    s32 playing = 0;

    valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    if (valid != NULL) {
        playing = fn_bIsPlayingTdstAdpcmData(valid);
    }
    func_001687B0(manager, valid, playing);
    return playing;
}

s32 fn_bSetParamTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice, SndParam2D* param)
{
    AdpcmData* valid;
    s32 result = 0;

    valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    if (valid != NULL && fn_bIsPlayingTdstAdpcmData(valid) != 0) {
        result = 1;
        if (fn_bSetParamTdstAdpcmData(valid, param, &manager->bytesRemaining) != 0) {
            manager->commandCount++;
        }
    }
    return result;
}

u32 gu_fn_iCloseTdstFileInfo(AdpcmManager* manager, AdpcmData* voice, s32 mode, AdpcmResource* resource)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    u32 result = 0xFFFFFFFF;

    if (valid != NULL) {
        result = fn_u32PushNextBlockResourceMemTdstAdpcmData(valid, mode, resource);
    }
    return result;
}

void fn_vPauseTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);

    if (valid != NULL) {
        fn_vPauseTdstAdpcmData(valid);
    }
}

void fn_vResumeTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);

    if (valid != NULL) {
        fn_vResumeTdstAdpcmData(valid);
    }
}

s32 fn_rGetPosTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice)
{
    AdpcmData* valid = fn_pstIsValidVoiceTdstAdpcmMngr(manager, voice);
    s32 result = -1;

    if (valid != NULL) {
        result = fn_rGetPosTdstAdpcmData(valid);
    }
    return result;
}

void fn_vSetStereoTdstAdpcmMngr(AdpcmManager* manager, s32 stereo)
{
    fn_iRemoteCall(ADPCM_RPC_SET_STEREO, 1, NULL, 0, &stereo, sizeof(stereo));
}

s32 fn_bGetStereoTdstAdpcmMngr(AdpcmManager* manager)
{
    s32 reply[4];

    fn_iRemoteCall(ADPCM_RPC_GET_STEREO, 1, reply, 4, NULL, 0);
    return reply[0];
}

s32 fn_iSynchroVoiceCallbackTdstAdpcmMngr(AdpcmData* voice, void* context)
{
    fn_bIsPlayingTdstAdpcmData(voice);
    return 0;
}

void fn_vSynchroSoundTdstAdpcmMngr(AdpcmManager* manager)
{
    fn_pstParseTdstAdpcmMngr(manager, NULL, fn_iSynchroVoiceCallbackTdstAdpcmMngr);
    if (manager->commandCount != 0) {
        fn_iRemoteCall((u16) manager->commandCount | 0xFFFF0000, 0, NULL, 0, manager->commandBuffer,
                       ADPCM_COMMAND_BUFFER_SIZE - manager->bytesRemaining);
    }
    manager->writeCursor = manager->commandBuffer;
    manager->bytesRemaining = ADPCM_COMMAND_BUFFER_SIZE;
    manager->commandCount = 0;
}
