#include "include_asm.h"
#include "sndchannelcd_inv.h"

typedef struct {
    unsigned int len;
    int start;
} Extent_inv;

extern int sysmem_5_inv(char *p);
extern int thsemap_5_inv(int sema);
extern int thsemap_6_inv(int sema);
extern int thsemap_8_inv(int sema);
extern int cdvdman_6_inv(int start, unsigned int n, char *data, int d);
extern int fn_u32PushTdstFifo_inv(tdstFifo_inv *r, Span_inv *s, int c);
extern int fn_u32PopTdstFifo_inv(tdstFifo_inv *r, void *s, int c);
extern int cdvdman_7_inv(int lsn);
int fn_iCallOwnerTdstChannelCDData_inv(tdstChannelCDData_inv *e, int event, int arg);
int fn_bNeedDataTdstChannelCDData_inv(tdstChannelCDData_inv *e);

static inline void ResetE_inv(tdstChannelCDData_inv *e)
{
    tdstFifo_inv *r;

    e->f.bAllocated = 0;
    e->f.bStarted = 0;
    e->f.bFull = 0;
    e->f.b3 = 0;
    e->f.bEndReached = 0;
    e->f.bStopped = 0;
    e->f.bSeeked = 0;
    e->f.bReadPending = 0;
    e->f.bReadyNotified = 0;
    e->f.bStopNotified = 0;
    e->f.bEndNotified = 0;
    r = &e->stFifo;
    r->base = 0;
    r->end = 0;
    r->rd = 0;
    r->wr = 0;
    r->count = 0;
    e->pstOwner = 0;
    e->hOwner = 0;
    e->lSector = 0;
    e->lSeekSector = 0;
    e->u32Pos = 0;
    e->u32Total = 0;
    e->u32SectorOffset = 0;
    e->u32Length = 0;
    e->lStartSector = 0;
}

typedef struct {
    int attr;
    int x4;
    int count;
    int max;
} SemaDesc_inv;

extern char *sysmem_4_inv(int a, unsigned int size, int c);
extern int thsemap_4_inv(SemaDesc_inv *d);

int fn_iConstructTdstChannelCDData_inv(tdstChannelCDData_inv *e, unsigned int size)
{
    SemaDesc_inv sd;
    unsigned int half;
    unsigned int total;
    char *buf;
    int sema;
    int r = -1;

    half = size / 2048 * 2048;
    total = half * 2;
#ifdef PC_PORT
    buf = pc_mem_alloc(total);
#else
    buf = sysmem_4_inv(0, total, 0);
#endif
    sd.attr = 1;
    sd.x4 = 0;
    sd.count = 0;
    sd.max = 1;
#ifdef PC_PORT
    sema = 1;
#else
    sema = thsemap_4_inv(&sd);
#endif
    if (buf != 0 && sema > 0) {
#ifdef PC_PORT
#else
        thsemap_6_inv(sema);
#endif
        r = 0;
        ResetE_inv(e);
        e->u32HalfSize = half;
        e->u32BufSize = total;
        e->pBuf = buf;
        e->lSema = sema;
    } else {
        if (buf != 0)
#ifdef PC_PORT
            pc_mem_free(buf);
#else
            sysmem_5_inv(buf);
#endif
        if (sema > 0)
#ifdef PC_PORT
            ;
#else
            thsemap_5_inv(sema);
#endif
    }
    half = r;
    return half;
}

int fn_iDestructTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    int r = -1;

    if (e->pBuf != 0) {
#ifdef PC_PORT
        pc_mem_free(e->pBuf);
#else
        sysmem_5_inv(e->pBuf);
        thsemap_5_inv(e->lSema);
#endif
        r = 0;
        e->u32HalfSize = 0;
        e->pBuf = 0;
        e->u32BufSize = 0;
        e->lSema = 0;
        ResetE_inv(e);
    }
    return r;
}

int fn_iSeekTdstChannelCDData_inv(tdstChannelCDData_inv *e, tdstChannelCDData_inv *o)
{
    int r = 0;
    int c;

    c = fn_bNeedDataTdstChannelCDData_inv(e);
    c = !e->f.bSeeked && c;
    c = !e->f.bReadPending && c;
#ifdef PC_PORT
    c = e->lSector != (o ? o->lSeekSector : 0) && c;
#else
    c = e->lSector != o->lSeekSector && c;
#endif
    if (c) {
#ifdef PC_PORT
        r = 1;
        e->lSeekSector = e->lSector;
#else
        if (cdvdman_7_inv(e->lSector) == 1) {
            r = 1;
            e->lSeekSector = e->lSector;
        } else {
            r = -1;
        }
#endif
    }
    e->f.bSeeked = 1;
    return r;
}

int fn_iReadSectorTdstChannelCDData_inv(tdstChannelCDData_inv *e, Span_inv *s, int arg)
{
    int r = 0;
    int c;
    unsigned int n;

    c = fn_bNeedDataTdstChannelCDData_inv(e);
    c = e->f.bSeeked && c;
    c = !e->f.bReadPending && c;
    if (c) {
        r = -1;
        e->f.bSeeked = 0;
        e->f.bReadPending = 1;
        n = s->size / 2048;
#ifdef PC_PORT
        {
            PcFile file = { e->lStartSector, e->u32Length };

            (void)arg;
            if (pc_file_read(&file, (e->lSector - e->lStartSector) * PC_FILE_SECTOR_SIZE, s->data, n * 2048) == 1)
                r = n * 2048;
        }
#else
        if (cdvdman_6_inv(e->lSector, n, s->data, arg) == 1)
            r = n * 2048;
#endif
    }
    return r;
}

int fn_iFillTdstChannelCDData_inv(tdstChannelCDData_inv *e, Span_inv *s)
{
    Span_inv sp;
    unsigned int lim;
    unsigned int t;
    int r = 0;
    int c;

    c = fn_bNeedDataTdstChannelCDData_inv(e);
    c = !e->f.bSeeked && c;
    c = e->f.bReadPending && c;
    if (c) {
        e->f.bReadPending = 0;
        sp.size = s->size - e->u32SectorOffset;
        sp.data = s->data + e->u32SectorOffset;
        lim = e->u32Length - e->u32Pos;
        if (lim < sp.size)
            sp.size = lim;
#ifdef PC_PORT
#else
        thsemap_8_inv(e->lSema);
#endif
        r = fn_u32PushTdstFifo_inv(&e->stFifo, &sp, 0);
        e->u32Pos += r;
        e->u32Total += r;
        e->u32SectorOffset = e->u32Pos % 2048;
        e->lSector = e->lStartSector + (e->u32Pos - e->u32SectorOffset) / 2048;
        e->lSeekSector = e->lSector;
        if (e->u32Pos >= e->u32Length) {
            e->f.bEndReached = 1;
            if (!e->f.bEndNotified) {
                e->f.bEndNotified = 1;
                fn_iCallOwnerTdstChannelCDData_inv(e, E_SNDITBASE_ENDREACHED, 0);
            }
        }
        if (!e->f.bFull && FifoFree_inv(&e->stFifo) == 0)
            e->f.bFull = 1;
#ifdef PC_PORT
#else
        thsemap_6_inv(e->lSema);
#endif
    }
    return r;
}

int fn_iReadTdstChannelCDData_inv(tdstChannelCDData_inv *e, int a, int n, void *dst)
{
    int r = 0;
    int c;

    if (!e->f.bStopped) {
        n = c = e->f.bAllocated && n;
        if (n) {
#ifdef PC_PORT
#else
            thsemap_8_inv(e->lSema);
#endif
            r = fn_u32PopTdstFifo_inv(&e->stFifo, dst, 0);
#ifdef PC_PORT
#else
            thsemap_6_inv(e->lSema);
#endif
        }
    }
    return r;
}

tdstChannelCDData_inv *fn_u32AllocateTdstChannelCDData_inv(tdstChannelCDData_inv *e, Extent_inv *x)
{
    tdstChannelCDData_inv *r = 0;
    tdstFifo_inv *q;

    if (x->start != 0) {
        if (!e->f.bAllocated) {
            e->f.bAllocated = 1;
            q = &e->stFifo;
            q->base = e->pBuf;
            q->end = e->pBuf + e->u32BufSize;
            q->rd = e->pBuf;
            q->wr = e->pBuf;
            q->count = 0;
        }
        e->lSector = x->start;
        e->u32Length = x->len;
        e->lStartSector = x->start;
        r = e;
    }
    return r;
}

int fn_u32DesallocateTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    ResetE_inv(e);
    if (e->pBuf != 0)
#ifdef PC_PORT
        pc_memset(e->pBuf, 0, e->u32BufSize);
#else
        sysclib_14_inv(e->pBuf, 0, e->u32BufSize);
#endif
    return 0;
}

int fn_iStartTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    e->u32Pos = 0;
    e->f.bStarted = 1;
    e->f.bStopped = 0;
    e->f.bEndReached = 0;
    e->f.bStopNotified = 0;
    e->f.bEndNotified = 0;
    e->lSector = e->lStartSector;
    return 0;
}

int fn_iStopTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    e->f.bStopped = 1;
    return 0;
}

int fn_iSetOwnerTdstChannelCDData_inv(tdstChannelCDData_inv *e, Info_inv *info)
{
    e->pstOwner = info->iface;
    e->hOwner = info->h;
    return 0;
}

int fn_iConnectTdstChannelCDData_inv(tdstChannelCDData_inv *e, int a)
{
    return 0;
}

int fn_iCallOwnerTdstChannelCDData_inv(tdstChannelCDData_inv *e, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (e->pstOwner != 0 && e->hOwner != 0) {
        InitMsg_inv(&msg, 1, e, 0);
        r = e->pstOwner->Notify(e->hOwner, event, &msg, arg);
    }
    return r;
}

int fn_iGetPosTdstChannelCDData_inv(tdstChannelCDData_inv *e, unsigned int *out)
{
    *out = e->u32Total;
    return 0;
}

void fn_vOperateCallTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    int c;

    if (e->f.bAllocated) {
        c = e->f.bFull;
        c = !e->f.bReadyNotified && c;
        if (c && fn_iCallOwnerTdstChannelCDData_inv(e, E_SNDITBASE_READY_inv, 0) >= 0)
            e->f.bReadyNotified = 1;
        c = e->f.bStopped;
        c = !e->f.bStopNotified && c;
        if (c && fn_iCallOwnerTdstChannelCDData_inv(e, E_SNDITBASE_STOPPED_inv, 0) >= 0)
            e->f.bStopNotified = 1;
    }
}

int fn_bNeedDataTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    int r = 0;

    if (e->f.bAllocated) {
#ifdef PC_PORT
#else
        thsemap_8_inv(e->lSema);
#endif
        r = !(FifoFree_inv(&e->stFifo) < FifoSize_inv(&e->stFifo) / 2);
#ifdef PC_PORT
#else
        thsemap_6_inv(e->lSema);
#endif
        r = !e->f.bFull || r;
        r = e->f.bStarted && r;
        r = !e->f.bEndReached && r;
        r = !e->f.bStopped && r;
    }
    return r;
}

void fn_vResetTdstChannelCDData_inv(tdstChannelCDData_inv *e)
{
    ResetE_inv(e);
}
