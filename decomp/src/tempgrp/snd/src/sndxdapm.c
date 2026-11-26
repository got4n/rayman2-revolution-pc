#include "common.h"

#include "ps2/r5900.h"
#include "snd/snd.h"

#define ADPCM_INVALID_VOICE 0xFFFFFFFFu
#define ADPCM_RESOURCE_LOADED 1
#define ADPCM_STEREO_PAIR 2
#define ADPCM_FADE_BLOCK 0x60000
#define ADPCM_FADE_DURATION 0x10000
#define ADPCM_NUMBER_OF_VOICES 0x10

typedef union SND_AdpcmFlagBits {
    u32 word;

    struct {
        unsigned bParamPitch : 1;
        unsigned bParamVolume : 1;
        unsigned bParamPan : 1;
        unsigned b3 : 1;
        unsigned b4 : 1;
        unsigned bStream : 1;
        unsigned bFromCD : 1;
        unsigned bLooping : 1;
        unsigned bInterleaved : 1;
        unsigned rest : 23;
    } unsignedBits;

    struct {
        signed bParamPitch : 1;
        signed bParamVolume : 1;
        signed bParamPan : 1;
        signed b3 : 1;
        signed b4 : 1;
        signed bStream : 1;
        signed bFromCD : 1;
        signed bLooping : 1;
        signed bInterleaved : 1;
        unsigned rest : 23;
    } signedBits;
} SND_AdpcmFlagBits;

extern s32 g_bInitOK;

s32 fn_iTestStatusAdpcmEnv(void);
s32 SND_fn_lPlaySampleADPCM(s32 resource, SndParam2D* param, s32 extra0, SndEndCallback_inv callback,
                            s32 argument);
void SND_fn_vRemoveCallbackSampleADPCM(void);
s32 SND_fn_bSetParamSampleADPCM(s32 voice, SndParam2D* param);
s32 SND_fn_bTestIsPlayingSampleADPCM(s32 voice);
void SND_fn_vStopSampleADPCM(s32 voice);
void SND_fn_vPauseSampleADPCM(s32 voice);
void SND_fn_vResumeSampleADPCM(s32 voice);
s32 SND_fn_bIsResourceLoopingSampleADPCM(SND_tdstAdpcmBinRes* resource);
s32 SND_fn_rGetPosSampleADPCM(s32 voice);
s32 SND_fn_rGetLengthSampleADPCM(void);
void SND_fn_vSetSoundVolumeSampleADPCM(u8 volume);
s32 SND_fn_ucGetSoundVolumeSampleADPCM(void);
void SND_fn_vSetReverseStereoSampleADPCM(void);
s32 SND_fn_bGetReverseStereoSampleADPCM(void);
s32 SND_fn_iInitSampleADPCM(void);
s32 SND_fn_bTestInitSampleADPCM(void);
void SND_fn_vDesInitSampleADPCM(void);
void SND_fn_vSetEffectSampleADPCM(void);
s32 SND_fn_lCreateNewBufferSampleADPCM(s32 voice, u16 extra0, u16 extra1, s32 extra2, SndParam2D* param,
                                 s32 extra3, s32 extra4);
void SND_fn_vSetParamBufferSampleADPCM(void* micro, SndParam2D* param);
void SND_fn_vDeleteBufferSampleADPCM(void);
void SND_fn_vPauseBufferSampleADPCM(void);
void SND_fn_vResumeBufferSampleADPCM(void);
s32 SND_fn_pvGetPtrBufferSampleADPCM(void);
s32 SND_fn_rGetPosBufferSampleADPCM(void);
s32 SND_fn_bLoadResBinarySampleADPCM(const SndAdpcmDiskResource* source,
                                     SND_tdstAdpcmBinRes* resource, s32 base);
s32 SND_fn_bLoadResBinaryAndConvertToWAVSampleADPCM(void);
void SND_fn_vUnLoadResSampleADPCM(void);
s32 SND_fn_bIsResLoadedSampleADPCM(void);
s32 SND_fn_lPlayTransitionSampleADPCM(s32 resource, s32 extra0, AdpcmDataCallback callback, s32 argument,
                                      SndParam2D* param);
s32 SND_fn_bSetParamTransitionSampleADPCM(s32 voice, SndParam2D* param);
s32 SND_fn_bSetNextTransitionSampleADPCM(s32 voice, s32 size);
s32 SND_fn_bDoTransitionWithFadeSampleADPCM(s32 voice, s32 size, s32 duration);
void SND_fn_vStopTransitionSampleADPCM(s32 voice);
void SND_fn_vPauseTransitionSampleADPCM(s32 voice);
void SND_fn_vResumeTransitionSampleADPCM(s32 voice);
void SND_fn_vSetStereoSampleADPCM(s32 stereo);
s32 SND_fn_bGetStereoSampleADPCM(void);
void SND_fn_vSynchroSampleADPCM(void);
s32 SND_fn_bCanFreeDataSampleADPCM(void);
void SND_fn_vReleaseDriverSampleADPCM(void);
void SND_fn_vRestoreDriverSampleADPCM(void);
void func_00168038(void);
void func_00168040(void);
s32 SND_fn_vGetTargetLabelSampleADPCM(void);
s32 SND_fn_vSetupTargetSampleADPCM(void);
s32 SND_fn_bCanSetupTargetSampleADPCM(void);
s32 SND_fn_lCreateNewBufferExSampleADPCM(void);
s32 SND_fn_lGetPosBufferExSampleADPCM(void);
s32 SND_fn_lGetNbVoiceWishedSampleADPCM(void);
s32 SND_fn_bSetResourceStaticVolumeSampleADPCM(s32 resource, u8 volume);

s32 SND_fn_lPlaySxdADPCM(s32 resource, _SoundParam* param3D, s32 extra0, SndEndCallback_inv callback,
                         s32 argument)
{
    SndParam2D param2D;

    if (g_bInitOK == 0) {
        return -1;
    }
    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_lPlaySampleADPCM(resource, &param2D, extra0, callback, argument);
}

void SND_fn_vRemoveCallbackSxdADPCM(void *a0)
{
    SND_fn_vRemoveCallbackSampleADPCM();
}

s32 SND_fn_bSetParamSxdADPCM(s32 voice, _SoundParam* param3D)
{
    SndParam2D param2D;

    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_bSetParamSampleADPCM(voice, &param2D);
}

s32 SND_fn_bTestIsPlayingSxdADPCM(s32 voice)
{
    return SND_fn_bTestIsPlayingSampleADPCM(voice);
}

void SND_fn_vStopSxdADPCM(s32 voice)
{
    SND_fn_vStopSampleADPCM(voice);
}

void SND_fn_vPauseSxdADPCM(s32 voice)
{
    SND_fn_vPauseSampleADPCM(voice);
}

void SND_fn_vResumeSxdADPCM(s32 voice)
{
    SND_fn_vResumeSampleADPCM(voice);
}

s32 SND_fn_bIsResourceLoopingSxdADPCM(SND_tdstAdpcmBinRes* resource)
{
    return SND_fn_bIsResourceLoopingSampleADPCM(resource);
}

s32 SND_fn_rGetPosSxdADPCM(s32 voice)
{
    return SND_fn_rGetPosSampleADPCM(voice);
}

s32 SND_fn_rGetLengthSxdADPCM(s32 a0)
{
    return SND_fn_rGetLengthSampleADPCM();
}

void SND_fn_vSetSoundVolumeSxdADPCM(s32 volume)
{
    SND_fn_vSetSoundVolumeSampleADPCM(volume);
}

s32 SND_fn_ucGetSoundVolumeSxdADPCM(void)
{
    return SND_fn_ucGetSoundVolumeSampleADPCM();
}

void SND_fn_vSetStereoSxdADPCM(s32 stereo)
{
    SND_fn_vSetStereoSampleADPCM(stereo);
}

s32 SND_fn_bGetStereoSxdADPCM(void)
{
    return SND_fn_bGetStereoSampleADPCM();
}

void SND_fn_vSetReverseStereoSxdADPCM(void)
{
    SND_fn_vSetReverseStereoSampleADPCM();
}

s32 SND_fn_bGetReverseStereoSxdADPCM(void)
{
    return SND_fn_bGetReverseStereoSampleADPCM();
}

s32 SND_fn_iInitSxdADPCM(void *a0)
{
    g_bInitOK = 1;
    return SND_fn_iInitSampleADPCM();
}

s32 SND_fn_bTestInitSxdADPCM(void)
{
    return SND_fn_bTestInitSampleADPCM();
}

void SND_fn_vDesInitSxdADPCM(void)
{
    SND_fn_vDesInitSampleADPCM();
}

void SND_fn_vSetEffectSxdADPCM(void)
{
    SND_fn_vSetEffectSampleADPCM();
}

void func_00167620(void)
{
}

void func_00167628(void)
{
}

void func_00167630(void)
{
}

s32 SND_fn_lCreateNewBufferSxdADPCM(s32 voice, u16 extra0, u16 extra1, s32 extra2, _SoundParam* param3D, s32 extra3,
                  s32 extra4)
{
    SndParam2D param2D;

    if (g_bInitOK == 0) {
        return -1;
    }
    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_lCreateNewBufferSampleADPCM(voice, extra0, extra1, extra2, &param2D, extra3, extra4);
}

void SND_fn_vSetParamBufferSxdADPCM(void* micro, _SoundParam* param3D)
{
    SndParam2D param2D;

    SND_fn_vParam3Dto2D(param3D, &param2D);
    SND_fn_vSetParamBufferSampleADPCM(micro, &param2D);
}

void SND_fn_vDeleteBufferSxdADPCM(void)
{
    SND_fn_vDeleteBufferSampleADPCM();
}

void SND_fn_vPauseBufferSxdADPCM(void)
{
    SND_fn_vPauseBufferSampleADPCM();
}

void SND_fn_vResumeBufferSxdADPCM(void)
{
    SND_fn_vResumeBufferSampleADPCM();
}

s32 SND_fn_pvGetPtrBufferSxdADPCM(void)
{
    return SND_fn_pvGetPtrBufferSampleADPCM();
}

s32 SND_fn_rGetPosBufferSxdADPCM(void)
{
    return SND_fn_rGetPosBufferSampleADPCM();
}

s32 SND_fn_bLoadResBinarySxdADPCM(const SndAdpcmDiskResource* source, SND_tdstAdpcmBinRes* resource,
                                  s32 base)
{
    return SND_fn_bLoadResBinarySampleADPCM(source, resource, base);
}

s32 SND_fn_bLoadResBinaryAndConvertToWAVSxdADPCM(const SndAdpcmDiskResource* source,
                                                  SND_tdstAdpcmBinRes* resource, s32 base)
{
    return SND_fn_bLoadResBinaryAndConvertToWAVSampleADPCM();
}

void SND_fn_vUnLoadResSxdADPCM(SND_tdstAdpcmBinRes* resource)
{
    SND_fn_vUnLoadResSampleADPCM();
}

s32 SND_fn_bIsResLoadedSxdADPCM(void)
{
    return SND_fn_bIsResLoadedSampleADPCM();
}

s32 SND_fn_lPlayTransitionSxdADPCM(s32 resource, s32 extra0, AdpcmDataCallback callback, s32 argument,
                                    _SoundParam* param3D)
{
    SndParam2D param2D;

    if (g_bInitOK == 0) {
        return -1;
    }
    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_lPlayTransitionSampleADPCM(resource, extra0, callback, argument, &param2D);
}

#ifdef PC_PORT
s32 SND_fn_bSetParamTransitionSxdADPCM(s32 voice, const void* param3D)
{
    SndParam2D param2D;

    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_bSetParamTransitionSampleADPCM(voice, &param2D);
}
#else
s32 SND_fn_bSetParamTransitionSxdADPCM(s32 voice, _SoundParam* param3D)
{
    SndParam2D param2D;

    SND_fn_vParam3Dto2D(param3D, &param2D);
    return SND_fn_bSetParamTransitionSampleADPCM(voice, &param2D);
}
#endif

s32 SND_fn_bSetNextTransitionSxdADPCM(s32 voice, s32 size)
{
    return SND_fn_bSetNextTransitionSampleADPCM(voice, size);
}

s32 SND_fn_bDoTransitionWithFadeSxdADPCM(s32 voice, s32 size)
{
    return SND_fn_bDoTransitionWithFadeSampleADPCM(voice, size, ADPCM_FADE_DURATION);
}

s32 SND_fn_bDoTransitionWithFadeSxdADPCM2(s32 voice, s32 size, s32 duration)
{
    return SND_fn_bDoTransitionWithFadeSampleADPCM(voice, size, duration);
}

void SND_fn_vStopTransitionSxdADPCM(s32 voice)
{
    SND_fn_vStopTransitionSampleADPCM(voice);
}

void SND_fn_vPauseTransitionSxdADPCM(s32 voice)
{
    SND_fn_vPauseTransitionSampleADPCM(voice);
}

void SND_fn_vResumeTransitionSxdADPCM(s32 voice)
{
    SND_fn_vResumeTransitionSampleADPCM(voice);
}

s32 SND_fn_bCanFreeDataADPCM(void)
{
    return SND_fn_bCanFreeDataSampleADPCM();
}

void SND_fn_vReleaseDriverSxdADPCM(void)
{
    SND_fn_vReleaseDriverSampleADPCM();
}

void SND_fn_vRestoreDriverSxdADPCM(void)
{
    SND_fn_vRestoreDriverSampleADPCM();
}

void func_00167920(void)
{
    func_00168038();
}

void func_00167928(void)
{
    func_00168040();
}

s32 SND_fn_vGetTargetLabelSxdADPCM(void)
{
    return SND_fn_vGetTargetLabelSampleADPCM();
}

void SND_fn_vSynchroSxdADPCM(void)
{
    SND_fn_vSynchroSampleADPCM();
}

s32 SND_fn_vSetupTargetSxdADPCM(void)
{
    if (g_bInitOK == 0) {
        return -1;
    }
    return SND_fn_vSetupTargetSampleADPCM();
}

s32 SND_fn_bCanSetupTargetSxdADPCM(void)
{
    return SND_fn_bCanSetupTargetSampleADPCM();
}

s32 SND_fn_lCreateNewBufferExSxdADPCM(void)
{
    return SND_fn_lCreateNewBufferExSampleADPCM();
}

void SND_fn_vSetDefaultRollOffSxdADPCM(SndRollOff* rollOff)
{
}

s32 SND_fn_lGetPosBufferExSxdADPCM(void)
{
    return SND_fn_lGetPosBufferExSampleADPCM();
}

s32 SND_fn_lGetNbVoiceWishedSxdADPCM(void)
{
    return SND_fn_lGetNbVoiceWishedSampleADPCM();
}

s32 SND_fn_bIsLibActiveSxdADPCM(void)
{
    return 1;
}

s32 SND_fn_bSetResourceStaticVolumeSxdADPCM(s32 resource, s32 volume)
{
    return SND_fn_bSetResourceStaticVolumeSampleADPCM(resource, volume);
}

s32 fn_iTestStatusAdpcmEnv(void)
{
    return 1;
}

s32 SND_fn_lPlaySampleADPCM(s32 resource, SndParam2D* param, s32 extra0, SndEndCallback_inv callback,
                            s32 argument)
{
    u32 voice = fn_u32AllocateVoiceAdpcmEnv(resource, 0, param);

    if (voice != 0) {
        fn_iStartVoiceAdpcmEnv(voice, resource, 0, argument, callback);
    } else {
        voice = ADPCM_INVALID_VOICE;
    }
    return voice;
}

void SND_fn_vRemoveCallbackSampleADPCM(void)
{
}

s32 SND_fn_bSetParamSampleADPCM(s32 voice, SndParam2D* param)
{
    return fn_bSetParamAdpcmEnv(voice, param);
}

s32 SND_fn_bTestIsPlayingSampleADPCM(s32 voice)
{
    return fn_bIsPlayingVoiceAdpcmEnv(voice);
}

void SND_fn_vStopSampleADPCM(s32 voice)
{
    fn_u32DesallocateVoiceAdpcmEnv(voice);
}

void SND_fn_vPauseSampleADPCM(s32 voice)
{
    fn_vPauseAdpcmEnv(voice);
}

void SND_fn_vResumeSampleADPCM(s32 voice)
{
    fn_vResumeAdpcmEnv(voice);
}

s32 SND_fn_bIsResourceLoopingSampleADPCM(SND_tdstAdpcmBinRes* resource)
{
    u64 flags = (u8) resource->flags;
    u32 looping;

    R5900_MMI_SHIFT("dsrl", looping, flags, 7);
    return -looping;
}

s32 SND_fn_rGetPosSampleADPCM(s32 voice)
{
    return fn_rGetPosAdpcmEnv(voice);
}

s32 SND_fn_rGetLengthSampleADPCM(void)
{
    return -1;
}

void SND_fn_vSetSoundVolumeSampleADPCM(u8 volume)
{
}

s32 SND_fn_ucGetSoundVolumeSampleADPCM(void)
{
    return 0;
}

void SND_fn_vSetReverseStereoSampleADPCM(void)
{
}

s32 SND_fn_bGetReverseStereoSampleADPCM(void)
{
    return 0;
}

s32 SND_fn_iInitSampleADPCM(void)
{
    s32 result = -1;

    if (fn_iConstructAdpcmEnv() >= 0) {
        result = 3;
    }
    return result;
}

s32 SND_fn_bTestInitSampleADPCM(void)
{
    return fn_iTestStatusAdpcmEnv() != -1;
}

void SND_fn_vDesInitSampleADPCM(void)
{
    fn_iDestructAdpcmEnv();
}

void SND_fn_vSetEffectSampleADPCM(void)
{
}

void SND_fn_vCreateMicroSampleADPCM(void)
{
}

void SND_fn_vSetMicroParamSampleADPCM(void)
{
}

void SND_fn_vDestroyMicroSampleADPCM(void)
{
}

s32 SND_fn_lCreateNewBufferSampleADPCM(s32 voice, u16 extra0, u16 extra1, s32 extra2, SndParam2D* param,
                                 s32 extra3, s32 extra4)
{
    return -1;
}

void SND_fn_vSetParamBufferSampleADPCM(void* micro, SndParam2D* param)
{
}

void SND_fn_vDeleteBufferSampleADPCM(void)
{
}

void SND_fn_vPauseBufferSampleADPCM(void)
{
}

void SND_fn_vResumeBufferSampleADPCM(void)
{
}

s32 SND_fn_pvGetPtrBufferSampleADPCM(void)
{
    return 0;
}

s32 SND_fn_rGetPosBufferSampleADPCM(void)
{
    return -1;
}

s32 SND_fn_bLoadResScriptSampleADPCM(void)
{
    return 0;
}

static __inline__ u32 inv_SND_fn_ulCopyFlag8(u32 destination, u32 source)
{
    SND_AdpcmFlagBits destinationBits;
    SND_AdpcmFlagBits sourceBits;

    destinationBits.word = destination;
    sourceBits.word = source;
    destinationBits.unsignedBits.bInterleaved = sourceBits.signedBits.bInterleaved;
    return destinationBits.word;
}

static __inline__ void inv_SND_fn_vCopyHeader(SND_tdstAdpcmBinRes* destination,
                                  const SndAdpcmDiskHeader* header)
{
    destination->lFormat = header->lFormat;
    destination->cNbChannels = header->cNbChannels;
    destination->ucBitsPerSample = header->ucBitsPerSample;
    destination->uwRate = header->uwRate;
    destination->lVolume = header->lVolume;
    destination->lStartPos = header->lStartPos;
}

s32 SND_fn_bLoadResBinarySampleADPCM(const SndAdpcmDiskResource* source,
                                     SND_tdstAdpcmBinRes* resource, s32 base)
{
    s32 loaded;
    s32 kind;
    s32 relocatedWord;
    SND_AdpcmFlagBits sourceFlags;
    SND_AdpcmFlagBits destinationFlags;
    const s32* block;
    const s32* pair;
    s32* outputBlock;
    s32* outputPair;
    SND_tdstAdpcmBinRes converted;

    sourceFlags.word = source->header.flags;
    loaded = 0;
    if (sourceFlags.signedBits.bStream < 0) {
        if (sourceFlags.signedBits.bFromCD < 0) {
            block = source->block;
            outputBlock = converted.block;
            if (base != 0) {
                converted.block[0] = block[0];
                outputBlock[1] = block[1] + base;
                outputBlock[2] = block[2];
                outputBlock[3] = block[3];
            } else {
                converted.block[0] = 0;
                outputBlock[1] = 0;
                outputBlock[2] = 0;
                outputBlock[3] = 0;
            }
            loaded = ADPCM_RESOURCE_LOADED;
        } else {
            pair = source->block;
            outputPair = converted.block;
            kind = (s8) source->header.cNbChannels;
            if (base != 0) {
                relocatedWord = pair[1] + base;
                converted.block[0] = source->block[0];
                outputPair[1] = relocatedWord;
                if (kind == ADPCM_STEREO_PAIR) {
                    converted.block[2] = source->block[2];
                    converted.block[3] = source->block[3] + base;
                } else {
                    converted.block[2] = 0;
                    converted.block[3] = 0;
                }
            } else {
                converted.block[0] = 0;
                outputPair[1] = 0;
                converted.block[2] = 0;
                converted.block[3] = 0;
            }
            loaded = ADPCM_RESOURCE_LOADED;
        }
    } else if (sourceFlags.signedBits.bFromCD >= 0) {
        pair = source->block;
        outputPair = converted.block;
        kind = (s8) source->header.cNbChannels;
        if (base != 0) {
            relocatedWord = pair[1] + base;
            converted.block[0] = source->block[0];
            outputPair[1] = relocatedWord;
            if (kind == ADPCM_STEREO_PAIR) {
                converted.block[2] = source->block[2];
                converted.block[3] = source->block[3] + base;
            } else {
                converted.block[2] = 0;
                converted.block[3] = 0;
            }
        } else {
            converted.block[0] = 0;
            outputPair[1] = 0;
            converted.block[2] = 0;
            converted.block[3] = 0;
        }
        loaded = ADPCM_RESOURCE_LOADED;
    }
    if (loaded == ADPCM_RESOURCE_LOADED) {
        destinationFlags.word = converted.flags;
        destinationFlags.unsignedBits.bParamPitch = sourceFlags.unsignedBits.bParamPitch;
        destinationFlags.unsignedBits.bParamVolume = sourceFlags.unsignedBits.bParamVolume;
        destinationFlags.unsignedBits.bParamPan = sourceFlags.unsignedBits.bParamPan;
        destinationFlags.unsignedBits.b3 = sourceFlags.unsignedBits.b3;
        destinationFlags.unsignedBits.b4 = sourceFlags.unsignedBits.b4;
        destinationFlags.unsignedBits.bStream = sourceFlags.unsignedBits.bStream;
        destinationFlags.unsignedBits.bFromCD = sourceFlags.unsignedBits.bFromCD;
        destinationFlags.unsignedBits.bLooping = sourceFlags.unsignedBits.bLooping;
        destinationFlags.word = inv_SND_fn_ulCopyFlag8(destinationFlags.word, sourceFlags.word);
        converted.kind = loaded;
        converted.flags = destinationFlags.word;
        inv_SND_fn_vCopyHeader(&converted, &source->header);
        *resource = converted;
    }
    return loaded;
}

s32 SND_fn_bLoadResScriptAndConvertToWAVSampleADPCM(void)
{
    return 0;
}

s32 SND_fn_bLoadResBinaryAndConvertToWAVSampleADPCM(void)
{
    return 0;
}

void SND_fn_vUnLoadResSampleADPCM(void)
{
}

s32 SND_fn_bIsResLoadedSampleADPCM(void)
{
    return 0;
}

s32 SND_fn_lPlayTransitionSampleADPCM(s32 resource, s32 extra0, AdpcmDataCallback callback, s32 argument,
                                      SndParam2D* param)
{
    u32 voice = fn_u32AllocateVoiceAdpcmEnv(resource, extra0, param);

    if (voice != 0) {
        fn_iStartVoiceAdpcmEnv(voice, resource, extra0, argument, callback);
    } else {
        voice = ADPCM_INVALID_VOICE;
    }
    return voice;
}

s32 SND_fn_bSetParamTransitionSampleADPCM(s32 voice, SndParam2D* param)
{
    return fn_bSetParamAdpcmEnv(voice, param);
}

s32 SND_fn_bSetNextTransitionSampleADPCM(s32 voice, s32 size)
{
    return fn_u32PushNextBlockResourceMemAdpcmEnv(voice, 0, size) != 0;
}

s32 SND_fn_bDoTransitionWithFadeSampleADPCM(s32 voice, s32 size, s32 duration)
{
    return fn_u32PushNextBlockResourceMemAdpcmEnv(voice, ADPCM_FADE_BLOCK, size) != 0;
}

void SND_fn_vStopTransitionSampleADPCM(s32 voice)
{
    fn_u32DesallocateVoiceAdpcmEnv(voice);
}

void SND_fn_vPauseTransitionSampleADPCM(s32 voice)
{
    fn_vPauseAdpcmEnv(voice);
}

void SND_fn_vResumeTransitionSampleADPCM(s32 voice)
{
    fn_vResumeAdpcmEnv(voice);
}

void SND_fn_vSetStereoSampleADPCM(s32 stereo)
{
    fn_vSetStereoAdpcmEnv(stereo);
}

s32 SND_fn_bGetStereoSampleADPCM(void)
{
    return fn_vGetStereoAdpcmEnv();
}

void SND_fn_vSynchroSampleADPCM(void)
{
    fn_vSynchroSoundAdpcmEnv();
}

s32 SND_fn_bCanFreeDataSampleADPCM(void)
{
    return 0;
}

void SND_fn_vReleaseDriverSampleADPCM(void)
{
}

void SND_fn_vRestoreDriverSampleADPCM(void)
{
}

void func_00168038(void)
{
}

void func_00168040(void)
{
}

s32 SND_fn_vGetTargetLabelSampleADPCM(void)
{
    return 0;
}

s32 SND_fn_vSetupTargetSampleADPCM(void)
{
    return -1;
}

s32 SND_fn_bCanSetupTargetSampleADPCM(void)
{
    return -1;
}

s32 SND_fn_lCreateNewBufferExSampleADPCM(void)
{
    return -1;
}

s32 SND_fn_lGetPosBufferExSampleADPCM(void)
{
    return 0;
}

s32 SND_fn_lGetNbVoiceWishedSampleADPCM(void)
{
    return ADPCM_NUMBER_OF_VOICES;
}

s32 SND_fn_bSetResourceStaticVolumeSampleADPCM(s32 resource, u8 volume)
{
    return 0;
}
