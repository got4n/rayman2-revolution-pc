#include "include_asm.h"
#include "sndchannelmemmono_inv.h"

extern int fn_u32AllocateTdstChannelMEMMonoData_inv();

extern int fn_u32DesallocateTdstChannelMEMMonoData_inv();

extern int fn_iSetOwnerTdstChannelMEMMonoData_inv();

extern int fn_iConnectTdstChannelMEMMonoData_inv();

extern int fn_iNotifyTdstChannelMEMMonoData_inv();

extern int fn_iStartTdstChannelMEMMonoData_inv();

extern int fn_iStopTdstChannelMEMMonoData_inv();

extern int fn_u32ReadTdstChannelMEMMonoData_inv();

extern int fn_iGetPosTdstChannelMEMMonoData_inv();

extern int fn_iConstructTdstChannelMEMMonoData_inv();
extern int fn_iDestructTdstChannelMEMMonoData_inv();

typedef struct {
    long bConstructed : 1;
} MngrFlags_inv;

typedef struct {
    MngrFlags_inv f;
    tdstChannelMEMMonoData_inv items[24];
} tdstChannelMEMMonoMngr_inv;

typedef int (*ChannelMEMMonoDataFn_inv)(tdstChannelMEMMonoData_inv *v, void *arg);

static inline tdstChannelMEMMonoData_inv *ValidateElemB_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h)
{
    tdstChannelMEMMonoData_inv *found = 0;

    if (h >= pool->items && h < &pool->items[24]) {
        if (((char *)h - (char *)pool->items) % sizeof(tdstChannelMEMMonoData_inv) == 0)
            found = h;
    }
    return found;
}

static inline int IsFree_inv(tdstChannelMEMMonoData_inv *v)
{
    return !v->f.bAllocated;
}

tdstChannelMEMMonoData_inv *fn_pstParseTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, void *arg, ChannelMEMMonoDataFn_inv fn)
{
    tdstChannelMEMMonoData_inv *r = 0;
    tdstChannelMEMMonoData_inv *v;

    for (v = pool->items; v < &pool->items[24]; v++) {
        if (fn(v, arg)) {
            r = v;
            break;
        }
    }
    return r;
}

int fn_iCountFreeTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoData_inv *v, int *n)
{
    if (IsFree_inv(v))
        (*n)++;
    return 0;
}

int fn_iIsFreeTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoData_inv *v)
{
    return IsFree_inv(v);
}

int fn_iConstructOneTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoData_inv *e, int x)
{
    fn_iConstructTdstChannelMEMMonoData_inv(e, x);
    return 0;
}

int fn_iDestructOneTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoData_inv *e, int x)
{
    fn_iDestructTdstChannelMEMMonoData_inv(e, x);
    return 0;
}

int fn_iConstructTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, int x)
{
    int r = -1;

    if (!pool->f.bConstructed) {
        fn_pstParseTdstChannelMEMMonoMngr_inv(pool, (void *)x, (ChannelMEMMonoDataFn_inv)fn_iConstructOneTdstChannelMEMMonoMngr_inv);
        r = 0;
        pool->f.bConstructed = 1;
    }
    return r;
}

int fn_iDestructTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool)
{
    int r;

    if (pool->f.bConstructed) {
        fn_pstParseTdstChannelMEMMonoMngr_inv(pool, 0, (ChannelMEMMonoDataFn_inv)fn_iDestructOneTdstChannelMEMMonoMngr_inv);
        r = 0;
        pool->f.bConstructed = 0;
    } else {
        r = 0;
    }
    return r;
}

int fn_iGetNumFreeVoiceTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool)
{
    int n = 0;

    fn_pstParseTdstChannelMEMMonoMngr_inv(pool, &n, (ChannelMEMMonoDataFn_inv)fn_iCountFreeTdstChannelMEMMonoMngr_inv);
    return n;
}

tdstChannelMEMMonoData_inv *fn_u32AllocateVoiceTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, Desc_inv *desc)
{
    tdstChannelMEMMonoData_inv *r = 0;
    tdstChannelMEMMonoData_inv *v = fn_pstParseTdstChannelMEMMonoMngr_inv(pool, 0, (ChannelMEMMonoDataFn_inv)fn_iIsFreeTdstChannelMEMMonoMngr_inv);

    if (v != 0)
        r = fn_u32AllocateTdstChannelMEMMonoData_inv(v, desc);
    return r;
}

int fn_u32ReallocateVoiceTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32AllocateTdstChannelMEMMonoData_inv(v, a0);
    return r;
}

int fn_u32DesallocateVoiceTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_u32DesallocateTdstChannelMEMMonoData_inv(v);
    return r;
}

int fn_iSetOwnerTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iSetOwnerTdstChannelMEMMonoData_inv(v, a0);
    return r;
}

int fn_iConnectTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iConnectTdstChannelMEMMonoData_inv(v, a0);
    return r;
}

int fn_bIsAllocatedTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = v->f.bAllocated != 0;
    return r;
}

int fn_iNotifyTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iNotifyTdstChannelMEMMonoData_inv(v, a0, a1, a2);
    return r;
}

int fn_iStartTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStartTdstChannelMEMMonoData_inv(v, a0);
    return r;
}

int fn_iStopTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iStopTdstChannelMEMMonoData_inv(v);
    return r;
}

int fn_iReadTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0, int a1, int a2)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = 0;
    if (v != 0)
        r = (int)fn_u32ReadTdstChannelMEMMonoData_inv(v, a0, a1, a2);
    return r;
}

int fn_iGetPosTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h, int a0)
{
    int r;
    tdstChannelMEMMonoData_inv *v = ValidateElemB_inv(pool, h);

    r = -1;
    if (v != 0)
        r = (int)fn_iGetPosTdstChannelMEMMonoData_inv(v, a0);
    return r;
}

int fn_iOperateCallOneTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoData_inv *v)
{
    fn_vOperateCallTdstChannelMEMMonoData_inv(v);
    return 0;
}

void fn_vOperateCallTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool)
{
    fn_pstParseTdstChannelMEMMonoMngr_inv(pool, 0, (ChannelMEMMonoDataFn_inv)fn_iOperateCallOneTdstChannelMEMMonoMngr_inv);
}

tdstChannelMEMMonoData_inv *fn_pstValidateTdstChannelMEMMonoMngr_inv(tdstChannelMEMMonoMngr_inv *pool, tdstChannelMEMMonoData_inv *h)
{
    return ValidateElemB_inv(pool, h);
}
