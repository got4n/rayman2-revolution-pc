#include "include_asm.h"
#include "sndchannelcd_inv.h"

typedef struct {
    long bConstructed : 1;
    long bRunning : 1;
    long bCdBusy : 1;
} tdstChannelCDMngrFlags_inv;

typedef struct {
    tdstChannelCDMngrFlags_inv f;
    int u32ReadSize;
    int pReadBuf;
    int lSemaCd;
    int lSemaSync;
    int lSemaExit;
    int lSemaIdle;
    int lThread;
    int x20;
    char a_cReadMode[4];
    int lState;
    struct tdstChannelCDData_inv *pstCur;
    struct tdstChannelCDData_inv *pstLastSeek;
    int a_pfnState[8];
    tdstChannelCDData_inv a_stData[2];
    int xec;
} tdstChannelCDMngr_inv;

typedef int (*ChannelCDStateFn_inv)(tdstChannelCDMngr_inv *m);

typedef int (*ChannelCDDataFn_inv)(tdstChannelCDData_inv *e, void *arg);

typedef struct {
    int attr;
    int x4;
    int count;
    int max;
} SemaDesc_inv;

typedef struct {
    int attr;
    int x4;
    ChannelCDStateFn_inv entry;
    int stack;
    int prio;
} ThreadDesc_inv;

tdstChannelCDMngr_inv *g_pstChannelCDMngr_inv = 0;
extern int thsemap_4_inv(SemaDesc_inv *d);
extern int thbase_4_inv(ThreadDesc_inv *d);
extern int sysmem_4_inv();
extern int fn_iConstructTdstChannelCDData_inv();
extern int cdvdman_13_inv();
extern int thbase_6_inv();
extern int thsemap_7_inv();
extern int thbase_5_inv();
extern int thsemap_5_inv();
extern int fn_bNeedDataTdstChannelCDData_inv();
extern int thsemap_6_inv();
extern int fn_iReadTdstChannelCDData_inv();
extern int fn_iStartTdstChannelCDData_inv();
extern int fn_u32DesallocateTdstChannelCDData_inv();
extern int fn_iStopTdstChannelCDData_inv();
extern int fn_iGetPosTdstChannelCDData_inv();
extern int fn_u32AllocateTdstChannelCDData_inv();
extern int fn_iConnectTdstChannelCDData_inv();
extern int fn_iSetOwnerTdstChannelCDData_inv();
extern int thsemap_8_inv();
extern int cdvdman_37_inv();
extern int cdvdman_39_inv();
extern int cdvdman_11_inv();
extern int fn_iSeekTdstChannelCDData_inv();
extern int fn_iReadSectorTdstChannelCDData_inv();
extern int fn_iFillTdstChannelCDData_inv();
extern int fn_vOperateCallTdstChannelCDData_inv();

static inline tdstChannelCDData_inv *ValidateElemE_inv(tdstChannelCDMngr_inv *pool, tdstChannelCDData_inv *h)
{
    tdstChannelCDData_inv *found = 0;

    if (h >= pool->a_stData && h < &pool->a_stData[2]) {
        if (((char *)h - (char *)pool->a_stData) % sizeof(tdstChannelCDData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int GetMgrHandler_inv(tdstChannelCDMngr_inv *m, unsigned int i)
{
    int r = 0;

    if (i < 8)
        r = m->a_pfnState[i];
    return r;
}

static inline int IsFreeE_inv(tdstChannelCDData_inv *e)
{
    return !e->f.bAllocated;
}

#ifdef PC_PORT
static inline void PcGive_inv(tdstChannelCDMngr_inv *m, int *flag)
{
    *flag = 1;
    pc_task_signal(m->lThread);
}

#endif
void fn_vCdCallbackTdstChannelCDMngr_inv(void)
{
    if (g_pstChannelCDMngr_inv != 0)
#ifdef PC_PORT
        pc_task_signal(g_pstChannelCDMngr_inv->lThread);
#else
        thsemap_7_inv(g_pstChannelCDMngr_inv->lSemaCd);
#endif
}

int fn_iStateSyncTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
#ifdef PC_PORT
#else
    cdvdman_37_inv(0);
    thsemap_8_inv(m->lSemaSync);
    cdvdman_37_inv(fn_vCdCallbackTdstChannelCDMngr_inv);
#endif
    m->lState = 6;
    return 0;
}

int fn_iStateStopTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    int i = 0;

    m->f.bCdBusy = 0;
#ifdef PC_PORT
    (void)i;
#else
    cdvdman_39_inv();
    do {
        if (cdvdman_11_inv(1) == 2)
            break;
        i++;
    } while (i <= 99999);
#endif
    m->lState = 6;
    return 0;
}

int fn_iStateIdleTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
#ifdef PC_PORT
#else
    thsemap_8_inv(m->lSemaIdle);
#endif
    m->lState = 6;
    return 0;
}

int fn_iStateSeekTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    int r;

    m->lState = 4;
    r = fn_iSeekTdstChannelCDData_inv(m->pstCur, m->pstLastSeek);
    if (r >= 0) {
        if (r == 1) {
            m->pstLastSeek = m->pstCur;
            m->f.bCdBusy = 1;
        }
    } else {
        m->lState = 1;
    }
    return 0;
}

int fn_iStateReadTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    int r;

    m->lState = 5;
    r = fn_iReadSectorTdstChannelCDData_inv(m->pstCur, &m->u32ReadSize, m->a_cReadMode);
    if (r != -1) {
        if (r != 0) {
            m->pstLastSeek = m->pstCur;
            m->f.bCdBusy = 1;
        }
    } else {
        m->lState = 1;
    }
    return 0;
}

int fn_iStateFillTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    m->lState = 6;
    fn_iFillTdstChannelCDData_inv(m->pstCur, &m->u32ReadSize);
    return 0;
}

int fn_iStateNextTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    tdstChannelCDData_inv **pc = &m->pstCur;
    int i;

    m->lState = 0;
    for (i = 0; i < 2; i++) {
        (*pc)++;
        if (*pc == &m->a_stData[2])
            *pc = m->a_stData;
        if (!IsFreeE_inv(*pc)) {
            m->lState = 2;
            if (fn_bNeedDataTdstChannelCDData_inv(*pc)) {
                m->lState = 3;
                break;
            }
        }
    }
    return 0;
}

#ifdef PC_PORT
int fn_iThreadTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    ChannelCDStateFn_inv fn;
    int *flag;
    int n;

    for (n = 0; n < 64; n++) {
        if (!m->f.bRunning)
            return 0;
        flag = m->lState == 0 ? &m->lSemaSync : m->lState == 2 ? &m->lSemaIdle : 0;
        if (flag != 0) {
            if (!*flag)
                return 0;
            *flag = 0;
        }
        m->f.bCdBusy = 0;
        fn = (ChannelCDStateFn_inv)GetMgrHandler_inv(m, m->lState);
        if (fn != 0)
            fn(m);
    }
    return 1;
}
#else
int fn_iThreadTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    ChannelCDStateFn_inv fn;

    g_pstChannelCDMngr_inv = m;
    while (m->f.bRunning) {
        while (m->f.bCdBusy) {
            thsemap_8_inv(m->lSemaCd);
            if (!cdvdman_11_inv(1))
                break;
        }
        m->f.bCdBusy = 0;
        fn = (ChannelCDStateFn_inv)GetMgrHandler_inv(m, m->lState);
        if (fn != 0)
            fn(m);
    }
    g_pstChannelCDMngr_inv = 0;
    thsemap_6_inv(m->lSemaExit);
    return 0;
}
#endif

#ifdef PC_PORT
int fn_iConstructTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, void *arg)
{
    int th;
    tdstChannelCDData_inv *it;
    int r = -1;

    if (!m->f.bConstructed) {
        th = pc_task_create((PcAddress)fn_iThreadTdstChannelCDMngr_inv, (uint32_t)m, 0x6A);
        m->u32ReadSize = 0x4800;
        m->pReadBuf = pc_mem_alloc(m->u32ReadSize);
        if (th > 0) {
            m->a_pfnState[0] = (int)fn_iStateSyncTdstChannelCDMngr_inv;
            m->a_pfnState[1] = (int)fn_iStateStopTdstChannelCDMngr_inv;
            m->a_pfnState[2] = (int)fn_iStateIdleTdstChannelCDMngr_inv;
            m->a_pfnState[3] = (int)fn_iStateSeekTdstChannelCDMngr_inv;
            m->a_pfnState[4] = (int)fn_iStateReadTdstChannelCDMngr_inv;
            m->a_pfnState[5] = (int)fn_iStateFillTdstChannelCDMngr_inv;
            m->a_pfnState[6] = (int)fn_iStateNextTdstChannelCDMngr_inv;
            m->lSemaCd = 0;
            m->lSemaSync = 0;
            m->lSemaExit = 0;
            m->lSemaIdle = 0;
            m->lThread = th;
            m->x20 = -1;
            m->a_cReadMode[0] = 0;
            m->a_cReadMode[1] = 1;
            m->a_cReadMode[2] = 0;
            m->a_cReadMode[3] = 0;
            m->lState = 0;
            m->pstCur = m->a_stData;
            m->pstLastSeek = 0;
            m->f.bCdBusy = 0;
            m->f.bConstructed = 1;
            m->f.bRunning = 1;
            for (it = m->a_stData; it < m->a_stData + 2; it++)
                fn_iConstructTdstChannelCDData_inv(it, 0x4000, arg);
            g_pstChannelCDMngr_inv = m;
            r = 0;
        }
    } else {
        r = 0;
    }
    if (r < 0)
        r = 0;
    return r;
}
#else
int fn_iConstructTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, void *arg)
{
    SemaDesc_inv sd;
    ThreadDesc_inv td;
    int s0;
    int s1;
    int s2;
    int s3;
    int th;
    tdstChannelCDData_inv *it;
    int r = -1;

    if (!m->f.bConstructed) {
        sd.attr = 1;
        sd.x4 = 0;
        sd.count = 0;
        sd.max = 1;
        s0 = thsemap_4_inv(&sd);
        s1 = thsemap_4_inv(&sd);
        s3 = thsemap_4_inv(&sd);
        s2 = thsemap_4_inv(&sd);
        td.attr = 0x2000000;
        td.entry = fn_iThreadTdstChannelCDMngr_inv;
        td.prio = 0x6A;
        td.stack = 0x800;
        td.x4 = 0;
        th = thbase_4_inv(&td);
        m->u32ReadSize = 0x4800;
        m->pReadBuf = sysmem_4_inv(0, m->u32ReadSize, 0);
        if (s0 > 0 && s1 > 0 && s2 > 0 && s3 > 0 && th > 0) {
            m->a_pfnState[0] = (int)fn_iStateSyncTdstChannelCDMngr_inv;
            m->a_pfnState[1] = (int)fn_iStateStopTdstChannelCDMngr_inv;
            m->a_pfnState[2] = (int)fn_iStateIdleTdstChannelCDMngr_inv;
            m->a_pfnState[3] = (int)fn_iStateSeekTdstChannelCDMngr_inv;
            m->a_pfnState[4] = (int)fn_iStateReadTdstChannelCDMngr_inv;
            m->a_pfnState[5] = (int)fn_iStateFillTdstChannelCDMngr_inv;
            m->a_pfnState[6] = (int)fn_iStateNextTdstChannelCDMngr_inv;
            m->lSemaCd = s0;
            m->lSemaSync = s1;
            m->lSemaExit = s2;
            m->lSemaIdle = s3;
            m->lThread = th;
            m->x20 = -1;
            m->a_cReadMode[0] = 0;
            m->a_cReadMode[1] = 1;
            m->a_cReadMode[2] = 0;
            m->a_cReadMode[3] = 0;
            m->lState = 0;
            m->pstCur = m->a_stData;
            m->pstLastSeek = 0;
            m->f.bCdBusy = 0;
            m->f.bConstructed = 1;
            m->f.bRunning = 1;
            for (it = m->a_stData; it < m->a_stData + 2; it++)
                fn_iConstructTdstChannelCDData_inv(it, 0x4000, arg);
            if (cdvdman_13_inv(0) == 2) {
                thbase_6_inv(m->lThread, m);
                r = 0;
            }
        } else {
            if (s0 > 0)
                thsemap_5_inv(s0);
            if (s1 > 0)
                thsemap_5_inv(s1);
            if (s2 > 0)
                thsemap_5_inv(s2);
            if (s3 > 0)
                thsemap_5_inv(s3);
            if (th > 0)
                thbase_5_inv(th);
        }
    } else {
        r = 0;
    }
    if (r < 0)
        r = 0;
    return r;
}
#endif

#ifdef PC_PORT
int fn_iDestructTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    if (m->f.bConstructed) {
        m->f.bRunning = 0;
        g_pstChannelCDMngr_inv = 0;
        m->lSemaCd = 0;
        m->lSemaSync = 0;
        m->lSemaExit = 0;
        m->lSemaIdle = 0;
        if (m->lThread > 0) {
            pc_task_delete(m->lThread);
            m->lThread = 0;
        }
        m->f.bConstructed = 0;
    }
    return 0;
}
#else
int fn_iDestructTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    if (m->f.bConstructed) {
        m->f.bRunning = 0;
        thsemap_8_inv(m->lSemaExit);
        if (m->lSemaCd > 0) {
            thsemap_5_inv(m->lSemaCd);
            m->lSemaCd = 0;
        }
        if (m->lSemaSync > 0) {
            thsemap_5_inv(m->lSemaSync);
            m->lSemaSync = 0;
        }
        if (m->lSemaExit > 0) {
            thsemap_5_inv(m->lSemaExit);
            m->lSemaExit = 0;
        }
        if (m->lSemaIdle > 0) {
            thsemap_5_inv(m->lSemaIdle);
            m->lSemaIdle = 0;
        }
        if (m->lThread > 0) {
            thbase_5_inv(m->lThread);
            m->lThread = 0;
        }
        m->f.bConstructed = 0;
    }
    return 0;
}
#endif

tdstChannelCDData_inv *fn_pstParseTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, void *arg, ChannelCDDataFn_inv fn)
{
    tdstChannelCDData_inv *r = 0;
    tdstChannelCDData_inv *e;

    for (e = m->a_stData; e < &m->a_stData[2]; e++) {
        if (fn(e, arg)) {
            r = e;
            break;
        }
    }
    return r;
}

int fn_iCountFreeTdstChannelCDMngr_inv(tdstChannelCDData_inv *e, int *n)
{
    if (IsFreeE_inv(e))
        (*n)++;
    return 0;
}

int fn_iIsFreeTdstChannelCDMngr_inv(tdstChannelCDData_inv *e)
{
    return IsFreeE_inv(e);
}

int fn_bIsUsedTdstChannelCDMngr_inv(tdstChannelCDData_inv *e)
{
    return e->f.bAllocated != 0;
}

int fn_bIsBusyTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    return fn_pstParseTdstChannelCDMngr_inv(m, 0, (ChannelCDDataFn_inv)fn_bIsUsedTdstChannelCDMngr_inv) != 0;
}

int fn_iGetNumFreeVoiceTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    int n = 0;

    fn_pstParseTdstChannelCDMngr_inv(m, &n, (ChannelCDDataFn_inv)fn_iCountFreeTdstChannelCDMngr_inv);
    return n;
}

int fn_u32AllocateVoiceTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, int x)
{
    int r = 0;
    tdstChannelCDData_inv *v;

    if (m->f.bConstructed) {
        v = fn_pstParseTdstChannelCDMngr_inv(m, 0, (ChannelCDDataFn_inv)fn_iIsFreeTdstChannelCDMngr_inv);
        if (v != 0)
            r = fn_u32AllocateTdstChannelCDData_inv(v, x);
    }
    return r;
}

int fn_u32DesallocateVoiceTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_u32DesallocateTdstChannelCDData_inv(v);
#ifdef PC_PORT
        PcGive_inv(m, &m->lSemaIdle);
#else
        thsemap_6_inv(m->lSemaIdle);
#endif
    }
    return r;
}

int fn_bIsAllocatedTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = 0;
    if (v != 0 && m->f.bConstructed) {
        r = v->f.bAllocated != 0;
    }
    return r;
}

int fn_iStartTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h, int a)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_iStartTdstChannelCDData_inv(v, a);
#ifdef PC_PORT
        PcGive_inv(m, &m->lSemaSync);
        PcGive_inv(m, &m->lSemaIdle);
#else
        thsemap_6_inv(m->lSemaSync);
        thsemap_6_inv(m->lSemaIdle);
#endif
    }
    return r;
}

int fn_iStopTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_iStopTdstChannelCDData_inv(v);
    }
    return r;
}

int fn_iReadTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h, int a, int b, int c)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = 0;
    if (v != 0 && m->f.bConstructed) {
        r = fn_iReadTdstChannelCDData_inv(v, a, b, c);
#ifdef PC_PORT
        PcGive_inv(m, &m->lSemaIdle);
#else
        thsemap_6_inv(m->lSemaIdle);
#endif
    }
    return r;
}

int fn_iSetOwnerTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h, int a)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_iSetOwnerTdstChannelCDData_inv(v, a);
    }
    return r;
}

int fn_iConnectTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h, int a)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_iConnectTdstChannelCDData_inv(v, a);
    }
    return r;
}

int fn_u32ReallocateVoiceTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h, int a)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_u32AllocateTdstChannelCDData_inv(v, a);
    }
    return r;
}

int fn_iGetPosTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h, int a)
{
    int r;
    tdstChannelCDData_inv *v = ValidateElemE_inv(m, h);

    r = -1;
    if (v != 0 && m->f.bConstructed) {
        r = fn_iGetPosTdstChannelCDData_inv(v, a);
    }
    return r;
}

int fn_iOperateCallOneTdstChannelCDMngr_inv(tdstChannelCDData_inv *e)
{
    fn_vOperateCallTdstChannelCDData_inv(e);
    return 0;
}

int fn_iOperateCallTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    return (int)fn_pstParseTdstChannelCDMngr_inv(m, 0, (ChannelCDDataFn_inv)fn_iOperateCallOneTdstChannelCDMngr_inv);
}

int fn_bIsValidStateTdstChannelCDMngr_inv(unsigned int i)
{
    return i < 8;
}

void fn_vSetStateHandlerTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, unsigned int i, int v)
{
    if (i < 8)
        m->a_pfnState[i] = v;
}

int fn_pfnGetStateHandlerTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m)
{
    int r = 0;
    unsigned int i = m->lState;

    if (i < 8)
        r = m->a_pfnState[i];
    return r;
}

tdstChannelCDData_inv *fn_pstValidateTdstChannelCDMngr_inv(tdstChannelCDMngr_inv *m, tdstChannelCDData_inv *h)
{
    return ValidateElemE_inv(m, h);
}
