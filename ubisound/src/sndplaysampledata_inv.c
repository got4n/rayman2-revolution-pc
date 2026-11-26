#include "include_asm.h"
#include "sndplaysample_inv.h"

tdstPlaySampleData_inv *fn_u32DesallocateTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj);
typedef struct {
    int x0;
    void *h;
} Note_inv;

extern void fn_vGetOutParam_inv(tdstOutParam_inv *out, tdstPlayParam_inv *mix, DescHead_inv *src);
extern int fn_bGetStereo_inv(void);
extern int fn_iRemoteCall(int a, int b, int c, int d, void *p, int n);

static inline void ClearParam_inv(tdstPlayParam_inv *t)
{
    t->lPitch = 0;
    t->ucVolume = 0;
    t->ucPan = 0;
    t->c6 = 0;
    t->c7 = 0;
}

static inline void ResetObj_inv(tdstPlaySampleData_inv *obj)
{
    obj->f.bStereo = 0;
    obj->f.bStarted = 0;
    obj->f.bDesallocating = 0;
    obj->f.bReady = 0;
    obj->f.bLeftStarted = 0;
    obj->f.bRightStarted = 0;
    obj->f.bLeftStopSent = 0;
    obj->f.bRightStopSent = 0;
    obj->f.bLeftReady = 0;
    obj->f.bRightReady = 0;
    obj->f.bLeftStopped = 0;
    obj->f.bRightStopped = 0;
    obj->f.bLeftEnd = 0;
    obj->f.bRightEnd = 0;
    obj->f.bCallbackSent = 0;
    obj->f.bPaused = 0;
    obj->f.bInNotify = 0;
    ClearHead_inv(&obj->desc.h);
#ifdef PC_PORT
    pc_memset(&obj->desc.u, 0, 16);
    pc_memset(&obj->stPendingBlock, 0, 16);
#else
    sysclib_14_inv(&obj->desc.u, 0, 16);
    sysclib_14_inv(&obj->stPendingBlock, 0, 16);
#endif
    obj->lCallbackArg = 0;
    obj->pstSpu = 0;
    obj->hLeft = 0;
    obj->hRight = 0;
    obj->x4c = 0;
    ClearParam_inv(&obj->stParam);
    obj->f.bAllocated = 0;
}

static inline int InitObj_inv(tdstPlaySampleData_inv *obj)
{
    obj->f.bStereo = 0;
    obj->f.bStarted = 0;
    obj->f.bDesallocating = 0;
    obj->f.bReady = 0;
    obj->f.bLeftStarted = 0;
    obj->f.bRightStarted = 0;
    obj->f.bLeftStopSent = 0;
    obj->f.bRightStopSent = 0;
    obj->f.bLeftReady = 0;
    obj->f.bRightReady = 0;
    obj->f.bLeftStopped = 0;
    obj->f.bRightStopped = 0;
    obj->f.bLeftEnd = 0;
    obj->f.bRightEnd = 0;
    obj->f.bCallbackSent = 0;
    obj->f.bPaused = 0;
    obj->f.bInNotify = 0;
    ClearHead_inv(&obj->desc.h);
#ifdef PC_PORT
    pc_memset(&obj->desc.u, 0, 16);
    pc_memset(&obj->stPendingBlock, 0, 16);
#else
    sysclib_14_inv(&obj->desc.u, 0, 16);
    sysclib_14_inv(&obj->stPendingBlock, 0, 16);
#endif
    obj->lCallbackArg = 0;
    obj->pstSpu = 0;
    obj->hLeft = 0;
    obj->hRight = 0;
    obj->x4c = 0;
    ClearParam_inv(&obj->stParam);
    obj->f.bAllocated = 0;
    return 0;
}

static inline void CopyParam_inv(tdstPlayParam_inv *d, tdstPlayParam_inv *s)
{
    d->lPitch = s->lPitch;
    d->ucVolume = s->ucVolume;
    d->ucPan = s->ucPan;
    d->c6 = s->c6;
    d->c7 = s->c7;
}

void fn_vCloseTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    if (obj->hLeft != 0)
        obj->pstSpu->DesallocateVoice(obj->hLeft);
    if (obj->hRight != 0)
        obj->pstSpu->DesallocateVoice(obj->hRight);
    ResetObj_inv(obj);
}

int fn_iConstructTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, int x)
{
    obj->lInterface = x;
    return InitObj_inv(obj);
}

int fn_iDestructTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    obj->lInterface = 0;
    return InitObj_inv(obj);
}

tdstPlaySampleData_inv *fn_u32AllocateTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, Desc_inv *desc)
{
    char b1[40];
    char b2[40];
    Msg_inv msg;
    tdstPlaySampleData_inv *ret;
    Iface_inv *iface;
    int ok;
    int n;

    ret = 0;
    obj->f.bAllocated = 1;
    iface = fn_pstFindInterface_inv(2, desc, 0, 0);
    if (iface != 0) {
        n = desc->h.cNbChannels;
        if (iface->GetNumFreeVoice() >= n) {
            fn_vSplitTdstDesc_inv((Desc_inv *)b1, (Desc_inv *)b2, desc);
            obj->f.bStereo = desc->h.cNbChannels == 2;
            obj->hLeft = iface->AllocateVoice(b1);
            ok = obj->hLeft != 0;
            if (obj->f.bStereo < 0) {
                obj->hRight = iface->AllocateVoice(b2);
                ok = obj->hRight != 0 && ok;
            }
            if (ok) {
                msg.lInterface = obj->lInterface;
                msg.kind = 4;
                msg.h = obj;
                CopyHead_inv(&obj->desc.h, &desc->h);
                obj->desc.u = desc->u;
                obj->pstSpu = iface;
                ok = iface->SetOwner(obj->hLeft, &msg) >= 0 && iface->Connect(obj->hLeft, &msg) >= 0;
                if (obj->f.bStereo < 0)
                    ok = iface->SetOwner(obj->hRight, &msg) >= 0 && iface->Connect(obj->hRight, &msg) >= 0 && ok;
                if (ok)
                    ret = obj;
            }
        }
    }
    if (ret == 0)
        fn_u32DesallocateTdstPlaySampleData_inv(obj);
    return ret;
}

tdstPlaySampleData_inv *fn_u32DesallocateTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    tdstPlaySampleData_inv *ret = obj;
    int c;
    int d;

    if (obj->f.bAllocated) {
        obj->f.bDesallocating = 1;
        if (!obj) ret = 0;
        c = obj->f.bLeftStarted;
        if (obj && c) ret = 0;
        if (obj && (d = obj->f.bRightStarted, obj) && d) ret = 0;
        d = obj->f.bRightStarted;
        if (obj && d) ret = 0;
        if (obj && (d = obj->f.bLeftStarted, obj) && d) ret = 0;
        fn_iStopTdstPlaySampleData_inv(obj);
        fn_vCloseTdstPlaySampleData_inv(obj);
        ret = 0;
    }
    return ret;
}

int fn_iStopTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    if (!obj->f.bLeftStopSent) {
        obj->f.bLeftStopSent = 1;
        if (obj->hLeft != 0)
            obj->pstSpu->u.spu.Stop(obj->hLeft);
    }
    if (!obj->f.bRightStopSent) {
        obj->f.bRightStopSent = 1;
        if (obj->hRight != 0)
            obj->pstSpu->u.spu.Stop(obj->hRight);
    }
    return 0;
}

int fn_iSetOwnerTdstPlaySampleData_inv(void)
{
    return 0;
}

int fn_iConnectTdstPlaySampleData_inv(void)
{
    return 0;
}

int fn_iGetNumChildTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    int n = 0;

    if (obj->pstSpu != 0) {
        n = obj->hLeft != 0;
        if (obj->hRight != 0)
            n++;
    }
    return n;
}

int fn_iGetChildTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, Info_inv *info, int idx)
{
    int r = -1;

    if (idx < fn_iGetNumChildTdstPlaySampleData_inv(obj)) {
        switch (idx) {
        case 0:
            info->kind = 2;
            info->h = obj->hLeft;
            info->iface = obj->pstSpu;
            break;
        case 1:
            info->kind = 2;
            info->h = obj->hRight;
            info->iface = obj->pstSpu;
            break;
        }
        r = 0;
    }
    return r;
}

int fn_iOnEndReachedTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, Note_inv *note, int arg)
{
    if (note->h == obj->hLeft)
        obj->f.bLeftEnd = 1;
    if (note->h == obj->hRight)
        obj->f.bRightEnd = 1;
    return 0;
}

int fn_iOnReadyTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, Note_inv *note, int arg)
{
    int r;

    if (note->h == obj->hLeft)
        obj->f.bLeftReady = 1;
    if (note->h == obj->hRight)
        obj->f.bRightReady = 1;
    r = obj->f.bLeftReady;
    r = obj->f.bRightReady && r;
    if (r)
        obj->f.bReady = 1;
    return 0;
}

int fn_iOnStoppedTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, Note_inv *note, int arg)
{
    int c;

    if (note->h == obj->hLeft)
        obj->f.bLeftStopped = 1;
    c = note->h == obj->hRight;
    if (c)
        obj->f.bRightStopped = 1;
    c = *(volatile long *)&obj->f;
    return 0;
}

int fn_iNotifyTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, unsigned int event, Note_inv *note, int arg)
{
    int r = -1;

    if (!obj->f.bInNotify) {
        obj->f.bInNotify = 1;
        switch (event) {
        case E_SNDITBASE_ENDREACHED:
            r = fn_iOnEndReachedTdstPlaySampleData_inv(obj, note, arg);
            break;
        case E_SNDITBASE_READY_inv:
            r = fn_iOnReadyTdstPlaySampleData_inv(obj, note, arg);
            break;
        case E_SNDITBASE_STOPPED_inv:
            r = fn_iOnStoppedTdstPlaySampleData_inv(obj, note, arg);
            break;
        }
        obj->f.bInNotify = 0;
    }
    return r;
}

int fn_iStartTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, tdstPlayParam_inv *mix)
{
    tdstOutParam_inv a;
    tdstOutParam_inv b;

    obj->f.bStarted = 1;
    fn_vGetOutParam_inv(&a, mix, &obj->desc.h);
    fn_vGetOutParam_inv(&b, mix, &obj->desc.h);
    if (fn_bGetStereo_inv() && obj->f.bStereo) {
        a.right = 0;
        b.left = 0;
    }
    if (obj->hLeft != 0) {
        obj->pstSpu->u.spu.SetParam(obj->hLeft, &a);
        if (obj->pstSpu->u.spu.Start(obj->hLeft, 0) >= 0)
            obj->f.bLeftStarted = 1;
    }
    if (obj->hRight != 0) {
        obj->pstSpu->u.spu.SetParam(obj->hRight, &b);
        if (obj->pstSpu->u.spu.Start(obj->hRight, 0) >= 0)
            obj->f.bRightStarted = 1;
    }
    obj->f.bLeftStopSent = 0;
    obj->f.bRightStopSent = 0;
    CopyParam_inv(&obj->stParam, mix);
    return 0;
}

void fn_vApplyParamTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, tdstPlayParam_inv *mix)
{
    tdstOutParam_inv a;
    tdstOutParam_inv b;

    fn_vGetOutParam_inv(&a, mix, &obj->desc.h);
    fn_vGetOutParam_inv(&b, mix, &obj->desc.h);
    if (fn_bGetStereo_inv() && obj->f.bStereo) {
        a.right = 0;
        b.left = 0;
    }
    if (obj->hLeft != 0)
        obj->pstSpu->u.spu.SetParam(obj->hLeft, &a);
    if (obj->hRight != 0)
        obj->pstSpu->u.spu.SetParam(obj->hRight, &b);
}

tdstPlaySampleData_inv *fn_pSetParamTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, tdstPlayParam_inv *t)
{
    tdstPlaySampleData_inv *ret = 0;

    if (obj->pstSpu != 0 && !obj->f.bPaused) {
        CopyParam_inv(&obj->stParam, t);
        fn_vApplyParamTdstPlaySampleData_inv(obj, t);
        ret = obj;
    }
    return ret;
}

tdstPlaySampleData_inv *fn_pPushNextBlockTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, int which, Quad_inv *q)
{
    tdstPlaySampleData_inv *ret = 0;

    if (obj->f.bAllocated) {
        if (q != 0)
            obj->stPendingBlock = *q;
        else
#ifdef PC_PORT
            pc_memset(&obj->stPendingBlock, 0, 16);
#else
            sysclib_14_inv(&obj->stPendingBlock, 0, 16);
#endif
        ret = obj;
    }
    return ret;
}

int fn_iSetCallbackArgTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj, int x)
{
    int r = -1;

    if (obj->f.bAllocated) {
        obj->lCallbackArg = x;
        r = 0;
    }
    return r;
}

int fn_iGetPosTdstPlaySampleData_inv(void)
{
    return -1;
}

int fn_iPauseTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    int r = -1;

    if (obj->f.bAllocated) {
        if (!obj->f.bPaused) {
            obj->f.bPaused = 1;
            if (obj->hLeft != 0)
                obj->pstSpu->u.spu.Pause(obj->hLeft);
            if (obj->hRight != 0)
                obj->pstSpu->u.spu.Pause(obj->hRight);
        }
        r = 0;
    }
    return r;
}

int fn_iResumeTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    int r = -1;

    if (obj->f.bAllocated) {
        if (obj->f.bPaused) {
            if (obj->hLeft != 0)
                obj->pstSpu->u.spu.Resume(obj->hLeft);
            if (obj->hRight != 0)
                obj->pstSpu->u.spu.Resume(obj->hRight);
            obj->f.bPaused = 0;
        }
        r = 0;
    }
    return r;
}

void fn_vOperateCallTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    int t;
    int x;

    if (obj->f.bAllocated) {
        t = obj->f.bLeftEnd;
        if (obj->f.bStereo)
            t = obj->f.bRightEnd && t;
        t = !obj->f.bCallbackSent && t;
        t = !obj->f.bDesallocating && t;
        if (t) {
            obj->f.bCallbackSent = 1;
            x = obj->lCallbackArg;
            fn_iRemoteCall(0x30000, 1, 0, 0, &x, 4);
        }
    }
}

void fn_vResetTdstPlaySampleData_inv(tdstPlaySampleData_inv *obj)
{
    ResetObj_inv(obj);
}
