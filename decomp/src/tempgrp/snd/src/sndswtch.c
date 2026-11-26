#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"

#define SND_RESOURCE_SWITCH 8
#define SND_PARAM_SWITCH_FROM_OBJECT 0x20
#define SND_PARAM_SWITCH_VOLUME 0x4
#define SWITCH_NO_DEFAULT -1

typedef struct __attribute__((packed, aligned(4))) SndSwitchDiskOldBits {
    u32 unk00;
    s32 type;
    u8 pad08[0x18];
    s32 childCount;
    u32 inv_ulInfoType;
    s32 defaultKey;
    u64 looping : 1;
} SndSwitchDiskOldBits;

typedef struct __attribute__((packed, aligned(4))) SndSwitchDiskBits {
    u32 unk00;
    s32 type;
    u8 pad08[0x18];
    s32 childCount;
    u32 inv_ulInfoType;
    s32 defaultKey;
    u64 looping : 1;
    u64 unk2C_1 : 1;
    u64 unk2C_2 : 30;
    u8 pad30[0x0C];
} SndSwitchDiskBits;

typedef struct SoundParamSwitch {
    s32 unk00;
    s32 unk04;
    s32 switchType;
    s32 switchObject;
} SoundParamSwitch;

typedef struct SwitchPlayParams {
    _SoundParam params;
    u8 extension[0x30];
    u8 switchVolume;
    u8 pad61[7];
} SwitchPlayParams;

typedef void (*SwitchEndCallback)(s32 param);

typedef struct SwitchInstance {
    s32 sound;
    SndBinResource* selected;
    SndBinResource* resource;
    s32 unk0C;
    SwitchEndCallback callback;
    s32 callbackParam;
    s32 playing;
    u8 volume;
} SwitchInstance;

s32 SND_fn_bIsResourceSwitch(SndBinResource* resource);

SndBinResource* SND_fn_tdGetEffectiveResourceSwitch(SndBinResource* resource,
                                                       _SoundParam* params, u8* volumeOut)
{
    SwitchChildren* children;
    SwitchChild* child;
    SoundParamSwitch* selection;
    s32 found;
    s32 count;
    s32 i;
    s32 key;
    u32 volume = 1;
    s32 depth = 0;
    s32 fallbackKey;

    while (resource != NULL && SND_fn_bIsResourceSwitch(resource)) {
        depth++;
        volume *= resource->swtch.volume;
        if (params->flags & SND_PARAM_SWITCH_FROM_OBJECT) {
            selection = (SoundParamSwitch*) &params->micros[params->microCount];
            key = SND_fn_lGetSwitchIndice(selection->switchType, selection->switchObject, resource);
        } else {
            key = resource->swtch.defaultKey;
        }

        children = &resource->swtch.children;
        child = resource->swtch.children.entries;
        count = children->count;
        found = 0;
        for (i = 0; i < count; i++) {
            if (child->key == key) {
                resource = child->target.resource;
                found = 1;
                break;
            }
            child++;
        }
        if (found) {
            continue;
        }

        fallbackKey = resource->swtch.defaultKey;
        if (fallbackKey != SWITCH_NO_DEFAULT) {
            child = children->entries;
            for (i = 0; i < count; i++) {
                if (child->key == fallbackKey) {
                    resource = child->target.resource;
                    found = 1;
                    break;
                }
                child++;
            }
            resource = found ? resource : NULL;
        } else {
            resource = NULL;
        }
    }

    if (depth >= 2) {
        *volumeOut = volume / depth;
    } else {
        *volumeOut = volume;
    }
    return resource;
}

void func_00162888(void)
{
}

void func_00162890(void)
{
}

s32 SND_fn_lPlaySwitch(SndBinResource* resource, _SoundParam* params, s32 unk0C,
                       SwitchEndCallback callback, s32 callbackParam)
{
    SwitchPlayParams play;
    SwitchInstance* instance;
    SndBinResource* selected;
    s32 volume;
    u32 flags;

    instance = SND_fn_pvMallocSndEx(0, sizeof(SwitchInstance));
    if (instance == NULL) {
        return -1;
    }

    instance->playing = 1;
    selected = SND_fn_tdGetEffectiveResourceSwitch(resource, params, &play.switchVolume);
    instance->resource = resource;
    instance->volume = play.switchVolume;
    instance->callback = callback;
    instance->callbackParam = callbackParam;
    instance->selected = selected;
    instance->unk0C = unk0C;
    if (selected != NULL) {
        memcpy(&play, params, (params->microCount << 3) + 0x38);
        volume = play.params.vol;
        volume *= play.switchVolume;
        flags = play.params.flags;
        flags |= SND_PARAM_SWITCH_VOLUME;
        play.params.flags = flags;
        volume >>= 7;
        play.params.vol = volume;

        instance->sound = SND_fn_lPlaySnd(selected, &play.params, unk0C, 0, 0, &instance->playing);
        if (instance->sound != -1) {
            return (s32) instance;
        }
    }
    SND_fn_vFreeSndEx(0, instance);
    return -1;
}

typedef union SndParamCopy {
    _SoundParam param;
    u8 bytes[0x60];
} SndParamCopy;

s32 SND_fn_bSetParamSwitch(s32 handle, _SoundParam* params)
{
    SwitchInstance* instance = (SwitchInstance*) handle;
    SndParamCopy play;
    SndBinResource* selected;
    u8 volume = 0x7F;
    s32 ok = 0;

    if (instance->resource->swtch.flags.bits.looping)
        selected = SND_fn_tdGetEffectiveResourceSwitch(instance->resource, params, &volume);
    else
        selected = instance->selected;
    memcpy(&play, params, (params->microCount << 3) + 0x38);
    if (selected == instance->selected) {
        play.param.vol = (play.param.vol * instance->volume) >> 7;
        play.param.flags |= SND_PARAM_SWITCH_VOLUME;
        if (instance->playing != 0)
            ok = SND_fn_bSetParamSnd((PlaySndSlot*) instance->sound, &play.param) != 0;
    } else {
        SND_fn_vStopSnd((PlaySndSlot*) instance->sound);
        play.param.vol = (play.param.vol * volume) >> 7;
        play.param.flags |= SND_PARAM_SWITCH_VOLUME;
        instance->sound =
            SND_fn_lPlaySnd(selected, &play.param, instance->unk0C, 0, 0, &instance->playing);
        instance->selected = selected;
        ok = instance->sound != -1;
        instance->volume = volume;
    }
    if (ok != 0)
        return 1;
    if (instance->callback != NULL)
        instance->callback(instance->callbackParam);
    SND_fn_vFreeSndEx(0, instance);
    return 0;
}

s32 SND_fn_bTestIsPlayingSwitch(SwitchInstance* instance)
{
    if (instance->playing != 0 && SND_fn_bTestIsPlayingSnd((PlaySndSlot*) instance->sound)) {
        return 1;
    }
    if (instance->callback != NULL) {
        instance->callback(instance->callbackParam);
    }
    SND_fn_vFreeSndEx(0, instance);
    return 0;
}

void SND_fn_vStopSwitch(SwitchInstance* instance)
{
    if (instance->playing != 0)
        SND_fn_vStopSnd((PlaySndSlot*) instance->sound);
    if (instance->callback != NULL)
        instance->callback(instance->callbackParam);
    SND_fn_vFreeSndEx(0, instance);
}

void SND_fn_vPauseSwitch(SwitchInstance* instance)
{
    if (instance->playing != 0) {
        SND_fn_vPauseSnd((PlaySndSlot*) instance->sound);
    }
}

void SND_fn_vResumeSwitch(SwitchInstance* instance)
{
    if (instance->playing != 0) {
        SND_fn_vResumeSnd((PlaySndSlot*) instance->sound);
    }
}

void func_00162C10(SwitchInstance* instance)
{
    instance->callback = NULL;
}

void SND_fn_vDesInitSwitch(void)
{
}

s32 SND_fn_rGetPosSwitch(SwitchInstance* instance)
{
    if (instance->playing != 0) {
        return SND_fn_rGetPosSnd((PlaySndSlot*) instance->sound);
    }
    return -1;
}

s32 SND_fn_rGetLengthSwitch(SwitchInstance* instance)
{
    s32 length;

    if (instance->playing != 0) {
        length = SND_fn_rGetLengthSnd((PlaySndSlot*) instance->sound);
    } else {
        length = -1;
    }
    return length;
}

void SND_fn_vConvertResDiskToMemSwitch(SndSwitchDisk* source, SndBinResource* destination,
                                          s32 unused)
{
    s32 flags = destination->swtch.flags.word;
    u8 volume;

    destination->swtch.unk00 = source->unk00;
    flags &= ~1;
    destination->swtch.type = source->type;
    destination->swtch.unk08 = source->unk08;
    volume = source->volume;
    destination->swtch.unk10 = 1;
    destination->swtch.volume = volume;
    destination->swtch.childCount = source->childCount;
    destination->swtch.inv_ulInfoType = source->inv_ulInfoType;
    destination->swtch.defaultKey = source->defaultKey;
    destination->swtch.flags.word = flags | (source->flags & 1);
}

s32 SND_fn_bLoadResBinarySwitch(SndSwitchDisk* source, SndBinResource* destination, u32 base)
{
    SwitchChild* child;
    s32 count;
    s32 i;

    base += source->childOffset;
    destination->swtch.children.entries = (SwitchChild*) base;
    child = (SwitchChild*) base;
    count = source->childCount;
    destination->swtch.children.count = count;
    for (i = 0; i < count; i++) {
        child->target.resource = SND_fn_pGetBinRes(child->target.id);
        child++;
        count = destination->swtch.children.count;
    }
    SND_fn_vConvertResDiskToMemSwitch(source, destination, 0);
    return 1;
}

s32 SND_fn_bLoadResBinarySwitchOld(void* source, SndBinResource* destination, u32 base)
{
    SndSwitchDiskOldBits* old = source;
    SndSwitchDiskBits* current = source;
    SndSwitchDiskBits converted;

    converted = *current;
    converted.defaultKey = old->defaultKey;
    converted.type = SND_RESOURCE_SWITCH;
    converted.childCount = old->childCount;
    converted.inv_ulInfoType = old->inv_ulInfoType;
    converted.looping = old->looping;
    converted.unk2C_1 = 0;
    return SND_fn_bLoadResBinarySwitch((SndSwitchDisk*) &converted, destination, base);
}

SndBinResource* SND_fn_tduGetLowestResourceSwitch(SndBinResource* resource, _SoundParam* params, s32 mode)
{
    u8 volume;
    SndBinResource* selected = SND_fn_tdGetEffectiveResourceSwitch(resource, params, &volume);

    return SND_fn_tduGetLowestResourceSnd(selected, params, mode);
}

s32 SND_fn_bIsResourceSwitch(SndBinResource* resource)
{
    s32 isSwitch = 0;

    if (resource != NULL) {
        isSwitch = resource->swtch.type == SND_RESOURCE_SWITCH;
    }
    return isSwitch;
}

s32 SND_fn_bSetResourceStaticVolumeSwitch(SndResourceHeaderView* resource, s32 volume)
{
    resource->staticVolume = volume;
    return 1;
}
