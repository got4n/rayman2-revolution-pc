#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"

#include "include_asm.h"

extern PlaySndSlot inv_SND_playSlots[64];

void desassocie_snd(void* slot);
extern s32 g_lCurrentGlobalPauseInstance;
extern s32 inv_SND_lSxdInitResult;

s32 associe_snd(PlaySnd*, s32, SndBinResource*, s32, SndOwnerState*, s32);
s32 pre_associe_snd(void);
void inv_SND_fn_vInitPlaySlots(void);

void SND_fn_vCopySoundParam(void* destination, _SoundParam* source)
{
    s32 size = source->microCount << 3;

    memcpy(destination, source, size + 0x38);
}

void func_0015F820(void)
{
}

s32 pre_associe_snd(void)
{
    PlaySndSlot* slot;
    s32 busy;
    s32 index;

    index = 0;
    while (index < 64) {
        busy = inv_SND_playSlots[index].state.bits.bit0;
        if (busy != 0) {
            break;
        }
        index = index + 1;
    }
    if (index == 64) {
        SND_fn_vDisplayErrorEx(0x14, "", 3);
        return -1;
    }
    slot = &inv_SND_playSlots[index];
    slot->state.bits.bit0 = 0;
    slot->state.bits.bit1 = 1;
    inv_SND_playSlots[index].inUse = 1;
    slot->soundHandle = -1;
    return (s32) slot;
}

void annule_pre_associe_snd(PlaySndSlot* slot)
{
    s32 flags = slot->state.flags;

    slot->owner = 0;
    slot->state.flags = flags | 1;
}

void desassocie_snd(void* slot)
{
    PlaySnd* entry = slot;
    s32* pointer = (s32*) entry->owner;

    if (pointer != 0) {
        *pointer = 0;
        entry->owner = 0;
    }
    {
        s32 flags = entry->state.flags;
        entry->state.flags = flags | 1;
    }
}

void invalidate_ptrvalid_snd(PlaySndSlot* slot)
{
    slot->owner = 0;
}

s32 associe_snd(PlaySnd* slot, s32 soundHandle, SndBinResource* resource, s32 a3_unused,
                SndOwnerState* owner, s32 bit2Value)
{
    s32 remaining;
    PlaySnd* entry = inv_SND_playSlots;

    for (remaining = 63; remaining >= 0; remaining--, entry++) {
        s32 bit0 = entry->state.bits.bit0;
        if (bit0) {
            continue;
        }

        {
            s32 unk00 = entry->soundHandle;
            if (unk00 == -1 && entry->state.bits.bit1 >= 0) {
            } else if (soundHandle == -1 || unk00 != soundHandle ||
                       resource->head.type != entry->resource->head.type) {
                if (entry->inUse != 0) {
                    continue;
                }
            }
        }
        desassocie_snd(entry);
    }

    if (soundHandle != -1) {
        slot->soundHandle = soundHandle;
        slot->resource = resource;
        slot->state.bits.bit0 = 0;
        slot->state.bits.bit1 = 0;
        slot->state.bits.bit2 = bit2Value;
        slot->inUse = 1;
        slot->owner = owner;
        slot->pauseInstance = 0;

        if (owner != NULL) {
            owner->active = 1;
        }

        return (s32) slot;
    }

    slot->owner = NULL;
    slot->state.bits.bit0 = 1;
    return -1;
}

s32 SND_fn_bLoadResBinarySnd(SndBinResourceDisk_inv* src, SndBinResource* dst, u32 base)
{
    s32 kind;

    dst->head.type = src->header.kind;
    kind = src->header.kind;
    switch (kind) {
    case 1:
        return SND_fn_bLoadResBinarySxd(src, dst, base);
    case 4:
        return SND_fn_bLoadResBinarySeq(&src->seq, dst, base);
    case 5:
        return SND_fn_bLoadResBinarySwitchOld(src, dst, base);
    case 8:
        return SND_fn_bLoadResBinarySwitch(&src->swtch, dst, base);
    case 11:
        return SND_fn_bLoadResBinaryTheme(&src->theme, dst, base);
    case 10:
        return SND_fn_bLoadResBinaryRandom(&src->random, dst, base);
    default:
        SND_fn_vDisplayError(0x15, "");
        return 0;
    }
}

void SND_fn_vStopBeforeUnLoadResSnd(SndBinResource* resource)
{
    s32 i;

    for (i = 0; i < 64; i++) {
        if (!inv_SND_playSlots[i].state.bits.bit0 && inv_SND_playSlots[i].resource == resource &&
            inv_SND_playSlots[i].inUse != 0) {
            if (inv_SND_playSlots[i].state.bits.bit2)
                SND_fn_vStopTransitionSnd(&inv_SND_playSlots[i]);
            else
                SND_fn_vStopSnd(&inv_SND_playSlots[i]);
        }
    }
}

void SND_fn_vUnLoadResSnd(SndBinResource* resource)
{
    switch (resource->head.type) {
    case 1:
        SND_fn_vUnLoadResSxd(resource);
        break;
    case 11:
        SND_fn_vUnLoadResTheme(resource);
        break;
    }
    resource->head.type = 0;
}

void inv_SND_fn_vInitPlaySlots(void)
{
    PlaySndSlot* cursor = inv_SND_playSlots;
    PlaySndSlot* end = cursor + 64;
    do {
        cursor->state.flags |= 1;
        cursor++;
    } while ((s32) cursor < (s32) end);
}

s32 SND_fn_iInitSnd(void* context)
{
    u8 rollOff[0x10];
    inv_SND_fn_vInitPlaySlots();
    inv_SND_lSxdInitResult = SND_fn_iInitSxd(context);
    SND_fn_vGetDefaultRollOff((SndRollOff*) rollOff);
    SND_fn_vSetDefaultRollOffSxd((SndRollOff*) rollOff);
    SND_fn_iInitSeq();
    SND_fn_vInitTheme();
    return 3;
}

void SND_fn_vSetDefaultRollOffSnd(SndRollOff* rollOff)
{
    SND_fn_vSetDefaultRollOff(rollOff);
    SND_fn_vSetDefaultRollOffSxd(rollOff);
}

void SND_fn_vSetDopplerFactorSnd(s32 factor)
{
    SND_fn_vSetDopplerFactor(factor);
    loc_163F70(factor);
}

s32 SND_fn_lPlaySnd(SndBinResource* resource, _SoundParam* params, s32 a2, SndEndCallback_inv a3, s32 t0,
                    s32* activeOut)
{
    s32 slot;
    s32 result;

    if (resource == (SndBinResource*) -1) {
        printf("*********ANTIBUG SON!!*********\n");
        return -1;
    }

    slot = pre_associe_snd();
    if (slot == -1) {
        return -1;
    }

    switch (resource->head.type) {
    case 1:
        result = SND_fn_lPlaySxd(resource, params, a2, a3, t0);
        break;
    case 4:
        result = SND_fn_lPlaySeq(resource, params, a2, a3, t0, (s32*) (slot + 0xC));
        break;
    case 11:
        result = SND_fn_lPlayTheme(-1, resource, params, a2, a3, t0, (s32*) (slot + 0xC));
        break;
    case 10:
        result = SND_fn_lPlayRandom(resource, params, a2, a3, t0);
        break;
    case 8:
        result = SND_fn_lPlaySwitch(resource, params, a2, a3, t0);
        break;
    default:
        SND_fn_vDisplayError(0x15, "");
        result = -1;
        break;
    }

    return associe_snd((PlaySnd*) slot, result, resource, a2, (SndOwnerState*) activeOut, 0);
}

s32 SND_fn_bSetParamSnd(PlaySndSlot* instance, void* params)
{
    s32 stride;
    u8* tableEntry;
    s32* slot;
    s32* resource;
    s32 offset;
    s32 result;
    s32 (*setParams)(s32, void*);

    stride = 0x18;
    tableEntry = (u8*) a_fnFunc;
    slot = (s32*) instance;
    resource = (s32*) slot[1];
    offset = resource[1];
    offset = offset * stride;
    tableEntry = (u8*) ((s32) tableEntry - (s32) (-(s32) offset));
    setParams = *(s32(**)(s32, void*))(tableEntry + 8);
    result = setParams(slot[0], params);
    if (result == 0) {
        desassocie_snd(slot);
    }
    return result;
}

s32 SND_fn_bTestIsPlayingSnd(PlaySndSlot* instance)
{
    s32 stride;
    u8* tableEntry;
    s32* slot;
    s32* resource;
    s32 offset;
    s32 result;
    s32 (*isPlaying)(s32);

    stride = 0x18;
    tableEntry = (u8*) a_fnFunc;
    slot = (s32*) instance;
    resource = (s32*) slot[1];
    offset = resource[1];
    offset = offset * stride;
    tableEntry = (u8*) ((s32) tableEntry - (s32) (-(s32) offset));
    isPlaying = *(s32(**)(s32))(tableEntry + 0x14);
    result = isPlaying(slot[0]);
    if (result == 0) {
        desassocie_snd(slot);
    }
    return result;
}

void SND_fn_vStopSnd(PlaySndSlot* hSnd)
{
    s32* p_lHandle = (s32*) hSnd;

    s32* p_lSound;
    s32* p_lType;
    s32 lFuncTable;
    s32 lEntrySize;
    s32 lOffset;
    s32 first;
    void (*fn)(s32);

    p_lSound = p_lHandle;
    first = -(p_lSound[2] & 1);
    if (first == 0) {
        lEntrySize = 0x18;
        lFuncTable = (s32) (*(u8(*)[]) & a_fnFunc);
        p_lType = (s32*) p_lSound[1];
        lOffset = p_lType[1];
        lOffset = lOffset * lEntrySize;
        lFuncTable = lFuncTable - (0 - lOffset);
        fn = *(void (**)(s32))(lFuncTable + 0xC);
        fn(p_lSound[0]);
    }

    desassocie_snd(p_lSound);
}

void inv_SND_fn_vStopAllOwnedSnd(void)
{
    s32 remaining;
    PlaySndSlot* slot;

    remaining = 0x3F;
    slot = inv_SND_playSlots;
    do {
        s32 first;

        remaining = remaining - 1;
        if (slot->inUse != 0) {
            first = -(slot->state.flags & 1);
            if (first == 0) {
                if (slot->owner != 0) {
                    SND_fn_vStopSnd(slot);
                }
            }
        }
        slot = slot + 1;
    } while (remaining >= 0);
}

void inv_SND_fn_vFreeFinishedSlots(void)
{
    s32 remaining;
    PlaySndSlot* slot;

    remaining = 0x3F;
    slot = inv_SND_playSlots;
    do {
        s32 bit0;
        s32 bit2;

        remaining = remaining - 1;
        if (slot->inUse != 0) {
            bit0 = slot->state.bits.bit0;
            if (!bit0) {
                if (slot->owner == 0) {
                    bit2 = slot->state.bits.bit2;
                    if (!bit2) {
                        if (SND_fn_bTestIsPlayingSnd(slot) == 0) {
                            slot->inUse = 0;
                            slot->state.bits.bit0 = 1;
                        }
                    }
                }
            }
        }
        slot = slot + 1;
    } while (remaining >= 0);
}

void SND_fn_vPauseSnd(PlaySndSlot* instance)
{
    s32 stride;
    s32* sound;
    s32* resource;
    s32 entryAddress;
    u8* table;
    void (*pause)(s32);

    stride = 0x18;
    sound = (s32*) instance;
    table = (u8*) a_fnFunc;
    resource = (s32*) sound[1];
    entryAddress = resource[1];
    entryAddress = entryAddress * stride;
    entryAddress = (s32) ((u8*) ((s32) entryAddress - (s32) (-(s32) table)));
    pause = *(void (**)(s32))(entryAddress + 0);
    pause(sound[0]);
}

void SND_fn_vResumeSnd(PlaySndSlot* slot)
{
    s32* instance = (s32*) slot;
    s32 stride;
    s32* resource;
    s32 entryAddress;
    s32 offset;
    void (*resume)(s32);
    stride = 0x18;
    entryAddress = (s32) a_fnFunc;
    resource = (s32*) instance[1];
    offset = resource[1];
    offset = offset * stride;
    entryAddress = entryAddress - (0 - offset);
    resume = ((void (**)(s32)) entryAddress)[1];
    resume(instance[0]);
}

void SND_fn_vRemoveCallbackSnd(PlaySndSlot* instance)
{
    switch (instance->resource->head.type) {
    case 1:
        SND_fn_vRemoveCallbackSxd((struct SndCallbackBuffer*) instance->soundHandle);
        break;
    case 4:
        loc_162098(instance->soundHandle);
        break;
    case 11:
        func_00159E28(instance->soundHandle);
        break;
    case 8:
        func_00162C10((struct SwitchInstance*) instance->soundHandle);
        break;
    case 10:
        SND_fn_vRemoveCallbackRandom((struct RandomInstance*) instance->soundHandle);
        break;
    default:
        SND_fn_vDisplayError(0x15, "");
        break;
    }
}

void SND_fn_vPauseAllSnd(void)
{
    s32 remaining;
    s32 stamp;
    s32 first;
    PlaySndSlot* slot;

    stamp = g_lCurrentGlobalPauseInstance;
    slot = (*(PlaySndSlot(*)[64]) & inv_SND_playSlots);
    remaining = 0x3F;

    do {
        if (slot->inUse != 0) {
            first = -(slot->state.flags & 1);
            if (first == 0) {
                if (slot->pauseInstance == 0) {
                    if (slot->owner != 0) {
                        slot->pauseInstance = stamp;
                        if (slot->state.flags << 29 < 0) {
                            SND_fn_vPauseTransitionSnd(slot);
                        } else {
                            SND_fn_vPauseSnd(slot);
                        }
                        stamp = g_lCurrentGlobalPauseInstance;
                    }
                }
            }
        }
        remaining = remaining - 1;
        slot = slot + 1;
    } while (remaining >= 0);

    stamp = stamp + 1;
    g_lCurrentGlobalPauseInstance = stamp;
}

void SND_fn_vResumeAllSnd(void)
{
    s32 remaining;
    s32 stamp;
    s32 flags;
    s32 first;
    s32 newerStamp;
    PlaySnd* slot;

    remaining = 0x3F;
    slot = (*(PlaySnd(*)[64]) & inv_SND_playSlots);
    g_lCurrentGlobalPauseInstance = g_lCurrentGlobalPauseInstance - 1;
    do {
        stamp = g_lCurrentGlobalPauseInstance;
        remaining = remaining - 1;
        if (slot->inUse != 0) {
            flags = slot->state.flags;
            first = -(flags & 1);
            if (first == 0) {
                newerStamp = stamp < slot->pauseInstance;
                if (newerStamp != 0) {
                    goto next;
                }
                if (slot->owner != 0) {
                    slot->pauseInstance = 0;
                    if (flags << 29 < 0) {
                        SND_fn_vResumeTransitionSnd(slot);
                        stamp = g_lCurrentGlobalPauseInstance;
                        goto next;
                    }
                    SND_fn_vResumeSnd(slot);
                    stamp = g_lCurrentGlobalPauseInstance;
                }
            }
        }
    next:
        slot = slot + 1;
    } while (remaining >= 0);
}

void SND_fn_vSetSoundVolumeSnd(s32 a0)
{
    SND_fn_vSetSoundVolumeSxd(a0 & 0xFF);
}

s32 SND_fn_ucGetSoundVolumeSnd(void)
{
    return SND_fn_ucGetSoundVolumeSxd();
}

void j_nullsub_103(void)
{
    inv_SND_fn_vNoOpSxd();
}

s32 SND_fn_bIsMusicPlayingSnd(void)
{
    return func_00163E70();
}

s32 SND_fn_bGetStereoSnd(void)
{
    return SND_fn_bTestIsPlayingTheme();
}

void SND_fn_vSetReverseStereoSnd(void)
{
    SND_fn_vSetReverseStereoSxd();
}

s32 SND_fn_bGetReverseStereoSnd(void)
{
    return SND_fn_bGetReverseStereoSxd();
}

s32 inv_SND_fn_bReturnTestInitSxd(void)
{
    return SND_fn_bTestInitSxd();
}

void SND_fn_vDesInitSnd(void)
{
    inv_SND_fn_vNoOpTheme();
    inv_SND_fn_vDesInitSequencePlayers();
    if (inv_SND_lSxdInitResult == 3) {
        SND_fn_vDesInitSxd();
    }
}

s32 func_00160420(void)
{
    return 3;
}

s32 func_00160428(void)
{
    return 1;
}

void func_00160430(void)
{
}

void func_00160438(void)
{
}

void j_j_j_nullsub_22(s32 effectId)
{
    j_j_nullsub_22();
}

s32 SND_fn_lCreateMicroSnd(SxdMicroParams_inv* params)
{
    return SND_fn_lCreateMicroSxd(params);
}

#ifdef PC_PORT
void SND_fn_vDestroyMicroSxd(s32* node);

void dbgSND_fn_vKillMicro(s32* node)
{
    SND_fn_vDestroyMicroSxd(node);
}
#else
void dbgSND_fn_vKillMicro(s32 lMicro)
{
    SND_fn_vDestroyMicroSxd(lMicro);
}
#endif

#ifdef PC_PORT
void SND_fn_vSetMicroParamSxd(u8* node, u8* params);

void SND_fn_vSetMicroParamSnd(u8* node, u8* params)
{
    SND_fn_vSetMicroParamSxd(node, params);
}
#else
void SND_fn_vSetMicroParamSnd(s32 lMicro, SxdMicroParams_inv* params)
{
    SND_fn_vSetMicroParamSxd(lMicro, params);
}
#endif

s32 SND_fn_lPlayTransitionSnd(SndBinResource* first, SndBinResource* next, SndTransitionCallback callback, struct SndThemePlayer* player,
                              _SoundParam* params)
{
    s32 slot;
    s32 result;

    slot = pre_associe_snd();
    if (slot == -1) {
        return -1;
    }
    if (first->head.type == 1) {
        result = SND_fn_lPlayTransitionExSxd(first, next, callback, player, params);
    } else {
        SND_fn_vDisplayError(0x15, "");
        result = -1;
    }
    return associe_snd((PlaySnd*) slot, result, first, 0, 0, 1);
}

int SND_fn_bSetParamTransitionSnd(int s1, void* param)
{
    int bResult;

    if (s1 == -1) {
        return 0;
    }
    if (((PlaySnd*) s1)->resource->head.type == 1) {
        if (((PlaySnd*) s1)->soundHandle != -1) {
            bResult = SND_fn_bSetParamTransitionSxd((s32*) ((PlaySnd*) s1)->soundHandle, param);
        } else {
            bResult = 0;
        }
    } else {
        SND_fn_vDisplayError(0x15, "");
        bResult = 0;
    }
    if (bResult == 0) {
        desassocie_snd((void*) s1);
    }
    return bResult;
}

s32 SND_fn_bSetNextTransitionSnd(s32 instance, SndBinResource* next)
{
    PlaySnd* slot = (PlaySnd*) instance;
    s32 result;

    if (slot->resource->head.type == 1) {
        result = SND_fn_bSetNextTransitionSxd((s32*) slot->soundHandle, next);
    } else {
        SND_fn_vDisplayError(0x15, "");
        result = 0;
    }
    slot->resource = next;
    return result;
}

s32 SND_fn_bDoTransitionWithFadeSnd(s32 instance, SndBinResource* nextResource, s32 fade)
{
    s32 soundHandle;
    s32 result;

    if (((PlaySnd*) instance)->resource->head.type == 1) {
        soundHandle = ((PlaySnd*) instance)->soundHandle;
        if (soundHandle == -1) {
            result = 0;
        } else {
            result = SND_fn_bDoTransitionWithFadeSxd((s32*) soundHandle, nextResource);
        }
    } else {
        SND_fn_vDisplayError(0x15, "");
        result = 0;
    }
    ((PlaySnd*) instance)->resource = nextResource;
    return result;
}

s32 SND_fn_bDoTransitionWithFadeSnd2(PlaySndSlot* instance, SndBinResource* next, s32 a2)
{
    s32* slot;
    s32 type;

    slot = (s32*) instance;
    type = 1;
    if (((s32*) slot[1])[1] == type) {
        return SND_fn_bDoTransitionWithFadeSxd2((s32*) slot[0], next, a2);
    }
    SND_fn_vDisplayError(0x15, "");
    return 0;
}

void SND_fn_vStopTransitionSnd(PlaySndSlot* instance)
{
    s32* slot;
    s32* resource;
    s32 type;

    slot = (s32*) instance;
    {
        s32 first;

        first = -(slot[2] & 1);
        if (first == 0) {
            resource = (s32*) slot[1];
            type = 1;
            if (resource[1] == type) {
                SND_fn_vStopTransitionSxd((s32*) slot[0]);
            } else {
                SND_fn_vDisplayError(0x15, "");
            }
        }
    }
    desassocie_snd(slot);
}

void SND_fn_vResumeTransitionSnd(PlaySndSlot* slot)
{
    s32* instance = (s32*) slot;
    s32* resource = (s32*) instance[1];

    if (resource[1] == 1) {
        SND_fn_vResumeTransitionSxd((struct SndSxdBuffer*) instance[0]);
    } else {
        SND_fn_vDisplayError(0x15, "");
    }
}

void SND_fn_vPauseTransitionSnd(PlaySndSlot* slot)
{
    SndBinResource* resource = slot->resource;

    if (resource->head.type == 1) {
        SND_fn_vPauseTransitionSxd((void*) slot->soundHandle);
        return;
    }
    SND_fn_vDisplayError(0x15, "");
}

void SND_fn_vReleaseDriverSnd(void)
{
    SND_fn_vReleaseDriverSxd();
}

void SND_fn_vRestoreDriverSnd(void)
{
    SND_fn_vRestoreDriverSxd();
}

void SND_fn_vForceReleaseDriverSnd(void)
{
    SND_fn_vForceReleaseDriverSxd();
}

void SND_fn_vForceRestoreDriverSnd(void)
{
    SND_fn_vForceRestoreDriverSxd();
}

s32 SND_fn_rGetPosSnd(PlaySndSlot* slot)
{
    s32* instance = (s32*) slot;
    s32 stride;
    s32* resource;
    s32 entryAddress;
    s32 offset;
    s32 (*getPosition)(s32);
    s32 position;

    if (instance[3] != 0) {
        stride = 0x18;
        resource = (s32*) instance[1];
        entryAddress = (s32) a_fnFunc;
        offset = resource[1];
        offset = offset * stride;
        entryAddress = entryAddress - (0 - offset);
        getPosition = *(s32(**)(s32))(entryAddress + 0x10);
        position = getPosition(instance[0]);
        return position;
    }
    position = -2;
end:
    return position;
}

s32 SND_fn_rGetLengthSnd(PlaySndSlot* snd)
{
    s32 kind;

    if (snd->inUse == 0) {
        return -1;
    }
    kind = snd->resource->head.type;
    switch (kind) {
    case 1:
        return SND_fn_rGetLengthSxd((void*) snd->soundHandle);
    case 4:
        return SND_fn_rGetPosSeq((void*) snd->soundHandle);
    case 11:
        return SND_fn_rGetLengthTheme((void*) snd->soundHandle);
    case 10:
        return SND_fn_rGetLengthRandom((void*) snd->soundHandle);
    case 8:
        return SND_fn_rGetLengthSwitch((void*) snd->soundHandle);
    default:
        SND_fn_vDisplayError(0x15, "");
        return -1;
    }
}

#ifdef PC_PORT
void SND_fn_vGetTargetLabelSxd(char* name, s32 size);

void SND_fn_vGetTargetLabelSnd(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSxd(name, size);
}
#else
void SND_fn_vGetTargetLabelSnd(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSxd(name, size);
}
#endif

void SND_fn_vSetupTargetSnd(void)
{
    SND_fn_vSetupTargetSxd();
}

s32 SND_fn_bCanSetupTargetSnd(void)
{
    return SND_fn_bCanSetupTargetSxd();
}

s32 SND_fn_lCreateNewBufferSnd(s32 voice, s32 extra0, s32 extra1, s32 extra2, _SoundParam* param3D,
                                s32 extra3, s32 user)
{
    return SND_fn_lCreateNewBufferSxd(voice, extra0 & 65535, extra1 & 65535, extra2, param3D, extra3, user);
}

void SND_fn_vDeleteBufferSnd(s32 lBuffer)
{
    SND_fn_vDeleteBufferSxd(lBuffer);
}

void SND_fn_vPauseBufferSnd(s32 lBuffer)
{
    SND_fn_vPauseBufferSxd(lBuffer);
}

void SND_fn_vResumeBufferSnd(s32 lBuffer)
{
    SND_fn_vResumeBufferSxd(lBuffer);
}

s32 SND_fn_rGetPosBufferSnd(s32 lBuffer)
{
    return SND_fn_rGetPosBufferSxd(lBuffer);
}

s32 SND_fn_lCreateNewBufferExSnd(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                 _SoundParam* param3D, void* user)
{
    return SND_fn_lCreateNewBufferExSxd(description, format, param3D, user);
}

s32 SND_fn_lGetPosBufferExSnd(s32 lBuffer)
{
    return SND_fn_lGetPosBufferExSxd(lBuffer);
}

s32 SND_fn_lPushBufferExSnd(s32 lBuffer, TransitionPushArg* data)
{
    return SND_fn_lPushBufferExSxd(lBuffer, data);
}

void SND_fn_vSetParamBufferSnd(s32 lBuffer, _SoundParam* param3D)
{
    SND_fn_vSetParamBufferSxd(lBuffer, param3D);
}

void func_001609D8(void)
{
    s32 remaining;
    PlaySndSlot* slot;

    remaining = 0x3F;
    slot = inv_SND_playSlots;
    do {
        s32 bit0;
        s32 bit2;

        remaining = remaining - 1;
        if (slot->inUse != 0) {
            bit0 = slot->state.bits.bit0;
            if (!bit0) {
                if (slot->owner == 0) {
                    bit2 = slot->state.bits.bit2;
                    if (!bit2) {
                        if (SND_fn_bTestIsPlayingSnd(slot) == 0) {
                            slot->inUse = 0;
                            slot->state.bits.bit0 = 1;
                        }
                    }
                }
            }
        }
        slot = slot + 1;
    } while (remaining >= 0);
}

void func_001609D8(void);

void SND_fn_vSynchroSnd(void)
{
    func_001609D8();
    SND_fn_vSynchroSxd();
}

s32 SND_fn_bCheckVersionResourceSnd(SndBinResourceDisk_inv* resource)
{
    if (resource->header.kind == 1) {
        return SND_fn_bCheckVersionResourceSxd(resource);
    }
    return 1;
}

SndBinResource* SND_fn_tduGetLowestResourceSnd(SndBinResource* resource, _SoundParam* params, s32 mode)
{
    SndBinResource* candidate;
    s32 kind;

    candidate = resource;
    if (candidate == 0) {
        return 0;
    }
    kind = candidate->head.type;
    switch (kind) {
    case 0:
    case 2:
    case 5:
    case 6:
    case 7:
    case 9:
        break;
    case 1:
    case 3:
    case 4:
    case 11:
        return candidate;
    case 10:
        return SND_fn_tduGetLowestResourceRandom(candidate, params, mode);
    case 8:
        return SND_fn_tduGetLowestResourceSwitch(candidate, params, mode);
    default:
        break;
    }
    SND_fn_vDisplayError(0x15, "");
    return SND_RES_FANTOME[0];
}

s32 SND_fn_lGetNbVoiceOptimumSnd(void)
{
    return SND_fn_lGetNbVoiceOptimumSxd();
}

s32 SND_fn_lGetNbVoiceWishedSnd(void)
{
    return SND_fn_lGetNbVoiceWishedSxd();
}

void SND_fn_vSetNbVoiceWishedSnd(s32 count)
{
    SND_fn_vSetNbVoiceWishedSxd(SND_fn_eGetMainFormatSxd_0(count), count);
}

s32 SND_fn_uGenerateResourceSnd(s32 a0, s32 a1, s32 a2)
{
    return 0;
}

void SND_fn_vDestroyResourceSnd(void *a0)
{
}

s32 SND_fn_bSetResourceStaticVolumeSnd(void* res, u8 volume)
{
    if (res != 0) {
        switch (((SndResourceHeaderView*) res)->type) {
        case 1:
            return SND_fn_bSetResourceStaticVolumeSxd(res, volume);
        case 4:
            return inv_SND_fn_bSetResourceStaticVolumeSequence(res, volume);
        case 8:
            return SND_fn_bSetResourceStaticVolumeSwitch(res, volume);
        case 10:
            return SND_fn_bSetResourceStaticVolumeRandom(res, volume);
        case 11:
            return SND_fn_bSetResourceStaticVolumeTheme(res, volume);
        }
    }
    return 0;
}
