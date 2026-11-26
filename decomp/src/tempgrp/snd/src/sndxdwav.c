#include "common.h"

#include "snd/snd.h"

s32 SND_fn_lPlaySxdWave(void* resource, _SoundParam* param3D, s32 a2, SndEndCallback_inv a3, s32 t0)
{
    SndParam2D param2D;

    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_lPlaySample(resource, &param2D, a2, a3, t0);
}

void SND_fn_vRemoveCallbackSxdWave(void* callback)
{
}

s32 SND_fn_bSetParamSxdWave(void* sound, _SoundParam* param3D)
{
    SndParam2D param2D;

    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_bSetParamSample(sound, &param2D);
}

s32 SND_fn_bTestIsPlayingSxdWave(s32 handle)
{
    return SND_fn_bTestIsPlayingSample(handle);
}

void j_nullsub_76(s32 a0)
{
    SND_fn_vStopSample();
}

void SND_fn_vPauseSxdWave(s32 a0)
{
    SND_fn_vPauseSample();
}

void SND_fn_vResumeSxdWave(s32 a0)
{
    SND_fn_vResumeSample();
}

s32 SND_fn_iInitSxdWave(void *a0)
{
    return SND_fn_iInitSample();
}

s32 SND_fn_bTestInitSxdWave(void)
{
    return SND_fn_bTestInitSample();
}

void SND_fn_vDesInitSxdWave(void)
{
    SND_fn_vDesInitSample();
}

void SND_fn_vSetDopplerFactorSxdWave(void)
{
}

void SND_fn_vCreateMicroSxdWave(SxdMicroParams_inv* params, SxdMicro* micro)
{
}

void SND_fn_vSetMicroParamSxdWave(void)
{
}

void SND_fn_vDestroyMicroSxdWave(SxdMicro* micro)
{
}

s32 SND_fn_bLoadResBinarySxdWave(SndSampleDisk* source, SndSample* sample, u32 base)
{
    return SND_fn_bLoadResBinarySample(source, sample);
}

void SND_fn_vConvertResDiskToMemSxdWave(void)
{
    SND_fn_vConvertResDiskToMemSample();
}

void loc_165220(SndSample* sample)
{
    SND_fn_vUnLoadResSample();
}

s32 SND_fn_bIsResLoadedSxdWave(void)
{
    return SND_fn_bIsResLoadedSample();
}

void SND_fn_lCreateNewBufferSxdWave(void* a0, u16 a1, u16 a2, void* a3, _SoundParam* param3D, s32 a5,
                                    s32 a6)
{
    SndParam2D param2D;
    void* params;

    if (param3D != NULL) {
        SND_fn_vParam3Dto2D(param3D, &param2D);
        params = &param2D;
    } else {
        params = &C_SAMPLEPARAM_NEUTRE;
    }
    SND_fn_lCreateNewBufferSample(a0, a1, a2, a3, params, a5, a6);
}

void SND_fn_vSetParamBufferSxdWave(void* buffer, _SoundParam* param3D)
{
    SndParam2D param2D;
    const SndParam2D* param;

    if (param3D != NULL) {
        SND_fn_vParam3Dto2D(param3D, &param2D);
        param = &param2D;
    } else {
        param = NULL;
    }
    SND_fn_vSetParamBufferSample(buffer, param);
}

void SND_fn_vDeleteBufferSxdWave(s32 a0)
{
    SND_fn_vEnterCriticalSectionThreadSnd_();
    SND_fn_vMouchardThreadsnd("/home/ps2/last_version/dev/cpa/tempgrp/snd/src/sndxdwav.c", 0xDD);
    SND_fn_vDeleteBufferSample(a0);
    SND_fn_vQuitCriticalSectionThreadSnd();
}

void loc_165368(void *a0)
{
    SND_fn_vPauseBufferSample();
}

void loc_165370(void *a0)
{
    SND_fn_vResumeBufferSample();
}

void* SND_fn_pvGetPtrBufferSxdWave(void* buffer)
{
    return SND_fn_pvGetPtrBufferSample(buffer);
}

s32 SND_fn_rGetPosBufferSxdWave(void* buffer)
{
    return SND_fn_rGetPosBufferSample(buffer);
}

static inline void* inv_SND_fn_p_vConvertBufferParams(_SoundParam* param3D, SndParam2D* param2D)
{
    if (param3D != NULL) {
        SND_fn_vParam3Dto2D(param3D, param2D);
        return param2D;
    }
    return &C_SAMPLEPARAM_NEUTRE;
}

s32 SND_fn_lCreateNewBufferExSxdWave(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                     _SoundParam* param3D, void* user)
{
    SndParam2D param2D;
    void* params = inv_SND_fn_p_vConvertBufferParams(param3D, &param2D);

    return SND_fn_lCreateNewBufferExSample(description, format, params, user);
}

s32 SND_fn_lGetPosBufferExSxdWave(void* buffer)
{
    return SND_fn_lGetPosBufferExSample(buffer);
}

s32 SND_fn_lPushBufferExSxdWave(void)
{
    return SND_fn_lPushBufferExSample();
}

void SND_fn_vReleaseDriverSxdWave(void)
{
    SND_fn_vReleaseDriverSample();
}

void SND_fn_vRestoreDriverSxdWave(void)
{
    SND_fn_vRestoreDriverSample();
}

void SND_fn_vForceReleaseDriverSxdWave(void)
{
    SND_fn_vForceReleaseDriverSample();
}

void SND_fn_vForceRestoreDriverSxdWave(void)
{
    SND_fn_vForceRestoreDriverSample();
}

s32 SND_fn_rGetPosSxdWave(s32 a0)
{
    return SND_fn_rGetPosSample();
}

s32 SND_fn_rGetLengthSxdWave(s32 a0)
{
    return SND_fn_rGetLengthSample();
}

#ifdef PC_PORT
void SND_fn_vGetTargetLabelSxdWave(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSample(name, size);
}
#else
void SND_fn_vGetTargetLabelSxdWave(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSample(name, size);
}
#endif

void SND_fn_vSetupTargetSxdWave(void)
{
    SND_fn_vSetupTargetSample();
}

s32 SND_fn_bCanSetupTargetSxdWave(void)
{
    return SND_fn_bCanSetupTargetSample();
}

void SND_fn_vSetReverseStereoSxdWave(void)
{
    SND_fn_vSetReverseSample();
}

s32 SND_fn_bGetReverseStereoSxdWave(void)
{
    return SND_fn_bGetReverseSample();
}

void j_nullsub_22(void)
{
    SND_fn_vSetEffectSample();
}

void loc_165520(s32 a0)
{
    SND_fn_vSetSoundVolumeSample(a0 & 0xFF);
}

s32 SND_fn_ucGetSoundVolumeSxdWave(void)
{
    return SND_fn_ucGetSoundVolumeSample();
}

void SND_fn_vSynchroSxdWave(void)
{
    SND_fn_vSynchroSample();
}

void SND_fn_vSetDefaultRollOffSxdWave(SndRollOff* rollOff)
{
}

s32 SND_fn_bCheckVersionResourceSxdWave(void)
{
    return 1;
}

void SND_fn_vSetNbVoiceWishedSxdWave(void)
{
    SND_fn_vSetNbVoiceWishedSample();
}

s32 SND_fn_lGetNbVoiceWishedSxdWave(void)
{
    return SND_fn_lGetNbVoiceWishedSample();
}

s32 SND_fn_lGetNbVoiceOptimumSxdWave(void)
{
    return SND_fn_lGetNbVoiceOptimumSample();
}

s32 SND_fn_bIsResourceLoopingSxdWave(SndSample* sample)
{
    return SND_fn_bIsResourceLoopingSample(sample);
}

s32 SND_fn_b3DOneShootShouldBeConvertedSxdWave(void)
{
    return 0;
}

s32 SND_fn_bSetResourceStaticVolumeSxdWave(s32 resource, s32 volume)
{
    return SND_fn_bSetResourceStaticVolumeSample(resource, volume & 0xFF);
}
