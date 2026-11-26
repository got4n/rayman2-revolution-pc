#include "include_asm.h"
#include "sndchannelmem_inv.h"

int fn_u32DesallocateTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan);

typedef struct {
    int x0;
    void *h;
} Note_inv;

typedef struct {
    int x0;
    int x4;
    int x8;
    int xc;
} Slot_inv;

extern int fn_iFindChild_inv(Info_inv *info, int kind, int arg);

extern int stdio_4_inv(const char *fmt, ...);

int fn_iConstructTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, int x)
{
    chan->f.b1 = 0;
    chan->f.b2 = 0;
    chan->f.b3 = 0;
    chan->f.b4 = 0;
    chan->f.b5 = 0;
    chan->f.b6 = 0;
    chan->f.b7 = 0;
    chan->f.b8 = 0;
    chan->f.b9 = 0;
    chan->f.b10 = 0;
    chan->f.b11 = 0;
    chan->f.b12 = 0;
    chan->f.b13 = 0;
    chan->f.b14 = 0;
    chan->lInterface = x;
    chan->pstOwner = 0;
    chan->hOwner = 0;
    chan->pstChannel = 0;
    chan->hLeft = 0;
    chan->hRight = 0;
    chan->pstSpu = 0;
    chan->hSpuLeft = 0;
    chan->hSpuRight = 0;
    chan->lStartPos = 0;
    chan->f.bAllocated = 0;
    return 0;
}

int fn_iDestructTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan)
{
    if (chan->hLeft != 0)
        chan->pstChannel->DesallocateVoice(chan->hLeft);
    if (chan->hRight != 0)
        chan->pstChannel->DesallocateVoice(chan->hRight);
    chan->f.b1 = 0;
    chan->f.b2 = 0;
    chan->f.b3 = 0;
    chan->f.b4 = 0;
    chan->f.b5 = 0;
    chan->f.b6 = 0;
    chan->f.b7 = 0;
    chan->f.b8 = 0;
    chan->f.b9 = 0;
    chan->f.b10 = 0;
    chan->f.b11 = 0;
    chan->f.b12 = 0;
    chan->f.b13 = 0;
    chan->f.b14 = 0;
    chan->pstOwner = 0;
    chan->hOwner = 0;
    chan->pstChannel = 0;
    chan->hLeft = 0;
    chan->hRight = 0;
    chan->pstSpu = 0;
    chan->hSpuLeft = 0;
    chan->hSpuRight = 0;
    chan->lStartPos = 0;
    chan->f.bAllocated = 0;
    return 0;
}

int fn_iStartTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, int x)
{
    int r = -1;

    if (!chan->f.b1) {
        if (chan->pstChannel->u.channel.Start(chan->hLeft, x) >= 0 && chan->pstChannel->u.channel.Start(chan->hRight, x) >= 0) {
            r = 0;
            chan->f.b1 = 1;
        }
    } else {
        if (chan->f.b2) {
            if (chan->pstChannel->u.channel.Start(chan->hLeft, x) >= 0) {
                r = 0;
                chan->f.b2 = 0;
            }
        }
        if (chan->f.b3) {
            if (chan->pstChannel->u.channel.Start(chan->hRight, x) >= 0) {
                r = 0;
                chan->f.b3 = 0;
            }
        }
    }
    chan->lStartPos = x;
    chan->f.b10 = 1;
    return r;
}

int fn_iCallOwnerTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (chan->pstOwner != 0 && chan->hOwner != 0) {
        InitMsg_inv(&msg, 1, chan, 0);
        r = chan->pstOwner->Notify(chan->hOwner, event, &msg, arg);
    }
    return r;
}

int fn_iStopTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan)
{
    int r = !chan->f.b6;

    r = !chan->f.b7 && r;
    if (!r) {
        if (chan->f.b6)
            chan->pstChannel->u.channel.Stop(chan->hLeft);
        if (chan->f.b7)
            chan->pstChannel->u.channel.Stop(chan->hRight);
    } else {
        fn_iCallOwnerTdstChannelMEMData_inv(chan, E_SNDITBASE_STOPPED_inv, 0);
    }
    return 0;
}

tdstChannelMEMData_inv *fn_u32AllocateTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, Desc_inv *desc)
{
    char b1[40];
    char b2[40];
    Msg_inv msg;
    tdstChannelMEMData_inv *ret;
    Iface_inv *iface;
    void *h1;
    void *h2;

    fn_vSplitTdstDesc_inv((Desc_inv *)b1, (Desc_inv *)b2, desc);
    ret = 0;
    h1 = ret;
    h2 = ret;
    chan->f.bAllocated = 1;
    if (!chan->f.b1) {
        iface = fn_pstFindInterface_inv(1, b1, 0, 0);
        if (iface != 0) {
            h1 = iface->AllocateVoice(b1);
            h2 = iface->AllocateVoice(b2);
            if (h1 != 0 && h2 != 0) {
                ret = chan;
                msg.lInterface = chan->lInterface;
                msg.kind = 1;
                msg.h = ret;
                iface->SetOwner(h1, &msg);
                iface->SetOwner(h2, &msg);
            }
        }
        chan->pstChannel = iface;
        chan->hLeft = h1;
        chan->hRight = h2;
    } else {
        chan->f.b11 = 1;
        iface = chan->pstChannel;
        h1 = chan->hLeft;
        h2 = chan->hRight;
        if (desc != &chan->desc) {
            CopyHead_inv(&chan->desc.h, &desc->h);
            if (desc->h.f.bStream < 0 && desc->h.f.bFromCD < 0) {
                CopyQuad_inv(&chan->desc.u.a, &desc->u.a);
            } else {
                chan->desc.u.b.stLeft.u32Size = desc->u.b.stLeft.u32Size;
                chan->desc.u.b.stLeft.pvVag = desc->u.b.stLeft.pvVag;
                chan->desc.u.b.stRight.u32Size = desc->u.b.stRight.u32Size;
                chan->desc.u.b.stRight.pvVag = desc->u.b.stRight.pvVag;
            }
        }
        if (chan->f.b4 && iface->ReallocateVoice(h1, b1) == h1)
            ret = chan;
        if (chan->f.b5 && iface->ReallocateVoice(h2, b2) == h2)
            ret = chan;
    }
    chan->f.b6 = 0;
    chan->f.b7 = 0;
    chan->f.b12 = 0;
    if (ret == 0)
        fn_u32DesallocateTdstChannelMEMData_inv(chan);
    return ret;
}

int fn_u32DesallocateTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan)
{
    fn_iDestructTdstChannelMEMData_inv(chan);
    return 0;
}

int fn_iReadTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, int id, int a, int b)
{
    int r = 0;

    if (chan->hSpuLeft == id)
        r = chan->pstChannel->u.channel.Read(chan->hLeft, id, a, b);
    if (chan->hSpuRight == id)
        r = chan->pstChannel->u.channel.Read(chan->hRight, id, a, b);
    return r;
}

int fn_iSetOwnerTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, Info_inv *info)
{
    chan->pstOwner = info->iface;
    chan->hOwner = info->h;
    return 0;
}

int fn_iConnectTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, int arg)
{
    Info_inv info;
    Slot_inv s0;
    Slot_inv s1;
    int r = -1;

    if (fn_iFindChild_inv(&info, 2, arg) >= 0) {
        r = 0;
        info.iface->GetNumChild(info.h);
        info.iface->GetChild(info.h, &s0, 0);
        info.iface->GetChild(info.h, &s1, 1);
        chan->pstSpu = info.iface;
        chan->hSpuLeft = s0.x4;
        chan->hSpuRight = s1.x4;
    }
    return r;
}

int fn_iGetNumChildTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan)
{
    int n = 0;

    if (chan->pstChannel != 0) {
        n = chan->hLeft != 0;
        if (chan->hRight != 0)
            n++;
    }
    return n;
}

int fn_iGetChildTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, Info_inv *info, int idx)
{
    int r = -1;

    if (idx < fn_iGetNumChildTdstChannelMEMData_inv(chan)) {
        switch (idx) {
        case 0:
            info->kind = 1;
            info->h = chan->hLeft;
            info->iface = chan->pstChannel;
            break;
        case 1:
            info->kind = 1;
            info->h = chan->hRight;
            info->iface = chan->pstChannel;
            break;
        }
        r = 0;
    }
    return r;
}

int fn_iNotifyTdstChannelMEMData(tdstChannelMEMData_inv *chan, SndItBaseEvent_inv event, Note_inv *note, int arg)
{
    int r = 0;
    int t;

    switch (event) {
    case E_SNDITBASE_ENDREACHED:
        r = !chan->f.b4;
        r = !chan->f.b5 && r;
        if (note->h == chan->hLeft) {
            chan->f.b2 = 1;
            chan->f.b4 = 1;
            chan->f.b5 = 0;
        }
        if (note->h == chan->hRight) {
            chan->f.b3 = 1;
            chan->f.b4 = 0;
            chan->f.b5 = 1;
        }
        if (!r) {
            if (chan->f.b11) {
                chan->f.b11 = 0;
                fn_u32AllocateTdstChannelMEMData_inv(chan, &chan->desc);
#ifdef PC_PORT
                pc_memset(&chan->desc, 0, sizeof(Desc_inv));
#else
                sysclib_14_inv(&chan->desc, 0, sizeof(Desc_inv));
#endif
            }
            if (chan->f.b10) {
                fn_iStartTdstChannelMEMData_inv(chan, chan->lStartPos);
                chan->f.b10 = 0;
            }
            chan->f.b4 = 0;
            chan->f.b5 = 0;
        } else {
#ifdef PC_PORT
            pc_log("fn_iNotifyTdstChannelMEMData E_SNDITBASE_ENDREACHED");
#else
            stdio_4_inv("fn_iNotifyTdstChannelMEMData E_SNDITBASE_ENDREACHED\n");
#endif
        }
        break;
    case E_SNDITBASE_READY_inv:
        if (note->h == chan->hLeft)
            chan->f.b6 = 1;
        if (note->h == chan->hRight)
            chan->f.b7 = 1;
        t = chan->f.b6;
        t = chan->f.b7 && t;
        t = !chan->f.b12 && t;
        if (t) {
            chan->f.b12 = 1;
            r = 1;
        }
        break;
    case E_SNDITBASE_STOPPED_inv:
        if (note->h == chan->hLeft)
            chan->f.b8 = 1;
        if (note->h == chan->hRight)
            chan->f.b9 = 1;
        t = chan->f.b8;
        t = chan->f.b9 && t;
        t = !chan->f.b13 && t;
        if (t) {
            chan->f.b13 = 1;
            r = 1;
        }
        break;
    }
    if (r)
        fn_iCallOwnerTdstChannelMEMData_inv(chan, (int)event, arg);
    return 0;
}

int fn_iGetPosTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan, unsigned int *out)
{
    int r = -1;
    unsigned int a;
    unsigned int b;

    if (chan->pstChannel->u.channel.GetPos(chan->hLeft, (int *)&a) >= 0 && chan->pstChannel->u.channel.GetPos(chan->hRight, (int *)&b) >= 0) {
        r = 0;
        *out = a < b ? a : b;
    }
    return r;
}

void func_00005BA0(void) {
}

void fn_vResetTdstChannelMEMData_inv(tdstChannelMEMData_inv *chan)
{
    chan->f.b1 = 0;
    chan->f.b2 = 0;
    chan->f.b3 = 0;
    chan->f.b4 = 0;
    chan->f.b5 = 0;
    chan->f.b6 = 0;
    chan->f.b7 = 0;
    chan->f.b8 = 0;
    chan->f.b9 = 0;
    chan->f.b10 = 0;
    chan->f.b11 = 0;
    chan->f.b12 = 0;
    chan->f.b13 = 0;
    chan->f.b14 = 0;
    chan->pstOwner = 0;
    chan->hOwner = 0;
    chan->pstChannel = 0;
    chan->hLeft = 0;
    chan->hRight = 0;
    chan->pstSpu = 0;
    chan->hSpuLeft = 0;
    chan->hSpuRight = 0;
    chan->lStartPos = 0;
    chan->f.bAllocated = 0;
}
