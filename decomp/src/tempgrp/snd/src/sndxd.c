#include "common.h"

#include "MTH/MTH.h"
#include "snd/snd.h"

#include "ps2/copy_types.h"

#include "include_asm.h"

extern SndSxdSampleList samples_sxd;

typedef struct SoundBufferStackNode SoundBufferStackNode;
typedef struct SoundBufferStackOwner SoundBufferStackOwner;

struct SoundBufferStackOwner {
    SoundBufferStackNode* head;
    SoundBufferStackNode* tail;
    s32 count;
};

struct SoundBufferStackNode {
    u8* start;
    s32 length;
    u8* data;
    s32 remaining;
    s32 id;
    s32 ready;
    SoundBufferStackNode* next;
    SoundBufferStackNode* prev;
    SoundBufferStackOwner* owner;
};

typedef struct SoundBufferStackView {
    s32 buffer;
    SndSxdBufferDesc_inv desc;
    SndSxdBufferFormat_inv format;
    void* user;
    SoundBufferStackOwner nodes;
    s32 shift;
    s32 position;
} SoundBufferStackView;

typedef struct SndSxdBuffer {
    s32 type;
    void* data;
    s32 unk08;
    s32 unk0C;
    s32 unk10;
} SndSxdBuffer;

void* CreateNewBufferStack(SndSxdBufferDesc_inv* src, SndSxdBufferFormat_inv* format, _SoundParam* param3D,
                           void* user);

typedef struct SndCallbackBuffer {
    s32 unk00;
    void* data;
    s32 type;
} SndCallbackBuffer;

extern s32 inv_SND_sxdMicroCount;

#ifdef PC_PORT
double dpadd(double, double);
double dpdiv(double, double);
double dpmul(double, double);
double litodp(s32);
s32 dptoli(double);
#endif
s32 PushBufferStack(SoundBufferStackView* stack, TransitionPushArg* buffer);
void DeleteBufferStack(SoundBufferStackView* stack);
void SND_fn_vResumeBufferSxd_001643A8(void*);
void* new_sample_sxd(s32 a, s32 b, s32 c, void* pD);

void SetParamBufferStack(SoundBufferStackView* stack, _SoundParam* param3D);

void del_sample_actif(s32* node);

void PauseBufferStack(SoundBufferStackView* stack);

void ResumeBufferStack(SoundBufferStackView* stack);

extern s32 g_lPushedBufferId;

void func_00163100(void)
{
    SndSxdSample* node;
    SndSxdSample* next;

    for (node = samples_sxd.first, next = node ? node->next : NULL; node != NULL;
         node = next, next = node ? node->next : NULL)
        del_sample_actif((s32*) node);
    samples_sxd.first = NULL;
    samples_sxd.last = NULL;
    samples_sxd.count = 0;
}

void SND_fn_vParam3Dto2D(_SoundParam* sound, SndParam2D* out)
{
    SxdMicro* micro;
    SndFixedVector3 delta;
    s32 side;
    s32 rear;
    s32 center;
    s32 volume;
    s32 pan;
    s32 dolby;
    s32 pitch;
    s32 micros;
    s32 i;
    s32 last;
    s32 distance;
    s32 sum;

    if (sound->flags & 1) {
        if (sound->flags & 4)
            out->volume = sound->vol;
        else
            out->volume = 0x7F;
        out->pitch = sound->pitch;
        out->pan = sound->pan;
        out->dolby = sound->dolby;
    } else {
        if (inv_SND_sxdMicroCount != 0) {
            volume = 0;
            micro = micros_sxd.head;
            pan = 0x40;
            dolby = 0;
            side = 0;
            center = 0;
            rear = 0;
            pitch = 0x10000;
            micros = 0;
            if (micro != NULL) {
                do {
                    if (!micro->inactive) {
                        for (i = 0; i < sound->microCount; i++)
                            if (sound->micros[i].link.micro == micro)
                                break;
                        if (i < sound->microCount && sound->micros[i].link.enabled != 0) {
                            MTH3D_M_vSubLowerXYZ_inv(&delta, &sound->position, &micro->position);
                            distance = SND_fn_rPseudoNormeVectorSnd(&delta);
                            if (sound->flags & 0x10) {
                                last = sound->microCount;
                                if ((u8) last & 0x100) {
                                    if (distance != 0)
                                        goto call;
                                }
                                if ((*(&sound->micros[last])).rollOff.nearDistance != 0) {
                                call:
                                    volume = SND_fn_ucDistanceToVolumeEx(
                                        distance, &sound->micros[last = sound->microCount].rollOff);
                                } else {
                                    volume = 0;
                                }
                            } else {
                                volume = SND_fn_ucDistanceToVolume_0(distance);
                            }
                            pan = SND_fn_ucPositionToPan(&sound->position, &micro->position, &micro->inv_stFrontAxis, &micro->inv_stRightAxis);
                            dolby = SND_fn_ucPositionToDolby(&sound->position, &micro->position, &micro->inv_stFrontAxis);
                            pitch = SND_fn_rDopplerPitch(&sound->position, &sound->velocity, &micro->position,
                                                         &micro->velocity);
                            rear += (volume * dolby) >> 7;
                            side += (((volume * pan) >> 7) * (0x7F - dolby)) >> 7;
                            center += (((volume * (0x7F - pan)) >> 7) * (0x7F - dolby)) >> 7;
                        }
                    }
                    micros++;
                    micro = micro->next;
                } while (micro != NULL);
            }
            if (micros == 1) {
                out->volume = volume;
                out->pan = pan;
                out->dolby = dolby;
                out->pitch = pitch;
            } else {
                center /= micros;
                side /= micros;
                rear /= micros;
                sum = side + center;
                out->volume = sum + rear;
                if (side != 0)
                    out->pan = (side * 0x7F) / sum;
                else
                    out->pan = 0;
                if (side + center != 0)
                    out->dolby = (rear * 0x7F) / (side + center);
                else
                    out->dolby = 0x7F;
                out->pitch = 0x10000;
            }
            out->pitch = SND_fn_rMulRealRealSnd(out->pitch, sound->pitch);
            if (sound->flags & 4)
                out->volume = (out->volume * sound->vol) >> 7;
        } else {
            *out = C_SAMPLEPARAM_NEUTRE;
            out->pitch = sound->pitch;
        }
    }
    out->reverb = sound->reverb;
}

s32 SND_fn_rGetNormeSxd(_SoundParam* sound)
{
    SxdMicro* micro;
    SndFixedVector3 delta;
    s32 distance;
    s32 i;
    u8 volume;
    u8 loudest = 0;

    if (inv_SND_sxdMicroCount != 0) {
        for (micro = micros_sxd.head; micro != NULL; micro = micro->next) {
            volume = 0;
            if (!micro->inactive) {
                for (i = 0; i < sound->microCount; i++)
                    if (sound->micros[i].link.micro == micro)
                        break;
                if (i < sound->microCount && sound->micros[i].link.enabled != 0) {
                    MTH3D_M_vSubLowerXYZ_inv(&delta, &sound->position, &micro->position);
                    distance = SND_fn_rPseudoNormeVectorSnd(&delta);
                    if (sound->flags & 0x10) {
                        i = sound->microCount;
                        if ((u8) i & 0x100) {
                            if (distance != 0)
                                goto call;
                        }
                        if ((*(&sound->micros[i])).rollOff.nearDistance != 0) {
                        call:
                            volume = SND_fn_ucDistanceToVolumeEx(
                                distance, &sound->micros[i = sound->microCount].rollOff);
                        } else {
                            volume = 0;
                        }
                    } else {
                        volume = SND_fn_ucDistanceToVolume_0(distance);
                    }
                }
            }
            loudest = volume < loudest ? loudest : volume;
        }
    }
    return loudest << 16;
}

void* new_sample_sxd(s32 voice, s32 lFormat, s32 bStream, void* resourceArg)
{
    s32 resource = (s32) resourceArg;

    SndSxdSample* n;

    for (n = samples_sxd.first; n != 0; n = n->next) {
        if (n->handle == voice && n->type == lFormat && (n->bStream == bStream || lFormat != 1)) {
            n->resource = resource;
            return n;
        }
    }
    n = SND_fn_pvMallocSndEx(0, 0x1C);
    *(f32*) &n->list = 0.0f;
    n->next = 0;
    n->previous = 0;
    if (n->list != 0) {
        n->list->count--;
        if (n->list->first == n) {
            n->list->first = n->next;
        }
        if (n->list->last == n) {
            n->list->last = n->previous;
        }
    }
    n->previous = 0;
    n->list = 0;
    n->next = 0;
    {
        SndSxdSample* t = samples_sxd.last;
        n->list = &samples_sxd;
        n->previous = t;
    }
    if (samples_sxd.first == 0) {
        samples_sxd.first = n;
    } else {
        samples_sxd.last->next = n;
    }
    samples_sxd.last = n;
    samples_sxd.count++;
    n->resource = resource;
    n->handle = voice;
    n->type = lFormat;
    n->bStream = bStream;
    return n;
}

void del_sample_actif(s32* node)
{
    SndSxdSample* sample = (SndSxdSample*) node;
    SndSxdSample* link;
    SndSxdSampleList* owner;

    link = sample->next;
    if (link != 0) {
        link->previous = sample->previous;
    }
    link = sample->previous;
    if (link != 0) {
        link->next = sample->next;
    }
    owner = sample->list;
    if (owner != 0) {
        owner->count = owner->count - 1;
        owner = sample->list;
        if (owner->first == sample) {
            owner->first = sample->next;
            owner = sample->list;
        }
        if (owner->last == sample) {
            owner->last = sample->previous;
        }
    }
    sample->next = 0;
    sample->previous = 0;
    sample->list = 0;
    SND_fn_vFreeSndEx(0, node);
}

s32 SND_fn_lPlaySxd(SndBinResource* resource, _SoundParam* param3D, s32 extra0,
                    SndEndCallback_inv callback, s32 argument)
{
    s32 lVoice;

    switch (resource->sxd.lFormat) {
    case 1:
        SND_fn_lPlaySxdWave(resource, param3D, extra0, callback, argument);
    case 4:
        lVoice = SND_fn_lPlaySxdADPCM((s32) resource, param3D, extra0, callback, argument);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        lVoice = -1;
        break;
    }
    if (lVoice != -1)
        return (s32) new_sample_sxd(lVoice, resource->sxd.lFormat, resource->sxd.flags.bStream, resource);
    return -1;
}

void SND_fn_vRemoveCallbackSxd(SndCallbackBuffer* cb)
{
    switch (cb->type) {
    case 1:
        SND_fn_vRemoveCallbackSxdWave(cb->data);
        break;
    case 4:
        SND_fn_vRemoveCallbackSxdADPCM(cb->data);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
}

s32 SND_fn_bSetParamSxd(SndSxdSample* sample, void* params)
{
    s32 result;

    switch (sample->type) {
    case 1:
        result = SND_fn_bSetParamSxdWave((void*) sample->handle, params);
        break;
    case 4:
        result = SND_fn_bSetParamSxdADPCM(sample->handle, params);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        result = 0;
        break;
    }
    if (result == 0) {
        del_sample_actif((s32*) sample);
    }
    return result;
}

s32 SND_fn_bTestIsPlayingSxd(SndSxdSample* sample)
{
    s32 result;

    switch (sample->type) {
    case 1:
        result = SND_fn_bTestIsPlayingSxdWave(sample->handle);
        break;
    case 4:
        result = SND_fn_bTestIsPlayingSxdADPCM(sample->handle);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        result = 0;
        break;
    }
    if (result == 0) {
        del_sample_actif((s32*) sample);
    }
    return result;
}

void SND_fn_vStopSxd(s32* sample)
{
    switch (sample[2]) {
    case 1:
        j_nullsub_76(sample[1]);
        break;
    case 4:
        SND_fn_vStopSxdADPCM(sample[1]);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
    del_sample_actif(sample);
}

void SND_fn_vPauseSxd(s32* sample)
{
    switch (sample[2]) {
    case 1:
        SND_fn_vPauseSxdWave(sample[1]);
        break;
    case 4:
        SND_fn_vPauseSxdADPCM(sample[1]);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
}

void SND_fn_vResumeSxd(s32* sample)
{
    switch (sample[2]) {
    case 1:
        SND_fn_vResumeSxdWave(sample[1]);
        break;
    case 4:
        SND_fn_vResumeSxdADPCM(sample[1]);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
}

void SND_fn_vConvertResDiskToMemSxd(s32* resource)
{
    if (resource[7] == 1) {
        SND_fn_vConvertResDiskToMemSxdWave();
    } else {
        SND_fn_vDisplayError(0x1B, "");
    }
}

s32 SND_fn_bLoadResBinarySxd(SndBinResourceDisk_inv* src, SndBinResource* dst, u32 base)
{
    dst->sxd.lFormat = src->sxd.type;
    switch ((u32) src->sxd.type) {
    case 1:
        return SND_fn_bLoadResBinarySxdWave(&src->sample, &dst->sample, base);
    case 4:
    case 5:
        if (SND_fn_b3DOneShootShouldBeConvertedSxdWave() && src->sxd.cNbChannels == 1)
            return SND_fn_bLoadResBinaryAndConvertToWAVSxdADPCM(&src->adpcm, &dst->adpcm, base);
        return SND_fn_bLoadResBinarySxdADPCM(&src->adpcm, &dst->adpcm, base);
    case 3:
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

void SND_fn_vUnLoadResSxd(SndBinResource* resource)
{
    s32 lType;
    s32 lValue;

    lType = resource->sxd.lFormat;
    lValue = 1;
    if (lType != lValue) {
        lValue = 4;
        if (lType == lValue) {
            goto L_adpcm;
        }
        goto L_err;
    }
    loc_165220(&resource->sample);
    return;

L_adpcm:
    SND_fn_vUnLoadResSxdADPCM(&resource->adpcm);
    return;

L_err:
    SND_fn_vDisplayError(0x1B, "");
}

s32 SND_fn_bIsResLoadedSxd(SndBinResource* resource)
{
    switch (resource->sxd.lFormat) {
    case 1:
        return SND_fn_bIsResLoadedSxdWave();
    case 4:
        return SND_fn_bIsResLoadedSxdADPCM();
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

s32 SND_fn_bIsResourceLoopingSxd(SndBinResource* resource)
{
    s32 lValue;
    s32 lType;

    lValue = 1;
    lType = resource->sxd.lFormat;
    if (lType != lValue) {
        lValue = 4;
        if (lType == lValue) {
            goto adpcm;
        }
        lValue = 0;
        return lValue;
    }
    return SND_fn_bIsResourceLoopingSxdWave(&resource->sample);

adpcm:
    return SND_fn_bIsResourceLoopingSxdADPCM(&resource->adpcm);

end:
    return lValue;
}

s32 SND_fn_rGetPosSxd(s32* sample)
{
    switch (sample[2]) {
    case 1:
        return SND_fn_rGetPosSxdWave(sample[1]);
    case 4:
        return SND_fn_rGetPosSxdADPCM(sample[1]);
    default:
        SND_fn_vDisplayError(0x1B, "");
        return -1;
    }
}

s32 SND_fn_rGetLengthSxd(s32* sample)
{
    s32 lValue;
    s32 lType;

    lValue = 1;
    lType = sample[2];
    if (lType != lValue) {
        lValue = 4;
        if (lType == lValue) {
            goto adpcm;
        }
        SND_fn_vDisplayError(0x1B, "");
        lValue = -1;
        return lValue;
    }
    return SND_fn_rGetLengthSxdWave(sample[1]);

adpcm:
    return SND_fn_rGetLengthSxdADPCM(sample[1]);

    return lValue;
}

s32 SND_fn_bCheckVersionResourceSxd(SndBinResourceDisk_inv* resource)
{
    switch (resource->sxd.type) {
    case 1:
        return SND_fn_bCheckVersionResourceSxdWave();
    case 4:
        return SND_fn_lGetPosBufferExSxdADPCM();
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

void SND_fn_vSetSoundVolumeSxd(s32 a0)
{
    loc_165520(a0 & 0xFF);
}

s32 SND_fn_ucGetSoundVolumeSxd(void)
{
    return SND_fn_ucGetSoundVolumeSxdWave();
}

void inv_SND_fn_vNoOpSxd(void)
{
}

s32 func_00163E70(void)
{
    return 0;
}

void SND_fn_vSetReverseStereoSxd(void)
{
    SND_fn_vSetReverseStereoSxdADPCM();
}

s32 SND_fn_bGetReverseStereoSxd(void)
{
    return SND_fn_bGetReverseStereoSxdWave();
}

s32 SND_fn_iInitSxd(void* context)
{
    s32 result;

    inv_SND_sxdMicroCount = 0;
    micros_sxd.head = 0;
    micros_sxd.tail = 0;
    micros_sxd.count = 0;
    samples_sxd.first = 0;
    samples_sxd.last = 0;
    samples_sxd.count = 0;

    result = SND_fn_iInitSxdWave(context);
    if (result == 3) {
        SND_fn_iInitSxdADPCM(context);
        SND_fn_iInitTransitionSxdWave(context);
    }

    return result;
}

s32 SND_fn_bTestInitSxd(void)
{
    return SND_fn_bTestInitSxdWave();
}

void SND_fn_vDesInitSxd(void)
{
    SND_fn_vDesInitSxdADPCM();
    SND_fn_vDesInitSxdWave();
}

void func_00163F60(void)
{
}

void j_j_nullsub_22(void)
{
    j_nullsub_22();
}

void loc_163F70(s32 a0)
{
    SND_fn_vSetDopplerFactorSxdWave();
}

s32 SND_fn_lCreateMicroSxd(SxdMicroParams_inv* params)
{
    typedef struct MicroSxdVolatileLinks {
        u32 pad[13];
        SxdMicro* next;
        SxdMicro* prev;
        SxdMicroList_inv* volatile owner;
    } MicroSxdVolatileLinks;

#define MICRO_SXD_VLINKS(p) ((MicroSxdVolatileLinks*) (p))
    SxdMicro* node;
    SxdMicro* back;
    SxdMicroList_inv* list;

    node = (SxdMicro*) SND_fn_pvMallocSndEx(0, 0x40);
    node->owner = 0;
    node->next = 0;
    list = MICRO_SXD_VLINKS(node)->owner;
    node->prev = 0;
    if (list != 0) {
        list->count = list->count - 1;
        list = MICRO_SXD_VLINKS(node)->owner;
        if (list->head == node) {
            list->head = MICRO_SXD_VLINKS(node)->next;
            list = MICRO_SXD_VLINKS(node)->owner;
        }
        if (list->tail == node) {
            list->tail = MICRO_SXD_VLINKS(node)->prev;
        }
    }

    node->prev = 0;
    node->owner = 0;
    node->next = 0;
    back = micros_sxd.tail;
    node->owner = &micros_sxd;
    node->prev = back;
    if (micros_sxd.head == 0) {
        micros_sxd.head = node;
    } else {
        micros_sxd.tail->next = node;
    }
    micros_sxd.tail = node;
    micros_sxd.count = micros_sxd.count + 1;

    if (params == 0) {
        node->inactive = 1;
    } else {
        node->position = params->position;
        node->velocity = params->velocity;
        node->inv_stFrontAxis = params->inv_stFrontAxis;
        node->inv_stRightAxis = params->inv_stRightAxis;
    }

    inv_SND_sxdMicroCount = inv_SND_sxdMicroCount + 1;
    SND_fn_vCreateMicroSxdWave(params, node);
    return (s32) node;
}

void SND_fn_vSetMicroParamSxd(s32 lMicro, SxdMicroParams_inv* params)
{
    SxdMicro* node = (SxdMicro*) lMicro;
    SxdMicro* cursor;

    cursor = micros_sxd.head;
    if (cursor == 0) {
        goto done;
    }
    if (cursor != node) {
        do {
            cursor = cursor->next;
            if (cursor == 0) {
                goto done;
            }
        } while (cursor != node);
    }
    if (cursor != 0) {
        cursor->inactive = 0;
        cursor->position = params->position;
        cursor->velocity = params->velocity;
        cursor->inv_stFrontAxis = params->inv_stFrontAxis;
        cursor->inv_stRightAxis = params->inv_stRightAxis;
    }
done:
    SND_fn_vSetMicroParamSxdWave();
}

void SND_fn_vDestroyMicroSxd(s32 lMicro)
{
    SxdMicro* node = (SxdMicro*) lMicro;
    SxdMicro* link;
    SxdMicroList_inv* owner;

    if (node != 0) {
        link = node->next;
        if (link != 0) {
            link->prev = node->prev;
        }
        link = node->prev;
        if (link != 0) {
            link->next = node->next;
        }
        owner = node->owner;
        if (owner != 0) {
            owner->count = owner->count - 1;
            owner = node->owner;
            if (owner->head == node) {
                owner->head = node->next;
                owner = node->owner;
            }
            if (owner->tail == node) {
                owner->tail = node->prev;
            }
        }
        node->next = 0;
        node->prev = 0;
        node->owner = 0;
        SND_fn_vFreeSndEx(0, node);
        inv_SND_sxdMicroCount = inv_SND_sxdMicroCount - 1;
    }
    SND_fn_vDestroyMicroSxdWave(node);
}

s32 SND_fn_lCreateNewBufferSxd(s32 voice, s32 extra0, s32 extra1, s32 extra2, _SoundParam* param3D, s32 extra3, s32 user)
{
    s32 lVoice;
    s32 uwExtra0;
    s32 uwExtra1;
    s32 lExtra2;
    s32 lExtra3;
    s32 lUser;
    _SoundParam* lParam3D;
    SndSxdBufferDesc_inv buffer;

    SndSxdBufferFormat_inv params;

    lExtra3 = extra3;
    uwExtra0 = extra0 & 65535;
    lVoice = voice;
    lParam3D = param3D;
    lUser = user;
    uwExtra1 = extra1 & 65535;
    lExtra2 = extra2;
    memset(&buffer, 0, 0x10);
    buffer.lFormat = 1;
    memset(&params, (buffer.unk04 = lVoice, buffer.uwBitsPerSample = uwExtra0, buffer.uwNbChannels = uwExtra1, buffer.rate = lExtra2, 0),
           0xC);
    params.callback = (void*) lExtra3;
    params.type = 0;
    return SND_fn_lCreateNewBufferExSxd(&buffer, &params, lParam3D, (void*) lUser);
}

void SND_fn_vSetParamBufferSxd(s32 lBuffer, _SoundParam* param3D)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;

    if (lBuffer == -1) {
        return;
    }
    if (buffer->type == 2) {
        SetParamBufferStack(buffer->data, param3D);
        return;
    }
    SND_fn_vSetParamBufferSxdWave(buffer->data, param3D);
}

void SND_fn_vDeleteBufferSxd(s32 lBuffer)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;

    if (lBuffer == -1) {
        return;
    }
    if (buffer->type == 2) {
        DeleteBufferStack(buffer->data);
        return;
    }
    SND_fn_vDeleteBufferSxdWave((s32) buffer->data);
}

void SND_fn_vPauseBufferSxd(s32 lBuffer)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;

    if (lBuffer == -1) {
        return;
    }
    if (buffer->type == 2) {
        PauseBufferStack(buffer->data);
        return;
    }
    loc_165368(buffer->data);
}

void SND_fn_vResumeBufferSxd(s32 lBuffer)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;

    if (lBuffer == -1) {
        return;
    }
    if (buffer->type == 2) {
        ResumeBufferStack(buffer->data);
        return;
    }
    loc_165370(buffer->data);
}

void* SND_fn_pvGetPtrBufferSxd(SndSxdBuffer* buffer)
{
    s32 type;

    if ((s32) buffer == -1) {
        return 0;
    }
    type = buffer->type;
    if (type != 2) {
        return SND_fn_pvGetPtrBufferSxdWave(buffer->data);
    }
    return 0;
}

s32 SND_fn_rGetPosBufferSxd(s32 lBuffer)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;
    s32 lPosition;

    if (lBuffer != -1) {
        if (buffer->type == 2) {
            lPosition = GetPosBufferStack(buffer->data);
            return lPosition;
        }
        lPosition = SND_fn_rGetPosBufferSxdWave(buffer->data);
    } else {
        lPosition = -1;
    }
    return lPosition;
}

s32 SND_fn_lCreateNewBufferExSxd(SndSxdBufferDesc_inv* description, SndSxdBufferFormat_inv* format,
                                 _SoundParam* param3D, void* user)
{
    s32 id;
    SndSxdBuffer* buffer;

    if (description->lFormat == 1) {
        if (format->type == 2) {
            id = (s32) CreateNewBufferStack(description, format, param3D, user);
        } else {
            id = SND_fn_lCreateNewBufferExSxdWave(description, format, param3D, user);
        }
    } else {
        SND_fn_vDisplayError(0x15, "");
        id = -1;
    }

    if (id != -1) {
        buffer = SND_fn_pvMallocSndEx(0, sizeof(SndSxdBuffer));
        if (buffer == NULL) {
            return -1;
        }
        buffer->unk08 = 0;
        buffer->unk0C = 0;
        buffer->unk10 = 0;
        buffer->type = format->type;
        buffer->data = (void*) id;
        return (s32) buffer;
    }
    return -1;
}

s32 SND_fn_lGetPosBufferExSxd(s32 lBuffer)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;
    s32 lPosition;

    if (lBuffer != -1) {
        if (buffer->type == 2) {
            lPosition = GetPosBufferStackEx(buffer->data);
            goto end;
        }
        lPosition = SND_fn_lGetPosBufferExSxdWave(buffer->data);
    } else {
        lPosition = -1;
    }
end:
    return lPosition;
}

s32 SND_fn_lPushBufferExSxd(s32 lBuffer, TransitionPushArg* data)
{
    SndSxdBuffer* buffer = (SndSxdBuffer*) lBuffer;

    if (lBuffer != -1) {
        return PushBufferStack(buffer->data, data);
    }
    return -1;
}

s32 func_001645A0(void)
{
    return -1;
}

s32 SND_fn_lPlayTransitionExSxd(SndBinResource* src, SndBinResource* a1, void* a2, void* a3,
                                void* t0)
{
    s32* slot;
    s32 lValue;
    s32 lType;

    slot = SND_fn_pvMallocSndEx(0, 8);
    if (slot != 0) {
        lValue = 1;
        lType = src->sxd.lFormat;
        if (lType != lValue) {
            lValue = 4;
            if (lType != lValue) {
                goto error;
            }
            lValue = SND_fn_lPlayTransitionSxdADPCM((s32) src, (s32) a1, a2, (s32) a3, t0);
            slot[1] = lValue;
        } else {
            lValue = SND_fn_lPlayTransitionSxdWave(&src->transition, &a1->transition, a2, a3, t0);
            slot[1] = lValue;
        }
        lValue = src->sxd.lFormat;
        slot[0] = lValue;
        goto check;

    error:
        SND_fn_vDisplayError(0x1B, "");
        lValue = -1;
        slot[1] = lValue;

    check:
        lType = slot[1];
        lValue = -1;
        if (lType != lValue) {
            lValue = (s32) slot;
            return lValue;
        }
        SND_fn_vFreeSndEx(0, slot);
    }
    lValue = -1;
done:
    return lValue;
}

s32 SND_fn_bSetParamTransitionSxd(s32* buffer, _SoundParam* param3D)
{
    switch (buffer[0]) {
    case 1:
        return SND_fn_bSetParamTransitionSxdWave(buffer[1], param3D);
    case 4:
        return SND_fn_bSetParamTransitionSxdADPCM(buffer[1], param3D);
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

s32 SND_fn_bSetNextTransitionSxd(s32* buffer, SndBinResource* next)
{
    switch (buffer[0]) {
    case 1:
        return SND_fn_bSetNextTransitionSxdWave((struct Transition*) buffer[1], &next->transition);
    case 4:
        return SND_fn_bSetNextTransitionSxdADPCM(buffer[1], (s32) next);
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

s32 SND_fn_bDoTransitionWithFadeSxd(s32* buffer, SndBinResource* next)
{
    switch (buffer[0]) {
    case 1:
        return SND_fn_bDoTransitionWithFadeSxdWave((struct Transition*) buffer[1], &next->transition);
    case 4:
        return SND_fn_bDoTransitionWithFadeSxdADPCM(buffer[1], (s32) next);
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

s32 SND_fn_bDoTransitionWithFadeSxd2(s32* x, SndBinResource* next, s32 mode)
{
    switch (x[0]) {
    case 1:
        return SND_fn_bDoTransitionWithFadeSxdWave2((struct Transition*) x[1], &next->transition, mode);
    case 4:
        return SND_fn_bDoTransitionWithFadeSxdADPCM2(x[1], (s32) next, mode);
    default:
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
}

void SND_fn_vStopTransitionSxd(s32* buffer)
{
    switch (buffer[0]) {
    case 1:
        SND_fn_vStopTransitionSxdWave(buffer[1]);
        break;
    case 4:
        SND_fn_vStopTransitionSxdADPCM(buffer[1]);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
    SND_fn_vFreeSndEx(0, buffer);
}

void SND_fn_vPauseTransitionSxd(s32* buffer)
{
    if ((s32) buffer == -1) {
        return;
    }
    switch (buffer[0]) {
    case 1:
        SND_fn_vPauseTransitionSxdWave((struct Transition*) buffer[1]);
        break;
    case 4:
        SND_fn_vPauseTransitionSxdADPCM(buffer[1]);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
}

void SND_fn_vResumeTransitionSxd(SndSxdBuffer* buffer)
{
    switch (buffer->type) {
    case 1:
        SND_fn_vResumeTransitionSxdWave(buffer->data);
        break;
    case 4:
        SND_fn_vResumeTransitionSxdADPCM((s32) buffer->data);
        break;
    default:
        SND_fn_vDisplayError(0x1B, "");
        break;
    }
}

void SND_fn_vReleaseDriverSxd(void)
{
    SND_fn_vReleaseDriverSxdADPCM();
    SND_fn_vReleaseDriverSxdWave();
}

void SND_fn_vRestoreDriverSxd(void)
{
    SND_fn_vRestoreDriverSxdWave();
    SND_fn_vRestoreDriverSxdADPCM();
}

void SND_fn_vForceReleaseDriverSxd(void)
{
    SND_fn_vForceReleaseDriverSxdWave();
}

void SND_fn_vForceRestoreDriverSxd(void)
{
    SND_fn_vForceRestoreDriverSxdWave();
}

#ifdef PC_PORT
void SND_fn_vGetTargetLabelSxd(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSxdWave(name, size);
}
#else
void SND_fn_vGetTargetLabelSxd(char* name, s32 size)
{
    SND_fn_vGetTargetLabelSxdWave(name, size);
}
#endif

void SND_fn_vSetupTargetSxd(void)
{
    SND_fn_vSetupTargetSxdWave();
}

s32 SND_fn_bCanSetupTargetSxd(void)
{
    return SND_fn_bCanSetupTargetSxdWave();
}

void SND_fn_vSynchroSxd(void)
{
    SND_fn_vSynchroSxdWave();
    SND_fn_vSynchroSxdADPCM();
}

s32 func_001649D8(void)
{
    return -1;
}

s32 func_001649E0(void)
{
    return 0;
}

void func_001649E8(void)
{
}

void func_001649F0(void)
{
}

void SND_fn_vSetDefaultRollOffSxd(SndRollOff* rollOff)
{
    SND_fn_vSetDefaultRollOffSxdWave(rollOff);
    SND_fn_vSetDefaultRollOffSxdADPCM(rollOff);
}

void fnRefreshBufferStack(SoundBufferStackView* stack, s32 unused, s32 size, void* dst)
{
    SoundBufferStackNode* node;
    s32 left;
    s32 amount;
    u8* p_ucDst;

    node = stack->nodes.head;
    if (node != (SoundBufferStackNode*) 0) {
        p_ucDst = dst;
        if (node->ready != 0) {
            if (size != 0) {
                left = size;
                do {
                    amount = node->remaining;
                    if ((u32) amount >= (u32) left) {
                        amount = left;
                    }
                    memcpy(p_ucDst, node->data, amount << stack->shift);
                    node->data += amount << stack->shift;
                    left -= amount;
                    p_ucDst += amount << stack->shift;
                    node->remaining -= amount;
                    if (node->remaining == 0) {
                        ((void (*)(s32, s32)) stack->format.callback)((s32) stack->user, node->id);
                        if (node->next != (SoundBufferStackNode*) 0) {
                            node->next->prev = node->prev;
                        }
                        if (node->prev != (SoundBufferStackNode*) 0) {
                            node->prev->next = node->next;
                        }
                        if (node->owner != (SoundBufferStackOwner*) 0) {
                            node->owner->count--;
                            if (node->owner->head == node) {
                                node->owner->head = node->next;
                            }
                            if (node->owner->tail == node) {
                                node->owner->tail = node->prev;
                            }
                        }
                        node->next = (SoundBufferStackNode*) 0;
                        node->prev = (SoundBufferStackNode*) 0;
                        node->owner = (SoundBufferStackOwner*) 0;
                        SND_fn_vFreeSndEx(0, node);
                        node = stack->nodes.head;
                        if (node == (SoundBufferStackNode*) 0) {
                            memset(p_ucDst, 0, left << stack->shift);
                            left = 0;
                        }
                    }
                } while (left != 0);
            }
            goto update_position;
        }
    }
    memset(dst, 0, size << stack->shift);

update_position:
    stack->position += size;
}

void* CreateNewBufferStack(SndSxdBufferDesc_inv* src, SndSxdBufferFormat_inv* format, _SoundParam* param3D,
                           void* user)
{
    SoundBufferStackView* stack;
    SndSxdBufferFormat_inv arg;

    stack = SND_fn_pvMallocSndEx(0, sizeof(SoundBufferStackView));
    stack->desc = *src;
    stack->format = *format;

    stack->user = user;
    {
        u16 a;
        u16 b;
        u32 hi;
        u32 sum;
        a = src->uwBitsPerSample;
        b = src->uwNbChannels;
        hi = (u32) a >> 3;
        stack->nodes.head = 0;
        sum = hi + b;
        stack->nodes.tail = 0;
        stack->shift = (s32) sum - 2;
        stack->nodes.count = 0;
        stack->position = 0;
    }
    arg.type = 1;
    arg.callback = (void*) fnRefreshBufferStack;
    arg.arg = 0;
    stack->buffer = SND_fn_lCreateNewBufferExSxd(&stack->desc, &arg, param3D, stack);

    return stack;
}

#ifdef PC_PORT
s32 GetPosBufferStack(SoundBufferStackView* stack)
{
    s32 divisor;
    s32 integerPosition;
    double position;
    double scale;
    double ratio;

    position = litodp(GetPosBufferStackEx(stack));
    divisor = stack->desc.rate;
    scale = litodp(divisor);
    if (divisor < 0) {
        scale = dpadd(scale, 4294967296.0);
    }
    ratio = dpdiv(position, scale);
    integerPosition = dptoli(ratio);
    if (integerPosition < 0x7FFF) {
        return dptoli(dpmul(ratio, 65536.0));
    }
    return -1;
}
#else
s32 GetPosBufferStack(SoundBufferStackView* stack)
{
    u32 divisor;
    s32 integerPosition;
    double position;
    double scale;
    double ratio;

    position = (double) GetPosBufferStackEx(stack);
    divisor = stack->desc.rate;
    scale = (double) divisor;
    ratio = position / scale;
    integerPosition = (s32) ratio;
    if (integerPosition < 0x7FFF) {
        return (s32) (ratio * 65536.0);
    }
    return -1;
}
#endif

s32 PushBufferStack(SoundBufferStackView* stack, TransitionPushArg* buffer)
{
    SoundBufferStackNode* node = SND_fn_pvMallocSndEx(0, sizeof(SoundBufferStackNode));
    s32 id = g_lPushedBufferId;

    {
        SoundBufferStackOwner** owner = &node->owner;

        node->next = NULL;
        node->prev = NULL;
        *owner = NULL;
    }
    node->start = buffer->data;
    node->length = buffer->size;
    node->id = id;
    id++;
    node->data = node->start;
    node->remaining = node->length;
    node->ready = 0;
    g_lPushedBufferId = id;
    if (id > 0x7FFFFFFE)
        g_lPushedBufferId = 0;
    if (node->owner != NULL) {
        node->owner->count--;
        if (node->owner->head == node)
            node->owner->head = node->next;
        if (node->owner->tail == node)
            node->owner->tail = node->prev;
    }
    node->prev = NULL;
    node->owner = NULL;
    node->next = NULL;
    node->prev = stack->nodes.tail;
    node->owner = &stack->nodes;
    if (stack->nodes.head == NULL)
        stack->nodes.head = node;
    else
        stack->nodes.tail->next = node;
    stack->nodes.tail = node;
    stack->nodes.count++;
    node->ready = 1;
    return node->id;
}

void SetParamBufferStack(SoundBufferStackView* stack, _SoundParam* param3D)
{
    SND_fn_vSetParamBufferSxd(stack->buffer, param3D);
}

void DeleteBufferStack(SoundBufferStackView* stack)
{
    SoundBufferStackNode* node;
    SoundBufferStackNode* next;

    SND_fn_vDeleteBufferSxd(stack->buffer);
    for (node = stack->nodes.head, next = node ? node->next : NULL; node != NULL;
         node = next, next = node ? node->next : NULL) {
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
        if (node->owner != NULL) {
            node->owner->count--;
            if (node->owner->head == node) {
                node->owner->head = node->next;
            }
            if (node->owner->tail == node) {
                node->owner->tail = node->prev;
            }
        }
        node->next = NULL;
        node->prev = NULL;
        node->owner = NULL;
        SND_fn_vFreeSndEx(0, node);
    }
    SND_fn_vFreeSndEx(0, stack);
}

void PauseBufferStack(SoundBufferStackView* stack)
{
    SND_fn_vPauseBufferSxd(stack->buffer);
}

void ResumeBufferStack(SoundBufferStackView* stack)
{
    SND_fn_vResumeBufferSxd_001643A8((void*) (long) stack->buffer);
}

s32 GetPosBufferStackEx(SoundBufferStackView* stack)
{
    return stack->position;
}

s32 fn_eMainTypeOfZip(void)
{
    return SND_fn_bIsLibActiveSxdADPCM() ? 4 : 1;
}

void SND_fn_vSetNbVoiceWishedSxd(s32 type, s32 count)
{
    if (type != 1 && type != 4) {
        SND_fn_vDisplayError(0x1B, "");
    }
}

s32 SND_fn_eGetMainFormatSxd_0(s32 a0)
{
    s32 v0, v1;
    v0 = SND_fn_b3DOneShootShouldBeConvertedSxdWave();
    if (v0 == 0) {
        v1 = SND_fn_bIsLibActiveSxdADPCM();
        if (v1 != 0)
            return 4;
    }
    return 1;
}

s32 SND_fn_lGetNbVoiceWishedSxd(void)
{
    return SND_fn_lGetNbVoiceWishedSxdWave();
}

s32 SND_fn_lGetNbVoiceOptimumSxd(void)
{
    return SND_fn_lGetNbVoiceOptimumSxdWave();
}

s32 SND_fn_bSetResourceStaticVolumeSxd(s32* resource, s32 volume)
{
    s32 lValue;
    s32 lType;

    lValue = 1;
    lType = resource[6];
    volume = volume & 0xFF;
    if (lType != lValue) {
        lValue = 4;
        if (lType == lValue) {
            goto L_adpcm;
        }
        SND_fn_vDisplayError(0x1B, "");
        return 0;
    }
    lValue = SND_fn_bSetResourceStaticVolumeSxdWave((s32) resource, volume);
    return lValue;

L_adpcm:
    lValue = SND_fn_bSetResourceStaticVolumeSxdADPCM((s32) resource, volume);
L_ret:
    return lValue;
}
