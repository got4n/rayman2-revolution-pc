#include "include_asm.h"
#include "sndspustream_inv.h"

static inline void CopyTrip_inv(Trip_inv *d, Trip_inv *s)
{
    d->pitch = s->pitch;
    d->left = s->left;
    d->right = s->right;
}

static inline void ClearTrip_inv(Trip_inv *t)
{
    t->pitch = 0;
    t->left = 0;
    t->right = 0;
}

static inline void ClearStream_inv(tdstSpuStreamData_inv *st)
{
    st->pstOwner = 0;
    st->u32Fetched = 0;
    st->u32LastPos = 0;
    st->hOwner = 0;
    st->hChannel = 0;
    st->pstChannel = 0;
    st->f.bStarted = 0;
    st->f.bReady = 0;
    st->f.bStopped = 0;
    st->f.bEndReached = 0;
    st->f.bStopRequested = 0;
    st->f.bReadyNotified = 0;
    st->f.bStopNotified = 0;
    st->f.bEndNotified = 0;
    st->f.bPaused = 0;
    ClearTrip_inv(&st->stParam);
}

typedef struct {
    unsigned short op;
    unsigned short reg;
    int value;
} SdBatch_inv;

extern int libsd_15_inv(SdBatch_inv *b, int a, int n);
extern int fn_u32ClearBlocks_inv(void *p, int size);
extern void fn_vMarkLoopBlocks_inv(unsigned char *buf, int size);
extern void fn_vMarkLastBlocks_inv(unsigned char *buf, int size);
int fn_iCallOwnerTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, int event, int arg);
#ifdef PC_PORT
#define PC_VOICE_inv(st) ((st)->lVoice * 24 + (st)->lCore)

void fn_vPlayTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    pc_channel_play(PC_VOICE_inv(st), (const void *)st->lSpuAddr);
}

void fn_vStopTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    pc_channel_stop(PC_VOICE_inv(st));
}
#endif

extern int fn_iFindChild_inv(Info_inv *info, int kind, int arg);
extern void *fn_pvAllocSpu_inv(int size);
extern void fn_vFreeSpu_inv(void *p);
extern int libsd_10_inv(unsigned short a);
extern int libsd_17_inv();

static inline void MarkEnd_inv(unsigned char *p, int size)
{
    p[size - 15] = 3;
}

static inline unsigned short NextIdx_inv(unsigned short i, unsigned short n)
{
    i++;
    if (i >= n)
        i = 1;
    return i;
}

int fn_iConstructTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, int n, int a, int b)
{
    int r = -1;
    int size = n << 1;
    void *buf = fn_pvAllocSpu_inv(size);

    if (buf != 0) {
        r = 0;
        ClearStream_inv(st);
        st->x1c = 1;
        st->lVoice = a;
        st->lCore = b;
        st->uwBlockTicks = 0;
        st->lBufOffset = 0;
        st->lHalfSize = n;
        st->lSpuAddr = (int)buf;
        st->lBufSize = size;
        st->f.bDesallocating = 0;
        st->f.bAllocated = 0;
    }
    return r;
}

int fn_iDestructTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    fn_vFreeSpu_inv((void *)st->lSpuAddr);
    st->lVoice = 0;
    st->lCore = 0;
    st->x1c = 0;
    st->uwBlockTicks = 0;
    st->lBufOffset = 0;
    st->lHalfSize = 0;
    st->lSpuAddr = 0;
    st->lBufSize = 0;
    ClearStream_inv(st);
    st->f.bDesallocating = 0;
    st->f.bAllocated = 0;
    return 0;
}

void fn_vConstructTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    st->x1c = NextIdx_inv(st->x1c, st->uwBlockTicks);
    if (st->x1c == 1)
        {
            int a = st->lBufOffset;

            st->lBufOffset = st->lHalfSize - a;
        }
}

int fn_iSetupVoiceTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, unsigned int rate)
{
    unsigned int pitch = (rate * 22369) >> 18;

    st->uwBlockTicks = 1881 / pitch;
#ifdef PC_PORT
    pc_channel_volume(PC_VOICE_inv(st), 0, 0);
    pc_channel_rate(PC_VOICE_inv(st), rate);
#else
    {
        SdBatch_inv b[6] = {
            { 1, st->lVoice | (st->lCore << 1), 0 },
            { 1, st->lVoice | (st->lCore << 1) | 0x100, 0 },
            { 1, st->lVoice | (st->lCore << 1) | 0x200, pitch },
            { 1, st->lVoice | (st->lCore << 1) | 0x300, 15 },
            { 1, st->lVoice | (st->lCore << 1) | 0x400, 0x1FC0 },
            { 3, st->lVoice | (st->lCore << 1) | 0x2040, st->lSpuAddr },
        };

        libsd_15_inv(b, 0, 6);
    }
#endif
    return 0;
}

unsigned int fn_u32FetchTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, Buf_inv *buf)
{
    unsigned int r = 0;
    int t;

    if (st->f.bStarted) {
        t = !st->f.bPaused;
        t = !st->f.bStopRequested && t;
        t = !st->f.bDesallocating && t;
        if (t) {
            if (!st->f.bEndReached) {
                st->f.bReady = 1;
                r = st->pstChannel->u.channel.Read(st->hChannel, st, 1, buf);
                st->u32Fetched += r;
            }
        } else {
            t = st->f.b6;
            t = st->f.b7 && t;
            t = !st->f.bStopNotified && t;
            t = st->f.bStopRequested && t;
            if (t)
                st->f.bStopped = 1;
        }
        if (r < buf->size) {
            fn_u32ClearBlocks_inv(buf->data + r, buf->size - r);
            t = !st->f.bStopRequested;
            t = !st->f.bPaused && t;
            if (t)
                st->f.bEndReached = 1;
        } else {
            fn_iCallOwnerTdstSpuStreamData_inv(st, E_SNDITBASE_BLOCKREAD_inv, 0);
        }
    }
    return r;
}

int fn_iSendFirstTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, Buf_inv *buf)
{
    int r = 0;

    if (buf->size == st->lBufSize) {
        unsigned char *p;

        fn_u32FetchTdstSpuStreamData_inv(st, buf);
        p = buf->data;
        p[1] = 6;
        p[0x11] = 2;
        MarkEnd_inv(buf->data, buf->size);
#ifdef PC_PORT
        pc_memcpy((char *)st->lSpuAddr + st->lBufOffset, buf->data, buf->size);
#else
        r = libsd_17_inv(0, 0, buf->data, st->lSpuAddr + st->lBufOffset, buf->size);
#endif
    }
    return r;
}

int fn_iSendNextTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, Buf_inv *buf)
{
    unsigned int got;
    int r = 0;
    int t;

    if (buf->size == st->lHalfSize && st->x1c == 1) {
        got = fn_u32FetchTdstSpuStreamData_inv(st, buf);
        if (got) {
            st->f.b6 = 0;
            st->f.b7 = 0;
        }
        t = !st->f.b6;
        t = !st->f.b7 || t;
        if (t) {
            if (st->lBufOffset == 0)
                fn_vMarkLoopBlocks_inv(buf->data, buf->size);
            else
                fn_vMarkLastBlocks_inv(buf->data, buf->size);
#ifdef PC_PORT
            pc_memcpy((char *)st->lSpuAddr + st->lBufOffset, buf->data, st->lHalfSize);
#else
            libsd_10_inv(st->lVoice | (st->lCore << 1) | 0x2240);
            r = libsd_17_inv(0, 0, buf->data, st->lSpuAddr + st->lBufOffset, st->lHalfSize);
#endif
        }
        if (!got) {
            if (!t && !st->f.bPaused) {
#ifdef PC_PORT
                pc_channel_volume(PC_VOICE_inv(st), 0, 0);
#else
                SdBatch_inv b[2] = {
                    { 1, st->lVoice | (st->lCore << 1), 0 },
                    { 1, st->lVoice | (st->lCore << 1) | 0x100, 0 },
                };

                libsd_15_inv(b, 0, 2);
#endif
            }
            if (st->lBufOffset == 0)
                st->f.b6 = 1;
            else
                st->f.b7 = 1;
        }
    }
    return r;
}

tdstSpuStreamData_inv *fn_u32AllocateTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    st->f.bAllocated = 1;
    st->f.bDesallocating = 0;
    return st;
}

int fn_u32DesallocateTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
#ifdef PC_PORT
    pc_channel_volume(PC_VOICE_inv(st), 0, 0);
#else
    SdBatch_inv b[2] = {
        { 1, st->lVoice | (st->lCore << 1), 0 },
        { 1, st->lVoice | (st->lCore << 1) | 0x100, 0 },
    };

    libsd_15_inv(b, 0, 2);
#endif
    st->pstOwner = 0;
    st->u32Fetched = 0;
    st->u32LastPos = 0;
    st->hOwner = 0;
    st->hChannel = 0;
    st->pstChannel = 0;
    st->f.bDesallocating = 1;
    st->f.bStarted = 0;
    st->f.bReady = 0;
    st->f.bStopped = 0;
    st->f.bEndReached = 0;
    st->f.bStopRequested = 0;
    st->f.bReadyNotified = 0;
    st->f.bStopNotified = 0;
    st->f.bEndNotified = 0;
    st->f.bPaused = 0;
    ClearTrip_inv(&st->stParam);
    st->f.bDesallocating = 0;
    st->f.bAllocated = 0;
    return 0;
}

int fn_iStartTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    st->f.bStarted = 1;
    st->f.bEndReached = 0;
    st->f.bStopped = 0;
    st->f.bStopRequested = 0;
    st->f.bStopNotified = 0;
    st->f.bEndNotified = 0;
    return 0;
}

int fn_iStopTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    if (!st->f.bStarted)
        st->f.bStopped = 1;
    st->f.bStopRequested = 1;
    st->f.bStopNotified = 0;
    return 0;
}

int fn_bIsCompatibleTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, DescHead_inv *d)
{
    int n = 0;
    int r;

    if (d != 0)
        n = 1881 / ((d->uwRate * 22369) >> 18);
    r = 0;
    if ((n == 0 || n == st->uwBlockTicks) && !st->f.bAllocated)
        r = 1;
    return r;
}

int fn_iSetOwnerTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, Info_inv *info)
{
    st->pstOwner = info->iface;
    st->hOwner = info->h;
    return 0;
}

int fn_iConnectTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, int arg)
{
    Info_inv info;
    int r = -1;

    if (fn_iFindChild_inv(&info, 1, arg) >= 0) {
        st->hChannel = info.h;
        r = 0;
        st->pstChannel = info.iface;
    }
    return r;
}

int fn_iGetNumChildTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    int n = 0;

    if (st->pstChannel != 0)
        n = st->hChannel != 0;
    return n;
}

int fn_iGetChildTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, Info_inv *info, int idx)
{
    int r = -1;

    if (idx < fn_iGetNumChildTdstSpuStreamData_inv(st)) {
        switch (idx) {
        case 0:
            info->kind = 1;
            info->h = st->hChannel;
            info->iface = st->pstChannel;
            break;
        }
        r = 0;
    }
    return r;
}

int fn_iCallOwnerTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (st->pstOwner != 0 && st->hOwner != 0) {
        InitMsg_inv(&msg, 1, st, 0);
        r = st->pstOwner->Notify(st->hOwner, event, &msg, arg);
    }
    return r;
}

tdstSpuStreamData_inv *fn_pSetParamTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, Trip_inv *t)
{
    if (!st->f.bDesallocating) {
        if (t->left != st->stParam.left || t->right != st->stParam.right) {
#ifdef PC_PORT
            pc_channel_volume(PC_VOICE_inv(st), t->left, t->right);
#else
            SdBatch_inv b[2] = {
                { 1, st->lVoice | (st->lCore << 1), t->left },
                { 1, st->lVoice | (st->lCore << 1) | 0x100, t->right },
            };

            libsd_15_inv(b, 0, 2);
#endif
            CopyTrip_inv(&st->stParam, t);
        }
    }
    return st;
}

int fn_iGetPosTdstSpuStreamData_inv(tdstSpuStreamData_inv *st, unsigned int *out)
{
    unsigned int t = st->u32Fetched;

    if (!st->f.bEndReached)
#ifdef PC_PORT
        t += (int)pc_channel_position(PC_VOICE_inv(st)) - st->lSpuAddr - (st->lHalfSize << 1);
#else
        t += libsd_10_inv(st->lVoice | (st->lCore << 1) | 0x2240) - st->lSpuAddr - (st->lHalfSize << 1);
#endif
    if (t < st->u32LastPos)
        t = st->u32LastPos;
    st->u32LastPos = t;
    *out = t;
    return 0;
}

int fn_iPauseTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    st->f.bPaused = 1;
    return 0;
}

int fn_iResumeTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    st->f.bPaused = 0;
    return 0;
}

void fn_vOperateCallTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    int t;

    if (st->f.bAllocated) {
        t = st->f.bReady;
        t = !st->f.bReadyNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamData_inv(st, E_SNDITBASE_READY_inv, 0) >= 0)
                st->f.bReadyNotified = 1;
        }
        t = st->f.bStopped;
        t = !st->f.bStopNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamData_inv(st, E_SNDITBASE_STOPPED_inv, 0) >= 0)
                st->f.bStopNotified = 1;
        }
        t = st->f.bEndReached;
        t = !st->f.bEndNotified && t;
        if (t) {
            if (fn_iCallOwnerTdstSpuStreamData_inv(st, E_SNDITBASE_ENDREACHED, 0) >= 0)
                st->f.bEndNotified = 1;
        }
    }
}

void fn_vResetTdstSpuStreamData_inv(tdstSpuStreamData_inv *st)
{
    ClearStream_inv(st);
    st->f.bDesallocating = 0;
    st->f.bAllocated = 0;
}

unsigned short fn_uwNextIndex_inv(unsigned short i, unsigned short n)
{
    return NextIdx_inv(i, n);
}
