#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"

#define RANDOM_NO_CHOICE -1
#define RANDOM_UNSET -2
#define RANDOM_RANGE 0x10000
#define SND_PARAM_SWITCH_VOLUME 0x4
#define SND_INVALID_SOUND -1

typedef struct RandomInstance {
    s32 sound;
    s32 playing;
    SndBinResource* resource;
} RandomInstance;

typedef struct RandomPlayParams {
    _SoundParam params;
    u8 extension[0x30];
} RandomPlayParams;

SndBinResource* SND_fn_tdGetEffectiveResourceRandom(SndBinResource* random);
SndBinResource* SND_fn_tdGetResourceChoosenRandom(SndBinResource* random);
void SND_fn_vConvertResDiskToMemRandom(SndRandomDisk* source, SndBinResource* destination,
                                          s32 unused);

SndBinResource* SND_fn_tdGetEffectiveResourceRandom(SndBinResource* random)
{
    SndBinResource* resource;
    s32 excludedPrevious = RANDOM_UNSET;
    s32 excludedCurrent = RANDOM_UNSET;
    s32 chosenIndex = RANDOM_NO_CHOICE;
    RandomDesc* desc;
    RandomChoice* choice;
    s32 index;
    s32 value;
    s32 count;
    s32 i;
    s32 found;

    resource = random;
    desc = &random->random.desc;

    index = resource->random.desc.previousIndex;
    if (index >= 0) {
        choice = &desc->choices[index];
        if (choice->repeatable == 0) {
            excludedPrevious = index;
        }
    }

    index = resource->random.desc.chosenIndex;
    if (index >= 0) {
        choice = &desc->choices[index];
        if (choice->repeatable == 0) {
            excludedCurrent = index;
        }
    } else if (index == RANDOM_NO_CHOICE) {
        if (resource->random.desc.silenceRepeat == 0) {
            excludedCurrent = RANDOM_NO_CHOICE;
        }
    }

    value = SND_fn_rGetRandomSnd(RANDOM_RANGE);
    found = 0;
    if (value >= desc->silenceWeight || excludedCurrent == RANDOM_NO_CHOICE) {
        choice = desc->choices;
        value -= desc->silenceWeight;
        count = desc->choiceCount;
        for (i = 0; i < count; i++, choice++) {
            if (choice->weight >= value && excludedPrevious != i) {
                resource = choice->target.resource;
                chosenIndex = i;
                found = 1;
                break;
            }
            value -= choice->weight;
        }
        if (!found) {
            if (excludedCurrent != 0) {
                chosenIndex = 0;
                choice = desc->choices;
                resource = choice->target.resource;
            } else {
                chosenIndex = RANDOM_NO_CHOICE;
                resource = NULL;
            }
            found = 1;
        }
    }

    if (!found) {
        chosenIndex = RANDOM_NO_CHOICE;
        resource = NULL;
    }

    random->random.desc.chosenIndex = chosenIndex;
    if (chosenIndex != RANDOM_NO_CHOICE) {
        random->random.desc.previousIndex = chosenIndex;
    }
    return resource;
}

SndBinResource* SND_fn_tdGetResourceChoosenRandom(SndBinResource* random)
{
    s32 index = random->random.desc.chosenIndex;
    RandomChoice* choice;

    if (index == RANDOM_NO_CHOICE || index >= random->random.desc.choiceCount) {
        return SND_RES_FANTOME[0];
    }
    choice = &random->random.desc.choices[index];
    return choice->target.resource;
}

s32 SND_fn_lPlayRandom(SndBinResource* resource, _SoundParam* params, s32 argument2,
                       SndEndCallback_inv argument3, s32 argument4)
{
    RandomPlayParams play;
    RandomInstance* instance;
    SndBinResource* selected;
    s32 volume;
    s32 flags;
    s32 result;

    instance = SND_fn_pvMallocSndEx(0, sizeof(RandomInstance));
    result = -1;
    if (instance == NULL) {
        return result;
    }
    result = 1;
    instance->resource = resource;
    instance->playing = result;
    selected = SND_fn_tdGetEffectiveResourceRandom(resource);
    if (selected != NULL) {
        memcpy(&play, params, (params->microCount << 3) + 0x38);
        volume = play.params.vol;
        flags = play.params.flags;
        volume = volume * (u8) instance->resource->random.volume;
        flags |= SND_PARAM_SWITCH_VOLUME;
        play.params.flags = flags;
        volume >>= 7;
        play.params.vol = volume;
        instance->sound = SND_fn_lPlaySnd(selected, &play.params, argument2, argument3, argument4,
                                          &instance->playing);
        if (instance->sound != SND_INVALID_SOUND) {
            return (s32) instance;
        }
    }
    SND_fn_vFreeSndEx(0, instance);
    return -1;
}

s32 SND_fn_bSetParamRandom(RandomInstance* instance, _SoundParam* params)
{
    volatile RandomPlayParams play;
    volatile RandomInstance* current = instance;
    SndBinResource* resource;
    s32 volume;
    s32 flags;

    memcpy((void*) &play, params, (params->microCount << 3) + 0x38);
    resource = current->resource;
    volume = play.params.vol;
    volume *= (u8) resource->random.volume;
    flags = play.params.flags;
    flags |= SND_PARAM_SWITCH_VOLUME;
    play.params.flags = flags;
    volume >>= 7;
    play.params.vol = volume;
    if (current->playing != 0 && SND_fn_bSetParamSnd((PlaySndSlot*) current->sound, (_SoundParam*) &play.params)) {
        return 1;
    }
    SND_fn_vFreeSndEx(0, instance);
    return 0;
}

s32 SND_fn_bTestIsPlayingRandom(RandomInstance* instance)
{
    if (instance->playing != 0 && SND_fn_bTestIsPlayingSnd((PlaySndSlot*) instance->sound)) {
        return 1;
    }
    SND_fn_vFreeSndEx(0, instance);
    return 0;
}

void SND_fn_vStopRandom(RandomInstance* instance)
{
    if (instance->playing != 0) {
        SND_fn_vStopSnd((PlaySndSlot*) instance->sound);
    }
    SND_fn_vFreeSndEx(0, instance);
}

void SND_fn_vPauseRandom(RandomInstance* instance)
{
    if (instance->playing != 0) {
        SND_fn_vPauseSnd((PlaySndSlot*) instance->sound);
    }
}

void SND_fn_vResumeRandom(RandomInstance* instance)
{
    if (instance->playing != 0) {
        SND_fn_vResumeSnd((PlaySndSlot*) instance->sound);
    }
}

void SND_fn_vRemoveCallbackRandom(RandomInstance* instance)
{
    if (instance->playing != 0) {
        SND_fn_vRemoveCallbackSnd((PlaySndSlot*) instance->sound);
    }
}

void func_00161398(void)
{
}

s32 SND_fn_rGetPosRandom(RandomInstance* instance)
{
    s32 position;

    if (instance->playing != 0) {
        position = SND_fn_rGetPosSnd((PlaySndSlot*) instance->sound);
    } else {
        position = -1;
    }
    return position;
}

s32 SND_fn_rGetLengthRandom(RandomInstance* instance)
{
    s32 length;

    if (instance->playing != 0) {
        length = SND_fn_rGetLengthSnd((PlaySndSlot*) instance->sound);
    } else {
        length = -1;
    }
    return length;
}

void SND_fn_vConvertResDiskToMemRandom(SndRandomDisk* source, SndBinResource* destination,
                                          s32 unused)
{
    destination->random.unk00 = source->unk00;
    destination->random.type = source->type;
    destination->random.unk08 = source->unk08;
    destination->random.volume = source->volume;
    destination->random.unk10 = 1;
    destination->random.desc.unk00 = source->choiceCount;
    destination->random.desc.silenceWeight = source->silenceWeight;
    destination->random.desc.silenceRepeat = source->silenceRepeat;
    destination->random.desc.chosenIndex = RANDOM_UNSET;
    destination->random.desc.previousIndex = RANDOM_UNSET;
}

s32 SND_fn_bLoadResBinaryRandom(SndRandomDisk* source, SndBinResource* destination, u32 base)
{
    RandomChoice* choice;
    s32 count;
    s32 i;

    base += source->choiceOffset;
    destination->random.desc.choices = (RandomChoice*) base;
    choice = (RandomChoice*) base;
    count = source->choiceCount;
    destination->random.desc.choiceCount = count;
    for (i = 0; i < count; i++) {
        choice->target.resource = SND_fn_pGetBinRes(choice->target.id);
        choice++;
        count = destination->random.desc.choiceCount;
    }
    SND_fn_vConvertResDiskToMemRandom(source, destination, 0);
    return 1;
}

SndBinResource* SND_fn_tduGetLowestResourceRandom(SndBinResource* resource, _SoundParam* params, s32 mode)
{
    SndBinResource* selected;

    if (mode != 0) {
        selected = SND_fn_tdGetEffectiveResourceRandom(resource);
    } else {
        selected = SND_fn_tdGetResourceChoosenRandom(resource);
    }
    return SND_fn_tduGetLowestResourceSnd(selected, params, mode);
}

s32 SND_fn_bSetResourceStaticVolumeRandom(SndBinResource* resource, s32 volume)
{
    resource->random.volume = volume;
    return 1;
}
