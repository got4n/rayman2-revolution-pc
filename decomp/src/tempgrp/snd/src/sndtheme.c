#include "common.h"

#include "snd/snd.h"

#include "ps2/libc.h"

typedef void (*SndThemeCallback)(s32 argument);

typedef struct SndThemePlayer {
    s32 transiting;
    s32 ended;
    s32 unk08;
    s32 changePending;
    s32 firstPart;
    SndBinResource* theme;
    SndThemePart* loopStart;
    s32 loopIndex;
    s32 loopCount;
    u32 repeat;
    SndThemePart* previousPart;
    s32 previousIndex;
    s32 transition;
    SndThemePart* currentPart;
    s32 partIndex;
    SndBinResource* resource;
    s32 unk40;
    SndBinResource* nextTheme;
    s32 unk48;
    SndThemeCallback callback;
    s32 callbackArgument;
    s32* inv_pPlaying;
    _SoundParam* soundParams;
    s32 soundParamsSize;
} SndThemePlayer;

extern SndThemePlayer g_stTheme;

void SND_fn_vConvertResDiskToMemTheme(SndThemeDisk* disk, SndBinResource* theme, s32 unused);
void SND_fn_vStopTheme(SndThemePlayer* player);

s32 raj_next_part(SndThemePlayer* player)
{
    SndBinResource* theme;
    SndBinResource* current;
    SndBinResource* next;
    SndThemeSequence* sequence;
    SndThemePart* part;
    SndThemePart* loopPart;
    SndThemePart* scan;
    s32 index;
    s32 restart;
    s32 ended;

    ended = 0;
    if (player->changePending != 0) {
        next = player->nextTheme;
        player->changePending = 0;
        if (next == NULL) {
        finished:
            player->ended = 1;
            return 0;
        }
        player->theme = next;
        if (player->firstPart != 0) {
            player->firstPart = 0;
            player->partIndex = 0;
            loopPart = next->theme.sequence.parts;
            player->currentPart = loopPart;
            player->resource = loopPart->resource.data;
            return 1;
        }
    }
    theme = player->theme;
    current = theme;
    sequence = &theme->theme.sequence;
    if (player->previousPart->loopEnd) {
        if (player->loopCount != 0) {
            player->loopCount--;
            loopPart = player->loopStart;
            if (loopPart != NULL) {
                player->currentPart = loopPart;
                player->partIndex = player->loopIndex;
                player->resource = loopPart->resource.data;
                part = loopPart;
            } else {
                SndThemePart* first = sequence->parts;

                player->partIndex = 0;
                player->currentPart = first;
                player->resource = first->resource.data;
                part = first;
            }
            goto checkLoopStart;
        }
        player->loopStart = NULL;
    } else if (player->loopCount == 0) {
        player->loopStart = NULL;
    }
    theme = player->theme;
    if ((u32) ++player->partIndex >= current->theme.sequence.partCount) {
        if (theme->theme.sequence.repeatCount != 0) {
            ended = ++player->repeat >= theme->theme.sequence.repeatCount;
        }
        if (ended) {
            if (current->theme.sequence.optionalPart == NULL) {
                goto finished;
            }
            part = player->currentPart;
            player->firstPart = 1;
            player->changePending = 1;
            player->resource = theme->theme.sequence.optionalPart->resource.data;
            player->nextTheme = SND_RES_FANTOME[0];
        } else {
            restart = sequence->repeatStart;
            player->partIndex = restart;
            player->currentPart = &sequence->parts[restart];
            player->resource = player->currentPart->resource.data;
            scan = player->currentPart;
            index = sequence->repeatStart;
            do {
                part = scan;
            } while (0);
            goto scanPart;
            do {
                if (scan == NULL || scan->loopEnd) {
                    break;
                }
            scanPart:
                if (scan == NULL) {
                    break;
                }
                if (scan->loopStart) {
                    player->loopIndex = index;
                    player->loopStart = scan;
                    player->loopCount = scan->loopCount;
                    break;
                }
                index--;
                scan--;
            } while (index >= 0);
        }
    } else {
        loopPart = player->currentPart;
        player->currentPart = loopPart + 1;
        player->resource = loopPart[1].resource.data;
        part = player->currentPart;
    }
checkLoopStart:
    if (part->loopStart && player->loopStart == NULL) {
        player->loopStart = part;
        player->loopIndex = player->partIndex;
        player->loopCount = part->loopCount;
    }
    return 1;
}

s32 change_theme(SndThemePlayer* player, SndBinResource* theme)
{
    s32 transition;
    SndThemePart* parts;
    SndThemePart* part;
    s32 unused[3];

    if (player->ended != 0)
        transition = 1;
    else
        transition = player->previousPart->inv_lTransitionType;
    player->firstPart = 0;
    player->changePending = 1;
    player->ended = 0;
    player->nextTheme = theme;
    if (theme->theme.sequence.partCount == 0) {
        player->currentPart = NULL;
        player->partIndex = -1;
        player->resource = SND_RES_FANTOME[0];
        return 0;
    }
    player->loopStart = NULL;
    player->loopIndex = -1;
    player->loopCount = -1;
    player->repeat = 0;
    player->partIndex = 0;
    parts = theme->theme.sequence.parts;
    player->currentPart = parts;
    if (transition != 2 || player->theme->theme.sequence.optionalPart == NULL) {
        player->resource = parts[0].resource.data;
    } else {
        player->firstPart = 1;
        player->resource = player->theme->theme.sequence.optionalPart->resource.data;
    }
    part = player->currentPart;
    if (part->loopStart) {
        player->loopStart = part;
        player->loopIndex = player->partIndex;
        player->loopCount = part->loopCount;
    }
    if (transition != 0)
        return (u8) transition & 0x100;
    return 1;
}

void start_new_theme(SndThemePlayer* player, SndBinResource* theme)
{
    SndThemePart* part;

    player->theme = theme;
    player->changePending = 0;
    player->unk08 = 0;
    player->firstPart = 0;
    player->ended = 0;
    if (theme->theme.sequence.partCount == 0) {
        player->previousPart = NULL;
        g_bWorkingTheme = 0;
        return;
    }
    player->loopStart = NULL;
    player->loopIndex = -1;
    player->loopCount = -1;
    player->repeat = 0;
    player->previousIndex = 0;
    part = theme->theme.sequence.parts;
    player->partIndex = 0;
    player->previousPart = part;
    player->currentPart = part;
    player->resource = part->resource.data;
    if (part->loopStart) {
        player->loopStart = part;
        player->loopIndex = 0;
        player->loopCount = part->loopCount;
    }
    raj_next_part(player);
}

void fn_Transition(SndThemePlayer* player)
{
    if (g_bWorkingTheme != 0) {
        return;
    }
    if (player->transiting == 0) {
        return;
    }
    if (player->ended != 0) {
        return;
    }
    player->previousIndex = player->partIndex;
    player->previousPart = player->currentPart;
    if (raj_next_part(player) == 0) {
        if (player->inv_pPlaying != NULL) {
            player->inv_pPlaying = NULL;
        }
        if (player->callback != NULL) {
            player->callback(player->callbackArgument);
        }
    } else {
        SND_fn_bSetNextTransitionSnd(
            player->transition,
            SND_fn_tduGetLowestResourceSnd(player->resource, player->soundParams, 1));
    }
}

s32 func_001597B8(SndBinResource** theme, s64* previousIndex, s64* loopCount)
{
    if (g_stTheme.transiting == 0) {
        return 0;
    }
    *theme = g_stTheme.theme;
    *previousIndex = g_stTheme.previousIndex;
    *loopCount = g_stTheme.loopCount;
    return 1;
}

s32 func_001597F0(s32* unk40, s32* changePending)
{
    if (g_stTheme.transiting == 0 || g_stTheme.unk08 == 0) {
        return 0;
    }
    *unk40 = g_stTheme.unk40;
    *changePending = g_stTheme.changePending;
    return 1;
}

void inv_SND_fn_vStopThemeIfPlaying(void)
{
    SndThemePlayer* player = &g_stTheme;

    if (SND_fn_bTestIsPlayingTheme() != 0) {
        SND_fn_vStopTheme(player);
    }
}

void SND_fn_vInitTheme(void)
{
    g_stTheme.transiting = 0;
    g_stTheme.soundParams = NULL;
    g_stTheme.soundParamsSize = 0;
}

void inv_SND_fn_vNoOpTheme(void)
{
}

void SND_fn_vConvertResDiskToMemTheme(SndThemeDisk* disk, SndBinResource* theme, s32 unused)
{
    theme->theme.header[0] = disk->header[0];
    theme->theme.header[1] = disk->header[1];
    theme->theme.header[2] = disk->header[2];
    theme->theme.parameter = disk->parameter;
    theme->theme.state = 1;
    theme->theme.sequence.repeatStart = disk->repeatStart;
    theme->theme.sequence.partCount = disk->partCount;
    theme->theme.sequence.repeatCount = disk->repeatCount;
    theme->theme.sequence.word10 = disk->word2C;
}

s32 SND_fn_bLoadResBinaryTheme(SndThemeDisk* disk, SndBinResource* theme, u32 base)
{
    SndThemePart* part;
    s32 index;

    base += disk->partsOffset;
    theme->theme.sequence.parts = (SndThemePart*) base;
    part = (SndThemePart*) base;
    theme->theme.sequence.copiedPartCount = disk->partCount;
    for (index = 0; index < theme->theme.sequence.copiedPartCount; index++, part++) {
        part->resource.data = SND_fn_pGetBinRes(part->resource.id);
    }
    if (disk->optionalPartOffset != -1) {
        theme->theme.sequence.optionalPart = (SndThemePart*) (base + disk->optionalPartOffset);
        theme->theme.sequence.optionalPart->resource.data =
            SND_fn_pGetBinRes(theme->theme.sequence.optionalPart->resource.id);
    } else {
        theme->theme.sequence.optionalPart = NULL;
    }
    SND_fn_vConvertResDiskToMemTheme(disk, theme, 0);
    return 1;
}

void SND_fn_vUnLoadResTheme(SndBinResource *a0)
{
}

s32 func_001599B0(SndBinResource* theme)
{
    SndThemePart* part;
    s32 balance;
    s32 index;

    if (theme->theme.sequence.repeatStart >= theme->theme.sequence.partCount ||
        theme->theme.sequence.partCount != theme->theme.sequence.copiedPartCount) {
        return 0;
    }
    part = theme->theme.sequence.parts;
    balance = 0;
    for (index = 0; index < theme->theme.sequence.partCount; index++, part++) {
        balance -= part->loopStart;
        if (balance >= 2) {
            return 0;
        }
        balance += part->loopEnd;
        if (balance < 0) {
            return 0;
        }
    }
    return balance == 0;
}

void raj_soundparam_theme(SndThemePlayer* player, _SoundParam* params)
{
    s32 size;

    if (params != NULL) {
        size = params->microCount * 8 + 0x38;
    } else {
        size = 0;
    }

    if (size != 0) {
        if (size != player->soundParamsSize) {
            if (player->soundParams != NULL) {
                player->soundParams = SND_fn_pvReAllocSndEx(0, player->soundParams, size);
            } else {
                player->soundParams = SND_fn_pvMallocSndEx(0, size);
            }
        }
        player->soundParamsSize = size;
        memcpy(player->soundParams, params, size);
    } else if (player->soundParams != NULL) {
        SND_fn_vFreeSndEx(0, player->soundParams);
        player->soundParams = NULL;
        player->soundParamsSize = 0;
    }
}

typedef union SndSoundParamCopy {
    _SoundParam param;
    u8 bytes[0x60];
} SndSoundParamCopy;

s32 SND_fn_lPlayTheme(s32 unused, SndBinResource* theme, _SoundParam* params, s32 unused3,
                      SndThemeCallback callback, s32 callbackArgument, s32* playing)
{
    SndThemePlayer* player = &g_stTheme;
    SndSoundParamCopy copy;

    raj_soundparam_theme(player, params);
    if ((player->transiting != 0 && theme == player->theme && player->changePending == 0) ||
        (player->changePending != 0 && player->nextTheme != NULL))
        return (s32) player;

    g_bWorkingTheme = 1;
    memcpy(&copy, params, params->microCount * 8 + 0x38);
    if (player->transiting != 0) {
        if (change_theme(player, theme) != 0) {
            SndBinResource* resource = SND_fn_tduGetLowestResourceSnd(player->resource, player->soundParams, 1);
            s32 fade = inv_SND_lThemeFadeDuration;

            if (inv_SND_bThemeFadeStarted == 0) {
                inv_SND_bThemeFadeStarted = 1;
                SND_fn_bDoTransitionWithFadeSnd2((PlaySndSlot*) player->transition, resource, fade);
            } else {
                SND_fn_bDoTransitionWithFadeSnd(player->transition, resource, fade);
            }
        } else {
            SND_fn_bSetNextTransitionSnd(
                player->transition,
                SND_fn_tduGetLowestResourceSnd(player->resource, player->soundParams, 1));
        }
    } else {
        SndBinResource* first;
        SndBinResource* next;

        start_new_theme(player, theme);
        player->callback = callback;
        player->callbackArgument = callbackArgument;
        player->inv_pPlaying = playing;
        copy.param.flags |= 4;
        copy.param.vol = (copy.param.vol * theme->theme.parameter) >> 7;
        first = SND_fn_tduGetLowestResourceSnd(player->previousPart->resource.data, &copy.param, 1);
        next = SND_fn_tduGetLowestResourceSnd(player->resource, &copy.param, 1);
        player->transition =
            SND_fn_lPlayTransitionSnd(first, next, fn_Transition, player, &copy.param);
    }
    if (player->transiting == 0) {
        if (player->transition != -1) {
            player->transiting = 1;
            *playing = 1;
        } else {
            *playing = 0;
        }
    }
    g_bWorkingTheme = 0;
    return player->transiting != 0 ? (s32) player : -1;
}

s32 SND_fn_bSetParamTheme(s32 handle, _SoundParam* params)
{
    SndThemePlayer* player = (SndThemePlayer*) handle;
    SndSoundParamCopy copy;

    raj_soundparam_theme(player, params);
    memcpy(&copy, params, params->microCount * 8 + 0x38);
    copy.param.vol = (copy.param.vol * player->theme->theme.parameter) >> 7;
    SND_fn_bSetParamTransitionSnd(player->transition, &copy.param);
    return player->transiting;
}

void* func_00159D60(void** object)
{
    return *object;
}

s32 SND_fn_bTestIsPlayingTheme(void)
{
    return g_stTheme.transiting;
}

void SND_fn_vStopTheme(SndThemePlayer* player)
{
    s32 transiting = player->transiting;

    g_bWorkingTheme = 1;
    if (transiting != 0) {
        SND_fn_vStopTransitionSnd((PlaySndSlot*) player->transition);
    }
    if (player->callback != NULL) {
        player->callback(player->callbackArgument);
    }
    player->transiting = 0;
    player->changePending = 0;
    printf("SND_fn_vStopThem:pTheme->bTransiting=FALSE\n");
    player->nextTheme = SND_RES_FANTOME[0];
    g_bWorkingTheme = 0;
}

void SND_fn_vPauseTheme(SndThemePlayer* player)
{
    if (player->transiting != 0) {
        SND_fn_vPauseTransitionSnd((PlaySndSlot*) player->transition);
    }
}

void SND_fn_vResumeTheme(SndThemePlayer* player)
{
    if (player->transiting != 0) {
        SND_fn_vResumeTransitionSnd((PlaySndSlot*) player->transition);
    }
}

void func_00159E28(s32 a0)
{
}

s32 SND_fn_rGetPosTheme(void)
{
    return -1;
}

s32 SND_fn_rGetLengthTheme(void *a0)
{
    return -1;
}

s32 SND_fn_bSetResourceStaticVolumeTheme(SndResourceHeaderView* resource, s32 value)
{
    resource->staticVolume = value;
    return 1;
}
