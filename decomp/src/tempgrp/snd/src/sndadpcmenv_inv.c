#include "common.h"

#include "snd/snd.h"

extern AdpcmManager gst_Env;

s32 fn_iConstructAdpcmEnv(void)
{
    return fn_iConstructTdstAdpcmMngr(&gst_Env);
}

s32 fn_iDestructAdpcmEnv(void)
{
    return fn_iDestructTdstAdpcmMngr(&gst_Env);
}

u32 fn_u32AllocateVoiceAdpcmEnv(s32 resource, s32 block, SndParam2D* param)
{
    return fn_u32AllocateVoiceTdstAdpcmMngr(&gst_Env, (AdpcmResource*) resource, (AdpcmResource*) block, param);
}

s32 fn_u32DesallocateVoiceAdpcmEnv(s32 voice)
{
    return fn_u32DesallocateVoiceTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

s32 fn_bIsAllocateVoiceAdpcmEnv(s32 voice)
{
    return fn_bIsAllocateVoiceTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

s32 fn_iStartVoiceAdpcmEnv(u32 voice, s32 environment, s32 resource, u32 argument, AdpcmDataCallback callback)
{
    return fn_iStartVoiceTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice, (AdpcmResource*) environment,
                                       (AdpcmResource*) resource, argument, callback);
}

s32 fn_iStopVoiceAdpcmEnv(s32 voice)
{
    return fn_iStopVoiceTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

s32 fn_bIsPlayingVoiceAdpcmEnv(s32 voice)
{
    return fn_bIsPlayingVoiceTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

s32 fn_bSetParamAdpcmEnv(s32 voice, SndParam2D* param)
{
    return fn_bSetParamTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice, param);
}

s32 fn_u32PushNextBlockResourceMemAdpcmEnv(s32 voice, s32 mode, s32 resource)
{
    return gu_fn_iCloseTdstFileInfo(&gst_Env, (AdpcmData*) voice, mode, (AdpcmResource*) resource);
}

void fn_vPauseAdpcmEnv(s32 voice)
{
    fn_vPauseTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

void fn_vResumeAdpcmEnv(s32 voice)
{
    fn_vResumeTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

s32 fn_rGetPosAdpcmEnv(s32 voice)
{
    return fn_rGetPosTdstAdpcmMngr(&gst_Env, (AdpcmData*) voice);
}

void fn_vSetStereoAdpcmEnv(s32 stereo)
{
    fn_vSetStereoTdstAdpcmMngr(&gst_Env, stereo);
}

s32 fn_vGetStereoAdpcmEnv(void)
{
    return fn_bGetStereoTdstAdpcmMngr(&gst_Env);
}

void fn_vSynchroSoundAdpcmEnv(void)
{
    fn_vSynchroSoundTdstAdpcmMngr(&gst_Env);
}
