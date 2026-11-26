#include "include_asm.h"
#include "sndspustreamstereo_inv.h"

typedef struct {
    int x0;
    void *h;
} Note_inv;

extern int fn_bGetStereo_inv(void);

static inline void CopyOut_inv(tdstOutParam_inv *d, tdstOutParam_inv *s)
{
    d->pitch = s->pitch;
    d->left = s->left;
    d->right = s->right;
}

int fn_u32DesallocateTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e);

int fn_iConstructTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, int x)
{
    e->lInterface = x;
    e->pstSpu = 0;
    e->hLeft = 0;
    e->hRight = 0;
    e->pstOwner = 0;
    e->hOwner = 0;
    e->f.bAllocated = 0;
    e->f.b1 = 0;
    e->f.b2 = 0;
    e->f.b3 = 0;
    e->f.b4 = 0;
    e->f.b5 = 0;
    e->f.b6 = 0;
    e->f.b7 = 0;
    e->f.b8 = 0;
    e->f.b9 = 0;
    e->f.b10 = 0;
    e->f.b11 = 0;
    e->f.b12 = 0;
    e->f.b13 = 0;
    return 0;
}

int fn_iDestructTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    if (e->hLeft != 0)
        e->pstSpu->DesallocateVoice(e->hLeft);
    if (e->hRight != 0)
        e->pstSpu->DesallocateVoice(e->hRight);
    e->pstSpu = 0;
    e->hLeft = 0;
    e->hRight = 0;
    e->pstOwner = 0;
    e->hOwner = 0;
    e->f.bAllocated = 0;
    e->f.b1 = 0;
    e->f.b2 = 0;
    e->f.b3 = 0;
    e->f.b4 = 0;
    e->f.b5 = 0;
    e->f.b6 = 0;
    e->f.b7 = 0;
    e->f.b8 = 0;
    e->f.b9 = 0;
    e->f.b10 = 0;
    e->f.b11 = 0;
    e->f.b12 = 0;
    e->f.b13 = 0;
    return 0;
}

int fn_iStartTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, int x)
{
    int r = -1;
    int t;

    if (!e->f.b1) {
        if (e->pstSpu->u.spu.Start(e->hLeft, x) >= 0 && e->pstSpu->u.spu.Start(e->hRight, x) >= 0)
            r = 0;
    } else {
        t = e->f.b8;
        t = e->f.b4 || t;
        if (t) {
            if (e->pstSpu->u.spu.Start(e->hLeft, x) >= 0) {
                r = 0;
                e->f.b4 = 0;
                e->f.b8 = 0;
            }
        }
        t = e->f.b9;
        t = e->f.b5 || t;
        if (t) {
            if (e->pstSpu->u.spu.Start(e->hRight, x) >= 0) {
                r = 0;
                e->f.b5 = 0;
                e->f.b9 = 0;
            }
        }
    }
    e->f.b1 = 1;
    e->f.b12 = 0;
    e->f.b13 = 0;
    return r;
}

int fn_iCallOwnerTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (e->pstOwner != 0 && e->hOwner != 0) {
        InitMsg_inv(&msg, 2, e, 0);
        r = e->pstOwner->Notify(e->hOwner, event, &msg, arg);
    }
    return r;
}

int fn_iStopTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    if (e->f.b1) {
        if (!e->f.b4)
            e->pstSpu->u.spu.Stop(e->hLeft);
        if (!e->f.b5)
            e->pstSpu->u.spu.Stop(e->hRight);
    } else {
        fn_iCallOwnerTdstSpuStreamStereoData_inv(e, E_SNDITBASE_STOPPED_inv, 0);
    }
    return 0;
}

tdstSpuStreamStereoData_inv *fn_u32AllocateTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, Desc_inv *desc)
{
    char b1[40];
    Msg_inv msg;
    tdstSpuStreamStereoData_inv *ret;
    Iface_inv *iface;
    void *h1;
    void *h2;

    fn_vSplitTdstDesc_inv((Desc_inv *)b1, 0, desc);
    e->f.bAllocated = 1;
    ret = 0;
    if (!e->f.b1) {
        h1 = ret;
        h2 = ret;
        iface = fn_pstFindInterface_inv(2, b1, 0, 0);
        if (iface != 0) {
            h1 = iface->AllocateVoice(b1);
            h2 = iface->AllocateVoice(b1);
            if (h1 != 0 && h2 != 0) {
                msg.lInterface = e->lInterface;
                msg.kind = 2;
                msg.h = e;
                if (iface->SetOwner(h1, &msg) >= 0 && iface->SetOwner(h2, &msg) >= 0)
                    ret = e;
            }
        }
        e->pstSpu = iface;
        e->hLeft = h1;
        e->hRight = h2;
    }
    if (ret == 0)
        fn_u32DesallocateTdstSpuStreamStereoData_inv(e);
    return ret;
}

int fn_u32DesallocateTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    fn_iDestructTdstSpuStreamStereoData_inv(e);
    return 0;
}

int fn_iSetOwnerTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, Info_inv *info)
{
    e->pstOwner = info->iface;
    e->hOwner = info->h;
    return 0;
}

int fn_iConnectTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, void *x)
{
    int r = -1;

    if (e->pstSpu->Connect(e->hLeft, x) >= 0 && e->pstSpu->Connect(e->hRight, x) >= 0)
        r = 0;
    return r;
}

int fn_iGetNumChildTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    int n = 0;

    if (e->pstSpu != 0) {
        n = e->hLeft != 0;
        if (e->hRight != 0)
            n++;
    }
    return n;
}

int fn_iGetChildTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, Info_inv *info, int idx)
{
    int r = -1;

    if (idx < fn_iGetNumChildTdstSpuStreamStereoData_inv(e)) {
        switch (idx) {
        case 0:
            info->kind = 2;
            info->h = e->hLeft;
            info->iface = e->pstSpu;
            break;
        case 1:
            info->kind = 2;
            info->h = e->hRight;
            info->iface = e->pstSpu;
            break;
        }
        r = 0;
    }
    return r;
}

int fn_iNotifyTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, unsigned int event, Note_inv *note)
{
    switch (event) {
    case E_SNDITBASE_ENDREACHED:
        if (note->h == e->hLeft)
            e->f.b8 = 1;
        if (note->h == e->hLeft)
            e->f.b9 = 1;
        break;
    case E_SNDITBASE_READY_inv:
        if (note->h == e->hLeft)
            e->f.b2 = 1;
        if (note->h == e->hRight)
            e->f.b3 = 1;
        break;
    case E_SNDITBASE_STOPPED_inv:
        if (note->h == e->hLeft)
            e->f.b4 = 1;
        if (note->h == e->hRight)
            e->f.b5 = 1;
        break;
    case E_SNDITBASE_BLOCKREAD_inv:
        if (note->h == e->hLeft)
            e->f.b6 = 1;
        if (note->h == e->hRight)
            e->f.b7 = 1;
        break;
    }
    return 0;
}

tdstSpuStreamStereoData_inv *fn_pSetParamTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, tdstOutParam_inv *in)
{
    tdstOutParam_inv a;
    tdstOutParam_inv b;

    CopyOut_inv(&a, in);
    b.pitch = in->pitch;
    b.left = in->left;
    b.right = in->right;
    if (fn_bGetStereo_inv()) {
        a.right = 0;
        b.left = 0;
    }
    if (e->hLeft != 0)
        e->pstSpu->u.spu.SetParam(e->hLeft, &a);
    if (e->hRight != 0)
        e->pstSpu->u.spu.SetParam(e->hRight, &b);
    return e;
}

int fn_iGetPosTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e, unsigned int *out)
{
    int r = -1;
    unsigned int a;
    unsigned int b;

    if (e->pstSpu->u.spu.GetPos(e->hLeft, (int *)&a) >= 0 && e->pstSpu->u.spu.GetPos(e->hRight, (int *)&b) >= 0) {
        *out = a < b ? a : b;
        r = 0;
    }
    return r;
}

int fn_iPauseTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    int r = -1;

    if (e->pstSpu->u.spu.Pause(e->hLeft) >= 0 && e->pstSpu->u.spu.Pause(e->hRight) >= 0) {
        r = 0;
        e->f.b10 = 1;
    }
    return r;
}

int fn_iResumeTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    int r = -1;

    if (e->pstSpu->u.spu.Resume(e->hLeft) >= 0 && e->pstSpu->u.spu.Resume(e->hRight) >= 0) {
        r = 0;
        e->f.b10 = 0;
    }
    return r;
}

void fn_vOperateCallTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    int t;

    if (e->f.bAllocated) {
        t = e->f.b2;
        t = e->f.b3 && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamStereoData_inv(e, E_SNDITBASE_READY_inv, 0) >= 0)
                e->f.b11 = 1;
        }
        t = e->f.b6;
        t = e->f.b7 && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamStereoData_inv(e, E_SNDITBASE_BLOCKREAD_inv, 0) >= 0) {
                e->f.b6 = 0;
                e->f.b7 = 0;
            }
        }
        t = e->f.b4;
        t = e->f.b5 && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamStereoData_inv(e, E_SNDITBASE_STOPPED_inv, 0) >= 0)
                e->f.b12 = 1;
        }
        t = e->f.b8;
        t = e->f.b9 && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamStereoData_inv(e, E_SNDITBASE_ENDREACHED, 0) >= 0)
                e->f.b13 = 1;
        }
    }
}

void fn_vResetTdstSpuStreamStereoData_inv(tdstSpuStreamStereoData_inv *e)
{
    e->pstSpu = 0;
    e->hLeft = 0;
    e->hRight = 0;
    e->pstOwner = 0;
    e->hOwner = 0;
    e->f.bAllocated = 0;
    e->f.b1 = 0;
    e->f.b2 = 0;
    e->f.b3 = 0;
    e->f.b4 = 0;
    e->f.b5 = 0;
    e->f.b6 = 0;
    e->f.b7 = 0;
    e->f.b8 = 0;
    e->f.b9 = 0;
    e->f.b10 = 0;
    e->f.b11 = 0;
    e->f.b12 = 0;
    e->f.b13 = 0;
}
