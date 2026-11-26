#include "common.h"

#include "ps2/libc.h"
#include "snd/snd.h"

s32 sceSifInitIopHeap(void);

s32 SND_fn_iInitSample(void)
{
#ifdef PC_PORT
    return 3;
#else
    if (sceSifInitIopHeap() == 0) {
        return 3;
    }
    return -1;
#endif
}

s32 SND_fn_bTestInitSample(void)
{
    return 1;
}

void SND_fn_vDesInitSample(void)
{
}

s32 SND_fn_lPlaySample(void* resource, SndParam2D* param, s32 a2, SndEndCallback_inv callback, s32 argument)
{
    return 0;
}

void SND_fn_vRemoveCallbackSample(void)
{
}

s32 SND_fn_bSetParamSample(void* sound, SndParam2D* param)
{
    return 0;
}

s32 SND_fn_bTestIsPlayingSample(s32 handle)
{
    return 0;
}

void SND_fn_vStopSample(void)
{
}

void SND_fn_vPauseSample(void)
{
}

void SND_fn_vResumeSample(void)
{
}

s32 SND_fn_rGetPosSample(void)
{
    return -1;
}

s32 SND_fn_rGetLengthSample(void)
{
    return -1;
}

s32 SND_fn_iInitBnkSample(void)
{
    return -1;
}

s32 SND_fn_bTestInitBnkSample(void)
{
    return 0;
}

void SND_fn_vDesInitBnkSample(void)
{
}

void SND_fn_vDesInitAllBnkSample(void)
{
}

void SND_fn_vSetEffectSample(void)
{
}

s32 SND_fn_tdGetResForVoiceSample(void)
{
    return 0;
}

s32 SND_fn_lCreateNewBufferSample(void *a0, u16 a1, u16 a2, void *a3, void *a4, s32 a5, s32 a6)
{
    return -1;
}

void SND_fn_vSetParamBufferSample(void* buffer, const SndParam2D* param)
{
}

void SND_fn_vDeleteBufferSample(s32 a0)
{
}

void SND_fn_vPauseBufferSample(void)
{
}

void SND_fn_vResumeBufferSample(void)
{
}

void* SND_fn_pvGetPtrBufferSample(void* buffer)
{
    return 0;
}

s32 SND_fn_rGetPosBufferSample(void* buffer)
{
    return 0;
}

s32 SND_fn_lCreateNewBufferExSample(void* a0, void* a1, void* params, void* a3)
{
    return -1;
}

s32 SND_fn_lGetPosBufferExSample(void* buffer)
{
    return -1;
}

s32 SND_fn_lPushBufferExSample(void)
{
    return -1;
}

s32 SND_fn_bLoadResScriptSample(void)
{
    return 0;
}

s32 SND_fn_bLoadResBinarySample(SndSampleDisk* source, SndSample* sample)
{
    sample->unk00 = source->unk00;
    sample->type = source->type;
    sample->unk08 = source->unk08;
    sample->staticVolume = source->staticVolume;
    sample->unk10 = 1;
    sample->lFormat = 1;
    sample->flags.bParamPitch = source->flags.bParamPitch;
    sample->flags.bParamVolume = source->flags.bParamVolume;
    sample->flags.bParamPan = source->flags.bParamPan;
    sample->flags.b3 = source->flags.b3;
    sample->flags.b4 = source->flags.b4;
    sample->flags.bStream = source->flags.bStream;
    sample->flags.bLooping = source->flags.bLooping;
    sample->cNbChannels = source->cNbChannels;
    sample->ucBitsPerSample = source->ucBitsPerSample;
    sample->uwRate = source->uwRate;
    return 1;
}

void SND_fn_vUnLoadResSample(void)
{
}

s32 SND_fn_bIsResLoadedSample(void)
{
    return 0;
}

s32 SND_fn_lPlayTransitionSample(void)
{
    return -1;
}

s32 SND_fn_bSetParamTransitionSample(void)
{
    return 0;
}

s32 SND_fn_bSetNextTransitionSample(void)
{
    return 0;
}

s32 SND_fn_bDoTransitionWithFadeSample(void)
{
    return 0;
}

void SND_fn_vStopTransitionSample(void)
{
}

void SND_fn_vPauseTransitionSample(void)
{
}

void SND_fn_vResumeTransitionSample(void)
{
}

void SND_fn_vReleaseDriverSample(void)
{
}

void SND_fn_vRestoreDriverSample(void)
{
}

void SND_fn_vGetTargetLabelSample(char* name, s32 size)
{
    if (size < 5) {
        strcpy(name, "psx2");
    }
}

void SND_fn_vSetupTargetSample(void)
{
}

s32 SND_fn_bCanSetupTargetSample(void)
{
    return 0;
}

void SND_fn_vSynchroSample(void)
{
}

s32 SND_fn_lGetNbVoiceWishedSample(void)
{
    return 0;
}

void SND_fn_vSetNbVoiceWishedSample(void)
{
}

s32 SND_fn_bIsResourceLoopingSample(SndSample* sample)
{
    return 0;
}

void SND_fn_vConvertResDiskToMemSample(void)
{
}

void SND_fn_vForceReleaseDriverSample(void)
{
}

void SND_fn_vForceRestoreDriverSample(void)
{
}

void SND_fn_vSetReverseSample(void)
{
}

void SND_fn_vSetSoundVolumeSample(s32 unused)
{
}

s32 SND_fn_bGetReverseSample(void)
{
    return 0;
}

s32 SND_fn_ucGetSoundVolumeSample(void)
{
    return 0;
}

s32 SND_fn_lGetNbVoiceOptimumSample(void)
{
    return 0;
}

s32 SND_fn_bSetResourceStaticVolumeSample(s32 resource, s32 volume)
{
    return 0;
}
