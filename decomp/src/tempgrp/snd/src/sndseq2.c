#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"

#define SEQUENCE_PLAYER_COUNT 15
#define NO_SOUND_HANDLE -1
#define NO_LOOP 0xFFFFFFFF

typedef void (*SndSequenceCallback)(s32 parameter);

typedef struct SndSequencePlayer {
    s32 playing : 1;
    s32 paused : 1;
    s32 idle : 1;
    s32 : 29;
    s32* activeOut;
    s32 soundActive;
    SndBinResource* sequence;
    SndSequenceElement* element;
    s32 index;
    s32 soundHandle;
    u32 loop;
    u32 loopStart;
    u32 loopEnd;
    _SoundParam* soundParams;
    s32 soundType;
    SndSequenceCallback callback;
    s32 callbackParameter;
    s32 busy;
} SndSequencePlayer;

extern SndSequencePlayer inv_SND_a_stSequencePlayer[SEQUENCE_PLAYER_COUNT];
extern s32 D_00393F78;
extern s32 D_00393F80;
extern u8 D_00393F7C;

s32 SND_fn_vNextElement(s32 player);

s32 get_SoundParam_member_size(_SoundParam* params)
{
    return params->microCount * 8 + 0x60;
}

void refresh_SoundParam_member(SndSequencePlayer* player, _SoundParam* params)
{
    _SoundParam* buffer;

    buffer = SND_fn_pvReAllocSndEx(0, player->soundParams, get_SoundParam_member_size(params));
    player->soundParams = buffer;
    memcpy(buffer, params, get_SoundParam_member_size(params));
}

void create_SoundParam_member(SndSequencePlayer* player, _SoundParam* params)
{
    _SoundParam* buffer;

    buffer = SND_fn_pvMallocSndEx(0, get_SoundParam_member_size(params));
    player->soundParams = buffer;
    memcpy(buffer, params, get_SoundParam_member_size(params));
}

void inv_SND_fn_vFreeSequenceSoundParams(SndSequencePlayer* player)
{
    SND_fn_vFreeSndEx(0, player->soundParams);
}

void SND_fn_vConvertResDiskToMemSeq(SndSequenceDisk* source, SndBinResource* destination, s32 unused)
{
    destination->seq.unk00 = source->unk00;
    destination->seq.type = source->type;
    destination->seq.unk08 = source->unk08;
    destination->seq.staticVolume = source->staticVolume;
    destination->seq.unk10 = 1;
    destination->seq.looping = source->looping;
    destination->seq.elementCount = source->elementCount;
    destination->seq.loopStart = source->loopStart;
    destination->seq.loopEnd = source->loopEnd;
    destination->seq.loopCount = source->loopCount - 1;
}

s32 SND_fn_bLoadResBinarySeq(SndSequenceDisk* source, SndBinResource* sequence, u32 base)
{
    s32 i;
    SndSequenceElement* element;

    base += source->elementsOffset;
    sequence->seq.elements = (SndSequenceElement*) base;
    element = sequence->seq.elements;
    sequence->seq.loadedElementCount = source->elementCount;
    for (i = 0; i < sequence->seq.loadedElementCount; i++) {
        element->resource.data = SND_fn_pGetBinRes(element->resource.id);
        element++;
    }
    SND_fn_vConvertResDiskToMemSeq(source, sequence, 0);
    return 1;
}

s32 SND_fn_iInitSeq(void)
{
    s32 i;

    for (i = 0; i < SEQUENCE_PLAYER_COUNT; i++) {
        inv_SND_a_stSequencePlayer[i].soundParams = NULL;
        inv_SND_a_stSequencePlayer[i].playing = 0;
        inv_SND_a_stSequencePlayer[i].paused = 0;
        inv_SND_a_stSequencePlayer[i].index = -1;
        inv_SND_a_stSequencePlayer[i].soundHandle = NO_SOUND_HANDLE;
        inv_SND_a_stSequencePlayer[i].idle = 1;
        inv_SND_a_stSequencePlayer[i].loop = 0;
        inv_SND_a_stSequencePlayer[i].loopStart = 0;
        inv_SND_a_stSequencePlayer[i].loopEnd = 0;
        inv_SND_a_stSequencePlayer[i].soundType = 0;
        inv_SND_a_stSequencePlayer[i].callback = NULL;
        inv_SND_a_stSequencePlayer[i].callbackParameter = 0;
    }
    D_00393F7C = 0;
    D_00393F78 = 1;
    D_00393F80 = 0;
    return 3;
}

s32 func_001617A8(void)
{
    return 0;
}

void inv_SND_fn_vDesInitSequencePlayers(void)
{
    s32 i;

    for (i = 0; i < SEQUENCE_PLAYER_COUNT; i++) {
        if (inv_SND_a_stSequencePlayer[i].soundParams != NULL) {
            inv_SND_fn_vFreeSequenceSoundParams(&inv_SND_a_stSequencePlayer[i]);
        }
        inv_SND_a_stSequencePlayer[i].playing = 0;
        inv_SND_a_stSequencePlayer[i].paused = 0;
        inv_SND_a_stSequencePlayer[i].index = -1;
        inv_SND_a_stSequencePlayer[i].soundHandle = NO_SOUND_HANDLE;
        inv_SND_a_stSequencePlayer[i].loop = 0;
        inv_SND_a_stSequencePlayer[i].loopStart = 0;
        inv_SND_a_stSequencePlayer[i].loopEnd = 0;
        inv_SND_a_stSequencePlayer[i].soundType = 0;
        inv_SND_a_stSequencePlayer[i].callback = NULL;
        inv_SND_a_stSequencePlayer[i].callbackParameter = 0;
        inv_SND_a_stSequencePlayer[i].idle = 1;
    }
}

s32 func_001618A8(void)
{
    return -1;
}

s32 func_001618B0(void)
{
    return 0;
}

void func_001618B8(void)
{
}

void func_001618C0(void)
{
}

void func_001618C8(void)
{
}

void test_next_element(s32 index, _SoundParam* params, s32 updateParams)
{
    s32 playing;

    playing = inv_SND_a_stSequencePlayer[index].soundActive == 0
                  ? 0
                  : (updateParams != 0 ? SND_fn_bSetParamSnd((PlaySndSlot*) inv_SND_a_stSequencePlayer[index].soundHandle, params)
                                       : SND_fn_bTestIsPlayingSnd((PlaySndSlot*) inv_SND_a_stSequencePlayer[index].soundHandle));
    if (playing != 0) {
        return;
    }
    inv_SND_a_stSequencePlayer[index].soundHandle = NO_SOUND_HANDLE;
    SND_fn_vNextElement(index);
}

s32 SND_fn_lPlaySeq(SndBinResource* sequence, _SoundParam* params, s32 type,
                    SndSequenceCallback callback, s32 callbackParameter, s32* activeOut)
{
    SndSequencePlayer* player;
    SndSequenceElement* element;
    s32 index;
    s32 soundHandle;

    if (activeOut != NULL) {
        *activeOut = 0;
    }
    for (index = 0; index < SEQUENCE_PLAYER_COUNT; index++) {
        if (!inv_SND_a_stSequencePlayer[index].playing) {
            break;
        }
    }
    if (index == SEQUENCE_PLAYER_COUNT) {
        SND_fn_vDisplayErrorEx(0x16, "", 3);
        return -1;
    }
    player = &inv_SND_a_stSequencePlayer[index];
    player->playing = 1;
    if (sequence->seq.elementCount == 0) {
        return -1;
    }
    if (sequence->seq.loopStart < sequence->seq.elementCount &&
        sequence->seq.loopEnd < sequence->seq.elementCount) {
        element = sequence->seq.elements;
        inv_SND_a_stSequencePlayer[index].element = element;
        soundHandle =
            SND_fn_lPlaySnd(element->resource.data, params, type, 0, 0, &inv_SND_a_stSequencePlayer[index].soundActive);
        if (soundHandle != NO_SOUND_HANDLE) {
            inv_SND_a_stSequencePlayer[index].sequence = sequence;
            inv_SND_a_stSequencePlayer[index].element = element;
            inv_SND_a_stSequencePlayer[index].loop = sequence->seq.loopCount;
            inv_SND_a_stSequencePlayer[index].loopStart = sequence->seq.loopStart;
            inv_SND_a_stSequencePlayer[index].loopEnd = sequence->seq.loopEnd;
            player->idle = 0;
            inv_SND_a_stSequencePlayer[index].activeOut = activeOut;
            inv_SND_a_stSequencePlayer[index].soundType = type;
            if (inv_SND_a_stSequencePlayer[index].soundParams != NULL) {
                refresh_SoundParam_member(player, params);
            } else {
                create_SoundParam_member(player, params);
            }
            inv_SND_a_stSequencePlayer[index].callback = callback;
            inv_SND_a_stSequencePlayer[index].callbackParameter = callbackParameter;
            inv_SND_a_stSequencePlayer[index].playing = 1;
            inv_SND_a_stSequencePlayer[index].paused = 0;
            inv_SND_a_stSequencePlayer[index].index = 0;
            inv_SND_a_stSequencePlayer[index].soundHandle = soundHandle;
            inv_SND_a_stSequencePlayer[index].busy = 0;
            if (inv_SND_a_stSequencePlayer[index].activeOut != NULL) {
                *inv_SND_a_stSequencePlayer[index].activeOut = 1;
            }
            return index;
        }
        player->playing = 0;
        return -1;
    }
    SND_fn_vDisplayError(0x1B, "");
    return -1;
}

s32 SND_fn_vNextElement(s32 index)
{
    SndSequenceElement* element;
    s32 soundHandle;
    s32 ended;
    SndBinResource* sequence;
    u32 lastIndex;
    u32 loopEnd;

    inv_SND_a_stSequencePlayer[index].busy = 1;
    element = NULL;
    soundHandle = NO_SOUND_HANDLE;
    ended = 0;
    if (inv_SND_a_stSequencePlayer[index].playing) {
        do {
            inv_SND_a_stSequencePlayer[index].soundHandle = NO_SOUND_HANDLE;
            sequence = inv_SND_a_stSequencePlayer[index].sequence;
            lastIndex = sequence->seq.elementCount - 1;
            loopEnd = sequence->seq.loopEnd;
            if (inv_SND_a_stSequencePlayer[index].loop != NO_LOOP) {
                if (inv_SND_a_stSequencePlayer[index].index >= loopEnd) {
                    element = sequence->seq.elements + sequence->seq.loopStart;
                    inv_SND_a_stSequencePlayer[index].index = (s16) sequence->seq.loopStart;
                    inv_SND_a_stSequencePlayer[index].loop--;
                } else {
                    element = inv_SND_a_stSequencePlayer[index].element + 1;
                    inv_SND_a_stSequencePlayer[index].index++;
                }
            } else if (inv_SND_a_stSequencePlayer[index].index < lastIndex) {
                element = inv_SND_a_stSequencePlayer[index].element + 1;
                inv_SND_a_stSequencePlayer[index].index++;
            } else if (sequence->seq.looping != 0) {
                element = sequence->seq.elements;
                inv_SND_a_stSequencePlayer[index].index = 0;
                inv_SND_a_stSequencePlayer[index].loop = inv_SND_a_stSequencePlayer[index].sequence->seq.loopCount;
            } else {
                inv_SND_a_stSequencePlayer[index].playing = 0;
                ended = 1;
            }
            if (!ended) {
                soundHandle = SND_fn_lPlaySnd(element->resource.data, inv_SND_a_stSequencePlayer[index].soundParams,
                                              inv_SND_a_stSequencePlayer[index].soundType, 0, 0,
                                              &inv_SND_a_stSequencePlayer[index].soundActive);
                if (soundHandle != NO_SOUND_HANDLE) {
                    inv_SND_a_stSequencePlayer[index].playing = 1;
                    inv_SND_a_stSequencePlayer[index].element = element;
                    inv_SND_a_stSequencePlayer[index].soundHandle = soundHandle;
                    inv_SND_a_stSequencePlayer[index].idle = 0;
                }
            } else if (inv_SND_a_stSequencePlayer[index].callback != NULL) {
                inv_SND_a_stSequencePlayer[index].callback(inv_SND_a_stSequencePlayer[index].callbackParameter);
                inv_SND_a_stSequencePlayer[index].callback = NULL;
                inv_SND_a_stSequencePlayer[index].callbackParameter = 0;
            }
        } while (!ended && soundHandle == NO_SOUND_HANDLE);
    }
    if (ended || soundHandle == NO_SOUND_HANDLE) {
        *inv_SND_a_stSequencePlayer[index].activeOut = 0;
    }
    inv_SND_a_stSequencePlayer[index].busy = 0;
}

void SND_fn_vStopSeq(s32 index)
{
    if (inv_SND_a_stSequencePlayer[index].playing) {
        inv_SND_a_stSequencePlayer[index].playing = 0;
        if (inv_SND_a_stSequencePlayer[index].soundActive != 0) {
            SND_fn_vStopSnd((PlaySndSlot*) inv_SND_a_stSequencePlayer[index].soundHandle);
        }
        if (inv_SND_a_stSequencePlayer[index].callback != NULL) {
            inv_SND_a_stSequencePlayer[index].callback(inv_SND_a_stSequencePlayer[index].callbackParameter);
            inv_SND_a_stSequencePlayer[index].callback = NULL;
            inv_SND_a_stSequencePlayer[index].callbackParameter = 0;
        }
        inv_SND_a_stSequencePlayer[index].playing = 0;
        *inv_SND_a_stSequencePlayer[index].activeOut = 0;
        inv_SND_a_stSequencePlayer[index].index = -1;
        inv_SND_a_stSequencePlayer[index].soundHandle = NO_SOUND_HANDLE;
    }
}

void inv_SND_fn_vPauseSequencePlayer(s32 index)
{
    if (inv_SND_a_stSequencePlayer[index].playing && inv_SND_a_stSequencePlayer[index].soundActive != 0 && !inv_SND_a_stSequencePlayer[index].paused) {
        SND_fn_vPauseSnd((PlaySndSlot*) inv_SND_a_stSequencePlayer[index].soundHandle);
        inv_SND_a_stSequencePlayer[index].paused = 1;
    }
}

void inv_SND_fn_vResumeSequencePlayer(s32 index)
{
    if (inv_SND_a_stSequencePlayer[index].playing && inv_SND_a_stSequencePlayer[index].paused) {
        SND_fn_vResumeSnd((PlaySndSlot*) inv_SND_a_stSequencePlayer[index].soundHandle);
        inv_SND_a_stSequencePlayer[index].paused = 0;
    }
}

void loc_162098(s32 index)
{
    if (inv_SND_a_stSequencePlayer[index].playing) {
        inv_SND_a_stSequencePlayer[index].callback = NULL;
        inv_SND_a_stSequencePlayer[index].callbackParameter = 0;
    }
}

s32 SND_fn_bSetParamSeq(s32 index, _SoundParam* params)
{
    SndSequencePlayer* player = &inv_SND_a_stSequencePlayer[index];

    if (player->playing) {
        refresh_SoundParam_member(player, params);
        test_next_element(index, params, 1);
        return player->playing;
    }
    return 0;
}

s32 SND_fn_bTestIsPlayingSeq(s32 index)
{
    test_next_element(index, NULL, 0);
    return inv_SND_a_stSequencePlayer[index].playing;
}

s32 func_001621B0(void)
{
    return -1;
}

s32 SND_fn_rGetPosSeq(void *a0)
{
    return -1;
}

s32 inv_SND_fn_bSetResourceStaticVolumeSequence(SndBinResource* sequence, u8 value)
{
    sequence->seq.staticVolume = value;
    return 1;
}
