#include "include_asm.h"
#include "sndchannelcdstereo_inv.h"

static inline void ClearRing_inv(tdstRingBuffer_inv *q)
{
    q->wr = 0;
    q->written = 0;
    q->limit = 0;
    q->rd = 0;
    q->read = 0;
}

static inline void ResetElemD_inv(tdstChannelCDStereoData_inv *e)
{
    e->f.bStarted = 0;
    e->f.b2 = 0;
    e->f.bReady = 0;
    e->f.b4 = 0;
    e->f.bStopping = 0;
    e->f.bStopped = 0;
    e->f.bFillLeft = 1;
    e->f.bReadyNotified = 0;
    e->f.bStopNotified = 0;
    e->f.b10 = 0;
    e->f.bEndReached = 0;
    e->f.bReadLeft = 1;
    e->pstOwner = 0;
    e->hOwner = 0;
    e->pstChannel = 0;
    e->hChannel = 0;
    e->pstSpu = 0;
    e->hSpuLeft = 0;
    e->hSpuRight = 0;
    e->u32BlockSize = 0;
    e->u32HalfLength = 0;
    ClearRing_inv(&e->stLeft);
    e->stLeft.span.size = 0;
    e->stLeft.span.data = 0;
    ClearRing_inv(&e->stRight);
    e->stRight.span.size = 0;
    e->stRight.span.data = 0;
    e->f.bAllocated = 0;
}

static inline void AdvanceRing_inv(tdstRingBuffer_inv *q, int n)
{
    q->limit += n;
}

extern unsigned int fn_u32WriteTdstRingBuffer_inv(tdstRingBuffer_inv *r, void *span);

extern unsigned int fn_u32ReadTdstRingBuffer_inv(tdstRingBuffer_inv *r, Span_inv *s);
extern int fn_u32ClearBlocks_inv();
extern void fn_vSplitTdstDesc_inv(Desc_inv *a, Desc_inv *b, Desc_inv *src);
extern Iface_inv *fn_pstFindInterface_inv(int kind, void *desc, int c, int d);
int fn_u32DesallocateTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e);

static inline int RingAtLimit_inv(tdstRingBuffer_inv *q)
{
    return q->written >= q->limit;
}

typedef struct {
    int x0;
    int x4;
    int x8;
    int xc;
} Slot_inv;

extern int fn_iFindChild_inv(Info_inv *info, int kind, int arg);
int fn_iCallOwnerTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, int event, int arg);

int fn_iConstructTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, int a, int *cfg)
{
    e->lInterface = a;
    e->u32ReadSize = cfg[0];
    e->pReadBuf = (char *)cfg[1];
    ResetElemD_inv(e);
    return 0;
}

int fn_iDestructTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    if (e->hChannel != 0)
        e->pstChannel->DesallocateVoice(e->hChannel);
    ResetElemD_inv(e);
    return 0;
}

int fn_iStartTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    if (!e->f.bStarted) {
        e->f.bStarted = 1;
        AdvanceRing_inv(&e->stLeft, e->u32HalfLength);
        AdvanceRing_inv(&e->stRight, e->u32HalfLength);
    }
    e->pstChannel->u.channel.Start(e->hChannel);
    e->f.bEndReached = 0;
    e->f.b10 = 0;
    e->f.bStopping = 0;
    e->f.bStopped = 0;
    e->f.bStopNotified = 0;
    return 0;
}

int fn_iStopTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    e->f.bStopping = 1;
    e->pstChannel->u.channel.Stop(e->hChannel);
    return 0;
}

tdstChannelCDStereoData_inv *fn_u32AllocateTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, Desc_inv *d)
{
    Desc_inv tmp;
    Msg_inv msg;
    Span_inv sa;
    Span_inv sb;
    tdstChannelCDStereoData_inv *ret = 0;

    e->f.bAllocated = 1;
    if (d->u.a.w0 != 0) {
        fn_vSplitTdstDesc_inv(&tmp, 0, d);
        tmp.u.a.w0 = (d->u.a.w0 + 2047) & ~2047;
        if (e->pstChannel != 0) {
            if (e->pstChannel->ReallocateVoice(e->hChannel, &tmp) == e->hChannel) {
                e->f.b4 = 0;
                ret = e;
            }
        } else {
            e->pstChannel = fn_pstFindInterface_inv(1, &tmp, 0, 0);
            if (e->pstChannel != 0) {
                e->hChannel = e->pstChannel->AllocateVoice(&tmp);
                if (e->hChannel != 0) {
                    msg.lInterface = e->lInterface;
                    msg.kind = 1;
                    msg.h = e;
                    e->pstChannel->SetOwner(e->hChannel, &msg);
                    sa.size = 8192;
                    sa.data = e->a_ucLeft;
                    ClearRing_inv(&e->stLeft);
                    e->stLeft.span.size = sa.size;
                    e->stLeft.span.data = sa.data;
                    sb.size = 8192;
                    sb.data = e->a_ucRight;
                    ClearRing_inv(&e->stRight);
                    e->stRight.span.size = sb.size;
                    e->stRight.span.data = sb.data;
                    ret = e;
                }
            }
        }
        e->f.bReadyNotified = 0;
        e->u32BlockSize = d->u.a.w3;
        e->u32HalfLength = (unsigned int)d->u.a.w0 >> 1;
    }
    if (ret == 0)
        fn_u32DesallocateTdstChannelCDStereoData_inv(e);
    return ret;
}

int fn_u32DesallocateTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    if (e->pstChannel != 0 && e->hChannel != 0)
        e->pstChannel->DesallocateVoice(e->hChannel);
    ResetElemD_inv(e);
    return 0;
}

int fn_iFillTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    int total = 0;
    tdstRingBuffer_inv *q;

    for (;;) {
        q = e->f.bFillLeft ? &e->stLeft : &e->stRight;
        if (q->span.size + q->read - q->written < e->u32BlockSize)
            break;
        if (!e->pstChannel->u.channel.Read(e->hChannel, e, 1, &e->u32ReadSize))
            break;
        e->f.bFillLeft = !e->f.bFillLeft;
        total += fn_u32WriteTdstRingBuffer_inv(q, &e->u32ReadSize);
        if (RingAtLimit_inv(q))
            AdvanceRing_inv(q, e->u32HalfLength);
    }
    return total;
}

unsigned int fn_u32ReadTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, int id, int more, Span_inv *s)
{
    unsigned int r = 0;
    unsigned int n;
    tdstRingBuffer_inv *q;
    int full = 0;

    if (e->hSpuLeft == id) {
        q = &e->stLeft;
        full = !e->f.bReadLeft;
    } else {
        q = &e->stRight;
        if (e->f.bReadLeft)
            full = 1;
    }
    if (!full) {
        e->f.bReadLeft = !e->f.bReadLeft;
        while (r < s->size && more) {
            Span_inv c;

            c.size = s->size - r;
            c.data = s->data + r;
            n = fn_u32ReadTdstRingBuffer_inv(q, &c);
            r += n;
            fn_iFillTdstChannelCDStereoData_inv(e);
            if (n == 0)
                break;
        }
    } else {
        r = fn_u32ClearBlocks_inv(s->data, s->size);
    }
    return r;
}

int fn_iSetOwnerTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, Info_inv *info)
{
    e->pstOwner = info->iface;
    e->hOwner = info->h;
    return 0;
}

int fn_iConnectTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, int arg)
{
    Info_inv info;
    Slot_inv s0;
    Slot_inv s1;
    int r = -1;

    if (fn_iFindChild_inv(&info, 2, arg) >= 0) {
        info.iface->GetNumChild(info.h);
        info.iface->GetChild(info.h, &s0, 0);
        info.iface->GetChild(info.h, &s1, 1);
        e->pstSpu = info.iface;
        e->hSpuLeft = s0.x4;
        e->hSpuRight = s1.x4;
        if (e->pstChannel->Connect(e->hChannel, (void *)arg) >= 0)
            r = 0;
    }
    return r;
}

int fn_iGetNumChildTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    int n = 0;

    if (e->pstChannel != 0)
        n = e->hChannel != 0;
    return n;
}

int fn_iGetChildTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, Info_inv *info, int idx)
{
    int r = -1;

    if (idx < fn_iGetNumChildTdstChannelCDStereoData_inv(e)) {
        switch (idx) {
        case 0:
            info->kind = 1;
            info->h = e->hChannel;
            info->iface = e->pstChannel;
            break;
        }
        r = 0;
    }
    return r;
}

int fn_iNotifyTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, unsigned int event)
{
    switch (event) {
    case E_SNDITBASE_READY_inv:
        fn_iFillTdstChannelCDStereoData_inv(e);
        e->f.bReady = 1;
        break;
    case E_SNDITBASE_ENDREACHED:
        e->f.bEndReached = 1;
        fn_iCallOwnerTdstChannelCDStereoData_inv(e, E_SNDITBASE_ENDREACHED, 0);
        break;
    case E_SNDITBASE_STOPPED_inv:
        e->f.bStopped = 1;
        break;
    }
    return 0;
}

int fn_iCallOwnerTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (e->pstOwner != 0 && e->hOwner != 0) {
        InitMsg_inv(&msg, 1, e, 0);
        r = e->pstOwner->Notify(e->hOwner, event, &msg, arg);
    }
    return r;
}

int fn_iGetPosTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e, unsigned int *out)
{
    unsigned int v;
    int r = -1;

    if (e->pstChannel->u.channel.GetPos(e->hChannel, (int *)&v) >= 0) {
        r = 0;
        *out = v >> 1;
    }
    return r;
}

void fn_vOperateCallTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    int t;

    if (e->f.bAllocated) {
        t = e->f.bReady;
        t = !e->f.bReadyNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstChannelCDStereoData_inv(e, E_SNDITBASE_READY_inv, 0) >= 0)
                e->f.bReadyNotified = 1;
        }
        t = e->f.bStopped;
        t = !e->f.bStopNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstChannelCDStereoData_inv(e, E_SNDITBASE_STOPPED_inv, 0) >= 0)
                e->f.bStopNotified = 1;
        }
    }
}

void fn_vResetTdstChannelCDStereoData_inv(tdstChannelCDStereoData_inv *e)
{
    ResetElemD_inv(e);
}
