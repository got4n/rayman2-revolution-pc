#include "include_asm.h"
#include "sndplaystream_inv.h"

tdstPlayStreamData_inv *fn_u32DesallocateTdstPlayStreamData_inv(tdstPlayStreamData_inv *v);
typedef Info_inv VoiceInfo_inv;

extern void fn_vGetOutParam_inv(tdstOutParam_inv *out, tdstPlayParam_inv *mix, DescHead_inv *src);

typedef struct {
    int x0;
    void *h;
} Note_inv;

extern int fn_u32GetFadeLength_inv(int which, Desc_inv *desc);
extern int fn_iRemoteCall(int a, int b, int c, int d, void *p, int n);
int fn_iStopTdstPlayStreamData_inv(tdstPlayStreamData_inv *v);

static inline void CopyParam_inv(tdstPlayParam_inv *d, tdstPlayParam_inv *s)
{
    d->lPitch = s->lPitch;
    d->ucVolume = s->ucVolume;
    d->ucPan = s->ucPan;
    d->c6 = s->c6;
    d->c7 = s->c7;
}

static inline void ClearParam_inv(tdstPlayParam_inv *t)
{
    t->lPitch = 0;
    t->ucVolume = 0;
    t->ucPan = 0;
    t->c6 = 0;
    t->c7 = 0;
}

static inline void ResetVoice_inv(tdstPlayStreamData_inv *v)
{
    v->f.bStarted = 0;
    v->f.bChannelStarted = 0;
    v->f.bSpuStarted = 0;
    v->f.bStopping = 0;
    v->f.b6 = 0;
    v->f.bRestartChannel = 0;
    v->f.bRestartSpu = 0;
    v->f.bCallbackSent = 0;
    v->f.bPaused = 0;
    v->f.bInNotify = 0;
    v->f.bChannelReady = 0;
    v->f.bSpuReady = 0;
    v->f.bChannelStopped = 0;
    v->f.bSpuStopped = 0;
    v->f.bNoNextBlock = 0;
    v->f.bSpuEndReached = 0;
    v->f.bFading = 0;
    v->f.bFadeDone = 0;
    v->f.bFadeActive = 0;
    v->f.bBlockChained = 0;
    ClearHead_inv(&v->desc.h);
#ifdef PC_PORT
    pc_memset(&v->desc.u, 0, 16);
    pc_memset(&v->stPendingBlock, 0, 16);
    pc_memset(&v->stFadeBlock, 0, 16);
#else
    sysclib_14_inv(&v->desc.u, 0, 16);
    sysclib_14_inv(&v->stPendingBlock, 0, 16);
    sysclib_14_inv(&v->stFadeBlock, 0, 16);
#endif
    v->lCallbackArg = 0;
    v->u32FadeLength = 0;
    v->lChannelFadePos = 0;
    v->lSpuFadePos = 0;
    v->pstChannel = 0;
    v->hChannel = 0;
    v->pstSpu = 0;
    v->hSpu = 0;
    v->lPos = 0;
    ClearParam_inv(&v->stParam);
}

int fn_iConstructTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, int x)
{
    ResetVoice_inv(v);
    v->lInterface = x;
    v->f.bDesallocating = 0;
    v->f.bAllocated = 0;
    return 0;
}

tdstPlayStreamData_inv *fn_u32AllocateTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, Desc_inv *desc)
{
    Msg_inv msg;
    tdstPlayStreamData_inv *ret;
    Iface_inv *i1;
    Iface_inv *i2;
    void *h1;
    void *h2;

    ret = 0;
    h1 = ret;
    h2 = ret;
    InitMsg_inv(&msg, 4, v, v->lInterface);
    v->f.bAllocated = 1;
    i1 = fn_pstFindInterface_inv(1, desc, 0, 0);
    i2 = fn_pstFindInterface_inv(2, desc, 0, 0);
    if (i1 != 0 && i2 != 0) {
        h1 = i1->AllocateVoice(desc);
        h2 = i2->AllocateVoice(desc);
        if (h1 != 0 && h2 != 0) {
            CopyHead_inv(&v->desc.h, &desc->h);
            v->desc.u = desc->u;
            if (i1->SetOwner(h1, &msg) >= 0 && i2->SetOwner(h2, &msg) >= 0)
                ret = v;
        }
    }
    v->pstChannel = i1;
    v->hChannel = h1;
    v->pstSpu = i2;
    v->hSpu = h2;
    if (ret != 0) {
        if (i1->Connect(h1, &msg) < 0 || i2->Connect(h2, &msg) < 0)
            ret = 0;
    }
    if (ret == 0)
        fn_u32DesallocateTdstPlayStreamData_inv(v);
    return ret;
}

void fn_vStartOutputTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    tdstOutParam_inv o;
    int r = !v->f.bSpuStarted;

    r = v->f.bRestartSpu || r;
    if (r) {
        v->f.bSpuStarted = 1;
        v->f.bRestartSpu = 0;
        fn_vGetOutParam_inv(&o, &v->stParam, &v->desc.h);
        v->pstSpu->u.spu.SetParam(v->hSpu, &o);
        v->pstSpu->u.spu.Start(v->hSpu, 0);
    }
}

tdstPlayStreamData_inv *fn_u32DesallocateTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    tdstPlayStreamData_inv *ret = v;
    int a;
    int b;
    int c;

    if (v->f.bAllocated) {
        v->f.bDesallocating = 1;
        a = v->f.bSpuStopped;
        a = v->f.bChannelStopped && a;
        b = v->f.bChannelStarted;
        b = v->f.bSpuStarted && b;
        c = v->f.bPaused;
        if (a || !b || c) {
            if (v->hChannel != 0)
                v->pstChannel->DesallocateVoice(v->hChannel);
            if (v->hSpu != 0)
                v->pstSpu->DesallocateVoice(v->hSpu);
            ResetVoice_inv(v);
            v->f.bDesallocating = 0;
            v->f.bAllocated = 0;
        } else {
            fn_iStopTdstPlayStreamData_inv(v);
        }
        ret = 0;
    }
    return ret;
}

int fn_iSetOwnerTdstPlayStreamData_inv(void)
{
    return 0;
}

int fn_iConnectTdstPlayStreamData_inv(void)
{
    return 0;
}

int fn_iGetNumChildTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    int n = 0;

    if (v->pstChannel != 0)
        n = v->hChannel != 0;
    if (v->pstSpu != 0 && v->hSpu != 0)
        n++;
    return n;
}

int fn_iGetChildTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, VoiceInfo_inv *info, int idx)
{
    int r = -1;

    if (idx < fn_iGetNumChildTdstPlayStreamData_inv(v)) {
        switch (idx) {
        case 0:
            info->kind = 1;
            info->h = v->hChannel;
            info->iface = v->pstChannel;
            break;
        case 1:
            info->kind = 2;
            info->h = v->hSpu;
            info->iface = v->pstSpu;
            break;
        }
        r = 0;
    }
    return r;
}

void fn_vStartChannelTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    int r = !v->f.bChannelStarted;

    r = v->f.bRestartChannel || r;
    if (r) {
        v->f.bChannelStarted = 1;
        v->f.bRestartChannel = 0;
        v->pstChannel->u.channel.Start(v->hChannel, v->desc.h.lStartPos);
    }
}

int fn_iOnReadyTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, Note_inv *note, int arg)
{
    if (note->h == v->hChannel) {
        v->f.bChannelReady = 1;
        v->f.bChannelStopped = 0;
        fn_vStartOutputTdstPlayStreamData_inv(v);
    }
    if (note->h == v->hSpu) {
        v->f.bSpuReady = 1;
        v->f.bSpuStopped = 0;
    }
    return 0;
}

static inline void CopyChan_inv(SampleChan_inv *d, SampleChan_inv *s)
{
    d->u32Size = s->u32Size;
    d->pvVag = s->pvVag;
}

void fn_vChainBlockTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    char zero[16];
    Desc_inv d;
    SampleMemory_inv *m;

    if (!v->desc.h.f.b7) {
#ifdef PC_PORT
        pc_memset(zero, 0, 16);
#else
        sysclib_14_inv(zero, 0, 16);
#endif
#ifdef PC_PORT
        if (pc_memcmp(&v->stPendingBlock, zero, 16) != 0) {
#else
        if (sysclib_11_inv(&v->stPendingBlock, zero, 16) != 0) {
#endif
            if (!(v->f.bFading < 0)) {
                v->desc.u.a = v->stPendingBlock;
                m = &v->desc.u.b;
#ifdef PC_PORT
                pc_memset(&v->stPendingBlock, 0, 16);
#else
                sysclib_14_inv(&v->stPendingBlock, 0, 16);
#endif
                CopyHead_inv(&d.h, &v->desc.h);
                if (v->desc.h.f.bStream < 0 && v->desc.h.f.bFromCD < 0) {
                    CopyChan_inv(&d.u.b.stLeft, &m->stLeft);
                    { Quad_inv *dq = &d.u.a; dq->w2 = ((Quad_inv *)m)->w2; dq->w3 = ((Quad_inv *)m)->w3; }
                } else {
                    d.u.b.stLeft.u32Size = v->desc.u.b.stLeft.u32Size;
                    d.u.b.stLeft.pvVag = m->stLeft.pvVag;
                    d.u.b.stRight.u32Size = v->desc.u.b.stRight.u32Size;
                    d.u.b.stRight.pvVag = v->desc.u.b.stRight.pvVag;
                }
                v->pstChannel->ReallocateVoice(v->hChannel, &d);
                v->pstChannel->u.channel.Start(v->hChannel, 0);
                v->f.bBlockChained = 1;
            }
        } else {
            v->f.bNoNextBlock = 1;
            if (!(v->f.bChannelReady < 0))
                fn_vStartOutputTdstPlayStreamData_inv(v);
        }
    } else {
        v->f.bRestartChannel = 1;
    }
    if (!(v->f.bNoNextBlock < 0))
        fn_vStartChannelTdstPlayStreamData_inv(v);
}

int fn_iOnEndReachedTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, Note_inv *note, int arg)
{
    if (note->h == v->hChannel)
        fn_vChainBlockTdstPlayStreamData_inv(v);
    if (note->h == v->hSpu)
        v->f.bSpuEndReached = 1;
    return 0;
}

int fn_iOnStoppedTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, Note_inv *note, int arg)
{
    if (note->h == v->hChannel) {
        v->f.bChannelReady = 0;
        v->f.bChannelStopped = 1;
    }
    if (note->h == v->hSpu)
        v->f.bSpuStopped = 1;
    return 0;
}

int fn_iOnBlockReadTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, Note_inv *note, int arg)
{
    int r;

    if (note->h == v->hSpu) {
        r = v->f.bFading;
        r = !v->f.bFadeDone && r;
        if (r)
            v->f.bFadeActive = 1;
    }
    return 0;
}

int fn_iNotifyTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, unsigned int event, Note_inv *note, int arg)
{
    int r = -1;

    if (!v->f.bInNotify)
        v->f.bInNotify = 1;
    switch (event) {
    case E_SNDITBASE_ENDREACHED:
        r = fn_iOnEndReachedTdstPlayStreamData_inv(v, note, arg);
        break;
    case E_SNDITBASE_READY_inv:
        r = fn_iOnReadyTdstPlayStreamData_inv(v, note, arg);
        break;
    case E_SNDITBASE_STOPPED_inv:
        r = fn_iOnStoppedTdstPlayStreamData_inv(v, note, arg);
        break;
    case E_SNDITBASE_BLOCKREAD_inv:
        r = fn_iOnBlockReadTdstPlayStreamData_inv(v, note, arg);
        break;
    }
    v->f.bInNotify = 0;
    return r;
}

int fn_iStartTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, tdstPlayParam_inv *t)
{
    int r = -1;

    if (v->f.bAllocated) {
        v->f.bStarted = 1;
        CopyParam_inv(&v->stParam, t);
        fn_vStartChannelTdstPlayStreamData_inv(v);
        r = 0;
    }
    return r;
}

int fn_iStopTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    int r = -1;

    if (v->f.bAllocated) {
        v->f.bStopping = 1;
        v->pstChannel->u.channel.Stop(v->hChannel, (void *)-1);
        r = 0;
    }
    return r;
}

tdstPlayStreamData_inv *fn_pSetParamTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, tdstPlayParam_inv *t)
{
    tdstPlayStreamData_inv *ret = 0;
    tdstOutParam_inv o;
    int k;

    if (v->f.bAllocated) {
        k = !v->f.bFading;
        k = v->f.bSpuReady && k;
        CopyParam_inv(&v->stParam, t);
        if (k) {
            fn_vGetOutParam_inv(&o, t, &v->desc.h);
            v->pstSpu->u.spu.SetParam(v->hSpu, &o);
        }
        ret = v;
    }
    return ret;
}

tdstPlayStreamData_inv *fn_pPushNextBlockTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, int which, Quad_inv *q)
{
    tdstPlayStreamData_inv *ret = 0;

    if (v->f.bAllocated) {
        if (q != 0) {
            if (which == 0) {
                v->stPendingBlock = *q;
            } else {
                v->f.bFading = 1;
                v->u32FadeLength = fn_u32GetFadeLength_inv(which, &v->desc);
                v->stFadeBlock = *q;
                v->pstChannel->u.channel.GetPos(v->hChannel, &v->lChannelFadePos);
                v->pstSpu->u.spu.GetPos(v->hSpu, &v->lSpuFadePos);
            }
            v->f.bBlockChained = 0;
            v->f.bCallbackSent = 0;
        } else {
#ifdef PC_PORT
            pc_memset(&v->stPendingBlock, 0, 16);
#else
            sysclib_14_inv(&v->stPendingBlock, 0, 16);
#endif
        }
        ret = v;
    }
    return ret;
}

int fn_iSetCallbackArgTdstPlayStreamData_inv(tdstPlayStreamData_inv *v, int x)
{
    int r = -1;

    if (v->f.bAllocated) {
        v->lCallbackArg = x;
        r = 0;
    }
    return r;
}

int fn_iGetPosTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    int r = -1;
    int t = !v->f.bDesallocating;

    t = v->f.bChannelStarted && t;
    if (t) {
        if (v->pstSpu->u.spu.GetPos(v->hSpu, &v->lPos) >= 0)
            r = 0;
    }
    return r;
}

int fn_iPauseTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    int r = -1;
    int t = v->f.bAllocated;

    t = !v->f.bDesallocating && t;
    t = !v->f.bPaused && t;
    if (t) {
        v->f.bPaused = 1;
        v->pstSpu->u.spu.Pause(v->hSpu);
        r = 0;
    }
    return r;
}

int fn_iResumeTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    int r = -1;
    int t = v->f.bAllocated;

    t = !v->f.bDesallocating && t;
    t = v->f.bPaused && t;
    if (t) {
        v->pstSpu->u.spu.Resume(v->hSpu);
        r = 0;
        v->f.bPaused = 0;
    }
    return r;
}

void fn_vOperateCallTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    tdstOutParam_inv o;
    int pos;
    int a;
    int b;
    int r;

    if (v->f.bAllocated) {
        if (v->f.bSpuEndReached) {
            v->f.b6 = 1;
            v->pstSpu->u.spu.Stop(v->hSpu);
        }
        if (v->f.bFadeActive) {
            unsigned int d;

            fn_vGetOutParam_inv(&o, &v->stParam, &v->desc.h);
            v->pstSpu->u.spu.GetPos(v->hSpu, &pos);
            d = pos - v->lSpuFadePos;
            if (v->u32FadeLength < d) {
                r = !v->f.bFadeDone;
                r = !v->f.bCallbackSent && r;
                if (r) {
                    v->f.bCallbackSent = 1;
                    a = v->lCallbackArg;
                    fn_iRemoteCall(0x30000, 1, 0, 0, &a, 4);
                }
                v->f.bFadeDone = 1;
                o.left = 0;
                o.right = 0;
            } else {
                o.left = (v->u32FadeLength - d) * o.left / v->u32FadeLength;
                o.right = (v->u32FadeLength - d) * o.right / v->u32FadeLength;
            }
            v->pstSpu->u.spu.SetParam(v->hSpu, &o);
        }
        r = v->f.bBlockChained;
        r = v->f.bSpuStopped || r;
        r = !v->f.bCallbackSent && r;
        r = !v->f.bDesallocating && r;
        if (r) {
            v->f.bCallbackSent = 1;
            b = v->lCallbackArg;
            fn_iRemoteCall(0x30000, 1, 0, 0, &b, 4);
        }
        if (v->f.bDesallocating)
            fn_u32DesallocateTdstPlayStreamData_inv(v);
    }
}

void fn_vResetTdstPlayStreamData_inv(tdstPlayStreamData_inv *v)
{
    ResetVoice_inv(v);
    v->f.bDesallocating = 0;
    v->f.bAllocated = 0;
}
