#include "include_asm.h"
#include "sndchannelmemmono_inv.h"
#include "sndiopbuf_inv.h"

extern void *sysclib_12_inv(void *dst, void *src, int n);

static inline void ResetF_inv(tdstChannelMEMMonoData_inv *e)
{
    e->f.bStarted = 0;
    e->f.bReady = 0;
    e->f.bEndReached = 0;
    e->f.bStopped = 0;
    e->f.bReadyNotified = 0;
    e->f.bStopNotified = 0;
    e->f.bEndNotified = 0;
    e->pstOwner = 0;
    e->hOwner = 0;
    e->u32Pos = 0;
    e->u32Total = 0;
    e->u32Size = 0;
    e->pData = 0;
}

int fn_iConstructTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    ResetF_inv(e);
    e->f.bAllocated = 0;
    return 0;
}

int fn_iDestructTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    ResetF_inv(e);
    e->f.bAllocated = 0;
    return 0;
}

int fn_iCallOwnerTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e, int event, int arg)
{
    Msg_inv msg;
    int r = -1;

    if (e->pstOwner != 0 && e->hOwner != 0) {
        InitMsg_inv(&msg, 1, e, 0);
        r = e->pstOwner->Notify(e->hOwner, event, &msg, arg);
    }
    return r;
}

int fn_iStartTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e, unsigned int pos)
{
    e->f.bStarted = 1;
    e->f.bEndReached = 0;
    e->f.bStopped = 0;
    e->f.bStopNotified = 0;
    e->f.bEndNotified = 0;
    e->u32Pos = pos;
    e->f.bReady = 1;
    return 0;
}

int fn_iStopTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    e->f.bStopped = 1;
    return 0;
}

tdstChannelMEMMonoData_inv *fn_u32AllocateTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e, Span_inv *s)
{
    e->f.bAllocated = 1;
    e->u32Size = s->size;
    e->pData = s->data;
    e->u32Pos = 0;
    return e;
}

int fn_u32DesallocateTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    ResetF_inv(e);
    e->f.bAllocated = 0;
    return 0;
}

unsigned int fn_u32ReadTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e, int a, int b, Span_inv *dst)
{
    unsigned int done;
    unsigned int rem;
    unsigned int n;
    unsigned int r = 0;
    int c;

    c = e->f.bAllocated;
    c = e->f.bStarted && c;
    c = e->f.bReady && c;
    c = !e->f.bStopped && c;
    if (c) {
        done = 0;
        while (!e->f.bEndReached && done < dst->size) {
            rem = dst->size - done;
            n = e->u32Size - e->u32Pos;
            if (rem < n)
                n = rem;
#ifdef PC_PORT
            pc_memcpy(dst->data + done, e->pData + e->u32Pos, n);
#else
            sysclib_12_inv(dst->data + done, e->pData + e->u32Pos, n);
#endif
            done += n;
            e->u32Pos += n;
            e->u32Total += n;
            if (e->u32Pos == e->u32Size) {
                e->f.bEndReached = 1;
                if (!e->f.bEndNotified) {
                    e->f.bEndNotified = 1;
                    fn_iCallOwnerTdstChannelMEMMonoData_inv(e, E_SNDITBASE_ENDREACHED, 0);
                }
            }
        }
        r = done;
    }
    return r;
}

int fn_iSetOwnerTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e, Info_inv *info)
{
    e->pstOwner = info->iface;
    e->hOwner = info->h;
    return 0;
}

int fn_iConnectTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    return 0;
}

int fn_iNotifyTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    return 0;
}

int fn_iGetPosTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e, unsigned int *out)
{
    *out = e->u32Total;
    return 0;
}

void fn_vOperateCallTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    int c;

    if (e->f.bAllocated) {
        c = e->f.bReady;
        c = !e->f.bReadyNotified && c;
        if (c && fn_iCallOwnerTdstChannelMEMMonoData_inv(e, E_SNDITBASE_READY_inv, 0) >= 0)
            e->f.bReadyNotified = 1;
        c = e->f.bStopped;
        c = !e->f.bStopNotified && c;
        if (c && fn_iCallOwnerTdstChannelMEMMonoData_inv(e, E_SNDITBASE_STOPPED_inv, 0) >= 0)
            e->f.bStopNotified = 1;
    }
}

void fn_vResetTdstChannelMEMMonoData_inv(tdstChannelMEMMonoData_inv *e)
{
    ResetF_inv(e);
    e->f.bAllocated = 0;
}
