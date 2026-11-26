#ifndef REVO_SND_SND_H
#define REVO_SND_SND_H

#include "ps2/types.h"

typedef struct {
    void (*pfnPause)(void*);
    s32 unk04[5];
} SND_TdstDriver;

extern SND_TdstDriver a_fnFunc[];

typedef struct RevoMSSoundAttenuation {
    s32 saturationDistance;
    s32 backgroundDistance;
} RevoMSSoundAttenuation;

typedef struct FillMicroParam {
    s32 id;
    s32 link;
} FillMicroParam;

typedef struct SndFixedVector3 {
    s32 x;
    s32 y;
    s32 z;
} SndFixedVector3;

typedef struct SxdMicroParams_inv {
    SndFixedVector3 position;
    SndFixedVector3 velocity;
    SndFixedVector3 inv_stFrontAxis;
    SndFixedVector3 inv_stRightAxis;
} SxdMicroParams_inv;

typedef struct SxdMicro {
    SndFixedVector3 position;
    SndFixedVector3 velocity;
    SndFixedVector3 inv_stFrontAxis;
    SndFixedVector3 inv_stRightAxis;
    s32 inactive : 1;
    struct SxdMicro* next;
    struct SxdMicro* prev;
    struct SxdMicroList_inv* owner;
} SxdMicro;

typedef struct SxdMicroList_inv {
    SxdMicro* head;
    SxdMicro* tail;
    s32 count;
} SxdMicroList_inv;

extern SxdMicroList_inv micros_sxd;

typedef struct SxdMicroLink {
    SxdMicro* micro;
    s32 enabled;
} SxdMicroLink;

typedef struct SndRollOff {
    s32 nearDistance;
    s32 farDistance;
} SndRollOff;

typedef union SxdMicroSlot_inv {
    FillMicroParam fill;
    SxdMicroLink link;
    SndRollOff rollOff;
} SxdMicroSlot_inv;

typedef struct _SoundParam {
    s32 pitch;
    u8 vol;
    u8 pan;
    u8 dolby;
    u8 reverb;
    SndFixedVector3 position;
    SndFixedVector3 velocity;
    s32 flags;
    s32 microCount;
    SxdMicroSlot_inv micros[1];
} _SoundParam;

typedef struct gu_tdstMicroSoundNode gu_tdstMicroSoundNode;

typedef struct gu_tdstMicroSoundHead {
    gu_tdstMicroSoundNode* head;
    gu_tdstMicroSoundNode* tail;
    s32 count;
} gu_tdstMicroSoundHead;

struct gu_tdstMicroSoundNode {
    s32 id;
    struct gu_tdstMicroSoundNode* next;
    struct gu_tdstMicroSoundNode* prev;
    gu_tdstMicroSoundHead* volatile owner;
};

#define ADPCM_VOICE_COUNT 48
#define ADPCM_COMMAND_BUFFER_SIZE 0x400

typedef struct SndParam2D {
    s32 pitch;
    u8 volume;
    u8 pan;
    u8 dolby;
    u8 reverb;
} SndParam2D;

typedef void (*AdpcmDataCallback)(s32 argument);

typedef struct AdpcmDataFlags {
    signed int allocated : 1;
    signed int started : 1;
    signed int playing : 1;
    signed int blockRequested : 1;
    signed int paused : 1;
    signed int lastBlock : 1;
    signed int stopping : 1;
} AdpcmDataFlags;

typedef struct AdpcmBlockInfo {
    s32 words[4];
} AdpcmBlockInfo;

typedef struct AdpcmFormatFlags {
    unsigned int bParamPitch : 1;
    unsigned int bParamVolume : 1;
    unsigned int bParamPan : 1;
    unsigned int b3 : 1;
    unsigned int b4 : 1;
    signed int bStream : 1;
    signed int bFromCD : 1;
    unsigned int bLooping : 1;
    signed int bInterleaved : 1;
} AdpcmFormatFlags;

typedef struct AdpcmDescriptor {
    AdpcmFormatFlags flags;
    u32 lFormat;
    u8 cNbChannels;
    u8 ucBitsPerSample;
    u16 uwRate;
    u32 lVolume;
    u32 lStartPos;
} AdpcmDescriptor;

typedef struct AdpcmResource {
    u8 unk00[4];
    s32 status;
    u8 unk08[0xC];
    AdpcmDescriptor descriptor;
    AdpcmBlockInfo block;
} AdpcmResource;

typedef struct AdpcmData {
    AdpcmDataFlags flags;
    s32 active;
    s32 sema;
    s32 remoteHandle0;
    s32 remoteHandle1;
    u32 voiceHandle;
    AdpcmResource* environment;
    AdpcmResource* resource;
    SndParam2D param;
    u32 callbackArgument;
    AdpcmDataCallback callback;
} AdpcmData;

typedef struct AdpcmManager {
    s32 constructed : 1;
    s32 unk00 : 31;
    s32 commandCount;
    s32 bytesRemaining;
    u8* writeCursor;
    AdpcmData voices[ADPCM_VOICE_COUNT];
    u8 commandBuffer[ADPCM_COMMAND_BUFFER_SIZE];
} AdpcmManager;

typedef s32 (*AdpcmVoiceCallback)(AdpcmData* voice, void* context);

AdpcmData* fn_pstIsValidVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
AdpcmData* fn_pstParseTdstAdpcmMngr(AdpcmManager* manager, void* context,
                                     AdpcmVoiceCallback callback);
s32 fn_iRemoteCall(s32 rpc_id, s32 mode, void* out_buf, s32 out_size, void* in_buf, s32 in_size);

extern union SndBinResource* SND_fn_pGetBinRes(u32 reference);
extern void* SND_fn_pGetBinEvent(u32 ref);
extern s32 SND_fn_bStartLoadBanks2(s32 slot);
extern s32 fn_iRemoteEEDesinit(void);
extern s32 fn_iRemoteEEInit(void);
extern void SND_fn_vDesInitBankSnd(void);
extern s32 SND_fn_bEndLoadBanks(void);
extern s32 SND_fn_bInitBank2(s32 bankId, s32 slot);
extern s32 SND_fn_bUnLoadBinaryBankAndData(s32 bankId);

extern void SND_fn_vEnterCriticalSectionThreadSnd_(void);
extern void SND_fn_vQuitCriticalSectionThreadSnd(void);
extern s32 func_0015F7F8(void);
extern void SND_fn_vDesInitThreadSnd(void);
extern void SND_fn_vMouchardThreadsnd(char* file, s32 line);

struct SndFixedVector3;
extern s32 SND_fn_rScalaireVectorSnd(const struct SndFixedVector3* left,
                                     const struct SndFixedVector3* right);
extern s32 SND_fn_rPseudoScalaireNormeVectorSnd(const struct SndFixedVector3* left,
                                       const struct SndFixedVector3* right);
extern s32 SND_fn_rPseudoNormeVectorSnd(const struct SndFixedVector3* vector);

struct VLNode;
extern s32 SND_fn_lSendRequestSound(s32 a0, s32 a1, s32 a2, s32 a3, void (*a4)(s32 parameter));
extern void SND_fn_vSetDopplerFactorSound(s32 factor);
extern void fnSetLastEventForSoundObjectType(s32 typeIndex, s32 event);
extern s32 SND_fn_iTestInitSound(void);
extern void SND_fn_vChangeVolumeSound(s32 object, s32 typeIndex, s32 volume);
extern void SND_fn_vKillAllObjectSound(void);
extern void SND_fn_vPauseSound(void);
extern void SND_fn_vRestoreVolumeLines(void);
extern void SND_fn_vResumeSound(void);
extern void SND_fn_vSynchroSound(void);
extern void stop_son_all(void);
extern void SND_fn_vSetMusicFadeOut(float value);
extern s32 SND_fn_iDesInitSound(void);
extern s32 SND_fn_lGetVolumeLineSound(s32* line);
extern void SND_fn_vDestroyMicroSound(s32* node);
extern void SND_fn_vKillObjectSound2(s32 object, s32 typeIndex);
extern void SND_fn_vSetVolumeLineSound(s32* line, s32 volume);
extern s32 SND_fn_iInitSound(void* parameters);
extern s32 SND_fn_lAddObjectTypeSound2(void* callbacks, struct VLNode* volumeLine);
struct HIE_tdstSuperObject_;
typedef void (*SndObjectInfoCallback_inv)(struct HIE_tdstSuperObject_* object, char* text, s32 size);
typedef s32 (*SndRollOffCallback_inv)(struct HIE_tdstSuperObject_* object, u32* distances);
extern void SND_fn_vSetInfoForTypeSound(s32 typeIndex, SndObjectInfoCallback_inv callback);
extern void SND_fn_vSetRollOffFunctionForTypeSound(s32 typeIndex, SndRollOffCallback_inv callback);
extern s32 SND_fn_bGetStereoSound(void *a0);
extern s32 SND_fn_bIsMusicPlayingSound(void *a0);
extern void j_j_nullsub_103(u8 a0);
extern void SND_fn_vGetTargetLabelSound(char* name, s32 size);
extern void j_j_j_j_nullsub_22(s32 a0);

typedef struct _SND_tdstBlockEvent _SND_tdstBlockEvent;

typedef union _SND_tduRefEvt {
    s32 lIndex;
    _SND_tdstBlockEvent* pstEvent;
} _SND_tduRefEvt;

typedef struct SND_tdstDeTune_inv {
    s32 rDelta;
} SND_tdstDeTune_inv;

typedef union SND_tduBlockEventParams_inv {
    struct {
        union SndBinResource* uResource;
        SND_tdstDeTune_inv stDeTune;
    } stPlay;
    struct {
        _SND_tduRefEvt uEvt;
    } stStop;
    struct {
        _SND_tduRefEvt uEvtStop;
        _SND_tduRefEvt uEvtGo;
    } stStopNGo;
    struct {
        _SND_tduRefEvt uEvtStop;
        _SND_tduRefEvt uEvtGo;
        s32 iTime;
    } stStopNGoFade;
    struct {
        union SndBinResource* uResource;
        s32 CoefA;
        s32 CoefB;
        SND_tdstDeTune_inv stDeTune;
    } stExtraParam;
    struct {
        union SndBinResource* uResource;
        s32 rtCoefPitch;
        s32 rtCoefVol;
        s32 rtCoefPan;
    } stExtraAll;
    struct {
        s32 iIdEffect;
    } stEffect;
    struct {
        s32 bConcernWithObjectType;
        s32 lNewVolume;
    } stChangeVolume;
} SND_tduBlockEventParams_inv;

struct _SND_tdstBlockEvent {
    s32 Id;
    s32 eType;
    SND_tduBlockEventParams_inv uParams;
    s32 bStopable : 1;
    s32 bDynamic : 1;
};

typedef struct SndResourceHeaderView {
    u32 unk00;
    s32 type;
    u32 unk08;
    u8 staticVolume;
} SndResourceHeaderView;

typedef struct SndSampleFlags {
    u32 bParamPitch : 1;
    u32 bParamVolume : 1;
    u32 bParamPan : 1;
    u32 b3 : 1;
    u32 b4 : 1;
    u32 bStream : 1;
    u32 bFromCD : 1;
    u32 bLooping : 1;
} SndSampleFlags;

typedef struct SndBinResourceHeader {
    u32 unk00;
    u32 kind;
    u32 unk08[4];
    SndSampleFlags sampleFlags;
    u32 unk1C;
    u32 unk20[7];
} SndBinResourceHeader;

typedef struct SndSequenceElement {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    union {
        s32 id;
        union SndBinResource* data;
    } resource;
} SndSequenceElement;

typedef struct SndSequenceDisk {
    u32 unk00;
    u32 type;
    u32 unk08;
    u8 unk0C[4];
    s32 elementsOffset;
    u8 staticVolume;
    u8 unk15[3];
    s32 looping : 1;
    s32 : 31;
    u32 loopStart;
    u32 loopEnd;
    u32 loopCount;
    s32 elementCount;
} SndSequenceDisk;

typedef struct SndSequence {
    u32 unk00;
    u32 type;
    u32 unk08;
    u8 staticVolume;
    u32 unk10;
    s32 looping;
    u32 loopStart;
    u32 loopEnd;
    u32 loopCount;
    u32 elementCount;
    SndSequenceElement* elements;
    s32 loadedElementCount;
} SndSequence;

typedef struct SndSwitchResource SndSwitchResource;

typedef union SwitchResourceFlags {
    u32 word;

    struct {
        s32 looping : 1;
    } bits;
} SwitchResourceFlags;

typedef union SwitchChildTarget {
    union SndBinResource* resource;
    s32 id;
} SwitchChildTarget;

typedef struct SwitchChild {
    SwitchChildTarget target;
    s32 key;
} SwitchChild;

typedef struct SwitchChildren {
    SwitchChild* entries;
    s32 count;
} SwitchChildren;

struct SndSwitchResource {
    u32 unk00;
    s32 type;
    u32 unk08;
    u8 volume;
    s32 unk10;
    SwitchChildren children;
    s32 childCount;
    u32 inv_ulInfoType;
    s32 defaultKey;
    SwitchResourceFlags flags;
};

typedef struct SndRandomResource SndRandomResource;

typedef union RandomChoiceTarget {
    union SndBinResource* resource;
    s32 id;
} RandomChoiceTarget;

typedef struct RandomChoice {
    RandomChoiceTarget target;
    s32 weight;
    s32 repeatable;
} RandomChoice;

typedef struct RandomDesc {
    s32 unk00;
    s32 silenceWeight;
    s32 silenceRepeat;
    s32 chosenIndex;
    s32 previousIndex;
    RandomChoice* choices;
    s32 choiceCount;
} RandomDesc;

struct SndRandomResource {
    u32 unk00;
    s32 type;
    u32 unk08;
    s8 volume;
    s32 unk10;
    RandomDesc desc;
};

typedef union SndThemeResource {
    s32 id;
    union SndBinResource* data;
} SndThemeResource;

typedef struct SndThemePart {
    SndThemeResource resource;
    s32 loopStart : 1;
    s32 loopEnd : 1;
    s32 loopCount;
    s32 inv_lTransitionType;
    s32 unk10;
} SndThemePart;

typedef struct SndThemeDisk {
    u32 header[3];
    u8 unk0C[4];
    u32 partsOffset;
    u8 parameter;
    u32 repeatStart;
    u8 unk1C[4];
    u32 repeatCount;
    s32 partCount;
    s32 optionalPartOffset;
    u32 word2C;
} SndThemeDisk;

typedef struct SndThemeSequence {
    u32 repeatStart;
    u8 unk04[4];
    s32 partCount;
    SndThemePart* optionalPart;
    u32 word10;
    u32 repeatCount;
    SndThemePart* parts;
    s32 copiedPartCount;
} SndThemeSequence;

typedef struct SndTheme {
    u32 header[3];
    u8 parameter;
    u32 state;
    SndThemeSequence sequence;
} SndTheme;

typedef struct SndSampleDisk {
    s32 unk00;
    s32 type;
    s32 unk08;
    u8 unk0C[8];
    u8 staticVolume;
    SndSampleFlags flags;
    u8 unk1C[4];
    u8 cNbChannels;
    u8 ucBitsPerSample;
    u16 uwRate;
} SndSampleDisk;

typedef struct SndSample {
    s32 unk00;
    s32 type;
    s32 unk08;
    u8 staticVolume;
    s32 unk10;
    SndSampleFlags flags;
    s32 lFormat;
    u8 cNbChannels;
    u8 ucBitsPerSample;
    u16 uwRate;
} SndSample;

typedef struct SND_tdstAdpcmBinRes {
    s32 unk00;
    s32 kind;
    u8 pad08[0x0C];
    s32 flags;
    s32 lFormat;
    u8 cNbChannels;
    u8 ucBitsPerSample;
    u16 uwRate;
    s32 lVolume;
    s32 lStartPos;
    s32 block[4];
    u8 pad38[0x0C];
} SND_tdstAdpcmBinRes;

typedef struct SndAdpcmDiskHeader {
    u32 flags;
    s32 lFormat;
    u8 cNbChannels;
    u8 ucBitsPerSample;
    u16 uwRate;
    s32 lVolume;
    s32 lStartPos;
} SndAdpcmDiskHeader;

typedef struct SndAdpcmDiskResource {
    u8 unk00[0x18];
    SndAdpcmDiskHeader header;
    s32 block[4];
} SndAdpcmDiskResource;

typedef struct SxdBinaryResourceView {
    s32 unk00[7];
    s32 type;
    s8 cNbChannels;
} SxdBinaryResourceView;

typedef struct SoundNodeControlBits {
    signed int bParamPitch : 1;
    signed int bParamVolume : 1;
    signed int bParamPan : 1;
    signed int b3 : 1;
    signed int b4 : 1;
    signed int bStream : 1;
    signed int bFromCD : 1;
    signed int bLooping : 1;
} SoundNodeControlBits;

typedef struct SxdResourceView {
    u32 unk00[5];
    SoundNodeControlBits flags;
    s32 lFormat;
} SxdResourceView;

typedef struct TransitionSource {
    u8 unk00[0x14];
    u32 unk14 : 5;
    s32 positional : 1;
    u32 unk14_6 : 26;
    u8 unk18[4];
    s8 channels;
    s8 ucBitsPerSample;
    u16 rate;
} TransitionSource;

typedef void (*TransitionCallbackFn)(void* context);

typedef union SndBinResource {
    SndBinResourceHeader header;
    SndResourceHeaderView head;
    SndSequence seq;
    SndSwitchResource swtch;
    SndRandomResource random;
    SndTheme theme;
    SxdResourceView sxd;
    SndSample sample;
    SND_tdstAdpcmBinRes adpcm;
    TransitionSource transition;
    u32 words[0x44 / 4];
} SndBinResource;

typedef void (*SndEndCallback_inv)(s32 parameter);

typedef struct SndRandomDisk {
    u32 unk00;
    s32 type;
    u32 unk08;
    u32 unk0C;
    s32 choiceOffset;
    u8 volume;
    s32 choiceCount;
    s32 silenceWeight;
    s32 silenceRepeat;
} SndRandomDisk;

typedef struct SndSwitchDisk {
    u32 unk00;
    s32 type;
    u32 unk08;
    u32 unk0C;
    s32 childOffset;
    u8 volume;
    u8 pad15[0x0B];
    s32 childCount;
    u32 inv_ulInfoType;
    s32 defaultKey;
    u32 flags;
} SndSwitchDisk;

typedef union SndBinResourceDisk_inv {
    SndBinResourceHeader header;
    SndSequenceDisk seq;
    SndSwitchDisk swtch;
    SndRandomDisk random;
    SndThemeDisk theme;
    SxdBinaryResourceView sxd;
    SndSampleDisk sample;
    SndAdpcmDiskResource adpcm;
} SndBinResourceDisk_inv;

typedef struct SndOwnerState {
    s32 active;
} SndOwnerState;

typedef struct PlaySnd {
    s32 soundHandle;
    SndBinResource* resource;
    union {
        s32 flags;
        struct {
            int bit0 : 1;
            int bit1 : 1;
            int bit2 : 1;
            int pad_bits : 29;
        } bits;
    } state;
    s32 inUse;
    SndOwnerState* owner;
    s32 pauseInstance;
} PlaySnd;

typedef PlaySnd PlaySndSlot;

typedef s32 TransitionHandle;

extern s32 SND_fn_bSetParamTransitionSxdWave(TransitionHandle handle, _SoundParam* parameters);
extern void SND_fn_vStopTransitionSxdWave(TransitionHandle handle);
extern s32 SND_fn_bSetParamTransitionSxdADPCM(s32 voice, _SoundParam* param3D);

extern void SND_fn_vSetDefaultRollOff(SndRollOff* rollOff);
extern u8 SND_fn_ucDistanceToVolumeEx(s32 distance, SndRollOff* rollOff);
extern u8 SND_fn_ucPositionToPan(const SndFixedVector3* position, const SndFixedVector3* reference,
                                 const SndFixedVector3* unused, const SndFixedVector3* direction);
extern u8 SND_fn_ucPositionToDolby(const SndFixedVector3* position, const SndFixedVector3* reference,
                                   const SndFixedVector3* direction);

extern void SND_fn_vParam3Dto2D(_SoundParam* sound, SndParam2D* out);

typedef struct SndSxdBufferDesc_inv {
    s32 lFormat;
    s32 unk04;
    u16 uwBitsPerSample;
    u16 uwNbChannels;
    s32 rate;
} SndSxdBufferDesc_inv;

typedef struct SndSxdBufferFormat_inv {
    s32 type;
    void* callback;
    s32 arg;
} SndSxdBufferFormat_inv;

typedef struct TransitionPushArg {
    s32 size;
    void* data;
} TransitionPushArg;

extern s32 SND_fn_lCreateNewBufferExSxd(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                        _SoundParam* param3D, void* user);
extern s32 SND_fn_lPushBufferExSxd(s32 lBuffer, TransitionPushArg* data);
extern void SND_fn_vSetParamBufferSxd(s32 lBuffer, _SoundParam* param3D);

typedef s32 (*SND_tfnRetObjectSoundValue)();
typedef void (*SND_tfnRetObjectSoundExtraCoef)(s32 obj, _SND_tduRefEvt uRef, s32* pPitch, s32* pPan,
                                               s32* pVolume);
typedef void (*SND_tfnRetObjectSoundVector)();

typedef struct SND_tdstTypeObjectSoundInfos {
    SND_tfnRetObjectSoundValue callback00;
    SND_tfnRetObjectSoundValue callback04;
    SND_tfnRetObjectSoundValue callback08;
    SND_tfnRetObjectSoundValue callback0C;
    SND_tfnRetObjectSoundValue callback10;
    SND_tfnRetObjectSoundValue callback14;
    SND_tfnRetObjectSoundValue retReverb;
    SND_tfnRetObjectSoundExtraCoef retExtraCoef;
    SND_tfnRetObjectSoundValue retSwitch;
    SND_tfnRetObjectSoundValue retMicroLink;
    SND_tfnRetObjectSoundVector retVectorI;
    SND_tfnRetObjectSoundVector retVectorJ;
    SND_tfnRetObjectSoundVector retVectorK;
} SND_tdstTypeObjectSoundInfos;

extern SND_tdstTypeObjectSoundInfos g_astTypeObjectSoundInfosDefault;

typedef s32 (*SND_tfnMicroParamValue)(s32 obj);

typedef struct _tdstAllRetMicroSound {
    SND_tfnMicroParamValue X;
    SND_tfnMicroParamValue Y;
    SND_tfnMicroParamValue Z;
    SND_tfnMicroParamValue VX;
    SND_tfnMicroParamValue VY;
    SND_tfnMicroParamValue VZ;
    SND_tfnMicroParamValue DNX;
    SND_tfnMicroParamValue DNY;
    SND_tfnMicroParamValue DNZ;
    SND_tfnMicroParamValue DTX;
    SND_tfnMicroParamValue DTY;
    SND_tfnMicroParamValue DTZ;
    SND_tfnMicroParamValue Filtre;
} _tdstAllRetMicroSound;

extern _tdstAllRetMicroSound g_st_pfnRetMicroSound;
extern void SND_fn_vSetRetMicroSound(_tdstAllRetMicroSound* src);

typedef struct SND_tstObjectSoundInfoBlock {
    u8 bytes[0x20];
} SND_tstObjectSoundInfoBlock;

typedef struct SndSxdSample SndSxdSample;

typedef struct SndSxdSampleList {
    SndSxdSample* first;
    SndSxdSample* last;
    s32 count;
} SndSxdSampleList;

struct SndSxdSample {
    s32 resource;
    s32 handle;
    s32 type;
    s32 bStream;
    SndSxdSample* next;
    SndSxdSample* previous;
    SndSxdSampleList* list;
};

extern s32 SND_fn_rMulRealRealQuickSnd(s32 a, s32 b);
extern s32 SND_fn_rDivRealRealQuickSnd(s32 a, s32 b);

s32 SND_fn_rGetRandomSnd(s32 range);
void SND_fn_vSndRealTwinToSndReal(s32 twinParam, s32* pOut1, s32* pOut2);

typedef s32 (*BACKCallback)(s32 arg0, s32 arg1);

typedef struct tdstBACKCall {
    s32 args[2];
    BACKCallback callback;
} tdstBACKCall;

typedef struct tdstBACKData {
    s32 flags;
    tdstBACKCall call;
} tdstBACKData;

typedef s32 (*SndCmdHandler)(s32* output, s32 outputSize, s32* input, s32 inputSize);

typedef u8* (*SndCommandHandler_inv)(u32 command, u8* message, s32 messageSize);
extern u8* func_001656B0(u32 command, u8* message, s32 messageSize);

typedef struct SndRpcEndpoint_inv {
    u32 rpcId;
    SndCommandHandler_inv handler;
} SndRpcEndpoint_inv;
extern s32 fn_iLoadFileEE(s32 file, void** output);

typedef struct SndCmdModule {
    u32 tag;
    u32 id;
    s32 lastCommand;
    SndCmdHandler* handlers;
} SndCmdModule;

void SND_fn_vKillAllObjectTypeSound(s32 typeIndex);

extern void* SND_RES_FANTOME[1];

extern void SND_fn_vInitMallocSnd(void);
extern void SND_fn_vDesInitMallocSnd(void);
extern void SND_fn_vFreeSnd(void* block);
extern void* SND_fn_pvMallocSnd(s32 size);
extern void* SND_fn_pvReAllocSndEx(s32 unused, void* block, s32 size);

extern void SND_fn_vInitThreadSnd(void);

extern s32 SND_fn_bDoTransitionWithFadeSxdADPCM2(s32 voice, s32 size, s32 duration);
extern s32 SND_fn_bDoTransitionWithFadeSxdADPCM(s32 voice, s32 size);
extern s32 SND_fn_bIsLibActiveSxdADPCM(void);
extern s32 SND_fn_bIsResLoadedSxdADPCM(void);
extern s32 SND_fn_bSetNextTransitionSxdADPCM(s32 voice, s32 size);
extern s32 SND_fn_bTestIsPlayingSxdADPCM(s32 voice);
extern s32 SND_fn_lGetPosBufferExSxdADPCM(void);
extern s32 SND_fn_lPlaySxdADPCM(s32 resource, _SoundParam* param3D, s32 extra0, SndEndCallback_inv callback,
                                s32 argument);
extern s32 SND_fn_rGetPosSxdADPCM(s32 voice);
extern void SND_fn_vDesInitSxdADPCM(void);
extern void SND_fn_vPauseSxdADPCM(s32 voice);
extern void SND_fn_vPauseTransitionSxdADPCM(s32 voice);
extern void SND_fn_vReleaseDriverSxdADPCM(void);
extern void SND_fn_vRestoreDriverSxdADPCM(void);
extern void SND_fn_vResumeSxdADPCM(s32 voice);
extern void SND_fn_vSetReverseStereoSxdADPCM(void);
extern void SND_fn_vStopSxdADPCM(s32 voice);
extern void SND_fn_vStopTransitionSxdADPCM(s32 voice);
extern void SND_fn_vSynchroSxdADPCM(void);
extern s32 SND_fn_bSetParamSxdADPCM(s32 voice, _SoundParam* param3D);
extern void SND_fn_vSetDefaultRollOffSxdADPCM(SndRollOff* rollOff);
extern s32 SND_fn_bSetResourceStaticVolumeSxdADPCM(s32 resource, s32 volume);
extern s32 SND_fn_lPlayTransitionSxdADPCM(s32 resource, s32 extra0, AdpcmDataCallback callback, s32 argument,
                                          _SoundParam* param3D);
extern s32 SND_fn_lPlaySxd(SndBinResource* resource, _SoundParam* params, s32 a2, SndEndCallback_inv callback,
                           s32 argument);
extern s32 SND_fn_lPlayTransitionExSxd(SndBinResource* first, SndBinResource* next, void* callback, void* player,
                                       void* params);
extern void SND_fn_vResumeTransitionSxdADPCM(s32 voice);
extern s32 SND_fn_rGetLengthSxdADPCM(s32 a0);
extern s32 SND_fn_iInitSxdADPCM(void *a0);
extern void SND_fn_vUnLoadResSxdADPCM(SND_tdstAdpcmBinRes* resource);
extern void SND_fn_vRemoveCallbackSxdADPCM(void *a0);

extern s32 SND_fn_lCreateNewBufferExSxdWave(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                            _SoundParam* param3D, void* user);
extern s32 SND_fn_b3DOneShootShouldBeConvertedSxdWave(void);
extern s32 SND_fn_bCheckVersionResourceSxdWave(void);
extern s32 SND_fn_bGetReverseStereoSxdWave(void);
extern s32 SND_fn_bTestInitSxdWave(void);
extern s32 SND_fn_lGetNbVoiceOptimumSxdWave(void);
extern s32 SND_fn_lGetNbVoiceWishedSxdWave(void);
extern s32 SND_fn_ucGetSoundVolumeSxdWave(void);
extern s32 SND_fn_bCanSetupTargetSxdWave(void);
extern s32 SND_fn_bIsResLoadedSxdWave(void);
extern void SND_fn_vDesInitSxdWave(void);
extern void SND_fn_vForceReleaseDriverSxdWave(void);
extern void SND_fn_vForceRestoreDriverSxdWave(void);
extern void SND_fn_vGetTargetLabelSxdWave(char* name, s32 size);
extern void SND_fn_vGetTargetLabelSample(char* name, s32 size);
extern void SND_fn_vReleaseDriverSxdWave(void);
extern void SND_fn_vRestoreDriverSxdWave(void);
extern void SND_fn_vSetDopplerFactorSxdWave(void);
extern void SND_fn_vSetMicroParamSxdWave(void);
extern void SND_fn_vSetupTargetSxdWave(void);
extern void SND_fn_vConvertResDiskToMemSxdWave(void);
extern void j_nullsub_22(void);
extern void SND_fn_vSynchroSxdWave(void);
extern void loc_165520(s32 a0);
extern void SND_fn_vDestroyMicroSxdWave(SxdMicro* micro);
extern void SND_fn_vCreateMicroSxdWave(SxdMicroParams_inv* params, SxdMicro* micro);
extern void SND_fn_vRemoveCallbackSxdWave(void* callback);
extern s32 SND_fn_bSetParamSxdWave(void* sound, _SoundParam* param3D);
extern s32 SND_fn_bSetResourceStaticVolumeSxdWave(s32 resource, s32 volume);
extern void SND_fn_vDeleteBufferSxdWave(s32 a0);
extern s32 SND_fn_iInitSxdWave(void *a0);
extern s32 SND_fn_rGetLengthSxdWave(s32 a0);
extern s32 SND_fn_rGetPosSxdWave(s32 a0);
extern void SND_fn_vSetDefaultRollOffSxdWave(SndRollOff* rollOff);
extern void j_nullsub_76(s32 a0);
extern void SND_fn_vPauseSxdWave(s32 a0);
extern void SND_fn_vResumeSxdWave(s32 a0);
extern void loc_165220(SndSample* sample);
extern void loc_165368(void *a0);
extern void loc_165370(void *a0);
extern void SND_fn_vSetParamBufferSxdWave(void* buffer, _SoundParam* param3D);

extern void SND_fn_vFreeSndEx(s32 unused, void* block);
extern void* SND_fn_pvMallocSndEx(s32 unused, s32 size);

extern s32 SND_fn_lPlaySxdWave(void* resource, _SoundParam* param3D, s32 a2, SndEndCallback_inv callback, s32 argument);

extern s32 fn_iInitBACKEnv(void);
extern s32 fn_iGetEEAdpcmInterface(SndCmdModule* module);
extern s32 fn_u32AllocateVoiceBACKEnv(tdstBACKCall* source);
extern s32 fn_u32DesallocateVoiceBACKEnv(s32 voice);

extern s32 fn_iConstructTdstBACKData(tdstBACKData* data);
extern s32 fn_iOperateCallTdstBACKData(tdstBACKData* data);
extern s32 fn_u32DesallocateTdstBACKData(tdstBACKData* data);
extern tdstBACKData* fn_u32AllocateTdstBACKData(tdstBACKData* data, tdstBACKCall* source);

extern s32 fn_iInitEECmdModule(void);

extern void inv_FreeIopHeap(s32 type, void* block);

extern s32 SND_fn_bCanSetupTargetSample(void);
extern s32 SND_fn_bGetReverseSample(void);
extern s32 SND_fn_bIsResLoadedSample(void);
extern s32 SND_fn_bTestInitSample(void);
extern s32 SND_fn_iInitSample(void);
extern s32 SND_fn_lGetNbVoiceOptimumSample(void);
extern s32 SND_fn_lGetNbVoiceWishedSample(void);
extern s32 SND_fn_lPushBufferExSample(void);
extern s32 SND_fn_rGetLengthSample(void);
extern s32 SND_fn_rGetPosSample(void);
extern s32 SND_fn_ucGetSoundVolumeSample(void);
extern void SND_fn_vConvertResDiskToMemSample(void);
extern void SND_fn_vDesInitSample(void);
extern void SND_fn_vForceReleaseDriverSample(void);
extern void SND_fn_vForceRestoreDriverSample(void);
extern void SND_fn_vPauseBufferSample(void);
extern void SND_fn_vPauseSample(void);
extern void SND_fn_vReleaseDriverSample(void);
extern void SND_fn_vRestoreDriverSample(void);
extern void SND_fn_vResumeBufferSample(void);
extern void SND_fn_vResumeSample(void);
extern void SND_fn_vSetEffectSample(void);
extern void SND_fn_vSetNbVoiceWishedSample(void);
extern void SND_fn_vSetReverseSample(void);
extern void SND_fn_vSetSoundVolumeSample(s32 unused);
extern void SND_fn_vSetupTargetSample(void);
extern void SND_fn_vStopSample(void);
extern void SND_fn_vSynchroSample(void);
extern void SND_fn_vUnLoadResSample(void);
extern s32 SND_fn_bSetResourceStaticVolumeSample(s32 resource, s32 volume);
extern s32 SND_fn_lCreateNewBufferExSample(void* a0, void* a1, void* params, void* a3);
extern s32 SND_fn_bSetParamSample(void* sound, SndParam2D* param);
extern s32 SND_fn_lPlaySample(void* resource, SndParam2D* param, s32 a2, SndEndCallback_inv callback, s32 argument);
extern void SND_fn_vSetParamBufferSample(void* buffer, const SndParam2D* param);
extern s32 SND_fn_bTestIsPlayingSample(s32 handle);
extern s32 SND_fn_bTestIsPlayingSxdWave(s32 handle);
extern s32 SND_fn_lCreateNewBufferSample(void *a0, u16 a1, u16 a2, void *a3, void *a4, s32 a5, s32 a6);
extern void SND_fn_vDeleteBufferSample(s32 a0);
extern void* SND_fn_pvGetPtrBufferSample(void* buffer);
extern void* SND_fn_pvGetPtrBufferSxdWave(void* buffer);
extern s32 SND_fn_rGetPosBufferSample(void* buffer);
extern s32 SND_fn_rGetPosBufferSxdWave(void* buffer);
extern s32 SND_fn_lGetPosBufferExSample(void* buffer);
extern s32 SND_fn_lGetPosBufferExSxdWave(void* buffer);
extern s32 SND_fn_bIsResourceLoopingSample(SndSample* sample);
extern s32 SND_fn_bLoadResBinarySample(SndSampleDisk* source, SndSample* sample);

extern s32 fn_iRecover(s32 threadId);
extern s32 func_00165D18(s32* threads, const SndRpcEndpoint_inv* server,
                         const SndRpcEndpoint_inv* client);

extern s32 fn_bIsPlayingVoiceAdpcmEnv(s32 voice);
extern s32 fn_iConstructAdpcmEnv(void);
extern s32 fn_rGetPosAdpcmEnv(s32 voice);
extern s32 fn_vGetStereoAdpcmEnv(void);
extern void fn_vSynchroSoundAdpcmEnv(void);
extern s32 fn_iDestructAdpcmEnv(void);
extern s32 fn_u32DesallocateVoiceAdpcmEnv(s32 voice);
extern u32 fn_u32AllocateVoiceAdpcmEnv(s32 resource, s32 block, SndParam2D* param);
extern s32 fn_iStartVoiceAdpcmEnv(u32 voice, s32 environment, s32 resource, u32 argument, AdpcmDataCallback callback);
extern s32 fn_bSetParamAdpcmEnv(s32 voice, SndParam2D* param);
extern s32 fn_u32PushNextBlockResourceMemAdpcmEnv(s32 voice, s32 mode, s32 resource);
extern void fn_vPauseAdpcmEnv(s32 voice);
extern void fn_vResumeAdpcmEnv(s32 voice);
extern void fn_vSetStereoAdpcmEnv(s32 stereo);

extern s32 fn_bGetStereoTdstAdpcmMngr(AdpcmManager* manager);
extern s32 fn_iConstructTdstAdpcmMngr(AdpcmManager* manager);
extern s32 fn_iDestructTdstAdpcmMngr(AdpcmManager* manager);
extern void fn_vSynchroSoundTdstAdpcmMngr(AdpcmManager* manager);
extern s32 fn_bIsAllocateVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern s32 fn_bIsPlayingVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern s32 fn_bSetParamTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice, SndParam2D* param);
extern s32 fn_iStartVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice, AdpcmResource* environment,
                                       AdpcmResource* resource, u32 value28, AdpcmDataCallback value2C);
extern s32 fn_iStopVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern s32 fn_rGetPosTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern u32 fn_u32AllocateVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmResource* resource, AdpcmResource* block,
                                            SndParam2D* extra);
extern u32 fn_u32DesallocateVoiceTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern u32 gu_fn_iCloseTdstFileInfo(AdpcmManager* manager, AdpcmData* voice, s32 mode,
                                    AdpcmResource* resource);
extern void fn_vPauseTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern void fn_vResumeTdstAdpcmMngr(AdpcmManager* manager, AdpcmData* voice);
extern void fn_vSetStereoTdstAdpcmMngr(AdpcmManager* manager, s32 stereo);

extern void SND_fn_vDesInitErrorSnd(void);
extern void SND_fn_vDisplayError(s32 error, char* text);
extern void SND_fn_vDisplayErrorEx(s32 error, char* text, s32 level);
extern void SND_fn_vInitErrorSnd(void* parameters);

extern s32 SND_fn_iDesInitOptions(void);
extern s32 SND_fn_iTestInitOptions(void);
extern s32 SND_fn_iTestDesInitOptions(void);
extern s32 SND_fn_iInitOptions(u32** options);

extern s32 SND_fn_bLoadResBinarySnd(SndBinResourceDisk_inv* source, SndBinResource* resource, u32 base);
extern void SND_fn_vUnLoadResSnd(SndBinResource* resource);
extern s32 SND_fn_lPlaySnd(SndBinResource* resource, _SoundParam* params, s32 a2, SndEndCallback_inv callback,
                           s32 callbackParameter, s32* activeOut);
extern SndBinResource* SND_fn_tduGetLowestResourceSnd(SndBinResource* resource, _SoundParam* params, s32 mode);
extern s32 SND_fn_bGetStereoSnd(void);
extern s32 SND_fn_bIsMusicPlayingSnd(void);
extern s32 SND_fn_lCreateNewBufferExSnd(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                        _SoundParam* param3D, void* user);
extern s32 SND_fn_lCreateNewBufferSnd(s32 voice, s32 extra0, s32 extra1, s32 extra2, _SoundParam* param3D,
                                       s32 extra3, s32 user);
extern s32 SND_fn_lCreateNewBufferSxd(s32 voice, s32 extra0, s32 extra1, s32 extra2, _SoundParam* param3D,
                                      s32 extra3, s32 user);
extern void SND_fn_vSetDopplerFactorSnd(s32 factor);
extern void SND_fn_vSetNbVoiceWishedSnd(s32 count);
extern void SND_fn_vSetNbVoiceWishedSxd(s32 type, s32 count);
extern s32 SND_fn_bCheckVersionResourceSxd(SndBinResourceDisk_inv* resource);
extern s32 SND_fn_lGetNbVoiceOptimumSnd(void);
extern s32 SND_fn_lGetNbVoiceWishedSnd(void);
extern s32 SND_fn_lGetPosBufferExSnd(s32 lBuffer);
extern s32 SND_fn_lPushBufferExSnd(s32 lBuffer, TransitionPushArg* data);
extern s32 SND_fn_rGetPosBufferSnd(s32 lBuffer);
extern s32 SND_fn_ucGetSoundVolumeSnd(void);
extern s32 SND_fn_bCanSetupTargetSnd(void);
extern s32 SND_fn_bGetReverseStereoSnd(void);
extern void SND_fn_vDeleteBufferSnd(s32 lBuffer);
extern void SND_fn_vDesInitSnd(void);
extern void SND_fn_vForceReleaseDriverSnd(void);
extern void SND_fn_vForceRestoreDriverSnd(void);
extern void SND_fn_vGetTargetLabelSnd(char* name, s32 size);
extern s32 SND_fn_bDoTransitionWithFadeSnd(s32 instance, SndBinResource* nextResource, s32 fade);
extern s32 SND_fn_lGetSwitchIndice(s32 switchType, s32 switchObject, SndBinResource* resource);
extern void SND_fn_vPauseAllSnd(void);
extern void SND_fn_vPauseBufferSnd(s32 lBuffer);
extern void SND_fn_vResumeAllSnd(void);
extern void SND_fn_vResumeBufferSnd(s32 lBuffer);
extern void SND_fn_vSetParamBufferSnd(s32 lBuffer, _SoundParam* param3D);
extern void SND_fn_vSetupTargetSnd(void);
extern void SND_fn_vSynchroSnd(void);
extern void inv_SND_fn_vStopAllOwnedSnd(void);
extern void SND_fn_vSetSoundVolumeSnd(s32 a0);
extern void SND_fn_vSetReverseStereoSnd(void);
extern void SND_fn_vReleaseDriverSnd(void);
extern void SND_fn_vRestoreDriverSnd(void);
extern void j_nullsub_103(void);
extern s32 SND_fn_bSetNextTransitionSnd(s32 instance, SndBinResource* next);
extern s32 SND_fn_bTestIsPlayingSnd(PlaySndSlot* instance);
extern void SND_fn_vPauseTransitionSnd(PlaySndSlot* slot);
extern void SND_fn_vResumeSnd(PlaySndSlot* slot);
extern void SND_fn_vResumeTransitionSnd(PlaySndSlot* slot);
extern void annule_pre_associe_snd(PlaySndSlot* slot);
extern void invalidate_ptrvalid_snd(PlaySndSlot* slot);
extern void SND_fn_vStopSnd(PlaySndSlot* hSnd);
extern void SND_fn_vStopTransitionSnd(PlaySndSlot* instance);
extern s32 SND_fn_iInitSnd(void* context);
extern s32 SND_fn_bSetParamSnd(PlaySndSlot* instance, void* params);
extern void SND_fn_vPauseSnd(PlaySndSlot* instance);
extern void SND_fn_vRemoveCallbackSnd(PlaySndSlot* instance);
struct SndThemePlayer;
typedef void (*SndTransitionCallback)(struct SndThemePlayer* player);
extern s32 SND_fn_lPlayTransitionSnd(SndBinResource* first, SndBinResource* next, SndTransitionCallback callback, struct SndThemePlayer* player,
                                     _SoundParam* params);
extern int SND_fn_bSetParamTransitionSnd(int s1, void* param);
extern s32 SND_fn_bDoTransitionWithFadeSnd2(PlaySndSlot* instance, SndBinResource* next, s32 a2);
extern s32 SND_fn_rGetPosSnd(PlaySndSlot* slot);
extern s32 SND_fn_rGetLengthSnd(PlaySndSlot* snd);
extern void dbgSND_fn_vKillMicro(s32 lMicro);
extern void SND_fn_vSetMicroParamSnd(s32 lMicro, SxdMicroParams_inv* params);
extern s32 SND_fn_lCreateMicroSnd(SxdMicroParams_inv* params);
extern s32 SND_fn_uGenerateResourceSnd(s32 a0, s32 a1, s32 a2);
extern void SND_fn_vDestroyResourceSnd(void *a0);

extern s32 SND_fn_rDivRealRealSnd(s32 numerator, s32 denominator);
extern s32 SND_fn_rMulRealRealSnd(s32 left, s32 right);

extern s32 SND_fn_bLoadResBinarySeq(SndSequenceDisk* source, SndBinResource* sequence, u32 base);
extern s32 SND_fn_lPlaySeq(SndBinResource* sequence, _SoundParam* params, s32 type, SndEndCallback_inv callback,
                           s32 callbackParameter, s32* activeOut);
extern void inv_SND_fn_vDesInitSequencePlayers(void);
extern void loc_162098(s32 index);
extern s32 SND_fn_iInitSeq(void);
extern s32 inv_SND_fn_bSetResourceStaticVolumeSequence(SndBinResource* sequence, u8 value);
extern s32 SND_fn_rGetPosSeq(void *a0);

extern u8 SND_fn_ucDistanceToVolume_0(s32 distance);
extern void SND_fn_vSetDopplerFactor(s32 factor);
extern s32 SND_fn_rDopplerPitch(const SndFixedVector3* sourcePosition,
                                const SndFixedVector3* sourceSpeed,
                                const SndFixedVector3* listenerPosition,
                                const SndFixedVector3* listenerSpeed);
extern void SND_fn_vGetDefaultRollOff(SndRollOff* rollOff);

extern s32 SND_fn_bLoadResBinaryTheme(SndThemeDisk* disk, SndBinResource* theme, u32 base);
extern s32 SND_fn_lPlayTheme(s32 unused, SndBinResource* theme, _SoundParam* params, s32 unused3,
                             SndEndCallback_inv callback, s32 callbackArgument, s32* playing);
extern s32 SND_fn_bTestIsPlayingTheme(void);
extern void SND_fn_vInitTheme(void);
extern void inv_SND_fn_vStopThemeIfPlaying(void);
extern void inv_SND_fn_vNoOpTheme(void);
extern s32 SND_fn_bSetResourceStaticVolumeTheme(SndResourceHeaderView* resource, s32 value);
extern s32 SND_fn_rGetLengthTheme(void *a0);
extern void func_00159E28(s32 a0);
extern void SND_fn_vUnLoadResTheme(SndBinResource *a0);

struct SndSxdBuffer;
extern void SND_fn_vDeleteBufferSxd(s32 lBuffer);
extern void SND_fn_vPauseBufferSxd(s32 lBuffer);
extern void SND_fn_vResumeBufferSxd(s32 lBuffer);
extern s32 SND_fn_rGetPosBufferSxd(s32 lBuffer);
extern s32 SND_fn_lGetPosBufferExSxd(s32 lBuffer);
struct SndCallbackBuffer;
extern s32 SND_fn_bGetReverseStereoSxd(void);
extern s32 SND_fn_bSetNextTransitionSxd(s32* buffer, SndBinResource* next);
extern s32 SND_fn_bTestInitSxd(void);
extern s32 SND_fn_iInitSxd(void* context);
extern s32 SND_fn_lGetNbVoiceOptimumSxd(void);
extern s32 SND_fn_lGetNbVoiceWishedSxd(void);
extern s32 SND_fn_ucGetSoundVolumeSxd(void);
extern s32 SND_fn_bCanSetupTargetSxd(void);
extern s32 func_00163E70(void);
extern void SND_fn_vForceReleaseDriverSxd(void);
extern void SND_fn_vForceRestoreDriverSxd(void);
extern void SND_fn_vGetTargetLabelSxd(char* name, s32 size);
extern void SND_fn_vReleaseDriverSxd(void);
extern void SND_fn_vRestoreDriverSxd(void);
extern void SND_fn_vSetDefaultRollOffSxd(SndRollOff* rollOff);
extern void SND_fn_vSetDefaultRollOffSnd(SndRollOff* rollOff);
extern void SND_fn_vSetReverseStereoSxd(void);
extern void SND_fn_vSetupTargetSxd(void);
extern void SND_fn_vUnLoadResSxd(SndBinResource* resource);
extern s32 SND_fn_bLoadResBinarySxd(SndBinResourceDisk_inv* src, SndBinResource* dst, u32 base);
extern s32 SND_fn_bLoadResBinarySxdWave(SndSampleDisk* source, SndSample* sample, u32 base);
extern s32 SND_fn_bLoadResBinarySxdADPCM(const SndAdpcmDiskResource* source, SND_tdstAdpcmBinRes* resource,
                                         s32 base);
extern s32 SND_fn_bLoadResBinaryAndConvertToWAVSxdADPCM(const SndAdpcmDiskResource* source,
                                                        SND_tdstAdpcmBinRes* resource, s32 base);
extern s32 SND_fn_bIsResourceLoopingSxdWave(SndSample* sample);
extern s32 SND_fn_bIsResourceLoopingSxdADPCM(SND_tdstAdpcmBinRes* resource);
extern void func_00163100(void);
extern void SND_fn_vDesInitSxd(void);
extern void SND_fn_vSynchroSxd(void);
extern void inv_SND_fn_vNoOpSxd(void);
extern void j_j_nullsub_22(void);
extern void j_j_j_nullsub_22(s32 effectId);
extern void SND_fn_vInitBankSnd(char* name);
extern void SND_fn_vCopySoundParam(void* destination, _SoundParam* source);
extern s32 SND_fn_pstGetLastEventForObjectTypeSound(s32 typeIndex);

typedef struct gu_CommandInput {
    u32 size;
    const void* data;
} gu_CommandInput;

typedef struct gu_CommandOutput {
    u32 remaining;
    u8* cursor;
} gu_CommandOutput;
extern s32 fn_u32FormatCommandChunk(s32 chunk, gu_CommandOutput* out, const gu_CommandInput* in, s32 flags);
extern void SND_fn_vSetSoundVolumeSxd(s32 a0);
extern s32 SND_fn_lCreateMicroSxd(SxdMicroParams_inv* params);
extern void SND_fn_vSetMicroParamSxd(s32 lMicro, SxdMicroParams_inv* params);
extern void SND_fn_vDestroyMicroSxd(s32 lMicro);
extern s32 SND_fn_bSetResourceStaticVolumeSxd(s32* resource, s32 volume);
extern s32 SND_fn_rGetLengthSxd(s32* sample);
extern void SND_fn_vPauseTransitionSxd(s32* buffer);
extern s32 SND_fn_bDoTransitionWithFadeSxd(s32* buffer, SndBinResource* next);
extern s32 SND_fn_bDoTransitionWithFadeSxd2(s32* x, SndBinResource* next, s32 mode);
extern void SND_fn_vRemoveCallbackSxd(struct SndCallbackBuffer* cb);
extern void SND_fn_vStopTransitionSxd(s32* buffer);
extern s32 SND_fn_bSetParamTransitionSxd(s32* buffer, _SoundParam* arg1);
extern void SND_fn_vResumeTransitionSxd(struct SndSxdBuffer* buffer);
extern void loc_163F70(s32 a0);
extern s32 SND_fn_eGetMainFormatSxd_0(s32 a0);

extern s32 fn_iInitServerFunc(SndCmdModule* modules, SndCmdModule* end);

typedef struct SndPsxMemory SndPsxMemory;
extern void fn_vConstruct_sndpsxmem(SndPsxMemory* memory);
extern void fn_vDestruct_sndpsxmem(SndPsxMemory* memory);
extern void func_00169DC0(SndPsxMemory* memory);
extern void* fn_pvMalloc_sndpsxmem(SndPsxMemory* memory, s32 kind, s32 size);
extern void* fn_pvReAlloc_sndpsxmem(SndPsxMemory* memory, void* data, s32 size);
extern s32 fn_iFree_sndpsxmem(SndPsxMemory* memory, void* data);

extern s32 SND_fn_bLoadResBinarySwitch(SndSwitchDisk* source, SndBinResource* destination, u32 base);
extern s32 SND_fn_bLoadResBinarySwitchOld(void* source, SndBinResource* destination, u32 base);
extern s32 SND_fn_lPlaySwitch(SndBinResource* resource, _SoundParam* params, s32 unk0C, SndEndCallback_inv callback,
                              s32 callbackParam);
struct SwitchInstance;
extern s32 SND_fn_bSetResourceStaticVolumeSwitch(SndResourceHeaderView* resource, s32 volume);
extern SndBinResource* SND_fn_tduGetLowestResourceSwitch(SndBinResource* resource, _SoundParam* params, s32 mode);
extern s32 SND_fn_rGetLengthSwitch(struct SwitchInstance* instance);
extern void func_00162C10(struct SwitchInstance* instance);

extern void* D_003938A8;
extern s32 g_bWorkingTheme;
extern u32 inv_SND_ulIopBlockCount;
extern s32 inv_SND_bThemeFadeStarted;
extern s32 inv_SND_lThemeFadeDuration;

extern s32 fn_iLoadFileCD(s32 file, s32* output);
extern s32 fn_iLoadFileIOP(s32 file, s32* output);
extern s32 fn_iLoadFileSPU(s32 file, s32* output);
extern void fn_vUnLoadFileIOP(s32 handle);
extern void fn_vUnLoadFileSPU(s32 handle);

struct tdstBACKMngr;
extern s32 fn_iConstructTdstBACKMngr(struct tdstBACKMngr* manager);
extern s32 fn_iDestructTdstBACKMngr(struct tdstBACKMngr* manager);
extern u32 fn_u32AllocateVoiceTdstBACKMngr(struct tdstBACKMngr* manager, tdstBACKCall* source);
extern s32 fn_iGetNumFreeVoiceTdstBACKMngr(struct tdstBACKMngr* manager);
extern u32 fn_u32DesallocateVoiceTdstBACKMngr(struct tdstBACKMngr* manager, tdstBACKData* slot);
extern s32 fn_iOperateCallTdstBACKMngr(struct tdstBACKMngr* manager, tdstBACKData* slot);

extern s32 SND_fn_bLoadResBinaryRandom(SndRandomDisk* source, SndBinResource* destination, u32 base);
extern s32 SND_fn_lPlayRandom(SndBinResource* resource, _SoundParam* params, s32 argument2,
                              SndEndCallback_inv argument3, s32 argument4);
struct RandomInstance;
extern s32 SND_fn_rGetLengthRandom(struct RandomInstance* instance);
extern s32 SND_fn_bSetResourceStaticVolumeRandom(SndBinResource* resource, s32 volume);
extern SndBinResource* SND_fn_tduGetLowestResourceRandom(SndBinResource* resource, _SoundParam* params, s32 mode);
extern void SND_fn_vRemoveCallbackRandom(struct RandomInstance* instance);

struct Transition;
extern void SND_fn_vResumeTransitionSxdWave(struct Transition* transition);
extern s32 SND_fn_bSetNextTransitionSxdWave(struct Transition* transition, TransitionSource* source);
extern s32 SND_fn_bDoTransitionWithFadeSxdWave2(struct Transition* transition, TransitionSource* source, s32 duration);
extern s32 SND_fn_bDoTransitionWithFadeSxdWave(struct Transition* transition, TransitionSource* source);
extern TransitionHandle SND_fn_lPlayTransitionSxdWave(TransitionSource* current, TransitionSource* next,
                                                      TransitionCallbackFn callback, void* callbackContext,
                                                      void* bufferContext);
extern void SND_fn_vPauseTransitionSxdWave(struct Transition* transition);
extern s32 SND_fn_iInitTransitionSxdWave(void *a0);

extern s32 fn_u32DesallocateTdstAdpcmData(AdpcmData* data);
extern s32 fn_iConstructTdstAdpcmData(AdpcmData* data);
extern s32 fn_iDestructTdstAdpcmData(AdpcmData* data);
extern u32 fn_u32AllocateTdstAdpcmData(AdpcmData* data, AdpcmResource* resource, AdpcmResource* block, SndParam2D* param);
extern s32 fn_iStopTdstAdpcmData(AdpcmData* data);
extern s32 fn_bIsPlayingTdstAdpcmData(AdpcmData* data);
extern u32 fn_u32PushNextBlockResourceMemTdstAdpcmData(AdpcmData* data, s32 mode, AdpcmResource* resource);
extern s32 fn_iStartTdstAdpcmData(AdpcmData* data, AdpcmResource* environment, AdpcmResource* resource,
                                  u32 argument, AdpcmDataCallback callback);
extern s32 fn_bSetParamTdstAdpcmData(AdpcmData* data, SndParam2D* param, void* out);
extern void fn_vPauseTdstAdpcmData(AdpcmData* data);
extern void fn_vResumeTdstAdpcmData(AdpcmData* data);
extern s32 fn_rGetPosTdstAdpcmData(AdpcmData* data);

extern SndParam2D C_SAMPLEPARAM_NEUTRE;

#endif
