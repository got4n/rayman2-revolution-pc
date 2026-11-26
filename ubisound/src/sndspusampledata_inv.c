#include "include_asm.h"
#include "sndspusample_inv.h"

typedef int (*IrqFn_inv)(void *arg);

typedef struct {
    void *arg;
    IrqFn_inv fn;
} IrqEntry_inv;

extern IrqEntry_inv a_stSpuCallback_inv[3];
extern int g_lSpuCallbackInit_inv;
extern int libsd_22_inv();

typedef struct {
    unsigned short op;
    unsigned short reg;
    int value;
} SdBatch_inv;

extern int libsd_15_inv(SdBatch_inv *b, int a, int n);
extern int libsd_10_inv(unsigned short a);

int fn_iCallOwnerTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, int event, int arg);
#ifdef PC_PORT
#define PC_VOICE_inv(e) ((e)->lVoice * 24 + (e)->lCore)
#endif
extern unsigned short libsd_8_inv(unsigned short reg);
extern void libsd_5_inv(unsigned short reg, int value);

static inline void CopyElemCTrip_inv(ElemCTrip_inv *d, ElemCTrip_inv *s)
{
    d->pitch = s->pitch;
    d->left = s->left;
    d->right = s->right;
}

static inline void ClearElemCTrip_inv(ElemCTrip_inv *t)
{
    t->pitch = 0;
    t->left = 0;
    t->right = 0;
}

static inline void ClearElemC_inv(tdstSpuSampleData_inv *e)
{
    e->pstOwner = 0;
    e->hOwner = 0;
    e->lRate = 0;
    e->f.bStarted = 0;
    e->f.bReady = 0;
    e->f.bEndReached = 0;
    e->f.bStopped = 0;
    e->f.bReadyNotified = 0;
    e->f.bStopNotified = 0;
    e->f.bEndNotified = 0;
    e->f.bPaused = 0;
    ClearElemCTrip_inv(&e->stParam);
    e->f.bAllocated = 0;
    e->x24 = 0;
    e->lSpuAddr = 0;
}

int fn_iDispatchSpuCallback_inv(void)
{
    IrqEntry_inv *e;

    for (e = a_stSpuCallback_inv; e < &a_stSpuCallback_inv[3]; e++) {
        if (e->fn != 0)
            e->fn(e->arg);
    }
    return 1;
}

int fn_iAddSpuCallback_inv(IrqFn_inv fn, void *arg)
{
    IrqEntry_inv *e;
    int r;

    if (g_lSpuCallbackInit_inv == 0) {
#ifdef PC_PORT
        pc_memset(a_stSpuCallback_inv, 0, sizeof(a_stSpuCallback_inv));
#else
        sysclib_14_inv(a_stSpuCallback_inv, 0, sizeof(a_stSpuCallback_inv));
#endif
#ifdef PC_PORT
#else
        libsd_22_inv(fn_iDispatchSpuCallback_inv);
#endif
        g_lSpuCallbackInit_inv++;
    }
    r = -1;
    for (e = a_stSpuCallback_inv; e < &a_stSpuCallback_inv[3]; e++) {
        if (e->fn == 0) {
            r = 0;
            e->arg = arg;
            e->fn = fn;
            break;
        }
    }
    return r;
}

int fn_iRemoveSpuCallback_inv(IrqFn_inv fn)
{
    IrqEntry_inv *e;
    int r = -1;

    for (e = a_stSpuCallback_inv; e < &a_stSpuCallback_inv[3]; e++) {
        if (e->fn == fn) {
            e->fn = 0;
            r = 0;
            break;
        }
    }
    return r;
}

int fn_iConstructTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, int a, int b)
{
    e->lVoice = a;
    e->lCore = b;
    ClearElemC_inv(e);
    return 0;
}

int fn_iDestructTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
    e->lVoice = -1;
    e->lCore = -1;
    ClearElemC_inv(e);
    return 0;
}

int fn_iStartTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
#ifdef PC_PORT
    e->f.bStarted = 1;
    e->f.bEndReached = 0;
    e->f.bStopped = 0;
    e->f.bStopNotified = 0;
    pc_channel_play(PC_VOICE_inv(e), (const void *)e->lSpuAddr);
    e->f.bReady = 1;
    return 0;
#else
    SdBatch_inv b[4] = {
        { 1, e->lVoice | (e->lCore << 1) | 0x300, 15 },
        { 1, e->lVoice | (e->lCore << 1) | 0x400, 8128 },
        { 3, e->lVoice | (e->lCore << 1) | 0x2040, e->lSpuAddr },
        { 2, e->lVoice | 0x1500, 1 << e->lCore },
    };

    e->f.bStarted = 1;
    e->f.bEndReached = 0;
    e->f.bStopped = 0;
    e->f.bStopNotified = 0;
    libsd_15_inv(b, 0, 4);
    e->f.bReady = 1;
    return 0;
#endif
}

int fn_iStopTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
#ifdef PC_PORT
    e->f.bStopped = 1;
    pc_channel_volume(PC_VOICE_inv(e), 0, 0);
    pc_channel_stop(PC_VOICE_inv(e));
    return 0;
#else
    SdBatch_inv b[3] = {
        { 1, e->lVoice | (e->lCore << 1), 0 },
        { 1, e->lVoice | (e->lCore << 1) | 0x100, 0 },
        { 2, e->lVoice | 0x1600, 1 << e->lCore },
    };

    e->f.bStopped = 1;
    libsd_15_inv(b, 0, 3);
    return 0;
#endif
}

tdstSpuSampleData_inv *fn_u32AllocateTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, Desc_inv *d)
{
    e->f.bAllocated = 1;
    e->lRate = d->h.uwRate;
    e->x24 = d->u.a.w0;
    e->lSpuAddr = d->u.a.w1;
    return e;
}

int fn_u32DesallocateTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
    fn_iStopTdstSpuSampleData_inv(e);
    ClearElemC_inv(e);
    return 0;
}

int fn_iSetOwnerTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, Info_inv *info)
{
    e->pstOwner = info->iface;
    e->hOwner = info->h;
    return 0;
}

void fn_vOperateCallTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
    int t;
    int m;

    if (e->f.bAllocated) {
        t = e->f.bReady;
        t = !e->f.bReadyNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuSampleData_inv(e, E_SNDITBASE_READY_inv, 0) >= 0)
                e->f.bReadyNotified = 1;
        }
        t = e->f.bReadyNotified;
        t = !e->f.bStopNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuSampleData_inv(e, E_SNDITBASE_STOPPED_inv, 0) >= 0)
                e->f.bStopNotified = 1;
        }
        t = e->f.bReadyNotified;
#ifdef PC_PORT
        (void)m;
        t = pc_channel_ended(PC_VOICE_inv(e)) && t;
#else
        t = (((int (*)(unsigned short))libsd_8_inv)(e->lVoice | 0x1700) & (m = 1 << e->lCore)) && t;
#endif
        if (t) {
            e->f.bEndReached = 1;
            if (!e->f.bEndNotified) {
                if (fn_iCallOwnerTdstSpuSampleData_inv(e, E_SNDITBASE_ENDREACHED, 0) >= 0)
                    e->f.bEndNotified = 1;
            }
        }
    }
}

int fn_iConnectTdstSpuSampleData_inv(void)
{
    return 0;
}

int fn_iNotifyTdstSpuSampleData_inv(void)
{
    return 0;
}

int fn_iCallOwnerTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (e->pstOwner != 0 && e->hOwner != 0) {
        InitMsg_inv(&msg, 1, e, 0);
        r = e->pstOwner->Notify(e->hOwner, event, &msg, arg);
    }
    return r;
}

tdstSpuSampleData_inv *fn_pSetParamTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, ElemCTrip_inv *t)
{
    if (t->left != e->stParam.left && t->right != e->stParam.right) {
#ifdef PC_PORT
        if (t->pitch != e->stParam.pitch) {
            pc_channel_rate(PC_VOICE_inv(e), t->pitch);
            e->stParam.pitch = t->pitch;
        }
        pc_channel_volume(PC_VOICE_inv(e), t->left, t->right);
        CopyElemCTrip_inv(&e->stParam, t);
#else
        SdBatch_inv b[2] = {
            { 1, e->lVoice | (e->lCore << 1), t->left },
            { 1, e->lVoice | (e->lCore << 1) | 0x100, t->right },
        };

        if (t->pitch != e->stParam.pitch) {
            libsd_5_inv(e->lVoice | (e->lCore << 1) | 0x200, ((unsigned int)t->pitch * 22369) >> 18);
            e->stParam.pitch = t->pitch;
        }
        libsd_15_inv(b, 0, 2);
        CopyElemCTrip_inv(&e->stParam, t);
#endif
    }
    return e;
}

int fn_iGetPosTdstSpuSampleData_inv(tdstSpuSampleData_inv *e, int *out)
{
#ifdef PC_PORT
    *out = (int)pc_channel_position(PC_VOICE_inv(e)) - e->lSpuAddr;
#else
    *out = libsd_10_inv(e->lVoice | (e->lCore << 1) | 0x2240) - e->lSpuAddr;
#endif
    return 0;
}

int fn_iPauseTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
#ifdef PC_PORT
    pc_channel_volume(PC_VOICE_inv(e), 0, 0);
#else
    SdBatch_inv b[2] = {
        { 1, e->lVoice | (e->lCore << 1), 0 },
        { 1, e->lVoice | (e->lCore << 1) | 0x100, 0 },
    };

    libsd_15_inv(b, 0, 2);
#endif
    e->f.bPaused = 1;
    return 0;
}

int fn_iResumeTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
#ifdef PC_PORT
    pc_channel_volume(PC_VOICE_inv(e), e->stParam.left, e->stParam.right);
#else
    SdBatch_inv b[2] = {
        { 1, e->lVoice | (e->lCore << 1), e->stParam.left },
        { 1, e->lVoice | (e->lCore << 1) | 0x100, e->stParam.right },
    };

    libsd_15_inv(b, 0, 2);
#endif
    e->f.bPaused = 0;
    return 0;
}

void fn_vResetTdstSpuSampleData_inv(tdstSpuSampleData_inv *e)
{
    ClearElemC_inv(e);
}
